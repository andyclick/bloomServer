#include <signal.h>
#include "mylock.h"    
#include <sys/types.h>
#include <unistd.h>
#include "InfoCrawler.h"
#include "readconf.h"
#include "DBAccess.h"
#include "utilcpp.h"
#include "test.h"
#include "client_test.h"
#ifdef COREDUMP
#include <google/coredumper.h>
#endif



InfoCrawler *InfoCrawler::_instance = NULL;
static void sigcleanup(int sig);

pthread_mutex_t InfoCrawler::server_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t InfoCrawler::server_cond = PTHREAD_COND_INITIALIZER;
#ifdef URLMEMCACHEDB
pthread_rwlock_t InfoCrawler::urlthreadlocalrwlock = PTHREAD_RWLOCK_INITIALIZER;
#endif
#ifdef HTMLMEMCACHEDB
pthread_rwlock_t InfoCrawler::htmlthreadlocalrwlock = PTHREAD_RWLOCK_INITIALIZER;
#endif
pthread_mutex_t InfoCrawler::delete_data_wait_mutex= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t InfoCrawler::delete_data_wait_cond= PTHREAD_COND_INITIALIZER;
pthread_rwlock_t InfoCrawler::hyper_clientsrwlock= PTHREAD_RWLOCK_INITIALIZER;
InfoCrawler::InfoCrawler(int sttype):starttype(sttype) {    
    if (access("log4cxx.xml", F_OK) == -1) {
        printf("Can not start server, can't find log4cxx.xml\n");
    }
    //initialize log
    _logGlobalCtrl = new LogGlobalCtrl;
    string p = "log4cxx.xml";
    xml::DOMConfigurator::configureAndWatch(p, 3000);

    _logGlobalCtrl->infolog = log4cxx::Logger::getLogger("in");
    _logGlobalCtrl->importantlog = log4cxx::Logger::getLogger("im");
    _logGlobalCtrl->veryimportantlog = log4cxx::Logger::getLogger("so");
    _logGlobalCtrl->errorlog = log4cxx::Logger::getLogger("er");
    mylog_info(_logGlobalCtrl->infolog, "initialize info log success - %s:%s:%d", INFO_LOG_SUFFIX);
    mylog_info(_logGlobalCtrl->importantlog, "initialize important log success - %s:%s:%d", INFO_LOG_SUFFIX);
    mylog_info(_logGlobalCtrl->veryimportantlog, "initialize very important log success - %s:%s:%d", INFO_LOG_SUFFIX);
    mylog_error(_logGlobalCtrl->errorlog, "initialize error log success - %s:%s:%d", INFO_LOG_SUFFIX);

    /* ----------- added by andy 2014/11/02 --------------- */
    _ThriftManager = new ThriftManager(this->_logGlobalCtrl);
    //_ThriftManager->start_thrift_server();
    /* ---------------------------------------------------- */
    _icconfig = new ICCONFIG;

}

InfoCrawler::~InfoCrawler() {
    delete _checkpointManager; 
    delete _dbManager; 
    delete _urlAnalyseManager; 
    delete _localDbManager; 
    delete _PageManager; 
    delete _fetcherManager; 
    delete _logStatisticManager; 
    delete _externalManager;
    delete _taskScheduleManager;
    delete _icconfig;
    mylock::uninit();
}

#ifdef HTMLMEMCACHEDB
MemCacheClient *InfoCrawler::getHtmlMcLocalThread() {
    MemCacheClient *mc = NULL; 
    map<pthread_t, MemCacheClient *>::iterator iter;
    pthread_rwlock_rdlock(&htmlthreadlocalrwlock);
    if (((iter = contentmclocals.find(pthread_self())) != contentmclocals.end())) {
        mc = iter->second;
        pthread_rwlock_unlock(&htmlthreadlocalrwlock);
    } else {
        pthread_rwlock_unlock(&htmlthreadlocalrwlock);

        pthread_rwlock_wrlock(&htmlthreadlocalrwlock);
        mc  = new MemCacheClient;
        mc->AddServer(_icconfig->contentmemcachedbip);
        contentmclocals[pthread_self()] = mc;
        pthread_rwlock_unlock(&htmlthreadlocalrwlock);
    }
    return mc;
}
#endif
/*
   Hyper_client_namespace  * InfoCrawler::get_hyper_clientThread() {
   Hyper_client_namespace * hcn=NULL;
//Thrift::Client * client_=NULL;
map<pthread_t,Hyper_client_namespace *>::iterator iter;
pthread_rwlock_rdlock(&hyper_clientsrwlock);
if (((iter = hyper_clients.find(pthread_self())) != hyper_clients.end())) {
hcn=iter->second;
pthread_rwlock_unlock(&hyper_clientsrwlock);
if(hcn)
{
return hcn;
}    
}
else
{
pthread_rwlock_unlock(&hyper_clientsrwlock);
}

pthread_rwlock_wrlock(&hyper_clientsrwlock);
try
{
hcn =new Hyper_client_namespace;
Thrift::Client *client_=new Thrift::Client(InfoCrawler::getInstance()->getConf()->hyperaddress,InfoCrawler::getInstance()->getConf()->hyperport);
// if (!client_->namespace_exists(InfoCrawler::getInstance()->getConf()->hypertablenamespace))
// client_->namespace_create(InfoCrawler::getInstance()->getConf()->hypertablenamespace);
hcn->myns_=client_->namespace_open(InfoCrawler::getInstance()->getConf()->hypertablenamespace);
hcn->client=client_;
hyper_clients[pthread_self()]=hcn;
}
catch(ClientException &e)
{
hcn=NULL;
}
pthread_rwlock_unlock(&hyper_clientsrwlock);

return hcn;
}
*/
#ifdef URLMEMCACHEDB
MemCacheClient *InfoCrawler::getUrlMcLocalThread() {
    MemCacheClient *mc = NULL; 
    map<pthread_t, MemCacheClient *>::iterator iter;
    pthread_rwlock_rdlock(&urlthreadlocalrwlock);
    if (((iter = urlmclocals.find(pthread_self())) != urlmclocals.end())) {
        mc = iter->second;
        pthread_rwlock_unlock(&urlthreadlocalrwlock);
        if (mc) {
            return mc;
        }
    } else {
        pthread_rwlock_unlock(&urlthreadlocalrwlock);
    }

    pthread_rwlock_wrlock(&urlthreadlocalrwlock);
    mc = new MemCacheClient;
    mc->AddServer(_icconfig->urlmemcachedbip);
    urlmclocals[pthread_self()] = mc;
    pthread_rwlock_unlock(&urlthreadlocalrwlock);

    return mc;
}

void InfoCrawler::deleteUrlMcLocalThread() {
    map<pthread_t, MemCacheClient *>::iterator iter;
    pthread_rwlock_wrlock(&urlthreadlocalrwlock);
    if (((iter = urlmclocals.find(pthread_self())) != urlmclocals.end())) {
        MemCacheClient *mc = iter->second;
        delete mc;
        iter->second = NULL;
    }
    pthread_rwlock_unlock(&urlthreadlocalrwlock);
}

#endif


InfoCrawler* const InfoCrawler::getInstance(int starttype)
{
    if(_instance == NULL) {
        _instance = new InfoCrawler(starttype);
    } 
    return _instance;
} 

void InfoCrawler::destroy() {
    delete this;
}

void InfoCrawler::startServer() {
    mylog_info(_logGlobalCtrl->infolog, "InfoCrawler: Begin at: ");
    if (access("bloomfilter.conf", R_OK) == -1) {
        mylog_fatal(_logGlobalCtrl->veryimportantlog, "Can not start server, can't find spider1.conf - %s:%s:%d",INFO_LOG_SUFFIX);
    }


    char strTime[128];
    time_t tDate;
    memset(strTime,0,128);
    time(&tDate);
    strftime(strTime, 127,"%a, %d %b %Y %H:%M:%S GMT", localtime(&tDate));
    int i;
#ifdef NSIG
    for (i = 1; i < NSIG; i++) {
#else
        for (i = 1; i < _sys_nsig; i++) {
#endif
            if (i == SIGCHLD || i == SIGWINCH || i == SIGSEGV || i == SIGPIPE) {
                continue;
            } else {
                signal(i, sigcleanup);
            }
        }
        signal(SIGCHLD, SIG_IGN);
        sig_phandler();

        char filename[128] ;filename[0] = 0;
        _ThriftManager->start_thrift_server();


    }
    int InfoCrawler::startDeleteDataThread() {
        pthread_t *tid = (pthread_t*)malloc(1 * sizeof(pthread_t));
        if (tid == NULL) {
            mylog_fatal(_logGlobalCtrl->veryimportantlog, "malloc error for tartDeleteDataThread hread - %s:%s:%d",INFO_LOG_SUFFIX);
        }

        if(pthread_create(tid, NULL, DeleteDataThread, this)) {
            mylog_fatal(_logGlobalCtrl->veryimportantlog, "create DeleteDataThread threads error - %s:%s:%d",INFO_LOG_SUFFIX);
        }

        pthread_detach(*tid);

        free(tid);

        return 0;
    }
    void *InfoCrawler::DeleteDataThread(void *manager ) {
        timespec mytime;
        InfoCrawler *m = static_cast<InfoCrawler*>(manager);
        while(!m->serverstop) {
            if (m->serverstop) break;
            mytime.tv_sec = time(NULL)+(60);  //Wait for 300 second
            mytime.tv_nsec = 0;
            pthread_mutex_lock(&delete_data_wait_mutex);
            pthread_cond_timedwait(&delete_data_wait_cond, &delete_data_wait_mutex, (const struct timespec *)&mytime);
            pthread_mutex_unlock(&delete_data_wait_mutex);
            if (m->ReadDeleteFlag()) m->deletedata();
        }
    }

    void InfoCrawler::stopServer() {
        if (starttype == START_TYPE_ONLY_EXTERNAL) {
            _externalManager->stop();
        } else {
            _localDbManager->stop();
            _urlAnalyseManager->stop();
            _fetcherManager->stop();
            _logStatisticManager->stop();
            _taskScheduleManager->stop();
            _externalManager->stop();
            _checkpointManager->stop();
            _dbManager->stop();
            _PageManager->stop();
        }

        beforestop();
        pthread_cond_signal(&server_cond);
    }

    static void sigcleanup(int sig)
    {
        static int sigterm_count = 1;
        InfoCrawler *infoCrawler = InfoCrawler::getInstance();
        ICCONFIG *icconf = infoCrawler->getConf();
        if (getpid() == infoCrawler->getMasterpid())
            unlink(icconf->pidfile);
        if (sig == SIGTERM && sigterm_count++ > icconf->maxsignalterm) {
            //mylog_fatal(infoCrawler->_logGlobalCtrl->veryimportantlog, "As so many SIGTERM signals received, compulsive exit - %s:%s:%d", INFO_LOG_SUFFIX);
        }
        infoCrawler->getThriftManager()->dump();
        //infoCrawler->stopServer();
        exit(0);
    }


    void InfoCrawler::initialize() {
        //make dir if not exists
        ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
        if (access(icconfig->dbpath, R_OK | W_OK) == -1) {
            make_path(icconfig->dbpath);
        }

        if (access(icconfig->checkpointpath, R_OK | W_OK) == -1) {
            make_path(icconfig->checkpointpath);
        }


        if (access(icconfig->logdir, R_OK | W_OK) == -1) {
            make_path(icconfig->logdir);
        }

        DBAccess::getInstance()->setDir(icconfig->dbpath);
    }

    void InfoCrawler::beforestop() {
        DBAccess::getInstance()->destroy();
    }

    void InfoCrawler::dump() {
        _checkpointManager->dump();
        _dbManager->dump(); 
        _urlAnalyseManager->dump(); 
        _localDbManager->dump(); 
        _PageManager->dump(); 
        _fetcherManager->dump(); 
        _logStatisticManager->dump(); 
        _taskScheduleManager->dump(); 
        _externalManager->dump(); 
    }

    void InfoCrawler::retrieve() {
        _checkpointManager->retrieve();
        _dbManager->retrieve(); 
        _urlAnalyseManager->retrieve(); 
        _localDbManager->retrieve(); 
        _PageManager->retrieve(); 
        _fetcherManager->retrieve(); 
        _logStatisticManager->retrieve(); 
        _taskScheduleManager->retrieve(); 
        _externalManager->retrieve(); 
    }
    bool InfoCrawler::ReadDeleteFlag()
    {
        int deleteflag = 0;
        char filename[128];filename[0] = 0;
        int ret = 0;
        ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
        vector<string> dirs;
        dirs.push_back(icconfig->checkpointpath);

        sprintf(filename, "%s/%s", icconfig->checkpointpath, DELETE_DATA_FLAG_FILE);
        FILE *f  = fopen(filename, "r");
        if (f) {
            char key[32] = ""; 
            ret = fread(key, 1, 32, f);
            if (ret == 0) {
                fclose(f);
                mylog_error(_logGlobalCtrl->errorlog, "can not read flag in readfetch for %s - %s:%s:%d", DELETE_DATA_FLAG_FILE,INFO_LOG_SUFFIX);
            } else {
                sscanf(key, "%d", &deleteflag);
                fclose(f);
                if (deleteflag == 1)
                {
                    f  = fopen(filename, "w");
                    if (f)
                    {
                        char key[4] = {'0',0};
                        ret = fwrite(key, 1, 1, f);
                        if (ret == 0) {
                            mylog_error(_logGlobalCtrl->errorlog, "can not write flag in  %s - %s:%s:%d", DELETE_DATA_FLAG_FILE,INFO_LOG_SUFFIX);
                        }
                        fclose(f);
                    }
                }
            }
        } else {
            mylog_error(_logGlobalCtrl->errorlog, "can not open flag file in read flag for %s - %s:%s:%d", DELETE_DATA_FLAG_FILE,INFO_LOG_SUFFIX);
        }
        if (deleteflag == 1)
        {
            return true;
        }else
        {
            return false;
        }
    }
    void InfoCrawler::deletedata()
    {
        mylog_info(_logGlobalCtrl->infolog, "start deletedata -%s:%s:%d\n", INFO_LOG_SUFFIX);
        _taskScheduleManager->stopinserttask();
        time_t timeToStop = time(NULL) + _icconfig->waitstopinterval; 
        while((_urlAnalyseManager->hostToFifoqueuesize() != 0 ) && time(NULL) < timeToStop )
        {
            if (serverstop) break;
            mylog_info(_logGlobalCtrl->infolog, "now time %d timeTostop %d hostToFifoqueuesize %d -%s:%s:%d", time(NULL),timeToStop,_urlAnalyseManager->hostToFifoqueuesize(),INFO_LOG_SUFFIX);
            my_sleep(60000000); //60 seconds
        }
        _localDbManager->stop();
        my_sleep(60000000); //60 seconds
        mylog_info(_logGlobalCtrl->infolog, "top _localDbManager canstop %d - %s:%s:%d", _localDbManager->canStoped,INFO_LOG_SUFFIX);
        _urlAnalyseManager->stop();
        mylog_info(_logGlobalCtrl->infolog, "top _urlAnalyseManager canstop %d - %s:%s:%d", _urlAnalyseManager->canStoped,INFO_LOG_SUFFIX);
        _fetcherManager->stop();
        mylog_info(_logGlobalCtrl->infolog, "top _fetcherManager canstop %d - %s:%s:%d", _fetcherManager->canStoped,INFO_LOG_SUFFIX);
        _logStatisticManager->stop();
        mylog_info(_logGlobalCtrl->infolog, "top _logStatisticManager canstop %d - %s:%s:%d", _logStatisticManager->canStoped,INFO_LOG_SUFFIX);
        _taskScheduleManager->stop(); 
        mylog_info(_logGlobalCtrl->infolog, "top _taskScheduleManager canstop %d - %s:%s:%d", _taskScheduleManager->canStoped,INFO_LOG_SUFFIX);
        _checkpointManager->stop();
        mylog_info(_logGlobalCtrl->infolog, "top _checkpointManager canstop %d - %s:%s:%d", _checkpointManager->canStoped,INFO_LOG_SUFFIX);
        _dbManager->stop();
        //    mylog_info(_logGlobalCtrl->infolog, "top _dbManager canstop %d - %s:%s:%d", _dbManager->canStoped,INFO_LOG_SUFFIX);
        _PageManager->stop();
        mylog_info(_logGlobalCtrl->infolog, "top _PageManager canstop %d - %s:%s:%d", _PageManager->canStoped,INFO_LOG_SUFFIX);
        while(!(_localDbManager->IsReadFetchedEnd()))
        {
            if (serverstop) break;
            my_sleep(10000000); //60 seconds
        }
        _externalManager->stop();
        mylog_info(_logGlobalCtrl->infolog, "top _externalManager canstop %d - %s:%s:%d", _externalManager->canStoped,INFO_LOG_SUFFIX);
        DBAccess::getInstance()->destroydbs();
        if (!(_localDbManager->deleteAllData()))
        {
            mylog_error(_logGlobalCtrl->errorlog, "can not delete all data - %s:%s:%d",INFO_LOG_SUFFIX);
        }

        if (starttype == START_TYPE_ONLY_EXTERNAL) {
            _externalManager->start();
        } else {
            _checkpointManager->start();
            _localDbManager->start();
            _PageManager->start();
            _dbManager->start();
            _taskScheduleManager->start();
            _urlAnalyseManager->start();
            _fetcherManager->start();
            _logStatisticManager->start();
            _externalManager->start();

            //_dbHypertableManager->start();

            _checkpointManager->started();
            _localDbManager->started();
            _PageManager->started();
            _dbManager->started();
            _taskScheduleManager->started();
            _urlAnalyseManager->started();
            _fetcherManager->started();
            _logStatisticManager->started();
            _externalManager->started();
        }

    }

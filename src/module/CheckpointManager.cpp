#include "CheckpointManager.h"
#include "InfoCrawler.h"
#include <time.h>

pthread_mutex_t CheckpointManager::mutexMaxid = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t CheckpointManager::mutex_checkpointthread = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t CheckpointManager::mux = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t CheckpointManager::checkpiontthread_wait_cond = PTHREAD_COND_INITIALIZER;

CheckpointManager::CheckpointManager(LogGlobalCtrl * pLogGlobalCtrl) {
   m_pLogGlobalCtrl = pLogGlobalCtrl;
}

CheckpointManager::~CheckpointManager() {

}

int CheckpointManager::start() {
    Manager::start();
    recover();
    startCheckpointThread();
    mylog_info(m_pLogGlobalCtrl->infolog, "checkpoint server start - %s:%s:%d", INFO_LOG_SUFFIX);
    return 0;
}

int CheckpointManager::stop() {
    int ret = Manager::stop();

    while(!canstoped()) {
        pthread_cond_signal(&checkpiontthread_wait_cond);
        my_sleep(500000); //0.5 seconds
    }

    return ret;
}

ulonglong CheckpointManager::getMaxUrlId() {
    ulonglong maxid;
    pthread_mutex_lock(&mutexMaxid);
    maxid = maxurlid++;
    pthread_mutex_unlock(&mutexMaxid);
    return maxid;
}

ulonglong CheckpointManager::GetMaxID(unsigned long pcid)
{
    static ulonglong last_millisecond = 0;
    static long last_id = 0;
    ulonglong millisecond = 0;
    ulonglong second_tmp = 0;
    ulonglong ret_tmp = 0;

    struct timeval tv;

    pthread_mutex_lock(&mux);

start:
    if(gettimeofday(&tv,NULL) == -1)
    {
        printf("gettimeofday error!\n");
        pthread_mutex_unlock(&mux);

        return 0;
    }

    second_tmp = tv.tv_sec;
    millisecond = second_tmp*1000 + tv.tv_usec/1000;

    if(millisecond > last_millisecond)
    {
        last_millisecond = millisecond;
        last_id = 0;
        ret_tmp = (millisecond << 23 | (pcid << 12));
        pthread_mutex_unlock(&mux);

        return ret_tmp;
    }
    else if(millisecond == last_millisecond)
    {
        if(last_id > 4095)
        {
            sleep(1);
            goto start;
        }
        else
        {
            last_id++;
            ret_tmp = (millisecond << 23 | (pcid << 12)) | last_id;
            pthread_mutex_unlock(&mux);

            return ret_tmp; 
        }
    }
    else
    {
        sleep(1);
        goto start;
    }
}       

int CheckpointManager::retrieveMaxUrlId() {
    //if can't not find maxurlid from file, then read db and find the maxurlid
    pthread_mutex_lock(&mutexMaxid);
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    char maxidbuf[32] ;maxidbuf[0] = 0;
    char filename[256] ;filename[0] = 0;
    sprintf(filename, "%s/%s", icconfig->checkpointpath, FILE_MAX_ID); 
    FILE *f = fopen(filename, "r");
    if (f) {
        size_t readlen = fread(maxidbuf, 1, sizeof(ulonglong), f);
        if (readlen <= 0) {
            mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "can not read maxid file for checkpoint - %s:%s:%d", INFO_LOG_SUFFIX);
        }
        fclose(f);
        sscanf(maxidbuf, "%llu", &maxurlid);
        maxurlid++;
    } else {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "can not open maxid file for checkpoint - %s:%s:%d", INFO_LOG_SUFFIX);
    }
    pthread_mutex_unlock(&mutexMaxid);
    return 0;
}

int CheckpointManager::startCheckpointThread() {
    pthread_t *tid = (pthread_t*)malloc(1 * sizeof(pthread_t));
    if (tid == NULL) {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "malloc error for startcheckpoint thread - %s:%s:%d", INFO_LOG_SUFFIX);
    }       

    if(pthread_create(tid, NULL, startCheckpoint, this)) {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "create startcheckpoint threads error - %s:%s:%d", INFO_LOG_SUFFIX);
    }
    pthread_detach(*tid);
    free(tid);

    return 0;
}

void *CheckpointManager::startCheckpoint(void *manager) {

    CheckpointManager *m =  static_cast<CheckpointManager *>(manager);
    InfoCrawler *infocrawler = InfoCrawler::getInstance();

    pthread_t *tid = (pthread_t*)malloc(1 * sizeof(pthread_t));
    if (tid == NULL) {
        mylog_fatal(m->m_pLogGlobalCtrl->veryimportantlog, "malloc error for startchekpoint - %s:%s:%d", INFO_LOG_SUFFIX);
    }

    if(pthread_create(tid, NULL, checkpointThread, manager)) {
        mylog_fatal(m->m_pLogGlobalCtrl->veryimportantlog, "create checkpointthread error - %s:%s:%d", INFO_LOG_SUFFIX);
    }

    pthread_join(*tid, NULL);

    free(tid);
 
    m->stoped();

    mylog_info(m->m_pLogGlobalCtrl->infolog, "checkpoint thread is over - %s:%s:%d", INFO_LOG_SUFFIX);
}

void *CheckpointManager::checkpointThread(void *manager) {

    CheckpointManager *m=  static_cast<CheckpointManager *>(manager);
    while(m->running()) {
        ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += icconfig->checkpointtime;

        mylog_info(m->m_pLogGlobalCtrl->infolog, "running is %d  %p checkpointthread for CheckpointManager - %s:%s:%d", m->running(),  manager,INFO_LOG_SUFFIX);
        pthread_mutex_lock(&mutex_checkpointthread);
        pthread_cond_timedwait(&checkpiontthread_wait_cond, &mutex_checkpointthread, &ts);
        pthread_mutex_unlock(&mutex_checkpointthread);

        if (!(m->running()))  {
            mylog_info(m->m_pLogGlobalCtrl->infolog, "checkpoint before server stop - %s:%s:%d", INFO_LOG_SUFFIX);
            m->checkpoint();
            break; //return
        } else {
            mylog_info(m->m_pLogGlobalCtrl->infolog, "checkpoint regularly - %s:%s:%d", INFO_LOG_SUFFIX);
            m->checkpoint();
        }
    }
    m->checkpoint();
    mylog_info(m->m_pLogGlobalCtrl->infolog, "checkpoint before server stop - %s:%s:%d", INFO_LOG_SUFFIX);
    //pthread_exit (NULL);    
}

int CheckpointManager::checkpoint() {
    //write data into db for backup
    InfoCrawler::getInstance()->dump();
    return 0;
}

int CheckpointManager::recover() {
    //write data into db for backup
    InfoCrawler::getInstance()->retrieve();
    return 0;
}

void CheckpointManager::dump() {
    //write max id;
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    ulonglong maxid = getMaxUrlId();
    char maxidbuf[32] ;maxidbuf[0] = 0;
    char filename[256] ;filename[0] = 0;
    sprintf(filename, "%s/%s", icconfig->checkpointpath, FILE_MAX_ID); 
    int len = sprintf(maxidbuf, "%llu", maxid);
    FILE *f = fopen(filename, "w+");
    if (f) {
        size_t writelen = fwrite(maxidbuf, 1, len, f);
        if (writelen != len) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "can not write maxid file in checkpoint - %s:%s:%d:%d", ERROR_LOG_SUFFIX);
        }
        fclose(f);
    } else {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not open maxid file in checkpoint - %s:%s:%d:%d", ERROR_LOG_SUFFIX);
    }
}

void CheckpointManager::retrieve() { 
    //retrieveMaxUrlId();
}

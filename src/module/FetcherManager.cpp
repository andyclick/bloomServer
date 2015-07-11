#include "mylock.h"
#include <stdlib.h>
#include "FetcherManager.h"
#include "InfoCrawler.h"
#include "UrlAnalyseManager.h"
#include "Url.h"
#include "util.h"
#include "Page.h"
#include "Http.h"
#include <curl/curl.h>
#include "test.h"
#include "client_test.h"
pthread_mutex_t FetcherManager::mutex_deleteother = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t curl_dns_rwlock = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t curl_cookie_rwlock = PTHREAD_RWLOCK_INITIALIZER;


string TimeToString1()
{
   char charFormat[30];
   struct timeval nowtimeval;
   gettimeofday(&nowtimeval,0);

   time_t now;
   struct tm *timenow;

   time(&now);
   timenow = localtime(&now);

   sprintf(charFormat, "%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%3.3d", timenow->tm_year + 1900,timenow->tm_mon + 1,timenow->tm_mday, timenow->tm_hour,timenow->tm_min,timenow->tm_sec, nowtimeval.tv_usec/1000);
   return charFormat;
}


FetcherManager::FetcherManager(LogGlobalCtrl * pLogGlobalCtrl) {
    m_pLogGlobalCtrl = pLogGlobalCtrl;
    sh = NULL;
}

FetcherManager::~FetcherManager() {

}

static void lock_function(CURL *handle, curl_lock_data data, curl_lock_access access, void *userptr) {
    if (data == CURL_LOCK_DATA_COOKIE) {
        if (access == CURL_LOCK_ACCESS_SINGLE) {
            pthread_rwlock_wrlock(&curl_cookie_rwlock);
        } else if (access == CURL_LOCK_ACCESS_SHARED) {
            pthread_rwlock_rdlock(&curl_cookie_rwlock);
        } else {
			pthread_rwlock_wrlock(&curl_cookie_rwlock);
		}
    } else if (data == CURL_LOCK_DATA_DNS) {
        if (access == CURL_LOCK_ACCESS_SINGLE) {
            pthread_rwlock_wrlock(&curl_dns_rwlock); 
        } else if (access == CURL_LOCK_ACCESS_SHARED) {
            pthread_rwlock_rdlock(&curl_dns_rwlock);
        } else {
			pthread_rwlock_rdlock(&curl_dns_rwlock);
		}
    }
}   

static void unlock_function(CURL *handle, curl_lock_data data, void *userptr) {
    if (data == CURL_LOCK_DATA_COOKIE) {
        pthread_rwlock_unlock(&curl_cookie_rwlock);
    } else if (data == CURL_LOCK_DATA_DNS) {
        pthread_rwlock_unlock(&curl_dns_rwlock);
    }
}

int FetcherManager::start() {
    Manager::start();
	curl_global_init(CURL_GLOBAL_ALL);
    sh = curl_share_init();
    curl_share_setopt(sh, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
    curl_share_setopt(sh, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
    curl_share_setopt(sh, CURLSHOPT_LOCKFUNC, lock_function);
    curl_share_setopt(sh, CURLSHOPT_UNLOCKFUNC, unlock_function);
    startCrawlerThread();
}

int FetcherManager::stop() {
    int ret = Manager::stop();

    while(!canstoped()) {
        my_sleep(500000); //0.5 seconds
    }

    curl_share_cleanup(sh);
    curl_global_cleanup();
    return ret;
}

int FetcherManager::startCrawlerThread() {
    pthread_t *tid = (pthread_t*)malloc(1 * sizeof(pthread_t));
    if (tid == NULL) {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "malloc error - %s:%s:%d", INFO_LOG_SUFFIX);
    }       

    if(pthread_create(tid, NULL, startCrawler, this)) {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "create threads error - %s:%s:%d", INFO_LOG_SUFFIX);
    }
        
    pthread_detach(*tid);
    free(tid);
    return 0;
}

void *FetcherManager::startCrawler(void *manager) {

    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    ICCONFIG *icconfig = infocrawler->getConf();
    FetcherManager *fetcherManager =  static_cast<FetcherManager *>(manager);

    pthread_t *tids = (pthread_t*)malloc(icconfig->fetchingthreads * sizeof(pthread_t));
    if (tids == NULL) {
        mylog_fatal(fetcherManager->m_pLogGlobalCtrl->veryimportantlog, "malloc error - %s:%s:%d", INFO_LOG_SUFFIX);
    }

    for(int i = 0; i < icconfig->fetchingthreads; i++) {
        if(pthread_create(&tids[i], NULL, fetchThread, manager)) {
            mylog_fatal(fetcherManager->m_pLogGlobalCtrl->veryimportantlog, "create threads error - %s:%s:%d", INFO_LOG_SUFFIX);
        }
    }

    for (unsigned int i = 0; i < icconfig->fetchingthreads; i++) {
        (void)pthread_join(tids[i], NULL);
    }

    free(tids);

    fetcherManager->stoped();

    mylog_info(fetcherManager->m_pLogGlobalCtrl->infolog, "Crawler thread is over - %s:%s:%d",INFO_LOG_SUFFIX);

    return NULL;
}

void *FetcherManager::fetchThread(void *manager) {
    FetcherManager *fetcherManager =  static_cast<FetcherManager *>(manager);
    fetcherManager->fetch();
    //pthread_exit(NULL);
    return NULL;
}



int FetcherManager::fetch() {
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    UrlAnalyseManager *urlAnalyseManager = infocrawler->getUrlAnalyseManager(); 

    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, ""); //just to start the cookie engine
    curl_easy_setopt(curl, CURLOPT_SHARE, sh);
    
    while(running()) {
        curl_easy_reset(curl);

        UrlNode *urlnode = NULL;
        bool html_from_outer= false;

        urlnode = urlAnalyseManager->getUrlFromOuterHtml();
        if (urlnode) {
            html_from_outer = true;
        } else {
            urlnode = urlAnalyseManager->getUrl();
        }

        if (urlnode == NULL) {
            my_sleep(100 * 1000); //0.1s
            continue;
        }
        if (!(urlnode->task))
        {
            mylog_info(m_pLogGlobalCtrl->infolog, "node task is null %s - %s:%s:%d",urlnode->url,INFO_LOG_SUFFIX);
        }
        TaskOtherInfo *taskother = infocrawler->getTaskScheduleManager()->getTaskOtherInfo(urlnode->taskid);
        int taskbatch = urlnode->taskbatch;
        if (urlnode->needtologin) {
            //need to login and cookie is null
            if (!(infocrawler->getTaskScheduleManager()->getCookieFromTask(urlnode->taskid))) {
                if (taskother->fetchingcookie){
                    infocrawler->getUrlAnalyseManager()->insertUrl(urlnode);
                    infocrawler->getTaskScheduleManager()->decreaseTaskUrlNum(urlnode->task,taskbatch);
#ifdef URLMEMCACHEDB
                    infocrawler->deleteUrlMcLocalThread(); 
#endif
                    continue;
                } else {
                    doLogin(curl, urlnode->task, urlnode);
                }
            }
        }

        /*if (urlnode->task->sourcetype == SOURCE_TYPE_COMPANY && urlnode->type & URL_TYPE_HOMEPAGE)
        {
            strcat(urlnode->url, "&event=32698647");
            strcpy(urlnode->refererurl, "http://search.china.alibaba.com/tools/validate_redirect.htm?ru=http%253A%252F%252Fsearch.china.alibaba.com%252Fcompany%252Fcompany_search.htm%253Fkeywords%253D%25CA%25D6%25BB%25FA%2526pageSize%253D30%2526n%253Dy%2526showStyle%253Dpopular%2526beginPage%253D4&event=32698647&n=y");
        }*/
        CUrl url;
        url.parse(urlnode->url);
        //wrong url format
        if (url.getUrl().empty()) {
            infocrawler->getTaskScheduleManager()->increaseTaskErrorUrlNum(urlnode->taskid);
            infocrawler->getTaskScheduleManager()->decreaseTaskUrlNum(urlnode->task, taskbatch);
            infocrawler->getLocalDbManager()->decidesaveFetched(urlnode);
            delete urlnode;
#ifdef URLMEMCACHEDB
            infocrawler->deleteUrlMcLocalThread(); 
#endif
            continue;
        }
        Page page;
        Buffer *content = create_buffer(DEFAULT_PAGE_BUF_SIZE);


        //do fetch
        HttpProtocol httpprotocol;
        char downstatistic[512] ;downstatistic[0] = 0;
        RESPONSE_HEADER rheader;

//        mylog_info(m_pLogGlobalCtrl->infolog, "before fetch %s %s %llu %d %d  - %s:%s:%d",url.getUrl().c_str(), urlnode->url, urlnode->id, urlnode->taskid, urlnode->errornum,INFO_LOG_SUFFIX);
        //int ret = httpprotocol.fetch(url, content, urlnode, page, infocrawler->getConf()->httptimeout,urlnode->task->tasksendtype);
//        int ret = httpprotocol.curl_fetch(curl, url, content, urlnode, infocrawler->getConf()->httptimeout, urlnode->task->tasksendtype, &rheader, downstatistic);
        int sendtype = urlnode->task->tasksendtype;
        if (urlnode->task->sourcetype == SOURCE_TYPE_COMPANY && urlnode->type & URL_TYPE_HOMEPAGE)
        {   
		/*FILE * f = fopen("ali.txt", "rb");
		char line[1024] = {0};
		int i = 0;
		string cookie;
		string post;
		while(fgets(line, 1023, f)) {
			char *newline = strtrim(line, NULL);
			if (i++ == 0) {
				cookie = newline;
			} else {
				post = newline;
			}
		}
		fclose(f);
*/
            sendtype = REQUEST_TYPE_GET;
        }

        int ret = 0;
        if (!html_from_outer) {
            ret = httpprotocol.curl_fetch(curl, url, content, urlnode, infocrawler->getConf()->httptimeout, sendtype, &rheader, downstatistic);
            mylog_info(m_pLogGlobalCtrl->infolog, "after fetched %s %s %d - %s:%s:%d",url.getUrl().c_str(), downstatistic, ret,INFO_LOG_SUFFIX);
        } else {
           add_buffer(content, (char *)urlnode->html.c_str(), urlnode->html.length()); 
           ret = urlnode->html.length();
           mylog_info(m_pLogGlobalCtrl->infolog, "get url from outer %s %d - %s:%s:%d", url.getUrl().c_str(), ret,INFO_LOG_SUFFIX);
        }

        /*if (ret == HTTP_FETCH_RET_REDIRECT) { //redirect
            int redirectnum = urlnode->redirectnum +1;
            if (redirectnum <= URL_FETCH_REDIRECT_TIMES)
            {
                UrlNode *newurlnode = new UrlNode(urlnode->task,urlnode->topicsource,urlnode->title,urlnode->taskbatch,(char *)urlnode->fatherurl,(char *)page.m_sLocation.c_str(), urlnode->other, urlnode->maxtype,urlnode->type, 0, urlnode->id,redirectnum ,urlnode->page,urlnode->layerid,urlnode->bbsid,urlnode->needtologin);

                newurlnode->insertother(URLNODE_OTHER_TYPE_COOKIE,(char *)page.m_sCookie.c_str(), page.m_sCookie.length());
                errorlog("ERROR: fetched %s %s relocated to %s %llu %d\n", url.getUrl().c_str(), urlnode->url, newurlnode->url, newurlnode->id, newurlnode->taskid);
                infocrawler->getUrlAnalyseManager()->insertUrl(newurlnode);
            }else
            {
                errorlog("ERROR: redirectunm > %d fetched %s %s relocated to %s %d\n", URL_FETCH_REDIRECT_TIMES, url.getUrl().c_str(), urlnode->url, (char * )page.m_sLocation.c_str(), urlnode->taskid);
            }
            urlnode->errornum = 0;
            */
        if (ret == HTTP_FETCH_RET_ERROR) {//just discard
            urlnode->errornum++;
            mylog_error(m_pLogGlobalCtrl->errorlog, "fetched %s  - %s:%s:%d:%d", url.getUrl().c_str(),INFO_LOG_SUFFIX,urlnode->errornum);
        /*} else if (ret == HTTP_FETCH_RET_ERROR_INVALIDHOST) { //invalid host, can not access
	  urlnode->errornum++;
            errorlog("ERROR: fetched %s invalidhost %d\n", url.getUrl().c_str(), urlnode->errornum);
            */
        } else if (ret == HTTP_FETCH_RET_ERROR_UNACCEPTED) { //content is invalid, discard
            urlnode->errornum = URL_FETCH_RETRY_TIMES; 
            //errorlog("ERROR: fetched %s unaccepted contenttype %d %s\n", url.getUrl().c_str(), urlnode->errornum, page.m_sContentType.c_str());
        } else { //ok
            //increase fetch num
            if (urlnode->type & URL_TYPE_NEEDTOSAVE) 
                infocrawler->getTaskScheduleManager()->increaseFetchNum(urlnode->task);
            
            urlnode->errornum = 0;
            //extract urls and analyse, insert new url into queue
            char nextpageurl[MAX_URL_LEN] ; nextpageurl[0] = 0; 
            int nextpage = infocrawler->getUrlAnalyseManager()->analyseUrls(urlnode, &rheader, content->data, ret, nextpageurl, html_from_outer);
            if (html_from_outer) {
               nextpage = 0; 
               nextpageurl[0] = 0;
            }
            
            //write content to disk if we need, write fetched url into dist
            if (urlnode->type & URL_TYPE_NEEDTOSAVE) {
                if (urlnode->task->sourcetype == SOURCE_TYPE_BBS)
                {
                    char oldurlnodedata[64];
                    int tasktmp = 0;
                    int pagetmp = 0;
                    ulonglong idtmp = 0;
                    if (InfoCrawler::getInstance()->getLocalDbManager()->alreadyfetched(urlnode,oldurlnodedata))
                    {
                        sscanf(oldurlnodedata, "%llu/%d/%*d/%*d/%d/%*u", &idtmp,&pagetmp, &tasktmp);
                        if (pagetmp == urlnode->page)
                        {
                            int rettmp = infocrawler->getLocalDbManager()->erasecontent(idtmp,tasktmp);
                        }
                    }
                }
                if (urlnode->nextpage == 1 && nextpage >1)
                {
                    urlnode->nextpage = nextpage;
                }
                mylock::get_instance()->get(urlnode->id);
                infocrawler->getLocalDbManager()->savecontent(urlnode, &rheader, content->data, ret, nextpage);
                mylock::get_instance()->put(urlnode->id);
                //infocrawler->getPageManager()->SavePage(content->data, ret, urlnode, &rheader);
                
                mylog_info(m_pLogGlobalCtrl->infolog, "fetched %s saved content%d %d title %s urlid %llu taskid %d batchid %d - %s:%s:%d",url.getUrl().c_str(), urlnode->errornum, ret,urlnode->title,urlnode->id, urlnode->taskid, urlnode->taskbatch ,INFO_LOG_SUFFIX);
                /*if ((urlnode->nextpage > 1)&& !(urlnode->type & URL_TYPE_HOMEPAGE)) {//if have nextpage, don't not save fetched
                    mylog_info(m_pLogGlobalCtrl->infolog, "fetched %s have next, so don't not save fetched - %s:%s:%d",url.getUrl().c_str(),INFO_LOG_SUFFIX);
                } else {
                    infocrawler->getLocalDbManager()->saveFetched(urlnode);
                    infocrawler->getLocalDbManager()->saveUrl(urlnode, SAVE_FATHER_URL);
                    mylog_info(m_pLogGlobalCtrl->infolog, "fetched %s save fetched%d - %s:%s:%d",url.getUrl().c_str(),urlnode->errornum,INFO_LOG_SUFFIX);
                }*/
                if (urlnode->nowpage == urlnode->totalpage)
                {
                    infocrawler->getLocalDbManager()->saveFetched(urlnode);
                    infocrawler->getLocalDbManager()->saveUrl(urlnode, SAVE_FATHER_URL);
                    
                    //0128.begin()
                    /* char *content=NULL;
                     char dbname[64] = "";
                     char recordname[64] = "";
                     char urldbname[64] = "";
                     DBAccess *dbaccess = DBAccess::getInstance();

                     getContentDBName1(urlnode, dbname);
                     getRecordKeyName1(urlnode, recordname);
                     getUrlDBName1(urlnode, urldbname);

                     int suffix = dbaccess->load(dbname);
                     string fileno;
                     DBD *dbd = dbaccess->get(suffix, fileno, recordname, NULL);
                     if (dbd != NULL)
                     {
                         DbHypertableManager * dbhyper=InfoCrawler::getInstance()->gethyper();
                          ICCONFIG   *ifcong_=InfoCrawler::getInstance()->getConf();        
                          
                        // dbhyper->get_now_time();
                        string now_=TimeToString1();
                        //string now_;
                        if( dbhyper->insert_data_to_hypertable_content(urlnode,dbd->datbuf,dbd->datlen_u,string("content_tbl"),string("gbk"),ret,now_) )
                        {
                              char fetchtbl[32];fetchtbl[0]=0;
                             //sprintf(fetchtbl,"fetch_%d_tbl",urlnode->taskid);
                              sprintf(fetchtbl,"fetch_%d_tbl",1);
                             if( dbhyper->insert_data_to_hypertable_fetch(urlnode,string(fetchtbl),ifcong_->spider_id,ret,now_) )
                             {
                                char memorytable[128]; memorytable[0]=0;
                                 //sprintf(memorytable,"url_%d_tbl",urlnode->taskid);
                                 sprintf(memorytable,"url_%d_tbl",1);
                                 dbhyper->insert_data_to_hypertable_memorytable(urlnode,memorytable);
                              }
                         }
                     
                          dbd_free(dbd);
                     }*/
                    //0128.end()
                    /*char * final_content;
                    final_content=NULL;
                    final_content=get_final_content(urlnode);
                    if(final_content !=NULL)
                    {
                        insert_data_to_hypertable(urlnode->fatherurl,final_content);
                        delete []final_content;
                    }*/
                    mylog_info(m_pLogGlobalCtrl->infolog, "fetched %s save fetched%d nowpage %d totalpage %d - %s:%s:%d",url.getUrl().c_str(),urlnode->errornum,urlnode->nowpage,urlnode->totalpage,INFO_LOG_SUFFIX);
                }else
                {
                    mylog_info(m_pLogGlobalCtrl->infolog, "fetched %s have next, so don't not save fetched nowpage %d totalpage %d  - %s:%s:%d",url.getUrl().c_str(),urlnode->nowpage,urlnode->totalpage,INFO_LOG_SUFFIX);
                }
            }
            //insert next page
            if (nextpageurl[0]) {
                UrlNode *newnode  = new UrlNode;
                if (!(urlnode->type & URL_TYPE_HOMEPAGE))
                    newnode->id = urlnode->id;
                newnode->task = urlnode->task;
                newnode->taskid = urlnode->taskid;
                newnode->copyother(urlnode->other,urlnode->maxtype);
                newnode->type = urlnode->type;
                newnode->page = urlnode->page + 1;
                newnode->copyurl(nextpageurl);
                newnode->copyfatherurl(urlnode->fatherurl);
                newnode->layerid  =  urlnode->layerid;
                newnode->needtologin = urlnode->needtologin;
                newnode->taskbatch= urlnode->taskbatch;
                newnode->copytitle(urlnode->title);
                newnode->copytopicsource(urlnode->topicsource);
                mylog_info(m_pLogGlobalCtrl->infolog, " now url %s new url %s title %s - %s:%s:%d",urlnode->url, newnode->url,urlnode->title,INFO_LOG_SUFFIX);
                infocrawler->getUrlAnalyseManager()->insertUrl(newnode, INSERT_URL_FORCED);
            }
        }
        
        free_buffer(content);

        Task *task = urlnode->task;
        int taskid = urlnode->taskid;

        //if get an error, we will retry but only fixed times
        if (urlnode->errornum > 0 && urlnode->errornum < URL_FETCH_RETRY_TIMES) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "fetched %s reinsert for error %d %llu - %s:%s:%d", url.getUrl().c_str(), urlnode->errornum, urlnode->id,INFO_LOG_SUFFIX);
            infocrawler->getUrlAnalyseManager()->insertUrl(urlnode, INSERT_URL_FORCED,false);
        } else if (urlnode->errornum >= URL_FETCH_RETRY_TIMES) {
            infocrawler->getTaskScheduleManager()->increaseTaskErrorUrlNum(taskid);
            //write error url to DB
//            infocrawler->getDbManager()->WriteFetchError(url.getUrl().c_str(),taskid,taskbatch);
            mylog_error(m_pLogGlobalCtrl->errorlog, "fetched finished and download url %s urlnodeid %llu taskid %d - %s:%s:%d:%d", url.getUrl().c_str(),  urlnode->id, taskid,INFO_LOG_SUFFIX,urlnode->errornum);
            infocrawler->getLocalDbManager()->decidesaveFetched(urlnode);

            if (urlnode->type & URL_TYPE_NEEDTOSAVE)
            {
                infocrawler->getLocalDbManager()->saveUrl(urlnode);
            }
            delete urlnode;
        } else {
            mylog_info(m_pLogGlobalCtrl->infolog, "fetched finished %s error %d %llu fatherurl %s - %s:%s:%d",url.getUrl().c_str(), urlnode->errornum, urlnode->id, urlnode->fatherurl,INFO_LOG_SUFFIX);
            delete urlnode;
        }

        infocrawler->getTaskScheduleManager()->decreaseTaskUrlNum(task, taskbatch); 

#ifdef URLMEMCACHEDB
        infocrawler->deleteUrlMcLocalThread(); 
#endif
    }
    curl_easy_cleanup(curl);
    mylog_info(m_pLogGlobalCtrl->infolog, "FetcherManager ISRUNNING false - %s:%s:%d",INFO_LOG_SUFFIX);
}

int FetcherManager::doLogin(CURL *curl, Task *task, UrlNode *urlnode) {
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    TaskOtherInfo *taskother = infocrawler->getTaskScheduleManager()->getTaskOtherInfo(task->id);
    if (!taskother){
        return -1;
    }
    if (!task){
        return -1;
    }
    CUrl url;
    url.parse(task->loginurl);
    
    if (url.getUrl().empty()) {
        return -1;
    }

    HttpProtocol httpprotocol;
    char downstatistic[512] ;downstatistic[0] = 0; 
    RESPONSE_HEADER rheader;

    mylog_info(m_pLogGlobalCtrl->infolog, "before login %s - %s:%s:%d",url.getUrl().c_str(),INFO_LOG_SUFFIX);
    int ret = httpprotocol.curl_login(curl, url, urlnode, infocrawler->getConf()->httptimeout, &rheader, downstatistic);
    mylog_info(m_pLogGlobalCtrl->infolog, "after login  %s %s %d - %s:%s:%d",url.getUrl().c_str(), downstatistic, ret,INFO_LOG_SUFFIX);
   /* if (ret == HTTP_FETCH_RET_REDIRECT) { //redirect
        errorlog("LOGIN ERROR: fetched %s  relocated to %s taskid %d\n", url.getUrl().c_str() ,(char *)page.m_sLocation.c_str(),task->id);
    } else*/ 
    if (ret == HTTP_FETCH_RET_ERROR) {//just discard
        mylog_error(m_pLogGlobalCtrl->errorlog, "login fetched %s taskid %d - %s:%s:%d:%d", url.getUrl().c_str(), task->id,INFO_LOG_SUFFIX,ret);
    } else if (ret == HTTP_FETCH_RET_ERROR_INVALIDHOST) { //invalid host, can not access
        mylog_error(m_pLogGlobalCtrl->errorlog, "login fetched %s taskid %d - %s:%s:%d:%d", url.getUrl().c_str(), task->id,INFO_LOG_SUFFIX,ret);
    } else if (ret == HTTP_FETCH_RET_ERROR_UNACCEPTED) { //content is invalid, discard
        mylog_error(m_pLogGlobalCtrl->errorlog, "LOGIN fetched %s unaccepted contenttyped %s taskid %d - %s:%s:%d:%d", url.getUrl().c_str(), rheader.contenttype.c_str(), task->id,INFO_LOG_SUFFIX,ret);
    }else
    {
        taskother->fetchingcookie = true;
        static char *loginok = "LOGIN OK";
        saveCookie(task->id, loginok, strlen(loginok));
        taskother->fetchingcookie = false;
        return 1;
    }
    return -1;
}

void FetcherManager::saveCookie(int taskid,char * cookie,int cookielen){
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    TaskOtherInfo *taskother = infocrawler->getTaskScheduleManager()->getTaskOtherInfo(taskid);
    if (taskother)
    {
        taskother->insertcookie(cookie,cookielen);
        mylog_info(m_pLogGlobalCtrl->infolog, "save cookie %s - %s:%s:%d",cookie,INFO_LOG_SUFFIX);
    }else
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "save cookie fail taskother is NULL id = %d - %s:%s:%d", taskid,INFO_LOG_SUFFIX);
    }
    return; 
}
void FetcherManager::dump()  {}
void FetcherManager::retrieve() {}


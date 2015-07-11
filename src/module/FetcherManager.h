#ifndef _MODULE_FETCHERMANAGER_H_
#define _MODULE_FETCHERMANAGER_H_
#include <pthread.h>
#include <curl/curl.h>
#include "Manager.h"
#include "util.h"
#include "ic_types.h"

#define URL_FETCH_RETRY_TIMES 3
#define URL_FETCH_REDIRECT_TIMES 4 
class FetcherManager:virtual public Manager {
public:
    FetcherManager(LogGlobalCtrl * pLogGlobalCtrl); 
    ~FetcherManager(); 
    virtual int start();
    virtual int stop();
    int fetch(); 
    void dump(); 
    void retrieve(); 
private:
    static pthread_mutex_t mutex_deleteother;
    static pthread_rwlock_t curl_dns_rwlock; 
    static pthread_rwlock_t curl_cookie_rwlock; 

    CURLSH *sh;
    
    int startCrawlerThread(); 
    static void *startCrawler(void *); 
    static void *fetchThread(void *); 
    int doLogin(CURL *curl, Task * task, UrlNode *urlnode);
    void saveCookie(int taskid,char * cookie,int cookielen);
    LogGlobalCtrl       *m_pLogGlobalCtrl;
};
#endif

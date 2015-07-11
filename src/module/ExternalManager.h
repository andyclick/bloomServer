#ifndef _MODULE_EXTERNALMANAGER_H_
#define _MODULE_EXTERNALMANAGER_H_

#include "Manager.h"
#include "ic_types.h"
#include "util.h"
#include "DBAccess.h"
#include "MyConnection.h"
#include <string>
#include <vector>
using namespace std;

#define EXTERNAL_MAXNCLI 200  //max length of client queue
#define EXTERNAL_DEFAULT_TIMEOUT 10  //max length of client queue
#define REQUEST_COMMAND_SEND_HTML 1
#define REQUEST_COMMAND_RECEIVE_URL 2
#define REQUEST_COMMAND_TEMPLATE   3  
#define REQUEST_COMMAND_GET_HTML   101  

#define SITE_TEMPLATE_ITEM_BBS_FINALPAGEFORMAT  "bbs.url"

#define RECEIVE_URL 0
#define RECEIVE_URL_TYPE    1
#define RECEIVE_URL_BBS_ID  2
#define RECEIVE_URL_TASK_ID 3

typedef struct __html_des__ {
    string url;
    string charset;
    int length;
    string htmlcontent;
    long task_id;
    string sitename;
    __html_des__() {
        length = 0;
        task_id = 0;
    }
} html_des;

typedef struct _DBSENDERROR {
    char key[32];
#ifndef HTMLMEMCACHEDB
    DBD * dbd;
#endif
    _DBSENDERROR()
    {
        key[0] = 0;
#ifndef HTMLMEMCACHEDB
        dbd = NULL;
#endif
    };
    
    ~_DBSENDERROR()
    {
#ifndef HTMLMEMCACHEDB
    if (this->dbd) dbd_free(this->dbd);
#endif
    };
}dbsenderror;
class ExternalManager: virtual public Manager {
public:
    ExternalManager(LogGlobalCtrl * pLogGlobalCtrl);
    virtual ~ExternalManager();
    virtual int start();
    virtual int stop();

    void dump();
    void retrieve();
    int ReadFinalPageFormat(int taskid, const char * url, char *&recvbuf);
#ifdef HTMLMEMCACHEDB
    int SendFile_MemcachedbDb(MyConnection connection);
#else
    int SendFile_db(MyConnection connection);
#endif

    int get_html(MyConnection &connetion);
    void get_real_html(char *buf, vector<html_des> &htmls); 


    void insert_html_from_outer(html_des &html) {

        pthread_rwlock_wrlock(&rwlock_html_from_outer);
        html_from_outer_.push_back(html);
        pthread_rwlock_unlock(&rwlock_html_from_outer);

    }

    bool pop_html_from_outer(html_des &html) {
        bool ret = false;
        pthread_rwlock_wrlock(&rwlock_html_from_outer);
        if (html_from_outer_.begin() != html_from_outer_.end()) {
            html = html_from_outer_.front();
            html_from_outer_.pop_front();
            ret = true;
        }
        pthread_rwlock_unlock(&rwlock_html_from_outer);
        return ret;
    }

    int get_html_from_outer_size() {
        int size = 0;
        pthread_rwlock_rdlock(&rwlock_html_from_outer);
        size = html_from_outer_.size();
        pthread_rwlock_unlock(&rwlock_html_from_outer);
        return size;
    }


private:

    static int iget;
    static int iput;

    static Thread *tptr;

    static int clifd[EXTERNAL_MAXNCLI];

    list<html_des> html_from_outer_;
    static pthread_rwlock_t rwlock_html_from_outer;

    list<dbsenderror *> senderror;

    static pthread_rwlock_t rwlock_socket_error;
    int thread_i;

    void thread_make(int i); 
    void thread_detach(int i); 
    void thread_join(int i);
    void thread_exit(int i); 
    void serve(int portnumber); 

    static void *thread_main(void *arg); 

    static pthread_mutex_t clifd_mutex;
    static pthread_cond_t clifd_cond;


    static int nthreads;

    int startServeThread(); 
    static void *startServe(void *manager); 
    int serveClient(int fd); 
    int ReceiveClineUrl(MyConnection connection);        
    int SendFile(MyConnection connection);
    void WriteDBD(MyConnection connection, DBD * dbd);
    dbsenderror* GetSendError();
    void SaveSendError(dbsenderror *dbd);
    LogGlobalCtrl       *m_pLogGlobalCtrl;
};

#endif

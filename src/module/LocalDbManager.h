#ifndef _MODULE_LOCALDBMANAGER_H_
#define _MODULE_LOCALDBMANAGER_H_

#include "Manager.h"
#include "util.h"
#include "Page.h"
#include "ic_types.h"
#include "DBAccess.h"

#ifdef URLMEMCACHEDB
#include "MemCacheClient.h"
#endif

#ifdef HTMLMEMCACHEDB
#include "MemCacheClient.h"
#endif

#define FETCHED_FIFO_QUEUE_FILE "fetched_fifoqueue_file"
#define FETCHED_FIFO_QUEUE_POS_FILE "fetched_fifoqueue_pos_file"
#define READ_FIFO_QUEUE_FILE "read_fifoqueue_file"
#define DELETE_READ_FIFO_QUEUE_POS_FILE "delete_read_fifoqueue_pos_file"
#define DELETE_READ_FIFO_QUEUE_FLAG_FILE "delete_read_fifoqueue_flag_file"
#define IDX_TEMPLATE "idx_template"
#define TASKDB_NUM 30
typedef struct _BasicHead {
    virtual ~_BasicHead() {}; 
} BasicHead;


typedef struct _ContentPageHead {
    ulonglong id;
    off_t offset;
    size_t length;
    int page;
    _ContentPageHead() {
        id = 0;
        offset = 0;
        length = 0;
        page = 1;
    }
} ContentPageHead; 

typedef struct _ContentHead: public BasicHead {
    size_t totallength;
    size_t headlength;
    int totalpage;
    int nowpage;
    int taskid;
    int batchid;
    ContentPageHead *contentPageHead;
    _ContentHead() {
        totallength = 0;
        headlength = 0;
        totalpage = 0;
        nowpage = 0;
        contentPageHead = NULL;
        taskid = -1;
        batchid = -1;
    }
    virtual ~_ContentHead() { }
} ContentHead;

class LocalDbManager:virtual public Manager {
public:
    LocalDbManager(LogGlobalCtrl * pLogGlobalCtrl);
    ~LocalDbManager();
    virtual int start();
    virtual int stop();

    int savecontent(UrlNode *urlnode, RESPONSE_HEADER *rheader, char *content, int contentlength, int nextpage); 
    bool alreadyfetched(UrlNode *urlnode, char * urlcontent = NULL);
    void dump(); 
    void retrieve(); 
    void saveFetched(UrlNode *urlnode); 
    void decidesaveFetched(UrlNode *urlnode);
    bool readFetched(list<char *> &fetched, int num); 
    DBD *readContent(ulonglong id, int taskid); 
    DBD *readCookie(int taskid); 
    int savecookiecontent(char *content, int contentlength , int taskid);
    static void readHead(char *buf, BasicHead *head); 
    void saveSend(char * buff) ;
    int saveUrl(UrlNode *urlnode, int savefatherurlflag = SAVE_URL);
    int erasecontent(ulonglong id ,int taskid); 
    int erasecontent_db(ulonglong id ,int taskid); 
    void saveAlreadyRead(ulonglong id, int taskid );
    void GetAlreadyRead(list<char *> &alreadyread);
    bool ReadDeleteAlreadyRead();
    void altertotalpageforced(UrlNode *urlnode);
    bool IsReadFetchedEnd();
    bool deleteAllData();
#ifdef HTMLMEMCACHEDB 
    int mc_store(char *key, const char *data, size_t datalen);
#endif
    void getContentDBName(UrlNode *urlnode, char *dbname); 
    void getRecordKeyName(UrlNode *urlnode, char *recordkeyname);
    static int writePageHead(char **buf, UrlNode *urlnode, RESPONSE_HEADER *rheader, int contentlength); 
    void getContentDBName(int taskid, char *dbname); 
    void getRecordKeyName(ulonglong id, char *recordname); 
private:
    void getMemcachedbKeyName(ulonglong id, int taskid,char *  memcachedbkey);
    void getUrlDBName(UrlNode *urlnode, char *dbname); 
    int getTaskDB(int taskid); 
    void getCookieKeyName(int taskid, char *cookiename); 
    void getCookieDBName( char *dbname);

    static int writeHead(char **buf, BasicHead *head); 
    static void freeHead(BasicHead *head); 
    static int appendPageHead(BasicHead *head); 
    static size_t getUrlContent(char *buf, UrlNode *urlnode); 
    static pthread_rwlock_t rwlock_fifoqueu_file;
    static pthread_rwlock_t rwlock_read_fifoqueu_file;
    static pthread_rwlock_t rwlock_socket_file;
private:
    LogGlobalCtrl       *m_pLogGlobalCtrl;
};
#endif

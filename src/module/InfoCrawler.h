#ifndef _MODULE_INFOCRAWLER_H_
#define _MODULE_INFOCRAWLER_H_


#include "DbManager.h"
#include "TaskScheduleManager.h"
#include "CheckpointManager.h"
#include "LogStatisticManager.h"
#include "ExternalManager.h"
#include "FetcherManager.h"
#include "UrlAnalyseManager.h"
#include "LocalDbManager.h"
#include "PageManager.h"
/* ---------- added by andy 2014/11/02 ------------------------- */
#include "ThriftManager.h"
/* ------------------------------------------------------------- */
#include "util.h"
#include "ic_types.h"
//#include "client_test.h"
//#include "DbHypertableManager.h"
#include "DbMySQLManager.h"

#ifdef URLMEMCACHEDB
#include "MemCacheClient.h"
#endif

#ifdef HTMLMEMCACHEDB
#include "MemCacheClient.h"
#endif
#define START_TYPE_DEFAULT       0
#define START_TYPE_ONLY_EXTERNAL    1

#define DELETE_DATA_FLAG_FILE "delete_data_flag_file"






class InfoCrawler {
public:
    InfoCrawler(int startype = START_TYPE_DEFAULT);
    ~InfoCrawler();
    static InfoCrawler * const getInstance(int starttype = START_TYPE_DEFAULT);
    void destroy(); 
    void startServer(); 
    void stopServer(); 
    ICCONFIG *getConf() { return this->_icconfig; }
    LogGlobalCtrl *getLogGlobalCtrl() { return this->_logGlobalCtrl; }
    pid_t   getMasterpid() { return master_pid; }
    
    CheckpointManager   *getCheckpointManager() { return _checkpointManager; }
    DbManager           *getDbManager() { return _dbManager; }
    UrlAnalyseManager   *getUrlAnalyseManager() { return _urlAnalyseManager; }
    LocalDbManager      *getLocalDbManager() { return _localDbManager; }
    PageManager         *getPageManager () { return _PageManager; }
    FetcherManager      *getFetcherManager() { return _fetcherManager; }
    LogStatisticManager *getLogStatisticManager() { return _logStatisticManager; }
    TaskScheduleManager *getTaskScheduleManager() { return _taskScheduleManager; }
    ExternalManager *getExternalManager() { return _externalManager; }
    ThriftManager* getThriftManager(){return _ThriftManager;}

//    DbHypertableManager *gethyper() { return _dbHypertableManager; }
    DbMySQLManager   *getdbmysql() { return _dbMySQLManager; }
    
    bool ReadDeleteFlag();
    void deletedata();
    static void *DeleteDataThread(void *manager );
    int startDeleteDataThread();
    void dump();
    void retrieve();
    
    LogGlobalCtrl       *_logGlobalCtrl;
#ifdef URLMEMCACHEDB
    MemCacheClient * getUrlMcLocalThread();
    void deleteUrlMcLocalThread(); 
#endif
#ifdef HTMLMEMCACHEDB
    MemCacheClient * getHtmlMcLocalThread();
#endif
    void deletehyer_client();
private:
    CheckpointManager   *_checkpointManager; 
    DbManager           *_dbManager; 
    UrlAnalyseManager   *_urlAnalyseManager; 
    LocalDbManager      *_localDbManager; 
    PageManager         *_PageManager; 
    /* ----------- added by andy 2014/11/02 -------------- */
    ThriftManager *_ThriftManager;
    /* --------------------------------------------------- */
    FetcherManager      *_fetcherManager; 
    LogStatisticManager *_logStatisticManager; 
    TaskScheduleManager *_taskScheduleManager; 
    ExternalManager     *_externalManager; 

//    DbHypertableManager * _dbHypertableManager;
    DbMySQLManager      * _dbMySQLManager;
    DbConnectionMySQLManager * db_connection_manager_;
    static InfoCrawler  *_instance;
    ICCONFIG            *_icconfig;
    pid_t               master_pid;


    static pthread_mutex_t server_mutex;
    static pthread_cond_t server_cond;

    static pthread_mutex_t delete_data_wait_mutex;
    static pthread_cond_t delete_data_wait_cond;
    
    void initialize(); 
    void beforestop();

    int starttype;
    bool serverstop;

    static pthread_rwlock_t hyper_clientsrwlock;

#ifdef HTMLMEMCACHEDB
    map<pthread_t, MemCacheClient *> contentmclocals;

    static pthread_rwlock_t htmlthreadlocalrwlock; 
#endif

#ifdef URLMEMCACHEDB
    map<pthread_t, MemCacheClient *> urlmclocals;

    static pthread_rwlock_t urlthreadlocalrwlock; 
#endif
};

#endif

#ifndef _MODULE_DBMANAGER_H_
#define _MODULE_DBMANAGER_H_

#include "Manager.h"
#include "ic_types.h"

#include   "stdlib.h"
//#include   "ocidfn.h"
//#include   "ocidem.h"
//#include   "oratypes.h"
//#include   "ocidem.h"
//#include   "ociapr.h"
//#include   "ocikpr.h"
//#include   "oci.h"
#include   <string.h>
#include   <sqlite3.h>

#define SELECT_MOD  1
#define NON_SELECT_MOD  2 
#define NEED_LOGON      1 


#define NOT_FETCHED_URL_TABLE_NAME  "not_fetched_url_table"
#define HOST_TABLE_NAME  "host_table"
#define SQLITE_DB_FILE_NAME  "spider_sqlite_db"

class DbManager: virtual public Manager {
public:
    DbManager(LogGlobalCtrl * pLogGlobalCtrl);
    ~DbManager();
    DbManager* const getInstance();
    virtual int start();
    virtual int stop();
    void dump();
    void retrieve();

    int WriteDownLoadBeginTime(int taskid,int batch);
    int WriteDownLoadEndTime(int saveurlnum, int linknum,int fetchfirstpagenum,int taskid,int batch);
    int WriteFetchError(const char * url, int taskid,int batch); 
    int  GetTaskBatch();
    string checkPattern(char *pattern,char * fromurl);
    string  checkFinalPageFormat(string url);
    bool turntoregular(char * ultimatepageurl);
    void TransferRegEx(char * sOrigi, char * sNew);
    int UpdateFetchTimeToSourceTable(int taskid , time_t nextfetchtime);


	 static int check_if_url_exists_callback(void *NotUsed, int argc, char **argv, char **azColName){
        int i;
        for(i=0; i<argc; i++){
            printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        }
        printf("\n");
        return 0;
    }

	  bool check_if_url_exists_in_table(char *url) {
        char *zErrMsg = NULL;
        char sqlbuf[512] = {0};
        sprintf(sqlbuf, "select count(*) a from not_fetched_url_table where url = '%s'", url);
        int rc = sqlite3_exec(unfetched_url_db, sqlbuf, check_if_url_exists_callback, 0, &zErrMsg);
        if(rc != SQLITE_OK){
            mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "SQL error: %s - %s:%s:%d", zErrMsg, INFO_LOG_SUFFIX);
            sqlite3_free(zErrMsg);
        }
    }

	bool insert_into_table(char *url);
    bool read_url_from_table();
    int create_sqlite_table();
    int open_sqlite_table(); 
    int close_sqlite_table(); 
    int delete_unfetched_url_by_id(int); 
    int insert_unfetched_url(char *url, void *data, int len, int hostid, bool pushback); 
    int get_unfetched_urls(list<UrlNode *> &urls, int hostid, int url_to_read); 
    int get_hostid_by_domain(char *domain,int &hostid); 
    int insert_host(char *domain);
private:
    static DbManager *_instance;
    bool InitDb();
    static void *startGetConfigThread(void *);
    static void * startGetConfig(void *manager);
    //sb4 error_proc(dvoid *errhp , sword status, int logonflag = -1);
    int ReadInfogetSource();
    int ReadKeyWord();
    //bool reconnect(sb4 errorcode);
    //selectType WriteLogsToDB(string sql,int flag = -1);
    void CheckUrl(char *srcurl, char *fromurl, char *outurl);
    void setlogon(bool logonflag);
    //sword OCI_StmtExecute(OCIStmt * stmhp,string sql , ub4 executemod = OCI_DEFAULT, int executeflag = SELECT_MOD); 
private:
    static pthread_mutex_t getinfo_wait_mutex;
    static pthread_cond_t getinfo_wait_cond;
    static pthread_rwlock_t rwlock_reconnect;
    static pthread_rwlock_t rwlock_logonflag;
    int readdbnum;
    int keywordreaddbnum;
    bool needlogon;
    int test;


    LogGlobalCtrl       *m_pLogGlobalCtrl;

	sqlite3 *unfetched_url_db;
};


#endif

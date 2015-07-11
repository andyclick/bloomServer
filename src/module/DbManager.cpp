#include "InfoCrawler.h"
#include <stdio.h> 
#include <unistd.h> 
#include <pthread.h> 
#include <time.h> 
#include "DbManager.h"
#include "readconf.h"
#include "Url.h"
#include "utilcpp.h"

static char * sqlselect = "select id, source,site,finalpageformat,interval,listpageformat,depth,charset,to_char(begintime,'yyyymmddhh24miss'),shieldedformat,sourcetype_id , state, PRIORITY, SENDTYPE , TARGET ,INTERVALAUTO from is_sitesource where mark = 6  and site is not null ";

static char * sqlselect1 = "select id, source,site,finalpageformat,interval,listpageformat,depth,charset,to_char(begintime,'yyyymmddhh24miss'),shieldedformat,sourcetype_id , state, PRIORITY, SENDTYPE , TARGET ,INTERVALAUTO from is_sitesource where state = 6 and site is not null ";

static char * sqlkeyword = "select source_id ,keyword, state ,b.id ,a.id from keyword_tbl a,keyword_source_tbl b where mark = 0 and b.keyword_id = a. id ";
static char * sqlkeyword1 = "select source_id ,keyword, state, b.id, a.id from keyword_tbl a ,keyword_source_tbl b where state = 1 and b.keyword_id = a.id";

pthread_mutex_t DbManager::getinfo_wait_mutex= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t DbManager::getinfo_wait_cond= PTHREAD_COND_INITIALIZER;

pthread_rwlock_t DbManager::rwlock_reconnect= PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t DbManager::rwlock_logonflag= PTHREAD_RWLOCK_INITIALIZER;

DbManager::DbManager(LogGlobalCtrl * pLogGlobalCtrl) {
    m_pLogGlobalCtrl = pLogGlobalCtrl;

    readdbnum = 0;
    keywordreaddbnum = 0;
    needlogon = true;
    test = 1;
}

static int test_is_exist_callback(void *count, int argc, char **argv, char **azColName){
    *(int *)count = atoi(argv[0]);
    return 0;
}

DbManager::~DbManager() {
}

int DbManager::start() {
    Manager::start();
    open_sqlite_table();
    return 0;
}

int DbManager::stop() {
    int ret = Manager::stop();
    while(!canstoped()) {
        pthread_cond_signal(&getinfo_wait_cond);
        my_sleep(500000); //0.5 seconds
    }
    close_sqlite_table();
    return ret;
}

void DbManager::setlogon(bool logonflag)
{
    pthread_rwlock_wrlock(&rwlock_logonflag);
    needlogon = logonflag;
    pthread_rwlock_unlock(&rwlock_logonflag);
}

void * DbManager::startGetConfig(void *manager)
{
    DbManager *m =  static_cast<DbManager*>(manager);
    pthread_t *tid = (pthread_t*)malloc(1 * sizeof(pthread_t));
    if (tid == NULL) {
        mylog_fatal(m->m_pLogGlobalCtrl->veryimportantlog, "malloc error - %s:%s:%d", INFO_LOG_SUFFIX);
    }

    if(pthread_create(tid, NULL, startGetConfigThread, manager)) {
        mylog_fatal(m->m_pLogGlobalCtrl->veryimportantlog, "create threads error - %s:%s:%d", INFO_LOG_SUFFIX);
    }
    pthread_join(*tid, NULL);

    free(tid);
    m->stoped();
    mylog_info(m->m_pLogGlobalCtrl->infolog, "DbManager startGetConfig thread is over - %s:%s:%d", INFO_LOG_SUFFIX);
}
void * DbManager::startGetConfigThread(void *manager)
{
  
}

int DbManager::ReadInfogetSource()
{
  
}

int DbManager::UpdateFetchTimeToSourceTable(int taskid , time_t nextfetchtime)
{
    char taskidstr[10] ={0};
    sprintf(taskidstr,"%d",taskid);
    char strfetchtime[8] ;strfetchtime[0] = 0;
    sprintf(strfetchtime, "%d", nextfetchtime);
    string  sqlupdate = "update is_sitesource set begintime = sysdate +";
    sqlupdate.append(strfetchtime);
    sqlupdate.append("/(24*3600)  where id  = ");
    sqlupdate.append(taskidstr);
   // WriteLogsToDB(sqlupdate);
}
int DbManager::WriteDownLoadBeginTime(int taskid,int batch)
{
    char taskidstr[10] ={0};
    sprintf(taskidstr,"%d",taskid);
    char taskbatch[32] ={0};
    sprintf(taskbatch,"%d",batch);

    string  sqlinsert ="insert into downloadlogs (id ,source_id, batch,begintime,PREDOWNLOADTIME, spiderid) values (SEQ_DOWNLOADLOGS.nextval ,";
    sqlinsert.append(taskidstr);sqlinsert.append(",");
    sqlinsert.append(taskbatch);sqlinsert.append(",");
    sqlinsert.append("sysdate, sysdate, ");
    char tmp[16] ;tmp[0] = 0;
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    sprintf(tmp, "%d", icconfig->spiderid);
    sqlinsert.append(tmp);
    sqlinsert.append(")");
    //WriteLogsToDB(sqlinsert);

    string sqlupdate = "update is_sitesource set status = '1' where status != 3 and  id = ";
    sqlupdate.append(taskidstr);
    ____BEGINTIMER
    //WriteLogsToDB(sqlupdate);
    ____ENDTIMER
}
int DbManager::WriteDownLoadEndTime(int saveurlnum, int linknum, int fetchfirstpagenum ,int taskid,int batch)
{
    char taskidstr[10] ={0}; 
    sprintf(taskidstr,"%d",taskid);
    
    char saveurlnumstr[10] ={0}; 
    sprintf(saveurlnumstr,"%d",saveurlnum);
    
    char fetchfirstpagenumstr[10] ={0}; 
    sprintf(fetchfirstpagenumstr,"%d",fetchfirstpagenum);
    
    char linknumstr[10] ={0}; 
    sprintf(linknumstr,"%d",linknum);
    
    char taskbatch[32] ={0}; 
    sprintf(taskbatch,"%d",batch);
    
    string  sql ="update downloadlogs set donetime = sysdate";
    sql.append(", donequantity = ");
    sql.append(saveurlnumstr);
    sql.append(" , uniquequantity = ");
    sql.append(linknumstr);
    sql.append(" , firstpagenum= ");
    sql.append(fetchfirstpagenumstr);
    sql.append(" where source_id = ");
    sql.append(taskidstr);
    sql.append(" and batch  =  ");
    sql.append(taskbatch);
     
    ____BEGINTIMER
    //WriteLogsToDB(sql);
    ____ENDTIMER

    string sqlupdate = "update is_sitesource set status = '2' where status != 3 and  id = ";
    sqlupdate.append(taskidstr);
    //WriteLogsToDB(sqlupdate);
}
int  DbManager::GetTaskBatch()
{
	int ret=0;
   /* string sql = "select task_seq.nextval  from dual";
    
    ____BEGINTIMER
    //selectType type = WriteLogsToDB(sql, SELECT_MOD);
    ____ENDTIMER
    
        int ret = type.intnum; 
    mylog_info(m_pLogGlobalCtrl->infolog, "Task Return value is %d - %s:%s:%d",ret, INFO_LOG_SUFFIX);*/
    return ret;
}
int DbManager::WriteFetchError(const char * url, int taskid, int batch) 
{
    return 0;
}
/*selectType DbManager::WriteLogsToDB(string sql,int flag)
{
      return 0; 
    //OCITransCommit(svchp,errhp,OCI_DEFAULT);
}*/
/*sword DbManager::OCI_StmtExecute(OCIStmt * stmhp,string sql, ub4 executemod, int executeflag) 
{
  
    return status;
}*/
/*sb4 DbManager::error_proc(dvoid   *errhp,   sword   status, int logonflag)
{
 
    return errcode;
}*/

void DbManager::dump() {
}

void DbManager::retrieve() {
}

/*bool DbManager::reconnect(sb4 errorcode)
{
   
    return false;
}*/

bool DbManager::turntoregular(char * ultimatepageurl)
{
    szReplace(ultimatepageurl,".","\\.");
    szReplace(ultimatepageurl,"[?]","[^ ]{1}");
    szReplace(ultimatepageurl,"?","\\?");
    szReplace(ultimatepageurl,"[$]","[0-9]+");
    szReplace(ultimatepageurl,"[*]","[^ ]+");
    return true;
}
string  DbManager::checkPattern(char *pattern,char * fromurl) {
    string ret = "";
    char *pToken = NULL;
    char *tokbuff = NULL;
    
    char *pTokenfromurl = NULL;
    char *tokbufffromurl = NULL;
    
    int len = strlen(pattern);
    int lenfromurl = strlen(fromurl);
    if (len <=0 )
    {
        return ret;
    }
   
    char *fromurltmp = new char[lenfromurl + 10];
    fromurltmp[0] = 0;
    strncpy(fromurltmp, fromurl,lenfromurl);
    fromurltmp[lenfromurl] = 0;
    pTokenfromurl = strtok_r(fromurltmp, "\r\n\t ",&tokbufffromurl);


    char *patterntmp = new char[len + 10];
    patterntmp[0] = 0;
    strncpy(patterntmp , pattern,len);
    patterntmp[len] = 0;
    pToken = strtok_r(patterntmp, "\r\n\t ",&tokbuff);
    while(pToken)
    {
        if (strlen(pToken)>=MAX_URL_LEN)
        {
            pToken = strtok_r(NULL, "\r\n\t ",&tokbuff);
            continue;
        }
        char pTokentmp[MAX_URL_LEN] ;pTokentmp[0] = 0;
        char tmpurl[MAX_URL_LEN] ;tmpurl[0] = 0;
        char outurl[MAX_URL_LEN] ;outurl[0] = 0;
        
        TransferRegEx(pToken,pTokentmp);
        
        //checkurl
        CheckUrl(pTokentmp, fromurltmp, outurl);
        //begin end flag
        MyURLencode(outurl, tmpurl, MAX_URL_LEN- 1);
        pTokentmp[0] = '^';
        strcpy(pTokentmp + 1, tmpurl);
        strcat(pTokentmp, "$");
        
        tmpurl[0] = 0;
        
        ret.append(pTokentmp);
        ret.append("\r\n");
        pToken = strtok_r(NULL, "\r\n\t ",&tokbuff);
    }
    if (patterntmp) delete [] patterntmp;
    if (fromurltmp) delete [] fromurltmp;
    return ret;
}
void DbManager::CheckUrl(char *srcurl, char *fromurl, char *outurl) {
    outurl[0] = 0;
    if ((!srcurl[0]) || (!fromurl[0]) || (strlen(srcurl)>=MAX_URL_LEN) || (strlen(fromurl) >= MAX_URL_LEN)) return;
    CUrl url;
    url.parse(srcurl);

    CUrl basefromurl;
    basefromurl.parse(fromurl);
    int len = strlen(srcurl);

    if (!url.m_sProtocol.empty() || !url.m_sHost.empty()) {
        if (len  > MAX_URL_LEN -1)
        {
            return ;
        }
        strcpy(outurl, srcurl);
    } else {
        if (srcurl[0] == '/') {
            if (( basefromurl.m_sProtocol.length()+basefromurl.m_sHost.length()) > MAX_URL_LEN - 50 )
            {
                return;
            }
            sprintf(outurl, "%s://%s:%d%s", basefromurl.m_sProtocol.c_str(), basefromurl.m_sHost.c_str(), basefromurl.m_sPort, srcurl);
        }else if (srcurl[0] == '?')
        {
            char baseurl[MAX_URL_LEN] ;baseurl[0] = 0;
            char *tmp = strrchr(fromurl, '?');
            if (tmp) {
                strncpy(baseurl, fromurl, tmp - fromurl);
                baseurl[tmp - fromurl] = 0;
                if (strlen(srcurl) + strlen(baseurl) > MAX_URL_LEN - 1) {
                }else
                {
                    sprintf(outurl, "%s%s", baseurl, srcurl);
                }
            }else
            {
                if (strlen(srcurl) + strlen(fromurl) > MAX_URL_LEN - 1) {
                }else
                {
                    sprintf(outurl, "%s%s", fromurl, srcurl);
                }
            }
        }else {
            char baseurl[MAX_URL_LEN] ;baseurl[0] = 0;
            char *tmp = strrchr(fromurl, '/');
            if (tmp) {
                strncpy(baseurl, fromurl, tmp - fromurl + 1);
                baseurl[tmp - fromurl + 1] = 0;
                if (strlen(srcurl) + strlen(baseurl) > MAX_URL_LEN - 1) {
                }else
                {
                    sprintf(outurl, "%s%s", baseurl, srcurl);
                }
            }
        }
        CUrl ret;
        ret.parse(outurl);
        if (ret.getUrl().length() > MAX_URL_LEN - 1) {
            strncpy(outurl, (char *)ret.getUrl().c_str(), MAX_URL_LEN - 1);
            outurl[MAX_URL_LEN - 1] = 0;
        } else {
            strcpy(outurl, (char *)ret.getUrl().c_str());
        }
    }
}
string  DbManager::checkFinalPageFormat(string url)
{
    unsigned int begin = 0,end = 0;
    string urltmp = "";
    string urltmp1 = url;
    int len = url.length();
    int i = 0;
    while((begin = urltmp1.find("<")) != string ::npos)
    {
        if ((end = urltmp1.find(">")) != string ::npos)
        {
            urltmp = urltmp1.substr(0,begin);
            urltmp.append("[^ ]+");
            if (end +1 < len)
            {
                urltmp.append(urltmp1.substr(end +1));
            }else
            { 
                urltmp1 = urltmp;
            }
        }else
        {
            break;
        }
        urltmp1 = urltmp;
        urltmp = "";
    }
    return urltmp1;
}  
void DbManager::TransferRegEx(char * sOrigi, char * sNew)
{
    int nLen = strlen(sOrigi);
    //sNew = (char*) malloc(nLen * 10);
    int i = 0, j = 0;
    while ( i < nLen )
    {
        if (j >= MAX_URL_LEN - 10)
        {
            sNew[j] = 0;
            break;
        }
        if ( sOrigi[i] == '.' 
            || sOrigi[i] == '*'
            || sOrigi[i] == '?'
            || sOrigi[i] == '|'
            || sOrigi[i] == '\\'
            || sOrigi[i] == '('
            || sOrigi[i] == ')'
            || sOrigi[i] == '{'
            || sOrigi[i] == '}'
            || sOrigi[i] == ']' )
        {
            sNew[j++] = '\\';
            sNew[j++] = sOrigi[i];
            i++;
            continue;
        }

        if ( sOrigi[i] == '[' )
        {
            if ( i + 2 < nLen && strncmp(sOrigi+i, "[*]", strlen("[*]")) == 0 )
            {
                sNew[j++] = '(';
                sNew[j++] = '.';
                sNew[j++] = '*';
                sNew[j++] = ')';
                i += 3;
                continue;
            }
            else if ( i + 2 < nLen && strncmp(sOrigi+i, "[?]", strlen("[?]")) == 0 )
            {
                sNew[j++] = '(';
                sNew[j++] = '.';
                sNew[j++] = '{';
                sNew[j++] = '1';
                sNew[j++] = '}';
                sNew[j++] = ')';
                i += 3;
                continue;
            }
            else if ( i + 2 < nLen && strncmp(sOrigi+i, "[$]", strlen("[$]")) == 0 )
            {
                sNew[j++] = '(';
                sNew[j++] = '[';
                sNew[j++] = '0';
                sNew[j++] = '-';
                sNew[j++] = '9';
                sNew[j++] = ']';
                sNew[j++] = '+';
                sNew[j++] = ')';
                i += 3;
                continue;
            }
            else
            {
                sNew[j++] = '\\';
                sNew[j++] = sOrigi[i];
                i++;
                continue;
            }
        }
        sNew[j++] = sOrigi[i++];
    }
    sNew[j] = 0;
}


int DbManager::open_sqlite_table() {
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    int rc;
    char * pErrMsg = NULL;

    char table[128] = {0};
    sprintf(table, "%s/%s", icconfig->dbpath, SQLITE_DB_FILE_NAME);

    rc = sqlite3_open(table, &unfetched_url_db);
    if(rc){
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "Can't open database: %s - %s:%s:%d", sqlite3_errmsg(unfetched_url_db), INFO_LOG_SUFFIX);
        sqlite3_close(unfetched_url_db);
        return(1);
    }

    char *sql = "PRAGMA cache_size = 2000";
    rc = sqlite3_exec(unfetched_url_db, sql, NULL, 0, &pErrMsg);
    if(rc != SQLITE_OK)
    {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "ÉèÖÃÊ§°Ü:%s - %s:%s:%d", pErrMsg ,INFO_LOG_SUFFIX);
        sqlite3_free(pErrMsg);
    }

    sql = "PRAGMA synchronous = OFF";
    rc = sqlite3_exec(unfetched_url_db, sql, NULL, 0, &pErrMsg);
    if(rc != SQLITE_OK)
    {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "ÉèÖÃÊ§°Ü:%s - %s:%s:%d", pErrMsg ,INFO_LOG_SUFFIX);
        sqlite3_free(pErrMsg);
    }

    sql = "PRAGMA temp_store = MEMORY";
    rc = sqlite3_exec(unfetched_url_db, sql, NULL, 0, &pErrMsg);
    if(rc != SQLITE_OK)
    {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "ÉèÖÃÊ§°Ü:%s - %s:%s:%d", pErrMsg ,INFO_LOG_SUFFIX);
        sqlite3_free(pErrMsg);
    }

    sql = "PRAGMA page_size = 8192";
    rc = sqlite3_exec(unfetched_url_db, sql, NULL, 0, &pErrMsg);
    if(rc != SQLITE_OK)
    {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "ÉèÖÃÊ§°Ü:%s - %s:%s:%d", pErrMsg ,INFO_LOG_SUFFIX);
        sqlite3_free(pErrMsg);
    }

    char sqlbuf[128] = {0};
    int count = 0;
    sprintf(sqlbuf,"SELECT COUNT(*) FROM sqlite_master where type='table' and name='%s'", NOT_FETCHED_URL_TABLE_NAME);
    rc = sqlite3_exec(unfetched_url_db, sqlbuf, test_is_exist_callback, &count, &pErrMsg);
    if(rc != SQLITE_OK){
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "SQL error:%s - %s:%s:%d", pErrMsg ,INFO_LOG_SUFFIX);
        sqlite3_free(pErrMsg);
    }

    if (count == 0) {
        char sqlbuf[256] = {0};
        sprintf(sqlbuf, "CREATE TABLE %s (ID INTEGER PRIMARY KEY, hostid integer, pushback tinyint, url varchar(512), urldata BLOB);", NOT_FETCHED_URL_TABLE_NAME);
        rc = sqlite3_exec(unfetched_url_db, sqlbuf, 0, 0, &pErrMsg);
        if (rc != SQLITE_OK)
        {
            mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "SQL error:%s - %s:%s:%d", pErrMsg ,INFO_LOG_SUFFIX);
            sqlite3_free(pErrMsg);
        }

        sprintf(sqlbuf, "create index url on %s (url);", NOT_FETCHED_URL_TABLE_NAME);
        rc = sqlite3_exec(unfetched_url_db, sql, 0, 0, &pErrMsg);
        if(rc != SQLITE_OK)
        {
            mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "SQL error:%s - %s:%s:%d", pErrMsg ,INFO_LOG_SUFFIX);
            sqlite3_free(pErrMsg);
        
        }


       sprintf(sqlbuf, "create index pushback on %s (pushback);", NOT_FETCHED_URL_TABLE_NAME);
       rc = sqlite3_exec(unfetched_url_db, sql, 0, 0, &pErrMsg);
       if(rc != SQLITE_OK)
       {
         mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "SQL error:%s - %s:%s:%d", pErrMsg ,INFO_LOG_SUFFIX);
         sqlite3_free(pErrMsg);
       }

        sprintf(sqlbuf, "create index hostid_idx on %s (hostid);", NOT_FETCHED_URL_TABLE_NAME);
        rc = sqlite3_exec(unfetched_url_db, sql, 0, 0, &pErrMsg);
        if(rc != SQLITE_OK)
        {
            mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "SQL error:%s - %s:%s:%d", pErrMsg ,INFO_LOG_SUFFIX);
            sqlite3_free(pErrMsg);
        }
    }


    sprintf(sqlbuf,"SELECT COUNT(*) FROM sqlite_master where type='table' and name='%s'", HOST_TABLE_NAME);
    rc = sqlite3_exec(unfetched_url_db, sqlbuf, test_is_exist_callback, &count, &pErrMsg);
    if(rc != SQLITE_OK){
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "SQL error:%s - %s:%s:%d", pErrMsg ,INFO_LOG_SUFFIX);
        sqlite3_free(pErrMsg);
    }

    if (count == 0) {
        char sqlbuf[256] = {0};
        sprintf(sqlbuf, "CREATE TABLE %s (ID INTEGER PRIMARY KEY, domain varchar(512));", HOST_TABLE_NAME);
        rc = sqlite3_exec(unfetched_url_db, sqlbuf, 0, 0, &pErrMsg);
        if (rc != SQLITE_OK)
        {
            mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "SQL error:%s - %s:%s:%d", pErrMsg ,INFO_LOG_SUFFIX);
            sqlite3_free(pErrMsg);
        }

        sprintf(sqlbuf, "create index domain on %s (domain);", HOST_TABLE_NAME);
        rc = sqlite3_exec(unfetched_url_db, sql, 0, 0, &pErrMsg);
        if(rc != SQLITE_OK)
        {
            mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "SQL error:%s - %s:%s:%d", pErrMsg ,INFO_LOG_SUFFIX);
            sqlite3_free(pErrMsg);
        }
    }

    return 0;
}

int DbManager::close_sqlite_table() {
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    sqlite3_close(unfetched_url_db);
    return 0;
}

int DbManager::delete_unfetched_url_by_id(int sqlite_id) {
    if (sqlite_id == 0) return 0;
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    int rc;
    sqlite3_stmt *plineInfo = 0;
    char * pErrMsg = NULL;

    char sqlbuf[1024] = {0};
    sprintf(sqlbuf, "delete from %s where id = %d", NOT_FETCHED_URL_TABLE_NAME, sqlite_id);
    rc = sqlite3_exec(unfetched_url_db, sqlbuf, NULL, 0, &pErrMsg);
    if (rc != SQLITE_OK) {
         mylog_error(m_pLogGlobalCtrl->errorlog, "SQL error:%s  %s! - %s:%s:%d", pErrMsg ,sqlbuf, INFO_LOG_SUFFIX);
        sqlite3_free(pErrMsg);
    }
    return 0;
}

int DbManager::insert_unfetched_url(char *url, void *data, int len, int hostid, bool pushback) {
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    int rc;
    sqlite3_stmt *plineInfo = 0;
    int ret = STATE_SUCCESS;

    char sqlbuf[1024] = {0};
    sprintf(sqlbuf, "insert into %s values(NULL, %d, %d, '%s', ?)", NOT_FETCHED_URL_TABLE_NAME, hostid, pushback?1:0, url);
    rc = sqlite3_prepare(unfetched_url_db, sqlbuf, -1, &plineInfo, 0);
    if (rc == SQLITE_OK && plineInfo != NULL) {
         sqlite3_bind_blob(plineInfo, 1, data, len, NULL);
         rc = sqlite3_step(plineInfo);
         while(rc == SQLITE_BUSY) {
             rc = sqlite3_step(plineInfo);
         }

         if(rc != SQLITE_DONE)
         {
             mylog_error(m_pLogGlobalCtrl->errorlog, "insert into blob value failure! %s %d - %s:%s:%d",  sqlbuf, rc, INFO_LOG_SUFFIX);
             ret = STATE_ERROR;
         }
    }
    rc = sqlite3_finalize(plineInfo);
    //rc = sqlite3_exec(unfetched_url_db, sqlbuf, test_is_exist_callback, &count, &pErrMsg);
    if(rc != SQLITE_OK)
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "%s %d - %s:%s:%d", sqlbuf, rc, INFO_LOG_SUFFIX);
    }
    //test
      /*list<UrlNode *>urls_;
      int hostid_;
      int  url_to_read_;
      get_unfetched_urls(urls_,hostid_,url_to_read_);*/
    //
    return ret;
}


int DbManager::get_unfetched_urls(list<UrlNode *> &urls, int hostid, int url_to_read) {
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    int rc;
    sqlite3_stmt *plineInfo = 0;

    char sqlbuf[1024] = {0};
    sprintf(sqlbuf, "select urldata, id from %s where hostid = %d order by pushback, id limit %d", NOT_FETCHED_URL_TABLE_NAME, hostid, url_to_read);
    //sprintf(sqlbuf, "select urldata, id from %s", NOT_FETCHED_URL_TABLE_NAME);
    rc = sqlite3_prepare(unfetched_url_db, sqlbuf, -1, &plineInfo, 0);

    if (rc == SQLITE_OK && plineInfo != NULL) {
         while ((rc = sqlite3_step(plineInfo)) == SQLITE_ROW) {
             UrlNode *urlnode = new UrlNode;
             char *buf = (char *)sqlite3_column_blob(plineInfo, 0);//Get blob
             urlnode->unserialize(buf);
             urlnode->sqlite_id = sqlite3_column_int(plineInfo, 1); //Get int 
             urls.push_back(urlnode);
         }
         //int blob_size = sqlite3_column_bytes(stat, 1); //Get size
    } else {
         mylog_error(m_pLogGlobalCtrl->veryimportantlog, "prepare! %d - %s:%s:%d", rc, INFO_LOG_SUFFIX);
    }

    rc = sqlite3_finalize(plineInfo);

    if(rc != SQLITE_OK)
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "%s - %s:%s:%d", sqlbuf, INFO_LOG_SUFFIX);
    }
    return 0;
}

int DbManager::get_hostid_by_domain(char *domain, int &id) {
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    int rc;
    sqlite3_stmt *plineInfo = 0;

    char sqlbuf[1024] = {0};
    sprintf(sqlbuf, "select id from %s where domain = '%s'", HOST_TABLE_NAME, domain);
    rc = sqlite3_prepare(unfetched_url_db, sqlbuf, -1, &plineInfo, 0);

    if (rc == SQLITE_OK && plineInfo != NULL) {
         if ((rc = sqlite3_step(plineInfo)) == SQLITE_ROW) {
             id = sqlite3_column_int(plineInfo, 0);//
         }
         //int blob_size = sqlite3_column_bytes(stat, 1); //Get size
    } else {
         mylog_error(m_pLogGlobalCtrl->veryimportantlog, "prepare! %d - %s:%s:%d", rc, INFO_LOG_SUFFIX);
    }

    rc = sqlite3_finalize(plineInfo);

    if(rc != SQLITE_OK)
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "%s - %s:%s:%d", sqlbuf, INFO_LOG_SUFFIX);
    }
    return 0;
}

int DbManager::insert_host(char *domain) {
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    int rc;
    sqlite3_stmt *plineInfo = 0;

    char sqlbuf[1024] = {0};
    sprintf(sqlbuf, "insert into %s values(NULL, '%s')", HOST_TABLE_NAME, domain);
    rc = sqlite3_prepare(unfetched_url_db, sqlbuf, -1, &plineInfo, 0);
    if (rc == SQLITE_OK && plineInfo != NULL) {
         rc = sqlite3_step(plineInfo);
         while(rc == SQLITE_BUSY) {
             rc = sqlite3_step(plineInfo);
         }

         if(rc != SQLITE_DONE)
         {
             mylog_error(m_pLogGlobalCtrl->errorlog, "insert into host value failure! %s %d - %s:%s:%d",  sqlbuf, rc, INFO_LOG_SUFFIX);
         }
    }
    rc = sqlite3_finalize(plineInfo);
    //rc = sqlite3_exec(unfetched_url_db, sqlbuf, test_is_exist_callback, &count, &pErrMsg);
    if(rc != SQLITE_OK)
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "%s %d - %s:%s:%d", sqlbuf, rc, INFO_LOG_SUFFIX);
    }
    return 0;
}

#include "DbMySQLManager.h"
//#include "Definition.h"

extern map<int,SiteSourceInfo *> mySourceInfo;
extern multimap<string ,TemplateInfo> multitemplates;
pthread_rwlock_t DbMySQLManager::wr_lock=PTHREAD_RWLOCK_INITIALIZER;
//pthread_rwlock_t DbMySQLManager::map_write= PTHREAD_RWLOCK_INITIALIZER;

DbMySQLManager::DbMySQLManager(DbConnectionManager *db_connection_manager) {
    db_connection_manager_=db_connection_manager;
}

DbMySQLManager::~DbMySQLManager() {
}

int DbMySQLManager::GetSourceInfo( void * conn, map<int,SiteSourceInfo *> &mySourceInfo,bool isUpdate) {
    if(conn==NULL)
    {
        conn = db_connection_manager_->get_connection();
    }
    int rettmp =COMMON_STATE_SUCCESS; 
    sql::Connection *connect = static_cast<sql::Connection *>(conn) ;  
    sql::Statement *stmt = NULL;  
    sql::ResultSet *res = NULL; 

    char sqlbuf[1024 * 5] = "";
    if(isUpdate)
        sprintf(sqlbuf, "select id,source_id,fetch_interval,name,host,homeurl,pageurl,ultimatepageurl,intxt from source_info where state=1");
    try {

        stmt = connect->createStatement();
        stmt->execute("set names gbk");
        res = stmt->executeQuery(sqlbuf);
        res->beforeFirst();
        int nnn = res->rowsCount();
        while (res->next())
        {
            SiteSourceInfo *tempSource = new SiteSourceInfo;
            char tempBuffer[1024];
            long id = res->getInt("id");

            tempSource->id = res->getInt("id");
            tempSource->source_id = res->getInt("source_id");
            tempSource->fetch_interval = res->getInt("fetch_interval");
            tempSource->name= res->getString("name");
            tempSource->host=res->getString("host");
            tempSource->homeurl=res->getString("homeurl");
            tempSource->pageurl=res->getString("pageurl");
            tempSource->ultimatepageurl=res->getString("ultimatepageurl");
            tempSource->intxt=res->getString("intxt");
            //加锁
            pthread_rwlock_wrlock(&wr_lock);      
            if (isUpdate && mySourceInfo[id]!=NULL )
            {
               mySourceInfo[id]->id=tempSource->id;
               mySourceInfo[id]->source_id=tempSource->source_id;
               mySourceInfo[id]->fetch_interval=tempSource->fetch_interval;
               mySourceInfo[id]->name=tempSource->name;
               mySourceInfo[id]->host=tempSource->host;
               mySourceInfo[id]->homeurl=tempSource->homeurl;
               mySourceInfo[id]->pageurl=tempSource->pageurl;
               mySourceInfo[id]->ultimatepageurl=tempSource->ultimatepageurl;
               mySourceInfo[id]->intxt=tempSource->intxt;
            }
            else
            {
               mySourceInfo[id] = tempSource;
            }
            pthread_rwlock_unlock(&wr_lock);
        }
        delete res;
        delete stmt;
    }
    catch (sql::SQLException &e)
    {
        rettmp =COMMON_STATE_ERROR;  
        //MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "testConnection  erro = %s, error code = %d, sql state = %s", e.what(), e.getErrorCode(), e.getSQLState().c_str());
    }


    db_connection_manager_->release_connection(conn);

    return rettmp;
}

int DbMySQLManager::executesql(string & sql) {
    int ret = EXECUTESQL_ERROR;
    void  * conn = this->db_connection_manager_->get_connection();
    sql::Connection *connect = static_cast<sql::Connection *>(conn) ;
    sql::Statement *stmt = NULL;
    sql::ResultSet *res = NULL;
    sql::PreparedStatement *pstmt;
    try
    {
        stmt = connect->createStatement();
        stmt->execute(sql.c_str());
        // return EXECUTESQL_SUCCESS;
    }
    catch (sql::SQLException &e)
    {
       // MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "testConnection  erro = %s, error code = %d, sql state =  %s", e.what(), e.getErrorCode(), e.getSQLState().c_str());
        ret = EXECUTESQL_ERROR;
    }
    db_connection_manager_->release_connection(conn);
    ret = EXECUTESQL_SUCCESS;
    return ret;
}

int DbMySQLManager::DestorySql(ResultSet* res, PreparedStatement* prep_stmt, Statement *stmt)
{
    if (res != NULL)
    {
        try
        {
            res->close();
        }
        catch(sql::SQLException &e)
        {
            //MY_ERROR_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "DestorySql erro = %s, error code = %d, sql state =  %s ", e.what(), e.getErrorCode(), e.getSQLState().c_str());
        }
        delete res;res = NULL;
    }
    if (prep_stmt!= NULL)
    {
        try
        {
            prep_stmt->close();
        }
        catch(sql::SQLException &e)
        {
           // MY_ERROR_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "DestorySql erro = %s, error code = %d, sql state =  %s ", e.what(), e.getErrorCode(), e.getSQLState().c_str());
        }
        delete prep_stmt;prep_stmt= NULL;

    }
    if (stmt != NULL)
    {
        try
        {
            stmt ->close();
        }
        catch(sql::SQLException &e)
        {
           // MY_ERROR_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "DestorySql erro = %s, error code = %d, sql state =  %s ", e.what(), e.getErrorCode(), e.getSQLState().c_str());
        }
        delete stmt;stmt = NULL;

    }
}

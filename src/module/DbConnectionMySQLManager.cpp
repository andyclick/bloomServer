#include "DbConnectionMySQLManager.h"
//#include "MainSingleton.h"
#include <stdlib.h>
#include "MySQLConnectionPool.h"

pthread_mutex_t DbConnectionMySQLManager::pool_mutex = PTHREAD_MUTEX_INITIALIZER;

DbConnectionMySQLManager::DbConnectionMySQLManager(char *user_name ,char *password, char *db_name,int max_conn, int min_conn, int incr_conn, char * db_server_name ) {
    user_name_ = user_name;
    password_ = password;
    db_name_ = db_name;
    max_conn_ = max_conn;
    min_conn_ = min_conn;
    incr_conn_ = incr_conn;
    db_server_name_=db_server_name;
    conn_pool_ = new MySQLConnectionPool(db_server_name_, user_name_,
                                        password_,min_conn, incr_conn,
                                        max_conn,db_name); 
    open_connection_pool();
}
DbConnectionMySQLManager::~DbConnectionMySQLManager() {
    //close_connectiddon_pool();
    delete conn_pool_;
    conn_pool_ = NULL;
}

int DbConnectionMySQLManager::close_connection_pool() {
    int ret = COMMON_STATE_SUCCESS;
}

void *DbConnectionMySQLManager::get_connection() {
    sql::Connection *  connect_ = conn_pool_->getConnection(); 
    return connect_;
}
int DbConnectionMySQLManager::release_connection(void * conn) {
   int ret = COMMON_STATE_SUCCESS;
   conn_pool_->returnConnection((sql::Connection * )conn); 
   return ret;
}

int DbConnectionMySQLManager::open_connection_pool()
{
    int ret = COMMON_STATE_SUCCESS;
    conn_pool_->createPool();
    return ret;
}
int DbConnectionMySQLManager::rollback(void * pConnect)
{
    sql::Connection * pConnecttmp = (sql::Connection *) pConnect;
    int ret = COMMON_STATE_SUCCESS;
    if (pConnecttmp== NULL) ret = COMMON_STATE_ERROR;
    try{
        pConnecttmp->rollback();
    }catch(sql::SQLException &e)
    {
        //MY_ERROR_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "rollback error \r\n erro = %s, error code = %d, sql state = %s", e.what(), e.getErrorCode(), e.getSQLState().c_str());
        ret = COMMON_STATE_ERROR;
    }catch (...)
    {
        ret = COMMON_STATE_ERROR;
    }

    return ret;
}
int DbConnectionMySQLManager::commit(void * pConnect)
{
    sql::Connection * pConnecttmp = (sql::Connection *) pConnect;
    int ret = COMMON_STATE_SUCCESS;
    if (pConnect == NULL) ret = COMMON_STATE_ERROR;
    try{
        pConnecttmp->commit();
    }catch(sql::SQLException &e)
    {
        rollback(pConnecttmp);
        //MY_ERROR_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "commit error \r\n erro = %s, error code = %d, sql state = %s", e.what(), e.getErrorCode(), e.getSQLState().c_str());
        ret = COMMON_STATE_ERROR;
    }catch (...)
    {
        ret = COMMON_STATE_ERROR;
    }
    return ret;
}
int DbConnectionMySQLManager:: end_transaction(void * pConnect)
{
    sql::Connection * pConnecttmp = (sql::Connection *) pConnect;
    int ret = COMMON_STATE_SUCCESS;
    if (pConnecttmp == NULL) ret = COMMON_STATE_ERROR;
    
    try{
        pConnecttmp->setAutoCommit(true); 
    }catch(sql::SQLException &e)
    {
        //MY_ERROR_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "commit error \r\n erro = %s, error code = %d, sql state = %s", e.what(), e.getErrorCode(), e.getSQLState().c_str());
        ret = COMMON_STATE_ERROR;
    }catch (...)
    {
        ret = COMMON_STATE_ERROR;
    }
    return ret;

}
int DbConnectionMySQLManager:: begin_transaction(void * pConnect)
{
    sql::Connection * pConnecttmp = (sql::Connection *) pConnect;
    int ret = COMMON_STATE_SUCCESS;
    if (pConnecttmp == NULL) ret = COMMON_STATE_ERROR;
    
    try{
        pConnecttmp->setAutoCommit(false); 
    }catch(sql::SQLException &e)
    {
        //MY_ERROR_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "commit error \r\n erro = %s, error code = %d, sql state = %s", e.what(), e.getErrorCode(), e.getSQLState().c_str());
        ret = COMMON_STATE_ERROR;
    }catch (...)
    {
        ret = COMMON_STATE_ERROR;
    }
    return ret;
}

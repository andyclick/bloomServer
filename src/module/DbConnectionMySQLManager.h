#ifndef __DB_MYSQL_CONNECTION_MANAGER_H__
#define __DB_MYSQL_CONNECTION_MANAGER_H__

#include "DbConnectionManager.h"
//#include "MySQLConnectionPool.h"
#include "Definition.h"
#include "config.h"
#include <pthread.h>
using namespace std;

class MySQLConnectionPool;

class DbConnectionMySQLManager:public DbConnectionManager {
public:
    DbConnectionMySQLManager(char *user_name ,char *password, char *db_name,int max_conn, int min_conn, int incr_conn, char * db_server_name ); 
    ~DbConnectionMySQLManager();

    int rollback(void * pConnect);
    void *get_connection();
    int release_connection(void * pConnect);
    
    int commit(void * pConnect);
    int end_transaction(void * pConnect);
    int begin_transaction(void * pConnect);
    
    MySQLConnectionPool * conn_pool_;

private:
    string user_name_;
    string password_;
    string db_name_;
    string db_server_name_;
    int max_conn_;
    int min_conn_;
    int incr_conn_;
    

private:
    int open_connection_pool();
    int close_connection_pool();

    static pthread_mutex_t pool_mutex;

};

#endif //__DB_MYSQL_CONNECTION_MANAGER_H__

#ifndef __SERVICE_DBORACLEMANAGER_H_
#define __SERVICE_DBORACLEMANAGER_H_
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include "DbManager.h"
#include "mycxxlog.h"
//#include "MainSingleton.h"
//#include "Manager.h"
#include <pthread.h>

//#include "Definition.h"
#include "DbConnectionMySQLManager.h"
//

#include <sys/types.h>

#ifdef _MSC_VER /* _WIN32 */
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
using namespace std;


//

#include <cppconn/driver.h>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
using namespace sql;

typedef unsigned int UINT;




class DbMySQLManager{
 public:
    DbMySQLManager(DbConnectionManager *db_connection_manager);
    ~DbMySQLManager();
    int GetSourceInfo( void * conn, map<int,SiteSourceInfo *> &mySourceInfo,bool isUpdate);
    int executesql(string & sql);
    DbConnectionManager *db_connection_manager_;
private:
     int DestorySql(ResultSet* res, PreparedStatement* prep_stmt, Statement *stmt);
    static pthread_rwlock_t wr_lock;
}; 
#endif

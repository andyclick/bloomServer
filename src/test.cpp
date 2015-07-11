#include <iostream>
#include <fstream>
#include "util.h"
#include "cmsinfo.h"
#include "ic_types.h"
#include "utilcpp.h"
#include "UrlAnalyseManager.h"
#include "DbManager.h"
#include "LocalDbManager.h"
#include "Http.h"
#include "uri.h"
#include "Url.h"
#include "hlink.h"
#include "ic_types.h"
#include <zlib.h>
#include "HtRegex.h"
#include "FSBigFile.h"
#include "DBAccess.h"
#include "DBAccess.h"
#include "DbConnectionMySQLManager.h"
using namespace std;

#include <cppconn/driver.h>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
using namespace sql;


int executesql(char * sql); 
int test_insert();
void testMysql();
int executesql(char * sql,int * count); 
int DestorySql(ResultSet* res, PreparedStatement* prep_stmt, Statement *stmt);

DbConnectionMySQLManager * db_connection_manager;
int main()
{
  testMysql();
    return 0;

}
void testMysql()
{
  string username = "root";
  string password = "root";
  string dbname = "test";
  string db_server_name = "tcp://118.194.35.22:9999";
  int max_conn = 5;
  int min_conn = 2;
  int incr_conn = 1;
  
  db_connection_manager = new DbConnectionMySQLManager((char *)username.c_str(), (char *)password.c_str(), (char *)dbname.c_str(), max_conn, min_conn, incr_conn, (char *)db_server_name.c_str());
  test_insert(); 
}

int test_insert()
{
  string sql = "insert into zn_test(url, insert_time, keyword) values ( ";
  sql.append("'");
  sql.append("http://liujing.test.cn/liujingtest");sql.append("' ,");
  sql.append("now()");
  sql.append(", '");sql.append("ÖÐ¹ú");
  sql.append("' ");
  sql.append(")");

  executesql((char *)sql.c_str());
}
int executesql(char * sql) {

      return executesql(sql, NULL);
}

int executesql(char * sql,int * count) {
  int ret = EXECUTESQL_ERROR;
  void  * conn = db_connection_manager->get_connection();
  sql::Connection *connect = static_cast<sql::Connection *>(conn) ;
  sql::Statement *stmt = NULL;
  sql::ResultSet *res = NULL;
  sql::PreparedStatement *pstmt = NULL;
  try
  {
    stmt = connect->createStatement();
    stmt->execute("set names utf-8");
    if (count)
    {
      res = stmt->executeQuery(sql);
      res->beforeFirst();
      while (res->next())
      {
        *count = res->getInt("count");
      }   
    }else   
    {
      stmt->executeUpdate(sql);
    }   
  }  
  catch (sql::SQLException &e)
  {
    ret = EXECUTESQL_ERROR;
    printf("executeSql  erro = %s, error code = %d, sql state = %s", e.what(), e.getErrorCode(), e.getSQLState().c_str());
  }   
  DestorySql(res,pstmt,stmt);
  db_connection_manager->release_connection(conn);
  ret = EXECUTESQL_SUCCESS;
  return ret;
}

int DestorySql(ResultSet* res, PreparedStatement* prep_stmt, Statement *stmt)
{
  if (res != NULL)
  {
    try
    {
      res->close();
    }
    catch(sql::SQLException &e)
    {
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
    }
    delete stmt;stmt = NULL;

  }
}

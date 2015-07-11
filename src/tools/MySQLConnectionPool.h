#ifndef __MYSQLCONNECTION________POOL_H_
#define __MYSQLCONNECTION________POOL_H_ 
#include <vector>
#include <string>

#include <cppconn/driver.h>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
//#include "MainSingleton.h"
using namespace std;
using namespace sql;
using namespace mysql;

class MySQLConnectionPool 
{
public:
	class PooledConnection {
		
        sql::Connection * connection;// 数据库连接
		
		bool busy ; // 此连接是否正在使用的标志，默认没有正在使用
		
		// 构造函数，根据一个 Connection 构告一个 PooledConnection 对象
	public :
		PooledConnection(sql::Connection * connection) {
			this->connection = connection;
			busy = false;
		}
		
		// 返回此对象中的连接
        sql::Connection * getConnection() {
			return connection;
		}
		
		// 设置此对象的，连接
		void setConnection(sql::Connection *connection) {
			this->connection = connection;
			this->busy=false;
		}
		
		// 获得对象连接是否忙
		bool isBusy() {
			return busy;
		}
		
		// 设置对象的连接正在忙
		void setBusy(bool busy) {
			this->busy = busy;
		}
	};	
private:
    sql::Driver *driver;	
	string dbUrl; // 数据 URL
	
	string dbUsername ; // 数据库用户名
	
	string dbPassword ; // 数据库用户密码
	
	string testTable ; // 测试连接是否可用的测试表名，默认没有测试表
	
    string dbname;
    int initialConnections ; // 连接池的初始大小
	
	int incrementalConnections;// 连接池自动增加的大小
	
	int maxConnections ; // 连接池最大的大小


	void *param;
	
	vector<PooledConnection *> connections; // 存放连接池中数据库连接的向量 , 初始时为 null
    static pthread_mutex_t mutex_connectionpool;
	
	void createConnections(int numConnections);//要创建的数据库连接的数目
    sql::Connection *newConnection();//返回一个数据库连接
    sql::Connection *getFreeConnection(); //函数从连接池向量 connections 中返回一个可用的的数据库连接
    sql::Connection *findFreeConnection(); //查找连接池中所有的连接，查找一个可用的数据库连接
	bool testConnection(sql::Connection * conn);//测试一个连接是否可用，如果不可用，关掉它并返回 false
	void closeConnection(sql::Connection *conn);//关闭一个数据库连接
	void wait(int mSeconds) ;//使程序等待给定的毫秒数
	
    void Destroy();
public:
	MySQLConnectionPool(string dbUrl, string dbUsername, string dbPassword ,
							   int initialConnections , int incrementalConnections, 
							   int maxConnections,string dbservername); 

	~MySQLConnectionPool();
	int getInitialConnections() ;//初始连接池中可获得的连接数量
	void setInitialConnections(int initialConnections) ;//设置连接池的初始大小
	int getIncrementalConnections() ;//返回连接池自动增加的大小 、
	void setIncrementalConnections(int incrementalConnections);//设置连接池自动增加的大小
	int getMaxConnections() ;//返回连接池中最大的可用连接数量
	string getTestTable();//获取数据库名
	void  setTestTable(string testTable);//获取表的名字	
	//同步方法
	 void createPool() ; // 创建一个数据库连接池，连接池中的可用连接的数量采用类成员???
     sql::Connection * getConnection() ;//返回一个可用的数据库连接对象

	void returnConnection(sql::Connection * conn) ;//此函数返回一个数据库连接到连接池中，并把此连接置为空闲。
	//同步方法
	void refreshConnections() ;//刷新连接池中所有的连接对象
	void closeConnectionPool() ;//关闭连接池中所有的连接，并清空连接池

};
#endif //end __MYSQLCONNECTION_POOL_H_

#include <string.h>
#include <stdio.h>
#include "MySQLConnectionPool.h"
using namespace sql;

pthread_mutex_t MySQLConnectionPool::mutex_connectionpool = PTHREAD_MUTEX_INITIALIZER;

MySQLConnectionPool::MySQLConnectionPool(string dbUrl, string dbUsername, string dbPassword ,
							   int initialConnections , int incrementalConnections, 
							   int maxConnections,string dbname) {
	this->dbUrl = dbUrl;
	this->dbUsername = dbUsername;
	this->dbPassword = dbPassword;
	this->incrementalConnections =incrementalConnections;
	this->initialConnections = initialConnections;
	this->maxConnections= maxConnections;
	this->dbname= dbname;
  driver = sql::mysql::get_driver_instance();
}

MySQLConnectionPool::~MySQLConnectionPool() {
	this->Destroy();
}

//初始连接池中可获得的连接数量
int MySQLConnectionPool::getInitialConnections() {
	return this->initialConnections;
}


//设置连接池的初始大小
void MySQLConnectionPool::setInitialConnections(int initialConnections) {
	this->initialConnections = initialConnections;
	
} 
//返回连接池自动增加的大小 
int MySQLConnectionPool:: getIncrementalConnections() {
	return this->incrementalConnections;
}

//设置连接池自动增加的大小
void MySQLConnectionPool::setIncrementalConnections(int incrementalConnections) {
	this->incrementalConnections = incrementalConnections;
}

//返回连接池中最大的可用连接数量
int MySQLConnectionPool::getMaxConnections() {
	return this->maxConnections;
}

//获取数据库名
string MySQLConnectionPool:: getTestTable() {
	return this->testTable;
}

//获取表的名字	
void  MySQLConnectionPool::setTestTable(string testTable) {
	this->testTable = testTable;
}
// 创建一个数据库连接池	
//CSingleLock singleLock(&m_CritSection);

void MySQLConnectionPool::createPool() {
	pthread_mutex_lock(&mutex_connectionpool);
	createConnections(this->initialConnections);
	pthread_mutex_unlock(&mutex_connectionpool);
}

//返回一个可用的数据库连接对象
sql::Connection *  MySQLConnectionPool::getConnection() {
	pthread_mutex_lock(&mutex_connectionpool);
	if (connections.empty()) {
		pthread_mutex_unlock(&mutex_connectionpool);
		return NULL; // 连接池还没创建，则返回 null		
	}
	//MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "connection 1");
    sql::Connection *conn = getFreeConnection(); // 获得一个可用的数据库连接
	//_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "connection 2");
	
	// 如果目前没有可以使用的连接，即所有的连接都在使用中
	pthread_mutex_unlock(&mutex_connectionpool);

	while (conn == NULL) {	
		// 等一会再试		
		wait(250);
		pthread_mutex_lock(&mutex_connectionpool);
	    //MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "connection 3");
		conn = getFreeConnection(); // 重新再试，直到获得可用的连接，如果
	  //  MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "connection 4");
		pthread_mutex_unlock(&mutex_connectionpool);
	}
	//MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "connection 5");
	return conn;// 返回获得的可用的连接
	
}

//此函数返回一个数据库连接到连接池中，并把此连接置为空闲。
void MySQLConnectionPool::returnConnection(sql::Connection *conn) {
	pthread_mutex_lock(&mutex_connectionpool);
	if (connections.empty()) {
		pthread_mutex_unlock(&mutex_connectionpool);
		return;
	}
	
	PooledConnection *pConn = NULL;
	
	// 遍历连接池中的所有连接，找到这个要返回的连接对象
	for (int i= 0 ; i < connections.size() ; i++)
	{	
		pConn =connections.at(i);
		
		// 先找到连接池中的要返回的连接对象	
		if (conn == pConn->getConnection()) {
			
			// 找到了 , 设置此连接为空闲状态
			pConn->setBusy(false);
			break;	
		}		
	}
	pthread_mutex_unlock(&mutex_connectionpool);
}

//刷新连接池中所有的连接对象
void MySQLConnectionPool::refreshConnections() {
	// 确保连接池己创新存在
	pthread_mutex_lock(&mutex_connectionpool);
	if (connections.empty()) {
		pthread_mutex_unlock(&mutex_connectionpool);
		return;
	}
	
	PooledConnection *pConn = NULL;
	
	for(int i =0 ; i<connections.size(); i++) {
		
		// 获得一个连接对象
		
		pConn = connections.at(i);
		
		// 如果对象忙则等 5 秒 ,5 秒后直接刷新
		
		if (pConn->isBusy()) {
			
			wait(5000); // 等 5 秒
			
		}
		
		// 关闭此连接，用一个新的连接代替它。
		
		closeConnection(pConn->getConnection());
		
		pConn->setConnection(newConnection());
		
		pConn->setBusy(false);
		
	}
	pthread_mutex_unlock(&mutex_connectionpool);
}

//关闭连接池中所有的连接，并清空连接池
void MySQLConnectionPool::closeConnectionPool() {
	// 确保连接池存在，如果不存在，返回
	pthread_mutex_lock(&mutex_connectionpool);
	if (connections.empty()) {
		pthread_mutex_unlock(&mutex_connectionpool);
		return;
	}

	PooledConnection *pConn = NULL;

	for(int i = 0 ; i < connections.size(); i++){
		
		pConn = connections.at(i);
		
		// 如果忙，等 5 秒
		if (pConn->isBusy()) {
			wait(5000); // 等 5 秒
		}
		
		//5 秒后直接关闭它	
		closeConnection(pConn->getConnection());	
		// 从连接池向量中删除它	
		delete pConn;
		connections.erase(connections.begin()+i);
	}

	pthread_mutex_unlock(&mutex_connectionpool);
	// 置连接池为空
	connections.clear();
}

//要创建的数据库连接的数目
void MySQLConnectionPool::createConnections(int numConnections) {
	
	for (int x = 0; x < numConnections; x++) {
		
		// 是否连接池中的数据库连接的数量己经达到最大？最大值由类成员 maxConnections
		
		// 指出，如果 maxConnections 为 0 或负数，表示连接数量没有限制。
		
		// 如果连接数己经达到最大，即退出。		
		if (this->maxConnections > 0 && this->connections.size() >= this->maxConnections) {
			break;
		}
		
		// 增加一个连接到连接池中（向量 connections 中）
		try {
			
            sql::Connection *connection= newConnection();
			if (connection != NULL)
			{
				PooledConnection *poolconn = new PooledConnection(connection);
				connections.push_back(poolconn);
			}
			
		} catch(...) {
			
		}
	}
}

//返回一个数据库连接
sql::Connection * MySQLConnectionPool::newConnection() {
    sql::Connection * m_pConnection=NULL;
	try
	{	
        m_pConnection = driver->connect(dbUrl, dbUsername, dbPassword);
        m_pConnection->setSchema(dbname);
		/*int num = 0;
		while(num++ < 100)
		{
			if(m_pConnection->State == adStateOpen) 
				break;
			
			Sleep(100);
		}
		if (m_pConnection->State != adStateOpen) {
			return NULL;
		}*/
		return m_pConnection;
	}
	catch(sql::SQLException &e)
	{
        //MY_ERROR_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "连接数据库失败!\r\n错误信息: erro = %s, error code = %d, sql state = %s", e.what(), e.getErrorCode(), e.getSQLState().c_str());
	}
	catch (...)
	{
		return NULL;
	}
	return NULL;
}

//函数从连接池向量 connections 中返回一个可用的的数据库连接
sql::Connection * MySQLConnectionPool::getFreeConnection() {
	// 从连接池中获得一个可用的数据库连接
    //MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "connection 1.1");	
    sql::Connection * conn = findFreeConnection();
    //MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "connection 1.2");	
	
	if (conn == NULL) {
		// 如果目前连接池中没有可用的连接
		
		// 创建一些连接
      //  MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "connection 1.3");	
		createConnections(incrementalConnections);
        //MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "connection 1.4");	
		// 重新从池中查找是否有可用连接
		conn = findFreeConnection();
		if (conn == NULL) {
			// 如果创建连接后仍获得不到可用的连接，则返回 null
			return NULL;
		}
	}
    //MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "connection 1.5");	
	return conn;
}

//查找连接池中所有的连接，查找一个可用的数据库连接
sql::Connection * MySQLConnectionPool::findFreeConnection() {
    sql::Connection * conn = NULL;
	
	PooledConnection *pConn = NULL;
	
	// 获得连接池向量中所有的对象
	for(int i = 0 ;i<connections.size(); i++) {
		
		pConn = connections.at(i);
		
		if (!pConn->isBusy()) {
			
			// 如果此对象不忙，则获得它的数据库连接并把它设为忙
			
			conn = pConn->getConnection();
			
			
			// 测试此连接是否可用
 			if (!testConnection(conn)) {
 				
 				// 如果此连接不可再用了，则创建一个新的连接，
 				
 				// 并替换此不可用的连接对象，如果创建失败，返回 null
 				
 				try {
 					conn = newConnection();
				} catch(...) {
 					return NULL;	
 				}
 				
				if (conn) {
 					pConn->setConnection(conn);	
					pConn->setBusy(true);
				} else {
					return NULL;
				}
			} else {
				pConn->setBusy(true);
			}
			break; // 己经找到一个可用的连接，退出	
		}	
	}
    //mysql_query( mysql, "SET NAMES" gbk"" );
    return conn;// 返回找到到的可用连接
}

//测试一个连接是否可用，如果不可用，关掉它并返回 false 
bool MySQLConnectionPool::testConnection(sql::Connection *conn) {
	bool ret = false;

    sql::Statement *stmt = NULL;
    sql::ResultSet *res = NULL;
	try {
		char sqlbuf[1024] = "";
		sprintf(sqlbuf, "select   now()");

		try {
	        stmt = conn->createStatement();
            res = stmt->executeQuery(sqlbuf);
            ret = true;
		}
		catch(sql::SQLException &e)
		{
            //MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "testConnection  erro = %s, error code = %d, sql state = %s", e.what(), e.getErrorCode(), e.getSQLState().c_str());
			closeConnection(conn);
		}
		
	} catch (...) {
		closeConnection(conn);
	}
    delete res;
    delete stmt;
    // 连接可用，返回 true
	return ret;
}

//关闭一个数据库连接
void MySQLConnectionPool::closeConnection(sql::Connection * poolconn) {
    if (poolconn)
    {
        try {
           poolconn->close();
        }catch (...) {
            
        }
    }
    delete poolconn;
}

//使程序等待给定的毫秒数
void MySQLConnectionPool::wait(int mSeconds) {
	//线程等待时间		
	sleep(mSeconds);
}
void MySQLConnectionPool::Destroy()
{
 	pthread_mutex_lock(&mutex_connectionpool);
	if (connections.empty()) {
		pthread_mutex_unlock(&mutex_connectionpool);
		return;
	}
	
	PooledConnection *pConn = NULL;
	
	// 遍历连接池中的所有连接，找到这个要返回的连接对象
	for (int i= 0 ; i < connections.size() ; i++)
	{	
		pConn =connections.at(i);
        closeConnection(pConn->getConnection());
    }
    connections.clear();
	pthread_mutex_unlock(&mutex_connectionpool);
}

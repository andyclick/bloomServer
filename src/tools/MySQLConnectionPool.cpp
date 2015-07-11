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

//��ʼ���ӳ��пɻ�õ���������
int MySQLConnectionPool::getInitialConnections() {
	return this->initialConnections;
}


//�������ӳصĳ�ʼ��С
void MySQLConnectionPool::setInitialConnections(int initialConnections) {
	this->initialConnections = initialConnections;
	
} 
//�������ӳ��Զ����ӵĴ�С 
int MySQLConnectionPool:: getIncrementalConnections() {
	return this->incrementalConnections;
}

//�������ӳ��Զ����ӵĴ�С
void MySQLConnectionPool::setIncrementalConnections(int incrementalConnections) {
	this->incrementalConnections = incrementalConnections;
}

//�������ӳ������Ŀ�����������
int MySQLConnectionPool::getMaxConnections() {
	return this->maxConnections;
}

//��ȡ���ݿ���
string MySQLConnectionPool:: getTestTable() {
	return this->testTable;
}

//��ȡ�������	
void  MySQLConnectionPool::setTestTable(string testTable) {
	this->testTable = testTable;
}
// ����һ�����ݿ����ӳ�	
//CSingleLock singleLock(&m_CritSection);

void MySQLConnectionPool::createPool() {
	pthread_mutex_lock(&mutex_connectionpool);
	createConnections(this->initialConnections);
	pthread_mutex_unlock(&mutex_connectionpool);
}

//����һ�����õ����ݿ����Ӷ���
sql::Connection *  MySQLConnectionPool::getConnection() {
	pthread_mutex_lock(&mutex_connectionpool);
	if (connections.empty()) {
		pthread_mutex_unlock(&mutex_connectionpool);
		return NULL; // ���ӳػ�û�������򷵻� null		
	}
	//MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "connection 1");
    sql::Connection *conn = getFreeConnection(); // ���һ�����õ����ݿ�����
	//_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "connection 2");
	
	// ���Ŀǰû�п���ʹ�õ����ӣ������е����Ӷ���ʹ����
	pthread_mutex_unlock(&mutex_connectionpool);

	while (conn == NULL) {	
		// ��һ������		
		wait(250);
		pthread_mutex_lock(&mutex_connectionpool);
	    //MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "connection 3");
		conn = getFreeConnection(); // �������ԣ�ֱ����ÿ��õ����ӣ����
	  //  MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "connection 4");
		pthread_mutex_unlock(&mutex_connectionpool);
	}
	//MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "connection 5");
	return conn;// ���ػ�õĿ��õ�����
	
}

//�˺�������һ�����ݿ����ӵ����ӳ��У����Ѵ�������Ϊ���С�
void MySQLConnectionPool::returnConnection(sql::Connection *conn) {
	pthread_mutex_lock(&mutex_connectionpool);
	if (connections.empty()) {
		pthread_mutex_unlock(&mutex_connectionpool);
		return;
	}
	
	PooledConnection *pConn = NULL;
	
	// �������ӳ��е��������ӣ��ҵ����Ҫ���ص����Ӷ���
	for (int i= 0 ; i < connections.size() ; i++)
	{	
		pConn =connections.at(i);
		
		// ���ҵ����ӳ��е�Ҫ���ص����Ӷ���	
		if (conn == pConn->getConnection()) {
			
			// �ҵ��� , ���ô�����Ϊ����״̬
			pConn->setBusy(false);
			break;	
		}		
	}
	pthread_mutex_unlock(&mutex_connectionpool);
}

//ˢ�����ӳ������е����Ӷ���
void MySQLConnectionPool::refreshConnections() {
	// ȷ�����ӳؼ����´���
	pthread_mutex_lock(&mutex_connectionpool);
	if (connections.empty()) {
		pthread_mutex_unlock(&mutex_connectionpool);
		return;
	}
	
	PooledConnection *pConn = NULL;
	
	for(int i =0 ; i<connections.size(); i++) {
		
		// ���һ�����Ӷ���
		
		pConn = connections.at(i);
		
		// �������æ��� 5 �� ,5 ���ֱ��ˢ��
		
		if (pConn->isBusy()) {
			
			wait(5000); // �� 5 ��
			
		}
		
		// �رմ����ӣ���һ���µ����Ӵ�������
		
		closeConnection(pConn->getConnection());
		
		pConn->setConnection(newConnection());
		
		pConn->setBusy(false);
		
	}
	pthread_mutex_unlock(&mutex_connectionpool);
}

//�ر����ӳ������е����ӣ���������ӳ�
void MySQLConnectionPool::closeConnectionPool() {
	// ȷ�����ӳش��ڣ���������ڣ�����
	pthread_mutex_lock(&mutex_connectionpool);
	if (connections.empty()) {
		pthread_mutex_unlock(&mutex_connectionpool);
		return;
	}

	PooledConnection *pConn = NULL;

	for(int i = 0 ; i < connections.size(); i++){
		
		pConn = connections.at(i);
		
		// ���æ���� 5 ��
		if (pConn->isBusy()) {
			wait(5000); // �� 5 ��
		}
		
		//5 ���ֱ�ӹر���	
		closeConnection(pConn->getConnection());	
		// �����ӳ�������ɾ����	
		delete pConn;
		connections.erase(connections.begin()+i);
	}

	pthread_mutex_unlock(&mutex_connectionpool);
	// �����ӳ�Ϊ��
	connections.clear();
}

//Ҫ���������ݿ����ӵ���Ŀ
void MySQLConnectionPool::createConnections(int numConnections) {
	
	for (int x = 0; x < numConnections; x++) {
		
		// �Ƿ����ӳ��е����ݿ����ӵ����������ﵽ������ֵ�����Ա maxConnections
		
		// ָ������� maxConnections Ϊ 0 ��������ʾ��������û�����ơ�
		
		// ��������������ﵽ��󣬼��˳���		
		if (this->maxConnections > 0 && this->connections.size() >= this->maxConnections) {
			break;
		}
		
		// ����һ�����ӵ����ӳ��У����� connections �У�
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

//����һ�����ݿ�����
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
        //MY_ERROR_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "�������ݿ�ʧ��!\r\n������Ϣ: erro = %s, error code = %d, sql state = %s", e.what(), e.getErrorCode(), e.getSQLState().c_str());
	}
	catch (...)
	{
		return NULL;
	}
	return NULL;
}

//���������ӳ����� connections �з���һ�����õĵ����ݿ�����
sql::Connection * MySQLConnectionPool::getFreeConnection() {
	// �����ӳ��л��һ�����õ����ݿ�����
    //MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "connection 1.1");	
    sql::Connection * conn = findFreeConnection();
    //MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "connection 1.2");	
	
	if (conn == NULL) {
		// ���Ŀǰ���ӳ���û�п��õ�����
		
		// ����һЩ����
      //  MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "connection 1.3");	
		createConnections(incrementalConnections);
        //MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "connection 1.4");	
		// ���´ӳ��в����Ƿ��п�������
		conn = findFreeConnection();
		if (conn == NULL) {
			// ����������Ӻ��Ի�ò������õ����ӣ��򷵻� null
			return NULL;
		}
	}
    //MY_INFO_LOG(MainSingleton::getInstance()->logger[LOGGER_INFO_IDX], "connection 1.5");	
	return conn;
}

//�������ӳ������е����ӣ�����һ�����õ����ݿ�����
sql::Connection * MySQLConnectionPool::findFreeConnection() {
    sql::Connection * conn = NULL;
	
	PooledConnection *pConn = NULL;
	
	// ������ӳ����������еĶ���
	for(int i = 0 ;i<connections.size(); i++) {
		
		pConn = connections.at(i);
		
		if (!pConn->isBusy()) {
			
			// ����˶���æ�������������ݿ����Ӳ�������Ϊæ
			
			conn = pConn->getConnection();
			
			
			// ���Դ������Ƿ����
 			if (!testConnection(conn)) {
 				
 				// ��������Ӳ��������ˣ��򴴽�һ���µ����ӣ�
 				
 				// ���滻�˲����õ����Ӷ����������ʧ�ܣ����� null
 				
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
			break; // �����ҵ�һ�����õ����ӣ��˳�	
		}	
	}
    //mysql_query( mysql, "SET NAMES" gbk"" );
    return conn;// �����ҵ����Ŀ�������
}

//����һ�������Ƿ���ã���������ã��ص��������� false 
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
    // ���ӿ��ã����� true
	return ret;
}

//�ر�һ�����ݿ�����
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

//ʹ����ȴ������ĺ�����
void MySQLConnectionPool::wait(int mSeconds) {
	//�̵߳ȴ�ʱ��		
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
	
	// �������ӳ��е��������ӣ��ҵ����Ҫ���ص����Ӷ���
	for (int i= 0 ; i < connections.size() ; i++)
	{	
		pConn =connections.at(i);
        closeConnection(pConn->getConnection());
    }
    connections.clear();
	pthread_mutex_unlock(&mutex_connectionpool);
}

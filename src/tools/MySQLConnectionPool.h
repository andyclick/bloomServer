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
		
        sql::Connection * connection;// ���ݿ�����
		
		bool busy ; // �������Ƿ�����ʹ�õı�־��Ĭ��û������ʹ��
		
		// ���캯��������һ�� Connection ����һ�� PooledConnection ����
	public :
		PooledConnection(sql::Connection * connection) {
			this->connection = connection;
			busy = false;
		}
		
		// ���ش˶����е�����
        sql::Connection * getConnection() {
			return connection;
		}
		
		// ���ô˶���ģ�����
		void setConnection(sql::Connection *connection) {
			this->connection = connection;
			this->busy=false;
		}
		
		// ��ö��������Ƿ�æ
		bool isBusy() {
			return busy;
		}
		
		// ���ö������������æ
		void setBusy(bool busy) {
			this->busy = busy;
		}
	};	
private:
    sql::Driver *driver;	
	string dbUrl; // ���� URL
	
	string dbUsername ; // ���ݿ��û���
	
	string dbPassword ; // ���ݿ��û�����
	
	string testTable ; // ���������Ƿ���õĲ��Ա�����Ĭ��û�в��Ա�
	
    string dbname;
    int initialConnections ; // ���ӳصĳ�ʼ��С
	
	int incrementalConnections;// ���ӳ��Զ����ӵĴ�С
	
	int maxConnections ; // ���ӳ����Ĵ�С


	void *param;
	
	vector<PooledConnection *> connections; // ������ӳ������ݿ����ӵ����� , ��ʼʱΪ null
    static pthread_mutex_t mutex_connectionpool;
	
	void createConnections(int numConnections);//Ҫ���������ݿ����ӵ���Ŀ
    sql::Connection *newConnection();//����һ�����ݿ�����
    sql::Connection *getFreeConnection(); //���������ӳ����� connections �з���һ�����õĵ����ݿ�����
    sql::Connection *findFreeConnection(); //�������ӳ������е����ӣ�����һ�����õ����ݿ�����
	bool testConnection(sql::Connection * conn);//����һ�������Ƿ���ã���������ã��ص��������� false
	void closeConnection(sql::Connection *conn);//�ر�һ�����ݿ�����
	void wait(int mSeconds) ;//ʹ����ȴ������ĺ�����
	
    void Destroy();
public:
	MySQLConnectionPool(string dbUrl, string dbUsername, string dbPassword ,
							   int initialConnections , int incrementalConnections, 
							   int maxConnections,string dbservername); 

	~MySQLConnectionPool();
	int getInitialConnections() ;//��ʼ���ӳ��пɻ�õ���������
	void setInitialConnections(int initialConnections) ;//�������ӳصĳ�ʼ��С
	int getIncrementalConnections() ;//�������ӳ��Զ����ӵĴ�С ��
	void setIncrementalConnections(int incrementalConnections);//�������ӳ��Զ����ӵĴ�С
	int getMaxConnections() ;//�������ӳ������Ŀ�����������
	string getTestTable();//��ȡ���ݿ���
	void  setTestTable(string testTable);//��ȡ�������	
	//ͬ������
	 void createPool() ; // ����һ�����ݿ����ӳأ����ӳ��еĿ������ӵ������������Ա???
     sql::Connection * getConnection() ;//����һ�����õ����ݿ����Ӷ���

	void returnConnection(sql::Connection * conn) ;//�˺�������һ�����ݿ����ӵ����ӳ��У����Ѵ�������Ϊ���С�
	//ͬ������
	void refreshConnections() ;//ˢ�����ӳ������е����Ӷ���
	void closeConnectionPool() ;//�ر����ӳ������е����ӣ���������ӳ�

};
#endif //end __MYSQLCONNECTION_POOL_H_

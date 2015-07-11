#include "BloomfilterConnectionPool.h"

pthread_mutex_t BloomfilterConnectionPool::mutex_connectionpool = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t BloomfilterConnectionPool::mutex_closepool = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t BloomfilterConnectionPool::mutex_closepool2 = PTHREAD_MUTEX_INITIALIZER;

BloomfilterConnectionPool::BloomfilterConnectionPool(uint16_t initialConnections)
{
    this->initialConnections = initialConnections;
    this->maxConnections = 60000;
    this->isClosing = 0;
    this->connections.clear();
    return;
}

void BloomfilterConnectionPool::setInitialConnections(uint16_t initialConnections)
{
    this->initialConnections = initialConnections;
}
uint16_t BloomfilterConnectionPool::getInitialConnections()
{
    return initialConnections;
}
uint16_t BloomfilterConnectionPool::createPool(char* ip, int32_t port)
{
    pthread_mutex_lock(&mutex_connectionpool);
    if (connections.empty())
    {
        try
        {
            createConnections(initialConnections, ip, port);
        }
        catch(...)
        {
            pthread_mutex_unlock(&mutex_connectionpool);
            return -1;
        }
    }
    pthread_mutex_unlock(&mutex_connectionpool);
    return 0;
}

uint16_t BloomfilterConnectionPool::createConnections(uint16_t numConnections, char* ip, int32_t port)
{
    for(int i = 0; i < numConnections; i++)
    {
        if(maxConnections > 0 && connections.size() >= maxConnections)
        {
            break;
        }
        ServClient* connection;
        try{
            connection = newConnection(ip, port);
        }
        catch(...)
        {
            connection = NULL;
            throw;
        }
        if (connection != NULL)
        {
            PooledConnection* poolconn = new PooledConnection(connection);
            connections.push_back(poolconn);
        }

    }
    return 0;
}

ServClient* BloomfilterConnectionPool::newConnection(char* ip, int32_t port)
{
    socket = boost::shared_ptr<TSocket>(new TSocket(ip, port));
    transport = boost::shared_ptr<TTransport>(new TFramedTransport(socket));
    protocol = boost::shared_ptr<TProtocol> (new TBinaryProtocol(transport));
    socket->setConnTimeout(500*1000*1000);
    socket->setRecvTimeout(500*1000*1000);
    socket->setSendTimeout(500*1000*1000);
    try
    {
        transport->open();
    }
    catch(...)
    {
        throw;
    }
    ServClient* connection = new ServClient(protocol);
    return connection;
}

void BloomfilterConnectionPool::wait(int mSeconds)
{
    sleep(mSeconds);
}

ServClient* BloomfilterConnectionPool::getConnection()
{
    pthread_mutex_lock(&mutex_connectionpool);
    if (connections.empty()) {
        pthread_mutex_unlock(&mutex_connectionpool);
        return NULL; // 连接池还没创建，则返回 null
    }
    ServClient *conn = getFreeConnection(); // 获得一个可用的client连接

    // 如果目前没有可以使用的连接，即所有的连接都在使用中
    pthread_mutex_unlock(&mutex_connectionpool);

    while (conn == NULL) {
        // 等一会再试
        wait(5);
        pthread_mutex_lock(&mutex_connectionpool);
        conn = getFreeConnection(); // 重新再试，直到获得可用的连接，如果
        pthread_mutex_unlock(&mutex_connectionpool);
    }
    return conn;// 返回获得的可用的连接
}

ServClient* BloomfilterConnectionPool::getFreeConnection()
{
    ServClient * conn = findFreeConnection();
    /*if (conn == NULL)
      {
      createConnections(incrementalConnections);
      conn = findFreeConnection();
      if (conn == NULL)
      {
      return NULL;
      }
      }*/
    return conn;
}

ServClient* BloomfilterConnectionPool::findFreeConnection()
{
    ServClient * conn = NULL;
    PooledConnection *pConn = NULL;
    for(int i = 0 ;i<connections.size(); i++)
    {
        pConn = connections.at(i);
        if (!pConn->isBusy())
        {
            conn = pConn->getConnection();
            pConn->setBusy(true);
            break;
        }
    }

    return conn;
}
void BloomfilterConnectionPool::setIncrementalConnections(int incrementalConnections)
{
    this->incrementalConnections = incrementalConnections;
}
void BloomfilterConnectionPool::returnConnection(ServClient * conn)
{
    pthread_mutex_lock(&mutex_connectionpool);
    if (connections.empty())
    {
        pthread_mutex_unlock(&mutex_connectionpool);
        return;
    }

    PooledConnection* pConn = NULL;
    for (int i = 0; i < connections.size(); i++)
    {
        pConn = connections.at(i);

        if (conn == pConn->getConnection())
        {
            pConn->setBusy(false);
            break;
        }
    }
    pthread_mutex_unlock(&mutex_connectionpool);
}

void BloomfilterConnectionPool::closeConnectionPool()
{
#if 0
    pthread_mutex_lock(&mutex_closepool);
    if (isClosing == 1)
    {
        while (isClosing == 1)
        {
            pthread_mutex_unlock(&mutex_closepool);
            //sleep(5);
            pthread_mutex_lock(&mutex_closepool);
        }
        pthread_mutex_unlock(&mutex_closepool);
        return;
    }
    isClosing = 1;
    pthread_mutex_unlock(&mutex_closepool);
#endif
    pthread_mutex_lock(&mutex_connectionpool);
    if (connections.empty())
    {
        pthread_mutex_unlock(&mutex_connectionpool);
#if 0
        pthread_mutex_lock(&mutex_closepool);
        isClosing = 0;
        pthread_mutex_unlock(&mutex_closepool);
#endif
        return;
    }
    PooledConnection* pConn = NULL;

    int ret = connections.size();
    for (int i = 0; i < connections.size(); i++)
    {
        pConn = connections.at(i);
#if 0
        while (pConn->isBusy())
        {
            // 等50秒时间有点长，是为了防止dump时，阻塞了client的请求。找时间优化一下对这个地方的处理。
            //wait(5);
        }
#endif
        //delete pConn->getConnection();
        if (!pConn->isBusy())
        {
            delete pConn->getConnection();
            delete pConn;
        }
        else
        {
            delete pConn;
        }
        //        connections.erase(connections.begin()+i);
    }
    connections.clear();
    pthread_mutex_unlock(&mutex_connectionpool);
    //printf("close executed unlock\n");
#if 0
    pthread_mutex_lock(&mutex_closepool);
    isClosing = 0;
    pthread_mutex_unlock(&mutex_closepool);
#endif
}

void BloomfilterConnectionPool::recycleConnectionPool()
{
    pthread_mutex_lock(&mutex_connectionpool);
    if (connections.empty())
    {
        pthread_mutex_unlock(&mutex_connectionpool);
        return;
    }
    PooledConnection* pConn = NULL;
    for (int i = 0; i < connections.size(); i++)
    {
        pConn = connections.at(i);
        if (!pConn->isBusy())
        {
            delete pConn->getConnection();
            delete pConn;
        }
        else
        {
            delete pConn;
        }
    }
    connections.clear();
    pthread_mutex_unlock(&mutex_connectionpool);
    return;
}

#ifndef __BloomfilterConnectionPool_h__
#define __BloomfilterConnectionPool_h__

#include "Serv.h"
#include <transport/TSocket.h>  
#include <transport/TBufferTransports.h>  
#include <protocol/TBinaryProtocol.h>  

using namespace apache::thrift;  
using namespace apache::thrift::protocol;  
using namespace apache::thrift::transport;  
using boost::shared_ptr;

class BloomfilterConnectionPool
{
public:
    class PooledConnection
    {
        ServClient* connection;
        bool busy;
    public:
        PooledConnection(ServClient* connection)
        {
            this->connection = connection;
            busy = false;
        }
        
        bool isBusy()
        {
            return busy;
        }

        void setBusy(bool busy)
        {
            this->busy = busy;
        }

        ServClient* getConnection()
        {
            return connection;
        }
    };
    BloomfilterConnectionPool(uint16_t initialConnections);
    void setInitialConnections(uint16_t initialConnections);
    uint16_t getInitialConnections();
    uint16_t createPool(char* ip, int32_t port);
    void setIncrementalConnections(int incrementalConnections);
    void returnConnection(ServClient * conn);
    void closeConnectionPool();
    void recycleConnectionPool();
    ServClient* getConnection();
private:
    uint16_t initialConnections;
    uint16_t incrementalConnections;
    uint16_t maxConnections;
    uint16_t isClosing;

    std::vector<PooledConnection*> connections;
    static pthread_mutex_t mutex_connectionpool;
    static pthread_mutex_t mutex_closepool;
    static pthread_mutex_t mutex_closepool2;

    uint16_t createConnections(uint16_t numConnections, char* ip, int32_t port);
    boost::shared_ptr<TSocket> socket;  
    boost::shared_ptr<TTransport> transport;
    boost::shared_ptr<TProtocol> protocol;  
    ServClient* newConnection(char* ip, int32_t port);
    ServClient* getFreeConnection();
    void wait(int mSeconds);
    ServClient* findFreeConnection();

};
#endif

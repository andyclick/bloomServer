#ifndef __SERVICE_DBCONNECTIONMANAGER_H_
#define __SERVICE_DBCONNECTIONMANAGER_H_
class DbConnectionManager{

public:

    virtual ~DbConnectionManager() {};
    virtual void * get_connection() = 0;
    virtual int release_connection(void * pConnect) = 0;

    virtual int rollback(void * pConnect) = 0;
    virtual int commit(void * pConnect) = 0;
    virtual int end_transaction(void * pConnect) = 0;
    virtual int begin_transaction(void * pConnect) = 0;
private :
    virtual int open_connection_pool() = 0;
    virtual int close_connection_pool() = 0;
};
#endif

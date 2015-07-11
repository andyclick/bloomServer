#ifndef _MODULE_THRIFTMANAGER_H_
#define _MODULE_THRIFTMANAGER_H_
#include <iostream>
#include <stdint.h>
#include "Manager.h"
/*#include "thrift_filter/Serv.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>*/
#include "ic_types.h"

class ThriftManager:virtual public Manager
{
public:
    ThriftManager(LogGlobalCtrl * pLogGlobalCtrl);
    int start();
    void retrieve();
    void dump();
    void *start_thrift_server();
private:
    LogGlobalCtrl *m_pLogGlobalCtrl;
};
#endif

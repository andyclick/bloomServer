#ifndef _MODULE_LOGSTATISTICMANAGER_H_
#define _MODULE_LOGSTATISTICMANAGER_H_

#include "Manager.h"
#include "util.h"
#include "ic_types.h"
class LogStatisticManager: virtual public Manager {
public:
    LogStatisticManager(LogGlobalCtrl * pLogGlobalCtrl);
    ~LogStatisticManager();
    virtual int start();
    virtual int stop();

    void dump(); 
    void retrieve(); 
private:
    LogGlobalCtrl       *m_pLogGlobalCtrl;
};

#endif

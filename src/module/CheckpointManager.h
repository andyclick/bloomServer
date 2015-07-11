#ifndef _MODULE_CHECKPOINTMANAGER_H_
#define _MODULE_CHECKPOINTMANAGER_H_

#include "Manager.h"
#include "util.h"
#include <pthread.h>
#include"ic_types.h"

#define FILE_MAX_ID "maxid.dat"
#define FILE_TASKS "tasks.dat"
#define FILE_KEYWORDS       "keywords.dat"

class CheckpointManager:virtual public Manager {
public:
    CheckpointManager(LogGlobalCtrl * pLogGlobalCtrl);
    ~CheckpointManager();
    virtual int start();
    virtual int stop();

    ulonglong getMaxUrlId(); 
    ulonglong GetMaxID(unsigned long pcid);
    void dump(); 
    void retrieve(); 
private:
    int retrieveMaxUrlId(); 
    int saveMaxUrlId(); 
    ulonglong maxurlid;
    static pthread_mutex_t mutexMaxid;
    int startCheckpointThread(); 
    static void *startCheckpoint(void *manager); 
    static void *checkpointThread(void *manager); 
    int checkpoint(); 
    int recover(); 


    static pthread_mutex_t mutex_checkpointthread;
    static pthread_mutex_t mux;
    static pthread_cond_t checkpiontthread_wait_cond;
    LogGlobalCtrl       *m_pLogGlobalCtrl;
};
#endif

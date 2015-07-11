#ifndef _MODULE_TASKSCHEDULEMANAGER_H_
#define _MODULE_TASKSCHEDULEMANAGER_H_

#include <vector>
#include <list>
#include <queue>
#include <map>
#include "Manager.h"
#include "ic_types.h"

using namespace std;
typedef multimap<int ,KeyWord *>::iterator Miter;
typedef pair< Miter,Miter > Tpair;
class TaskScheduleManager:virtual public Manager {
public:
    TaskScheduleManager(LogGlobalCtrl * pLogGlobalCtrl);
    ~TaskScheduleManager();
    virtual int start();
    virtual int stop();
    int push(Task *task); 
    Task *pop(); 
    Task *getNextTask(time_t *nexttime); 
    int putTask(Task *task, int taskfromdb = TASK_UNFROM_DB); 
    int remove(int); 
    int removeTaskFromPriorityQueue(int); 

    int getTaskIdByUrl(char *url); 
    Task *getTask(int id); 
    Task *getTaskClone(int id); 

    void increaseFetchNum(Task *task); 
    void increaseTaskSavefetchedNum(int taskid);
    void dump(); 
    void retrieve(); 
    
    TaskOtherInfo *getTaskOtherInfo(int id, bool *exists = NULL, bool needmutex = false);
    int combineTaskOtherInfo(TaskOtherInfo *des, TaskOtherInfo *src);
    int putTaskOtherInfo(TaskOtherInfo * taskotherinfo); 
    TaskOtherInfo *poptaskother(int taskid, bool needmutex = false);
    bool increaseTaskUrlNum(int taskid);
    bool InsertKeywordMap(KeyWord * keyword);

    void increaseTaskInsUrl(int taskid); 
    int decreaseTaskUrlNum(Task *task, int taskbatch);
    void increaseTaskErrorUrlNum(int taskid);
    char *getCookieFromTask(int taskid);
    
    bool caninserttask() {return canInsertTask;}
    void startinserttask() {canInsertTask = true;}
    void stopinserttask() {canInsertTask= false;}
    int keywordmapsize();
private:
    LogGlobalCtrl       *m_pLogGlobalCtrl;
    bool canInsertTask;
    priority_queue<Task *, vector<Task *>, TaskLesser> tasks;
    map<int, Task *> tasksmap;
    map<int, Task *> taskallmap;
    map<int, TaskOtherInfo *> tasksothermap;
    multimap<int , KeyWord *> keywordsmap;
    
    static pthread_rwlock_t rwlock_keywordmaps;

    static pthread_rwlock_t rwlock_priority_queue;
    static pthread_rwlock_t rwlock_map;
    static pthread_rwlock_t rwlock_mapother;
    static pthread_rwlock_t rwlock_task_running;
    static pthread_rwlock_t rwlock_taskurl;

    static pthread_mutex_t mutex_gettaskthread;
    static pthread_cond_t gettaskthread_wait_cond;

    static pthread_mutex_t delete_taskcontent_wait_mutex;
    static pthread_cond_t delete_taskcontent_wait_cond;
    
    static pthread_mutex_t stop_mutex;
    static pthread_cond_t stop_cond;

    static void *getTaskThread(void *manager); 
    static void *startGetTask(void *manager); 
        
    static void * startDeleteTaskContent(void *manager);
    static void * DeleteTaskContentThread(void *manager);
    
    int startGetTaskThread(); 
    int startDeleteAlreadReadThread();
    int recalculateTime(Task *task); 
    int combineTask(Task *des, Task *src); 
    void checkTaskUrlNum(int taskid , int urlnum); 
    string  checkFinalPageFormat(string url);
    void deleteTaskOther(Task * task, int taskbatch);
    void deleteTaskContentkThread();
    void DelKeywordMap();
};
#endif

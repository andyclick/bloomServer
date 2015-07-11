#include "TaskScheduleManager.h"
#include "CheckpointManager.h"
#include "InfoCrawler.h"
#include "FSBigFile.h"
#include "utilcpp.h"

pthread_rwlock_t TaskScheduleManager::rwlock_priority_queue = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t TaskScheduleManager::rwlock_map = PTHREAD_RWLOCK_INITIALIZER;

pthread_rwlock_t TaskScheduleManager::rwlock_keywordmaps= PTHREAD_RWLOCK_INITIALIZER;

pthread_rwlock_t TaskScheduleManager::rwlock_mapother = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t TaskScheduleManager::rwlock_task_running= PTHREAD_RWLOCK_INITIALIZER;

pthread_rwlock_t TaskScheduleManager::rwlock_taskurl= PTHREAD_RWLOCK_INITIALIZER;

pthread_mutex_t TaskScheduleManager::mutex_gettaskthread = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t TaskScheduleManager::gettaskthread_wait_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t TaskScheduleManager::stop_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t TaskScheduleManager::stop_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t TaskScheduleManager::delete_taskcontent_wait_mutex= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t TaskScheduleManager::delete_taskcontent_wait_cond= PTHREAD_COND_INITIALIZER;

TaskScheduleManager::TaskScheduleManager(LogGlobalCtrl * pLogGlobalCtrl) {

    m_pLogGlobalCtrl = pLogGlobalCtrl;
}

TaskScheduleManager::~TaskScheduleManager() {
    DelKeywordMap();
}

int TaskScheduleManager::start() {
    Manager::start();
    startinserttask();
    startGetTaskThread();
    startDeleteAlreadReadThread();
    mylog_info(m_pLogGlobalCtrl->infolog, "startGetTask thread is start - %s:%s:%d",INFO_LOG_SUFFIX);
    return 0;

}

int TaskScheduleManager::stop() {
    int ret = Manager::stop();
    while(!canstoped()) {
        pthread_cond_signal(&gettaskthread_wait_cond);
        pthread_cond_signal(&delete_taskcontent_wait_cond);
        my_sleep(500000); //0.5 seconds
    }
    return ret;
}

int TaskScheduleManager::push(Task *task) {
    pthread_rwlock_wrlock(&rwlock_priority_queue);
    tasks.push(task);
    pthread_rwlock_unlock(&rwlock_priority_queue);
    return 0;
}

Task *TaskScheduleManager::pop() {
    Task *task = NULL;
    pthread_rwlock_wrlock(&rwlock_priority_queue);
    task = tasks.top();
    tasks.pop();
    pthread_rwlock_unlock(&rwlock_priority_queue);

    if (task) {
        map<int, Task *>::iterator iter;
        pthread_rwlock_wrlock(&rwlock_map);
        if ((iter = tasksmap.find(task->id)) != tasksmap.end()) {
            tasksmap.erase(task->id); 
            mylog_info(m_pLogGlobalCtrl->infolog, "tasksmap erase %d - %s:%s:%d", task->id,INFO_LOG_SUFFIX);
        }
        pthread_rwlock_unlock(&rwlock_map);
    }
    return task;
}

Task *TaskScheduleManager::getNextTask(time_t *nexttime) {
    Task *task = NULL;
    pthread_rwlock_wrlock(&rwlock_priority_queue);
    if (tasks.size() > 0) {
        mylog_info(m_pLogGlobalCtrl->infolog, "tasks's size = %d for getNextTask - %s:%s:%d", tasks.size(),INFO_LOG_SUFFIX);
        task = tasks.top();
    } else {
        mylog_info(m_pLogGlobalCtrl->infolog, "tasks is null for getNextTask - %s:%s:%d", INFO_LOG_SUFFIX);
    }
    pthread_rwlock_unlock(&rwlock_priority_queue);
    if (task && task->sourcestate[0] == '1') {
        if (task->timeToFetch <= time(NULL)) {
            mylog_info(m_pLogGlobalCtrl->infolog, "get task successfully timeToFetch = %d now = %d taskid = %d for getNextTask - %s:%s:%d",task->timeToFetch, time(NULL), task->id , INFO_LOG_SUFFIX);
            *nexttime = 1;
            if (task->intervalauto[0] == '1')
            {
                recalculateTime(task);
            }
            int ratio = 1;
            if (task->sourcetype == SOURCE_TYPE_BBS ||
                //task->sourcetype == SOURCE_TYPE_BLOG ||
                task->sourcetype == SOURCE_TYPE_SEARCHENGINE ||
                task->sourcetype == SOURCE_TYPE_TOPIC
               ) {

                struct tm o;
                time_t t = time(NULL);
                struct tm *tmpm = localtime_r(&t, &o);

                if (tmpm->tm_hour >= 0 && tmpm->tm_hour < 6) {
                    ratio = 4;
                    mylog_info(m_pLogGlobalCtrl->infolog, "Task %d sourcetype %d ration is %d - %s:%s:%d",task->id, task->sourcetype, ratio,INFO_LOG_SUFFIX);
                }
            } else { //news and blog
                /*struct tm o;
                time_t t = time(NULL);
                struct tm *m = localtime_r(&t, &o);

                if (m->tm_hour >= 0 && m->tm_hour < 4) {
                    ratio = 4;
                    Log("Task %d %d ration is %d \n", task->id, task->sourcetype, ratio);
                }*/
                //do nothing
            }
            task->timeToFetch = time(NULL) + task->currentInterval * ratio;
            task->lastFetchNum = 0;
            putTask(task);
            return task;
        } else {
            mylog_info(m_pLogGlobalCtrl->infolog, "get task unsuccessful timeToFetch = %d now = %d taskid = %d for getNextTask - %s:%s:%d",task->timeToFetch, time(NULL), task->id, INFO_LOG_SUFFIX);
            //*nexttime  = task->timeToFetch - time(NULL);
            *nexttime  = 10;
            return NULL;
        }
    } else {
        if (task ) {
            mylog_info(m_pLogGlobalCtrl->infolog, "as task %d sourcestate = %d ,don't support, remove it from tasks in  getNextTask - %s:%s:%d", task->id, task->sourcestate[0],INFO_LOG_SUFFIX);
            Task *task = this->pop();
            if (task->intervalauto[0] == '1')
            {
                recalculateTime(task);
            }
            task->timeToFetch = time(NULL) + task->currentInterval;
            task->lastFetchNum = 0;
            //putTask(task);
            //delete task;
            *nexttime = 1;
        }else{
            *nexttime = 10;
        }
        return NULL;
    }
}

TaskOtherInfo *TaskScheduleManager::poptaskother(int taskid, bool needmutex) {
    TaskOtherInfo *taskotherinfo= NULL;
    map<int, TaskOtherInfo*>::iterator iter;
    pthread_rwlock_wrlock(&rwlock_mapother);
    if ((iter = tasksothermap.find(taskid)) != tasksothermap.end()) {
        taskotherinfo = iter->second;
        if (needmutex) {
            if (taskotherinfo->alreadyused) {
                mylog_info(m_pLogGlobalCtrl->infolog, "tasksothermap can not erase %d already used - %s:%s:%d", taskid,INFO_LOG_SUFFIX);
                pthread_rwlock_unlock(&rwlock_mapother);
                return NULL;
            } else {
                taskotherinfo->alreadyused = true;
            }
        }
        tasksothermap.erase(taskid);
        mylog_info(m_pLogGlobalCtrl->infolog, "tasksothermap erase %d - %s:%s:%d", taskid,INFO_LOG_SUFFIX);
    } else
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can't find taskOther %d - %s:%s:%d", taskid,INFO_LOG_SUFFIX);
    }
    pthread_rwlock_unlock(&rwlock_mapother);

    return taskotherinfo;
}
int TaskScheduleManager::putTaskOtherInfo(TaskOtherInfo * taskotherinfo) {
    map<int, TaskOtherInfo*>::iterator iter;
    pthread_rwlock_wrlock(&rwlock_mapother);
    int ret = 0;
    if ((iter = tasksothermap.find(taskotherinfo->id)) == tasksothermap.end()) {
        tasksothermap[taskotherinfo->id] = taskotherinfo;
        ret =1;     
        mylog_info(m_pLogGlobalCtrl->infolog, "putTaskOtherInfo tasksothermap erase %d - %s:%s:%d", taskotherinfo->id,INFO_LOG_SUFFIX);
    } else {
        ret = 0;
        mylog_info(m_pLogGlobalCtrl->infolog, "putTaskOtherInfo taskother already exist %d - %s:%s:%d", taskotherinfo->id,INFO_LOG_SUFFIX);
    }
    pthread_rwlock_unlock(&rwlock_mapother);
    return ret;
}

int TaskScheduleManager::combineTaskOtherInfo(TaskOtherInfo *des, TaskOtherInfo *src) {
    des->fetchingcookie = src->fetchingcookie;
    des->insertcookie(src->taskcookie , src->taskcookielen);
}
int TaskScheduleManager::putTask(Task *task, int taskfromdb) {
    map<int, Task *>::iterator iter;
    pthread_rwlock_wrlock(&rwlock_map);
    int ret = 0;
    if ((iter = tasksmap.find(task->id)) == tasksmap.end()) {
       if (task->sourcestate[0] == '1')
       {
            tasksmap[task->id] = task;
            this->push(task);
            mylog_info(m_pLogGlobalCtrl->infolog, "putTask %d - %s:%s:%d", task->id ,INFO_LOG_SUFFIX);
            ret = 1;
       }else
       {
           mylog_info(m_pLogGlobalCtrl->infolog, "putTask %d can't pub state %s - %s:%s:%d", task->id,task->sourcestate,INFO_LOG_SUFFIX);
           ret = 0;
       }
    }else
    {
        if (taskfromdb == TASK_FROM_DB)
        {
            combineTask(iter->second, task);
            if (task->timeToFetch == 1)
            {
                removeTaskFromPriorityQueue(task->id);
                this->push(iter->second);
                mylog_info(m_pLogGlobalCtrl->infolog, "tmptasks id %d, timetofetch %d - %s:%s:%d", iter->second->id, iter->second->timeToFetch,              INFO_LOG_SUFFIX);
            }
        }else
        {
            removeTaskFromPriorityQueue(task->id);
            this->push(iter->second);
            mylog_info(m_pLogGlobalCtrl->infolog, "tmptasks id %d, timetofetch %d - %s:%s:%d", iter->second->id, iter->second->timeToFetch,INFO_LOG_SUFFIX);
        }
        ret = 0;
    }
    pthread_rwlock_unlock(&rwlock_map);
    return ret;
}

int TaskScheduleManager::combineTask(Task *des, Task *src) {
    strcpy(des->name, src->name);
    strcpy(des->industry, src->industry);
    strcpy(des->homeurl, src->homeurl);
    strcpy(des->ultimatepageurl, src->ultimatepageurl);
    strcpy(des->bbsultimatepageurl, src->bbsultimatepageurl);
    strcpy(des->loginurl, src->loginurl);
    des->sourcestate[0] = src->sourcestate[0];
    des->sourcestate[1] = src->sourcestate[1];
    des->interval =  src->interval;
    des->currentInterval = src->interval;
    des->sourcetype =  src->sourcetype;
    strcpy(des->pageurl, src->pageurl);
    strcpy(des->ignoreurl, src->ignoreurl);
    des->timeToFetch=  src->timeToFetch;
    des->depth =  src->depth;
    des->tasksendtype = src->tasksendtype;
    des->intervalauto[0] = src->intervalauto[0];
}

int TaskScheduleManager::remove(int id) {
    map<int, Task *>::iterator iter;
    pthread_rwlock_wrlock(&rwlock_map);
    if ((iter = tasksmap.find(id)) == tasksmap.end()) {
        tasksmap.erase(id); 
        pthread_rwlock_unlock(&rwlock_map);
        removeTaskFromPriorityQueue(id);
    } else {
        pthread_rwlock_unlock(&rwlock_map);
    }
    return 0;
}

int TaskScheduleManager::removeTaskFromPriorityQueue(int id) {
    vector<Task *> tmptasks;
    pthread_rwlock_wrlock(&rwlock_priority_queue);
    Task * task = tasks.top();
    if (task->id == id) 
    {
        tasks.pop();
        pthread_rwlock_unlock(&rwlock_priority_queue);
    }else
    {
        while(!tasks.empty()) {
            Task *t = tasks.top();
            tasks.pop();
            if (t->id != id) {
                tmptasks.push_back(t);
            }
        }
        pthread_rwlock_unlock(&rwlock_priority_queue);

        for(int i = 0;i < tmptasks.size(); i++) {
            this->push(tmptasks[i]);
        }
    }
    return 0;
}
Task *TaskScheduleManager::getTaskClone(int id) {
    map<int, Task *>::iterator iter;
    Task *task = NULL;
    pthread_rwlock_rdlock(&rwlock_map);
    if ((iter = tasksmap.find(id)) != tasksmap.end()) {
        Task * task = new Task;
        task->clone(task,iter->second);
    }
    pthread_rwlock_unlock(&rwlock_map);
    return task;
}

TaskOtherInfo *TaskScheduleManager::getTaskOtherInfo(int id, bool *exists, bool needmutex) {
    map<int, TaskOtherInfo*>::iterator iter;
    TaskOtherInfo *taskother = NULL;
    pthread_rwlock_wrlock(&rwlock_mapother);
    if ((iter = tasksothermap.find(id)) != tasksothermap.end()) {
        taskother = iter->second;
        if (exists) *exists = true;
        if (needmutex) {
            if (taskother->alreadyused) {
                taskother = NULL;
                mylog_info(m_pLogGlobalCtrl->infolog, "can't get taskid %d inused - %s:%s:%d", id,INFO_LOG_SUFFIX);
            } else {
                taskother->alreadyused = true ;
            }
        }
    } else {
        if (exists) *exists = false;
        mylog_info(m_pLogGlobalCtrl->infolog, "can't find taskid %d - %s:%s:%d", id,INFO_LOG_SUFFIX);
    }
    pthread_rwlock_unlock(&rwlock_mapother);
    return taskother;
}

Task *TaskScheduleManager::getTask(int id) {
    map<int, Task *>::iterator iter;
    Task *task = NULL;
    pthread_rwlock_rdlock(&rwlock_map);
    if ((iter = tasksmap.find(id)) != tasksmap.end()) {
        task = iter->second;
    } else {
        mylog_info(m_pLogGlobalCtrl->infolog, "can't find task %d - %s:%s:%d", id,INFO_LOG_SUFFIX);
    }
    pthread_rwlock_unlock(&rwlock_map);
    return task;
}

int TaskScheduleManager::getTaskIdByUrl(char *url) {
    UrlAnalyseManager *urlAnalyseManager = InfoCrawler::getInstance()->getUrlAnalyseManager();
    map<int, Task *>::iterator iter;
    Task *task = NULL;
    pthread_rwlock_rdlock(&rwlock_map);
    for(iter = tasksmap.begin(); iter != tasksmap.end(); ++iter) {
        task = iter->second;
        if (urlAnalyseManager->getType(url, task) != URL_TYPE_NOTYPE) {
            pthread_rwlock_unlock(&rwlock_map);
            return task->id;
        }
    }
    pthread_rwlock_unlock(&rwlock_map);

    mylog_info(m_pLogGlobalCtrl->infolog, "can't find task by url %s - %s:%s:%d", url, INFO_LOG_SUFFIX);
    return -1;
}

int TaskScheduleManager::recalculateTime(Task *task) {
    struct tm o;
    time_t t = time(NULL);
    struct tm *m = localtime_r(&t, &o);

    if (m->tm_hour > 17 || m->tm_hour < 9 || m->tm_wday == 0 || m->tm_wday == 6) {
        mylog_info(m_pLogGlobalCtrl->infolog, "no need to recalculatetime for TaskScheduleManager %d-%d-%d %d:%d:%d - %s:%s:%d", 1900 + m->tm_year, m->tm_mon + 1, m->tm_mday, m->tm_hour, m->tm_min, m->tm_sec ,INFO_LOG_SUFFIX);
        return 1;
    }

    if (task->lastFetchNum == 0) {
        task->currentInterval *= 2;
    } else {
        if (task->currentInterval > task->interval) {
            task->currentInterval /= 2;
        }
    }

    //for debug
/*    if (task->currentInterval > 600) { //interval should not be more than one week
        task->currentInterval = 600;
    }*/
    //
    if (task->currentInterval > 604800) { //interval should not be more than one week
        task->currentInterval = 604800;
    }
    //
    if (task->currentInterval < task->interval) { //interval should not be less than interval 
        task->currentInterval =  task->interval;
    }
    return 0;
}

void TaskScheduleManager::dump() {
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    char filename[256] ;filename[0] = 0;
    static vector<string> dirs;
    dirs.push_back(icconfig->checkpointpath);
    sprintf(filename, "%s/%s", icconfig->checkpointpath, FILE_TASKS);
    FSBigFile bigfile(filename, &dirs, 'w');
    map<int, Task *>::iterator iter;
    char key[16] ;key[0] = 0;
    pthread_rwlock_rdlock(&rwlock_map);
    pthread_rwlock_rdlock(&rwlock_priority_queue);
    for(iter = tasksmap.begin(); iter != tasksmap.end(); ++iter) {
        sprintf(key, "%d", iter->second->id);
        WriteContent(&bigfile, key, strlen(key), (char *)iter->second, sizeof(*(iter->second)));
    }
    pthread_rwlock_unlock(&rwlock_priority_queue);
    pthread_rwlock_unlock(&rwlock_map);
    
    int len = sizeof(KeyWord);
    sprintf(filename, "%s/%s", icconfig->checkpointpath, FILE_KEYWORDS);
    FILE *f = fopen(filename, "w+");
    if (f) {
        int num = keywordmapsize();
        size_t writelen = fwrite(&num, 1, 4, f);
        if (writelen != 4) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "can not write keyword file in checkpoint - %s:%s:%d:%d", ERROR_LOG_SUFFIX);
        }
        pthread_rwlock_rdlock(&rwlock_keywordmaps);
        for (Miter iter = keywordsmap.begin(); iter !=keywordsmap.end();iter++)
        {
            KeyWord *keyword = iter->second;
            mylog_error(m_pLogGlobalCtrl->errorlog, "dump keyword %d %d %s  - %s:%s:%d:%d",keyword->state, keyword->source_id,keyword->keyword,ERROR_LOG_SUFFIX);
            size_t writelen = fwrite(iter->second, 1, len, f);
            if (writelen != len) {
                mylog_error(m_pLogGlobalCtrl->errorlog, "can not write keyword file in checkpoint - %s:%s:%d:%d", ERROR_LOG_SUFFIX);
            }    
        }
        pthread_rwlock_unlock(&rwlock_keywordmaps);
        fclose(f);
    } else {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not open keyword file for write - %s:%s:%d:%d", ERROR_LOG_SUFFIX);
    }
}

void TaskScheduleManager::retrieve() {
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    static vector<string> dirs;
    dirs.push_back(icconfig->checkpointpath);
    char filename[256] ;filename[0] =  0;
   
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    map<int,SiteSourceInfo *> mySourceInfo;
    infocrawler->getdbmysql()->GetSourceInfo(NULL,mySourceInfo,true);
    map<int,SiteSourceInfo *>::iterator it=mySourceInfo.begin();
    for(it;it !=mySourceInfo.end();it++)
    {
        Task *task = new Task;
        //InfoCrawler::getInstance()->gethyper()->create_table(it->second->id);
        //InfoCrawler::getInstance()->gethyper()->create_table(1);  
        task->id = it->second->id;
        strcpy(task->name,it->second->name.c_str());
        strcpy(task->host, it->second->host.c_str());
        strcpy(task->homeurl,it->second->homeurl.c_str());
        strcpy(task->ultimatepageurl,it->second->ultimatepageurl.c_str());
        strcpy(task->pageurl,it->second->pageurl.c_str());
        strcpy(task->intxt,it->second->intxt.c_str());
        task->timeToFetch = time(NULL) + 1;
        task->interval =  60;
        task->currentInterval =  60;
        task->noaccessInterval = 2;//2 second
        task->sourcestate[0] = '1';
        task->depth = 5;
        task->sourcetype = SOURCE_TYPE_COMPANY;
        if (putTask(task) == 0)
         {
           delete task;
         }
        delete it->second;
        it->second=NULL;
    }
    mySourceInfo.clear();

    /*Task *task = new Task; 
    task->id = 9;
    strcpy(task->name, "alibabacompany");
    strcpy(task->host, "alibaba.com");
    strcpy(task->homeurl, "^(http://search\\.china\\.alibaba\\.com/company/company_search\\.htm\\?(.*))$\n^http://china\\.alibaba\\.com/gongsi/(.*)\\.html(.*)$");
   // strcpy(task->ultimatepageurl, "^http://[^ ]+\\.cn\\.alibaba\\.com/athena/contact/[^ ]+\\.html$\n^http://[^ ]+\\.cn\\.alibaba\\.com/athena/companyprofile/[^ ]+\\.html$\n^http://china\\.alibaba\\.com/member/bizcomm_profile_front_interface\\.htm\\?memberId=[^ ]+&iframe_delete=true$\n^http://china\\.alibaba\\.com/company/detail/intro/[^ ]+\\.html$\n^http://corp\\.china\\.alibaba\\.com/page/contactinfo\\.htm\\?memberId=[^ ]+$\n^http://china\\.alibaba\\.com/member/ajax/memberCardJson\\.htm\\?callback=jQuery&loginId=[^ ]+$\n^http://china\\.alibaba\\.com/company/detail/contact/[^ ]+\\.html$\n^http://club\\.china\\.alibaba\\.com/misc/memberajax/memberInfoForAjax\\.html\\?memberId=[^ ]+&iframe_delete=true$\n");
    //strcpy(task->ultimatepageurl, "^http://[^ ]+\\.cn\\.alibaba\\.com/athena/contact/[^ ]+\\.html$\n^http://[^ ]+\\.cn\\.alibaba\\.com/athena/companyprofile/[^ ]+\\.html$\n^http://china\\.alibaba\\.com/member/bizcomm_profile_front_interface\\.htm\\?memberId=[^ ]+&iframe_delete=true$\n^http://china\\.alibaba\\.com/company/detail/intro/[^ ]+\\.html$\n^http://china\\.alibaba\\.com/company/detail/contact/[^ ]+\\.html$\n^http://china\\.alibaba\\.com/member/ajax/memberCardJson\\.htm\\?callback=jQuery&loginId=[^ ]+$");
    //strcpy(task->ultimatepageurl, "^http://[^ ]+\\.cn\\.alibaba\\.com/athena/contact/[^ ]+\\.html$\n^http://[^ ]+\\.cn\\.alibaba\\.com/athena/companyprofile/[^ ]+\\.html$\n^http://[^ ]+\\. cn\\.alibaba\\.com/athena/bizreflist/[^-]+\\.html$");
    //strcpy(task->pageurl, "^http://[^ ]+\\.cn\\.alibaba.com(/)?$\n^http://china\\.alibaba\\.com/company/detail/[^ /]+\\.html$");
    strcpy(task->ultimatepageurl,"^http://www+\\.[^ ]+\\.qiyesou.com/introduce(/)?$\n^http://www+\\.[^ ]+\\.qiyesou.com/contact(/)?$");
    strcpy(task->pageurl,"^http://www+\\.[^ ]+\\.qiyesou.com(/)?$");
    task->timeToFetch = time(NULL) + 1;
    task->interval =  60;
    task->currentInterval =  60;
    task->noaccessInterval = 2;//2 second
    task->sourcestate[0] = '1';
    task->depth = 3;
    task->sourcetype = SOURCE_TYPE_COMPANY;
    if (putTask(task) == 0) {
	    delete task;
    }*/

/*task->id = 9;
    strcpy(task->name, "testsite1");
    strcpy(task->industry, "012");
    strcpy(task->homeurl, "http://club.chinaren.com/bbs/left2_m2.jsp?boardid=29&P=0");
    strcpy(task->ultimatepageurl, "http://club\\.chinaren\\.com/[0-9]+\\.html");
    strcpy(task->pageurl, "http://club\\.chinaren\\.com/bbs/left2_m2\\.jsp\\?boardid=29&P=[0-9]+");
    //strcpy(task->ultimatepageurl, "http://www\\.pcpop\\.com/doc/0/405/405368\\.shtml");
    //strcpy(task->ultimatepageurl, "http://book\\.sina\\.com\\.cn/excerpt/sz/mx/[0-9]+-[0-9]+-[0-9]+/1452255926\\.shtml");
    //strcpy(task->ultimatepageurl, "http://book\\.sina\\.com\\.cn/excerpt/sz/rw/1028255877\\.shtml");
    task->timeToFetch = time(NULL) + 1; 
    task->interval = 3 * 60;
    task->currentInterval = 3 * 60;
    task->sourcestate[0] = '2';
    task->depth = 3;
    task->sourcetype == SOURCE_TYPE_BBS;
    if (putTask(task) == 0) {
        delete task;
    }
*/
    /*task->id = 9;
    strcpy(task->name, "bbs_test");
    strcpy(task->industry, "012");
    strcpy(task->homeurl, "http://bbs.pcauto.com.cn/forum-16516.html");
    strcpy(task->ultimatepageurl, "http://bbs.pcauto.com.cn/topic-[^ ]+.html");
    strcpy(task->pageurl, "http://bbs.pcauto.com.cn/forum-16516.html");
    task->timeToFetch = time(NULL) + 1;
    task->interval = 3 * 60;
    task->currentInterval = 3 * 60;
    task->sourcestate[0] = '2';
    task->depth = 3;
    task->sourcetype = SOURCE_TYPE_BBS;
    if (putTask(task) == 0) {
        delete task;
    }
    
    task = new Task; 
    task->id = 10;
    strcpy(task->name, "blog_test");
    strcpy(task->industry, "012");
    strcpy(task->homeurl, "http://blog.sina.com.cn/");
    strcpy(task->ultimatepageurl, "http://blog.sina.com.cn/[^ ]+.html(\?tj=1)?");
    strcpy(task->pageurl, "http://blog.sina.com.cn/lm/21/2006/0427/4.html?tj=1\r\nhttp://blog.sina.com.cn/lm/21/2006/0705/12.html");
    task->timeToFetch = time(NULL) + 1;
    task->interval = 3 * 60;
    task->currentInterval = 3 * 60;
    task->sourcestate[0] = '2';
    task->depth = 3;
    task->sourcetype = SOURCE_TYPE_BLOG;
    if (putTask(task) == 0) {
        delete task;
    }
    
    task = new Task; 
    task->id = 11;
    strcpy(task->name, "search_test");
    strcpy(task->industry, "012");
    strcpy(task->homeurl, "http://www.baidu.com/s?wd=mp3");
    strcpy(task->ultimatepageurl, "");
    strcpy(task->pageurl, "");
    task->timeToFetch = time(NULL) + 1;
    task->interval = 3 * 60;
    task->currentInterval = 3 * 60;
    task->sourcestate[0] = '2';
    task->depth = 3;
    task->sourcetype = SOURCE_TYPE_SEARCHENGINE;
    if (putTask(task) == 0) {
        delete task;
    }

    task = new Task; 
    task->id = 8 ;
    strcpy(task->name, "topic_test");
    strcpy(task->industry, "012");
    strcpy(task->homeurl, "http://finance.sina.com.cn/zt/index.html");
    strcpy(task->ultimatepageurl, "");
    strcpy(task->pageurl, "");
    task->timeToFetch = time(NULL) + 1; 
    task->interval = 3 * 60;
    task->currentInterval = 3 * 60;
    task->sourcestate[0] = '2';
    task->depth = 3;
    task->sourcetype = SOURCE_TYPE_TOPIC;
    if (putTask(task) == 0) {
        delete task;
    }

    
    task = new Task; 
    task->id = 2;
    strcpy(task->name, "testsite3");
    strcpy(task->industry, "013");
    strcpy(task->homeurl, "http://www.ali213.net/");
    strcpy(task->ultimatepageurl, "http://www\\.ali213\\.net/News/html/[0-9]+/[0-9]+\\.html");
    //strcpy(task->ultimatepageurl, "http://book\\.sina\\.com\\.cn/excerpt/sz/mx/[0-9]+-[0-9]+-[0-9]+/1452255926\\.shtml");
    //strcpy(task->ultimatepageurl, "http://book\\.sina\\.com\\.cn/excerpt/sz/rw/1028255877\\.shtml");
    task->timeToFetch = time(NULL) + 1; 
    task->interval = 10 * 60;
    task->currentInterval = 3 * 60;
    task->sourcestate[0] = '2';
    if (putTask(task) == 0) {
        delete task;
    }

    task = new Task; 
    task->id = 3;
    strcpy(task->name, "testsite4");
    strcpy(task->industry, "013");
    strcpy(task->homeurl, "http://news.sohu.com/");
    strcpy(task->ultimatepageurl, "http://news\\.sohu\\.com/[0-9]+/n[0-9]+\\.shtml");
    //strcpy(task->ultimatepageurl, "http://book\\.sina\\.com\\.cn/excerpt/sz/mx/[0-9]+-[0-9]+-[0-9]+/1452255926\\.shtml");
    //strcpy(task->ultimatepageurl, "http://book\\.sina\\.com\\.cn/excerpt/sz/rw/1028255877\\.shtml");
    task->timeToFetch = time(NULL) + 1; 
    task->interval = 10 * 60;
    task->currentInterval = 3 * 60;
    task->sourcestate[0] = '2';
    if (putTask(task) == 0) {
        delete task;
    }

    task = new Task; 
    task->id = 4;
    strcpy(task->name, "testsite5");
    strcpy(task->industry, "013");
    strcpy(task->homeurl, "http://news.sina.com.cn/");
    strcpy(task->ultimatepageurl, "http://news\\.sina\\.com\\.cn/c/[0-9]{4}-[0-9]{2}-[0-9]{2}/[0-9]+\\.shtml");
    //strcpy(task->ultimatepageurl, "http://book\\.sina\\.com\\.cn/excerpt/sz/mx/[0-9]+-[0-9]+-[0-9]+/1452255926\\.shtml");
    //strcpy(task->ultimatepageurl, "http://book\\.sina\\.com\\.cn/excerpt/sz/rw/1028255877\\.shtml");
    task->timeToFetch = time(NULL) + 1; 
    task->interval = 10 * 60;
    task->currentInterval = 3 * 60;
    task->sourcestate[0] = '2';
    if (putTask(task) == 0) {
        delete task;
    }*/
    
}

void *TaskScheduleManager::DeleteTaskContentThread(void *manager ) {
    timespec mytime;
    TaskScheduleManager *m = static_cast<TaskScheduleManager *>(manager);
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    while(m->running()) {
        if (!(m->running()))  break; //return
        //mytime.tv_sec = time(NULL) + icconfig->dbinterval;
        mytime.tv_sec = time(NULL)+( 60);  //Wait for 300 second
        mytime.tv_nsec = 0;
        pthread_mutex_lock(&delete_taskcontent_wait_mutex);
        pthread_cond_timedwait(&delete_taskcontent_wait_cond, &delete_taskcontent_wait_mutex, (const struct timespec *)&mytime);
        pthread_mutex_unlock(&delete_taskcontent_wait_mutex);
        if (infocrawler->getLocalDbManager()->ReadDeleteAlreadyRead()) m->deleteTaskContentkThread();
    }
}

void TaskScheduleManager::deleteTaskContentkThread() {
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    list<char *> alreadread;                                                                                                     
    infocrawler->getLocalDbManager()->GetAlreadyRead(alreadread);                                                              
    list<char *>::iterator iter;                                                                                              

    for(iter = alreadread.begin(); iter != alreadread.end(); ++iter) {                                                              
        ulonglong id = 0;                                                                                                     
        int taskid = 0;                                                                                                       
        if (*iter) {                                                    
            sscanf(*iter, "%llu/%d", &id, &taskid);
            int rettmp = infocrawler->getLocalDbManager()->erasecontent(id ,taskid);
            free(*iter);                                                                                                      
        } else {                                                                                                              
            continue;                                                                                                         
        }
    }
}
int TaskScheduleManager::startGetTaskThread() {
    pthread_t *tid = (pthread_t*)malloc(1 * sizeof(pthread_t));
    if (tid == NULL) {
        mylog_fatal_errno(m_pLogGlobalCtrl->veryimportantlog, "malloc error for startGetTaskThread thread\n");
    }       

    if(pthread_create(tid, NULL, startGetTask, this)) {
        perror("create startGetTask threads error");
        mylog_fatal_errno(m_pLogGlobalCtrl->veryimportantlog, "create startGetTask threads error\n");
    }

    pthread_detach(*tid);
    free(tid);

    return 0;
}

int TaskScheduleManager::startDeleteAlreadReadThread() {
    pthread_t *tid = (pthread_t*)malloc(1 * sizeof(pthread_t));
    if (tid == NULL) {
        mylog_fatal_errno(m_pLogGlobalCtrl->veryimportantlog, "malloc error for startDeleteAlreadReadThread thread\n");
    }       

    if(pthread_create(tid, NULL, startDeleteTaskContent, this)) {
        mylog_fatal_errno(m_pLogGlobalCtrl->veryimportantlog, "create DeleteTaskContentkThread threads error\n");
    }

    pthread_detach(*tid);
    free(tid);

    return 0;
}
void *TaskScheduleManager::startDeleteTaskContent(void *manager)
{

    TaskScheduleManager *m = static_cast<TaskScheduleManager *>(manager);
    InfoCrawler *infocrawler = InfoCrawler::getInstance();

    pthread_t *tid = (pthread_t*)malloc(1 * sizeof(pthread_t));
    if (tid == NULL) {
        mylog_fatal_errno(m->m_pLogGlobalCtrl->veryimportantlog, "malloc error for startGetTask\n");
    }

    if(pthread_create(tid, NULL, DeleteTaskContentThread, manager)) {
        mylog_fatal_errno(m->m_pLogGlobalCtrl->veryimportantlog, "create DeleteTaskContentThread error\n");
    }

    pthread_join(*tid, NULL);

    free(tid);

    m->stoped();

    mylog_info(m->m_pLogGlobalCtrl->infolog, "startDeleteTaskContentk thread is over - %s:%s:%d",INFO_LOG_SUFFIX);
}
void *TaskScheduleManager::startGetTask(void *manager) {

    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    TaskScheduleManager *m = static_cast<TaskScheduleManager *>(manager);

    pthread_t *tid = (pthread_t*)malloc(1 * sizeof(pthread_t));
    if (tid == NULL) {
        mylog_fatal_errno(m->m_pLogGlobalCtrl->veryimportantlog, "malloc error for startGetTask\n");
    }

    if(pthread_create(tid, NULL, getTaskThread, manager)) {
        mylog_fatal_errno(m->m_pLogGlobalCtrl->veryimportantlog, "create getTaskThread error\n");
    }

    pthread_join(*tid, NULL);

    free(tid);

    m->stoped();

    mylog_info(m->m_pLogGlobalCtrl->infolog, "startGetTask thread is over - %s:%s:%d",INFO_LOG_SUFFIX);
}

void *TaskScheduleManager::getTaskThread(void *manager) {

    time_t nexttime = 1;
    TaskScheduleManager *m = static_cast<TaskScheduleManager *>(manager);
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    while(m->running()) {
        ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        if (nexttime == 10) {
            ts.tv_sec += 5;
        } else {
            ts.tv_nsec += nexttime;
        }

        pthread_mutex_lock(&mutex_gettaskthread);
        pthread_cond_timedwait(&gettaskthread_wait_cond, &mutex_gettaskthread, &ts);
        pthread_mutex_unlock(&mutex_gettaskthread);

        if (!(m->running()))  break; //return
        if (!m->canInsertTask) continue;
        
        Task *task = m->getNextTask(&nexttime);
        if (task) {
            
            int lastfetchnum = task->lastFetchNum;
//            infocrawler->getDbManager()->UpdateFetchTimeToSourceTable(task->id, task->currentInterval);
            int taskbatch = 0;
            bool taskotherexists = false;
            TaskOtherInfo *taskother = m->getTaskOtherInfo(task->id, &taskotherexists, true);
            if (taskother != NULL) {
                pthread_rwlock_wrlock(&rwlock_task_running);
                if(!taskother->running){
                    taskother->running = true;
                    pthread_rwlock_unlock(&rwlock_task_running);
                    mylog_info(m->m_pLogGlobalCtrl->infolog, "Task other != NULL and running = false taskid = %d - %s:%s:%d",task->id, INFO_LOG_SUFFIX);
//                    taskbatch =  infocrawler->getDbManager()->GetTaskBatch();
                    taskother->taskbatch = taskbatch;
//                    infocrawler->getDbManager()->WriteDownLoadBeginTime(task->id,taskbatch);
                } else {
                    taskother->fetchnum += lastfetchnum;
                    pthread_rwlock_unlock(&rwlock_task_running);
                    int reinsertintervaltmp = 0;
                    if (task->interval < icconfig->resinsertinterval)
                    {
                        reinsertintervaltmp = task->interval;
                    }else
                    {
                        reinsertintervaltmp = icconfig->resinsertinterval;
                    }
                    if (task->sourcetype == SOURCE_TYPE_BBS ||task->sourcetype == SOURCE_TYPE_COMPANY || time(NULL) - taskother->begintime < reinsertintervaltmp) {
                        mylog_info(m->m_pLogGlobalCtrl->infolog, "task is running ,increass fetchnum task %d fetchnum %d urlnum = %d interval %d  - %s:%s:%d", task->id,taskother->fetchnum, taskother->urlnum, reinsertintervaltmp, INFO_LOG_SUFFIX);
                        taskother->alreadyused = false;
                        continue;
                    } else {
                        mylog_info(m->m_pLogGlobalCtrl->infolog, "task is running reinsert begintime = %d current time = %d ,increass fetchnum task %d fetchnum %d urlnum = %d - %s:%s:%d", taskother->begintime, time(NULL), task->id,taskother->fetchnum, taskother->urlnum ,INFO_LOG_SUFFIX);
                        taskother->begintime = time(NULL);
                    }
                }
            } else if (!taskotherexists) {
                pthread_rwlock_wrlock(&rwlock_task_running);
                taskother  = new TaskOtherInfo(task->id);
                int ret = m->putTaskOtherInfo(taskother);
                if(!taskother->running){
                    taskother->running = true;
                    pthread_rwlock_unlock(&rwlock_task_running);
//                    taskbatch =  infocrawler->getDbManager()->GetTaskBatch();
                    taskother->taskbatch = taskbatch;
//                    infocrawler->getDbManager()->WriteDownLoadBeginTime(task->id,taskbatch);
                } else {
                    pthread_rwlock_unlock(&rwlock_task_running);
                }
            } else { //taskother do not exist
                pthread_rwlock_unlock(&rwlock_task_running);
                continue;
            }

            if ((task->sourcetype == SOURCE_TYPE_BBS) && (!task->ultimatepageurl[0])) {
             //read ultimatepageurl from mainserver
                char * recvbuf = NULL;
                int ret =infocrawler->getExternalManager()->ReadFinalPageFormat(task->id,task->homeurl,recvbuf); 
                if (ret >0 ){
                   if (recvbuf && recvbuf[0])
                   {
//                       string strultimatepageurl =  infocrawler->getDbManager()->checkPattern(recvbuf, task->homeurl);
//                       string strultimatepageurl1 = infocrawler->getDbManager()->checkFinalPageFormat(strultimatepageurl);
                       //sprintf(task->bbsultimatepageurl,"%s",(strultimatepageurl1.substr(0,MAX_URL_LEN_DATA_SOURCE-1)).c_str());
                   }
                   mylog_info(m->m_pLogGlobalCtrl->infolog, "GET FinalPageFormat TASK ID %d FinalPageFormat url %s homeurl%s - %s:%s:%d", task->id,task->bbsultimatepageurl,task->homeurl,INFO_LOG_SUFFIX);
                }else
                {
                    mylog_error(m->m_pLogGlobalCtrl->errorlog, "socket error task id %d - %s:%s:%d:%d", task->id,INFO_LOG_SUFFIX,ret);
                }
                if (recvbuf) delete [] recvbuf; 
            }            

            char *pToken = NULL;
            char *tokbuff = NULL;
            int len = strlen(task->homeurl);
            char *homeurltmp = new char[len + 1];
            homeurltmp[0] = 0;
            strncpy(homeurltmp,task->homeurl,len); 
            homeurltmp[len] = 0;
            pToken = strtok_r(homeurltmp, "\n\r\t ",&tokbuff);
            while(pToken)
            {
                pthread_rwlock_wrlock(&rwlock_keywordmaps);
                Tpair tpair = m->keywordsmap.equal_range(task->id);
                if (tpair.first != tpair.second)
                {
                    for (Miter iter = tpair.first; iter != tpair.second; ++iter)
                    {
                        char newhomeurl[4096];
                        newhomeurl[0] = 0;
                        char temp[1024];
                        temp[0] = 0;
                        urlInCode(iter->second->keyword, temp);
                        strcpy(newhomeurl, pToken);
                        szReplace(newhomeurl, "[keyword]", temp);
                        InfoCrawler::getInstance()->getUrlAnalyseManager()->insertHomePageUrl(newhomeurl, NULL, task, taskother->taskbatch);
                    }
                    pthread_rwlock_unlock(&rwlock_keywordmaps);
                }else
                {
                    pthread_rwlock_unlock(&rwlock_keywordmaps);
                    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
                    FILE *f = NULL;
                    //if (f= fopen(icconfig->alicompanyurl,"rb"))
                    if(f= fopen(task->intxt,"rb"))
                    {
                        char str[1024] = "";
                        while(fgets(str,1024,f))
                        {
                            char * newlen = strtrim(str,NULL);
                            InfoCrawler::getInstance()->getUrlAnalyseManager()->insertHomePageUrl(newlen, NULL, task,taskbatch);

                        }
                        fclose(f);
                    }else
                    {
                        mylog_info(m->m_pLogGlobalCtrl->infolog, "can not open file %s %s:%s:%d:%d\n",icconfig->alicompanyurl ,INFO_LOG_SUFFIX);
                    }
                }
                pToken = strtok_r(NULL, "\n\r\t ",&tokbuff);
            }
            taskother->alreadyused = false;
            if (homeurltmp) delete [] homeurltmp;
            mylog_info(m->m_pLogGlobalCtrl->infolog, "get task at %d  next fetch time  %d  task id %d task interval = %d currentInterval = %d nexttime = %d lastfetchNum = %d taskbatch %d - %s:%s:%d:%d", time(NULL), task->timeToFetch, task->id, task->interval, task->currentInterval, nexttime, task->lastFetchNum, taskother->taskbatch ,INFO_LOG_SUFFIX);
        }
    }
}

void TaskScheduleManager::increaseFetchNum(Task *task) {
    static pthread_mutex_t increasefetchnum_mutex = PTHREAD_MUTEX_INITIALIZER;
    if (task)
    {   
        pthread_mutex_lock(&increasefetchnum_mutex); 
        task->lastFetchNum++;
        pthread_mutex_unlock(&increasefetchnum_mutex); 

    }else
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "task is NULL %d - %s:%s:%d", task->id,INFO_LOG_SUFFIX);
    }
}

void TaskScheduleManager::increaseTaskErrorUrlNum(int taskid) {
    TaskOtherInfo *taskother = getTaskOtherInfo(taskid);
    if (taskother) {
        pthread_rwlock_wrlock(&rwlock_taskurl);
        taskother->errorurlnum++;
        pthread_rwlock_unlock(&rwlock_taskurl);
    } else {

        mylog_error(m_pLogGlobalCtrl->errorlog, "taskother is NULL %d - %s:%s:%d", taskid,INFO_LOG_SUFFIX);
    }
}

void TaskScheduleManager::increaseTaskSavefetchedNum(int taskid) {
    TaskOtherInfo *taskother = getTaskOtherInfo(taskid);
    if (taskother)
    {
        pthread_rwlock_wrlock(&rwlock_taskurl);
        taskother->savefetched++;
        pthread_rwlock_unlock(&rwlock_taskurl);
    }else
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "taskother is NULL %d - %s:%s:%d", taskid,INFO_LOG_SUFFIX);
    }
}
bool TaskScheduleManager::increaseTaskUrlNum(int taskid) {
    TaskOtherInfo *taskother = getTaskOtherInfo(taskid);
    if (taskother)
    {
        pthread_rwlock_wrlock(&rwlock_taskurl);
        taskother->urlnum++;
        pthread_rwlock_unlock(&rwlock_taskurl);
        return true;
    }else
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "taskother is NULL %d - %s:%s:%d", taskid,INFO_LOG_SUFFIX);
        return false;
    }
}
void TaskScheduleManager::increaseTaskInsUrl(int taskid) {
    TaskOtherInfo *taskother = getTaskOtherInfo(taskid);
    if (taskother) {
        pthread_rwlock_wrlock(&rwlock_taskurl);
        taskother->insurlnum++;
        pthread_rwlock_unlock(&rwlock_taskurl);
    } else {
        mylog_error(m_pLogGlobalCtrl->errorlog, "taskother is NULL %d - %s:%s:%d", taskid,INFO_LOG_SUFFIX);
    }
}
int TaskScheduleManager::decreaseTaskUrlNum(Task * task,int taskbatch) {
    TaskOtherInfo *taskother = getTaskOtherInfo(task->id);
    if (taskother)
    {
        pthread_rwlock_wrlock(&rwlock_taskurl);
        if (--(taskother->urlnum) == 0 ) {
            pthread_rwlock_unlock(&rwlock_taskurl);
            deleteTaskOther(task, taskbatch);
        } else {
            pthread_rwlock_unlock(&rwlock_taskurl);
        }
    }else
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "taskother is NULL %d - %s:%s:%d", task->id,INFO_LOG_SUFFIX);
    }
    return -1;
}
void TaskScheduleManager::deleteTaskOther(Task * task, int taskbatch)
{
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    //TaskOtherInfo * taskother1 = infocrawler->getTaskScheduleManager()->getTaskOtherInfo(task->id);
    TaskOtherInfo * taskother1 = infocrawler->getTaskScheduleManager()->poptaskother(task->id, true);
    if (!taskother1)
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "taskother is null task id %d - %s:%s:%d", task->id,INFO_LOG_SUFFIX);
        return;
    }

    taskother1->fetchnum += task->lastFetchNum;
//    infocrawler->getDbManager()->WriteDownLoadEndTime(taskother1->fetchnum, taskother1->insurlnum,taskother1->savefetched,task->id,taskbatch);
    mylog_info(m_pLogGlobalCtrl->infolog, "Task finish taskid %d batch %d onlyurl %d inserturl %d fetchnum %d - %s:%s:%d", task->id, taskbatch, taskother1->urls.size(), taskother1->insurlnum,  taskother1->fetchnum,INFO_LOG_SUFFIX);
    delete taskother1;

}

void TaskScheduleManager::checkTaskUrlNum(int taskid , int urlnum) {
    /*if (urlnum != 0) return ;
    TaskOtherInfo *taskother = getTaskOtherInfo(taskid);
    if (taskother)
    {
        taskother->running = false;
    }*/
}
char *TaskScheduleManager::getCookieFromTask(int taskid) {
    TaskOtherInfo *taskother = getTaskOtherInfo(taskid);
    if (taskother){
        return taskother->taskcookie;
    }else {
        mylog_error(m_pLogGlobalCtrl->errorlog, "taskother is null task id %d - %s:%s:%d", taskid,INFO_LOG_SUFFIX);
    }
    return NULL;
}
bool TaskScheduleManager::InsertKeywordMap(KeyWord * keyword)
{
    int source_id = keyword->source_id;
    char * keywordstr = keyword->keyword;
    int state = keyword->state;
    bool ret = false;
    pthread_rwlock_wrlock(&rwlock_keywordmaps);
    Tpair tpair = keywordsmap.equal_range(source_id);
    for (Miter iter = tpair.first; iter != tpair.second; )
    {
        if (strcmp(iter->second->keyword,keywordstr)== 0)
        {
            if (state ==0)
            {
                free(iter->second);
                keywordsmap.erase(iter++);
            }else
            {
                iter++;
            }
            pthread_rwlock_unlock(&rwlock_keywordmaps);
            return ret;
        }else
        {
                iter++;
        }
    }
    if (state !=0)
    {
        keywordsmap.insert(make_pair(source_id , keyword));
        ret = true;
    }
    pthread_rwlock_unlock(&rwlock_keywordmaps);
    return ret;
}
void TaskScheduleManager::DelKeywordMap()
{
    pthread_rwlock_wrlock(&rwlock_keywordmaps);
    for (Miter iter = keywordsmap.begin(); iter !=keywordsmap.end();)
    {
        free(iter->second);
        keywordsmap.erase(iter++);
    }
    pthread_rwlock_unlock(&rwlock_keywordmaps);
}
int TaskScheduleManager::keywordmapsize()
{
    int ret;
    pthread_rwlock_wrlock(&rwlock_keywordmaps);
    ret = keywordsmap.size();
    pthread_rwlock_unlock(&rwlock_keywordmaps);
    return ret;
}


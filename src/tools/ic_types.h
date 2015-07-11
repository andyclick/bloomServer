#ifndef _IC_TYPES_H_
#define _IC_TYPES_H_

#define STATE_SUCCESS   0
#define STATE_ERROR     1

#define DEBUG_SECTION_SYSTEM    1 

#define URL_TYPE_NOTYPE                     0
#define URL_TYPE_FINALPAGE                  1
#define URL_TYPE_LISTPAGE                   2
#define URL_TYPE_HOMEPAGE                   4
#define URL_TYPE_BBS_FINALPAGE              5
#define URL_TYPE_TOPIC_LISTPAGE             6


#define URL_TYPE_NEEDTODOWN                 16
#define URL_TYPE_NEEDTOANALYSEURL           32
#define URL_TYPE_NEEDTOSAVE                 64
#define URL_TYPE_NEEDTOADDNEW              128

#define URL_NODE_OTHER_COOKIE               1

#define MAX_URL_LEN    1024
#define MAX_URL_LEN_DATA_SOURCE     8192 
#define MAX_HOST_LEN    128
#define MAX_TITLE_LEN    128

#define URLNODE_OTHER_TYPE_COOKIE         0 

#define MAX_LAYER_ID                      3

#define REQUEST_TYPE_GET                 1
#define REQUEST_TYPE_POST                2 

#define SOURCE_TYPE_INFO                    1 
#define SOURCE_TYPE_BBS                     2 
#define SOURCE_TYPE_BLOG                    3 
#define SOURCE_TYPE_SEARCHENGINE            4                   
#define SOURCE_TYPE_TOPIC                   5                   
#define SOURCE_TYPE_COMPANY                 6                   

#define SOURCE_TYPE_COURT                   200                   

#define TASK_FROM_DB        1
#define TASK_UNFROM_DB      2 

#define MEMCACHEDB_RECONNECT_NUM        100

#define PAGEBUF_NEED_DEF            1
#define INFO_LOG_SUFFIX           __FILE__,__FUNCTION__,__LINE__
#define ERROR_LOG_SUFFIX           __FILE__,__FUNCTION__,__LINE__,errno

#define SAVE_FATHER_URL  1
#define SAVE_URL  2

#include <string>
#include <list>
#include <set>
#include "util.h"
#include "utilcpp.h"
#include "mycxxlog.h"
using namespace std;

typedef union _SELECTTYPE{
    int intnum;
}selectType;

typedef struct _KeyWord{
    char keyword[257];
    int state;
    int source_id;
    _KeyWord(){
        memset(keyword, 0, 257);
        state = 0;
        source_id = 0;
    };
    void clone(_KeyWord *des , _KeyWord *src){
        memcpy(des , src ,sizeof(_KeyWord));
    };
} KeyWord;
typedef struct _LOGLOBALCTRL{
    LoggerPtr infolog;
    LoggerPtr errorlog;
    LoggerPtr importantlog;
    LoggerPtr veryimportantlog;
} LogGlobalCtrl;
typedef struct _ICCONFIG {
    char logfile[128];
    char alicompanyurl[128];
    char errfile[128];
    char logdir[128];
    char pidfile[128];
    char dbpath[128];
    char useragent[128]; //for http protocol
    int maxsignalterm;
    char isfetching;
    int fetchingthreads;
    int httptimeout;
    int checkpointtime; //seconds
    int dbinterval; //seconds
    char checkpointpath[256];
    char dbuser[64];
    char dbpassword[64];
    char dbservername[64];
    char urltemplate[1024];
    char mainserverip[128];
    vector <string> localips;
    int localipnum;
    int mainserverport;
    char contentmemcachedbip[128];
    char urlmemcachedbip[128];
    int externalserverport;
    int dbreconnectnum;
    int maxfinalnextpagetofetch; //should no more than 30
    int maxlistnextpagetofetch; 
    int spiderid;
    int spidernum;
    int resinsertinterval;
    char selectneedappend[128];
    char idxtemplate[64];
    int waitstopinterval;
    int dbretrytimes;
    int dbretrysleeptime;
    int maxpagebufsize;
    int ifdef;

    char db_user_name_a[128];
    char db_password_a[128];
    char db_server_name_a[128];
    char db_name_a[128];
    int db_min_conn_a;
    int db_max_conn_a;
    int db_incr_conn_a;

    char hypertablenamespace[128];

    char hyperaddress[128];
    int hyperport;
    int spider_id;
    
    _ICCONFIG() {
        db_user_name_a[0]=0;
        db_password_a[0]=0;
        db_server_name_a[0]=0;
        db_name_a[0]=0;
        db_min_conn_a=0;
        db_max_conn_a=0;
        db_incr_conn_a=0;
        hyperaddress[0]=0;
        hyperport=0;
        hypertablenamespace[0]=0;

        logfile[0] = 0;
        alicompanyurl[0] = 0;
        errfile[0] = 0;
        logdir[0] = 0;
        pidfile[0] = 0;
        dbpath[0] = 0;
        useragent[0] = 0;
        maxsignalterm = 3;
        fetchingthreads = 100;
        isfetching = false;
        httptimeout = 1;
        checkpointtime = 60 * 60;
        checkpointpath[0] = 0;
        dbuser[0] = 0;
        dbpassword[0] = 0;
        dbservername[0] = 0;
        urltemplate[0] = 0;
        externalserverport = 3000;
        dbreconnectnum = 0;
        dbinterval = 60 * 10;
        maxfinalnextpagetofetch = 30;
        mainserverip[0] = 0;
        mainserverport= 3456;
        localipnum = 0;
        maxlistnextpagetofetch =30;   
        spiderid = 0;
        spidernum = 0;
        contentmemcachedbip[0] = 0;
        urlmemcachedbip[0] = 0;
        int resinsertinterval=7200;
        selectneedappend[0] = 0;
        idxtemplate[0] = 0;
        waitstopinterval = 3600;
        dbretrytimes = 3;
        dbretrysleeptime = 5;//five seconds
        maxpagebufsize = 4 *1024*100*30;// 30 M
        ifdef = PAGEBUF_NEED_DEF;// 30 M
       /* infolog = NULL; 
        errorlog = NULL;
        importantlog = NULL;
        veryimportantlog = NULL;*/
    }
} ICCONFIG;

typedef struct _Task {
    int id;
    char name[400];
    char host[64];
    char industry[32];
    char homeurl[MAX_URL_LEN_DATA_SOURCE];
    char pageurl[MAX_URL_LEN_DATA_SOURCE];//list page
    char autodown[2];
    char sourcestate[16];
    char downloadtime[32];
    char charset[40];
    char loginurl[MAX_URL_LEN_DATA_SOURCE];
    char ultimatepageurl[MAX_URL_LEN_DATA_SOURCE];
    char bbsultimatepageurl[MAX_URL_LEN_DATA_SOURCE];
    char ignoreurl[MAX_URL_LEN_DATA_SOURCE];
    int tasksendtype;
    time_t  timeToFetch;
    time_t  timeOfTheseFetch;
    int  lastFetchNum;
    time_t  interval;  //seconds between fetch, initialized from db and can not be changed  
    time_t  currentInterval;  //seconds between fetch, adjust at each fetch  
    int sourcetype;
    int depth;
    char intervalauto [4];
    int noaccessInterval; 
    char intxt[32];
    _Task() {
        intxt[0]=0;
        id = 0;
        name[0] = 0;
        host[0] = 0;
        industry[0] = 0;
        homeurl[0] = 0;
        pageurl[0] = 0;
        autodown[0] = 0;
        sourcestate[0] = 0;
        downloadtime[0] = 0;
        charset[0] = 0;
        ultimatepageurl[0] = 0;
        bbsultimatepageurl[0] = 0;
        ignoreurl[0] = 0;
        loginurl[0] = 0;
        timeToFetch = 0;
        timeOfTheseFetch = 0;
        lastFetchNum = 1; //
        interval = 0;
        currentInterval = 0;
        tasksendtype = REQUEST_TYPE_GET;
        sourcetype = SOURCE_TYPE_INFO; 
        depth = 3;
        intervalauto[0] = 0;
        noaccessInterval = 0;   
    };

    void clone(_Task *des, _Task *src) {
        memcpy(des, src, sizeof(_Task));
    };
} Task;

typedef struct _TaskOtherInfo {
    int id;
    bool fetchingcookie;
    bool running;
    char *taskcookie;
    int taskcookielen;
    int errorurlnum; 
    int fetchnum;//
    int urlnum;
    int insurlnum;
    bool alreadyused;
    set<string> urls;
    int taskbatch;
    int savefetched;
    pthread_rwlock_t rwlock_taskother_urls;
    size_t begintime;
    _TaskOtherInfo(int id)
    {
        this->id = id;
        fetchingcookie = false;
        taskcookie = NULL;
        taskcookielen =0;
        errorurlnum =0;
        running = false;
        fetchnum= 0;
        urlnum= 0;
        insurlnum = 0;
        taskbatch = 0;
        savefetched = 0;
        alreadyused = true;
        pthread_rwlock_init(&rwlock_taskother_urls,NULL);
        begintime = time(NULL);
    };
    void insertcookie(char * cookie, int cookielen)
    {
        if (cookie && cookielen >0)
        {
            if (this->taskcookie) delete [] this->taskcookie;

            this->taskcookie = new char [cookielen+1];
            strncpy(this->taskcookie ,cookie ,cookielen);
            this->taskcookie[cookielen] = 0;
            taskcookielen = cookielen;
        }
    };

    void appendcookie(char *cookie, int cookielen)
    {
        if (cookie && cookielen >0)
        {
            int cookielentmp = this->taskcookielen;
            this->taskcookielen = this->taskcookielen +cookielen;

            char * cookietmp = new char [taskcookielen + 1];
            strncpy(cookietmp ,this->taskcookie,cookielentmp);
            strncpy(cookietmp + cookielentmp, cookie, cookielen);
            if (this->taskcookie) delete [] this->taskcookie;
            this->taskcookie = cookietmp;
            this->taskcookie[taskcookielen] = 0;
        }
    };
    bool findurl(string url)
    {
        set<string>::iterator urlsiter;
        pthread_rwlock_wrlock(&rwlock_taskother_urls);
        if ((urlsiter= urls.find(url)) != urls.end())
        {
            pthread_rwlock_unlock(&rwlock_taskother_urls);
            return true;
        }else
        {
            pthread_rwlock_unlock(&rwlock_taskother_urls);
            return false;
        }
    };

    void inserttaskurl(string url)
    {
        pthread_rwlock_wrlock(&rwlock_taskother_urls);
        urls.insert(url);        
        pthread_rwlock_unlock(&rwlock_taskother_urls);
    }
    void deletetaskurl(string url) { 
        pthread_rwlock_wrlock(&rwlock_taskother_urls);
        urls.erase(url);
        pthread_rwlock_unlock(&rwlock_taskother_urls);
    }

    ~_TaskOtherInfo()
    {
        if (this->taskcookie) delete [] this->taskcookie;
    }
} TaskOtherInfo;

typedef struct _Other{
    int len;
    char * data;
    _Other()
    {
        len = 0;
        data = NULL;
    };
    _Other(int len,char * data)
    {
        if (data && data[0])
        {
            this->data = new char [len+1];
            strncpy(this->data, data, len);
            this->data[len] = 0;
        }
        this->len = len;
    };
    char * getdata()
    {
        return this ->data;
    }
    ~_Other()
    {
        if (this->data) delete[] this->data;
    };
} Other;

typedef struct _UrlNode {
    char url[MAX_URL_LEN];
    char username[128];
    char fatherurl[MAX_URL_LEN];
    char refererurl[MAX_URL_LEN];
    char title[MAX_TITLE_LEN];
    char topicsource[MAX_TITLE_LEN];
    int  taskid;
    Other ** other;
    int type; //type of url, including need to save, need to down ...
    char errornum;
    ulonglong id;
    int page;
    int maxtype;
    int layerid;
    int bbsid;
    bool needtologin;
    char redirectnum;
    int taskbatch;
    Task * task;
    int nextpage;
    int ipnum;
    set<string> urls;
	int nowpage;    
	int totalpage;
    string html;
	 long keyid;
	  long searchid;
	   int fetched_page;
    char onepage[32];
	long sqlite_id;
_UrlNode() {
        url[0] = 0;
        username[0] = 0;
        taskid = 0;
        type = URL_TYPE_NOTYPE;
        errornum = 0;
        id = 0;
        page = 1;
        maxtype = -1;
        other = NULL;
        layerid = 1;
        bbsid = -1;
        needtologin = false;
        redirectnum = 0;
        fatherurl[0] = 0; 
        refererurl[0] = 0; 
        taskbatch = -1;
        title[0] = 0;
        topicsource[0] = 0;
        task = NULL;
        ipnum = 0;
        nextpage = 1;
		fetched_page = 0;
		sqlite_id = 0;
	nowpage = 1;    
	totalpage =1;
    html = "";
}

 int serialize(char *buf) {
        int len = 0; 
        len += serializeString(buf, url);
        len += serializeString(buf, fatherurl);
        len += serializeString(buf, title);
        len += serializeString(buf, topicsource);
        len += serializeInt(buf, taskid);
        len += serializeInt(buf, type);
        len += serializeChar(buf, errornum);
        len += serializeULongLong(buf, id);
        len += serializeInt(buf, page);
        len += serializeInt(buf, maxtype);
        len += serializeInt(buf, layerid);
        len += serializeInt(buf, bbsid);
        len += serializeBool(buf, needtologin);
        len += serializeChar(buf, redirectnum);
        len += serializeInt(buf, taskbatch);
        len += serializeLong(buf, keyid);
        len += serializeLong(buf, searchid);
        len += serializeInt(buf, nextpage);
        len += serializeInt(buf, totalpage);
        len += serializeInt(buf, fetched_page);
        len += serializeString(buf, onepage);
        len += serializeInt(buf, ipnum);
        len += serializeLong(buf, sqlite_id);
        return len;
    }

    void unserialize(char *buf) {
        unserializeString(buf, url);
        unserializeString(buf, fatherurl);
        unserializeString(buf, title);
        unserializeString(buf, topicsource);
        unserializeInt(buf, taskid);
        unserializeInt(buf, type);
        unserializeChar(buf, errornum);
        unserializeULongLong(buf, id);
        unserializeInt(buf, page);
        unserializeInt(buf, maxtype);
        unserializeInt(buf, layerid);
        unserializeInt(buf, bbsid);
        unserializeBool(buf, needtologin);
        unserializeChar(buf, redirectnum);
        unserializeInt(buf, taskbatch);
        unserializeLong(buf, keyid);
        unserializeLong(buf, searchid);
        unserializeInt(buf, nextpage);
        unserializeInt(buf, totalpage);
        unserializeInt(buf, fetched_page);
        unserializeString(buf, onepage);
        unserializeInt(buf, ipnum);
        unserializeLong(buf, sqlite_id);
    }

    _UrlNode(Task * task,char * topicsource,char * title,int taskbatch,char *fatherurl,char *url, void *other,int  othermaxtype, int type, int errornum, ulonglong id, int redirectnum,int page = 1,int layerid = 1, int bbsid = -1 , bool needtologin = false) {
        if (strlen(url) > MAX_URL_LEN - 1) {
            strncpy(this->url, url, MAX_URL_LEN - 1);
            this->url[MAX_URL_LEN - 1] = 0;
        } else {
            strcpy(this->url, url);
        }
        
        if (strlen(fatherurl) > MAX_URL_LEN - 1) {
            strncpy(this->fatherurl, fatherurl, MAX_URL_LEN - 1);
            this->fatherurl[MAX_URL_LEN - 1] = 0;
        } else {
            strcpy(this->fatherurl, fatherurl);
        }
        if (title)
        {
            this->copytitle(title);
        }else
        {
            this->title[0] = 0;
        }
        if (topicsource)
        {
            this->copytopicsource(topicsource);
        }else
        {
            this->topicsource[0] = 0;
        }

        this->other = NULL;
        this->maxtype= -1;
        if (other)
        {
            this ->copyother((Other **)other,othermaxtype);
        }
        this->taskid = task->id;
        this->type = type;
        this->errornum = errornum;
        this->id = id;
        this->page = page;
        this->layerid = layerid;
        this->needtologin = needtologin;
        this->redirectnum = redirectnum;
        this->taskbatch= taskbatch;
        this->bbsid = bbsid;
        this->task = task;
        nextpage = 1;
        ipnum = 0;
        this->refererurl[0] = 0;
        this->username[0] = 0;
        html = "";
    }

    void copyurl(char *url) {
        if (strlen(url) > MAX_URL_LEN - 1) {
            strncpy(this->url, url, MAX_URL_LEN - 1);
            this->url[MAX_URL_LEN - 1] = 0;
        } else {
            strcpy(this->url, url);
        }
    }
     void copyusername(char * username) {
        if (strlen(username) > MAX_URL_LEN - 1) {
            strncpy(this->username, username, 128 - 1);
            this->username[MAX_URL_LEN - 1] = 0;
        } else {
            strcpy(this->username, username);
        }
    }

    void copyfatherurl(char *fatherurl) {
        if (strlen(fatherurl) > MAX_URL_LEN - 1) {
            strncpy(this->fatherurl, fatherurl, MAX_URL_LEN - 1);
            this->fatherurl[MAX_URL_LEN - 1] = 0;
        } else {
            strcpy(this->fatherurl, fatherurl);
        }
    }
    void copyrefererurl(char *refererurl) {
        if (strlen(refererurl) > MAX_URL_LEN - 10) {
            strncpy(this->refererurl, refererurl, MAX_URL_LEN - 10);
            this->refererurl[MAX_URL_LEN - 10] = 0;
        } else {
            strcpy(this->refererurl, refererurl);
        }
    }

    void copytitle(char *title) {
        if (strlen(title) > MAX_TITLE_LEN - 1) {
            strncpy(this->title, title, MAX_TITLE_LEN - 1);
            this->title[MAX_TITLE_LEN - 1] = 0;
        } else {
            strcpy(this->title, title);
        }
    }

    void copytopicsource(char * topicsource) {
        if (strlen(topicsource) > MAX_TITLE_LEN - 1) {
            strncpy(this->topicsource, topicsource, MAX_TITLE_LEN - 1);
            this->topicsource[MAX_TITLE_LEN - 1] = 0;
        } else {
            strcpy(this->topicsource, topicsource);
        }
    }
    void copyother(Other **src ,int srcmax)
    {
        for(int i=0 ; i <= srcmax; i++)
        {
            if (src[i])
            {
                insertother(i,src[i]->data ,src[i]->len);
            }
        }
    };


    void insertother(int type, char *data, int len)
    {
        if (!data || len <=0) return;

        if (type <= maxtype)
        {
            if (this->other) {
                if (this ->other[type]) {
                    delete (this ->other [type]);
                    this->other[type] = new Other (len , data);
                } else {
                    this->other[type] = new Other (len , data);
                }
            } else {
                other = (Other **)malloc(sizeof(Other *) * (type + 1));
                memset(other,0,sizeof(Other*)*(type +1));
                this->other[type] = new Other (len , data);
            }
        } else {
            if (this->other)
            {
                other = (Other **)realloc(other , (sizeof(Other *) * (type + 1)));
                for(int i = maxtype + 1 ; i < type +1 ; i++)
                {
                    other[i] = NULL;
                }
                this->other[type]  = new Other (len , data);
            }else
            {
                other = (Other **)malloc(sizeof(Other *) * (type + 1));
                memset(other,0,sizeof(Other*)*(type +1));
                this->other[type] = new Other (len , data);
            }
            this->maxtype = type;
        }
    };
    void * getothervalue(int type)
    {
        if (type  <= maxtype)
        {
            if (other)
            {
                if (this->other[type])
                {
                    return other[type] -> getdata();
                }
            }
        }
        return NULL  ;
    }

    ~_UrlNode() {
        for(int i=0 ; i<=maxtype ; i++)
        {
            if (!other[i]) continue;
            delete other[i];
        }
        if(other) free(other);
    }

} UrlNode;

typedef struct _HostsFifoQueue {
    list<UrlNode *> urls;
    int urlnum;
    time_t lastFetchTime;
    int totalFetchNum;
    pthread_rwlock_t rwlock_hosts_urls;
    _HostsFifoQueue() {
        urlnum = 0;
        lastFetchTime = 0;
        totalFetchNum = 0;
       pthread_rwlock_init(&rwlock_hosts_urls,NULL);
    }
    void push_back(UrlNode *node)
    {
        pthread_rwlock_wrlock(&rwlock_hosts_urls);
        this->urls.push_back(node);
        urlnum++;
        pthread_rwlock_unlock(&rwlock_hosts_urls);
    }
    void push_front(UrlNode *node)
    {
        pthread_rwlock_wrlock(&rwlock_hosts_urls);
        this->urls.push_front(node);
        urlnum++;
        pthread_rwlock_unlock(&rwlock_hosts_urls);
    }

    bool empty() {
        bool ret = false;
        pthread_rwlock_wrlock(&rwlock_hosts_urls);
        ret = urls.empty();
        pthread_rwlock_unlock(&rwlock_hosts_urls);
        return ret;
    }

    UrlNode *pop()
    {
        UrlNode *node = NULL;
        pthread_rwlock_wrlock(&rwlock_hosts_urls);
        if (!urls.empty()) {
            node = urls.front();
            urls.pop_front();
            urlnum--;
        }
        pthread_rwlock_unlock(&rwlock_hosts_urls);
        return node;
    }
} HostsFifoQueue;

class HostNode {
public:
    char host[MAX_URL_LEN];
    HostsFifoQueue *fifoqueue;
    ulonglong nexttime; //millisecond
    int id;
    int emptytimes;
    int available;
    time_t interval; //millisecond
    int retnum;
	int sqlite_host_id;
    pthread_rwlock_t rwlock_host;
    HostNode() {
        host[0] = 0;
        available = false; //is used; must set false
        fifoqueue = NULL;
        nexttime = 0;
        id = -1;
        emptytimes = 0;
        interval = 500; // 0.5 seconds;
        retnum= 0;
		 sqlite_host_id = -1;
        pthread_rwlock_init(&rwlock_host, NULL);
    }
    void clear() {
        available = true;
        host[0] = 0;
        nexttime = 0;
        emptytimes = 0;
        retnum = 0;
		sqlite_host_id = -1;
    }
public:
	void push_back(UrlNode *urlnode, bool only_memory = false);
    void push_front(UrlNode *urlnode, bool only_memory = false);
};


typedef struct {
    pthread_t thread_tid;
    long thread_count;
} Thread;

class TaskLesser
{
    public:
    bool   operator() (Task   *a,   Task   *b)
    {
        return a->timeToFetch  >  b->timeToFetch;
    }
};

class HostsLesser
{
    public:
    bool   operator() (HostNode *a, HostNode *b)
    {
        return a->nexttime  >  b->nexttime;
    }
};


typedef struct _RESPONSE_HEADER {
    string contenttype;
    string charset;
    string location;
    int contentlength;
    int retcode;
    _RESPONSE_HEADER() {
        retcode = 0;
    }
} RESPONSE_HEADER;

#endif

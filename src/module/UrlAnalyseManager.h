#ifndef _MODULE_URLANALYSEMANAGER_H_
#define _MODULE_URLANALYSEMANAGER_H_

#include "Manager.h"
#include "ic_types.h"
#include <queue>
#include <vector>
#include <list>
#include <map>
#include <string>
#include "Page.h"
#include "hash_table.h"

using namespace std;
#define INSERT_URL_FORCED 1
#define INSERT_URL_NOT_FORCED 0

struct onfindpackage
{
    void *manager;
    UrlNode *node;
    list<UrlNode *> *urls;
    char *nextpageurl;
    HashTable hashtablenextnum;
    HashTable hashtablenextpage;
};

class UrlAnalyseManager: virtual public Manager {
public:
    UrlAnalyseManager(LogGlobalCtrl * pLogGlobalCtrl);
    ~UrlAnalyseManager();
    virtual int start();
    virtual int stop();
    int insertUrl(UrlNode *node, int flag = INSERT_URL_NOT_FORCED, bool ispushback = true,bool only_memory = false);
    UrlNode *getUrl();
    UrlNode *getUrlFromOuterHtml(); 
    int analyseUrls(UrlNode *urlnode, RESPONSE_HEADER *rheader, char *content, int contentlen, char *nextpageurl, bool no_need_to_analyze_mytype = false);
    void dump(); 
    void retrieve(); 
    int insertHomePageUrl(char *url,void * other, Task *task,int taskseq);
    int insertUrl(char *url, void * other, Task *task, int taskbatch);
    bool is(char *url, char *pattern);
    bool isHomeUrl(char *url, char *taskhomeurl);
    int getType(int urltype);
    int getType(char *url, Task *task);
    int hostToFifoqueuesize();
    int GetUserName(char * url,UrlNode * node,UrlNode * newnode,UrlNode * newnode1,UrlNode * newnode2);

    int PutFinalPage(map<string ,UrlNode *> & finalpagelist,UrlNode * urlnode);
private:
    LogGlobalCtrl       *m_pLogGlobalCtrl;
    static pthread_rwlock_t rwlock_priority_queue;
    static pthread_rwlock_t rwlock_host_map;
    static pthread_rwlock_t rwlock_hosts_vector;
    static pthread_rwlock_t rwlock_urls_list;
    static pthread_mutex_t mutexDetect;
	static pthread_rwlock_t rwlock_unfetched_url_table;

    priority_queue<HostNode *, vector<HostNode *>, HostsLesser> hosts;
    map<string, HostNode *> hostToFifoqueue;
    vector<HostNode *> hostsVector;

    int findEmptyVector();
    void extractUrlsFromContent(UrlNode *urlnode, RESPONSE_HEADER *rheader, char *content, list<UrlNode *> &urls, char *nextpageurl);
    int removeNodeFromPriorityQueue(int id);

	void get_host_sqlite_id(HostNode *hostnode); 


    int getHomeType();
    int remove(int id);

    HostNode *pop();
    HostNode *getNextHost(int & ipnum ); 
    int push(HostNode *host);

    HashTable hashtablenextpage;
};
#endif

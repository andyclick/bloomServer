#include "InfoCrawler.h"
#include "UrlAnalyseManager.h"
#include "CheckpointManager.h"
#include "Url.h"
#include "list.h"
#include "uri.h"
#include "hlink.h"
#include "Page.h"
#include "Http.h"
#include "HtRegex.h"
#include "util.h"
#include "finalpage.h"
#include "ic_types.h"
//#include <valgrind/valgrind.h>
#include "hash_table.h"
#include <boost/regex.hpp>

pthread_rwlock_t UrlAnalyseManager::rwlock_priority_queue = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t UrlAnalyseManager::rwlock_host_map = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t UrlAnalyseManager::rwlock_hosts_vector = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t UrlAnalyseManager::rwlock_urls_list = PTHREAD_RWLOCK_INITIALIZER;
pthread_mutex_t UrlAnalyseManager::mutexDetect = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t UrlAnalyseManager::rwlock_unfetched_url_table = PTHREAD_RWLOCK_INITIALIZER;


static int onfind(const char *elem, const char *attr, struct uri *uri, void *arg); 
void killhtmlcode(char *rin,char *rout);

UrlAnalyseManager::UrlAnalyseManager(LogGlobalCtrl * pLogGlobalCtrl) {
    static char *nextpage1 = "\xCF\xC2\xD2\xBB\xD2\xB3";
    static char *nextpage2 = "[\xCF\xC2\xD2\xBB\xD2\xB3]";
    static char *nextpage3 = "\xE4\xB8\x8B\xE4\xB8\x80\xE9\xA1\xB5";
    static char *nextpage4 = "[\xE4\xB8\x8B\xE4\xB8\x80\xE9\xA1\xB5]";
    int ret = 0;
    hashtablenextpage = create_hash_table(383); //should be prime number
    hash_find_or_add(hashtablenextpage, nextpage1, strlen(nextpage1), 0, &ret);
    hash_find_or_add(hashtablenextpage, nextpage2, strlen(nextpage2), 0, &ret);
    hash_find_or_add(hashtablenextpage, nextpage3, strlen(nextpage3), 0, &ret);
    hash_find_or_add(hashtablenextpage, nextpage4, strlen(nextpage4), 0, &ret);
    m_pLogGlobalCtrl = pLogGlobalCtrl;
}

UrlAnalyseManager::~UrlAnalyseManager() {
    free_hash_table(hashtablenextpage);

}

int UrlAnalyseManager::start() {
    Manager::start();
    return 0;

}

int UrlAnalyseManager::stop() {
        for(int i = 0; i < hostsVector.size(); i++) {
        HostNode *hostnode = hostsVector[i];
        list<UrlNode *>::iterator iter;
        for(iter = hostnode->fifoqueue->urls.begin(); 
            iter != hostnode->fifoqueue->urls.end(); ++iter) {
            UrlNode *urlnode = *iter;
            char buf[sizeof(UrlNode)];
            int len = urlnode->serialize(buf);
            if (hostnode->sqlite_host_id == -1) {
                int ret = InfoCrawler::getInstance()->getDbManager()->get_hostid_by_domain(hostnode->host, hostnode->sqlite_host_id);
                if (ret == STATE_SUCCESS) {
                    if (hostnode->sqlite_host_id == -1) {
                        int ret = InfoCrawler::getInstance()->getDbManager()->insert_host(hostnode->host);
                        if (ret == STATE_SUCCESS) {
                            InfoCrawler::getInstance()->getDbManager()->get_hostid_by_domain(hostnode->host, hostnode->sqlite_host_id);
                        }	
                    }
                }
            }
            if (hostnode->sqlite_host_id > 0) {
                InfoCrawler::getInstance()->getDbManager()->insert_unfetched_url(urlnode->url, buf, len, hostnode->sqlite_host_id, false) ;
            }
            delete urlnode;
        }
        delete hostnode;
    }

    return Manager::stop();

}

int UrlAnalyseManager::insertHomePageUrl(char *url, void * other, Task *task, int taskbatch) { //only for insert homepage
    if (!task)
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "insert task is null - %s:%s:%d:%d", task->id,ERROR_LOG_SUFFIX);
        return -1;
    }
    UrlNode *urlnode = NULL;
    bool needtologin = false;
    if (task->loginurl[0])
    {
        needtologin = true;
    }
    if (task->sourcetype == SOURCE_TYPE_SEARCHENGINE)
    {
        urlnode = new UrlNode(task,NULL, NULL,taskbatch,url,url, other, -1, getType(SOURCE_TYPE_SEARCHENGINE), 0, 0,1 ,1, 1,-1 ,needtologin);
    } else {
        urlnode = new UrlNode(task, NULL, NULL,taskbatch,url,url, other, -1, getHomeType(), 0, 0,1 ,1, 1,-1 ,needtologin);
    }
    mylog_info(m_pLogGlobalCtrl->infolog, "insert homepage %s %d %d %llu batch id %d - %s:%s:%d", url,task->id, urlnode->type, urlnode->id,taskbatch,INFO_LOG_SUFFIX);
    insertUrl(urlnode, INSERT_URL_NOT_FORCED, false);
}

int UrlAnalyseManager::insertUrl(char *url, void * other, Task *task, int taskbatch) { 
    if (!task)
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "insert task is null - %s:%s:%d:%d", task->id,ERROR_LOG_SUFFIX);
        return -1;
    }
    UrlNode *urlnode = NULL;

    bool needtologin = false;
    if (task->loginurl[0])
    {
        needtologin = true;
    }

    urlnode = new UrlNode(task, NULL, NULL,taskbatch,url,url, other, -1, getType(url, task), 0, 0,1 ,1, 1,-1 , needtologin);

    mylog_info(m_pLogGlobalCtrl->infolog, "insert common url %s %d %d %llu batch id %d - %s:%s:%d", url,task->id, urlnode->type, urlnode->id,taskbatch,INFO_LOG_SUFFIX);

    insertUrl(urlnode, INSERT_URL_NOT_FORCED, false);
}

int UrlAnalyseManager::insertUrl(UrlNode *node ,int flag, bool ispushback, bool only_memory) {
    CUrl url;
    url.parse(node->url);
    map<string, HostNode *>::iterator iter;
    int ret = -1;
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    if (url.m_sHost.empty() || url.m_sHost.length() > MAX_URL_LEN - 1) {
        mylog_info(m_pLogGlobalCtrl->infolog, "host is NULL host len %d  - %s:%s:%d",url.m_sHost.length(),INFO_LOG_SUFFIX);
        infocrawler->getLocalDbManager()->decidesaveFetched(node);
        delete node;
        return 0;
    }
     
    if (node->id < 1) {
        ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
        ulonglong idtmp = 0;
        while(idtmp == 0 && running())
        {
            idtmp = InfoCrawler::getInstance()->getCheckpointManager()->GetMaxID(icconfig->spiderid);
        }
        node->id = idtmp; 
        //node->id = InfoCrawler::getInstance()->getCheckpointManager()->getMaxUrlId();
    }
    
    TaskOtherInfo *taskother = infocrawler->getTaskScheduleManager()->getTaskOtherInfo(node->taskid);
    if (taskother && flag != INSERT_URL_FORCED)
    {
        //if ((node->type & URL_TYPE_FINALPAGE) &&(node->task->sourcetype != SOURCE_TYPE_SEARCHENGINE ) && 
          //  (node->task->sourcetype != SOURCE_TYPE_BBS) && (node->task->sourcetype != SOURCE_TYPE_COURT))
        if (node->type & URL_TYPE_LISTPAGE) 
        {
           if (InfoCrawler::getInstance()->getLocalDbManager()->alreadyfetched(node)) {
                mylog_info(m_pLogGlobalCtrl->infolog, "already fetched %llu %s  - %s:%s:%d", node->id ,node->url,INFO_LOG_SUFFIX);
                delete node;
                return 0;
            }
            //0128.begin()
            /*char memorytable[128]; memorytable[0]=0;
           // sprintf(memorytable,"url_%d_tbl",node->taskid);
            sprintf(memorytable,"url_%d_tbl",1);
            if(InfoCrawler::getInstance()->gethyper()->check_fetch_url(node,string(memorytable)))
            {
                 mylog_info(m_pLogGlobalCtrl->infolog, "already fetched %llu %s  - %s:%s:%d", node->id ,node->url,INFO_LOG_SUFFIX);
                 delete node;
                 return 0;
            }*/
           //0128.end()
        }
        if (taskother->findurl(node->url))
        {
           mylog_info(m_pLogGlobalCtrl->infolog, "already fetched %llu %s in batch %d  - %s:%s:%d", node->id ,node->url,node->taskbatch,INFO_LOG_SUFFIX);
            delete node;
            return 0;
        }else
        {
            //mylog_info(m_pLogGlobalCtrl->infolog, "int in to taskother list%llu %s in batch %d  - %s:%s:%d", node->id ,node->url,node->taskbatch,INFO_LOG_SUFFIX);
            taskother->inserttaskurl(node->url);
        }
    }else
    {
        if (taskother && !(taskother->findurl(node->url)))
        {
            mylog_info(m_pLogGlobalCtrl->infolog, "int in to taskother list%llu %s in batch %d  - %s:%s:%d", node->id ,node->url,node->taskbatch,INFO_LOG_SUFFIX);
            taskother->inserttaskurl(node->url);
        }
    }

    mylog_info(m_pLogGlobalCtrl->infolog, "insert new url %s id = %llu taskid = %d type = %d layerid = %d taskbatch = %d  %d %d - %s:%s:%d", node->url, node->id, node->taskid,node->type,node->layerid, node->taskbatch ,node->page,node->nextpage ,INFO_LOG_SUFFIX);

    string hoststmp ;
    if (node->task->host[0])
    {
        hoststmp = node->task->host;
    }else
    {
        hoststmp = url.m_sHost;
    }
    pthread_rwlock_rdlock(&rwlock_host_map); 
    if ((iter = hostToFifoqueue.find(hoststmp)) 
        != hostToFifoqueue.end()) { //find host queue
        HostNode *hostnode = iter->second;
        if (hostnode->fifoqueue) {
            infocrawler->getTaskScheduleManager()->increaseTaskUrlNum(node->taskid);
            infocrawler->getTaskScheduleManager()->increaseTaskInsUrl(node->taskid);
            if (ispushback) {
                hostnode->push_back(node,only_memory);
            } else {
                hostnode->push_front(node,only_memory);
            }
        } else {
            mylog_error(m_pLogGlobalCtrl->errorlog, "fifoqueue is null, can't push urlnode  url = %s id = %llu taskid = %d host = %s - %s:%s:%d", node->url, node->id, node->taskid, iter->second->host ,INFO_LOG_SUFFIX);
            infocrawler->getLocalDbManager()->decidesaveFetched(node);
            delete node;
        }
        pthread_rwlock_unlock(&rwlock_host_map); 
    } else { //not find
        pthread_rwlock_unlock(&rwlock_host_map); 

        pthread_rwlock_wrlock(&rwlock_hosts_vector); 
        int hostid = findEmptyVector();
        pthread_rwlock_unlock(&rwlock_hosts_vector); 
        if (hostid != -1 ) { //find a empty host 
            HostNode *hostnode = hostsVector[hostid];
            if (hostnode) {
                if (node->task->sourcetype == SOURCE_TYPE_SEARCHENGINE)
                {
                    hostnode->interval = 1000;// 1 seconds; 
                }else if (node->task->sourcetype == SOURCE_TYPE_COMPANY)
                {
                    hostnode->interval = node->task->noaccessInterval * 1000 ;// 1 seconds; 
                }
                infocrawler->getTaskScheduleManager()->increaseTaskUrlNum(node->taskid);
                infocrawler->getTaskScheduleManager()->increaseTaskInsUrl(node->taskid);
                if (ispushback) {
                    hostnode->push_back(node,only_memory);
                } else {
                    hostnode->push_front(node,only_memory);
                }
                hostnode->nexttime = getNow() + (2 * 1000);// plus two millisecond
                strcpy(hostnode->host, (char *)hoststmp.c_str());
				get_host_sqlite_id(hostnode);
                
                pthread_rwlock_wrlock(&rwlock_host_map); 
                hostToFifoqueue.insert(make_pair(hostnode->host, hostnode));
                pthread_rwlock_unlock(&rwlock_host_map); 

                push(hostsVector[hostid]);
                mylog_info(m_pLogGlobalCtrl->infolog, "find an empty hostnode hostid = %d %llu %s %llu %d %s - %s:%s:%d", hostnode->id, hostnode->nexttime, node->url, node->id, node->taskid, hostnode->host ,INFO_LOG_SUFFIX);
            } else{
                mylog_error(m_pLogGlobalCtrl->errorlog, "hostnode is NULL - %s:%s:%d", INFO_LOG_SUFFIX);
                infocrawler->getLocalDbManager()->decidesaveFetched(node);
                delete node;
            }
        } else {
            //make a new host node
            HostNode *hostnode = new HostNode;
            if (node->task->sourcetype == SOURCE_TYPE_SEARCHENGINE)
            {
                hostnode->interval = 1000;// 1 seconds; 
            }else if (node->task->sourcetype == SOURCE_TYPE_COMPANY)
            {
                hostnode->interval = node->task->noaccessInterval * 1000 ;// 1 seconds; 
            }
            HostsFifoQueue *fifoqueue = new HostsFifoQueue;
            hostnode->fifoqueue = fifoqueue;
            hostnode->nexttime = getNow() + (2 * 1000);// plus two millisecond
            strcpy(hostnode->host, (char *)hoststmp.c_str());

			get_host_sqlite_id(hostnode);

            //insert new host node into hostvector
            pthread_rwlock_wrlock(&rwlock_hosts_vector);
            hostsVector.push_back(hostnode);
            int id = hostsVector.size() - 1;
            pthread_rwlock_unlock(&rwlock_hosts_vector);

            //insert new url into fifoqueue
            infocrawler->getTaskScheduleManager()->increaseTaskUrlNum(node->taskid);
            infocrawler->getTaskScheduleManager()->increaseTaskInsUrl(node->taskid);
            if (ispushback) {
                hostnode->push_back(node,only_memory);
            } else {
                hostnode->push_front(node,only_memory);
            }
            hostnode->id = id;

            //insert new host to hostnode map
            pthread_rwlock_wrlock(&rwlock_host_map); 
            hostToFifoqueue.insert(make_pair(hostnode->host, hostnode));
            pthread_rwlock_unlock(&rwlock_host_map); 
            push(hostnode);

        }
    }
    return 0;
}

/*
 * find empty item in vector to hold urls which have the same host 
 *
 * */
int UrlAnalyseManager::findEmptyVector() {
    for(int i = 0; i < hostsVector.size(); i++) {
        if (hostsVector[i]->available == true) {
            hostsVector[i]->available = false;
            return i;
        }
    }
    return -1;
}

int UrlAnalyseManager::push(HostNode *host) {
    pthread_rwlock_wrlock(&rwlock_priority_queue);
    hosts.push(host);
    pthread_rwlock_unlock(&rwlock_priority_queue);
    return 0;
}

HostNode *UrlAnalyseManager::pop() {
    HostNode *node = NULL;
    node = hosts.top();
    hosts.pop();
    return node;
}

HostNode *UrlAnalyseManager::getNextHost(int & ipnum ) {
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    HostNode *node = NULL;
    pthread_rwlock_wrlock(&rwlock_priority_queue);
    if (hosts.size() == 0) {
        pthread_rwlock_unlock(&rwlock_priority_queue);
        return NULL;
    } else {
        node = hosts.top();
    }


    if (node->nexttime <= getNow()) {
        if ((++node->retnum) <= icconfig->localipnum)
        {
            ipnum = node->retnum;
            pthread_rwlock_unlock(&rwlock_priority_queue);
            return node;
        }else
        {
            node->retnum = 0;
            node = this->pop(); 
            pthread_rwlock_unlock(&rwlock_priority_queue);
            node->nexttime = getNow() + node->interval; 
            pthread_rwlock_wrlock(&rwlock_host_map); 
            if (node->fifoqueue->empty()) {//urls is empty
                node->emptytimes++;
            } else {
                node->emptytimes = 0;
            }
            if (node->emptytimes > 100) {
                hostToFifoqueue.erase(node->host);
                mylog_info(m_pLogGlobalCtrl->infolog, "nexthost's emptytime exceed 10 so remove if rom host to fifoqueue and hosts node = %d %llu %llu emptytime = %d interval =   %d host = %s size = %d urlsize %d fifourlsize %d - %s:%s:%d", node->id, node->nexttime, getNow(), node->emptytimes, node->interval, node->host, hostToFifoqueue.size(),node->fifoqueue->urlnum,node->fifoqueue->urls.size(),INFO_LOG_SUFFIX);

                node->clear();//this node  can be used by others
                pthread_rwlock_unlock(&rwlock_host_map); 
                mylog_info(m_pLogGlobalCtrl->infolog, "current status hostToFifoqueue = %d hostsVector = %d hosts = %d  - %s:%s:%d", hostToFifoqueue.size(),hostsVector.size(), hosts.size(),INFO_LOG_SUFFIX);
                return NULL;
            } else {
                //push host to time back
                pthread_rwlock_unlock(&rwlock_host_map); 
                push(node);
            }
            if (icconfig->localipnum == 0)
            {
                return node;
            }
            return NULL;
        }
    } else {
        pthread_rwlock_unlock(&rwlock_priority_queue);
        return NULL;
    }
}

UrlNode *UrlAnalyseManager::getUrlFromOuterHtml() {
    ExternalManager *externalManager = InfoCrawler::getInstance()->getExternalManager();
    TaskScheduleManager *taskScheduleManager = InfoCrawler::getInstance()->getTaskScheduleManager();

    
    html_des html;
    bool haveHtml = externalManager->pop_html_from_outer(html);
    if(haveHtml) {
        bool have_task_other = InfoCrawler::getInstance()->getTaskScheduleManager()->increaseTaskUrlNum(html.task_id);
        if (!have_task_other)  {
            externalManager->insert_html_from_outer(html);
            return NULL;
        }

        UrlNode *node = new UrlNode;
        strcpy(node->url, (char *)html.url.c_str());
        node->taskid = html.task_id; 
        node->task = taskScheduleManager->getTask(html.task_id);
        node->type = getType(node->url, node->task);
        node->html = html.htmlcontent;
        return node;
        
    }
    return NULL;
}

UrlNode *UrlAnalyseManager::getUrl() {
    int ipnum = 0;
    HostNode *hostnode = getNextHost(ipnum);
    UrlNode *node = NULL;
    if (!hostnode) {
        return node;
    }
reget:
    if (hostnode->fifoqueue->urlnum > 10)  {
        mylog_info(m_pLogGlobalCtrl->infolog, "host %s host queue len = %d - %s:%s:%d",hostnode->host, hostnode->fifoqueue->urlnum,INFO_LOG_SUFFIX);
    }

	    if (hostnode->fifoqueue->urlnum < 20) {
        list<UrlNode *> urls;
        list<UrlNode *>::iterator iter ;
        pthread_rwlock_wrlock(&rwlock_unfetched_url_table);
        if (hostnode->fifoqueue->urlnum < 20 && hostnode->sqlite_host_id > 0) {
            InfoCrawler::getInstance()->getDbManager()->get_unfetched_urls(urls, hostnode->sqlite_host_id, 1000);
            for(iter = urls.begin(); iter != urls.end(); ++iter) {
                InfoCrawler::getInstance()->getDbManager()->delete_unfetched_url_by_id((*iter)->sqlite_id);
                UrlNode *urlnode = *iter;
                urlnode->other = NULL;
                urlnode->task = InfoCrawler::getInstance()->getTaskScheduleManager()->getTask(urlnode->taskid);
                InfoCrawler::getInstance()->getUrlAnalyseManager()->insertUrl(urlnode, INSERT_URL_NOT_FORCED, false, true);
                //hostnode->push_back(urlnode, true);
            }
        }
        pthread_rwlock_unlock(&rwlock_unfetched_url_table);
    }
    //pthread_rwlock_wrlock(&rwlock_urls_list); 
    node = hostnode->fifoqueue->pop();
    if (node)
    {
        node->ipnum = ipnum - 1;
    }
    //pthread_rwlock_unlock(&rwlock_urls_list); 
    return node;
}

int UrlAnalyseManager::remove(int id) {
    removeNodeFromPriorityQueue(id);
    return 0;
}

int UrlAnalyseManager::removeNodeFromPriorityQueue(int id) {
    vector<HostNode *> tmphosts;
    pthread_rwlock_rdlock(&rwlock_priority_queue);
    while(!hosts.empty()) {
        HostNode *t = hosts.top();
        if (t->id != id) {
            tmphosts.push_back(t);
        }
    }
    pthread_rwlock_unlock(&rwlock_priority_queue);
    for(int i = 0;i < tmphosts.size(); i++) {
        this->push(tmphosts[i]);
    }
    return 0;
}
int UrlAnalyseManager::PutFinalPage(map<string ,UrlNode *> & finalpagelist,UrlNode * urlnode)
{
    int ret = 0;
    map<string ,UrlNode *> ::iterator iter;
    if ((iter = finalpagelist.find(urlnode->url)) == finalpagelist.end())
    {
        finalpagelist[urlnode->url] = urlnode;
        ret = 1;
        mylog_info(m_pLogGlobalCtrl->infolog, "PutFinalPage urls %s  - %s:%s:%d",urlnode->url,INFO_LOG_SUFFIX);
    }
    return ret;

}
/*
 * analyse urls and determine which url will insert into queue, analyse finalpage, sort list.
 */
int UrlAnalyseManager::analyseUrls(UrlNode *urlnode, RESPONSE_HEADER *rheader, char *content, int contentlen, char *nextpageurl, bool no_need_to_analyze_mytype) {
    map<string,UrlNode *> UrlnodeMap;
    map<string,UrlNode *>::iterator mapiter;
    list<UrlNode *> urls;
    map<string ,UrlNode *> finalpagelist;
    list<UrlNode *> otherpagelist;
    list<UrlNode *>::iterator iter;
    CheckpointManager *checkpointManager = InfoCrawler::getInstance()->getCheckpointManager();

    if (contentlen == 0) {
        return 0;
    }

    extractUrlsFromContent(urlnode, rheader, content, urls, nextpageurl);

    TaskOtherInfo *taskother = InfoCrawler::getInstance()->getTaskScheduleManager()->getTaskOtherInfo(urlnode->taskid);
    // this url is homepage or listpage
    if (urlnode->type & URL_TYPE_NEEDTOANALYSEURL) {
        mylog_info(m_pLogGlobalCtrl->infolog, "urls size %d and content length = %d %s - %s:%s:%d", urls.size(), contentlen, urlnode->url,INFO_LOG_SUFFIX);
        if (urlnode->type & URL_TYPE_LISTPAGE )
        {
            if (urlnode->task->sourcetype == SOURCE_TYPE_BBS)
            {
                //add listpagew turn page
                if (nextpageurl[0] == 0) {
                    FinalPage::getInstance()->nextPage(urlnode, urls, nextpageurl);
                }
                if (strcmp(nextpageurl, urlnode->url) == 0 || urlnode->page > InfoCrawler::getInstance()->getConf()->maxlistnextpagetofetch) {
                    nextpageurl[0] = 0;
                }
            }else
            {
                nextpageurl[0] = 0;
            }
        }else if (urlnode->type & URL_TYPE_FINALPAGE) {
            if (nextpageurl[0] == 0) {
                  FinalPage::getInstance()->nextPage(urlnode, urls, nextpageurl);
            }
            if ((strcmp(nextpageurl, urlnode->url) == 0) || (urlnode->page >= InfoCrawler::getInstance()->getConf()->maxfinalnextpagetofetch ) ) {
                  nextpageurl[0] = 0;
            }
        }else if (urlnode->type & URL_TYPE_HOMEPAGE) {
            
            if (urlnode->task->sourcetype == SOURCE_TYPE_COURT)
            {
                if (nextpageurl[0] == 0) {
                      FinalPage::getInstance()->nextPage(urlnode, urls, nextpageurl);
                }
            }else if (urlnode->task->sourcetype == SOURCE_TYPE_COMPANY)
            {
                if (nextpageurl[0] == 0) {
                      FinalPage::getInstance()->nextPage(urlnode, urls, nextpageurl);
                }
            }else
            {
                    nextpageurl[0] = 0;
            }
        }
        if (urlnode->layerid <= urlnode->task->depth)
        {
           /* if (urlnode->task->sourcetype == SOURCE_TYPE_BBS)
            {
                for(iter = urls.begin(); iter != urls.end(); ) {
                    UrlNode *nodetmp = *iter;
                    if ((mapiter = UrlnodeMap.find(nodetmp->url)) != UrlnodeMap.end())
                    {
                        int lennew = 0,lenold = 0;
                        lennew = strlen(nodetmp->title);
                        lenold= strlen((mapiter->second)->title);
                        if (lennew > lenold)
                        {
                            list<UrlNode *>::iterator itertmp;
                            UrlNode *nodempatmp = mapiter->second;
                            //delete old iter
                            if ((itertmp = find(urls.begin(),urls.end(),nodempatmp))!= urls.end())
                            {
                                delete mapiter->second;
                                urls.erase(itertmp);
                                iter++;
                                mapiter->second = nodetmp;
                            }else
                            {
                                iter++;

                            }
                        }else
                        {
                            //delete new iter
                            delete nodetmp; 
                            iter = urls.erase(iter);
                        }
                    }else
                    {
                        iter++;
                        UrlnodeMap.insert(make_pair(nodetmp->url, nodetmp));
                        //Debug(0, 1, ("Debug:analyse insert new URL is %s new title is %s \n",nodetmp->url,nodetmp->title));
                    }
                }
            }*/
            int pagenumtmp = 3;
            if (urlnode->type & URL_TYPE_LISTPAGE) {
                urlnode->nextpage = 2;
//                urlnode->page ++;
                UrlNode *newnode = new UrlNode;
                UrlNode *newnode1 = new UrlNode;
                UrlNode *newnode2 = new UrlNode;
                GetUserName(urlnode->url ,urlnode , newnode,newnode1,newnode2);
                if (newnode->url[0])
                {
                    newnode->id = urlnode->id;
//                    newnode->page = urlnode->page ++;
                    newnode->task = urlnode->task;
                    newnode->taskid = urlnode->taskid;
                    newnode->copyother(urlnode->other,urlnode->maxtype);
                    newnode->type = getType(newnode->url, urlnode->task);
                    newnode->layerid = urlnode->layerid +1;
                    newnode->needtologin = urlnode->needtologin;
                    newnode->copyfatherurl(urlnode->url);
                    newnode->taskbatch = urlnode->taskbatch;
                    newnode->nextpage = 2;
                    //finalpagelist.push_back(newnode);
                    if (PutFinalPage(finalpagelist,newnode)==1)
                    {

                        newnode->page = urlnode->page ++;
                    }else
                    {

                        delete newnode;
                    }
                }else
                {
                    delete newnode;
                }
                if (newnode1->url[0])
                {
                    newnode1->id = urlnode->id;
//                    newnode1->page = urlnode->page ++;
                    newnode1->task = urlnode->task;
                    newnode1->taskid = urlnode->taskid;
                    newnode1->copyother(urlnode->other,urlnode->maxtype);
                    newnode1->type = getType(newnode1->url, urlnode->task);
                    newnode1->layerid = urlnode->layerid +1;
                    newnode1->needtologin = urlnode->needtologin;
                    newnode1->copyfatherurl(urlnode->url);
                    newnode1->taskbatch = urlnode->taskbatch;
                    newnode1->nextpage = 2;
                    if (PutFinalPage(finalpagelist,newnode1)==1)
                    {

                        newnode1->page = urlnode->page ++;
                    }else
                    {

                        delete newnode1;
                    }
                    //finalpagelist.push_back(newnode1);
                }else
                {
                    delete newnode1;
                }
                
                if (newnode2 ->url[0])
                {
                    newnode2->id = urlnode->id;
                    newnode2->task = urlnode->task;
                    newnode2->taskid = urlnode->taskid;
                    newnode2->copyother(urlnode->other,urlnode->maxtype);
                    newnode2->type = getType(newnode2->url, urlnode->task);
                    newnode2->layerid = urlnode->layerid +1;
                    newnode2->needtologin = urlnode->needtologin;
                    newnode2->copyfatherurl(urlnode->url);
                    newnode2->taskbatch = urlnode->taskbatch;
                    newnode2->nextpage = 2;
                    if (PutFinalPage(finalpagelist,newnode2)==1)
                    {

                        newnode2->page = urlnode->page ++;
                    }else
                    {

                        delete newnode2;
                    }
                    pagenumtmp = 5;
                }else
                {
                    delete newnode2;
                }
            }
            for(iter= urls.begin(); iter!= urls.end(); ++iter) {
                UrlNode *newnode = *iter;
                string rr1="http://www.qq745609656.qiyesou.com/introduce/";
                string rr2="http://www.qq745609656.qiyesou.com/contact/";
                if( rr1==string(newnode->url) || rr2==string(newnode->url) )
                {
                    mylog_info(m_pLogGlobalCtrl->infolog, "type_check: %d  new URL:%s  listurl: %s  tuep: %d - %s:%s:%d",newnode->type, newnode->url, urlnode->url,urlnode->type,INFO_LOG_SUFFIX);
                }
                if (urlnode->task->sourcetype == SOURCE_TYPE_SEARCHENGINE)
                {
                    newnode->type = getType(SOURCE_TYPE_SEARCHENGINE);
                } else {

                    newnode->type = getType(newnode->url, urlnode->task);
                }
                if ((newnode->type & URL_TYPE_FINALPAGE) && (urlnode->task->sourcetype == SOURCE_TYPE_TOPIC))
                {
                    newnode->type = getType(SOURCE_TYPE_TOPIC);
                    newnode->copytopicsource(urlnode->title);
                }
                if (no_need_to_analyze_mytype) {
                    if (newnode->type == urlnode->type) {
                        delete newnode;
                        continue;
                    }
                }
                //mylog_info(m_pLogGlobalCtrl->infolog, "type %d ALL URL  %s father url %s  - %s:%s:%d",newnode->type, newnode->url,urlnode->url,INFO_LOG_SUFFIX);
                if (newnode->type & URL_TYPE_NEEDTOADDNEW) {
                   if ((urlnode->task->sourcetype == SOURCE_TYPE_COMPANY))
                   {
                       if ((urlnode->type & URL_TYPE_LISTPAGE) && (newnode->type & URL_TYPE_FINALPAGE)) {
                           newnode->id = urlnode->id;
//                           newnode->page = urlnode->page ++;
                           if (newnode->page < pagenumtmp )
                           {
                                newnode->nextpage = 2;
                           }
                       }
                   }
                    newnode->task = urlnode->task;
                    newnode->taskid = urlnode->taskid;
                    newnode->copyother(urlnode->other,urlnode->maxtype);
                    newnode->layerid = urlnode->layerid + 1;
                    newnode->needtologin = urlnode->needtologin;
                    newnode->copyfatherurl(urlnode->url);
                    newnode->taskbatch = urlnode->taskbatch;
                    if (newnode->type & URL_TYPE_FINALPAGE) {
                        if ((urlnode->task->sourcetype == SOURCE_TYPE_TOPIC))
                        {
                            if ((newnode->layerid == urlnode->task->depth))
                            {
                                finalpagelist[newnode->url]=newnode;
                            }else
                            {
                                delete newnode;
                            }
                        }else
                        {
                            if ((urlnode->task->sourcetype == SOURCE_TYPE_COMPANY) &&  (urlnode->type & URL_TYPE_HOMEPAGE) && (newnode->type & URL_TYPE_FINALPAGE))
                            {
                                delete newnode;
                            }else
                            {
                                if (PutFinalPage(finalpagelist,newnode)==1)
                                {

                                    newnode->page = urlnode->page ++;
                                }else
                                {

                                    delete newnode;
                                }
                            }
                        }
                    } else if ((newnode->type & URL_TYPE_LISTPAGE) || (newnode->type & URL_TYPE_HOMEPAGE)) {
                        if(newnode->layerid <= urlnode->task->depth)
                        {
                            otherpagelist.push_back(newnode);
                        }else
                        {
                            delete newnode;
                        }
                    }else
                    {
                        delete newnode;
                    }
                } else {
                    delete newnode;
                }
            }
        }else
        {
            for(iter = urls.begin(); iter != urls.end(); ++iter) {
                UrlNode *newnode = *iter;
                delete newnode;
            }

        }
    } else if (urlnode->type & URL_TYPE_FINALPAGE) {
        //add finalpage turn page
        if (nextpageurl[0] == 0) {
            FinalPage::getInstance()->nextPage(urlnode, urls, nextpageurl);
        }
        if (urlnode->task->sourcetype == SOURCE_TYPE_SEARCHENGINE)
        {
            if ((strcmp(nextpageurl, urlnode->url) == 0) || (urlnode->page >= 10 ) ) {
                nextpageurl[0] = 0;
            }
        }else  if (urlnode->task->sourcetype == SOURCE_TYPE_COMPANY)
        {
                nextpageurl[0] = 0;
        }else
        {
            if ((strcmp(nextpageurl, urlnode->url) == 0) || (urlnode->page >= InfoCrawler::getInstance()->getConf()->maxfinalnextpagetofetch ) ) {
                nextpageurl[0] = 0;
            }
        }

        for(iter = urls.begin(); iter != urls.end(); ++iter) {
            UrlNode *newnode = *iter;
            delete newnode;
        }
    } else {
        nextpageurl[0] = 0;
        for(iter = urls.begin(); iter != urls.end(); ++iter) {
            UrlNode *newnode = *iter;
            delete newnode;
        }
    }

    if (urlnode->type & URL_TYPE_LISTPAGE)      urlnode->totalpage = urlnode->page;
    //firstly insert final page urls, so that it will be fetched first, secondly insert other urls
    for(mapiter = finalpagelist.begin(); mapiter != finalpagelist.end(); ++mapiter) 
    {    
        UrlNode *nodetmp =mapiter->second;
        if (urlnode->task && (!(is(nodetmp->url,urlnode->task->ignoreurl)))) {
            nodetmp->totalpage = urlnode->page;
            mylog_info(m_pLogGlobalCtrl->infolog, "url %s  tadkid %d page %d totalpage %d urlnode->page %d - %s:%s:%d", nodetmp->url,urlnode->taskid,nodetmp->page,nodetmp->totalpage,urlnode->page,INFO_LOG_SUFFIX);
            if (nodetmp->nextpage >1)
            {
                insertUrl(nodetmp, INSERT_URL_NOT_FORCED, false);
            }else
            {
                insertUrl(nodetmp);
            }
        } else {
            mylog_info(m_pLogGlobalCtrl->infolog, "IGNORE URL %s,TASK ID %d - %s:%s:%d", nodetmp->url,urlnode->taskid,INFO_LOG_SUFFIX);
            delete nodetmp;
        }

    }
    //shield url
    for(iter = otherpagelist.begin(); iter != otherpagelist.end(); ++iter) {
        if (urlnode->task && (!(is((*iter)->url, urlnode->task->ignoreurl)))) {
            insertUrl(*iter);
        } else
        {
            mylog_info(m_pLogGlobalCtrl->infolog, "IGNORE URL %s,TASK ID %d - %s:%s:%d", (*iter)->url,urlnode->taskid,INFO_LOG_SUFFIX);
            delete *iter;
        }
    } 

    if (nextpageurl[0]) {
        return urlnode->page + 1; //have next page
    }

    return 1;
}

void UrlAnalyseManager::extractUrlsFromContent(UrlNode *urlnode, RESPONSE_HEADER *rheader, char *content, list<UrlNode *> &urls, char *nextpageurl) {
    struct uri page_uri;
    pthread_mutex_lock(&mutexDetect);
    if (rheader->location.empty())
    {
        uri_parse_string(urlnode->url, &page_uri);
    }
    else
    {
        uri_parse_string(rheader->location.c_str(), &page_uri);
    }

    HashTable hashtablenextnum = create_hash_table(383); //should be prime number
    int ret = 0;
    char nextpagenum[10] ;nextpagenum[0] = 0;
    sprintf(nextpagenum, "%d", urlnode->page + 1);
    hash_find_or_add(hashtablenextnum, nextpagenum, strlen(nextpagenum), 0, &ret);
    sprintf(nextpagenum, "[%d]", urlnode->page + 1);
    hash_find_or_add(hashtablenextnum, nextpagenum, strlen(nextpagenum), 0, &ret);
    sprintf(nextpagenum, "%02d", urlnode->page + 1);
    hash_find_or_add(hashtablenextnum, nextpagenum, strlen(nextpagenum), 0, &ret);
    sprintf(nextpagenum, "[%02d]", urlnode->page + 1);
    hash_find_or_add(hashtablenextnum, nextpagenum, strlen(nextpagenum), 0, &ret);

    struct onfindpackage p = {this, urlnode, &urls, nextpageurl, hashtablenextnum, hashtablenextpage};
    hlink_detect_string(content, &page_uri, onfind, &p);
    uri_destroy(&page_uri);

    free_hash_table(hashtablenextnum);

    pthread_mutex_unlock(&mutexDetect);
}

int onfind(const char *elem, const char *attr, struct uri *uri, void *arg) {
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    struct onfindpackage *p = (struct onfindpackage*)arg;
    char buff[MAX_URL_LEN - 1] ;buff[0] = 0;
    UrlAnalyseManager *analyseManager = (UrlAnalyseManager *)p->manager;
    list<UrlNode *> *urls = p->urls;
    //if (uri_combine(uri, buff, MAX_URL_LEN - 1, C_SCHEME | C_AUTHORITY | C_PATH | C_QUERY ) >= 0)
    uri_combine(uri, buff, MAX_URL_LEN - 1, C_SCHEME | C_AUTHORITY | C_PATH | C_QUERY );
    if (true)
    {
        /*FILE * f = fopen("buff.txt","a+");
        fwrite(buff,1,strlen(buff),f);
        fwrite("\n",1,1,f);
        fclose(f);*/
//        mylog_info(m_pLogGlobalCtrl->infolog, "buff  %s - %s:%s:%d",buff, INFO_LOG_SUFFIX);
        UrlNode *node = new UrlNode;
        node->copyurl(buff); 
        char *anchor_text_out = NULL;
        char *tmpanchor_text = NULL;
        char *anchor_text_outtmp = NULL;
        HtRegex regex;
        if (uri->anchor_text)
        {
           /* anchor_text_out = (char *)malloc(strlen(uri->anchor_text) + 10);
            memset(anchor_text_out,0,strlen(uri->anchor_text) + 10);
            //VALGRIND_PRINTF("anchor_text %s url %s fatherurl %s ", uri->anchor_text,buff,node->url);
            killhtmlcode(uri->anchor_text, anchor_text_out);
            if (anchor_text_out[0])
            {   
                tmpanchor_text = strtrim(anchor_text_out, NULL);
                node->copytitle(tmpanchor_text);
                Debug(0,1,("anchor text %s ,anchor_text_out %s\n",uri->anchor_text,tmpanchor_text));
            }*/
           anchor_text_outtmp =  (char*) malloc(strlen(uri->anchor_text) + 10);
            //memset(anchor_text_outtmp,0,strlen(uri->anchor_text) + 10);
            regex.replace(uri->anchor_text, "<[^>]+>",  "", anchor_text_outtmp);
            if (anchor_text_outtmp)
            {
                tmpanchor_text = strtrim(anchor_text_outtmp, NULL);
                node->copytitle(tmpanchor_text);
                //Debug(0,1,("anchor text %s ,anchor_text_out %s\n",uri->anchor_text,anchor_text_outtmp));
            }
        }
        //Debug(0,2, ("Debug: buff %s,%s extracted url %s anchor text = %s,title %s from onfind\n",buff ,p->node->url, node->url, uri->anchor_text ? uri->anchor_text : "",node->title));
//        mylog_info(m_pLogGlobalCtrl->infolog, "Debug: buff %s,%s extracted url %s anchor text = %s,title %s from onfind - %s:%s:%d\n",buff ,p->node->url, node->url, uri->anchor_text ? uri->anchor_text : "",node->title, INFO_LOG_SUFFIX);
        urls->push_back(node);
        if ((p->node->type & URL_TYPE_FINALPAGE) || (p->node->type & URL_TYPE_LISTPAGE) || (p->node->type & URL_TYPE_HOMEPAGE)) {
            if (tmpanchor_text && tmpanchor_text[0]) {
                int len = strlen(tmpanchor_text);
                if (p->nextpageurl[0] == 0 )
                {
                    if ( len <= 4 ) {
                        HashEntry entry = hash_find(p->hashtablenextnum, tmpanchor_text, len);
                        if (entry)
                        {
                            strcpy(p->nextpageurl, buff);
                        }
                    } else { //for get nextpage url
                        HashEntry entry = hash_find(p->hashtablenextpage, tmpanchor_text, len);
                        if (entry)
                        {
                            strcpy(p->nextpageurl, buff);
                        }
                    }
                }
            }
        }
        if (anchor_text_outtmp) free(anchor_text_outtmp);
        if (anchor_text_out) free (anchor_text_out);
    }
    uri_destroy(uri);
    free(uri);
    return 0;
}
int UrlAnalyseManager::getHomeType()
{
//    return URL_TYPE_HOMEPAGE | URL_TYPE_NEEDTODOWN | URL_TYPE_NEEDTOANALYSEURL | URL_TYPE_NEEDTOADDNEW | URL_TYPE_NEEDTOSAVE ;
    return URL_TYPE_HOMEPAGE | URL_TYPE_NEEDTODOWN | URL_TYPE_NEEDTOANALYSEURL;
}
int UrlAnalyseManager::getType(char *url, Task *task) {
    if (!task) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "task is null - %s:%s:%d:%d", ERROR_LOG_SUFFIX);
        return URL_TYPE_NOTYPE;
    }
    //Debug(0, 2, ("Debug: %s matched %s %d\n", url, task->ultimatepageurl, is(url, task->ultimatepageurl)));
    
    char ultimatepageurl[MAX_URL_LEN_DATA_SOURCE];
    ultimatepageurl[0] = 0;
    if ((task->sourcetype == SOURCE_TYPE_BBS) && (!task->ultimatepageurl[0]))
    {
        strcpy(ultimatepageurl, task->bbsultimatepageurl);
    }else
    {
        strcpy(ultimatepageurl,task->ultimatepageurl);
    }
    if (is(url, ultimatepageurl)) {
        if (task->sourcetype ==SOURCE_TYPE_INFO)
        {
            return URL_TYPE_FINALPAGE | URL_TYPE_NEEDTODOWN | URL_TYPE_NEEDTOSAVE | URL_TYPE_NEEDTOADDNEW |URL_TYPE_NEEDTOANALYSEURL;
        }else
        {
            return URL_TYPE_FINALPAGE | URL_TYPE_NEEDTODOWN | URL_TYPE_NEEDTOSAVE | URL_TYPE_NEEDTOADDNEW ;
        }
    } else if (is(url, task->pageurl)) {
        return URL_TYPE_LISTPAGE | URL_TYPE_NEEDTODOWN | URL_TYPE_NEEDTOANALYSEURL |URL_TYPE_NEEDTOSAVE| URL_TYPE_NEEDTOADDNEW;
    } else if (is(url, task->homeurl)) {
        return URL_TYPE_HOMEPAGE | URL_TYPE_NEEDTODOWN | URL_TYPE_NEEDTOANALYSEURL | URL_TYPE_NEEDTOADDNEW;
        //return URL_TYPE_NOTYPE;
    } else {
        return URL_TYPE_NOTYPE;
    }
}

int UrlAnalyseManager::getType(int urltype) {
    if (urltype == URL_TYPE_BBS_FINALPAGE)
    {
        return URL_TYPE_BBS_FINALPAGE | URL_TYPE_NEEDTODOWN | URL_TYPE_NEEDTOSAVE | URL_TYPE_NEEDTOADDNEW | URL_TYPE_NEEDTOANALYSEURL;
    }else if (urltype == URL_TYPE_FINALPAGE)
    {
        return URL_TYPE_FINALPAGE | URL_TYPE_NEEDTODOWN | URL_TYPE_NEEDTOSAVE | URL_TYPE_NEEDTOADDNEW | URL_TYPE_NEEDTOANALYSEURL;
    }else if (urltype == URL_TYPE_LISTPAGE)
    {
        return URL_TYPE_LISTPAGE | URL_TYPE_NEEDTODOWN | URL_TYPE_NEEDTOANALYSEURL | URL_TYPE_NEEDTOADDNEW;
    }else if (urltype == SOURCE_TYPE_TOPIC)
    {
        return URL_TYPE_FINALPAGE | URL_TYPE_NEEDTODOWN | URL_TYPE_NEEDTOSAVE | URL_TYPE_NEEDTOADDNEW ;
    }else if (urltype == SOURCE_TYPE_SEARCHENGINE)
    {
        return URL_TYPE_FINALPAGE | URL_TYPE_NEEDTODOWN | URL_TYPE_NEEDTOSAVE | URL_TYPE_NEEDTOADDNEW ;
    }
}
/*bool UrlAnalyseManager::is(char *url, char *pattern) {
    char *pToken = NULL;
    HtRegex regex;
    char *tokbuff = NULL;
    int len = strlen(pattern);
    char *patterntmp = new char[len + 1];
    patterntmp[0] = 0;
    strncpy(patterntmp , pattern,len);
    patterntmp[len] = 0;
    pToken = strtok_r(patterntmp, "\r\n\t",&tokbuff);
    while(pToken)
    {
        regex.set(pToken);
        if (regex.match(url, 0, 0) == 1) {
            if (patterntmp) delete [] patterntmp;
            return true;
        }
        pToken = strtok_r(NULL, "\r\n\t",&tokbuff);
    }
    if (patterntmp) delete [] patterntmp;
    return false;
} */


bool UrlAnalyseManager::is(char* url,char * format)
{
       if(format ==NULL || url ==NULL)
                  return false;
       boost::cmatch matchwhat;
       try
       {
          boost::regex expression(format,boost::regex::normal|boost::regex::icase);

          if(boost::regex_match(url,matchwhat,expression))
          {
              return true;
          }
       }
       catch(...)
       {
           return false;
       }

       return false;
}

bool UrlAnalyseManager::isHomeUrl(char *url, char *homeurl) {
    char *pToken = NULL;
    char *tokbuff = NULL;
    int len = strlen(homeurl);
    char *homeurltmp= new char[len + 1];
    homeurltmp[0] = 0;
    strncpy(homeurltmp, homeurl ,len);
    homeurltmp[len] = 0;
    pToken = strtok_r(homeurltmp, "\r\n\t ",&tokbuff);
    while(pToken)
    {
        if (strcmp(url, pToken) == 0)
        {
            if (homeurltmp) delete [] homeurltmp; 
            return true;
        }
        pToken = strtok_r(NULL, "\r\n\t ",&tokbuff);
    }
    if (homeurltmp) delete [] homeurltmp;
    return false;
}
void UrlAnalyseManager::dump() {

}
void UrlAnalyseManager::retrieve() {

}
void killhtmlcode(char *rin,char *rout)
{
    int nlen = strlen(rin);
    int l=0;
    char *p=rin;
    char *p1=NULL;
    int nfind=0;

    char *rtemp = new char [nlen +10];

    rtemp[0] = 0;
    rout[0]=0;

    while(*p)
    {
        if( *p != '<' )
        {
            rtemp[l++] = *p++;
        }
        else
        {
            p1 = p;
            p1++;

            if( *p1=='>' )
            {
                nfind = 1;
            }
            else if( *p1 < 0 )
            {
                nfind = 1;
            }
            int j;
            for(j = 1; j < 200 && nfind < 1 && *p1 ;j++)
            {
                if( *p1 == '<')
                {
                    nfind = 1;
                }
                else if( *p1 == '>')
                {
                    nfind = 2;
                }
                else if( j > 180 )
                {
                    nfind = 1;
                }

                p1++;
            }

            if( nfind == 1 )
            {
                rtemp[l++]=*p++;
            }
            else if( nfind == 2 )
            {
                p += j;
            }
            nfind = 0;
        }
    }
    rtemp[l]=0;
    strcpy(rout,rtemp);

    delete [] rtemp;
}
int  UrlAnalyseManager::hostToFifoqueuesize()
{
    return hostToFifoqueue.size();
}
int UrlAnalyseManager::GetUserName(char * url,UrlNode * node,UrlNode * newnode,UrlNode * newnode1, UrlNode * newnode2)
{
    string urlfrees = "http://china.alibaba.com/company/detail/";
    string urlfreee = ".html";
    string urlcharges = "http://";
    string urlchargee = ".cn.alibaba.com";
    string urltmp = url;
    int begin = 0,end = 0;
    if ((begin = urltmp.find(urlfrees))!= string ::npos)
    {
        if ((end= urltmp.find(urlfreee))!= string ::npos)
        {
            string username = urltmp.substr(begin + urlfrees.length(), end - begin - urlfrees.length());
            node->copyusername((char *)username.c_str());
        }
        
        if (node->username[0])
        {
            string urltmp = "";
            urltmp = "http://china.alibaba.com/member/ajax/memberCardJson.htm?callback=jQuery&loginId=";
            urltmp.append(node->username);
            newnode1->copyurl((char *)urltmp.c_str()); 
        }

    }else if ((begin = urltmp.find(urlcharges))!= string ::npos)
    {
        if ((end = urltmp.find(urlchargee))!= string ::npos)
        {
            string username = urltmp.substr(begin + urlcharges.length(), end - begin - urlcharges.length());
            node->copyusername((char *)username.c_str());
        }
    }
    if (node->username[0])
    {
        string urltmp = "";
        urltmp = "http://china.alibaba.com/member/bizcomm_profile_front_interface.htm?memberId=";
        urltmp.append(node->username);
        urltmp.append("&iframe_delete=true");
        newnode->copyurl((char *)urltmp.c_str()); 
        
        urltmp = "";
        urltmp = "http://club.china.alibaba.com/misc/memberajax/memberInfoForAjax.html?memberId=";
        urltmp.append(node->username);
        urltmp.append("&iframe_delete=true");
        newnode2->copyurl((char *)urltmp.c_str());
    }
}

void UrlAnalyseManager::get_host_sqlite_id(HostNode *hostnode) {
    if (hostnode->sqlite_host_id == -1) {
        int ret = InfoCrawler::getInstance()->getDbManager()->get_hostid_by_domain(hostnode->host, hostnode->sqlite_host_id);
        if (ret == STATE_SUCCESS) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "can not get sqlite_host_id for host  - %s:%s:%d:%d", hostnode->host, INFO_LOG_SUFFIX);
        }
    }
}


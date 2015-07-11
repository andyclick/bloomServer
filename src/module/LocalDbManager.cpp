#include <typeinfo>
#include <zlib.h>
#include "LocalDbManager.h"
#include "DBAccess.h"
#include "FSBigFile.h"
#include "InfoCrawler.h"
#include "utilcpp.h"

using namespace std;
pthread_rwlock_t LocalDbManager::rwlock_fifoqueu_file = PTHREAD_RWLOCK_INITIALIZER; 
pthread_rwlock_t LocalDbManager::rwlock_read_fifoqueu_file = PTHREAD_RWLOCK_INITIALIZER; 
pthread_rwlock_t LocalDbManager::rwlock_socket_file = PTHREAD_RWLOCK_INITIALIZER; 

LocalDbManager::LocalDbManager(LogGlobalCtrl * pLogGlobalCtrl) {
    m_pLogGlobalCtrl = pLogGlobalCtrl;
}

LocalDbManager::~LocalDbManager() {
}

int LocalDbManager::start() {
    Manager::start();

    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    char filename[128] ;filename[0] = 0;
    sprintf(filename, "%s/%s", icconfig->dbpath, FETCHED_FIFO_QUEUE_POS_FILE);

    FILE *f  = fopen(filename, "r+");
    if (!f) {
        f  = fopen(filename, "w+");
        if (f) {
            char key[32] ;key[0] = 0;
            int keylen = sprintf(key, "%llu", (ulonglong)0);
            fwrite(key, 1, keylen, f);
            fclose(f);
        }
    } else {
        fclose(f);
    }

    sprintf(filename, "%s/%s", icconfig->dbpath, DELETE_READ_FIFO_QUEUE_POS_FILE);

    f  = fopen(filename, "r+");
    if (!f) {
        f  = fopen(filename, "w+");
        if (f) {
            char key[32] ;key[0] = 0;
            int keylen = sprintf(key, "%llu", (ulonglong)0);
            fwrite(key, 1, keylen, f);
            fclose(f);
        }
    } else {
        fclose(f);
    }
    
    sprintf(filename, "%s/%s", icconfig->dbpath, DELETE_READ_FIFO_QUEUE_FLAG_FILE);

    f  = fopen(filename, "r+");
    if (!f) {
        f  = fopen(filename, "w+");
        if (f) {
            char key[32] ;key[0] = 0;
            int keylen = sprintf(key, "%d", (int)0);
            fwrite(key, 1, keylen, f);
            fclose(f);
        }
    } else {
        fclose(f);
    }
   
    return 0;
}

int LocalDbManager::stop() {
    return Manager::stop();
}
/*
 *  *0, ok     
 *   *1, fail
 *    *-1, not found 
 *     */
int LocalDbManager::erasecontent_db(ulonglong id ,int taskid) {
    char dbname[64] ;dbname[0] = 0;
    char keyname[64] ;keyname[0] = 0;
    char Memcachedbkeyname[64] ;Memcachedbkeyname[0] = 0;
    int ret = -1;  
    getContentDBName(taskid, dbname);
    getRecordKeyName(id, keyname);
    getMemcachedbKeyName(id, taskid, Memcachedbkeyname);
#ifdef HTMLMEMCACHEDB
    int len = strlen (Memcachedbkeyname);
    MemCacheClient *mc = InfoCrawler::getInstance()->getHtmlMcLocalThread();
    MemCacheClient::MemRequest req;
    req.mKey  = Memcachedbkeyname;
    //____BEGINTIMER
    mc->Del(req);
    //____ENDTIMER
    
    int re_connect = 0;
    while(req.mResult == MCERR_NOSERVER)
    {
        if (!running())
        {
            return -1;
        } else {
            mylog_error(m_pLogGlobalCtrl->errorlog, "CAN NTO CONNECT TO MEMCACHEDB key %s - %s:%s:%d", Memcachedbkeyname,INFO_LOG_SUFFIX);
            req.mData.SetReadBufLenEmpty();
            mc->Del(req);
        }
        my_sleep(500000); //0.5 seconds
        
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not erasecontent %s - %s:%s:%d", Memcachedbkeyname,INFO_LOG_SUFFIX);
        if (re_connect++ >= MEMCACHEDB_RECONNECT_NUM )
        {
            mylog_error(m_pLogGlobalCtrl->errorlog, "CAN NTO RECONNECT TO MEMCACHEDB key %s re_connect num %d - %s:%s:%d", Memcachedbkeyname,re_connect,INFO_LOG_SUFFIX);
            req.mResult = MCERR_NOTFOUND;
            break;
        }
    }
    if (req.mResult == MCERR_NOSERVER)
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "memcachedb delete ERROR urlid %llu taskid %d - %s:%s:%d", id, taskid,INFO_LOG_SUFFIX);
    }else if (req.mResult == MCERR_NOTFOUND)
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "NOT_FOUND urlid %llu taskid %d when memcachedb delete - %s:%s:%d", id, taskid,INFO_LOG_SUFFIX);
    }else if (req.mResult == MCERR_OK ) 
    {
        mylog_info(m_pLogGlobalCtrl->infolog, "MEMCACHEDB Delete Task content success id %llu task %d - %s:%s:%d", id , taskid,INFO_LOG_SUFFIX);
    }
#else
    DBAccess *access = DBAccess::getInstance();

    int suffix = access->load(dbname);
    string fileno;

    DBD *dbd = access->get(suffix, fileno, keyname, NULL);
    ret  = -1;
    if (dbd)
    {
        ret = access->erase(suffix, keyname, fileno, NULL);
    }else
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not get DBD in erase DB%s %s - %s:%s:%d", dbname, keyname,INFO_LOG_SUFFIX);
    }
    dbd_free(dbd);    
    if (ret == 0) 
    {
        mylog_info(m_pLogGlobalCtrl->infolog, "Delete Task content success id %llu task %d  - %s:%s:%d", id, taskid,INFO_LOG_SUFFIX);
    }
#endif
    return ret;
}
void LocalDbManager::altertotalpageforced(UrlNode *urlnode)
{
    char dbname[64] ;dbname[0] = 0;
    char recordname[64] ;recordname[0] = 0;
    char memcachedbkey[64] ;memcachedbkey[0] = 0;
    char urldbname[64] ;urldbname[0]= 0;
    DBAccess *dbaccess = DBAccess::getInstance();

    getContentDBName(urlnode, dbname);
    getRecordKeyName(urlnode, recordname);
    getMemcachedbKeyName(urlnode->id, urlnode->taskid, memcachedbkey);
    getUrlDBName(urlnode, urldbname);
#ifdef HTMLMEMCACHEDB
    MemCacheClient::MemRequest req;
    int len = strlen (memcachedbkey);
    MemCacheClient *mc = InfoCrawler::getInstance()->getHtmlMcLocalThread();

    req.mKey  = memcachedbkey;
    mc->Get(req);
    int re_connect = 1;
    while(req.mResult == MCERR_NOSERVER)
    {       
        if (!(running()))
        {
            return; 
        }else
        {
            mylog_error(m_pLogGlobalCtrl->errorlog, "CAN NTO CONNECT TO MEMCACHEDB key %s - %s:%s:%d", memcachedbkey, INFO_LOG_SUFFIX);
            req.mData.Deallocate();
            mc->Get(req);
        }
        my_sleep(500000); //0.5 seconds
        if (re_connect++ >= MEMCACHEDB_RECONNECT_NUM )
        {
            mylog_error(m_pLogGlobalCtrl->errorlog, "CAN NTO CONNECT TO MEMCACHEDB key %s re_connect num %d - %s:%s:%d", memcachedbkey, re_connect,INFO_LOG_SUFFIX);
            req.mResult = MCERR_NOTFOUND;
            break;
        }
    }

    if (req.mResult != MCERR_NOTFOUND){
        mylog_info(m_pLogGlobalCtrl->infolog, "CONTENT CAN NOT FOUND IN MEMCACHEDB key %s datalen %d - %s:%s:%d", memcachedbkey, req.mData.GetReadSize(),INFO_LOG_SUFFIX);
#else
        int suffix = dbaccess->load(dbname);
        int suffixurl = dbaccess->load(urldbname);
        string fileno;
        DBD *dbd = dbaccess->get(suffix, fileno, recordname, NULL);
        if (dbd != NULL) { //new record
#endif
            ContentHead *head = new ContentHead;
#ifdef HTMLMEMCACHEDB
            readHead((char *)req.mData.GetReadBuffer(), head);
#else
            readHead(dbd->datbuf, head);
#endif

            int lastheadlength = head->headlength;
            head->totalpage = head->nowpage;
            char *headbuf = NULL;
            writeHead(&headbuf, head);
            char *storedata = (char *)malloc(dbd->datlen_u + 100);
            memcpy(storedata, headbuf, head->headlength);
            memcpy(storedata + head->headlength, dbd->datbuf + lastheadlength, dbd->datlen_u - lastheadlength);
            int ret = dbaccess->store(suffix, recordname, storedata, dbd->datlen_u - lastheadlength + head->headlength, fileno, NULL);
            if (ret == 0 || ret == 1) {
            } else {
            mylog_error(m_pLogGlobalCtrl->errorlog, "can't store content for changed head %d %d %s %s - %s:%s:%d", lastheadlength, head->headlength, dbname, recordname,INFO_LOG_SUFFIX);
            }
            freeHead(head);
            free(headbuf);
            free(storedata);
            dbd_free(dbd);
        }
    }
/*
 * file format
 * taskid/cookie
 */
/*
 * file format
 * headlength/totallength/totalpage/nowpage/page1_id/page1_offset/page1_length/page/page2_id/page2_offset/page2_length/page1content/page2conent/page
 */
int LocalDbManager::savecontent(UrlNode *urlnode, RESPONSE_HEADER *rheader, char *content, int contentlength, int nextpage) {
    char dbname[64] ;dbname[0] = 0;
    char recordname[64] ;recordname[0] = 0;
    char memcachedbkey[64] ;memcachedbkey[0] = 0;
    char urldbname[64] ;urldbname[0] = 0;
    DBAccess *dbaccess = DBAccess::getInstance();

    getContentDBName(urlnode, dbname);
    getRecordKeyName(urlnode, recordname);
    getMemcachedbKeyName(urlnode->id, urlnode->taskid, memcachedbkey);
    getUrlDBName(urlnode, urldbname);
#ifdef HTMLMEMCACHEDB
    MemCacheClient::MemRequest req;
    int len = strlen (memcachedbkey);
    MemCacheClient *mc = InfoCrawler::getInstance()->getHtmlMcLocalThread();
    
    req.mKey  = memcachedbkey;
    //____BEGINTIMER
    mc->Get(req);
    //____ENDTIMER
    int re_connect = 1;
    while(req.mResult == MCERR_NOSERVER)
    {
        if (!(running()))
        {
            return req.mResult;
        }else
        {
            mylog_error(m_pLogGlobalCtrl->errorlog, "CAN NTO CONNECT TO MEMCACHEDB key %s - %s:%s:%d", memcachedbkey,INFO_LOG_SUFFIX);
            req.mData.Deallocate();
            mc->Get(req);
        }
        my_sleep(500000); //0.5 seconds
        if (re_connect++ >= MEMCACHEDB_RECONNECT_NUM )
        {
            mylog_error(m_pLogGlobalCtrl->errorlog, "CAN NTO RECONNECT TO MEMCACHEDB key %s , re_connect num %d - %s:%s:%d",memcachedbkey, re_connect,INFO_LOG_SUFFIX);
            req.mResult = MCERR_NOTFOUND;
            break;
        }
    }

    if (req.mResult == MCERR_NOTFOUND){
   
        mylog_info(m_pLogGlobalCtrl->infolog, "CONTENT CAN NOT FOUND IN MEMCACHEDB key %s datalen %d - %s:%s:%d", memcachedbkey, req.mData.GetReadSize(),INFO_LOG_SUFFIX);
#else
    int suffix = dbaccess->load(dbname);
    int suffixurl = dbaccess->load(urldbname);
    string fileno;
    DBD *dbd = dbaccess->get(suffix, fileno, recordname, NULL);
    if (dbd == NULL) { //new record
#endif
        ContentHead head;
        ContentPageHead pagehead;
        head.contentPageHead = &pagehead;
        char *pageheadbuf = NULL, *headbuf = NULL;
        int pageheadlen = writePageHead(&pageheadbuf, urlnode, rheader, contentlength); 
        
        //dispose head
        head.totalpage = nextpage;
        head.nowpage = 1;

        urlnode->nowpage = head.nowpage;
        head.taskid = urlnode->taskid;
        head.batchid = urlnode->taskbatch;
        pagehead.id = urlnode->id;
        pagehead.offset = 0;
        pagehead.length = pageheadlen + contentlength;
        pagehead.page = urlnode->page;
        head.totallength = pagehead.length;

        writeHead(&headbuf, &head); //get headbuf
        
        //dispose content
        char *newcontent = (char *)malloc(sizeof(char) * pagehead.length);
        memcpy(newcontent, pageheadbuf, pageheadlen);
        memcpy(newcontent + pageheadlen, content, contentlength);

        size_t storedatlen = compressBound(head.totallength) + head.headlength;
        char *storedata = (char *)malloc(storedatlen);

        memcpy(storedata, headbuf, head.headlength);
        storedatlen -= head.headlength;
        int compressret = def(newcontent, pagehead.length, storedata + head.headlength, &storedatlen, -1);
        if (compressret != Z_OK || head.totallength == storedatlen) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "can't def content pagehead.length = %d storedatlen = %d head.totallength = %d - %s:%s:%d", pagehead.length, storedatlen, head.totallength,INFO_LOG_SUFFIX);
        }

        if (storedatlen > DATLEN_MAX) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "exceed max dat len url = %s taskid = %d id = %llu - %s:%s:%d", urlnode->url, urlnode->taskid, urlnode->id,INFO_LOG_SUFFIX);
        }

        //____BEGINTIMER
#ifdef HTMLMEMCACHEDB
            int ret = mc_store(memcachedbkey, storedata, head.headlength + storedatlen);
#else
            int ret = dbaccess->store(suffix, recordname, storedata, head.headlength + storedatlen, fileno, NULL);
#endif
        //____ENDTIMER

#ifdef HTMLMEMCACHEDB
        if (ret == MCERR_OK) {
#else
        if (ret == 0 || ret == 1) {
#endif
            ret = saveUrl(urlnode);
#ifdef HTMLMEMCACHEDB
            if (ret != MCERR_OK) {
#else
            if (ret != 0 && ret != 1) {
#endif
                mylog_error(m_pLogGlobalCtrl->errorlog, "can't store url into db %s %s - %s:%s:%d", dbname, memcachedbkey,INFO_LOG_SUFFIX);
            }
        } else {
            mylog_error(m_pLogGlobalCtrl->errorlog, "can't store url into db %s %s - %s:%s:%d", dbname, memcachedbkey,INFO_LOG_SUFFIX);
        }
        free(pageheadbuf);
        free(headbuf);
        free(newcontent);
        free(storedata);
    } else { //find 
        ContentHead *head = new ContentHead;
#ifdef HTMLMEMCACHEDB
        mylog_info(m_pLogGlobalCtrl->infolog, "CONTENT FOUND IN MEMCACHEDB key %s datalen %d - %s:%s:%d", memcachedbkey, req.mData.GetReadSize(),INFO_LOG_SUFFIX);
        readHead((char *)req.mData.GetReadBuffer(), head);
#else
        readHead(dbd->datbuf, head);
#endif
        for(int i = 0; i < head->nowpage; i++) {
            if (head->contentPageHead[i].id == urlnode->id 
                && head->contentPageHead[i].page == urlnode->page) {
                freeHead(head);
#ifdef HTMLMEMCACHEDB
                //mc_res_free(req, res);
#else
                dbd_free(dbd);
#endif
                return 0; //already saved
            }
        }

        char *pageheadbuf = NULL, *headbuf = NULL;
        int pageheadlen = writePageHead(&pageheadbuf, urlnode, rheader, contentlength); 

        char *tmp = NULL;
        size_t tmplen = 0;
#ifdef HTMLMEMCACHEDB
        inf((char *)req.mData.GetReadBuffer()+ head->headlength, req.mData.GetReadSize() - head->headlength,  &tmp, &tmplen);
        mylog_info(m_pLogGlobalCtrl->infolog, "CONTENT FOUND IN MEMCACHEDB key %s datalen %d tmplen = %d head->totallength = %d head->headlength %d - %s:%s:%d",  memcachedbkey, req.mData.GetReadSize(), tmplen,head->totallength, head->headlength,INFO_LOG_SUFFIX);
#else
        inf(dbd->datbuf + head->headlength, dbd->datlen - head->headlength,  &tmp, &tmplen);
#endif
        char *newcontent = (char *)malloc(sizeof(char) * (tmplen + contentlength + pageheadlen));
        memcpy(newcontent, tmp, tmplen);
        memcpy(newcontent + tmplen, pageheadbuf, pageheadlen);
        memcpy(newcontent + tmplen + pageheadlen, content, contentlength);

        free(tmp);

        //
        //if (urlnode->nextpage > 1)
        { //have next page
            head->totalpage += 1;
        }
        head->totallength += contentlength + pageheadlen;

        int idx = appendPageHead(head);
        head->contentPageHead[idx].id = urlnode->id; 
        head->contentPageHead[idx].offset = tmplen; 
        head->contentPageHead[idx].page = urlnode->page; 
        head->contentPageHead[idx].length = contentlength + pageheadlen; 

        writeHead(&headbuf, head);
        urlnode->nowpage = head->nowpage;

        size_t storedatlen = compressBound(head->totallength) + head->headlength;
        char *storedata = (char *)malloc(storedatlen);

        memcpy(storedata, headbuf, head->headlength);
        storedatlen -= head->headlength;
        int compressret = def(newcontent, head->totallength, storedata + head->headlength, &storedatlen, -1);

        if (compressret != Z_OK || head->totallength == storedatlen) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "can not def content storedatlen = %d head.totallength = %d in multipage %d %d - %s:%s:%d", storedatlen, head->totallength, head->totalpage, head->nowpage , INFO_LOG_SUFFIX);
        }


        if (storedatlen > DATLEN_MAX) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "exceed max dat len url = %s taskid = %d id = %llu - %s:%s:%d", urlnode->url, urlnode->taskid, urlnode->id , INFO_LOG_SUFFIX);
        }

        //____BEGINTIMER
#ifdef HTMLMEMCACHEDB
        int ret = mc_store(memcachedbkey, storedata, storedatlen + head->headlength);
#else
        int ret = dbaccess->store(suffix, recordname, storedata, storedatlen + head->headlength, fileno, NULL);
#endif
        //____ENDTIMER

#ifdef HTMLMEMCACHEDB
        if (ret == MCERR_OK) {
#else
        if (ret == 0 || ret == 1) {
#endif
            ret = saveUrl(urlnode);
#ifdef HTMLMEMCACHEDB
            if (ret != MCERR_OK) {
#else
            if (ret != 0 && ret != 1) {
#endif
                mylog_error(m_pLogGlobalCtrl->errorlog, "can not store url into db %s %s - %s:%s:%d", dbname, recordname,INFO_LOG_SUFFIX);
            }
        } else {
            mylog_error(m_pLogGlobalCtrl->errorlog, "can't store content into db %s %s - %s:%s:%d", dbname, recordname,INFO_LOG_SUFFIX);
        }
        free(newcontent);
        free(storedata);
        free(pageheadbuf);
        free(headbuf);
        freeHead(head);
#ifdef HTMLMEMCACHEDB
        //mc_res_free(req, res);
#else
        dbd_free(dbd);
#endif
    }
    return 0;
}
/*void LocalDbManager::saveUrl(UrlNode *urlnode)
{
    char recordname[16] ;recordname[0] = 0;
    getRecordKeyName(urlnode, recordname);
    char urldbname[16] ;urldbname[0] = 0;
    DBAccess *dbaccess = DBAccess::getInstance();
    getUrlDBName(urlnode, urldbname);
    int suffixurl = dbaccess->load(urldbname);

    string newfileno;
    char urlcontent[32] ;urlcontent[0] = 0;
    size_t urlcontentlen = getUrlContent(urlcontent, urlnode);
    int ret = dbaccess->store(suffixurl, urlnode->url, urlcontent, urlcontentlen, newfileno, NULL);
    if (ret != 0 && ret != 1) {
        errorlog("saveUrl can not store url into db %s %s\n", urldbname, recordname);
    }
}*/
#ifdef HTMLMEMCACHEDB
int LocalDbManager::mc_store(char *key, const char *data, size_t datalen)
{
    int keylen = strlen(key);
    if (keylen == 0) 
        return 3;
    int ret = -1; 
    MemCacheClient *mc = InfoCrawler::getInstance()->getHtmlMcLocalThread();
    MemCacheClient::MemRequest req;
    req.mKey  = key;
    req.mData.WriteBytes(data, datalen);
    
    ____BEGINTIMER
    mc->Set(req);
    ____ENDTIMER 

    int re_connect = 0; 
    while(req.mResult == MCERR_NOSERVER)
    {
        //memcache *mc = InfoCrawler::getInstance()->setMcLocalThread();
        //mc_server_activate_all(mc);
        if (!running())
        {
            return -1;
        } else {
            mylog_error(m_pLogGlobalCtrl->errorlog, "CAN NTO CONNECT TO MEMCACHEDB key %s int mc_store  Set - %s:%s:%d", key,INFO_LOG_SUFFIX);
            req.mData.SetReadBufLenEmpty();
            mc->Set(req);
        }
        my_sleep(500000); //0.5 seconds
        mylog_error(m_pLogGlobalCtrl->errorlog, "mc_store can not store content %s into memcachedb - %s:%s:%d", key,INFO_LOG_SUFFIX);
        if (re_connect++ >= MEMCACHEDB_RECONNECT_NUM )
        {
            mylog_error(m_pLogGlobalCtrl->errorlog, "CAN NTO RECONNECT TO MEMCACHEDB key %s int mc_store  Set re_connect num %d - %s:%s:%d", key, re_connect,INFO_LOG_SUFFIX);
            req.mResult = MCERR_NOTFOUND;
            break;
        }
    }
    mylog_info(m_pLogGlobalCtrl->infolog, "MEMCACHEDB Save content key %s  datalen %d - %s:%s:%d", key, datalen,INFO_LOG_SUFFIX);
    return req.mResult;
    
}
#endif
int LocalDbManager::saveUrl(UrlNode *urlnode,int savefatherurlflag)
{
#ifdef URLMEMCACHEDB
    int len = strlen (urlnode->url);
    char urlcontent[128] ;urlcontent[0] = 0;
    size_t urlcontentlen = getUrlContent(urlcontent, urlnode);
    int ret = -1;
    MemCacheClient *mc = InfoCrawler::getInstance()->getUrlMcLocalThread();
    MemCacheClient::MemRequest req;
    if (urlnode->fatherurl && savefatherurlflag == SAVE_FATHER_URL)
    {
        req.mKey  = urlnode->fatherurl;
    }else
    {
        req.mKey  = urlnode->url;
    }
    req.mData.WriteBytes(urlcontent, urlcontentlen);
    //____BEGINTIMER
    mc->Set(req);
    //____ENDTIMER
    int re_connect = 1;
    while(req.mResult == MCERR_NOSERVER)
    {
        //memcache *mc = InfoCrawler::getInstance()->setMcLocalThread();
      //  mc_server_activate_all(mc);
        if (!running())
        {
            return -1;
        }else
        {
            mylog_error(m_pLogGlobalCtrl->errorlog, "CAN NTO CONNECT TO MEMCACHEDB key %s int saveUrl  Set - %s:%s:%d", urlnode->url,INFO_LOG_SUFFIX);
            req.mData.SetReadBufLenEmpty(); 
            //____BEGINTIMER
            mc->Set(req);
            //____ENDTIMER
        }
        my_sleep(500000); //0.5 seconds
        mylog_error(m_pLogGlobalCtrl->errorlog, "saveUrl can not store url %s into memcachedb  - %s:%s:%d", urlnode->url,INFO_LOG_SUFFIX);
        if (re_connect++ >= MEMCACHEDB_RECONNECT_NUM )
        {
            mylog_error(m_pLogGlobalCtrl->errorlog, "CAN NTO RECONNECT TO MEMCACHEDB key %s int saveUrl  Set, re_connect %d - %s:%s:%d", urlnode->url, re_connect,INFO_LOG_SUFFIX);
            req.mResult = MCERR_NOTFOUND;
            break;
        }
    }
    ret = req.mResult;
#else
    char recordname[64] ;recordname[0] = 0;
    getRecordKeyName(urlnode, recordname);
    char urldbname[64] ;urldbname[0] = 0;
    DBAccess *dbaccess = DBAccess::getInstance();
    getUrlDBName(urlnode, urldbname);
    int suffixurl = dbaccess->load(urldbname);

    string newfileno = "";
    char urlcontent[128] ;urlcontent[0] = 0;
    size_t urlcontentlen = getUrlContent(urlcontent, urlnode);
    char * Key = NULL;
    if (urlnode->fatherurl  && savefatherurlflag == SAVE_FATHER_URL)
    {
        Key  = urlnode->fatherurl;
    }else
    {
        Key  = urlnode->url;
    }
    int ret = dbaccess->store(suffixurl, Key, urlcontent, urlcontentlen, newfileno, NULL);
    if (ret != 0 && ret != 1) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "saveUrl can not store url into db %s %s - %s:%s:%d", urldbname, recordname,INFO_LOG_SUFFIX);
    }
#endif
    return ret;
}
void LocalDbManager::getCookieDBName( char *dbname) {
    sprintf(dbname, "task_cookie");
}
void LocalDbManager::getUrlDBName(UrlNode *urlnode, char *dbname) {
    sprintf(dbname, "url_task_%d", getTaskDB(urlnode->taskid));
}

void LocalDbManager::getContentDBName(UrlNode *urlnode, char *dbname) {
    getContentDBName(urlnode->taskid, dbname);
}

void LocalDbManager::getContentDBName(int taskid, char *dbname) {
    sprintf(dbname, "task_%d", getTaskDB(taskid));
}

void LocalDbManager::getCookieKeyName(int taskid, char *cookiename) {
    sprintf(cookiename, "%d", getTaskDB(taskid));
}
void LocalDbManager::getMemcachedbKeyName(ulonglong id, int taskid,char *  memcachedbkey)
{
    sprintf(memcachedbkey,"%llu/%d",id , taskid);
}
void LocalDbManager::getRecordKeyName(UrlNode *urlnode, char *recordname) {
    //sprintf(recordname, "%llu", urlnode->id);
    getRecordKeyName(urlnode->id, recordname);
}

void LocalDbManager::getRecordKeyName(ulonglong id, char *recordname) {
    sprintf(recordname, "%llu", id);
}

int LocalDbManager::getTaskDB(int taskid) {
    return taskid % TASKDB_NUM;    
}

int LocalDbManager::writeHead(char **buf, BasicHead *head) {
    if (strstr(typeid(*head).name(), "ContentHead")) {
        Buffer *buffer = create_buffer(100);
        char newbuf[64] ;newbuf[0] = 0;
        int ret = 0;
        ContentHead *contenthead = (ContentHead *)head;
        ret = sprintf(newbuf, "%d/%d/%d/%d/%d/", contenthead->totallength, contenthead->totalpage, contenthead->nowpage, contenthead->taskid, contenthead->batchid);
        add_buffer(buffer, newbuf, ret);

        for(int i = 0; i < contenthead->nowpage; i++) {
            ret = sprintf(newbuf, "%llu/%d/%d/%d/", contenthead->contentPageHead[i].id, 
                contenthead->contentPageHead[i].offset, contenthead->contentPageHead[i].length, 
                contenthead->contentPageHead[i].page);
                add_buffer(buffer, newbuf, ret);
        }
        int headlength  = buffer->length;
        *buf = (char *)malloc(sizeof(char) * headlength + 32);
        contenthead->headlength = sprintf(*buf, "%8d/%s", headlength + 8 + 1, buffer->data); //8 for %8d, 1 for /
        free_buffer(buffer);
        return headlength;
    }
}

void LocalDbManager::readHead(char *buf, BasicHead *head) {
    if (strstr(typeid(*head).name(), "ContentHead")) {
        ContentHead *contenthead = (ContentHead *)head;
        int ret = 0;
        char buftmp[64] ;buftmp[0] = 0;
        sscanf(buf, "%8d/%d/%d/%d/%d/%d/", &contenthead->headlength, &contenthead->totallength
            , &contenthead->totalpage, &contenthead->nowpage, &contenthead->taskid, &contenthead->batchid);

        //in order to get bytes which sscanf already readed
        ret = sprintf(buftmp, "%8d/%d/%d/%d/%d/%d/", contenthead->headlength, contenthead->totallength
            , contenthead->totalpage, contenthead->nowpage, contenthead->taskid, contenthead->batchid);
        buf += ret;
        
        ContentPageHead *pagehead = new ContentPageHead[contenthead->nowpage];
        contenthead->contentPageHead = pagehead;
        for(int i = 0; i < contenthead->nowpage; i++, buf += ret) {
            sscanf(buf, "%llu/%d/%d/%d/", &contenthead->contentPageHead[i].id, 
                &contenthead->contentPageHead[i].offset, &contenthead->contentPageHead[i].length,
                &contenthead->contentPageHead[i].page);

            ret = sprintf(buftmp, "%llu/%d/%d/%d/", contenthead->contentPageHead[i].id, 
                contenthead->contentPageHead[i].offset, contenthead->contentPageHead[i].length,
                contenthead->contentPageHead[i].page);
        }
    }
}

size_t LocalDbManager::getUrlContent(char *buf, UrlNode *urlnode) {
    return sprintf(buf, "%llu/%d/%d/%d/%d/%u", urlnode->id, urlnode->page, urlnode->errornum, urlnode->type, urlnode->taskid,time(NULL));
}

int LocalDbManager::appendPageHead(BasicHead *head) {
    if (strstr(typeid(*head).name(), "ContentHead")) {
        ContentHead *contenthead = (ContentHead *)head;
        if (contenthead->contentPageHead) {
            ContentPageHead *pagehead = new ContentPageHead[contenthead->nowpage + 1];
            memcpy(pagehead, contenthead->contentPageHead, sizeof(ContentPageHead) * contenthead->nowpage);
            delete[] contenthead->contentPageHead;
            contenthead->contentPageHead = pagehead;
            contenthead->nowpage += 1;
            return contenthead->nowpage - 1;
        } else {
            ContentPageHead *pagehead = new ContentPageHead[1];
            contenthead->contentPageHead = pagehead;
            return 0;
        }
    }
}

void LocalDbManager::freeHead(BasicHead *head) {
    if (strstr(typeid(*head).name(), "ContentHead")) {
        ContentHead *contenthead = (ContentHead *)head;
        delete[] contenthead->contentPageHead;
        delete head;
    }
}

int LocalDbManager::writePageHead(char **buf, UrlNode *urlnode, RESPONSE_HEADER *rheader, int contentlength) {
    char tmp[MAX_URL_LEN + 30] ;tmp[0] = 0;
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    string str;
    str.reserve(100);
    str.append("version: 1.0\n");
    sprintf(tmp, "contentlength: %d\n", contentlength);
    str.append(tmp);
    sprintf(tmp, "url: %s\n", urlnode->url);
    str.append(tmp);
    sprintf(tmp, "page: %d\n", urlnode->page);
    str.append(tmp);
    sprintf(tmp, "charset: %s\n", rheader->charset.c_str());
    str.append(tmp);
    sprintf(tmp, "contenttype: %s\n", rheader->contenttype.c_str());
    str.append(tmp);
    sprintf(tmp, "id: %llu\n", urlnode->id);
    str.append(tmp);
    sprintf(tmp, "date: %u\n", time(NULL));
    str.append(tmp);
    sprintf(tmp, "fatherurl: %s\n",urlnode->fatherurl);
    str.append(tmp);
    sprintf(tmp, "title: %s\n",urlnode->title);
    str.append(tmp);
    sprintf(tmp, "spiderid: %d\n", icconfig->spiderid);
    str.append(tmp);
    sprintf(tmp, "topicsource: %s\n",urlnode->topicsource);
    str.append(tmp);
    sprintf(tmp, "\n\n");
    str.append(tmp);
    int ret = str.length();
    *buf = strdup(str.c_str());
    return ret;
}

/*bool LocalDbManager::alreadyfetched(UrlNode *urlnode) {
    DBAccess *access = DBAccess::getInstance();
    char urldbname[16] ;urldbname[0] = 0;
    getUrlDBName(urlnode, urldbname);
    int suffixurl = access->load(urldbname);
    string fileno;
    DBD *dbd = access->get(suffixurl, fileno, urlnode->url, NULL);
    if (dbd) {
        dbd_free(dbd);
        return true;
    } else {
        return false;
    }
}*/

bool LocalDbManager::alreadyfetched(UrlNode *urlnode, char * urlcontent) {
#ifdef URLMEMCACHEDB
    int len = strlen (urlnode->url);
    
    MemCacheClient::MemRequest req;
    MemCacheClient *mc = InfoCrawler::getInstance()->getUrlMcLocalThread();
    
    req.mKey  = urlnode->url;
    //____BEGINTIMER
    mc->Get(req);
    //____ENDTIMER

    int i=1;
    int re_connect = 0;
    while(req.mResult == MCERR_NOSERVER)
    {
        if (!(running()))
        {
            return false;
        }else
        {
            mylog_error(m_pLogGlobalCtrl->errorlog, "CAN NTO CONNECT TO MEMCACHEDB key %s int alreadyfetched GET - %s:%s:%d", urlnode->url,INFO_LOG_SUFFIX);
            req.mData.Deallocate();
            //____BEGINTIMER
            mc->Get(req);
            //____ENDTIMER
        }
        my_sleep(500000); //0.5 seconds
        if (re_connect++ >= MEMCACHEDB_RECONNECT_NUM )
        {
            mylog_error(m_pLogGlobalCtrl->errorlog, "CAN NTO RECONNECT TO MEMCACHEDB key %s int alreadyfetched GET re_connect num %d - %s:%s:%d", urlnode->url, re_connect ,INFO_LOG_SUFFIX);
            req.mResult = MCERR_NOTFOUND;
            break;
        }
    }
    if (req.mResult == MCERR_OK)
    {
        if (urlcontent)
        {
            strncpy(urlcontent, (char *)req.mData.GetReadBuffer(), req.mData.GetReadSize());
            urlcontent[req.mData.GetReadSize()] = 0;
        }
        return true;
    }else
    {
        return false;
    }
#else
    DBAccess *access = DBAccess::getInstance();
    char urldbname[64] ;urldbname[0] = 0;
    getUrlDBName(urlnode, urldbname);
    int suffixurl = access->load(urldbname);
    string fileno;
    DBD *dbd = access->get(suffixurl, fileno, urlnode->url, NULL);
    if (dbd) {
        if (urlcontent)
        {
            strncpy(urlcontent,dbd->datbuf,dbd->datlen);
            urlcontent[dbd->datlen] = 0;
        }
        dbd_free(dbd);
        return true;
    } else {
        return false;
    }
#endif
}
/*bool LocalDbManager::alreadyfetched(UrlNode *urlnode, char * urlcontent) {
    DBAccess *access = DBAccess::getInstance();
    char urldbname[16] ;urldbname[0] = 0;
    getUrlDBName(urlnode, urldbname);
    int suffixurl = access->load(urldbname);
    string fileno;
    DBD *dbd = access->get(suffixurl, fileno, urlnode->url, NULL);
    if (dbd) {
        strncpy(urlcontent,dbd->datbuf,dbd->datlen);
        urlcontent[dbd->datlen] = 0;
        dbd_free(dbd);
        return true;
    } else {
        return false;
    }
}*/

void LocalDbManager::saveFetched(UrlNode *urlnode) {
    
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    vector<string> dirs;
    dirs.push_back(icconfig->dbpath);

    pthread_rwlock_wrlock(&rwlock_fifoqueu_file);
    char filename[128];filename[0] = 0;
    sprintf(filename, "%s/%s", icconfig->dbpath, FETCHED_FIFO_QUEUE_FILE);
    FSBigFile *bigfile  = new FSBigFile(filename, &dirs, 'a');

    char key[32] ;key[0] = 0;
    char data[32] ;data[0] = 0;
    int keylen = sprintf(key, "%llu/%d", urlnode->id, urlnode->taskid);

    mylog_info(m_pLogGlobalCtrl->infolog, "current pos =  %s %s %llu  - %s:%s:%d", urlnode->url, key, bigfile->tell(),INFO_LOG_SUFFIX);

    int ret = WriteContent(bigfile, key, keylen, "", 0);
    if (ret != 0) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not write urlnode for FSBigFile %s - %s:%s:%d", FETCHED_FIFO_QUEUE_FILE,INFO_LOG_SUFFIX);
    }

    pthread_rwlock_unlock(&rwlock_fifoqueu_file);
   
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    infocrawler->getTaskScheduleManager()->increaseTaskSavefetchedNum(urlnode->taskid);
   
    delete bigfile;
}
void LocalDbManager::decidesaveFetched(UrlNode *urlnode)
{
    if (urlnode->page > 1)
    {
        InfoCrawler *infocrawler = InfoCrawler::getInstance();
        if (infocrawler->getPageManager()->FindInDb(urlnode))
        {
            altertotalpageforced(urlnode);
        }else
        {
            infocrawler->getPageManager()->altermaptotalpageforced(urlnode);
        }
        saveFetched(urlnode);
        mylog_error(m_pLogGlobalCtrl->errorlog, "fatherurl %s url %s page %d so saveFetched - %s:%s:%d", urlnode->fatherurl, urlnode->url,urlnode->page,INFO_LOG_SUFFIX);
    }
}
int LocalDbManager::erasecontent(ulonglong id ,int taskid) {
    int ret = 0;
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    if (infocrawler->getPageManager()->FindInDb(taskid, id))
    {
        erasecontent_db(taskid, id);
    }else if (infocrawler->getPageManager()->FindInMap(id))
    {
        infocrawler->getPageManager()->erasecontent_map(id);
    }
    return ret ;
    
}
void LocalDbManager::saveAlreadyRead(ulonglong id, int taskid ) {
    
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    vector<string> dirs;
    dirs.push_back(icconfig->dbpath);

    pthread_rwlock_wrlock(&rwlock_read_fifoqueu_file);
    char filename[128];filename[0] = 0;
    sprintf(filename, "%s/%s", icconfig->dbpath, READ_FIFO_QUEUE_FILE);
    FSBigFile *bigfile  = new FSBigFile(filename, &dirs, 'a');
    
    char strTime[128] ;strTime[0] = 0; 
    time_t tDate;
    time(&tDate);
    strftime(strTime, 127,"%Y-%m-%d %H:%M:%S", localtime(&tDate));
    
    char key[64] ;key[0] = 0;
    char a[] = {9,0};
    int keylen = sprintf(key, "%llu/%d%s%s\n", id, taskid,a,strTime);


    int ret = bigfile->write(key, keylen); 
    //Debug(0, 1, ("Debug: READ_FIFO_QUEUE_FILE current pos = %s %llu in saveFetched LocalDbManager\n", key, bigfile->tell()));

    if (ret != 0) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can't write urlnode for FSBigFile - %s:%s:%d", READ_FIFO_QUEUE_FILE,INFO_LOG_SUFFIX);
    }

end:
    pthread_rwlock_unlock(&rwlock_read_fifoqueu_file);
    delete bigfile;
}
bool LocalDbManager::ReadDeleteAlreadyRead()
{
    int deleteflag = 0;
    char filename[128];filename[0] = 0;
    int ret = 0;
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    vector<string> dirs;
    dirs.push_back(icconfig->dbpath);
    
    pthread_rwlock_wrlock(&rwlock_read_fifoqueu_file);
    sprintf(filename, "%s/%s", icconfig->dbpath, DELETE_READ_FIFO_QUEUE_FLAG_FILE);
    FILE *f  = fopen(filename, "r");
    if (f) {
        char key[32] = ""; 
        ret = fread(key, 1, 32, f);
        if (ret == 0) {
            fclose(f);
            mylog_error(m_pLogGlobalCtrl->errorlog, "can not read flag in readfetch for %s - %s:%s:%d", DELETE_READ_FIFO_QUEUE_FLAG_FILE,INFO_LOG_SUFFIX);
            goto end;
        } else {
            sscanf(key, "%d", &deleteflag);
            fclose(f);
            f  = fopen(filename, "w");
            if (f)
            {
                char key[4] = {'0',0};
                ret = fwrite(key, 1, 1, f);
                if (ret == 0) {
                    fclose(f);
                    mylog_error(m_pLogGlobalCtrl->errorlog, "can't write flag in  %s - %s:%s:%d", DELETE_READ_FIFO_QUEUE_FLAG_FILE,INFO_LOG_SUFFIX);
                    goto end;
                }else
                {
                    fclose(f);
                    goto end;
                }
                fclose(f);
            }
        }
    } else {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not open flag file in read flag for - %s:%s:%d", DELETE_READ_FIFO_QUEUE_FLAG_FILE,INFO_LOG_SUFFIX);
        goto end;
    }
end:
    pthread_rwlock_unlock(&rwlock_read_fifoqueu_file);
    if (deleteflag == 1)
    {
        return true;
    }else
    {
        return false;
    }
}
//don't forget to free key after using it
void LocalDbManager::GetAlreadyRead(list<char *> &alreadyread) {
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    vector<string> dirs;
    dirs.push_back(icconfig->dbpath);

    int lastdeletekeylen = 0;
    int64_t pos = 0;
    int ret = 0; 
    char filename[128];filename[0] = 0;
    FSBigFile *bigfile = NULL;
    
    char lastdeletekey [64] ;lastdeletekey[0] = 0;
    pthread_rwlock_wrlock(&rwlock_read_fifoqueu_file);
    sprintf(filename, "%s/%s", icconfig->dbpath, DELETE_READ_FIFO_QUEUE_POS_FILE);
   
    FILE *f  = fopen(filename, "r");
    if (f) {
        char key[128] = ""; 
        ret = fread(key, 1, 128, f);
        if (ret == 0) {
            fclose(f);
            mylog_error(m_pLogGlobalCtrl->errorlog, "can not read position in readfetch for FSBigFile %s - %s:%s:%d", DELETE_READ_FIFO_QUEUE_POS_FILE,INFO_LOG_SUFFIX);
            goto end;
        } else {
            sscanf(key, "%llu/%s", &pos, lastdeletekey);
            fclose(f);
        }
    } else {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not open position file in readfetch for FSBigFile %s - %s:%s:%d", DELETE_READ_FIFO_QUEUE_POS_FILE,INFO_LOG_SUFFIX);
        goto end;
    }

    sprintf(filename, "%s/%s", icconfig->dbpath, READ_FIFO_QUEUE_FILE);

    if (access(filename, R_OK) == -1) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not open fetched file in readFetched for LocalDbManager %s - %s:%s:%d", READ_FIFO_QUEUE_FILE,INFO_LOG_SUFFIX);
        goto end;
    }
   
    bigfile  = new FSBigFile(filename, &dirs, 'r');

    char str[1024];
    
    if (bigfile->seek(pos, SEEK_SET) != 0) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not set position for FSBigFile %s %llu - %s:%s:%d", READ_FIFO_QUEUE_FILE, pos,INFO_LOG_SUFFIX);
        delete bigfile;
        goto end;
    }
    if ((ret = getlines(bigfile, str)) > -1)
    {
        char * poskey = NULL;
        if (((poskey = strstr(str,lastdeletekey)) == NULL) || (lastdeletekey[0] == '\0'))
        {
            mylog_error(m_pLogGlobalCtrl->errorlog, "can not find already read key %s pos %llu in %s - %s:%s:%d", lastdeletekey, pos, READ_FIFO_QUEUE_FILE ,INFO_LOG_SUFFIX);
            if (bigfile->seek(0, SEEK_SET) != 0) {
                mylog_error(m_pLogGlobalCtrl->errorlog, "can not set position for FSBigFile %s 0 - %s:%s:%d", READ_FIFO_QUEUE_FILE,INFO_LOG_SUFFIX);
                delete bigfile;
                goto end;
            }
        }
        pos = bigfile->tell(); 
        while((ret = getlines(bigfile, str)) > -1)
        {
            char *key = (char *)malloc(sizeof(char) * 64);
            key[0] = 0; 
            sscanf(str,"%s\t%*s",key);
            alreadyread.push_back(key);
            lastdeletekeylen= sprintf(lastdeletekey, "%llu/%s", pos, key);
            pos = bigfile->tell(); 
        }
    }else
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not get pos %llu  %s in %s - %s:%s:%d", pos, lastdeletekey,INFO_LOG_SUFFIX);
        goto end;
    }

    delete bigfile;
    sprintf(filename, "%s/%s", icconfig->dbpath, DELETE_READ_FIFO_QUEUE_POS_FILE);
    f  = fopen(filename, "w");
    if (f) {
        ret = fwrite(lastdeletekey, 1, lastdeletekeylen, f);
        if (ret == 0) {
            fclose(f);
            mylog_error(m_pLogGlobalCtrl->errorlog, "can not write position in read fetch for FSBigFile %s - %s:%s:%d", DELETE_READ_FIFO_QUEUE_POS_FILE,INFO_LOG_SUFFIX);
            goto end;
        } else {
            mylog_error(m_pLogGlobalCtrl->errorlog, "read fetched pos is %llu - %s:%s:%d", pos,INFO_LOG_SUFFIX);
        }
        fclose(f);
    } else {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not open for write position file in readfetch for FSBigFile %s - %s:%s:%d", DELETE_READ_FIFO_QUEUE_POS_FILE,INFO_LOG_SUFFIX);
        goto end;
    }
end:
    pthread_rwlock_unlock(&rwlock_read_fifoqueu_file);
}
bool LocalDbManager::deleteAllData()
{
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    string oldfile = icconfig->dbpath;
    oldfile.append("/");
    oldfile.append(IDX_TEMPLATE);
    string newfile = icconfig->logdir;
    newfile.append("/");
    newfile.append(IDX_TEMPLATE);
    if (rename(oldfile.c_str(),newfile.c_str())==0)
    {
        myrmdir(icconfig->dbpath);            
        {
            if (mkdir(icconfig->dbpath,S_IRWXO|S_IRWXG|S_IRWXU) == 0)
            {
                if (rename(newfile.c_str(),oldfile.c_str()) != 0)
                {
                    mylog_error(m_pLogGlobalCtrl->errorlog, "Can't mv from %s to %s - %s:%s:%d", oldfile.c_str(),newfile.c_str(),INFO_LOG_SUFFIX);
                }else
                {
                    return true;
                }
            }
        }
    }else
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Can't mv from %s to %s - %s:%s:%d", oldfile.c_str(),newfile.c_str(),INFO_LOG_SUFFIX);
    }
    return false;
}
bool LocalDbManager::IsReadFetchedEnd()
{
    int64_t posfilepos = 0;
    int64_t queuqfilepos = 0;
    int64_t pos = 0;
    int ret = 0; 
    char filename[128];filename[0] = 0;
    FSBigFile *bigfile = NULL;
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    vector<string> dirs;
    dirs.push_back(icconfig->dbpath);

    pthread_rwlock_wrlock(&rwlock_read_fifoqueu_file);
    sprintf(filename, "%s/%s", icconfig->dbpath, FETCHED_FIFO_QUEUE_POS_FILE);
    FILE *f  = fopen(filename, "r");
    if (f) {
        char key[32] = "";
        ret = fread(key, 1, 32, f);
        if (ret == 0) {
            fclose(f);
            mylog_error(m_pLogGlobalCtrl->errorlog, "can not read position in readfetch for FSBigFile %s %llu - %s:%s:%d", FETCHED_FIFO_QUEUE_FILE, posfilepos,INFO_LOG_SUFFIX);
        } else {
            sscanf(key, "%llu", &posfilepos);
            fclose(f); 
        }   
    } else {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not open position file in IsReadFetchedEnd for FSBigFile %s - %s:%s:%d", FETCHED_FIFO_QUEUE_FILE, INFO_LOG_SUFFIX);
        goto end;
    }

    sprintf(filename, "%s/%s", icconfig->dbpath, FETCHED_FIFO_QUEUE_FILE);

    if (access(filename, R_OK) == -1) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not open fetched file in readFetched for LocalDbManager %s - %s:%s:%d", FETCHED_FIFO_QUEUE_FILE, INFO_LOG_SUFFIX);
        goto end;
    }
    bigfile  = new FSBigFile(filename, &dirs, 'r');

    if (bigfile->seek(pos, SEEK_END) != 0) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not set position for FSBigFile %s %llu - %s:%s:%d", FETCHED_FIFO_QUEUE_FILE, pos,INFO_LOG_SUFFIX);
        delete bigfile;
        goto end;
    }
    queuqfilepos = bigfile->tell();
    delete bigfile;
end:
    pthread_rwlock_unlock(&rwlock_read_fifoqueu_file);
    if (posfilepos >=queuqfilepos )
    {
        return true;
    }
    return false;
}
//don't forget to free key after using it
bool LocalDbManager::readFetched(list<char *> &fetched, int num) {
    
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    vector<string> dirs;
    dirs.push_back(icconfig->dbpath);

    char *key = NULL;
    char *data= NULL;
    int keylen = 0;
    int datalen = 0;
    int i = 0;
    int64_t pos = 0;
    char poskey[32] ;poskey[0] = 0;
    char filename[128];filename[0] = 0;
    FSBigFile *bigfile = NULL;
    int ret = 0;
    static int64_t lastpos = 0;

    pthread_rwlock_wrlock(&rwlock_fifoqueu_file);

    sprintf(filename, "%s/%s", icconfig->dbpath, FETCHED_FIFO_QUEUE_POS_FILE);
    FILE *f  = fopen(filename, "r");
    if (f) {
        char key[32] = ""; 
        ret = fread(key, 1, 32, f);
        if (ret == 0) {
            fclose(f);
            mylog_error(m_pLogGlobalCtrl->errorlog, "can not read position in readfetch for FSBigFile %s %llu %llu - %s:%s:%d",FETCHED_FIFO_QUEUE_FILE, pos, lastpos,INFO_LOG_SUFFIX);

            if (lastpos > 0  && lastpos > pos) {
                pos = lastpos;
            }

           // goto end;
        } else {
            sscanf(key, "%llu", &pos);
            fclose(f);
        }
    } else {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not open position file in readfetch for FSBigFile %s - %s:%s:%d",FETCHED_FIFO_QUEUE_FILE,INFO_LOG_SUFFIX);
        goto end;
    }

    sprintf(filename, "%s/%s", icconfig->dbpath, FETCHED_FIFO_QUEUE_FILE);

    if (access(filename, R_OK) == -1) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not open fetched file in readFetched for LocalDbManager %s - %s:%s:%d",FETCHED_FIFO_QUEUE_FILE,INFO_LOG_SUFFIX);
        goto end;
    }
    bigfile  = new FSBigFile(filename, &dirs, 'r');

    if (bigfile->seek(pos, SEEK_SET) != 0) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not set position for FSBigFile %s %llu - %s:%s:%d",FETCHED_FIFO_QUEUE_FILE,pos ,INFO_LOG_SUFFIX);
        delete bigfile;
        goto end;
    }

    while(i++ < num && (ret = ReadContent(bigfile, &key, &keylen, &data, &datalen)) > -1) {
        fetched.push_back(key);
        free(data);
    }

    pos = bigfile->tell(); 
    delete bigfile;
    

    sprintf(filename, "%s/%s", icconfig->dbpath, FETCHED_FIFO_QUEUE_POS_FILE);
    f  = fopen(filename, "w");
    if (f) {
        char key[32] ;key[0] = 0;
        keylen = sprintf(key, "%llu", pos);
        ret = fwrite(key, 1, keylen, f);
        fflush(f);
        if (ret == 0) {
            fclose(f);
            mylog_error(m_pLogGlobalCtrl->errorlog, "can not write position in read fetch for FSBigFile %s %s  - %s:%s:%d:%d",FETCHED_FIFO_QUEUE_FILE, key,INFO_LOG_SUFFIX,ret);
            goto end;
        } else {
            mylog_error(m_pLogGlobalCtrl->errorlog, "read fetched pos is %llu %s %d  - %s:%s:%d",pos, key, ret, INFO_LOG_SUFFIX);
            lastpos = pos;
        }
        fclose(f);
    } else {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not open for write position file in readfetch for FSBigFile %s  - %s:%s:%d",FETCHED_FIFO_QUEUE_FILE, INFO_LOG_SUFFIX);
        goto end;
    }

end:
    pthread_rwlock_unlock(&rwlock_fifoqueu_file);
}

DBD *LocalDbManager::readCookie(int taskid) {
    DBAccess *access = DBAccess::getInstance();
    char cookiedbname[64] ;cookiedbname[0] = 0;
    char cookiename[16] ;cookiename[0] = 0;
    DBAccess *dbaccess = DBAccess::getInstance();
    getCookieDBName(cookiedbname);
    getCookieKeyName(taskid,cookiename);
    int suffix = dbaccess->load(cookiedbname);
    string fileno;
    DBD *dbd = dbaccess->get(suffix, fileno, cookiename , NULL);
    if (!dbd) return NULL;
    return dbd;
}

/*#ifdef MEMCACHEDB
memcache_res *LocalDbManager::readContentMemcacheDb(ulonglong id, int taskid) {
    char recordname[64] ;recordname[0] = 0;
    getMemcachedbKeyName(id, taskid, recordname);

    memcache_req *req = mc_req_new();
    int len = strlen (recordname);
    memcache_res *res = mc_req_add(req, recordname, len);
    memcache *mc = InfoCrawler::getInstance()->getMcLocalThread(GET_CONTENT_MC);
    mc_get(mc, req);

    if (res->_flags & MCM_RES_FOUND){
        return res;
    }else
    {
        return NULL;
    }
}
#endif
*/
DBD *LocalDbManager::readContent(ulonglong id, int taskid) {
    DBAccess *access = DBAccess::getInstance();
    char dbname[64] ;dbname[0] = 0;
    char keyname[32] ;keyname[0] = 0;

    getContentDBName(taskid, dbname);
    getRecordKeyName(id, keyname);

    int suffix = access->load(dbname);
    string fileno;

    DBD *dbd = access->get(suffix, fileno, keyname, NULL);
    if (!dbd) return NULL;
    ContentHead *head = new ContentHead;
    readHead(dbd->datbuf, head);

    if (head->totalpage != head->nowpage) {
        dbd_free(dbd);
        mylog_info(m_pLogGlobalCtrl->infolog, "totalpage %d is not equal to %d nowpage %llu - %s:%s:%d",  head->totalpage, head->nowpage,id,INFO_LOG_SUFFIX);
        return NULL;
    }

    freeHead(head);
    return dbd;
}
int LocalDbManager::savecookiecontent(char *content, int contentlength , int taskid) 
{
    char cookiedbname[16] ;cookiedbname[0] = 0;
    char cookiename[16] ;cookiename[0] = 0;
    DBAccess *dbaccess = DBAccess::getInstance();
    getCookieDBName(cookiedbname);
    getCookieKeyName(taskid,cookiename);
    int suffix = dbaccess->load(cookiedbname);
    string fileno;
    DBD *dbd = dbaccess->get(suffix, fileno, cookiename , NULL);
    //____BEGINTIMER
        int ret = dbaccess->store(suffix, cookiename, content, contentlength, fileno, NULL);
    //____ENDTIMER
    if (ret != 0 && ret != 1) 
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not store cookie into db %s %s  - %s:%s:%d",cookiedbname, cookiename, INFO_LOG_SUFFIX);
    }

    return 0;
}

void LocalDbManager::dump() {}
void LocalDbManager::retrieve() {}



#include <zlib.h>
#include "ic_types.h"
#include "InfoCrawler.h"
#include "PageManager.h"
#include "Http.h"

pthread_rwlock_t PageManager::rwlock_page_map = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t PageManager::rwlock_page_buf_size = PTHREAD_RWLOCK_INITIALIZER;



PageManager::PageManager(LogGlobalCtrl * pLogGlobalCtrl)
{
    m_pLogGlobalCtrl = pLogGlobalCtrl;
    totalpagebufsize = 0;
}
PageManager::~PageManager()
{
}
int PageManager::start() {
    Manager::start();
    return 0;
}
int PageManager::stop() {
    int ret = Manager::stop();
    dumpmap();
    totalpagebufsize = 0;
    return ret;
}
void PageManager::dump() {
}   
void PageManager::retrieve() {
} 
int PageManager::SavePage(char * content,int contentlen, UrlNode *urlnode, RESPONSE_HEADER *rheader) {
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    Buffer * buffer =  CreateBuffer(content, contentlen, urlnode, rheader);
    int pagebufsize = buffer->length * sizeof (char);
    //size_t pagebufsize = contentlen * sizeof (char);
    if (FindInDb(urlnode))
    {

        mylog_info(m_pLogGlobalCtrl->infolog, "page find in db task id %d urlnode id %llu page %d nextpage %d url %s - %s:%s:%d", urlnode->taskid , urlnode->id , urlnode->page, urlnode->nextpage , urlnode->url,INFO_LOG_SUFFIX);
        free_buffer(buffer); 
        SaveInDb( content,contentlen, urlnode, rheader);
    }else if (FindInMap(urlnode))// include two situtation  , urlnode id  save in memorry or urlnode id have never been saved
    {
        mylog_info(m_pLogGlobalCtrl->infolog, "page find in map task id %d urlnode id %llu page %d nextpage %d url %s - %s:%s:%d", urlnode->taskid , urlnode->id , urlnode->page, urlnode->nextpage , urlnode->url,INFO_LOG_SUFFIX);
        if (urlnode->nextpage > 1)
        {
            int ret = IncreaseTotalBufSize( pagebufsize, urlnode);
            if (ret >0)
            {
                //save in memory
                SaveInMap(buffer , urlnode);
            }else //have nextpage but totalpagebufsize + pagebufsize > define max buf size so save in HD
            {
                //merge page and  save in hard disk
                if (SaveInMap(buffer , urlnode) == 1)
                {       
                   MergePage(urlnode, pagebufsize);
                }else   
                {       
                    return 0;
                } 
            }
        }else //no next page save in db
        {
            //merge page and  save in hard disk
            if (SaveInMap(buffer , urlnode) == 1)
            {       
               MergePage(urlnode, pagebufsize);
            }else   
            {       
                return 0;
            } 
        }
        
    }else
    {
        mylog_info(m_pLogGlobalCtrl->infolog, "page can't find in map or db task id %d urlnode id %llu page %d nextpage %d url %s - %s:%s:%d", urlnode->taskid , urlnode->id , urlnode->page, urlnode->nextpage , urlnode->url,INFO_LOG_SUFFIX);
        if (urlnode->nextpage > 1)
        {
            int ret = IncreaseTotalBufSize( pagebufsize, urlnode);
            if (ret >0)
            {
                //save in memory
                SaveInMap(buffer , urlnode);
            }else // have nextpage but pagebufsize +totalpagebufsize > define max buf size
            {
                free_buffer(buffer); 
                SaveInDb( content,contentlen, urlnode, rheader);
            }
        }else // no nextpage so save in hard disk
        {
            free_buffer(buffer); 
            SaveInDb( content,contentlen, urlnode, rheader);
        }
    }
}
int PageManager::GetMapSize()
{
    int ret =0;
    pthread_rwlock_wrlock(&rwlock_page_map);
    ret = pagemap.size();
    pthread_rwlock_unlock(&rwlock_page_map);
    return ret;
}
bool PageManager::FindInMap(UrlNode *urlnode)
{
    bool ret = false;
    map<ulonglong ,PAGEBUF*>::iterator iter;
    pthread_rwlock_rdlock(&rwlock_page_map);
    if ((iter = pagemap.find(urlnode->id)) != pagemap.end())
    {
        ret = true;
    }
    pthread_rwlock_unlock(&rwlock_page_map);
    return ret;
}
bool PageManager::FindInMap(ulonglong urlnodeid)
{
    bool ret = false;
    map<ulonglong ,PAGEBUF*>::iterator iter;
    pthread_rwlock_rdlock(&rwlock_page_map);
    if ((iter = pagemap.find(urlnodeid)) != pagemap.end())
    {
        ret = true;
    }
    pthread_rwlock_unlock(&rwlock_page_map);
    return ret;
}
void PageManager::erasecontent_map(ulonglong urlnodeid)
{
    PAGEBUF* pagebuf = NULL;
    map<ulonglong ,PAGEBUF*>::iterator iter;
    pthread_rwlock_wrlock(&rwlock_page_map);
    if ((iter = pagemap.find(urlnodeid)) != pagemap.end())
    {
        pagebuf = iter->second;
        pagemap.erase(iter);
        pthread_rwlock_unlock(&rwlock_page_map);
        mylog_info(m_pLogGlobalCtrl->infolog, "erase content %llu   - %s:%s:%d", urlnodeid, INFO_LOG_SUFFIX);
    }else
    {
        pthread_rwlock_unlock(&rwlock_page_map);
    }
    if (pagebuf)
    {
        DecreaseTotalBufSize((pagebuf->totallength )* sizeof(char) , NULL);
        delete pagebuf;
    }

    

}
void PageManager::altermaptotalpageforced(UrlNode *urlnode)
{
    if (FindInMap(urlnode))
    {
        PAGEBUF* pagebuf = NULL;
        map<ulonglong ,PAGEBUF*>::iterator iter;
        pthread_rwlock_wrlock(&rwlock_page_map);
        if ((iter = pagemap.find(urlnode->id)) != pagemap.end())
        {
            pagebuf = iter->second;
            pagebuf->totalpage = pagebuf->nowpage;
            pthread_rwlock_unlock(&rwlock_page_map);
            mylog_info(m_pLogGlobalCtrl->infolog, "altermaptotalpageforced  %llu %s  - %s:%s:%d", pagebuf->urlnodeid , urlnode->url,INFO_LOG_SUFFIX);
            MergePage(urlnode, 0); 
        }else
        {
            pthread_rwlock_unlock(&rwlock_page_map);
        }
    
    
    }
}
bool PageManager::FindInDb(UrlNode *urlnode)
{
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    bool ret = false;
    DBAccess *dbaccess = DBAccess::getInstance();
    char recordname[64] ;recordname[0] = 0;
    char dbname[64] ;dbname[0] = 0;
    infocrawler->getLocalDbManager()->getContentDBName(urlnode, dbname);
    infocrawler->getLocalDbManager()->getRecordKeyName(urlnode, recordname);
    int suffix = dbaccess->load(dbname);
    string fileno;
    DBD *dbd = dbaccess->get(suffix, fileno, recordname, NULL);
    if (dbd != NULL)
    {
        ret = true;
    }
    dbd_free(dbd);
    return ret;
}
bool PageManager::FindInDb(int taskid, ulonglong urlnodeid)
{
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    bool ret = false;
    DBAccess *dbaccess = DBAccess::getInstance();
    char recordname[64] ;recordname[0] = 0;
    char dbname[64] ;dbname[0] = 0;
    infocrawler->getLocalDbManager()->getContentDBName(taskid, dbname);
    infocrawler->getLocalDbManager()->getRecordKeyName(urlnodeid, recordname);
    int suffix = dbaccess->load(dbname);
    string fileno;
    DBD *dbd = dbaccess->get(suffix, fileno, recordname, NULL);
    if (dbd != NULL)
    {
        ret = true;
    }
    dbd_free(dbd);
    return ret;
}
Buffer * PageManager::CreateBuffer(char * content,int contentlength, UrlNode *urlnode, RESPONSE_HEADER *rheader)
{
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    char *pageheadbuf = NULL;
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    int pageheadlen = infocrawler->getLocalDbManager()->writePageHead(&pageheadbuf, urlnode, rheader, contentlength);
    Buffer * ret  = NULL;
    Buffer *buffer = create_buffer(pageheadlen);
    add_buffer(buffer, pageheadbuf, pageheadlen);
    add_buffer(buffer, content, contentlength);
    free(pageheadbuf);
    if (icconfig->ifdef == PAGEMANAGE_DEF)
    {
        size_t storedatlen = compressBound(buffer->length); 
        char *storedata = (char *)malloc(storedatlen);
        
        int compressret = def(buffer->data, buffer->length, storedata, &storedatlen, -1);

        if (compressret != Z_OK || buffer->length == storedatlen) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "can't def content storedatlen = %d - %s:%s:%d",storedata ,INFO_LOG_SUFFIX);
        }

        if (storedatlen > DATLEN_MAX) {
            mylog_error(m_pLogGlobalCtrl->errorlog, " exceed max dat len url = %s taskid = %d id = %llu - %s:%s:%d",urlnode->url, urlnode->taskid, urlnode->id ,INFO_LOG_SUFFIX);
        }
        Buffer *buffer1 = create_buffer(DEFAULT_PAGE_BUF_SIZE);
        add_buffer(buffer1, storedata, storedatlen);
        ret = buffer1;
        free_buffer(buffer);
        free(storedata);
    }else
    {
        ret = buffer;
    }
    return ret;
}
int PageManager::SaveInMap(Buffer *buffer, UrlNode *urlnode)
{
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    int ret = -1;
    SaveInMap(buffer, urlnode, &ret);
    if (ret == 1)
    {
        int rettmp = infocrawler->getLocalDbManager()->saveUrl(urlnode);
        if (rettmp != 0 && rettmp != 1) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "save url error url %s - %s:%s:%d",urlnode->url,INFO_LOG_SUFFIX);
        }
    }else
    {
        free_buffer(buffer); 
        mylog_error(m_pLogGlobalCtrl->errorlog, " SaveInMap error taskid %d urlnode %llu urlnodepage %d - %s:%s:%d",urlnode->taskid, urlnode->id, urlnode->page ,INFO_LOG_SUFFIX);
    }
    return ret;
}
void PageManager::SaveInMap(Buffer *buffer, UrlNode *urlnode, int * ret)
{
    *ret = 1;
    map<ulonglong ,PAGEBUF*>::iterator iter;
    pthread_rwlock_wrlock(&rwlock_page_map);
    if ((iter = pagemap.find(urlnode->id)) != pagemap.end())
    {
        PAGEBUF* pagebuf= iter->second;
        if (pagebuf->pages[urlnode->page] == NULL)
        {
            pagebuf->pages[urlnode->page] = buffer;
            if (urlnode->nextpage > 1) { //have next page
                pagebuf->totalpage += 1;
            } 
            pagebuf->nowpage += 1;
            pagebuf->totallength += buffer->length;
            mylog_info(m_pLogGlobalCtrl->infolog, "%llu save in map %s totallength %d currentbuf size %d totalpage %d nowpage %d  - %s:%s:%d", urlnode->id , urlnode->url,pagebuf->totallength ,buffer->length, pagebuf->totalpage, pagebuf->nowpage , INFO_LOG_SUFFIX);
        }else
        {
            mylog_info(m_pLogGlobalCtrl->infolog, "page already exsit %d - %s:%s:%d", urlnode->page,INFO_LOG_SUFFIX);
            *ret = 0;
        }
    }else
    {
        PAGEBUF* pagebuf = new  PAGEBUF;
        pagebuf->pages[urlnode->page] = buffer; 
        pagebuf->totalpage = urlnode->nextpage;
        pagebuf->nowpage = 1;
        pagebuf->taskid = urlnode->taskid;
        pagebuf->urlnodeid = urlnode->id;
        pagebuf->batchid = urlnode->taskbatch;
        pagebuf->totallength += buffer->length;
        pagemap[urlnode->id] = pagebuf;
        mylog_info(m_pLogGlobalCtrl->infolog, "%llu save in map %s totallength %d currentbuf size %d totalpage %d nowpage %d  - %s:%s:%d", urlnode->id , urlnode->url,pagebuf->totallength ,buffer->length, pagebuf->totalpage, pagebuf->nowpage , INFO_LOG_SUFFIX);
    }
    pthread_rwlock_unlock(&rwlock_page_map);
}
void PageManager::MergePage( UrlNode *urlnode, int lastcontentlen) 
{

    PAGEBUF* pagebuf = NULL;
    map<ulonglong ,PAGEBUF*>::iterator iter;
    pthread_rwlock_wrlock(&rwlock_page_map);
    if((iter = pagemap.find(urlnode->id)) != pagemap.end())
    {
        pagebuf = iter->second;
        pagemap.erase(iter);
        pthread_rwlock_unlock(&rwlock_page_map);
    }else
    {
        pthread_rwlock_unlock(&rwlock_page_map);
        mylog_info(m_pLogGlobalCtrl->infolog, "can't find urlnodeid %llu - %s:%s:%d", urlnode->id,INFO_LOG_SUFFIX);
    }
    if (pagebuf != NULL)
    {
        mylog_info(m_pLogGlobalCtrl->infolog, "%llu pagemap size %d lastcontentlen %d pagebuf total size %d dicrease %d - %s:%s:%d", urlnode->id , GetMapSize(), lastcontentlen, pagebuf->totallength , pagebuf->totallength - lastcontentlen,INFO_LOG_SUFFIX);
        DecreaseTotalBufSize((pagebuf->totallength - lastcontentlen)* sizeof(char) , urlnode);
        SaveInDb(pagebuf, urlnode);
        delete pagebuf;
    }
}
void PageManager::dumpmap() {
    PAGEBUF* pagebuf = NULL;
    map<ulonglong ,PAGEBUF*>::iterator iter;
    pthread_rwlock_wrlock(&rwlock_page_map);
    for (iter = pagemap.begin(); iter != pagemap.end(); iter++)
    {
        pagebuf = iter->second;
        SaveInDb(pagebuf);
        DecreaseTotalBufSize((pagebuf->totallength )* sizeof(char) , NULL);
        mylog_info(m_pLogGlobalCtrl->infolog, "%llu pagemap size %d  pagebuf total size %d totalpage %d nowpage %d - %s:%s:%d", pagebuf->urlnodeid , pagemap.size(),pagebuf->totallength,pagebuf->totalpage,pagebuf->nowpage ,INFO_LOG_SUFFIX);
        delete pagebuf;
        pagemap.erase(iter);
    }
    pthread_rwlock_unlock(&rwlock_page_map);
}   
void PageManager::SaveInDb(PAGEBUF* pagebuf ,UrlNode *urlnode) 
{
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    char dbname[64];  dbname[0] =0;
    char recordname[64];recordname[0] =0;
    DBAccess *dbaccess = DBAccess::getInstance();

    infocrawler->getLocalDbManager()->getContentDBName(pagebuf->taskid, dbname);
    infocrawler->getLocalDbManager()->getRecordKeyName(pagebuf->urlnodeid, recordname);
    Buffer ** pages =  pagebuf->pages;
    int totallen = 0;
    
    Buffer *buffer = create_buffer(100);
    Buffer *contentbuffer = create_buffer(DEFAULT_PAGE_BUF_SIZE);
    char newbuf[64]; newbuf[0] = 0;
    int ret = 0;
    
    if (icconfig->ifdef != PAGEMANAGE_DEF)
    {
        ret = sprintf(newbuf, "%d/%d/%d/%d/%d/", pagebuf->totallength, pagebuf->totalpage, pagebuf->nowpage, pagebuf->taskid, pagebuf->batchid);
        add_buffer(buffer, newbuf, ret);
        int offset = 0;
        for(int i = 0; i<= MAXFINALNEXTPAGETOFETCH ; i++)
        {
           Buffer * page = pages[i];
           if (page != NULL)
           {
               ret = sprintf(newbuf, "%llu/%d/%d/%d/", pagebuf->urlnodeid, offset , page->length,i);
               add_buffer(contentbuffer , page->data, page->length);
               add_buffer(buffer, newbuf, ret);
               offset += page->length; 
           }
        }
    }else
    {
        int totalbufsizetmp = 0;
        int offset = 0;
        Buffer *buffertmp = create_buffer(100);
        for(int i = 0; i<= MAXFINALNEXTPAGETOFETCH ; i++)
        {
           Buffer * page = pages[i];
           if (page != NULL)
           {
               char *tmp = NULL;
               size_t tmplen = 0;
               inf(page->data, page->length,  &tmp, &tmplen);
               ret = sprintf(newbuf, "%llu/%d/%d/%d/", pagebuf->urlnodeid, offset , tmplen,i);
               add_buffer(buffertmp, newbuf, ret);
               add_buffer(contentbuffer, tmp, tmplen);
               offset += tmplen; 
               free(tmp);
               totalbufsizetmp += tmplen;
           }
        }
        ret = sprintf(newbuf, "%d/%d/%d/%d/%d/", totalbufsizetmp, pagebuf->totalpage, pagebuf->nowpage, pagebuf->taskid, pagebuf->batchid);
        add_buffer(buffer, newbuf, ret);
        add_buffer(buffer, buffertmp->data, buffertmp->length);
        free_buffer(buffertmp);
    }

    int headlength = buffer->length;

    char *headbuf = (char *)malloc(sizeof(char) * headlength + 32);
    int headlength_tmp = sprintf(headbuf, "%8d/%s", headlength + 8 + 1, buffer->data); //8 for %8d, 1 for /

    size_t storedatlen = compressBound(contentbuffer->length) + headlength_tmp;
    char *storedata = (char *)malloc(storedatlen);
    memcpy(storedata, headbuf, headlength_tmp);
    storedatlen -= headlength_tmp;

    int compressret = def(contentbuffer->data, contentbuffer->length, storedata + headlength_tmp, &storedatlen, -1);

    if (compressret != Z_OK || pagebuf->totallength == storedatlen) {
       mylog_error(m_pLogGlobalCtrl->errorlog, "can't def content storedatlen = %d head.totallength = %d in multipage %d - %s:%s:%d",storedatlen, pagebuf->totallength, pagebuf->totalpage ,INFO_LOG_SUFFIX);
    }

    if (storedatlen > DATLEN_MAX) {
        if (urlnode)
        {
            mylog_error(m_pLogGlobalCtrl->errorlog, "exceed max dat len url = %s taskid = %d id = %llu - %s:%s:%d",urlnode->url,pagebuf->taskid, pagebuf->urlnodeid ,INFO_LOG_SUFFIX);
        }
    }
    int suffix = dbaccess->load(dbname);
    string fileno;
    DBD *dbd = dbaccess->get(suffix, fileno, recordname, NULL);
    if (dbd == NULL)
    {
        ret = dbaccess->store(suffix, recordname, storedata, headlength_tmp + storedatlen, fileno, NULL);
        if ((ret == 0 || ret == 1) && urlnode!= NULL ) {
            /*    ret = infocrawler->getLocalDbManager()->saveUrl(urlnode);
                if (ret != 0 && ret != 1) {
                    mylog_error(m_pLogGlobalCtrl->errorlog, "SaveInDb can not store url into db %s %s - %s:%s:%d", dbname, recordname,INFO_LOG_SUFFIX);*/
        }else
        {
            mylog_error(m_pLogGlobalCtrl->errorlog, "can't store content into db %s %s - %s:%s:%d", dbname, recordname,INFO_LOG_SUFFIX);
        }
    }else
    {
         mylog_error(m_pLogGlobalCtrl->errorlog, "dbd is not NULL when SaveInDb taskid %d urlnodeid %llu totalpage %d nowpage %d - %s:%s:%d", urlnode->taskid, urlnode->id, pagebuf->totalpage,  pagebuf->nowpage, recordname,INFO_LOG_SUFFIX);
    }
    dbd_free(dbd);
    free(storedata);
    free(headbuf);
    free_buffer(buffer);
    free_buffer(contentbuffer);
}
void PageManager::SaveInDb(char * content,int contentlen, UrlNode *urlnode, RESPONSE_HEADER *rheader) 
{
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    infocrawler->getLocalDbManager()->savecontent(urlnode, rheader, content, contentlen, urlnode->nextpage);
}

int PageManager::DecreaseTotalBufSize(size_t decreasesize, UrlNode* urlnode)
{
    if (urlnode )
    {
        mylog_info(m_pLogGlobalCtrl->infolog, "urlnode id %llu url %s totalpagebufsize %d decreasesize %d decrease after %d - %s:%s:%d", urlnode->id,urlnode->url, totalpagebufsize, decreasesize, totalpagebufsize - decreasesize,INFO_LOG_SUFFIX);
    }
    pthread_rwlock_wrlock(&rwlock_page_buf_size);
    totalpagebufsize -= decreasesize;
    pthread_rwlock_unlock(&rwlock_page_buf_size);
}
int PageManager::IncreaseTotalBufSize(size_t addsize, UrlNode* urlnode)
{
    int ret = -1;
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    pthread_rwlock_wrlock(&rwlock_page_buf_size);
    if ((totalpagebufsize + addsize) < icconfig->maxpagebufsize)
    {
        mylog_info(m_pLogGlobalCtrl->infolog, "urlnode id %llu url %s totalpagebufsize %d increasesize %d increase after %d - %s:%s:%d", urlnode->id,urlnode->url, totalpagebufsize, addsize, totalpagebufsize + addsize,INFO_LOG_SUFFIX);
        totalpagebufsize = totalpagebufsize + addsize;
        ret = totalpagebufsize;
    }
    pthread_rwlock_unlock(&rwlock_page_buf_size);
    if (ret < 0){
        mylog_info(m_pLogGlobalCtrl->infolog, "buf too large urlnode id %llu url %s totalpagebufsize %d increasesize %d increase after %d definx %d  - %s:%s:%d", urlnode->id,urlnode->url, totalpagebufsize, addsize, totalpagebufsize + addsize, icconfig->maxpagebufsize,INFO_LOG_SUFFIX);
    }
    return ret;
}
int PageManager::GetTotalBufSize()
{
    int ret = 0;
    pthread_rwlock_rdlock(&rwlock_page_buf_size);
    ret = totalpagebufsize;
    pthread_rwlock_unlock(&rwlock_page_buf_size);
    return ret;
}

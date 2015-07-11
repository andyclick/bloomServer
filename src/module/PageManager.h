#ifndef KUROBOT_PAGEMANAGER_H
#define KUROBOT_PAGEMANAGER_H

//#include "InfoCrawler.h"
#include "Manager.h"
#include "ic_types.h"
#include <map>
using namespace std;
#define MAXFINALNEXTPAGETOFETCH         30

#define NEED_SAVE_URL       30
#define PAGEMANAGE_DEF      1
typedef struct _PAGEBUF{
    Buffer ** pages; 
    int totalpage;
    int nowpage;
    int taskid;
    int batchid;
    int totallength; 
    ulonglong urlnodeid;
    _PAGEBUF()
    {
        //ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
        //page = new  (Buffer *)[icconfig->maxfinalnextpagetofetch + 1]; 
        pages = new  Buffer *[MAXFINALNEXTPAGETOFETCH + 1]; 
        for(int i = 0; i<= MAXFINALNEXTPAGETOFETCH ; i++)
        {
            pages[i] = NULL;
        }
        totalpage=0;
        nowpage=0;
        taskid=0;
        batchid=0;
        totallength = 0 ;
        urlnodeid = 0;
    }
    ~_PAGEBUF()
    {
       // ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
        for(int i = 0; i<= MAXFINALNEXTPAGETOFETCH ; i++)
        {
            if (pages[i])
            {
                free_buffer(pages[i]);
            }
        }
        if (pages) delete [] pages;
    }
} PAGEBUF;
class PageManager: virtual public Manager{
public:
    PageManager(LogGlobalCtrl * pLogGlobalCtrl);
    ~PageManager();
    int start();
    int stop(); 
    void dump();
    void retrieve();
    int SavePage(char * content,int contentlen, UrlNode *urlnode, RESPONSE_HEADER *rheader); 
    void altermaptotalpageforced(UrlNode *urlnode);
    void MergePage(UrlNode *urlnode, int lastcontentlen); 
    bool FindInDb(UrlNode *urlnode);
    bool FindInDb(int taskid, ulonglong urlnodeid);
    void erasecontent_map(ulonglong urlnodeid);
    bool FindInMap(ulonglong urlnodeid);
private:
    bool FindInMap(UrlNode *urlnode);
    Buffer * CreateBuffer(char * content,int contentlength, UrlNode *urlnode, RESPONSE_HEADER *rheader);
    void SaveInMap(Buffer *buffer, UrlNode *urlnode, int * ret);
    int SaveInMap(Buffer *buffer, UrlNode *urlnode);
    void SaveInDb(char * content,int contentlen, UrlNode *urlnode, RESPONSE_HEADER *rheader);
    int IncreaseTotalBufSize(size_t addsize, UrlNode *urlnode);
    int DecreaseTotalBufSize(size_t decreasesize, UrlNode *urlnode);
    void SaveInDb(PAGEBUF* pagebuf ,UrlNode *urlnode =NULL); 
    void dumpmap(); 
    int GetTotalBufSize();
    int GetMapSize();
private:
    size_t totalpagebufsize;
    map <ulonglong, PAGEBUF*> pagemap;
    static pthread_rwlock_t rwlock_page_map;
    static pthread_rwlock_t rwlock_page_buf_size;
    LogGlobalCtrl       *m_pLogGlobalCtrl;
};
#endif

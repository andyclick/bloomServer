
#include "test.h"
#include "util.h"
#include "cmsinfo.h"
#include "ic_types.h"
#include "MyConnection.h"
#include "utilcpp.h"
#include "UrlAnalyseManager.h"
#include "LocalDbManager.h"
#include "Http.h"
#include "uri.h"
#include "Url.h"
#include "hlink.h"
#include "ic_types.h"
#include <zlib.h>
#include "HtRegex.h"
#include "FSBigFile.h"
#include "DBAccess.h"
#include "DBAccess.h"
#include "InfoCrawler.h"
#include <sys/types.h>
//#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h> 
//#include "Distance.h"
#include "sqlite3.h"
//#include <stdarg.h>. 
#include "mycxxlog.h"
//#include "PriorityQueue.h"
void testExternal(); 
void testCmsinfo(); 
void testdump(); 
void testonfind();
void testpattern();
void testDB(); 
void testGetContentByTaskId(int needtaskid); 
void testGetHttp(char *);
void CheckUrl(char *srcurl, char *fromurl, char *outurl);
bool szReplace1(char* szStr,const char* szSrc,const char* szDst);
bool turntoregular(char * ultimatepageurl);
string  checkPattern(char *pattern);
void testMemcache();
void DeleteListUrl();
void TransferRegEx(char * sOrigi, char * sNew);
bool is(char *url, char *pattern);
void SetUrlToMemecachedb();
void testMemcache1(char *url,char * content, int contentlen);
int onfind(const char *elem, const char *attr, struct uri *uri, void *arg); 
void ReadcontentfromHTML();
void MemcacheDel(char * url);
void testociDb();
char * get_final_content(UrlNode *urlnode);
//sb4 error_proc(dvoid   *errhp,   sword   status);
#define BUF_LEN 1024
#define MAXTHREAD 100 
#define MAX_PUSH   100000
void * thr_fn(void *arg); 
void *testThreadExit();
void *print_message_function( void *ptr );
void testLD();
void testonfind1();
static int callback(void * notused , int agc, char ** argv, char **azColName);
void testSqlite();
void ReadUrlFromFile();
//void testMyPriorityQueue();
void teststl();
void mylog_infotest(const char *stringformat, ...); 
std::string myvalistformtest(const char* format, va_list args); 
pthread_mutex_t mux;
ulonglong GetMaxID(unsigned long pcid);
void testonfind3();
char *strtrim1(char *str, const char *trim);
string stringstrim(string &src, const char *trim); 
string string_trim(string &str); 
string GetUserName(char * url);

void get_decompress(char * content)
{
   ContentHead *contenthead = new ContentHead;
   LocalDbManager::readHead(content, contenthead);
   char *tmp = NULL;
   size_t tmplen = contenthead->totallength + 1;
   int compressret = inf(content + contenthead->headlength,4089- contenthead->headlength,&tmp, &tmplen);
    if (compressret != Z_OK || tmplen != contenthead->totallength)
    {

    }
    else
    {
       FILE *f = fopen("c.dat", "w+");
       fwrite(tmp, 1, tmplen, f);
       fclose(f);
    }

}



/*template< typename T >
struct aligned_sizeof
{
    enum
    {
        no_rtti   = ( sizeof( T ) + AlignmentT - 1 ) & ~( AlignmentT - 1 ),
        with_rtti = RuntimeTypeCheckT ?
            no_rtti + aligned_sizeof<vtable_ptr>::no_rtti :
            no_rtti
    };
};
struct aa{
    struct bb{
        int a ;
        int b;
    };
};*/
int main_test(int ac, char **av) {
    testDB();
    //ReadcontentfromHTML();
    //ReadUrlFromFile();
    //testGetHttp("http://auto.cnr.cn/news/storys_12143_1.html");
    return 0;
    //char * a = (char *) malloc(10); 
    //char * b = (char *) malloc(10); 
    /* aa::bb c;
    int size = aligned_sizeof<aa::bb>;
    printf("%d\n",size);*/
    /*char *hccheckdate = "hccheckdate\n";
    char *date_value = "date_value";
    FILE *logfp;
    logfp = fopen("los1.log", "w+");
    fwrite(hccheckdate, strlen(hccheckdate), 1, logfp);
    fwrite(date_value, strlen(date_value), 1, logfp);
    char aa[] = "\r\n";
    fwrite(aa, 2, 1, logfp);
    fflush(logfp);
    fclose(logfp);*/
    // unsigned long long int id = 11073231593261760512L;
/*    char a[] = "abc";
    char * b = new char [strlen(a) + 1];
    int i = 0;
    for(i=0 ; i<strlen(a);i++)
    {
        b[i] = a[i];
    }
    b [i] = 0;*/
    return 0;
    /*  double *a=(double*)malloc(10*sizeof(double)); 
        a[10]=10;
        char * b = (char *)malloc(10*sizeof(char));
        b[0] = 0;*/
    /*    char string[] = "wardsaaaaseparatedaaaabyaaaaspaces";
          char *running;
          char *token;

          running=string;
          while((token = strsep (&running,"aaaa"))!=NULL)
          printf("%s\n",token);
          */
    //printf("%d\n",string::npos);   
    //    testonfind3();
    //pthread_mutex_init(&mux,0);
    //printf("%llu \n",GetMaxID(9));
    //  mylog_infotest("i want test parameter - %s :  %s : %d \n",__FILE__,__FUNCTION__,__LINE__); 
    //teststl();
    //testMyPriorityQueue();
    //printf("%d\n",sizeof (ContentHead));
    //testociDb();
    //testExternal(); 
    //testSqlite();
    /*    char out[64] = "";
          CheckUrl("?p=2", "http://auto.haixiachina.com/article/2009/0724/Uw5TBlJcOmI4NDIwNGJi.html?p=1", out);
          printf("out %s\n", out); 
          */  
    // testonfind1();
    /*    if (uri_combine(page_uri, buff, MAX_URL_LEN - 1, C_SCHEME | C_AUTHORITY | C_PATH | C_QUERY ) >= 0)
          {

          }
          */  
    // testLD();
    //testThreadExit();
    /*   int err; 
         int i; 
         pthread_t ntid[MAXTHREAD];
         string message = "";
         char stri[8] = "";   
         void *pth_join_ret;
         for (int j = 1;j<=6;j++)
         {
         for (i = 0; i < MAXTHREAD; i++) 
         { 
         message = "thread";
         sprintf(stri,"%d",i);
         message.append(stri);
         err = pthread_create(&ntid[i], NULL, thr_fn, (void *)message.c_str()); 
         if (err != 0) 
         fprintf(stderr, "%d can't create thread %d: %s\n",j, i, strerror(err)); 
         else 
         printf("%d thread %d create success\n",j, i); 
         }
         for (i = 0; i < MAXTHREAD; i++) 
         {
         printf(" %d thread %d  joined\n",j, i); 
         pthread_join(ntid[i], &pth_join_ret);
         }*/
    /*for (i = 0; i < 100000; i++) 
      {
      sleep(1000);
      printf("at while\n");
      }*/
    // }
    //sleep(500000); 
    /* string oldfile = "/app/spider1/data/idx_template";
       string newfile = "/app/spider1/logs/idx_template";
       string dbpath = "/app/spider1/data";
    //if (rename(oldfile.c_str(),newfile.c_str())==0)
    if (true)
    {
    myrmdir((char *)dbpath.c_str());
    //myrmdir1(dbpath.c_str());
    //if (rm(dbpath.c_str()))
    {
    if (mkdir(dbpath.c_str(),S_IRWXO|S_IRWXG|S_IRWXU) == 0)
    {
    if (rename(newfile.c_str(),oldfile.c_str()) != 0)
    {
    errorlog("Can't mv from %s to %s\n",oldfile.c_str(),newfile.c_str());
    }else
    {
    return true;
    }
    }
    }
    }else
    {
    errorlog("Can't mv from %s to %s\n",oldfile.c_str(),newfile.c_str());
    }*/
    /*    Buffer *b = create_buffer(1);
          add_buffer(b, "1", 1);
          free_buffer(b);
          printf("ok\n");
          */  //testDB();
    //testociDb();
    //    testonfind();
    return 0;
    //SetUrlToMemecachedb();
    /*    list<string> ooo;
          ooo.push_back("1");
          ooo.push_back("2");
          ooo.push_back("3");
          string n = ooo.front();
          ooo.pop_front();
          ooo.push_back("4");
          printf("%s\n", n.c_str());
          return 0;*/
    /*UrlAnalyseManager manager;
      bool ret = manager.is("http://book.sina.com.cn/excerpt/sz/rw/2009-05-08/1043255764.shtml", "http://book\\.sina\\.com\\.cn/excerpt/sz/rw/[0-9]+-[0-9]+-[0-9]+/[0-9]+\\.shtml");
      printf("ret = %d\n", ret);
      */
    /*unsigned long long a1 = 7;
      int a2 = 6;
      printf("%d/%d\n", a1, a2);
      */
    //testonfind();
    //testpattern();
    //testdump(); 
    //DeleteListUrl();
    //testDB(); 
    //testGetHttp("http://www.texindex.com.cn/news/list--b10277b17713b12094b12603------1.html");
    //testGetHttp("http://www.pcpop.com");
    //testGetHttp("http://www.texindex.com.cn/Articles/2009-6-30/182360.html");
    //testGetContentByTaskId(-1);
    /*char filename[256] = "";
      static vector<string> dirs;
      dirs.push_back("/usr/local/spider1/checkpoint");
      sprintf(filename, "/usr/local/spider1/checkpoint/tasks.dat");
      FSBigFile bigfile(filename, &dirs, 'w');
      */// testExternal();
    /*char strTime[128] = "";                                                                                               
      time_t tDate;                                                                                                         
      time(&tDate);
      strftime(strTime, 127,"%Y-%m-%d %H:%M:%S", localtime(&tDate));                                                        

      char key[64] = "";                                                                                                    
      char a[] = {9,0};
      int keylen = sprintf(key, "%d%s%s\n",1,a ,strTime);
      printf("%s\n",key);
      */
    /*char str[]="153848/98359\t2010-03-03 14:18:15\n";

    //char key[32];
    char * key = (char *)malloc(sizeof(char) *64);
    key[0] = 0; 
    sscanf(str,"%s\t%*s",key);

    printf("%s\n",key);
    free(key);*/
#ifdef COREDUMP
    SetCoreDump();
#endif
    /*char *a = NULL;
      strcpy(a, "10");
      int c = 0;
      int b = 1/c;
      */
    /*  FILE *f = fopen("/backup/spider1-1.0.0.1/src/listurl.txt", "r");
        fseek(f,0,SEEK_END);
        int filelen = ftell(f);
        rewind(f);
        char *strlisturl= new char [filelen + 1];
        fread(strlisturl,1,filelen,f);
        strlisturl[filelen]=0;
        fclose(f);
        char *pageurl= strtrim(strlisturl, NULL);
        string pattern = checkPattern(pageurl);
        is("http://www.pcpop.com/common/Article_64_007200216_6_201_30.htm",(char *)pattern.c_str());
        */return 1;
}
/*void testociDb()
{
    int readdbnum= 0;
    OCIEnv  *envhp = NULL;
    OCIError *errhp = NULL;
    OCIServer *srvhp = NULL;
    OCISvcCtx *svchp = NULL;
    OCIDefine *defhp = NULL;
    OCIEnvCreate(&envhp ,OCI_THREADED ,(dvoid *)0 , 0 , 0 , 0 , 0, (dvoid **)0);
    error_proc(errhp ,OCIHandleAlloc((dvoid   *)   envhp , (dvoid   **)&errhp , OCI_HTYPE_ERROR , (size_t)0 ,(dvoid **)0));
    error_proc(errhp , OCIHandleAlloc((dvoid *)envhp , (dvoid **)&srvhp , OCI_HTYPE_SERVER , (size_t)0 , (dvoid **)0));
    error_proc(errhp , OCIHandleAlloc((dvoid *)envhp , (dvoid **)&svchp , OCI_HTYPE_SVCCTX , (size_t)0 , (dvoid   **)0));
    error_proc(errhp , OCIAttrSet((dvoid   *)svchp , OCI_HTYPE_SVCCTX , (dvoid *)srvhp , (ub4)0 , OCI_ATTR_SERVER , (OCIError   *)errhp));
    sword status =-1;
    string username = "qiruan";
    string servername = "qiruan";
    string pasword = "edpri_new";
    status = OCILogon(envhp , errhp , &svchp , (unsigned char *)username.c_str() , username.length() , (unsigned char *)pasword.c_str(),pasword.length() , (unsigned char *)servername.c_str(), servername.length());
    if (status == OCI_SUCCESS) {
        printf("SUCESS connect to %s,USERNAME is %s\n" , servername.c_str(), username.c_str());
    } else {
        //printf("user :%s,password:%s,dbservername:%s\n",icconfig->dbuser,icconfig->dbpassword,icconfig->dbservername);
        printf("connect   fail!\n");
        printf("-----ORA_search,ERROR in OCILogon-----\n");
        error_proc(errhp,status);
    }
    status =-1;
    OCIStmt *stmthp = NULL;
    OCIStmt *stmthp_update = NULL;
    error_proc(errhp , OCIHandleAlloc((dvoid *)envhp , (dvoid **)&stmthp , OCI_HTYPE_STMT , (size_t)0 , (dvoid   **)0));
    //error_proc(errhp , OCIHandleAlloc((dvoid *)envhp , (dvoid **)&stmthp_update , OCI_HTYPE_STMT , (size_t)0 , (dvoid   **)0));
    sb4 errornum =0;
    string sql = "update is_sitesource set status = '1' where id = 2556";
    //string sql = "update downloadlogs set donetime = sysdate, donequantity = 0 , uniquequantity = '82' where source_id = 598 and batch  =  24890283";
    //string sql = "update is_sitesource set begintime = sysdate + 18289030/(24*3600) where id  = 278";

    selectType TotalRows ;
    TotalRows.intnum= 0;

    char * sqlstr = new char[sql.length() + 10];
    sprintf(sqlstr,"%s",sql.c_str());
    error_proc(errhp , OCIStmtPrepare(stmthp ,errhp ,reinterpret_cast<unsigned char *> (sqlstr), (ub4)sql.length(),
            OCI_NTV_SYNTAX, OCI_DEFAULT));

    if ((status=OCIStmtExecute(svchp , stmthp , errhp , (ub4)1 , (ub4)0 , (CONST OCISnapshot *)NULL ,  (OCISnapshot *)NULL ,
                0)) &&  status != OCI_SUCCESS_WITH_INFO)
    {
        error_proc(errhp,status);
    }
    ////OCIAttrGet((dvoid*)stmthp, OCI_HTYPE_STMT, (dvoid*)&TotalRows, (ub4 *)0, OCI_ATTR_ROW_COUNT, errhp);
    ////Log("OCI_SUCCESS sql %s TotalRows %d\n",sql.c_str(),TotalRows);  
    if (sqlstr) delete[] sqlstr;
    OCIHandleFree(stmthp,OCI_HTYPE_STMT);

    //string sql = "select id, source,site,finalpageformat,interval,listpageformat,depth,charset,to_char(begintime,'yyyymmddhh24miss'),shieldedformat,sourcetype_id , state, PRIORITY, SENDTYPE , TARGET ,INTERVALAUTO from is_sitesource where state = 1 and mark = 1  and site is not null";

    /*string  sqlinsert ="insert into downloadlogs (id ,source_id, batch,begintime,PREDOWNLOADTIME, spiderid) values (SEQ_DOWNLOADLOGS.nextval ,";
      sqlinsert.append("555");sqlinsert.append(",");
      sqlinsert.append("55555");sqlinsert.append(",");
      sqlinsert.append("sysdate, sysdate, ");
      string sql = sqlinsert;
      errornum = error_proc(errhp , OCIStmtPrepare(stmthp ,errhp ,reinterpret_cast<unsigned char *> ((char *)sql.c_str()) , (ub4)(sql.length()) ,OCI_NTV_SYNTAX ,OCI_DEFAULT));
      Task *task = new Task;
      char begintime[32]={0};
      char depth[32] = {0};
      char tasksendtype[32] = {0};
      char interval[64] = {0};
      char priority[4] = {0};
      */

    /* OCIDefineByPos(stmthp, &defhp, errhp, 1, &task->id,sizeof(task->id), SQLT_INT, NULL, NULL, NULL, OCI_DEFAULT);
       OCIDefineByPos(stmthp, &defhp, errhp, 2, &task->name,sizeof(task->name), SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT);
       OCIDefineByPos(stmthp, &defhp, errhp, 3, &task->homeurl,sizeof(task->homeurl), SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT);
       OCIDefineByPos(stmthp, &defhp, errhp, 4, &task->ultimatepageurl,sizeof(task->ultimatepageurl), SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT);
       OCIDefineByPos(stmthp, &defhp, errhp, 5, &interval,sizeof(interval), SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT);
       OCIDefineByPos(stmthp, &defhp, errhp, 6, &task->pageurl,sizeof(task->pageurl), SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT);
       OCIDefineByPos(stmthp, &defhp, errhp, 7, &depth,sizeof(task->depth), SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT);
       OCIDefineByPos(stmthp, &defhp, errhp, 8, &task->charset,sizeof(task->charset), SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT);
       OCIDefineByPos(stmthp, &defhp, errhp, 9, &begintime, sizeof(begintime),SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT);
       OCIDefineByPos(stmthp, &defhp, errhp, 10, &task->ignoreurl,sizeof(task->ignoreurl), SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT);
       OCIDefineByPos(stmthp, &defhp, errhp, 11, &task->sourcetype,sizeof(task->sourcetype), SQLT_INT, NULL, NULL, NULL, OCI_DEFAULT);
       OCIDefineByPos(stmthp, &defhp, errhp, 12, &task->sourcestate,sizeof(task->sourcestate), SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT);
       OCIDefineByPos(stmthp, &defhp, errhp, 13, &priority,sizeof(priority), SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT);
       OCIDefineByPos(stmthp, &defhp, errhp, 14, &tasksendtype,sizeof(tasksendtype), SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT);
       OCIDefineByPos(stmthp, &defhp, errhp, 15, &task->loginurl,sizeof(task->loginurl), SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT);
       OCIDefineByPos(stmthp, &defhp, errhp, 16, &task->intervalauto,sizeof(task->intervalauto), SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT);
       */
    //if ((status=OCIStmtExecute(svchp , stmthp , errhp , (ub4)0 , (ub4)0 , (CONST OCISnapshot *)NULL ,  (OCISnapshot *)NULL , OCI_DEFAULT)) &&  status !=     OCI_SUCCESS_WITH_INFO)
    /*if ((status=OCIStmtExecute(svchp , stmthp , errhp , (ub4)1 , (ub4)0 , (CONST OCISnapshot *)NULL ,  (OCISnapshot *)NULL , OCI_COMMIT_ON_SUCCESS)) && status != OCI_SUCCESS_WITH_INFO)
      {
      error_proc(errhp,status);
      errorlog("ERROR: -----ORA_search,ERROR   in   OCIStmtExecute-----\n");
      delete task;
      OCIHandleFree(stmthp,OCI_HTYPE_STMT);
      OCIHandleFree(stmthp_update,OCI_HTYPE_STMT);
      return ;
      }else
      {
      return;
      }
      int a=0;
      sword swResult;
      readdbnum++;
      printf("Fetch num :%d\n",readdbnum);
      while((swResult = OCIStmtFetch (stmthp,errhp,(ub4)1,(ub4)OCI_FETCH_NEXT,(ub4)OCI_DEFAULT)) != OCI_NO_DATA)
      {

      printf("Fetch loop :%d readnum = %d\n",a, readdbnum);
      a++;
      char sqlupdate[1024];
      sqlupdate[0]=0;
      if (priority[0] == '9')
      {
      sprintf(sqlupdate,"update is_sitesource set mark = '2', priority = '1'  where id = '%d'",task->id);
      }else
      {
      sprintf(sqlupdate,"update is_sitesource set mark = '2' where id = '%d'",task->id);
      }
      error_proc(errhp , OCIStmtPrepare(stmthp_update,errhp ,reinterpret_cast<unsigned char *> (sqlupdate) , (ub4)strlen((char *) sqlupdate) ,OCI_NTV_SYNTAX ,OCI_DEFAULT));

      if ((status=OCIStmtExecute(svchp , stmthp_update, errhp , (ub4)1 , (ub4)0 , (CONST OCISnapshot *)NULL ,  (OCISnapshot *)NULL , OCI_COMMIT_ON_SUCCESS)) && status != OCI_SUCCESS_WITH_INFO)
      {
      error_proc(errhp,status);
      errorlog("-----ORA_update,ERROR   in   OCIStmtExecute-----\n");
      delete task;
      OCIHandleFree(stmthp,OCI_HTYPE_STMT);
      OCIHandleFree(stmthp_update,OCI_HTYPE_STMT);
      return ;
      }
      if (depth[0])
      {
      task->depth = atoi(depth);
      if (task->depth ==0)
      {
      task->depth = 3;
      }
      }else
      {
      task->depth = 3;
      }
      if (tasksendtype[0])
      {
      task->tasksendtype = atoi(tasksendtype);
      }else
      {
      task->tasksendtype = REQUEST_TYPE_GET;
      }
      task->interval = atoi(interval);
      task->interval *= 60;
      if (task->interval == 0) {
      task->interval  = 30 * 60;
      }
      task->currentInterval = task->interval;
      task->timeOfTheseFetch =  time(NULL);

      printf("get task->id is %d\n",task->id);
      printf("get task->name is %s\n",task->name);
      printf("get task->homeurl is %s\n",task->homeurl);
      printf("get task->ultimatepageurl is %s\n",task->ultimatepageurl);
    printf("get task->interval is %d\n",task->interval);
    printf("get task->pageurl is %s\n",task->pageurl);
    printf("get depth is %d\n",task->depth);
    printf("get task->charset is %s\n",task->charset);
    printf("get begin time %s task->timeToFetch is %d\n",begintime,task->timeToFetch);
    printf("get task->ignoreurl is %s\n",task->ignoreurl);
    printf("get task->sourcetype is %d\n",task->sourcetype);
    printf("get task->sourcestate is %s\n",task->sourcestate);
    printf("get task->tasksendtype is %d\n",task->tasksendtype);
    printf("get task->loginurl is %s\n",task->loginurl);
    printf("get task->intervalauto is %s\n",task->intervalauto);
}
delete task;

OCIHandleFree(stmthp,OCI_HTYPE_STMT);
OCIHandleFree(stmthp_update,OCI_HTYPE_STMT);

}*/
void ReadcontentfromHTML()
{
    DBAccess *access = DBAccess::getInstance();
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    access->setDir("/app/spider_for_ali_free/data");
    static vector<string> dirs;
    dirs.push_back("/app/spider_for_ali_free/data");
    

    ulonglong id = 11210475317393133568LLU;
    //unsigned long long int id = 11073231593261760512L;
    //unsigned long long int id = 11073231593261760512;
//    ulonglong id = 10000000000000000000;

    DBD *dbd = infocrawler->getLocalDbManager()->readContent(id , 9);
    if (dbd) {
        if (dbd->datlen_u == 0)
        {
            printf("dbd->datlen_u is 0\n");
            return;
        }
        ContentHead *contenthead = new ContentHead;
        LocalDbManager::readHead(dbd->datbuf, contenthead);

        char *tmp = NULL;
        size_t tmplen = contenthead->totallength;
        int compressret = inf(dbd->datbuf+ contenthead->headlength, dbd->datlen_u- contenthead->headlength,  &tmp, &tmplen);

        char a[256] = "";
        strncpy(a, dbd->datbuf, 255);
        a[255] = 0;
        printf("b = %s\n", a);

        if (compressret != Z_OK || tmplen != contenthead->totallength) {
            printf("contenthead->headlength = %d contenthead->totallength = %d contenthead->totalpage = %d \
                contenthead->nowpage = %d contenthead->taskid = %d tmplen = %d\n",
                contenthead->headlength, contenthead->totallength, contenthead->totalpage,
                contenthead->nowpage, contenthead->taskid, tmplen);
        }

        char filename[128] = "";
        sprintf(filename, "/app/spider1/f%d", 1);
        FILE *f = fopen(filename, "wb+");
        fwrite(tmp, 1, tmplen, f);
        fwrite("\n", 1, 1, f);
        fclose(f);

        sprintf(filename, "/app/spider1/f%d");
        f = fopen(filename, "a+");
        fwrite(dbd->datbuf+ contenthead->headlength, 1, contenthead->totallength, f);
        fwrite("\n", 1, 1, f);
        fclose(f);
        free(tmp);
    }
    return;
}
/*void testMemcache()
  {
  Connection connection;
  string servername = "127.0.0.1";
  int ret = openConnect(connection, servername, 21201, 0, 3);
  if (ret == 1) {
  ret = connectServer(connection);
  }
  char url[] = "http://swingchen.javaeye.com/blog/152800";
  int len = strlen(url);
  char strlen [10]= "";
  char returnval[1024] = "";
  sprintf(strlen, "%d", len);
  string sendstr = "set ";sendstr.append(url); sendstr.append(" 0 0 "); sendstr.append(strlen); sendstr.append( "\r\n");
  ret = connection.Write((char *)sendstr.c_str(), sendstr.length());    
  ret = connection.Write(url);
  ret = connection.Write("\r\n");
  ret = connection.Read(returnval, 1024);
  connection.Close();
  }*/
/*void testMemcache()
  {
  memcache *mc = mc_new(); 
  mc_server_add(mc, "127.0.0.1", "21201");

  int expire = 100;
  char url[] = "http://swingchen.javaeye.com/blog/152800";
  int len = strlen(url);
  char valurl[] = "val:http://swingchen.javaeye.com/blog/152800";
  int vallen = strlen(valurl);
  int flags  = 0; 
//mc_set(mc, url, len, valurl, vallen, expire, flags); 

char *a = "ddddddddxxxxx";
memcache_req *req = NULL;
req = mc_req_new();
memcache_res *res = mc_req_add(req, a, strlen(a));   

printf("res->_flagss %d\n",res->_flags);
req = mc_req_new();
res = mc_req_add(req, a, strlen(a));   
mc_get(mc, req);
printf("res->_flagss %d\n",res->_flags);
mc_res_free(req, res);

req = mc_req_new();
res = mc_req_add(req, a, strlen(a));   
printf("res->_flagss %d\n",res->_flags);
mc_get(mc, req);
printf("res->_flagss %d\n",res->_flags);
mc_res_free(req, res);

req = mc_req_new();
res = mc_req_add(req, a, strlen(a));   
printf("res->_flagss %d\n",res->_flags);
mc_get(mc, req);
printf("res->_flagss %d\n",res->_flags);

mc_res_free(req, res);

req = mc_req_new();
res = mc_req_add(req, url, len);   
printf("res->_flagss %d\n",res->_flags);
mc_get(mc, req);
if (res->_flags & MCM_RES_FOUND )
printf("result: %.*s\n",res->bytes,res->val);
printf("res->_flagss %d\n",res->_flags);
mc_res_free(req, res);

req = mc_req_new();
res = mc_req_add(req, url, len);   
printf("res->_flagss %d\n",res->_flags);
mc_get(mc, req);
if (res->_flags & MCM_RES_FOUND )
printf("result: %.*s\n",res->bytes,res->val);
printf("res->_flagss %d\n",res->_flags);
//mc_res_free(req, res);

printf("res->_flagss %d\n",res->_flags);
mc_get(mc, req);
if (res->_flags & MCM_RES_FOUND )
printf("result: %.*s\n",res->bytes,res->val);
printf("res->_flagss %d\n",res->_flags);
mc_res_free(req, res);

// printf("%s\n", mc_aget(mc, url, len));

 *int hold_timer = 0;
 printf("%d",(int)mc_delete(mc, url, len, hold_timer));
 */
/*int ret = mc_incr(mc, url, len, 10);
//mc_decr(mc, url, len, 5);

printf("%s\n", mc_aget(mc, url, len));

mc_free(mc);
}*/
int getTaskDB(int taskid) {
    return taskid % TASKDB_NUM;    
}

void getUrlDBName1(UrlNode *urlnode, char *dbname) {
    sprintf(dbname, "url_task_%d", getTaskDB(urlnode->taskid));
}

void getContentDBName1(int taskid, char *dbname) {
    sprintf(dbname, "task_%d", getTaskDB(taskid));
}

void getContentDBName1(UrlNode *urlnode, char *dbname) {
    getContentDBName1(urlnode->taskid, dbname);
}


void getRecordKeyName1(ulonglong id, char *recordname) {
    sprintf(recordname, "%llu", id);
}

void getRecordKeyName1(UrlNode *urlnode, char *recordname) {
    //sprintf(recordname, "%llu", urlnode->id);
    getRecordKeyName1(urlnode->id, recordname);
}
void testGetHttp(char *urltmp) {
    CURL *curl = curl_easy_init();
    CURLSH *sh;
    sh = curl_share_init();
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, ""); //just to start the cookie engine
    curl_easy_setopt(curl, CURLOPT_SHARE, sh);
    curl_easy_reset(curl);
    
    UrlNode *urlnode = new UrlNode;
    urlnode->copyurl(urltmp);
    HttpProtocol httpprotocol;
    CUrl url;
    url.parse(urltmp);
    Buffer *content = create_buffer(1024 * 20);
    Page page;
    char downstatistic[512] ;downstatistic[0] = 0;
    RESPONSE_HEADER rheader;
    
    int ret = httpprotocol.curl_fetch(curl, url, content, urlnode, 10, 1, &rheader, downstatistic);
    curl_easy_cleanup(curl);
    delete urlnode;
    free_buffer(content);
}

void testGetContentByTaskId(int needtaskid) {
    DBAccess *access = DBAccess::getInstance();
    access->setDir("/usr/local/infocrawler/data");

    static vector<string> dirs;
    dirs.push_back("/usr/local/infocrawler/data");

    char *key = NULL;
    char *data= NULL;
    int keylen = 0;
    int datalen = 0;
    int i = 0;
    int64_t pos = 0;
    char poskey[32] = "";
    char filename[128]= "";
    FSBigFile *bigfile = NULL;
    int ret = 0;

    sprintf(filename, "%s/%s", "/usr/local/infocrawler/data", FETCHED_FIFO_QUEUE_FILE);

    bigfile  = new FSBigFile(filename, &dirs, 'r');

    if (bigfile->seek(0, SEEK_SET) != 0) {
        delete bigfile;
        return;
    }

    list<char *> fetched;

    while(i++ < 1000000 && (ret = ReadContent(bigfile, &key, &keylen, &data, &datalen)) > -1) {
        fetched.push_back(key);
        free(data);
    }

    delete bigfile;

    list<char *>::iterator iter;

    sprintf(filename, "/app/spider1/f%d.txt", needtaskid);
    FILE *f1 = fopen(filename, "w+");
    for(iter = fetched.begin(); iter != fetched.end(); ++iter) {
        ulonglong id = 0;
        int taskid = 0;
        if (*iter) {
            sscanf(*iter, "%llu/%d", &id, &taskid);
            free(*iter);
            if (needtaskid != -1 && taskid != needtaskid) {
                continue;
            }
        } else {
            continue;
        }

        char dbname[16] = "";
        char keyname[32] = "";

        getContentDBName1(taskid, dbname);
        getRecordKeyName1(id, keyname);

        int suffix = access->load(dbname);
        string fileno;

        DBD *dbd = access->get(suffix, fileno, keyname, NULL);
        if (!dbd) continue ;

        if (dbd) {
            //for test
            ContentHead *contenthead = new ContentHead;
            LocalDbManager::readHead(dbd->datbuf, contenthead);

            if (contenthead->totalpage != contenthead->nowpage) {
                dbd_free(dbd);
                printf("Debug: totalpage %d is not equal to %d nowpage in readcontent for LocalDbManager", contenthead->totalpage, contenthead->nowpage);
                return;
            }

            char *tmp = NULL;
            size_t tmplen = dbd->datlen;
            int compressret = inf(dbd->datbuf + contenthead->headlength, dbd->datlen - contenthead->headlength,  &tmp, &tmplen);
            tmp[tmplen] = 0;

            if (compressret != Z_OK || tmplen != contenthead->totallength) {
                char *a = (char *)malloc(contenthead->headlength + 1);
                strncpy(a, dbd->datbuf, contenthead->headlength);
                a[contenthead->headlength] = 0;
                printf("contenthead->headlength = %d contenthead->totallength = %d contenthead->totalpage = %d \
                    contenthead->nowpage = %d contenthead->taskid = %d tmplen = %d\n", 
                    contenthead->headlength, contenthead->totallength, contenthead->totalpage, 
                    contenthead->nowpage, contenthead->taskid, tmplen);
                printf("a = %s\n", a);

            } else {

                for(int i = 0; i < contenthead->totalpage; i++) {
                    if (contenthead->contentPageHead[i].offset + contenthead->contentPageHead[i].length > contenthead->totallength) {
                        printf("totallength = %d offset = %d length = %d i = %d\n", contenthead->contentPageHead[i].offset + contenthead->contentPageHead[i].length, i);
                    } else {
                        char *currentPage = (char *)malloc(contenthead->contentPageHead[i].length * 2 + 1);
                        memcpy(currentPage, tmp + contenthead->contentPageHead[i].offset, contenthead->contentPageHead[i].length);
                        free(currentPage);
                    }
                }


                fwrite(dbd->datbuf, 1, contenthead->headlength, f1);
                fwrite("\n", 1, 1, f1);

                sprintf(filename, "/backup/infocrawler/temp/html_%d_%d.txt", needtaskid, id);
                FILE *f = fopen(filename, "w+");
                int ret = fwrite(tmp, 1, tmplen, f);
                if (ret != tmplen) {
                    printf("can not write file %d %d %d\n", id, tmplen, ret);
                }
                fclose(f);

            }
            free(tmp);
            delete contenthead;
            dbd_free(dbd);
        } else {
            //Debug(0, 1, ("Debug: can not get data id = %d taskid = %d\n", id, taskid));
        }
    }
    fclose(f1);

}

char * get_final_content(UrlNode *urlnode)
{
     char *content=NULL;
     char dbname[64] = "";
     char recordname[64] = "";
     char urldbname[64] = "";
     DBAccess *dbaccess = DBAccess::getInstance();

     getContentDBName1(urlnode, dbname);
     getRecordKeyName1(urlnode, recordname);
     getUrlDBName1(urlnode, urldbname);

     int suffix = dbaccess->load(dbname);
     string fileno;
     DBD *dbd = dbaccess->get(suffix, fileno, recordname, NULL);
     if (dbd != NULL)
     {
       ContentHead *contenthead = new ContentHead;
       LocalDbManager::readHead(dbd->datbuf, contenthead);
        char *tmp = NULL;
       size_t tmplen = contenthead->totallength + 1;
       int compressret = inf(dbd->datbuf + contenthead->headlength, dbd->datlen_u,  &tmp, &tmplen);
       if (compressret != Z_OK || tmplen != contenthead->totallength) 
       {

       }
       else
       {
          content=new char[dbd->datlen_u+1];
          content[0]=0;
         // content=new char[tmplen];
         // content[0]=0;
          strcpy(content,dbd->datbuf);
         // strcpy(content,tmp);
       }
     }

     return content;

}

void testDB() {

    char dbname[64] = "";
    char recordname[64] = "";
    char urldbname[64] = "";
    DBAccess *dbaccess = DBAccess::getInstance();
     DBAccess::getInstance()->setDir("/backup/spider_all/spider/src/data");

    UrlNode *urlnode = new UrlNode;
    urlnode->taskid = 9;
    urlnode->id = 11389413569800998919LLU; 
    //urlnode->id = 1960;

    getContentDBName1(urlnode, dbname);
    getRecordKeyName1(urlnode, recordname);
    getUrlDBName1(urlnode, urldbname);

    int suffix = dbaccess->load(dbname);

    string fileno;

    DBD *dbd = dbaccess->get(suffix, fileno, recordname, NULL);
    if (dbd != NULL) { //new record
        char a[256] = "";
        strncpy(a, dbd->datbuf, 255);
        a[255] = 0;
        printf("a =  %s\n", a);

        ContentHead *contenthead = new ContentHead;
        LocalDbManager::readHead(dbd->datbuf, contenthead);

        FILE *f = fopen("test.dat", "w+");
        fwrite(dbd->datbuf, 1, dbd->datlen_u, f);
        fclose(f);

        char *tmp = NULL;
        size_t tmplen = contenthead->totallength + 1;
        int compressret = inf(dbd->datbuf + contenthead->headlength, dbd->datlen_u,  &tmp, &tmplen);

        if (compressret != Z_OK || tmplen != contenthead->totallength) {
            char a[64] = "";
            strncpy(a, dbd->datbuf, 63);
            printf("contenthead->headlength = %d contenthead->totallength = %d contenthead->totalpage = %d \
                contenthead->nowpage = %d contenthead->taskid = %d tmplen = %d\n", 
                contenthead->headlength, contenthead->totallength, contenthead->totalpage, 
                contenthead->nowpage, contenthead->taskid, tmplen);
            printf("a = %s\n", a);
        } else {
            FILE *f = fopen("c.dat", "w+");
            fwrite(tmp, 1, tmplen, f);
            fclose(f);
            size_t storedatlen = compressBound(contenthead->totallength);
            char *storedata = (char *)malloc(storedatlen);

            int compressret = def(tmp, tmplen, storedata, &storedatlen, -1);
            if (compressret != Z_OK || contenthead->totallength == storedatlen) {
                printf("not ok\n");  
            } else {
                FILE *f = fopen("a.dat", "w+");
                fwrite(storedata, 1, storedatlen, f);
                fclose(f);


                char *tmp = NULL;
                size_t tmplen = storedatlen;
                int compressret = inf(storedata, storedatlen,  &tmp, &tmplen);
                f = fopen("b.dat", "w+");
                fwrite(tmp, 1, tmplen, f);
                fclose(f);
            }
        }

        dbd_free(dbd);
    }
}

void testExternal() {

    MyConnection connection;
    string servername = "127.0.0.1";
    int ret = openConnect(connection, servername, 6000, 0, 3);
    if (ret == 1) {
        ret = connectServer(connection);
    }
    char protocol[17];
    char ack[1] = "";
    sprintf(protocol, "20090910%8d%8d%8d", 1, 1, -1);
    ret = connection.Write(protocol, 32);
    if (ret != 32) {
        printf("can not write %d\n", ret);
        return; 
    }
    char head[8] = "";
    char head1[8] = "";
    int i = 0;
    connection.Read(head1, 8);
    //while(connection.Read(head, 8) == 8) {
    if(connection.Read(head, 8) == 8) {
        size_t len = atoi(head);
        if (len == 0) {
            //     break;
        }
        printf("len = %d\n",len );
        char *content = (char *)malloc(len + 1);
        int ret = connection.Read(content, len);
        if (ret != len) {
            //break;
        }

        ContentHead *contenthead = new ContentHead;
        LocalDbManager::readHead(content, contenthead);

        char *tmp = NULL;
        size_t tmplen = contenthead->totallength;
        int compressret = inf(content + contenthead->headlength, len - contenthead->headlength,  &tmp, &tmplen);

        char a[256] = "";
        strncpy(a, content, 255);
        a[255] = 0;
        printf("b = %s\n", a);

        if (compressret != Z_OK || tmplen != contenthead->totallength) {
            //char a[64] = "";
            //strncpy(a, content, 63);
            printf("contenthead->headlength = %d contenthead->totallength = %d contenthead->totalpage = %d \
                contenthead->nowpage = %d contenthead->taskid = %d tmplen = %d\n", 
                contenthead->headlength, contenthead->totallength, contenthead->totalpage, 
                contenthead->nowpage, contenthead->taskid, tmplen);
            //printf("a = %s\n", a);
        }

        char filename[128] = "";
        sprintf(filename, "/backup/spider1/temp/f%d", i);
        FILE *f = fopen(filename, "wb+");
        fwrite(tmp, 1, tmplen, f);
        fwrite("\n", 1, 1, f);
        fclose(f);

        sprintf(filename, "/backup/spider1/temp/f%d");
        f = fopen(filename, "a+");
        fwrite(content + contenthead->headlength, 1, contenthead->totallength, f);
        fwrite("\n", 1, 1, f);
        fclose(f);

        free(tmp);
        free(content);
        i++;
    }
    connection.Write(ack, 1); //write acknowledge
    connection.Close();

}

void testCmsinfo() {
    /*char *ret = GetUrlsPattern("http://www.sina.com.cn");
    printf("%s\n", ret?ret:"NULL");
    free(ret);

    ret = CheckHtmlPattern("http://www.sina.com.cn", "tmpl");
    printf("%s\n", ret?ret:"NULL");
    free(ret);

    char out[32] = "";
    ret = CheckUrlPattern("http://www.sina.com.cn", "pattern", out);
    printf("%s %s\n", ret?ret:"NULL", out);
    free(ret);

    ret = GetHtmlPattern("http://www.sina.com.cn", "titile", "2009", "sina company", "kurt", "<html><body></body></htm>", "http://www.hc360.com");
    printf("%s\n", ret?ret:"NULL");
    free(ret);*/
    return ;
}

void testdump() {
    char filename[256] = "";
    static vector<string> dirs;
    dirs.push_back("./");
    sprintf(filename, "test.dat");
    FSBigFile bigfile(filename, &dirs, 'w');
    map<int, Task *>::iterator iter;
    char key[16] = "";


    Task *task = new Task; 
    task->id = 1;
    strcpy(task->name, "testsite1");
    strcpy(task->industry, "012");
    strcpy(task->homeurl, "http://book.sina.com.cn/excerpt/index.shtml");
    strcpy(task->ultimatepageurl, "http://book\\.sina\\.com\\.cn/excerpt/sz/mx/[0-9]+-[0-9]+-[0-9]+/[0-9]+\\.shtml");
    //strcpy(task->ultimatepageurl, "http://book\\.sina\\.com\\.cn/excerpt/sz/mx/[0-9]+-[0-9]+-[0-9]+/1452255926\\.shtml");
    //strcpy(task->ultimatepageurl, "http://book\\.sina\\.com\\.cn/excerpt/sz/rw/1028255877\\.shtml");
    task->timeToFetch = time(NULL) + 1; 
    task->interval = 3 * 60;
    task->currentInterval = 3 * 60;

    sprintf(key, "%d", task->id);
    WriteContent(&bigfile, key, strlen(key), (char *)task, sizeof(*task));

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

    sprintf(key, "%d", task->id);
    WriteContent(&bigfile, key, strlen(key), (char *)task, sizeof(*task));
}


int onfind(const char *elem, const char *attr, struct uri *uri, void *arg) {
    char buff[MAX_URL_LEN - 1] = "";
    if (uri_combine(uri, buff, MAX_URL_LEN - 1, C_SCHEME | C_AUTHORITY | C_PATH | C_QUERY ) >= 0)
    {
        printf("%s\n", buff);
    }
    uri_destroy(uri);
    free(uri);
}

void testonfind() {
    FILE *f = fopen("3.html", "rb");
    //char content[] = "<a href=\"../test/test.html\">title</a>";
    char *content = (char *)malloc(1024 * 1024);
    int ret  = fread(content, 1, 1024 * 1024, f);
    content[ret] = 0;
    struct uri page_uri;
    //uri_parse_string("http://www.sxfyw.gov.cn/otype.asp?funo1=ÐÎ¿ìhttp", &page_uri);
    uri_parse_string(content, &page_uri);

    hlink_detect_string(content, &page_uri, onfind, NULL);
    uri_destroy(&page_uri);
    fclose(f);
}

void testonfind1() {
    struct uri page_uri;
    char a[] = "[<a href=\"go2party.php?param=a%3A8%3A%7Bs%3A8%3A%22party_id%22%3Bs%3A7%3A%229541527%22%3Bs%3A10%3A%22party_name%22%3Bs%3A25%3A%22%B1%B1%BE%A9+%B5%DA%B6%FE%BD%A8%D6%FE%B9%A4%B3%CC%D3%D0%CF%DE%B9%AB%CB%BE%22%3Bs%3A11%3A%22card_number%22%3Bs%3A3%3A%22123%22%3Bs%3A7%3A%22case_id%22%3Bs%3A25%3A%22%282009%29%D1%E3%B7%A8%C3%F1%D6%B4%D7%D6%B5%DA00042%BA%C5%22%3Bs%3A8%3A%22reg_date%22%3Bs%3A10%3A%222009-01-03%22%3Bs%3A10%3A%22case_state%22%3Bs%3A6%3A%22%D6%B4%D0%D0%D6%D0%22%3Bs%3A11%3A%22apply_money%22%3Bs%3A9%3A%22702845.21%22%3Bs%3A10%3A%22court_name%22%3Bs%3A20%3A%22%CE%F7%B0%B2%CA%D0%D1%E3%CB%FE%C7%F8%C8%CB%C3%F1%B7%A8%D4%BA%22%3B%7D&type=public&PartyName=%B1%B1%BE%A9&CardNumber=&SelectCourt%5B0%5D=B&Submit=%B2%E9+%D1%AF&zx_next_page=2\">find</a>]";
    uri_parse_string("http://zhixing.court.gov.cn/search/?type=public&PartyName=%C4%DA%C3%C9&CardNumber=&SelectCourt%5B0%5D=B&Submit=%B2%E9+%D1%AF", &page_uri);
    hlink_detect_string(a, &page_uri, onfind, NULL);
    uri_destroy(&page_uri);
}

bool is(char *url, char *pattern) {
    char *pToken = NULL;
    HtRegex regex;
    char *tokbuff = NULL;
    int len = strlen(pattern);
    char *patterntmp = new char[len + 1];
    patterntmp[0] = 0;
    strncpy(patterntmp , pattern,len);
    patterntmp[len] = 0;
    pToken = strtok_r(patterntmp, "\r\n\t",&tokbuff);
    FILE* f = fopen("/backup/spider1-1.0.0.1_back1/src/logs.txt","a+");
    while(pToken)
    {
        regex.set(pToken);
        if (regex.match(url, 0, 0) == 1) {
            string logs = "pattern:";
            logs.append(pToken);
            logs.append("list url:");
            logs.append(url);
            logs.append("\n");
            printf("%s",logs.c_str());
            int ret = fwrite(logs.c_str(), 1, logs.length(), f);
            if (patterntmp) delete [] patterntmp;
            fclose(f);
            return true;
        }
        pToken = strtok_r(NULL, "\r\n\t",&tokbuff);
    }
    fclose(f);
    if (patterntmp) delete [] patterntmp;
    return false;
}

void testpattern() {
    char a[] = "http://product.intozgc.com/cell_phone_77965";
    char pattern[] = "^http://www\\.intozgc\\.com/classlist\\|([0-9]+)\\|1\\.html$";
    printf("%d\n", is(a, pattern));
}
void CheckUrl(char *srcurl, char *fromurl, char *outurl) {
    outurl[0] = 0;
    if (!srcurl[0]) return;
    CUrl url;
    url.parse(srcurl);

    CUrl basefromurl;
    basefromurl.parse(fromurl);

    if (!url.m_sProtocol.empty() || !url.m_sHost.empty()) {
        if (strlen(srcurl) > MAX_URL_LEN - 1) return;
        strcpy(outurl, srcurl);
    } else {
        if (srcurl[0] == '/') {
            if (basefromurl.m_sProtocol.length() + basefromurl.m_sHost.length() + strlen(srcurl) > MAX_URL_LEN - 50) {
                return;
            }
            sprintf(outurl, "%s://%s:%d%s", basefromurl.m_sProtocol.c_str(), basefromurl.m_sHost.c_str(), basefromurl.m_sPort, srcurl);
        } else if (srcurl[0] == '?')
        {
            char baseurl[MAX_URL_LEN] = "";
            char *tmp = strrchr(fromurl, '?');
            if (tmp) {
                strncpy(baseurl, fromurl, tmp - fromurl );
                baseurl[tmp - fromurl] = 0;
                if (strlen(srcurl) + strlen(baseurl) > MAX_URL_LEN - 1) {
                }else
                {
                    sprintf(outurl, "%s%s", baseurl, srcurl);
                }
            }else
            {
                if (strlen(srcurl) + strlen(fromurl) > MAX_URL_LEN - 1) {
                }else
                {
                    sprintf(outurl, "%s%s", fromurl, srcurl);
                }
            }
        }else {
            char baseurl[MAX_URL_LEN] = "";
            char *tmp = strrchr(fromurl, '/');
            if (tmp) {
                strncpy(baseurl, fromurl, tmp - fromurl + 1);
                baseurl[tmp - fromurl + 1] = 0;
                if (strlen(srcurl) + strlen(baseurl) > MAX_URL_LEN - 1) {
                } else {
                    sprintf(outurl, "%s%s", baseurl, srcurl);
                }
            }
        }
        CUrl ret;
        ret.parse(outurl);
        if (ret.getUrl().length() > MAX_URL_LEN - 1) {
            strncpy(outurl, (char *)ret.getUrl().c_str(), MAX_URL_LEN - 1);
            outurl[MAX_URL_LEN - 1] = 0;
        } else {
            strcpy(outurl, (char *)ret.getUrl().c_str());
        }
    }
}
/*void CheckUrl(char *srcurl, char *fromurl, char *outurl) {
  outurl[0] = 0;
  if ((!srcurl[0]) || (!fromurl[0])) return;
  CUrl url;
  url.parse(srcurl);

  CUrl basefromurl;
  basefromurl.parse(fromurl);

  if (!url.m_sProtocol.empty() || !url.m_sHost.empty()) {
  strcpy(outurl, srcurl);
  } else {
  if (srcurl[0] == '/') {
  sprintf(outurl, "%s://%s:%d%s", basefromurl.m_sProtocol.c_str(), basefromurl.m_sHost.c_str(), basefromurl.m_sPort, srcurl);
  } else {
  char baseurl[MAX_URL_LEN] = "";
  char *tmp = strrchr(fromurl, '/');
  if (tmp) {
  strncpy(baseurl, fromurl, tmp - fromurl + 1);
  baseurl[tmp - fromurl + 1] = 0;
  if (strlen(srcurl) + strlen(baseurl) > MAX_URL_LEN - 1) {
  int len = MAX_URL_LEN - strlen(srcurl) - 10;
  if (len < 0) {
  len = MAX_URL_LEN - strlen(baseurl) - 10;
  }
  srcurl[len] = 0;
  }
  sprintf(outurl, "%s%s", baseurl, srcurl);
  } 
  }
  CUrl ret;
  ret.parse(outurl);
  if (ret.getUrl().length() > MAX_URL_LEN - 1) {
  strncpy(outurl, (char *)ret.getUrl().c_str(), MAX_URL_LEN - 1);
  outurl[MAX_URL_LEN - 1] = 0;
  } else {
  strcpy(outurl, (char *)ret.getUrl().c_str());
  }
  }
  }*/
bool szReplace1(char* szStr,const char* szSrc,const char* szDst)
{
    char    *p1 = szStr, *p2;
    int     nSrc = strlen(szSrc),nStr = strlen(szStr),  nDst = strlen(szDst);

    while((p2 = strstr(p1, szSrc)))
    {
        memmove(p2 + nDst, p2 + nSrc, nStr - (p2 - szStr) - nSrc + 1);
        memcpy(p2, szDst, nDst);
        p1 = p2 + nDst;
        nStr += nDst - nSrc;
    }

    return true;
}
bool turntoregular(char * ultimatepageurl)
{   
    szReplace1(ultimatepageurl,".","\\.");
    szReplace1(ultimatepageurl,"[?]","[^ ]{1}");
    szReplace1(ultimatepageurl,"?","\\?");
    szReplace1(ultimatepageurl,"[$]","[0-9]+");
    szReplace1(ultimatepageurl,"[*]","[^ ]+");
    return true;
}
/*void SetUrlToMemecachedb()
{
    DBAccess *access = DBAccess::getInstance();
    access->setDir("/app/spider1/data");

    static vector<string> dirs;
    dirs.push_back("/app/spider1/data");
    char urldbname[16] = "";
    char strpageurl[] = "";

    string fileno = "";

    for (int i=2; i<=17; i++)
    {
        sprintf(urldbname, "url_task_%d", i);
        DBD *dbd = NULL,  *last_dbd = NULL;
        off_t last_offset = 0;
        string db_fileno; 
        int suffix = access->load(urldbname);
        while((dbd = access->gets(suffix, &last_dbd, &last_offset, db_fileno)) != NULL) {
            testMemcache1((char *)dbd->idxbuf,(char * )dbd->datbuf, dbd->datlen);
        }
        if (dbd != NULL) {
            dbd_free(dbd);
        } else {
            dbd_free(last_dbd);
        }
    }
}*/
void ReadUrlFromFile()
{
    map<string ,string > usernamemap;
    string filename ="/backup/spider1_for_ali_free/ali_free_username.txt";
    FILE *f  = fopen(filename.c_str(), "r");
    int i = 0;
    if (f)
    {
        char str[1024];
        while(fgets(str,1024,f))
        {
            int len = strlen(str) - 1;
            str[len] = 0;
            usernamemap.insert(make_pair(str, str));
            i++;
        }
    }
    fclose(f);
    
    filename ="needdelete";
    //filename ="/backup/spider1_for_ali_free/already.txt";
    f  = fopen(filename.c_str(), "r");
    if (f)
    {
        char str[1024];
        while(fgets(str,1024,f))
        {
            int len = strlen(str) - 1;
            str[len] = 0;
            string username = GetUserName(str);
            if (!username.empty())
            {
                if (usernamemap.find(username) == usernamemap.end())
                {
                    MemcacheDel(str);
                }
            }
        }
    }
    fclose(f);
}
string GetUserName(char * url)
{
    string ret = "";
    string urlfrees = "http://china.alibaba.com/company/detail/";
    string urlfreee = ".html";
    string urlcharges = "http://";
    string urlchargee = ".cn.alibaba.com/";
    string urltmp = url;
    int begin = 0,end = 0;
    if ((begin = urltmp.find(urlfrees))!= string ::npos)
    {
        if ((end= urltmp.find(urlfreee))!= string ::npos)
        {
            ret = urltmp.substr(begin + urlfrees.length(), end - begin - urlfrees.length());
        }

    }else if ((begin = urltmp.find(urlcharges))!= string ::npos)
    {
        if ((end = urltmp.find(urlchargee))!= string ::npos)
        {
            ret = urltmp.substr(begin + urlcharges.length(), end - begin - urlcharges.length());
        }
    }
    return ret;
}
void MemcacheDel(char * url)
{
    MemCacheClient * pClient = new MemCacheClient;
    pClient->AddServer("127.0.0.1:5766");
    MemCacheClient::MemRequest req;
    req.mKey  = url;
    //req.mData.WriteBytes(content,contentlen);
    pClient->Del(req);

    string urltmp = url; 
    if (req.mResult == MCERR_OK)
    {
        FILE * f;
        f= fopen("Delok.txt", "a+");
        fwrite( urltmp.c_str(), 1, urltmp.length(), f );
        fclose(f);
    }else if (req.mResult == MCERR_NOSERVER)
    {
        FILE * f;
        f= fopen("Delerror.txt", "a+");
        fwrite( urltmp.c_str(), 1, urltmp.length(), f );
        fclose(f);
    }
    delete pClient;
}
void testMemcache1(char *url,char * content, int contentlen)
{
    MemCacheClient * pClient = new MemCacheClient;
    pClient->AddServer("127.0.0.1:21202");
    MemCacheClient::MemRequest req;
    req.mKey  = url;
    req.mData.WriteBytes(content,contentlen);
    pClient->Set(req);

    string urltmp = url;
    urltmp.append("  ");
    urltmp.append(content);
    urltmp.append("\n");
    if (req.mResult == MCERR_OK)
    {
        FILE * f;
        f= fopen("insertok.txt", "a+");
        fwrite( urltmp.c_str(), 1, urltmp.length(), f );
        fclose(f);
    }else if (req.mResult == MCERR_NOSERVER)
    {
        FILE * f;
        f= fopen("inserterror.txt", "a+");
        fwrite( urltmp.c_str(), 1, urltmp.length(), f );
        fclose(f);
    }
}
void DeleteListUrl()
{

    FILE *f = fopen("/backup/spider1-1.0.0.1_back1/src/listurl.txt", "r");
    fseek(f,0,SEEK_END);
    int filelen = ftell(f);
    rewind(f);
    char *strlisturl= new char [filelen + 1];
    fread(strlisturl,1,filelen,f);
    strlisturl[filelen]=0;
    fclose(f);
    DBAccess *access = DBAccess::getInstance();
    // access->setDir("/oracle/spider1/data");
    access->setDir("/oracle/spider1test/data_back_20100318");
    static vector<string> dirs;
    //dirs.push_back("/oracle/spider1/data");
    dirs.push_back("/oracle/spider1test/data_back_20100318");

    char urldbname[16] = "";
    char *pageurl= strtrim(strlisturl, NULL);
    string pattern = checkPattern(pageurl);
    string fileno = "";
    //for (int i=0; i<=29; i++)
    for (int i=9; i<=9; i++)
    {
        sprintf(urldbname, "url_task_%d", i);
        DBD *dbd = NULL,  *last_dbd = NULL;
        printf("urldbname %s\n", urldbname);
        off_t last_offset = 0;
        string db_fileno; 
        int suffix = access->load(urldbname);
        while((dbd = access->gets(suffix, &last_dbd, &last_offset, db_fileno)) != NULL) {
            printf("list url: %s\n", (char *)dbd->idxbuf);
            if (is((char *)dbd->idxbuf, (char *)pattern.c_str()))
            {
                int ret = access->erase(suffix, (char *)dbd->idxbuf , fileno, NULL);
            }
        }   
        if (dbd != NULL) {
            dbd_free(dbd); 
        } else {    
            dbd_free(last_dbd);
        }       
    }

    if (strlisturl) delete [] strlisturl;
}
string  checkPattern(char *pattern) {
    string ret = "";
    char *pToken = NULL;
    char *tokbuff = NULL;

    char *tokbufffromurl = NULL;

    int len = strlen(pattern);
    if (len <=0 )
    {
        return ret;
    }

    char *patterntmp = new char[len + 10];
    patterntmp[0] = 0; 
    strncpy(patterntmp , pattern,len);
    patterntmp[len] = 0;
    pToken = strtok_r(patterntmp, "\r\n\t ",&tokbuff);
    while(pToken)
    {
        if (strlen(pToken)>=MAX_URL_LEN)
        {
            pToken = strtok_r(NULL, "\r\n\t ",&tokbuff);
            continue;
        }
        char pTokentmp[MAX_URL_LEN] = "";
        char tmpurl[MAX_URL_LEN] = "";
        char outurl[MAX_URL_LEN] = "";

        TransferRegEx(pToken,pTokentmp);

        MyURLencode(pTokentmp, tmpurl, MAX_URL_LEN- 1);
        pTokentmp[0] = '^';
        strcpy(pTokentmp + 1, tmpurl);
        strcat(pTokentmp, "$");

        tmpurl[0] = 0;

        ret.append(pTokentmp);
        ret.append("\r\n");
        pToken = strtok_r(NULL, "\r\n\t ",&tokbuff);
    }
    if (patterntmp) delete [] patterntmp;
    return ret;
}
void TransferRegEx(char * sOrigi, char * sNew)
{
    int nLen = strlen(sOrigi);
    int i = 0, j = 0;
    while ( i < nLen )
    {
        if (j >= MAX_URL_LEN - 10)
        {
            sNew[j] = 0;
            break;
        }
        if ( sOrigi[i] == '.'
            || sOrigi[i] == '*'
            || sOrigi[i] == '?'
            || sOrigi[i] == '|'
            || sOrigi[i] == '\\'
            || sOrigi[i] == '('
            || sOrigi[i] == ')'
            || sOrigi[i] == '{'
            || sOrigi[i] == '}'
            || sOrigi[i] == ']' )
        {
            sNew[j++] = '\\';
            sNew[j++] = sOrigi[i];
            i++;
            continue;
        }

        if ( sOrigi[i] == '[' )
        {
            if ( i + 2 < nLen && strncmp(sOrigi+i, "[*]", strlen("[*]")) == 0 )
            {
                sNew[j++] = '(';
                sNew[j++] = '.';
                sNew[j++] = '*';
                sNew[j++] = ')';
                i += 3;
                continue;
            }
            else if ( i + 2 < nLen && strncmp(sOrigi+i, "[?]", strlen("[?]")) == 0 )
            {
                sNew[j++] = '(';
                sNew[j++] = '.';
                sNew[j++] = '{';
                sNew[j++] = '1';
                sNew[j++] = '}';
                sNew[j++] = ')';
                i += 3;
                continue;
            }
            else if ( i + 2 < nLen && strncmp(sOrigi+i, "[$]", strlen("[$]")) == 0 )
            {
                sNew[j++] = '(';
                sNew[j++] = '[';
                sNew[j++] = '0';
                sNew[j++] = '-';
                sNew[j++] = '9';
                sNew[j++] = ']';
                sNew[j++] = '+';
                sNew[j++] = ')';
                i += 3;
                continue;
            }
            else
            {
                sNew[j++] = '\\';
                sNew[j++] = sOrigi[i];
                i++;
                continue;
            }
        }
        sNew[j++] = sOrigi[i++];
    }
}
/*sb4 error_proc(dvoid   *errhp,   sword   status)
{
    text   errbuf[512];
    sb4   errcode = 0;
    switch   (status)
    {
    case   OCI_SUCCESS:
        break;
    case   OCI_SUCCESS_WITH_INFO:
        printf("OCI   error:   OCI_SUCCESS_WITH_INFO\n");
        (void)OCIErrorGet((dvoid   *)errhp,(ub4)1,NULL,&errcode,errbuf,(ub4)sizeof(errbuf),OCI_HTYPE_ERROR);
        printf("ERROR NUM:%d\nERROR info:%s\n",errcode,errbuf);
        break;
    case   OCI_NEED_DATA:
        printf("OCI   error:   OCI_NEED_DATA\n");
        break;
    case   OCI_NO_DATA:
        printf("OCI   error:   OCI_NO_DATA\n");
        break;
    case   OCI_ERROR:
        (void)OCIErrorGet((dvoid   *)errhp,(ub4)1,NULL,&errcode,errbuf,(ub4)sizeof(errbuf),OCI_HTYPE_ERROR);
        printf("ERROR:q
NUM:%d\nERROR info:%s\n",errcode,errbuf);
        break;
    case   OCI_INVALID_HANDLE:
        printf("OCI   error:   OCI_INVALID_HANDLE\n");
        break;
    case   OCI_STILL_EXECUTING:
        printf("OCI   error:   OCI_STILL_EXECUTING\n");
        break;
    default:  
        break;
    }
    int i = 0;
    //while (reconnect(errcode) && (i++) <= icconfig->dbreconnectnum);
    return errcode;
}*/
/*void rm(const char * name)   
  {   
  DIR *dir;   
  struct dirent *read_dir;   
  struct stat st;   
  char buf[BUF_LEN];   

  if(lstat(name, &st) < 0)   
  {   
  fprintf(stderr, "Lstat Error!\n");   
  exit(1);   
  }   

  if(S_ISDIR(st.st_mode))   
  {   
  if((dir = opendir(name)) == NULL)   
  {   
  fprintf(stderr, "remove [%s] faild\n", name);   
  exit(1);   
  }   

  while((read_dir = readdir(dir)) != NULL)   
  {   
  if(strcmp(read_dir->d_name, ".") == 0 ||   
  strcmp(read_dir->d_name, "..") == 0)   
  continue;   
  sprintf(buf, "%s/%s", name, read_dir->d_name);   
  rm(buf);   
  }   
  }   
  printf("rm :%s\n", name);   
  if(remove(name) < 0)   
  {   
  fprintf(stderr, "remove [%s] faild\n", name);   
  }   
  }*/
void * thr_fn(void *arg) 
{ 
    //     pthread_detach(pthread_self()); 
    //    pthread_cancel(pthread_self()); 
    //sleep(100000);
    char *message = (char *) arg;
    printf("%s is over\n",message);
    return NULL;
}
void *testThreadExit()
{
    pthread_t thread1, thread2;
    char *message1 = "Thread 1";
    char *message2 = "Thread 2";
    int iret1, iret2;
    void *pth_join_ret1;//="thread1 has done!";
    void *pth_join_ret2;//="thread2 has done!";

    //    iret1 = pthread_create( &thread1, NULL, print_message_function, (void*)"thread one_here");
    iret1 = pthread_create( &thread1, NULL, print_message_function, (void*)message1);
    iret2 = pthread_create( &thread2, NULL, print_message_function, (void*) message2);

    pthread_join( thread1, &pth_join_ret1);
    pthread_join( thread2, &pth_join_ret2);
    printf("Thread 1 returns: %d\n",iret1);
    printf("Thread 2 returns: %d\n",iret2);
    printf("pthread_join 1 returns: %s\n",(char *)pth_join_ret1);
    printf("pthread_join 2 returns: %s\n",(char *)pth_join_ret2);
    exit(0);
}
void *print_message_function( void *ptr )
{
    char messagetmp[1024] = "";
    string messagetmp1;
    char *message;
    message = (char *) ptr;
    messagetmp1 = message;
    sprintf(messagetmp,"\t PID: %ld \n", pthread_self());
    messagetmp1.append(messagetmp);
    printf("in thread %s",messagetmp1.c_str());
    pthread_exit ((void *) messagetmp1.c_str());   
}
void testLD()
{
    /*FILE *f = NULL;
      f = fopen("nextpage_134_1.txt" , "rb");
      FILE *f1 = fopen("nextpageout.txt" , "w+");
      if (f && f1)
      {
      char str[1024];
      char a[]={0x09,0};
      while(fgets(str,1024,f))
      {
      str[strlen(str) - 1] = 0;
      string strurl = str; 
      char url[1024] = "";
      char urlnextpage[1024] = "";
      char * pos = NULL;
      if (pos = strstr(str,a))
      {
      strncpy(url , str , pos - str);
      strncpy(urlnextpage , pos+1 , strlen(str) - (pos - str) -1);
      CDistance m_Distance;
      double distance = m_Distance.LD(url,urlnextpage);
      char strdistanc[10] = "";
      sprintf(strdistanc, "%f" , distance);
      string writestr =  strdistanc;
      writestr.append(a);
      writestr.append(url);
      writestr.append(a);
      writestr.append(urlnextpage);
      writestr.append("\n");
      printf("%s",writestr.c_str());
      fwrite(writestr.c_str(), writestr.length(), 1, f1);  
      }
      }
      }
      fclose(f);
      fclose(f1);
      */
    /*CDistance m_Distance;
      char url[] = "http://www.cbinews.com/htmlnews/2010-06-26/128464.htm";
      char urlnextpage[] = "http://www.cbinews.com/inc/showcontent.jsp?articleid=128464&currentPage=2";
      double distance = m_Distance.LD(url , urlnextpage);
      printf("%f\n",distance);*/
}
bool IsReadFetchedEnd()
{
    int64_t posfilepos = 0;
    int64_t queuqfilepos = 0;
    int64_t pos = 0;
    int ret = 0;  
    char filename[128]= "";
    FSBigFile *bigfile = NULL;
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    vector<string> dirs; 
    dirs.push_back(icconfig->dbpath);

    sprintf(filename, "%s/%s", icconfig->dbpath, FETCHED_FIFO_QUEUE_POS_FILE);
    FILE *f  = fopen(filename, "r");
    if (f) {
        char key[32] = "";
        ret = fread(key, 1, 32, f);
        if (ret == 0) {
            fclose(f);
            //errorlog("in IsReadFetchedEnd can not read position in readfetch for FSBigFile %s %d %d\n", FETCHED_FIFO_QUEUE_FILE, posfilepos);
        } else {
            sscanf(key, "%llu", &posfilepos);
            fclose(f);
        }
    } else {
        //errorlog("can not open position file in IsReadFetchedEnd for FSBigFile %s\n", FETCHED_FIFO_QUEUE_FILE);
        goto end;
    }

    sprintf(filename, "%s/%s", icconfig->dbpath, FETCHED_FIFO_QUEUE_FILE);

    if (access(filename, R_OK) == -1) {
        //errorlog("can not open fetched file in readFetched for LocalDbManager %s\n", FETCHED_FIFO_QUEUE_FILE);
        goto end;
    }
    bigfile  = new FSBigFile(filename, &dirs, 'r');

    if (bigfile->seek(pos, SEEK_END) != 0) {
        //errorlog("can not set position for FSBigFile %s %d\n", FETCHED_FIFO_QUEUE_FILE, pos);
        delete bigfile;
        goto end;
    }
    queuqfilepos = bigfile->tell();
    delete bigfile;
end:
    if (posfilepos >=queuqfilepos )
    {
        return true;
    }
    return false;
}
static int callback(void * notused , int argc, char ** argv, char **azColName)
{
    int i;
    static int calltimes = 0;
    calltimes++;
    for(i=0;i<argc;i++)
    {
        printf("%s = %s ",azColName[i], argv[i]?argv[i]:"NULL");
    }
    printf("\n");
    return 0;

}
class IntLesser
{
public:
    bool   operator() (int a,   int b)
    {
        return a  <  b;
    }
};
class IntLesser1
{
public:
    bool   operator() (int a,   int b)
    {
        return a  >  b;
    }
};
/*void testMyPriorityQueue()
  {

  PriorityQueue<int , vector<int >, IntLesser> tasks;
  int i = MAX_PUSH;
  srand(MAX_PUSH);
  int timer_second_begin; 
  int timer_micro_begin; 
  getNow1(&timer_second_begin, &timer_micro_begin);
  while(i>=0)
  {
  int ran_num=rand();
//printf("%d ", ran_num);
tasks.mypush(ran_num);
i--;
}
int timer_second_end; 
int timer_micro_end; 
getNow1(&timer_second_end, &timer_micro_end); 
int timer_second = timer_second_end - timer_second_begin;
int timer_micro= timer_micro_end - timer_micro_begin;
ulonglong micro = (1000000 * timer_second + timer_micro);
printf("my priority_queue timer %s:%d %d %d %d %d %d %d %llu\n", __FILE__, __LINE__, timer_second_begin, timer_micro_begin, timer_second_end, timer_micro_end, timer_second, timer_micro, micro);
printf("queue size %d\n",tasks.size());
int size = tasks.size();
for(int j = 0 ; j<size; j++)
{
printf("%d ", tasks.top());
tasks.mypop();
}
printf("\n"); 



getNow1(&timer_second_begin, &timer_micro_begin);
priority_queue<int, vector<int>, IntLesser1> tasks1;

i = MAX_PUSH;
while(i>=0)
{
int ran_num=rand();
//printf("%d ", ran_num);
tasks1.push(ran_num);
i--;
}
getNow1(&timer_second_end, &timer_micro_end); 
timer_second = timer_second_end - timer_second_begin;
timer_micro= timer_micro_end - timer_micro_begin;
micro = (1000000 * timer_second + timer_micro);
printf("STL priority_queue  timer %s:%d %d %d %d %d %d %d %llu\n", __FILE__, __LINE__, timer_second_begin, timer_micro_begin, timer_second_end, timer_micro_end, timer_second, timer_micro, micro);

printf("queue size %d\n",tasks1.size());
size = tasks1.size();
for(int j = 0 ; j< size;j++)
{
printf("%d ", tasks1.top());
tasks1.pop();
}
printf("\n"); 
}*/
void teststl()
{
    time_t start, finish;
    int num = 0;
    list<int> coll;

    start = clock();
    for(int i=0;i<10000;++i){
        coll.push_back(i);
        num += coll.size();
    }
    finish = clock();
    cout<<finish - start<<"   num:"<<num<<endl;

    coll.clear();
    start = clock();
    for(int  i=0;i<10000;++i){
        coll.push_back(i);
    }
    finish = clock();
    cout<<finish - start<<endl;
}
void mylog_infotest( const char *stringformat, ...) {
    va_list va;
    va_start(va, stringformat);
    std::string buf = myvalistformtest(stringformat, va);
    va_end(va);
    printf("%s\n",buf.c_str());
}
std::string myvalistformtest(const char* format, va_list args) {
    size_t size = 1024;
    char* buffer = new char[size];

    while (1) {
        //int n = vsprintf(buffer,size,  format, args);
        int n = VSNPRINTF(buffer, size, format, args);

        if ((n > -1) && (static_cast<size_t>(n) < size)) {
            std::string s(buffer);
            delete [] buffer;
            return s;
        }

        size = (n > -1) ?
            n + 1 :   // ISO/IEC 9899:1999
            size * 2; // twice the old size

        delete [] buffer;
        buffer = new char[size];
    } 
}
ulonglong GetMaxID(unsigned long pcid)
{
    static ulonglong last_millisecond = 0;
    static long last_id = 0;
    ulonglong millisecond = 0;
    ulonglong second_tmp = 0;
    ulonglong ret_tmp = 0;

    struct timeval tv;

    pthread_mutex_lock(&mux);

start:
    if(gettimeofday(&tv,NULL) == -1)
    {
        printf("gettimeofday error!\n");
        pthread_mutex_unlock(&mux);

        return 0;
    }

    second_tmp = tv.tv_sec;
    millisecond = second_tmp*1000 + tv.tv_usec/1000;

    if(millisecond > last_millisecond)
    {
        last_millisecond = millisecond;
        last_id = 0;
        ret_tmp = (millisecond << 23 | (pcid << 12));
        pthread_mutex_unlock(&mux);

        return ret_tmp;
    }
    else if(millisecond == last_millisecond)
    {
        if(last_id > 4095)
        {
            sleep(1);
            goto start;
        }
        else
        {
            last_id++;
            ret_tmp = (millisecond << 23 | (pcid << 12)) | last_id;
            pthread_mutex_unlock(&mux);

            return ret_tmp; 
        }
    }
    else
    {
        sleep(1);
        goto start;
    }
}
void testonfind3()
{
    struct uri page_uri;
    char *url = "http://zhixing.court.gov.cn/search/?type=public&PartyName=%C4%DA%C3%C9&CardNumber=&SelectCourt%5B0%5D=B&Submit=%B2%E9+%D1%AF";
    char *o = (char *)malloc(1024 * 1024 * 1024);

    FILE *f= fopen("test1.html", "r+");
    int ret = fread(o, 1, 1024 * 1024 * 1024, f);
    o[ret] = 0;                                                                                                    
    uri_parse_string(url, &page_uri);
    void *t = NULL;
    hlink_detect_string(o, &page_uri, onfind, t);
    uri_destroy(&page_uri);
    fclose(f);
}
char *strtrim1(char *str, const char *trim)
{           
    trim = " \t\n\r\09";
    return str;
}
string string_trim(string &str) { 
    if(str.empty()) {
        return str;
    }

    int i=0;
    for( i=0;i<str.length();i++) { 
        char c=str[i];
        if(c!=' '&&c!='\r'&&c!='\n'&&c!='\t') {
            break;
        }
    }

    if(i==str.length()) {
        return str;
    }

    int e=0;
    for( e=str.length()-1;e>=0;e--) { 
        char c=str[e];
        if(c!=' '&&c!='\r'&&c!='\n'&&c!='\t') {
            break;
        }
    }
    string ret = str.substr(i);
    return ret;
}



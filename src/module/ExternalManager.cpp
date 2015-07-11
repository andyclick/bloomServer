#include "InfoCrawler.h"
#include "ExternalManager.h"
#include "util.h"
#include <list>
#include "utilcpp.h"

int ExternalManager::nthreads = 10;
int ExternalManager::iget = 0;
int ExternalManager::iput = 0; 
Thread *ExternalManager::tptr = NULL;
int ExternalManager::clifd[EXTERNAL_MAXNCLI];

pthread_mutex_t ExternalManager::clifd_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ExternalManager::clifd_cond = PTHREAD_COND_INITIALIZER;
pthread_rwlock_t ExternalManager::rwlock_socket_error = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t ExternalManager::rwlock_html_from_outer = PTHREAD_RWLOCK_INITIALIZER;

ExternalManager::ExternalManager(LogGlobalCtrl * pLogGlobalCtrl) {
    m_pLogGlobalCtrl = pLogGlobalCtrl;
}

ExternalManager::~ExternalManager() {

}

int ExternalManager::start() {
    int ret = Manager::start();
    startServeThread();
    mylog_info(m_pLogGlobalCtrl->infolog, "External Server thread is start - %s:%s:%d", INFO_LOG_SUFFIX);
    return ret;
}

int ExternalManager::stop() {
    int ret = Manager::stop();

    while(!canstoped()) {
        my_sleep(500000); //0.5 seconds
    }
    return ret;
}


int ExternalManager::startServeThread() {

    pthread_t *tid = (pthread_t*)malloc(1 * sizeof(pthread_t));
    if (tid == NULL) {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "malloc error pthread_t for ExternalManager - %s:%s:%d:%d", ERROR_LOG_SUFFIX);
    }       

    if(pthread_create(tid, NULL, startServe, this)) {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "create threads error startServe for ExternalManager - %s:%s:%d:%d", ERROR_LOG_SUFFIX);
    }

    pthread_detach(*tid);
    free(tid);

    return 0;
}



void *ExternalManager::startServe(void *manager) {

    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    ICCONFIG *icconfig = infocrawler->getConf();
    ExternalManager *externalManager = static_cast<ExternalManager *>(manager);

    externalManager->serve(icconfig->externalserverport);

    externalManager->stoped();

    mylog_info(externalManager->m_pLogGlobalCtrl->infolog, "External Server thread is over - %s:%s:%d", INFO_LOG_SUFFIX);
}


void ExternalManager::dump() {}
void ExternalManager::retrieve() {}

void ExternalManager::serve(int portnumber) {
    struct sockaddr_in sin, c_in;
	int s, cs, clen, on = 1;
	fd_set R;
	struct timeval to;
	int x;

	if (portnumber < 1) {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "Illegal port number: %d for ExternalManager - %s:%s:%d", portnumber,INFO_LOG_SUFFIX);
	}

	memset(&sin, '\0', sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(portnumber);
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "socket for ExternalManager - %s:%s:%d:%d", ERROR_LOG_SUFFIX);
	}
	if ((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &on,
		    sizeof(on))) < 0) {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "setsockopt (SO_REUSEADDR = on) for ExternalManager - %s:%s:%d:%d", ERROR_LOG_SUFFIX);
	}
	if (bind(s, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
		(void) close(s);
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "bind for ExternalManager - %s:%s:%d:%d", ERROR_LOG_SUFFIX);
	}
	if (listen(s, 5) < 0) {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "listen for ExternalManager - %s:%s:%d:%d", ERROR_LOG_SUFFIX);
	}
	clen = sizeof(c_in);
    
    mylog_info(m_pLogGlobalCtrl->infolog, "Ready to serve information for ExternalServer... - %s:%s:%d", INFO_LOG_SUFFIX);

    tptr = (Thread*)calloc(nthreads, sizeof(Thread));
    iget = iput = 0;
    for(thread_i = 0; thread_i < nthreads; thread_i++) {
        thread_make(thread_i);
    }
    for(thread_i = 0; thread_i < nthreads; thread_i++) {
        thread_detach(thread_i);
    }
	while (running()) {
		to.tv_sec = 10;	// Lunix modifies 'to' on return 
		to.tv_usec = 0;
		FD_ZERO(&R);
		FD_SET(s, &R);
		x = select(s + 1, &R, 0, 0, &to);
		if (x <= 0) {
            continue;
        }

		if ((cs = accept(s, (struct sockaddr *) &c_in, (socklen_t*)&clen)) < 0) {
			if (errno == EINTR)
				continue;
            mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "accept in serveClient for ExternalManager - %s:%s:%d:%d", ERROR_LOG_SUFFIX);
			exit(1);
		}
		//setsocket_linger(cs, 30);	// 30 sec linger time 
        pthread_mutex_lock(&clifd_mutex);
        int tmp = iput + 1;
        if (tmp == EXTERNAL_MAXNCLI) {
            tmp = 0;
        }
        if (tmp == iget) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "exceed max connections, this socket will be close - %s:%s:%d", INFO_LOG_SUFFIX);
            pthread_mutex_unlock(&clifd_mutex);
            close(cs);
            continue;
        }

        clifd[iput] = cs;
        if (++iput == EXTERNAL_MAXNCLI) {
            iput = 0;
        }

        pthread_cond_signal(&clifd_cond);
        pthread_mutex_unlock(&clifd_mutex);
	}
    for(thread_i = 0; thread_i < nthreads; thread_i++) {
        pthread_cond_signal(&clifd_cond);
    }
    close(s);
    free(tptr);
    tptr = NULL;
}

void ExternalManager::thread_make(int i) {
    pthread_create(&tptr[i].thread_tid, NULL, &thread_main, (void *)this);
    return;
}
void ExternalManager::thread_detach(int i) {
    pthread_detach(tptr[i].thread_tid);
    return;
}
void ExternalManager::thread_join(int i) {
    pthread_join(tptr[i].thread_tid, NULL);
    return;
}
void ExternalManager::thread_exit(int i) {
    //pthread_exit(&tptr[i].thread_tid);
    pthread_cancel(tptr[i].thread_tid);
    return;
}

void *ExternalManager::thread_main(void *arg) {
    int connfd;

    for(;;) {
        pthread_mutex_lock(&clifd_mutex);
        while(iget == iput) {
            pthread_cond_wait(&clifd_cond, &clifd_mutex);
            if (!((ExternalManager *)arg)->running())
            {
                pthread_mutex_unlock(&clifd_mutex);
                return NULL;
            }
        }
        connfd = clifd[iget]; // connected socket to service 
        if (++iget == EXTERNAL_MAXNCLI)
            iget = 0;
        pthread_mutex_unlock(&clifd_mutex);
        ((ExternalManager *)arg)->serveClient(connfd);
    }
    return NULL;
}

int ExternalManager::serveClient(int fd) {
    MyConnection connection(fd);
    connection.Timeout(EXTERNAL_DEFAULT_TIMEOUT);
    
    int n = 16;
    char buf[16 +1] ;buf[0] = 0;

    int ret = connection.Read(buf, n);
    buf[n] = 0;

    if (ret != n) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "read socket error %d for ExternalManager - %s:%s:%d", ret,INFO_LOG_SUFFIX);
        return -1;
    }

    if(strncmp(buf, "20090910", 8) != 0) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "read head error for ExternalManager - %s:%s:%d", INFO_LOG_SUFFIX);
        return -1;
    }

    int command = atoi(buf + 8);
    mylog_info(m_pLogGlobalCtrl->infolog, "send socket  read %s  - %s:%s:%d",buf ,INFO_LOG_SUFFIX);
    if (command == REQUEST_COMMAND_SEND_HTML)
    {
#ifdef HTMLMEMCACHEDB
        SendFile_MemcachedbDb(connection);
#else
        SendFile_db(connection);
#endif    
    } else if (command == REQUEST_COMMAND_RECEIVE_URL) {
        ReceiveClineUrl(connection);            
    } else if (command == REQUEST_COMMAND_GET_HTML) {
        get_html(connection);
    }

    if (connection.IsOpen()) {
        connection.Close();
    }
    
    return 0;
}

void ExternalManager::get_real_html(char *buf, vector<html_des> &htmls) {
    char *begin = NULL, *end = NULL;
    char tmpbuf[1024] = "";
    int taglen = 0;
    char *middle_buf = buf;
    char *tmp = NULL;

    while ((tmp = strstr(middle_buf, "##!!!##")) != NULL) {
        html_des html;
        char *finalpos = strstr(tmp, "\n\n\n");
        if (!finalpos) {
            break;
        }
        
        finalpos += 3;

        begin = strstr(tmp, "url:");
        taglen = strlen("url:");
        if (begin && begin < finalpos) {
            end = strstr(begin, "\n");
            if (end && end <=finalpos) {
                int len = end - (begin + taglen);
                html.url.append(begin + taglen, len);
                html.url = stringstrim(html.url);
            }
        }

        begin = strstr(tmp, "charset:");
        taglen = strlen("charset:");
        if (begin && begin < finalpos) {
            end = strstr(begin, "\n");
            if (end && end <=finalpos) {
                int len = end - (begin + taglen);
                html.charset.append(begin + taglen, len);
                html.charset = stringstrim(html.charset);
            }
        }

        begin = strstr(tmp, "length:");
        taglen = strlen("length:");
        if (begin && begin < finalpos) {
            end = strstr(begin, "\n");
            if (end && end <=finalpos) {
                int len = end - (begin + taglen);
                char len_str[32] = "";
                strncpy(len_str, begin + taglen, len);
                html.length = atoi(strtrim(len_str, NULL));
                int htmllen = strlen(finalpos);
                if (html.length > htmllen) {
                    mylog_error(m_pLogGlobalCtrl->errorlog, " html len %d is greater than htmllen %d - %s:%s:%d:%d", html.length, htmllen, ERROR_LOG_SUFFIX);
                    html.length = htmllen;
                }
            }
        }

        int back_bytes = (html.length > 100)?(html.length - 100):0;
        begin = strstr(finalpos + back_bytes, "site_name:");
        taglen = strlen("site_name:");
        if (begin && begin < (finalpos + html.length)) {
            end = strstr(begin, "\n");
            if (end && end <= (finalpos + html.length)) {
                int len = end - (begin + taglen);
                html.sitename.append(begin + taglen, len);
                html.sitename = stringstrim(html.sitename);
            }
        }

        begin = strstr(finalpos + back_bytes, "site_id:");
        taglen = strlen("site_id:");
        if (begin && begin < finalpos + html.length) {
            end = strstr(begin, "\n");
            if (end && end <= (finalpos + html.length)) {
                int len = end - (begin + taglen);
                string site_id;
                site_id.append(begin + taglen, len);
                site_id = stringstrim(site_id);
                html.task_id = atoi(site_id.c_str()); 
            }
        }

        html.htmlcontent.append(finalpos, html.length);
        htmls.push_back(html);

        middle_buf =  finalpos + html.length;
    }
}

int ExternalManager::get_html(MyConnection &connection)
{
	char tmp[9] = ""; 	
	int ret = connection.Read(tmp, 8);
    if (ret != 8) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "read html len error %d/8 - %s:%s:%d:%d", ret ,ERROR_LOG_SUFFIX);
        return 0;
    }
    
    int ack = 1;
    if (get_html_from_outer_size() > 400) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "html from outer > 400 - %s:%s:%d:%d", ERROR_LOG_SUFFIX);
        ack = 0;
    }

    ret = connection.Write((char *)&ack, 4);
    if (ret != 4) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "read html len error %d/4 - %s:%s:%d:%d", ret ,ERROR_LOG_SUFFIX);
        return 0;
    }

    if (ack == 0) {
        return 0;
    }

    int html_len = atoi(tmp);

    char *html = (char *)malloc(html_len + 1);
	ret = connection.Read(html, html_len);
    if (ret != html_len) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "read html error %d/%d - %s:%s:%d:%d", ret , html_len, ERROR_LOG_SUFFIX);
        free(html);
        return 0;
    }
    html[html_len] = 0;

    char *raw_html = NULL;
    size_t raw_html_len = 0;
    inf(html, html_len,  &raw_html, &raw_html_len);
    raw_html[raw_html_len] = 0;
    free(html);

    vector<html_des> htmls;
    get_real_html(raw_html, htmls);

    free(raw_html);

    mylog_info(m_pLogGlobalCtrl->infolog, "get html len = %d rawlen = %d size = %d outer = %d - %s:%s:%d:%d", html_len, raw_html_len, htmls.size(), get_html_from_outer_size(), ERROR_LOG_SUFFIX);


    //find max len html
    int idx = 0;
    int maxlen = 0;
    if (htmls.size() > 0) {
        for(int i = 0; i < htmls.size(); i++) {
            if (htmls[i].length > maxlen) {
                idx = i;
                maxlen = htmls[i].length;
            }
        }

        TaskScheduleManager *taskScheduleManager = InfoCrawler::getInstance()->getTaskScheduleManager();
        int taskId = taskScheduleManager->getTaskIdByUrl((char *)htmls[idx].url.c_str());
        if (taskId > 0) {
            htmls[idx].task_id = taskId;
            insert_html_from_outer(htmls[idx]);
        }
        
        //get urls;
        /*InfoCrawler *infocrawler = InfoCrawler::getInstance();
        Task *task = infocrawler->getTaskScheduleManager()->getTask(1);

        bool taskotherexists = false;
        TaskOtherInfo *taskother = infocrawler->getTaskScheduleManager()->getTaskOtherInfo(task->id, &taskotherexists, true);

        infocrawler->getUrlAnalyseManager()->insertUrl((char *)htmls[idx].url.c_str(), taskother, task, taskother?taskother->taskbatch:0);
        */
    }
}

int ExternalManager::ReceiveClineUrl(MyConnection connection)
{
    char *pToken = NULL , *tokbuff = NULL;
    char *pToken1 = NULL , *tokbuff1 = NULL;
    int n=8;
    char lenbuf[10];lenbuf[0] = 0;
    
    int ret = connection.Read(lenbuf, n);
    lenbuf[n] = 0;
    
    long clen = atol(lenbuf);
    char * RecvBuf = new char[clen + 2];
    ret = connection.Read(RecvBuf , clen);
    RecvBuf[ret] = 0;
    pToken = strtok_r(RecvBuf, (char * )0x08 , &tokbuff);
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    while(pToken)
    {
        
        pToken1 = strtok_r(pToken, (char * )0x07 , &tokbuff1);
        int i = 0;
        UrlNode *newnode  = new UrlNode;
        while(pToken1)
        {
            switch (i++)
            {
            case RECEIVE_URL:
                    newnode->copyurl(pToken1);
                    break;
            case RECEIVE_URL_TYPE: 
                    newnode->type = infocrawler->getUrlAnalyseManager()->getType(atoi(pToken1)); 
                    break;
            case RECEIVE_URL_BBS_ID:
                    newnode->bbsid= atoi (pToken1);
                    break;
            case RECEIVE_URL_TASK_ID:
                    newnode->taskid = atoi (pToken1);
                    break;
            }
            pToken1 = strtok_r(NULL, (char *)0x07, &tokbuff1);
        }
        if (newnode->type == URL_TYPE_BBS_FINALPAGE)
        {   
            newnode->layerid = MAX_LAYER_ID;
        }else if (newnode->type = URL_TYPE_LISTPAGE)
        {
            newnode->layerid = MAX_LAYER_ID - 1;
        }
        infocrawler->getUrlAnalyseManager()->insertUrl(newnode);
        pToken = strtok_r(NULL, (char * )0x08, &tokbuff);
    }
    if(RecvBuf) delete []RecvBuf;
    return 0;
}
#ifdef HTMLMEMCACHEDB
int ExternalManager::SendFile_MemcachedbDb(Connection connection)
{
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    int ret = -1;
    
    int n = 16;
    char buf[16+1] ;buf[0] = 0;

    ret = connection.Read(buf, n);
    buf[n] = 0;

    int num = atoi(buf);
    int requiretaskid = atoi(buf + 8);


    char sendhead[8+1] = {0};
    sprintf(sendhead , "20090910");
    sendhead[8]=0;    
    ret = connection.Write(sendhead,8);
    mylog_info(m_pLogGlobalCtrl->infolog, " write head %s 8 %d in serveClient for ExternalManager - %s:%s:%d", sendhead,ret,INFO_LOG_SUFFIX);
    
   int errornum = 0;
   for (errornum = num ; errornum >0 ;errornum --)
   {
       dbsenderror *dbderror = GetSendError();
       if (dbderror)
       {
            ulonglong id = 0;
            int taskid = 0;
            char recordname[32] ;recordname[0] = 0;
           sscanf(dbderror->key,"%llu/%d", &id, &taskid);
           int len = strlen (dbderror->key);
           MemCacheClient::MemRequest req;
           MemCacheClient *mc = InfoCrawler::getInstance()->getHtmlMcLocalThread();
           req.mKey  =  recordname;
           //____BEGINTIMER
           mc->Get(req);
          // ____ENDTIMER

           int re_connect = 1;
           while(req.mResult == MCERR_NOSERVER)
           {
                if (!(running()))
                {
                    return MCERR_NOTFOUND;
                }else
                {
                    req.mData.Deallocate();
                    mc->Get(req);
                }
                mylog_error(m_pLogGlobalCtrl->errorlog, "CAN NTO CONNECT TO MEMCACHEDB key %s - %s:%s:%d", recordname,INFO_LOG_SUFFIX);
                my_sleep(500000); //0.5 seconds
                if (re_connect++ >= MEMCACHEDB_RECONNECT_NUM )
                {
                    mylog_error(m_pLogGlobalCtrl->errorlog, "CAN NTO RECONNECT TO MEMCACHEDB key %s int SendFile_MemcachedbDb GET re_connect num %d - %s:%s:%d", recordname, re_connect,INFO_LOG_SUFFIX);
                    req.mResult = MCERR_NOTFOUND;
                    break;
                }
           }
           if(req.mResult == MCERR_OK){
               if (req.mData.GetReadSize()) {
                   char datalen[9] ;datalen[0]= 0;
                   sprintf(datalen, "%8d",req.mData.GetReadSize());
                   datalen[8] = 0;
                   ret = connection.Write(datalen, 8);
                   if (ret != 8)
                   {
                       mylog_error(m_pLogGlobalCtrl->errorlog, "write length %d %d  %s - %s:%s:%d", ret, req.mData.GetReadSize(), datalen,INFO_LOG_SUFFIX);
                       SaveSendError(dbderror);
                       continue;
                   }
                   mylog_info(m_pLogGlobalCtrl->infolog, "write length %d %d - %s:%s:%d", ret, req.mData.GetReadSize(),INFO_LOG_SUFFIX);
                   ret = connection.Write((char *)req.mData.GetReadBuffer(), req.mData.GetReadSize());
                   if (ret != req.mData.GetReadSize())
                   {
                       mylog_error(m_pLogGlobalCtrl->errorlog, "write length %d %d  %s  - %s:%s:%d", ret, req.mData.GetReadSize(), datalen,INFO_LOG_SUFFIX);
                       SaveSendError(dbderror);
                       continue;
                   }
                   mylog_info(m_pLogGlobalCtrl->infolog, "write length %d %d - %s:%s:%d", ret, req.mData.GetReadSize(),INFO_LOG_SUFFIX);
                   infocrawler->getLocalDbManager()->saveAlreadyRead(id, taskid);
               }
           }
           delete dbderror;
       }else
       {
           break;
           mylog_error(m_pLogGlobalCtrl->errorlog, "send socket GetSendError is NULL - %s:%s:%d", INFO_LOG_SUFFIX);
       }
   }
    num = errornum;
    list<char *> fetched;
    infocrawler->getLocalDbManager()->readFetched(fetched, num);
    list<char *>::iterator iter;
    for(iter = fetched.begin(); iter != fetched.end(); ++iter) {
        ulonglong id = 0;
        int taskid = 0;
        if (*iter) {
            sscanf(*iter, "%llu/%d", &id, &taskid);
            free(*iter);
        } else {
            continue;
        }

        if (requiretaskid > 0 && requiretaskid != taskid) {
            continue;
        }
        char recordname[32] ;recordname[0] = 0;
        infocrawler->getLocalDbManager()->getMemcachedbKeyName(id, taskid, recordname);

        int len = strlen (recordname);
        MemCacheClient::MemRequest req;
        MemCacheClient *mc = InfoCrawler::getInstance()->getHtmlMcLocalThread();
        req.mKey  =  recordname;
        ____BEGINTIMER 
        mc->Get(req);
        ____ENDTIMER

        int re_connect = 1;
        while(req.mResult == MCERR_NOSERVER)
        {       
            if (!(running()))
            {       
                return MCERR_NOTFOUND;
            }else   
            {       
                req.mData.Deallocate();
                mc->Get(req);
            }       
            mylog_error(m_pLogGlobalCtrl->errorlog, "CAN NTO CONNECT TO MEMCACHEDB key %s GET - %s:%s:%d", recordname,INFO_LOG_SUFFIX);
            my_sleep(500000); //0.5 seconds
            if (re_connect++ >= MEMCACHEDB_RECONNECT_NUM )
            {
                mylog_error(m_pLogGlobalCtrl->errorlog, "CAN NTO RECONNECT TO MEMCACHEDB key %s GET re_connect num %d - %s:%s:%d", recordname, re_connect,INFO_LOG_SUFFIX);
                req.mResult = MCERR_NOTFOUND;
                break;
            }
        }

        if (req.mResult == MCERR_OK){
            if (req.mData.GetReadSize()) {
                char datalen[9] ;datalen[0] = 0;
                sprintf(datalen, "%8d",req.mData.GetReadSize());
                datalen[8] = 0;
                ret = connection.Write(datalen, 8);
                if (ret != 8)
                {
                    mylog_error(m_pLogGlobalCtrl->errorlog, "ERROR write length %d %d  %s  - %s:%s:%d",ret, req.mData.GetReadSize(), datalen,INFO_LOG_SUFFIX);
                    dbsenderror * dberror = new dbsenderror;
                    sprintf(dberror->key,"%llu/%d",id, taskid);            
                    SaveSendError(dberror);
                    continue;
                }
                mylog_info(m_pLogGlobalCtrl->infolog, "write length %d %d - %s:%s:%d",ret, req.mData.GetReadSize() ,INFO_LOG_SUFFIX);
                ret = connection.Write((char *)req.mData.GetReadBuffer(), req.mData.GetReadSize());
                if (ret != req.mData.GetReadSize())
                {
                    mylog_error(m_pLogGlobalCtrl->errorlog, "ERROR write length %d %d  %s  - %s:%s:%d",ret, req.mData.GetReadSize(), datalen,INFO_LOG_SUFFIX);
                    dbsenderror *dberror = new dbsenderror;
                    sprintf(dberror->key,"%llu/%d",id, taskid);            
                    SaveSendError(dberror);
                    continue;
                }
                mylog_info(m_pLogGlobalCtrl->infolog, "write content %d %d - %s:%s:%d",ret, req.mData.GetReadSize() ,INFO_LOG_SUFFIX);
                infocrawler->getLocalDbManager()->saveAlreadyRead(id,taskid);
            }
            //mc_res_free(req, res);
        } else {
            //mc_res_free(req, res);
            mylog_info(m_pLogGlobalCtrl->infolog, "can not get data id = %llu taskid = %d - %s:%s:%d",id, taskid ,INFO_LOG_SUFFIX);
            continue;
        }
    }

    char endbuf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    ret = connection.Write(endbuf, 8);
    mylog_info(m_pLogGlobalCtrl->infolog, " write endbuf %d %d - %s:%s:%d",8 , ret,INFO_LOG_SUFFIX);
    char ack[1] = "";
    ret = connection.Read(ack, 1);
    mylog_info(m_pLogGlobalCtrl->infolog, " read ack %d %d - %s:%s:%d",1 , ret,INFO_LOG_SUFFIX);
    return 0;
}
#else
int ExternalManager::SendFile_db(MyConnection connection)
{
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    char socketsend[1024];socketsend[0] = 0;
    int ret = -1;
    
    int n = 16;
    char buf[16+1] ;buf[0] = 0;

    ret = connection.Read(buf, n);
    buf[n] = 0;
    sprintf(socketsend,"read number %s %d %d \n", buf, n, ret);

    int num = atoi(buf);
    int requiretaskid = atoi(buf + 8);


    char sendhead[8+1] = {0};
    sprintf(sendhead , "20090910");
    sendhead[8]=0;    
    ret = connection.Write(sendhead,8);
    //Debug(0, 1, ("Debug: write head %s 8 %d in serveClient for ExternalManager\n", sendhead,ret));
    socketsend[0]=0;
    //sprintf(socketsend,"head %s 8 %d \n",sendhead,ret);
    
   int errornum = 0;
   for (errornum = num ; errornum >0 ;errornum --)
   {
       dbsenderror *dbderror = GetSendError();
       if (dbderror && dbderror->dbd)
       {
           if (dbderror->dbd->datlen_u) {
               ulonglong id = 0;
               int taskid = 0;
               socketsend[0]=0;
               char datalen[9] ;datalen[0] = 0;
               sprintf(datalen, "%8d",dbderror->dbd->datlen_u);
               datalen[8] = 0;
               ret = connection.Write(datalen, 8);
               if (ret != 8)
               {
                   mylog_error(m_pLogGlobalCtrl->errorlog, "write head length %d %d  %s - %s:%s:%d", ret, dbderror->dbd->datlen_u, datalen,INFO_LOG_SUFFIX);
                   SaveSendError(dbderror);
                   continue;
               }
               //Debug(0, 1, ("Debug: write length %d %d in serveClient for ExternalManager\n", ret, dbderror->dbd->datlen_u));
               //sprintf(socketsend,"write length %d %d\n",ret, dbderror->dbd->datlen_u);
               ret = connection.Write(dbderror->dbd->datbuf, dbderror->dbd->datlen_u);
               if (ret != dbderror->dbd->datlen_u)
               {
                   mylog_error(m_pLogGlobalCtrl->errorlog, "write content length %d %d  %s - %s:%s:%d", ret, dbderror->dbd->datlen_u, datalen,INFO_LOG_SUFFIX);
                   SaveSendError(dbderror);
                   continue;
               }
               //Debug(0, 1, ("Debug: write content %d %d in serveClient for ExternalManager\n", dbderror->dbd->datlen_u, ret));
               //socketsend[0]=0;
               //sprintf(socketsend,"write content %d %d\n",ret, dbderror->dbd->datlen_u, strlen(dbderror->dbd->datbuf));
               sscanf(dbderror->key,"%llu/%d", &id, &taskid);
               mylog_error(m_pLogGlobalCtrl->errorlog, "write content %d %llu/%d %s - %s:%s:%d", ret, id, taskid, connection.Get_PeerIP(),INFO_LOG_SUFFIX);
               infocrawler->getLocalDbManager()->saveAlreadyRead(id, taskid);
           }
           delete dbderror;
       }else
       {
           break;
           mylog_error(m_pLogGlobalCtrl->errorlog, "send socket GetSendError is NULL - %s:%s:%d",INFO_LOG_SUFFIX);
       }
   }
    num = errornum;
    list<char *> fetched;
    infocrawler->getLocalDbManager()->readFetched(fetched, num);
    list<char *>::iterator iter;
    int sendnum = 0;
    for(iter = fetched.begin(); iter != fetched.end(); ++iter, sendnum++) {
        ulonglong id = 0;
        int taskid = 0;
        if (*iter) {
            sscanf(*iter, "%llu/%d", &id, &taskid);
            free(*iter);
        } else {
            continue;
        }

        if (requiretaskid > 0 && requiretaskid != taskid) {
            continue;
        }

        DBD *dbd = infocrawler->getLocalDbManager()->readContent(id, taskid);
        if (dbd) {
            if (dbd->datlen_u) {
                socketsend[0]=0;
                char datalen[9] ;datalen[0] = 0;
                sprintf(datalen, "%8d",dbd->datlen_u);
                datalen[8] = 0;
                ret = connection.Write(datalen, 8);
                if (ret != 8)
                {
                    mylog_error(m_pLogGlobalCtrl->errorlog, "ERROR write head length %d %d  %s %d - %s:%s:%d",ret, dbd->datlen_u, datalen, sendnum,INFO_LOG_SUFFIX);
                    dbsenderror * dberror = new dbsenderror;
                    sprintf(dberror->key,"%llu/%d",id, taskid);            
                    dberror->dbd = dbd;
                    SaveSendError(dberror);
                    continue;
                }
                //Debug(0, 1, ("Debug: write length %d %d in serveClient for ExternalManager\n", ret, dbd->datlen_u));
                //sprintf(socketsend,"write length %d %d\n",ret, dbd->datlen_u);
                ret = connection.Write(dbd->datbuf, dbd->datlen_u);
                if (ret != dbd->datlen_u)
                {
                    mylog_error(m_pLogGlobalCtrl->errorlog, "ERROR write content length %d %d  %s %d - %s:%s:%d",ret, dbd->datlen_u, datalen, sendnum,INFO_LOG_SUFFIX);
                    dbsenderror *dberror = new dbsenderror;
                    sprintf(dberror->key,"%llu/%d",id, taskid);            
                    dberror->dbd = dbd;
                    SaveSendError(dberror);
                    continue;
                }
                mylog_error(m_pLogGlobalCtrl->errorlog, "write content %d %llu/%d %s - %s:%s:%d",dbd->datlen_u, id, taskid, connection.Get_PeerIP(),INFO_LOG_SUFFIX);
                //socketsend[0]=0;
                //sprintf(socketsend,"write content %d %d\n",ret, dbd->datlen_u, strlen(dbd->datbuf));
                infocrawler->getLocalDbManager()->saveAlreadyRead(id,taskid);
            }
            dbd_free(dbd);
        } else {
            mylog_info(m_pLogGlobalCtrl->infolog, "can not get data id = %llu taskid = %d - %s:%s:%d",id, taskid ,INFO_LOG_SUFFIX);
        }
    }

    char endbuf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    ret = connection.Write(endbuf, 8);
    //Debug(0, 1, ("Debug: write endbuf %d %d in serveClient for ExternalManager\n", 8, ret));
    char ack[1] = "";
    ret = connection.Read(ack, 1);
    //Debug(0, 1, ("Debug: read ack %d %d in serveClient for ExternalManager\n", 1, ret));
    return 0;
}
#endif
/*void ExternalManager::WriteDBD(Connection connection, DBD * dbd)
{
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    int ret = 0;
    char socketsend[1024]="";
    if (dbd->datlen_u) {
        socketsend[0]=0;
        char datalen[9] ;datalen[0] = 0;
        sprintf(datalen, "%8d",dbd->datlen_u);
        datalen[8] = 0;
        ret = connection.Write(datalen, 8);
        if (ret != 8)
        {
            Debug(0, 1, ("Debug:ERROR write length %d %d  %s in serveClient for ExternalManager\n", ret, dbd->datlen_u, datalen));
            SaveSendError(dbd);
            return;
        }
        Debug(0, 1, ("Debug: write length %d %d in serveClient for ExternalManager\n", ret, dbd->datlen_u));
        sprintf(socketsend,"write length %d %d\n",ret, dbd->datlen_u);
        ret = connection.Write(dbd->datbuf, dbd->datlen_u);
        if (ret != dbd->datlen_u)
        {
            Debug(0, 1, ("Debug:ERROR write length %d %d  %s in serveClient for ExternalManager\n", ret, dbd->datlen_u, datalen));
            SaveSendError(dbd);
            return;
        }
        Debug(0, 1, ("Debug: write content %d %d in serveClient for ExternalManager\n", dbd->datlen_u, ret));
        socketsend[0]=0;
        sprintf(socketsend,"write content %d %d\n",ret, dbd->datlen_u, strlen(dbd->datbuf));
    }
    dbd_free(dbd);
}*/
dbsenderror * ExternalManager::GetSendError()
{
    dbsenderror *dbd = NULL;
    pthread_rwlock_wrlock(&rwlock_socket_error);
    if (!senderror.empty()) {
        dbd = senderror.front();
        senderror.pop_front();
    }
    pthread_rwlock_unlock(&rwlock_socket_error);
    return dbd;
}
void ExternalManager::SaveSendError(dbsenderror * db)
{
    pthread_rwlock_wrlock(&rwlock_socket_error);
    senderror.push_back(db);
    pthread_rwlock_unlock(&rwlock_socket_error);
}
int ExternalManager::ReadFinalPageFormat(int taskid,const char * url,char *&recvbuf)
{
    int retrynum = 0;
retry:
    MyConnection connection;
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    string servername = icconfig->mainserverip; 
    int ret = openConnect(connection, servername, icconfig->mainserverport, EXTERNAL_DEFAULT_TIMEOUT, 3);
    if (ret == 1) {
        ret = connectServer(connection);
    }
    
    if (ret != 1) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not connect mainserver %s %d - %s:%s:%d",(char *)servername.c_str(), icconfig->mainserverport,INFO_LOG_SUFFIX);
        connection.Close();
        if (retrynum++ < 3) {
            goto retry;
        } else {
            return -1;
        }
    }
    
    char division[] = {(char)0x08,0};
    char strtaskid[32] = {0};
    sprintf(strtaskid,"%8d",taskid);
    
    string writebuf = "";
    writebuf.append(strtaskid);
    writebuf.append(division);
    writebuf.append(url);
    writebuf.append(division);
    writebuf.append(SITE_TEMPLATE_ITEM_BBS_FINALPAGEFORMAT);
    
    int writebuflen = writebuf.length();
    char sendhead[33] = {0};
    sprintf(sendhead , "20090910%8d%8d",REQUEST_COMMAND_TEMPLATE,writebuflen);
    int len = strlen(sendhead) + writebuflen;
    char * sendbuf = new char [len + 10];
    sendbuf[0] = 0;
    sprintf(sendbuf,"%s%s",sendhead,writebuf.c_str());
    ret = connection.Write(sendbuf,len);
    
    if (sendbuf)  delete [] sendbuf;
    if (ret != len)
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "write socket error %d  - %s:%s:%d",ret,INFO_LOG_SUFFIX);
        connection.Close(); 
        return -1; 
    }
     

    int n = 16;
    char buf[18] ;buf[0] = 0;

    ret = connection.Read(buf, n);
    buf[n] = 0;

    if (ret != n) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "red socket error %d  - %s:%s:%d",ret,INFO_LOG_SUFFIX);
        connection.Close();
        return -1;
    }

    if(strncmp(buf, "20090910", 8) != 0) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "read head error for ReadFinalPageFormat  - %s:%s:%d",INFO_LOG_SUFFIX);
        connection.Close();
        return -1;
    }
        
    int recvlen = atoi(buf + 8);
    recvbuf = new char [recvlen + 10];
    ret = connection.Read(recvbuf , recvlen);    
    recvbuf[recvlen] = 0;
    if (ret != recvlen)
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "read error for ReadFinalPageFormat ret %d  - %s:%s:%d",ret , INFO_LOG_SUFFIX);
        connection.Close();
        return -1;
    }
    //Log("INFO:sendbuf %s recvbuf %s homeulr %s\n",sendbuf,recvbuf,url);
    connection.Close();
    return  ret;
}

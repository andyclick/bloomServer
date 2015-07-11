#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <curl/curl.h>

#include "InfoCrawler.h"
//#include "content_encoding.h"
#include "Http.h"
#include "MyConnection.h"
#include "utilcpp.h"
#include "InfoCrawler.h"

int CurlDebugCB(CURL *handle, curl_infotype type,unsigned char *data, size_t size,void * node);

HttpProtocol::HttpProtocol()
{ 
}

HttpProtocol::~HttpProtocol()
{ 
}

int ReadBody(MyConnection *connection, Buffer *buffer, Page &page)
{

    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    char    docBuffer[8192];
    int     bytesRead = 0;
    int     length = 0;

    int     bytesToGo = page.m_nContentLength;

    if (bytesToGo <= 0 || bytesToGo > MAX_PAGE_BUF_SIZE)
        bytesToGo = MAX_PAGE_BUF_SIZE;

    char ooo[64] ;ooo[0] = 0;
    sprintf(ooo, "bytestogo = %d", bytesToGo);

    Buffer *tmpbuffer = create_buffer(1024 * 20);

    while (bytesToGo)
    { 
        int ret = 0;
        int needlen = 0;
        if (bytesToGo > 8192) {
            needlen  = 8192;
        } else {
            needlen  = bytesToGo;
        }
        bytesRead = connection->Read(docBuffer, needlen, &ret);
        if (bytesRead <= 0) {
            if (buffer)
                add_buffer(tmpbuffer, docBuffer, ret); 
            length += ret;
            break;
        }

        if (buffer)
            add_buffer(tmpbuffer, docBuffer, bytesRead); 

        length += bytesRead;
        bytesToGo -= bytesRead;
    }

    /*if (page.m_sContentEncoding.compare("deflate") == 0) {
        z_stream z;
        int ret = deflate_write(tmpbuffer->data, &z, tmpbuffer->length, buffer);
        errorlog("Http Request some site return deflated content outlength = %d\n", tmpbuffer->length);
    } else if (page.m_sContentEncoding.compare("gzip") == 0) {
        z_stream z;
        int zlib_init = 0;
        int ret = gzip_write(tmpbuffer->data, &z, tmpbuffer->length , zlib_init,  buffer);
        errorlog("Http Request some site return deflated content outlength = %d\n", tmpbuffer->length);
    } else {
        add_buffer(buffer, tmpbuffer->data, tmpbuffer->length);
    }*/

    add_buffer(buffer, tmpbuffer->data, tmpbuffer->length);

    sprintf(ooo + strlen(ooo), " length = %d", buffer->length);
    mylog_error(m_pLogGlobalCtrl->errorlog, "%s - %s:%s:%d", ooo,INFO_LOG_SUFFIX);   

    free_buffer(tmpbuffer);
    return buffer->length;
}       


int ReadChunkedBody(MyConnection *connection, Buffer *buf)
{   
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
#define  BSIZE  8192

    int            length = 0;
    int            chunk_size;
    char           buffer[BSIZE+1];
    char            chunckheader[64];
    int             chunk, rsize;

    if (!connection->Read_Line(chunckheader, 63, "\r\n"))
        return -1;

    chunckheader[63] = 0;
    if (strlen(chunckheader) == 0) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request Read chunk header chunckheader = %s - %s:%s:%d",chunckheader,INFO_LOG_SUFFIX);   
        return -1;
    }

    sscanf ((char *)chunckheader, "%x", &chunk_size);

    while (chunk_size > 0)
    {
        chunk = chunk_size;

        do {
            if (chunk > BSIZE) {
                rsize = BSIZE;
            } else {
                rsize = chunk;
            }

            int ret = 0;
            int len = connection->Read(buffer, rsize, &ret);
            if (len <= 0) {
                mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request Read chunk erro rsize = %d len = %d - %s:%s:%d",rsize, ret,INFO_LOG_SUFFIX);   
                return -1;
            }

            chunk -= len;

            if (len > MAX_PAGE_BUF_SIZE - buf->length) {
                len = MAX_PAGE_BUF_SIZE - buf->length;
            }

            length += len;

            if (buffer)
                add_buffer(buf, buffer, len); 

            if (length >= MAX_PAGE_BUF_SIZE) {
                return length;
            }


        } while (chunk);

        if (!connection->Read_Line(chunckheader, 63, "\r\n"))
            return -1;

        if (!connection->Read_Line(chunckheader, 63, "\r\n"))
            return -1;

        chunckheader[63] = 0;

        if (strlen(chunckheader) == 0) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request Read chunk header chunckheader = %s lastchuncksize = %d - %s:%s:%d",chunckheader, chunk_size,INFO_LOG_SUFFIX);   
            return -1;
        }

        sscanf ((char *)chunckheader, "%x", &chunk_size);

        //test if chunck is ok
        char tmpchunsizebuf[64] ;tmpchunsizebuf[0 ]= 0;
        sprintf(tmpchunsizebuf, "%x", chunk_size);
        if (strlen(chunckheader) != strlen(tmpchunsizebuf)) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request Read chunk chunckheader = %s chunk_size = %d - %s:%s:%d",chunckheader, chunk_size,INFO_LOG_SUFFIX);   
        }
        //
    }

    return length;
}

int HttpProtocol::login(CUrl &u, Page &page, int httptimeout)
{
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    InfoCrawler *infocrawler = InfoCrawler::getInstance();

    time_t now;
    now = time(NULL);

    int sock, bytesRead = 0;
    int ret = HTTP_FETCH_RET_ERROR, selectRet;

    string requestBuf;

    int port = 80;

    if( u.m_sPort > 0 ) port = u.m_sPort;

    if( u.getUrl().empty()) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request Url is empty - %s:%s:%d",INFO_LOG_SUFFIX);   
        return HTTP_FETCH_RET_ERROR;
    }
    
    requestBuf = postRequestBuf(u);

    mylog_info(m_pLogGlobalCtrl->infolog, "requestBuf %s - %s:%s:%d",requestBuf.c_str(),INFO_LOG_SUFFIX);
    sock = createSocket(u.m_sHost.c_str(), port, httptimeout);

    if(sock == -1) { // invalid host
        return HTTP_FETCH_RET_ERROR_INVALIDHOST;
    }

    ret = write(sock, requestBuf.c_str(), requestBuf.length());

    if( ret == 0 ) {
        close(sock);
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request write error - %s:%s:%d",INFO_LOG_SUFFIX);   
        return HTTP_FETCH_RET_ERROR;
    }

    char headerBuf[HEADER_BUF_SIZE];
    memset(headerBuf, 0, HEADER_BUF_SIZE);
    ret = read_header(sock, headerBuf, httptimeout);
    if(ret < 0) {
        close(sock);
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request Http Request read_header error %s - %s:%s:%d",u.getUrl().c_str(),INFO_LOG_SUFFIX);   
        return HTTP_FETCH_RET_ERROR;
    }

    if( strlen(headerBuf) == 0 ){
        close(sock);
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request Header empty %s - %s:%s:%d",u.getUrl().c_str(),INFO_LOG_SUFFIX);   
        return HTTP_FETCH_RET_ERROR;
    }

    page.ParseHeaderInfo(headerBuf, u.getUrl());

    if (!(HTTP_SUCCESS_STATUS(page.m_nStatusCode) || HTTP_REDIRECTION_STATUS(page.m_nStatusCode)))
    {
        close(sock);
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request status code error %d %s - %s:%s:%d",page.m_nStatusCode, u.getUrl().c_str(),INFO_LOG_SUFFIX);   
        return HTTP_FETCH_RET_ERROR;
    }

    if (HTTP_REDIRECTION_STATUS(page.m_nStatusCode))
    {
        if (page.m_sLocation.empty() || page.m_sLocation.size() > MAX_URL_LEN)
        {
                close(sock);
                    mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request location is null or too long %s %s - %s:%s:%d", page.m_sLocation.c_str(), u.getUrl().c_str(),INFO_LOG_SUFFIX);   
                        return HTTP_FETCH_RET_ERROR;
        } else {
                close(sock);
                    return HTTP_FETCH_RET_REDIRECT;
        }
    }
    if (sock >= 0)                                                                                                            
    {                                                                                                                         
        ret = close(sock);                                                                                                    
        sock = -1;                                                                                                                
    }
    return 0; 
}

static int showstatistic(CURL *curl, char *downstatistic) {
    long redirect_count = 0;
    curl_easy_getinfo(curl, CURLINFO_REDIRECT_COUNT, &redirect_count);

    if (downstatistic) {
        double namelookup_time = 0, connect_time = 0, appconnect_time = 0, 
            pretransfer_time = 0, starttransfer_time = 0, total_time = 0, redirect_time = 0,
            download_speed = 0, download_size = 0, contentlen = 0; 
        long header_size = 0;

        curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME, &namelookup_time);
        curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &connect_time);
        curl_easy_getinfo(curl, CURLINFO_APPCONNECT_TIME, &appconnect_time);
        curl_easy_getinfo(curl, CURLINFO_PRETRANSFER_TIME, &pretransfer_time);
        curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME, &starttransfer_time);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
        curl_easy_getinfo(curl, CURLINFO_REDIRECT_TIME, &redirect_time);
        curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &download_speed);
        curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &download_size);
        curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE, &header_size);
        curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentlen);
        

        sprintf(downstatistic, " nt %f ct %f at %f pt %f st %f tt %f rt %f sd %f sid %f hs %d rc %d cl %f", 
            namelookup_time, connect_time, appconnect_time, 
            pretransfer_time, starttransfer_time, total_time, redirect_time,
            download_speed, download_size, header_size, redirect_count, contentlen);
    }
    return redirect_count;
}

long bufferwriter(void *data, int size, int nmemb, Buffer *buff) {
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    long sizes = size * nmemb;
    if (data) {
        add_buffer(buff, (char *)data, sizes);
        if (buff->length > MAX_PAGE_BUF_SIZE) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request Content length excess maximum length %d - %s:%s:%d", buff->length,INFO_LOG_SUFFIX);   
            return 0;
        }
    }
    return sizes;
}

int HttpProtocol::curl_login(CURL *curl, CUrl &u, UrlNode *urlnode, int httptimeout, RESPONSE_HEADER *rheader, char *downstatistic) {
    Buffer *buff = create_buffer(1024 * 20);
    int ret = curl_fetch(curl, u, buff, urlnode, httptimeout, REQUEST_TYPE_POST, rheader, downstatistic);
    free_buffer(buff);
    return ret;
}

int HttpProtocol::curl_fetch(CURL *curl, CUrl &u, Buffer *buff, UrlNode *urlnode, int httptimeout, int requesttype, RESPONSE_HEADER *rheader, char *downstatistic) 
{
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    errno = 0;

    //CURL *curl = NULL;
    CURLcode code;
    long retcode = 0;
    char *contenttype = NULL;
    char *urlnodecookie = NULL;

    char error[CURL_ERROR_SIZE] ;error[0] = 0;
    //curl = curl_easy_init();
    int ret = HTTP_FETCH_RET_ERROR;


	string cookie;
	string post;
        if (urlnode->task->sourcetype == SOURCE_TYPE_COMPANY && urlnode->type & URL_TYPE_HOMEPAGE)
        {   
		FILE * f = fopen("ali.txt", "rb");
		char line[1024 * 4] = {0};
		int i = 0;
		while(f && fgets(line, 1023 * 3, f)) {
			char *newline = strtrim(line, NULL);
			if (i++ == 0) {
				cookie = newline;
			} else {
				post = newline;
			}
		}
		if (f) fclose(f);
        }   

    if (icconfig->localipnum > 0)
    {
        string iptmp = icconfig->localips[urlnode->ipnum];
        code = curl_easy_setopt(curl, CURLOPT_INTERFACE , iptmp.c_str());
        mylog_error(m_pLogGlobalCtrl->errorlog, "user IP %s  - %s:%s:%d",iptmp.c_str(), INFO_LOG_SUFFIX);   
        if (code != CURLE_OK) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_INTERFACE - %s:%s:%d", INFO_LOG_SUFFIX);   
            goto end;
        }
    }
    code = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);
    if (code != CURLE_OK) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_ERRORBUFFER - %s:%s:%d", INFO_LOG_SUFFIX);   
        goto end;
    }

	/*
    code = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    if (code != CURLE_OK) {
        errorlog("Http Request can not setopt CURLOPT_VERBOSE\n");
        goto end;
    }
	*/
    code = curl_easy_setopt(curl, CURLOPT_MAXCONNECTS, 1);
    if (code != CURLE_OK) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_MAXCONNECTS %d %s %s - %s:%s:%d",  code, curl_easy_strerror(code), (char *)u.getUrl().c_str(),INFO_LOG_SUFFIX);   
        goto end;
    }

    code = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    if (code != CURLE_OK) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_NOSIGNAL %d %s %s - %s:%s:%d", code, curl_easy_strerror(code), (char *)u.getUrl().c_str() ,INFO_LOG_SUFFIX);   
        goto end;
    }

    code = curl_easy_setopt(curl,CURLOPT_AUTOREFERER , 1);
    if (code != CURLE_OK) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_MAXCONNECTS %d %s %s - %s:%s:%d", code, curl_easy_strerror(code), (char *)u.getUrl().c_str(),INFO_LOG_SUFFIX);
        goto end;
    }

    if (cookie.length() > 0) {
	    code = curl_easy_setopt(curl, CURLOPT_COOKIE, (char *)cookie.c_str());
	    //code = curl_easy_setopt(curl, CURLOPT_COOKIELIST, (char *)cookie.c_str());
	    if (code != CURLE_OK) {
		    mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_COOKIE %d %s %s\n", code, curl_easy_strerror(code), (char *)u.getUrl().c_str());
		    goto end;
	    }
    }

    if (requesttype == REQUEST_TYPE_POST) {
        code = curl_easy_setopt(curl, CURLOPT_POST, 1);
        if (code != CURLE_OK) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_POST %d %s %s - %s:%s:%d", code, curl_easy_strerror(code), (char *)u.getUrl().c_str(),INFO_LOG_SUFFIX);   
            goto end;
        }

        char url[MAX_URL_LEN] ;url[0] = 0;
        char param[MAX_URL_LEN * 3] ;param[0] = 0;
        strcpy(url, (char *)u.getUrl().c_str());
        char *tmp = strchr(url,'?');

        if (tmp) {
            *tmp = 0;
            strcpy(param, tmp + 1); 
        }
	strcat(param, "&");
	strcat(param, (char *)post.c_str());

        code = curl_easy_setopt(curl, CURLOPT_URL, url);
        if (code != CURLE_OK) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_URL %d %s %s - %s:%s:%d",code, curl_easy_strerror(code), url ,INFO_LOG_SUFFIX);   
            goto end;
        }

        if (param[0]) {
            code = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, param);
            if (code != CURLE_OK) {
                mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_POSTFIELDS %d %s %s %s - %s:%s:%d",code, curl_easy_strerror(code), (char *)u.getUrl().c_str(), param,INFO_LOG_SUFFIX);   
                goto end;
            }
        }
    } else {
        code = curl_easy_setopt(curl, CURLOPT_URL, (char *)u.getUrl().c_str());
        if (code != CURLE_OK) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_URL %d %s %s - %s:%s:%d",code, curl_easy_strerror(code), (char *)u.getUrl().c_str(),INFO_LOG_SUFFIX);   
            goto end;
        }
    }

    code = curl_easy_setopt(curl, CURLOPT_PORT, u.m_sPort);
    if (code != CURLE_OK) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_PORT %d %s %s  - %s:%s:%d",code, curl_easy_strerror(code), (char *)u.getUrl().c_str(),INFO_LOG_SUFFIX);   
        goto end;
    }

    code = curl_easy_setopt(curl, CURLOPT_USERAGENT, infocrawler->getConf()->useragent);
    if (code != CURLE_OK) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_USERAGENT %d %s %s  - %s:%s:%d",code, curl_easy_strerror(code), (char *)u.getUrl().c_str(),INFO_LOG_SUFFIX);   
        goto end;
    }



    /*urlnodecookie = (char *)urlnode->getothervalue(URLNODE_OTHER_TYPE_COOKIE);
    if (urlnodecookie) {
        code = curl_easy_setopt(curl, CURLOPT_COOKIELIST, urlnodecookie);
        if (code != CURLE_OK) {
            errorlog("Http Request can not setopt CURLOPT_COOKIE %d %s %s\n", code, curl_easy_strerror(code), (char *)u.getUrl().c_str());
            goto end;
        }
    } else {
        curl_easy_setopt(curl, CURLOPT_COOKIELIST, "ALL");
    }
    */

	if (false) {
		code = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		if (code != CURLE_OK) {
			mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_VERBOSE %d %s %s  - %s:%s:%d",code, curl_easy_strerror(code), (char *)u.getUrl().c_str(),INFO_LOG_SUFFIX);
			goto end;
		}

		code = curl_easy_setopt(curl,  CURLOPT_READDATA, (char *)u.getUrl().c_str());
		if (code != CURLE_OK) {
			mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_READDATA %d %s %s  - %s:%s:%d",code, curl_easy_strerror(code), (char *)u.getUrl().c_str(),INFO_LOG_SUFFIX);
			goto end;
		}

		code = curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, CurlDebugCB);
		if (code != CURLE_OK) {
			mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_DEBUGFUNCTION %d %s %s  - %s:%s:%d",code, curl_easy_strerror(code), (char *)u.getUrl().c_str(),INFO_LOG_SUFFIX);
			goto end;
		}
	}

    code = curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_NONE);
    if (code != CURLE_OK) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_FOLLOWLOCATION %d %s %s  - %s:%s:%d",code, curl_easy_strerror(code), (char *)u.getUrl().c_str(),INFO_LOG_SUFFIX);   
        goto end;
    }

    code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    if (code != CURLE_OK) {
        
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_FOLLOWLOCATION %d %s %s  - %s:%s:%d",code, curl_easy_strerror(code), (char *)u.getUrl().c_str(),INFO_LOG_SUFFIX);   
        goto end;
    }

    code = curl_easy_setopt(curl, CURLOPT_MAXFILESIZE, MAX_PAGE_BUF_SIZE);
    if (code != CURLE_OK) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_MAXFILESIZE %d %s %s  - %s:%s:%d",code, curl_easy_strerror(code), (char *)u.getUrl().c_str(),INFO_LOG_SUFFIX);   
        goto end;
    }
    
    code = curl_easy_setopt(curl, CURLOPT_MAXREDIRS, URL_FETCH_REDIRECT_TIMES); //may cause CURLE_TOO_MANY_REDIRECTS error
    if (code != CURLE_OK) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_MAXREDIRS %d %s %s  - %s:%s:%d",code, curl_easy_strerror(code), (char *)u.getUrl().c_str(),INFO_LOG_SUFFIX);   
        goto end;
    }

    code = curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
    if (code != CURLE_OK) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_REDIR_PROTOCOLS %d %s %s  - %s:%s:%d", code, curl_easy_strerror(code), (char *)u.getUrl().c_str(),INFO_LOG_SUFFIX);   
        goto end;
    }

    if (urlnode && (urlnode->fatherurl[0] || urlnode->refererurl[0])) {
        if (urlnode->refererurl[0])
        {
            code = curl_easy_setopt(curl, CURLOPT_REFERER, urlnode->refererurl);
        }else
        {
            code = curl_easy_setopt(curl, CURLOPT_REFERER, urlnode->fatherurl);
        }
        if (code != CURLE_OK) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_REFERER %d %s %s %s %d - %s:%s:%d",code, curl_easy_strerror(code), (char *)u.getUrl().c_str(), urlnode->fatherurl, httptimeout,INFO_LOG_SUFFIX);
            goto end;
        }
    }
    code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, bufferwriter);
    if (code != CURLE_OK) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_WRITEFUNCTION %d %s %s  - %s:%s:%d",  code, curl_easy_strerror(code), (char *)u.getUrl().c_str(),INFO_LOG_SUFFIX);   
        goto end;
    }

    code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, buff);
    if (code != CURLE_OK) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_WRITEDATA %d %s %s  - %s:%s:%d",  code, curl_easy_strerror(code), (char *)u.getUrl().c_str(),INFO_LOG_SUFFIX);   
        goto end;
    }

    code = curl_easy_setopt(curl, CURLOPT_ENCODING, "gzip, deflate");
    if (code != CURLE_OK) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_ENCODING %d %s %s  - %s:%s:%d", code, curl_easy_strerror(code), (char *)u.getUrl().c_str(),INFO_LOG_SUFFIX);   
        goto end;
    }

    code = curl_easy_setopt(curl, CURLOPT_TIMEOUT, httptimeout * 15); //five minutes
    if (code != CURLE_OK) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_TIMEOUT %d %s %s %d  - %s:%s:%d", code, curl_easy_strerror(code), (char *)u.getUrl().c_str(), httptimeout,INFO_LOG_SUFFIX);   
        goto end;
    }

    code = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, httptimeout);
    if (code != CURLE_OK) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not setopt CURLOPT_CONNECTTIMEOUT %d %s %s %d  - %s:%s:%d", code, curl_easy_strerror(code), (char *)u.getUrl().c_str(),httptimeout,INFO_LOG_SUFFIX);   
        goto end;
    }

    code = curl_easy_perform(curl);
    if (code != CURLE_OK) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request CURLE_ERROR %d %s %s %s  - %s:%s:%d", code, curl_easy_strerror(code), (char *)u.getUrl().c_str(), error,INFO_LOG_SUFFIX);   
        goto end;
    }

    code = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &retcode);
    if (code == CURLE_OK) {
        rheader->retcode = retcode;
        if (!HTTP_SUCCESS_STATUS(retcode)) {
            ret = HTTP_FETCH_RET_ERROR;
            mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request status code error %d %s %s %d  - %s:%s:%d", code, curl_easy_strerror(code), u.getUrl().c_str(), retcode,INFO_LOG_SUFFIX);   
            goto end;
        }
    } else {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not getinfo CURLINFO_RESPONSE_CODE %d %s %s  - %s:%s:%d",  code, curl_easy_strerror(code), (char *)u.getUrl().c_str(),INFO_LOG_SUFFIX);   
        goto end;
    }

    code = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &contenttype);
    if (code == CURLE_OK) {
        if (contenttype) {
            char *contenttypetmp = strdup(contenttype);
            char *delim = strchr(contenttypetmp, ';');
            if(delim) {
                *delim = 0;
                rheader->contenttype = strtrim(contenttypetmp, NULL);
                char *charsetbegin = strstr(delim + 1, "charset=");
                if(charsetbegin) {
                    rheader->charset = strtrim(charsetbegin + strlen("charset="), NULL);
                }
            } else {
                rheader->contenttype = strtrim(contenttypetmp, NULL);
            }
            free(contenttypetmp);
            toLowercase(rheader->contenttype);
            toLowercase(rheader->charset);
            if (!HTTP_ACCEPT_CONETNT(rheader->contenttype)) {
                ret = HTTP_FETCH_RET_ERROR_UNACCEPTED;
                mylog_error(m_pLogGlobalCtrl->errorlog, "ERROR: fetched %s unaccepted contenttype %s %s  - %s:%s:%d",  u.getUrl().c_str(), rheader->contenttype.c_str(), rheader->charset.c_str(),INFO_LOG_SUFFIX);   
                goto end;
            }
        } 
    } else {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request can not getinfo CURLINFO_CONTENT_TYPE %d %s %s %s  - %s:%s:%d",  code, curl_easy_strerror(code), (char *)u.getUrl().c_str(), error,INFO_LOG_SUFFIX);   
        goto end;
    }

    ret = buff->length;
    rheader->contentlength = buff->length;

end:
    int redirect_count = showstatistic(curl, downstatistic);
    if (redirect_count > 0) {
        char *redirecturl = NULL;
        code = curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &redirecturl);
        rheader->location = redirecturl;
    }
    return ret;
}

int HttpProtocol::fetch(CUrl &u, Buffer *buff, UrlNode *urlnode, Page &page, int httptimeout, int requesttype)
{
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    errno = 0;
    
	time_t now;
    now = time(NULL);

    int sock, bytesRead = 0;
	int ret = HTTP_FETCH_RET_ERROR, selectRet;

	string requestBuf = "";

	int port = 80;

    if( u.m_sPort > 0 ) port = u.m_sPort;

	if( u.getUrl().empty()) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request Url is empty - %s:%s:%d",INFO_LOG_SUFFIX);   
		return HTTP_FETCH_RET_ERROR;
	}
    if (requesttype == REQUEST_TYPE_GET)
    {
        requestBuf = getRequestBuf(u,urlnode);
    }else if (requesttype == REQUEST_TYPE_POST){
        requestBuf = postRequestBuf(u);
    }

    //Debug(1, 1, ("Debug: requesttype %d  requestBuf\n %s \n",requesttype, requestBuf.c_str()));
    sock = createSocket(u.m_sHost.c_str(), port, httptimeout);

	if(sock == -1) { // invalid host
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request invalid host %s - %s:%s:%d",u.m_sHost.c_str(),INFO_LOG_SUFFIX);   
		return HTTP_FETCH_RET_ERROR_INVALIDHOST;
	}

	ret = write(sock, requestBuf.c_str(), requestBuf.length());

	if( ret == 0 ) {
		close(sock);
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request write error - %s:%s:%d",INFO_LOG_SUFFIX);   
		return HTTP_FETCH_RET_ERROR;		
	}

	char headerBuf[HEADER_BUF_SIZE];
	memset(headerBuf, 0, HEADER_BUF_SIZE);
	ret = read_header(sock, headerBuf, httptimeout);
	if(ret < 0) { 
		close(sock); 
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request Http Request read_header error %s - %s:%s:%d",u.getUrl().c_str(),INFO_LOG_SUFFIX);   
		return HTTP_FETCH_RET_ERROR;
	}

	if( strlen(headerBuf) == 0 ){
		close(sock);
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request Header empty %s - %s:%s:%d",u.getUrl().c_str(),INFO_LOG_SUFFIX);   
		return HTTP_FETCH_RET_ERROR;
	}

	page.ParseHeaderInfo(headerBuf, u.getUrl());

	if (!(HTTP_SUCCESS_STATUS(page.m_nStatusCode) || HTTP_REDIRECTION_STATUS(page.m_nStatusCode)))
	{
		close(sock);
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request status code error %d %s - %s:%s:%d",page.m_nStatusCode, u.getUrl().c_str(),INFO_LOG_SUFFIX);   
		return HTTP_FETCH_RET_ERROR;
	}

	if (HTTP_REDIRECTION_STATUS(page.m_nStatusCode))
	{
		if (page.m_sLocation.empty() || page.m_sLocation.size() > MAX_URL_LEN)
		{	
			close(sock);
            mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request location is null or too long %s %s - %s:%s:%d",page.m_sLocation.c_str(), u.getUrl().c_str(),INFO_LOG_SUFFIX);   
			return HTTP_FETCH_RET_ERROR;
		} else {
			close(sock);
			return HTTP_FETCH_RET_REDIRECT;
		}
	}

	// content begin
    if(!page.m_sContentType.empty() && !HTTP_ACCEPT_CONETNT(page.m_sContentType)) {
		close(sock);
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request Content can not be accepted Error, %s %s - %s:%s:%d",page.m_sContentType.c_str(), u.getUrl().c_str(),INFO_LOG_SUFFIX);   
		return HTTP_FETCH_RET_ERROR_UNACCEPTED;
	}

	if (page.m_nContentLength == -1)
	{
		close(sock);
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request Content length Error %s - %s:%s:%d",u.getUrl().c_str(),INFO_LOG_SUFFIX);   
		return HTTP_FETCH_RET_ERROR;
	}

	if (page.m_nContentLength == 0 || page.m_nContentLength < 20)
	{ // Allocate enough memory to hold the page 
		page.m_nContentLength = MAX_PAGE_BUF_SIZE;
	}

	if (page.m_nContentLength > MAX_PAGE_BUF_SIZE)
	{
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request Content length excess maximum length %d, %s - %s:%s:%d",page.m_nContentLength, u.getUrl().c_str(),INFO_LOG_SUFFIX);   
		close(sock);
		return HTTP_FETCH_RET_ERROR;
	}

	int flags;
	flags = fcntl(sock,F_GETFL,0);
    if(flags < 0) {
		close(sock);
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request fnctl error - %s:%s:%d",INFO_LOG_SUFFIX);   
		return HTTP_FETCH_RET_ERROR;
	}
    flags |= O_NONBLOCK;
    if(fcntl(sock, F_SETFL, flags) < 0) {
		close(sock);
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request fnctl error - %s:%s:%d",INFO_LOG_SUFFIX);   
		return HTTP_FETCH_RET_ERROR;
	}
    

    MyConnection connection(sock);
    connection.Timeout(httptimeout);

    if (page.m_sTransferEncoding.compare("chunked") == 0) {
        bytesRead = ReadChunkedBody(&connection, buff);
    } else {
        bytesRead = ReadBody(&connection, buff, page);
    }

    //deal with enconding
    if (bytesRead <= 0) {
        connection.Close();
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request Cann't get content in %s %d - %s:%s:%d",u.getUrl().c_str(), errno,INFO_LOG_SUFFIX);   
        return HTTP_FETCH_RET_ERROR;
    }

    connection.Close();

	return bytesRead;
}
	
int HttpProtocol::createSocket(const char *host, int port, int timeout)
{
	LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    int sock;		// Socket descriptor
	struct sockaddr_in sa;	// Socket address

	unsigned long inaddr;
	int ret;

    Host *h = get_host((char *)host);

    if (h == NULL) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request Create socket fail: Unknow host %s - %s:%s:%d",host,INFO_LOG_SUFFIX);   
        return -1;
    }
	char *ip = h->dotaddr;
	if(ip == NULL) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request invalid host: %s - %s:%s:%d",host,INFO_LOG_SUFFIX);   
        free(h);
		return -1;
	} else {
        //inside
        inaddr = (unsigned long)inet_addr(ip);
        free(h);

        if( inaddr == INADDR_NONE ){
            // release the buffer, be careful
            //free(ip); 
            ip = NULL;
            mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request invalid ip: %d - %s:%s:%d",inaddr,INFO_LOG_SUFFIX);   
            return -1;
        }

        memcpy((char *)&sa.sin_addr, (char *)&inaddr, sizeof(inaddr));

        // release the buffer, be carful
        //free(ip); 
        ip = NULL;
	}

	/* Copy host address from hostent to (server) socket address */
	sa.sin_family = AF_INET;		
	sa.sin_port = htons(port);	/* Put portnum into sockaddr */

	sock = -1;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0 ) { 
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request socket error - %s:%s:%d",INFO_LOG_SUFFIX);   
		return -1;
	}

	int optval = 1;
  	if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR,
		(char *)&optval, sizeof (optval)) < 0) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request setsockopt error - %s:%s:%d",INFO_LOG_SUFFIX);   
		close(sock);
		return -1;
	}

    ret = nonb_connect(sock, (struct sockaddr *)&sa, timeout);
    if(ret == -1) { 
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request nonb_connect error %s %d - %s:%s:%d", host, timeout,INFO_LOG_SUFFIX);   
        close(sock);
        return -1; 
    }
    return sock;
}
		

int HttpProtocol::read_header(int sock, char *headerPtr, int timeout)
{
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
	fd_set rfds;
	struct timeval tv;
	int bytesRead = 0, newlines = 0, ret, selectRet;

	int flags;

	flags=fcntl(sock,F_GETFL,0);
	if(flags<0){
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request fcntl in read_header - %s:%s:%d", INFO_LOG_SUFFIX);   
		return -1;
	}

    flags|=O_NONBLOCK;
    if(fcntl(sock,F_SETFL,flags)<0){
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request fcntl in read_header - %s:%s:%d", INFO_LOG_SUFFIX);   
		return -1;
	}
    char headerbyte = 0;
	while(newlines != 2) {
        do {
            FD_ZERO(&rfds);
            FD_SET(sock, &rfds);
            tv.tv_sec = timeout;
            tv.tv_usec = 0;

            if(timeout >= 0)
                selectRet = select(sock+1, &rfds, NULL, NULL, &tv);
            else            /* No timeout, can block indefinately */
                selectRet = select(sock+1, &rfds, NULL, NULL, NULL);
                //selectRet = 1;

            if(selectRet == 0) {
                mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request timeout(%d) - %s:%s:%d", timeout,INFO_LOG_SUFFIX);   
                return -1;
             } else if (selectRet == -1 && (errno != EINTR && errno != EAGAIN)) {
                mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request timeout(%d) - %s:%s:%d:%d", timeout, ERROR_LOG_SUFFIX);   
                return -1;
            }
        }
        while(selectRet < 0 && (errno == EINTR || errno == EAGAIN));

		ret = read(sock, &headerbyte, 1);
        if(ret == -1) {
            if ((errno == EINTR || errno == EAGAIN)) {
                continue;
            } else {
                mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request read error in read_header - %s:%s:%d", timeout,INFO_LOG_SUFFIX);   
                return -1;
            }
        } else if (ret == 0) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "et = 0 and bytes alread read %d - %s:%s:%d", bytesRead,INFO_LOG_SUFFIX);   
            return -1;
        } else {
            bytesRead++;
        }

        if (bytesRead > 1024 * 10) {
            mylog_error(m_pLogGlobalCtrl->errorlog, "exceed max header len 1024 * 10 in read_header - %s:%s:%d", INFO_LOG_SUFFIX);   
            return -1;
        }

        if (bytesRead < HEADER_BUF_SIZE - 1) {
            *headerPtr = headerbyte;
            headerPtr++;
        }
		if(headerbyte== '\r') {
			continue;
		}
		else if(headerbyte== '\n')
			newlines++;
		else    
			newlines = 0;
                
	}
        
	headerPtr -= 2;
	*headerPtr = '\0';

    flags &= ~O_NONBLOCK;
	fcntl(sock,F_GETFL, flags);
	return bytesRead;
}

/*
 *function nonblocking connect
 *parameter sec is the second of timing out
 * return: -1 error occure, otherwise, 0 
 */

int HttpProtocol::nonb_connect(int sockfd, struct sockaddr* sa, int sec)
{
	LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    int flags, n, error = 0;
	int status;
    fd_set rset, wset;
	struct timeval timeout;

	//set the socket as nonblocking
	flags = fcntl(sockfd, F_GETFL, 0);
	if(flags < 0) return -1;
	flags |= O_NONBLOCK;

	if(fcntl(sockfd, F_SETFL, flags) < 0) {
        close(sockfd);
        mylog_error(m_pLogGlobalCtrl->errorlog, "Http Request fcntl() in nonb_connect - %s:%s:%d", INFO_LOG_SUFFIX);   
		return -1;
	}

	if((n = connect(sockfd, sa, sizeof(struct sockaddr))) == 0) {
		flags &= ~O_NONBLOCK;
		fcntl(sockfd, F_SETFL, flags);
		return sockfd;
    }

    if (n == 0)
        goto done;

    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    wset = rset;
    timeout.tv_sec = sec;
    timeout.tv_usec = 0;

    if ((n = select(sockfd + 1, &rset, &wset, NULL, 
                sec? &timeout : NULL)) == 0) {
        close(sockfd); //timeout 
        errno = ETIMEDOUT;
        return -1;
    }

    if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
        socklen_t len = sizeof(error);
        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
            close(sockfd); //timeout 
            return -1;         /* Solaris pending error   */
        }
    } else {
        close(sockfd); //timeout 
        return -1;
    }


done:
    flags &= ~O_NONBLOCK;
    fcntl(sockfd,   F_SETFL, flags);    /* restore file status flags */
    if (error) {
        close(sockfd);                   /* just in case */
        errno = error;
        return -1;
    }
    return 0;
}

string HttpProtocol::getRequestBuf(CUrl &u , UrlNode *urlnode)
{
    string requestBuf = "";
    string cookie ="";
    InfoCrawler *infocrawler = InfoCrawler::getInstance();
    int port = 80;

    if( u.m_sPort > 0 ) port = u.m_sPort;
    
    //get cookie
    TaskOtherInfo *taskother = infocrawler->getTaskScheduleManager()->getTaskOtherInfo(urlnode->taskid); 
    if (taskother){
        if (taskother->taskcookie){
            cookie = taskother->taskcookie;
        }
    }
    char * urlnodecookie = (char *)urlnode->getothervalue(URLNODE_OTHER_TYPE_COOKIE);
    if (urlnodecookie){
        cookie.append(urlnodecookie);
    }
    /* Copy the url passed in into a buffer we can work with, change, etc. */
    if(u.m_sPath.length() < 1 ) {
        /* The url has no '/' in it, assume the user is making a root-level request */
        requestBuf.append("GET ");
        requestBuf.append(HTTP_VERSION);
        requestBuf.append(HTTP_SEPARATOR);
    } else {
        requestBuf.append("GET ");
        requestBuf.append(u.m_sPath);
        requestBuf.append(" ");
        requestBuf.append(HTTP_VERSION);
        requestBuf.append(HTTP_SEPARATOR);
    }

    requestBuf.append("Host: ");
    requestBuf.append(u.m_sHost);
    requestBuf.append(HTTP_SEPARATOR);


    requestBuf.append("Cookie: ");
    if (!cookie.empty())
    {
        requestBuf.append(cookie);
    }

    requestBuf.append(HTTP_SEPARATOR);

    requestBuf.append("User-Agent: ");
    requestBuf.append(infocrawler->getConf()->useragent);
    requestBuf.append(HTTP_SEPARATOR);

    requestBuf.append("Connection: Close");
    requestBuf.append(HTTP_SEPARATOR);
    requestBuf.append(HTTP_SEPARATOR); //end of request
    
    return requestBuf;
}

string HttpProtocol::postRequestBuf(CUrl &u)
{
    string path, Postparameter ,Postpath,PosttrequestBuf;
    int port = 80;
    if( u.m_sPort > 0 ) port = u.m_sPort;

    path = u.m_sPath;
    string host = u.m_sHost;
    unsigned int end = path.find_last_of("?");
    if (end != string::npos) {
        Postparameter=path.substr(end+1,path.length());
        Postpath = path.substr(0, end) ;
    }

    if( Postpath.length() < 1 ){
        PosttrequestBuf.append("POST  ");
        PosttrequestBuf.append(HTTP_VERSION);
        PosttrequestBuf.append(HTTP_SEPARATOR);
    }else{
        PosttrequestBuf.append("POST  ");
        PosttrequestBuf.append(Postpath);
        PosttrequestBuf.append(" ");
        PosttrequestBuf.append(HTTP_VERSION);
        PosttrequestBuf.append(HTTP_SEPARATOR);
    }
    PosttrequestBuf.append("Host: ");
    PosttrequestBuf.append(host);
    PosttrequestBuf.append(HTTP_SEPARATOR);
    PosttrequestBuf.append("Content-Type: application/x-www-form-urlencoded");
    PosttrequestBuf.append(HTTP_SEPARATOR);

    PosttrequestBuf.append("Content-Length: ");
    char len[10];
    len[0]=0;
    sprintf(len,"%d",Postparameter.length());
    PosttrequestBuf.append(len);
    PosttrequestBuf.append(HTTP_SEPARATOR);

    PosttrequestBuf.append("Connection: Keep-Alive ");
    PosttrequestBuf.append(HTTP_SEPARATOR);
    PosttrequestBuf.append("Cache-Control: no-cache");
    PosttrequestBuf.append(HTTP_SEPARATOR);
    PosttrequestBuf.append(HTTP_SEPARATOR);
    PosttrequestBuf.append(Postparameter.c_str());
    PosttrequestBuf.append(HTTP_SEPARATOR);
    
    return PosttrequestBuf;
}

int CurlDebugCB(CURL *handle, curl_infotype type,unsigned char *data, size_t size,void * node)
{
	LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
	static const char * const loghead[] = {
		"","=> Send header", "=> Send data", "<= Recv header", "<= Recv data", "<= Recv SSL data", "=> Send SSL data"
	};

	switch(type) {
		case CURLINFO_HEADER_OUT:
			mylog_error(m_pLogGlobalCtrl->errorlog, "%s========%s  %s - %s:%s:%d", loghead[type],data,node?node:"",INFO_LOG_SUFFIX);
			break;
		case CURLINFO_DATA_OUT:
			mylog_error(m_pLogGlobalCtrl->errorlog, "%s========%s  %s - %s:%s:%d", loghead[type],data,node?node:"",INFO_LOG_SUFFIX);
			break;
		case CURLINFO_HEADER_IN:
			mylog_error(m_pLogGlobalCtrl->errorlog, "%s========%s  %s - %s:%s:%d", loghead[type],data,node?node:"",INFO_LOG_SUFFIX);
			break;
		case CURLINFO_DATA_IN:
			mylog_error(m_pLogGlobalCtrl->errorlog, "%s========%s  %s - %s:%s:%d", loghead[type],data,node?node:"",INFO_LOG_SUFFIX);

			break;
		case CURLINFO_SSL_DATA_IN:
			mylog_error(m_pLogGlobalCtrl->errorlog, "%s========%s  %s - %s:%s:%d", loghead[type],data,node?node:"",INFO_LOG_SUFFIX);

			break;
		case CURLINFO_SSL_DATA_OUT:
			mylog_error(m_pLogGlobalCtrl->errorlog, "%s========%s  %s - %s:%s:%d", loghead[type],data,node?node:"",INFO_LOG_SUFFIX);
			break;
		case CURLINFO_TEXT:
			mylog_error(m_pLogGlobalCtrl->errorlog, "%s========%s  %s - %s:%s:%d", loghead[type],data,node?node:"",INFO_LOG_SUFFIX);
			break;
	}

}

#include "cmsinfo.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h> 
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static char ip[32] ="";
static char host[] = "cmsinfo.search.tech";
static int port = 3456;

void SetServer(char *_ip, int _port) {
	strcpy(ip, _ip);
	port = _port;
}

char *GetUrlsPattern(char *url) {
	int len = strlen(url);
	char *content = (char *)malloc(len + 8 + 1);
	int sendlen = sprintf(content, "%8d%s", len, url);
	
	char *out = NULL;
	int ret = GetResult(sendlen, content, COMMAND_GET_URL_PATTERN, &out);
	free(content);
    if (!out) {
         out = strdup("  ");
    }     
	return out;
}

char *CheckUrlPattern(char *url, char *pattern, char *out) {
	
	int urllen = strlen(url);
	int patternlen = strlen(pattern);
	char *content = (char *)malloc(urllen + patternlen + 16 + 1);
	int sendlen = sprintf(content, "%8d%8d%s%s", urllen, patternlen, url, pattern);

	char *rout = NULL;
	int ret = GetResult(sendlen, content, COMMAND_CHECK_URL_PATTERN, &rout);
	free(content);

	if (ret == 0) {		
		int outlen = 0, out1len = 0;
        char *routtmp = rout;
		sscanf(rout, "%8d", &outlen); rout += 8;
		sscanf(rout, "%8d", &out1len); rout += 8;
		char *out1 = (char *)malloc(out1len + 1);
		
		memcpy(out, rout, outlen);
		out[outlen] = 0;
		
		memcpy(out1, rout + outlen, out1len);
		out1[out1len] = 0;
        free(routtmp);
		return out1;
	}

	return strdup(" ");
}

char *CheckHtmlPattern(char *url, char *tmpl) {
	int urllen = strlen(url);
	int tmpllen = strlen(tmpl);
	char *content = (char *)malloc(urllen + 8 + tmpllen + 8 + 1);
	int sendlen = sprintf(content, "%8d%8d%s%s", urllen, tmpllen, url, tmpl);
	
	char *out = NULL;
	int ret = GetResult(sendlen, content, COMMAND_CHECK_HTML_PATTERN, &out);
	free(content);

    if (!out) {
         out = strdup("  ");
    }     

	return out;
}

char *GetHtmlPattern (char *url, char *title, char *pubdate, 
					  char *src, char *writer, char *content, char *checkurl) {
	int urllen = strlen(url);
	int titlelen = strlen(title);
	int pubdatelen = strlen(pubdate);
	int srclen = strlen(src);
	int writerlen = strlen(writer);
	int contentlen = strlen(content);
	int checkurllen = strlen(checkurl);

	char *rcontent = (char *)malloc(8 * 7 + urllen + titlelen + pubdatelen + srclen + writerlen + contentlen + checkurllen + 1);
	int sendlen = sprintf(rcontent, "%8d%8d%8d%8d%8d%8d%8d%s%s%s%s%s%s%s", urllen, titlelen, pubdatelen, srclen, writerlen, contentlen, checkurllen
		, url, title, pubdate, src, writer, content, checkurl);

	char *out = NULL;
	int ret = GetResult(sendlen, rcontent, COMMAND_GET_HTML_PATTERN, &out);
	free(rcontent);

    if (!out) {
         out = strdup("  ");
    }     

	return out;
}

int GetResult(int len, char *content, int command, char **out) {

	int  pSocket;
	struct sockaddr_in dst_addr;
	long lTimeOut = 1 * 1000;
	long lLen = 0, lAllLen = 0;

	long lSendBufLen = 0;
	long lRecvBufLen = 0;


	int ret;
	
	pSocket = socket(AF_INET,SOCK_STREAM,0);
	if(pSocket < 0)
	{
		return -1;
	}


    if (!ip[0]) {
        struct hostent	*ent = NULL;
        int ret = 0; 
        int my_err = 1; 
        char buf[4096]; 
        struct hostent host_ent, *host_ent_result = NULL; 
        ret = gethostbyname_r(host, &host_ent, buf, 4096, &host_ent_result, &my_err); 
        ent = host_ent_result;

        //if (my_err != 0) {
        //   return -1;
        //}
        if (ret != 0 || host_ent_result == NULL) {
            return -1;
        }

        struct in_addr addr;
        memcpy((char *) &addr.s_addr, ent->h_addr, sizeof(addr));
        strcpy(ip, inet_ntoa(addr));
        
        /*struct hostent  *ent = gethostbyname(host);
        if (!ent) {
            return -1;
        }
        struct in_addr addr;
        memcpy((char *) &addr.s_addr, ent->h_addr, sizeof(addr));
        strcpy(ip, inet_ntoa(addr));
        */
    }

	memset(&dst_addr,0,sizeof(dst_addr));
	dst_addr.sin_family = AF_INET;
	dst_addr.sin_port = htons(port);
	dst_addr.sin_addr.s_addr = inet_addr(ip); 


	lRecvBufLen = 10 << 10;
	lSendBufLen = 10 << 10;


	ret = setsockopt(pSocket,SOL_SOCKET ,SO_RCVTIMEO,(char *)&lTimeOut,sizeof(lTimeOut));
	ret = setsockopt(pSocket,SOL_SOCKET ,SO_SNDTIMEO,(char *)&lTimeOut,sizeof(lTimeOut));
	ret = setsockopt(pSocket,SOL_SOCKET ,SO_RCVBUF , (char *)&lRecvBufLen,sizeof(long));
	ret = setsockopt(pSocket,SOL_SOCKET ,SO_SNDBUF , (char *)&lSendBufLen,sizeof(long));


	ret = connect(pSocket,(struct sockaddr *)&dst_addr,sizeof(dst_addr));
	if(ret < 0)
	{
		close(pSocket);
		return -2;
	}
	
	lAllLen = 0;
	char pSendBuf[25] ;pSendBuf[0] = 0;
	int lSendLen = sprintf(pSendBuf, "%s%8d%8d", S_HEAD, len, command);
	do
	{
		lLen = send(pSocket, pSendBuf + lAllLen, lSendLen - lAllLen, 0);
		if(lLen > 0) lAllLen += lLen;
	}while(lLen > 0 && lAllLen < lSendLen);

	if(lAllLen != lSendLen)
	{
		close(pSocket);
		return -3;
	}

	lSendLen = len;
	lAllLen = 0;
	do
	{
		lLen = send(pSocket, content + lAllLen, lSendLen - lAllLen, 0);
		if(lLen > 0) lAllLen += lLen;
	}while(lLen > 0 && lAllLen < lSendLen);

	if(lAllLen != lSendLen)
	{
		close(pSocket);
		return -4;
	}

	char pRecvBuf[16] ;pRecvBuf[0] = 0;
	int lRecvLen = 16;
	lAllLen = 0;
	do
	{
		lLen = recv(pSocket, pRecvBuf + lAllLen, lRecvLen - lAllLen, 0);
		if(lLen > 0) lAllLen += lLen;
	}while(lLen > 0 && lAllLen < lRecvLen );
	if(lAllLen != lRecvLen)
	{
		close(pSocket);
		return -5;
	}

	if(strncmp(pRecvBuf, S_HEAD, 8) !=0) {
		close(pSocket);
		return -6;
	}

	char outlenstr[9] ;outlenstr[0] = 0;
	strncpy(outlenstr, pRecvBuf + 8, 8);
	outlenstr[8] = 0;
	lRecvLen = atoi(outlenstr);

	*out = (char *)malloc(lRecvLen + 1);
	(*out)[lRecvLen] = 0;
	lAllLen = 0;
	do
	{
		lLen = recv(pSocket, *out + lAllLen, lRecvLen - lAllLen, 0);
		if(lLen > 0) lAllLen += lLen;
	}while(lLen > 0 && lAllLen < lRecvLen );

	if(lAllLen != lRecvLen)
	{
		close(pSocket);
        free(*out);
        *out = NULL;
		return -7;
	}

	//end
	close(pSocket);
	return 0;
}

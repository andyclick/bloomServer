#ifndef _HTTP_H_031105_
#define _HTTP_H_031105_

#include        <map>
#include        <string>
using namespace std;

#include <stdlib.h>
#include <time.h>
#include "util.h"
#include "Url.h"
#include "Page.h"
#include "ic_types.h"
#include <curl/curl.h>

/*
 *    HTTP/1.0 Status Codes from:
 *     http://info.cern.ch/hypertext/WWW/Protocols/HTTP/HTRESP.html
 *  
 *    HTTP/1.1 Status Codes from:
 *     RFC 2068 "Hypertext Transfer Protocol -- HTTP/1.1"
 */
#define HTTP_ACCEPT_CONETNT(x) \
( \
    ((x) == "text/html") || \
    ((x) == "application/x-json") || \
    ((x) == "text/plain") || \
    ((x) == "application/xml") || \
    ((x) == "application/json") || \
    ((x) == "text/xml") \
)

#define HTTP_SUCCESS_STATUS(x) \
( \
    ((x) == 200) || /* Success: OK */ \
    ((x) == 201) || /* Success: CREATED */ \
    ((x) == 202) || /* Success: Accepted */ \
    ((x) == 203) || /* Success: Partial Information */ \
    ((x) == 204) || /* Success: No Response */ \
    ((x) == 205) || /* 1.1 Success: Reset Content */ \
    ((x) == 206)    /* 1.1 Success: Partial Content */ \
)

#define HTTP_REDIRECTION_STATUS(x) \
( \
    ((x) == 301) || /* Redirection: Moved */ \
    ((x) == 302) || /* Redirection: Found */ \
    ((x) == 303) || /* Redirection: Method */ \
    ((x) == 304) || /* Redirection: Not Modified */ \
    ((x) == 305) || /* 1.1 Redirection: Use Proxy */ \
    ((x) == 307)    /* 1.1 Redirection: Temporary */ \
)

#define HTTP_UNAUTHORIZED_STATUS(x) \
( \
    ((x) == 401) || /* Error: Unauthorized */ \
    ((x) == 407)    /* 1.1 Error: Proxy Authentication Required */ \
)

#define HTTP_ERROR_STATUS(x) \
( \
    ((x) == 400) || /* Error: Bad request */ \
    ((x) == 402) || /* Error: PaymentRequired */ \
    ((x) == 403) || /* Error: Forbidden */ \
    ((x) == 404) || /* Error: Not found */ \
    ((x) == 405) || /* 1.1 Error: Method Not Allowed */ \
    ((x) == 406) || /* 1.1 Error: Not Acceptable */ \
    ((x) == 408) || /* 1.1 Error: Request Timeout */ \
    ((x) == 409) || /* 1.1 Error: Conflict */ \
    ((x) == 410) || /* 1.1 Error: Gone */ \
    ((x) == 411) || /* 1.1 Error: Length Required */ \
    ((x) == 412) || /* 1.1 Error: Precondition Failed */ \
    ((x) == 413) || /* 1.1 Error: Request Entity Too Large */ \
    ((x) == 414) || /* 1.1 Error: Request-URI Too Long */ \
    ((x) == 415) || /* 1.1 Error: Unsupported Media Type */ \
    ((x) == 500) || /* Error: Internal Error */ \
    ((x) == 501) || /* Error: Not implemented */ \
    ((x) == 502) || /* Error: Service temporarily overloaded */ \
    ((x) == 503) || /* Error: Gateway timeout 503 */ \
    ((x) == 504) || /* 1.1 Error: Gateway Timeout */ \
    ((x) == 505)    /* 1.1 Error: HTTP Version Not Supported */ \
)

#define HTTP_VALID_STATUS(x) \
(  \
    HTTP_SUCCESS_STATUS(x)      || \
    HTTP_REDIRECTION_STATUS(x)  || \
    HTTP_ERROR_STATUS(x)        || \
    HTTP_UNAUTHORIZED_STATUS(x)     \
)

#define HTTP_SEPARATOR                      "\r\n"
#define HTTP_FETCH_RET_REDIRECT             -2 
#define HTTP_FETCH_RET_ERROR                -1
#define HTTP_FETCH_RET_ERROR_INVALIDHOST    -3
#define HTTP_FETCH_RET_ERROR_UNACCEPTED     -4
#define HEADER_BUF_SIZE                     4024 
#define DEFAULT_PAGE_BUF_SIZE               1024 * 40
#define MAX_PAGE_BUF_SIZE                   2 * 1024 * 1024
#define HTTP_VERSION                        "HTTP/1.1"

#define HTTP_SENG_TYPE_GET                     1 
#define HTTP_SEND_TYPE_POST                    2
class HttpProtocol
{
public:
	HttpProtocol();
	virtual ~HttpProtocol();

    int curl_fetch(CURL *curl, CUrl &u, Buffer *buff, UrlNode *urlnode, int httptimeout, int requesttype, RESPONSE_HEADER *rheader, char *downstatistic); 
    int curl_login(CURL *curl, CUrl &u, UrlNode *urlnode, int httptimeout, RESPONSE_HEADER *rheader, char *downstatistic); 
    int fetch(CUrl &u, Buffer *fileBuf, UrlNode *urlnode, Page &page, int httptimeout, int requesttype);
    int login(CUrl &u, Page &page, int httptimeout);

private:
	int read_header(int sock, char *headerPtr, int timeout);
	int createSocket(const char *host, int port, int timeout);
	int nonb_connect(int, struct sockaddr*, int timeout);
    string postRequestBuf(CUrl &u);
    string getRequestBuf(CUrl &u, UrlNode *urlnode);
    
    //int login(CUrl &u, Page &page, int httptimeout);
};

#endif /* _HTTP_H_031105_ */

#ifndef _URL_H_030728_
#define _URL_H_030728_

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "util.h"

using namespace std;


#define NNTP_DEFAULT_PORT 119

using namespace std;


enum url_scheme {
	SCHEME_HTTP,
	SCHEME_HTTPS,
	SCHEME_FTP,
	SCHEME_INVALID
};

const int DEFAULT_HTTP_PORT = 80;
const int DEFAULT_FTP_PORT  = 21;

class CUrl
{
public:
	enum url_scheme m_eScheme;	// URL scheme

	string m_sHost;		// Extracted hostname 
	string m_sUrl;			// Original URL
	int	m_sPort;		    // Port number
	string m_sPath;		// Request
	string m_sUser;		// Request
    string m_sProtocol;
    string _signature;
    string _url; //constructed url
    string _extension; //constructed url

public:
	CUrl();
	~CUrl();

	//bool ParseUrl(string strUrl);

	// break  an URL into scheme, host, port and request.
	// result as member variants
	bool ParseUrlEx(string strUrl);
	// break an URL into scheme, host, port and request.
	// result url as argvs
	void ParseUrlEx(const char *url, char *protocol, int lprotocol,
			char *host, int lhost,
			char *request, int lrequest, int *port);

	// get the ip address by host name

	bool IsValidHost(const char *ip);
    bool IsValidScheme(); 
	bool IsForeignHost(string host);
	bool IsImageUrl(string url);
	//bool IsVisitedUrl(const char *url);
	bool IsUnReachedUrl(const char *url);
	bool IsValidHostChar(char ch);
    string getUrl(); 

    string &signature();
    void parse(const string &u);
    void parse(char *u); 
    void dump();
    string &extension(); 
private:
    int DefaultPort();
    int slashes(const string &tprotocol);
    void constructURL();
    void normalizePath();
	void ParseScheme (const char *url);

};

#endif /* _URL_H_030728_ */


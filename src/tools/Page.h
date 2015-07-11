#ifndef _Page_H_030728_
#define _Page_H_030728_

#include <string>
#include <map>
#include <vector>
#include <list>
#include "Url.h"

using namespace std;

class Page
{
public:
    // plain text or other
    enum page_type {
        PLAIN_TEXT,
        OTHER	
    };

	// url & location
	string m_sUrl;		

	// header
	string m_sHeader;
	int m_nLenHeader;

	int m_nStatusCode;
	int m_nContentLength;
	string m_sLocation;
	bool m_bConnectionState;	// if "Connection: close" false, otherwise true
	string m_sContentEncoding;
	string m_sContentType;
	string m_sCharset;
	string m_sTransferEncoding;
    string m_sCookie;

	// content
	string m_sContent;
	int m_nLenContent;
	string m_sContentNoTags;


	// link, in a lash-up state
	string m_sContentLinkInfo;


	enum page_type m_eType;

	// parsed url lists
	//list<string>	m_listLink4SE;

public:
	Page();
	Page(string strUrl, string strLocation, char* header, char* body, int nLenBody);
	~Page();

	// parse header information from the header content
	void ParseHeaderInfo(string header, string url);

	// parse hyperlinks from the page content
	bool IsFilterLink(string plink);
    static void CheckUrl(char *srcurl, char *fromurl, char *outurl); 

private:
	// parse header information from the header content
	void GetStatusCode(string header);
	void GetContentLength(string header);
	void GetConnectionState(string header);
	void GetLocation(string header);
	void GetCharset(string header);
	void GetContentEncoding(string header);
	void GetContentType(string header);
	void GetTransferEncoding(string header);
    void GetCookie(string headerBuf);

	// parse hyperlinks from the web page
	bool GetContentLinkInfo();

};

#endif /* _Page_H_030728_ */


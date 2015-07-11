/*Page handling
 */

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <iterator>
#include "Url.h"
#include "Http.h"
#include "Page.h"
#include "utilcpp.h"

Page::Page()
{
	m_nStatusCode = 0;
	m_nContentLength = 0;
	m_sLocation = "";
	m_bConnectionState = false;
	m_sContentEncoding = "";
	m_sContentType = "";
	m_sCharset = "";
	m_sTransferEncoding = "";
	m_sContentLinkInfo = "";
    m_sContentNoTags = "";
    m_sCookie = "";
    m_eType = PLAIN_TEXT;
}

Page::Page( string strUrl, string strLocation, char* header, char* body, int nLenBody)
{
	//assert( header != NULL );
	//assert( body != NULL );
	//assert( nLenBody > 0 );

	// Page();
	m_nStatusCode = 0;
	m_nContentLength = 0;
	m_sLocation = "";
	m_bConnectionState = false;
	m_sContentEncoding = "";
	m_sContentType = "";
	m_sCharset = "";
	m_sTransferEncoding = "";

	m_sContentLinkInfo = "";

    m_sContentNoTags = "";
    m_eType = PLAIN_TEXT;

	m_sUrl = strUrl;
	m_sLocation = strLocation;
	m_sHeader = header;
	m_nLenHeader = strlen(header);

	m_sContent.assign(body, nLenBody);
	m_nLenContent = nLenBody;

}

Page::~Page()
{
}

void Page::ParseHeaderInfo(string strHeader, string url)
{
	m_sUrl = url;
	GetStatusCode(strHeader);
	GetContentLength(strHeader);
	GetLocation(strHeader);
	GetConnectionState(strHeader);
    GetCharset(strHeader);
    GetContentEncoding(strHeader);
    GetContentType(strHeader);
	GetTransferEncoding(strHeader);
    GetCookie(strHeader);
}

void Page::GetStatusCode(string headerBuf)
{
	toLowercase(headerBuf);

	char *charIndex = strstr((char *)headerBuf.c_str(), "http/");
	if (charIndex == NULL)
	{
		m_nStatusCode = -1;
		return;
	}

	while(*charIndex != ' '){
		charIndex++;
	}
	charIndex++;
	
	int ret = sscanf(charIndex, "%i", &m_nStatusCode);
	if (ret != 1)  m_nStatusCode = -1;
}

void Page::GetContentLength(string headerBuf)
{
	toLowercase(headerBuf);

	char *charIndex = strstr((char *)headerBuf.c_str(), "content-length");
	if (charIndex == NULL) return;

	while(*charIndex != ' '){
		charIndex++;
	}
	charIndex++;
	
	int ret = sscanf(charIndex, "%i", &m_nContentLength);
	if (ret != 1)  m_nContentLength = -1;
}

void Page::GetLocation(string headerBuf)
{
	string::size_type pre_idx,idx;
	const string delims("\r\n\t");

	string strBuf =  headerBuf;
	toLowercase(headerBuf);

	idx = headerBuf.find("location:");
	if (idx != string::npos)
	{
		pre_idx = idx + sizeof("location:") -1;
		idx = headerBuf.find_first_of(delims, pre_idx );
		if (idx != string::npos)
		{
			//m_sLocation = headerBuf.substr(pre_idx, idx - pre_idx);
			m_sLocation = strBuf.substr(pre_idx, idx - pre_idx);
		}
	}
    m_sLocation = stringstrim(m_sLocation);
    char outurl[MAX_URL_LEN]; outurl[0] = 0;
    CheckUrl((char *)m_sLocation.c_str(), (char *)m_sUrl.c_str(), outurl);
    m_sLocation = outurl;
}

void Page::GetCharset(string headerBuf)
{
	string::size_type pre_idx,idx;
	const string delims(" \",;>\r\n\t");

	toLowercase(headerBuf);

	idx = headerBuf.find("charset=");

	if (idx != string::npos)
	{
		pre_idx = idx + sizeof("charset=") -1;
		idx = headerBuf.find_first_of(delims, pre_idx );
		if(idx != string::npos){
			m_sCharset = headerBuf.substr(pre_idx, idx - pre_idx);
		}
	}
    m_sCharset = stringstrim(m_sCharset);
}

void Page::GetContentEncoding(string headerBuf)
{
	string::size_type pre_idx,idx;
	const string delims("\r\n\t");

	toLowercase(headerBuf);

	idx = headerBuf.find("content-encoding:");
	if (idx != string::npos)
	{
		pre_idx = idx + sizeof("content-encoding:") -1;
		idx = headerBuf.find_first_of(delims, pre_idx );
		if (idx != string::npos)
		{
			m_sContentEncoding = headerBuf.substr(pre_idx, idx - pre_idx);
		}
	}
    m_sContentEncoding = stringstrim(m_sContentEncoding);
}

void Page::GetConnectionState(string headerBuf)
{
	string::size_type pre_idx,idx;
	const string delims(";\r\n\t");

	toLowercase(headerBuf);

	idx = headerBuf.find("connection:");
	if (idx != string::npos)
	{
		pre_idx = idx + sizeof("connection:") -1;
		idx = headerBuf.find_first_of(delims, pre_idx );
		if (idx != string::npos)
		{
			string str = headerBuf.substr(pre_idx, idx - pre_idx);
			//cout << "Connection state: " << str << endl;
			//if (str == "close") m_bConnectionState = false;
			if (str == "keep-alive") m_bConnectionState = true;
		}
	}
}

void Page::GetContentType(string headerBuf)
{
	string::size_type pre_idx,idx;
	const string delims(";\r\n\t");

	toLowercase(headerBuf);

	idx = headerBuf.find("content-type:");
	if (idx != string::npos)
	{
		pre_idx = idx + sizeof("content-type:") -1;
		idx = headerBuf.find_first_of(delims, pre_idx );
		if (idx != string::npos)
		{
			m_sContentType = headerBuf.substr(pre_idx, idx - pre_idx);
		}
	}
    m_sContentType = stringstrim(m_sContentType);
}


void Page::GetTransferEncoding(string headerBuf)
{
	string::size_type pre_idx,idx;
	const string delims(";\r\n\t");

	toLowercase(headerBuf);

	idx = headerBuf.find("transfer-encoding:");
	if ( idx != string::npos)
	{
		pre_idx = idx + sizeof("transfer-encoding:") -1;
		idx = headerBuf.find_first_of(delims, pre_idx );
		if(idx != string::npos)
		{
			m_sTransferEncoding = headerBuf.substr(pre_idx, idx - pre_idx);
		}
	}
    m_sTransferEncoding = stringstrim(m_sTransferEncoding);
}

void Page::GetCookie(string headerBuf)
{
    string::size_type pre_idx,idx;
    const string delims("\r\n\t");

    string strBuf =  headerBuf;
    toLowercase(headerBuf);

    idx = headerBuf.find("set-cookie:");
    while (idx != string::npos)
    {
        pre_idx = idx + sizeof("set-cookie:") -1;
        idx = headerBuf.find_first_of(delims, pre_idx );
        if (idx != string::npos)
        {
            m_sCookie.append(strBuf.substr(pre_idx, idx - pre_idx));
            idx = headerBuf.find("set-cookie:",idx);
        }
    }
    m_sCookie = stringstrim(m_sCookie, NULL);
}
/*
 * Filter spam links
 * If it is, return ture; otherwise false
 */
bool Page::IsFilterLink(string plink)
{
	if( plink.empty() ) return true;
	if( plink.size() > MAX_URL_LEN ) return true;

	string link = plink, tmp;
	string::size_type idx = 0;

	
	toLowercase(plink);

	// find two times following symbols, return false
	tmp = link;
	idx = tmp.find("?");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("?");
		if( idx != string::npos ) return true;
	}

	tmp = link;
	idx = tmp.find("-");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("+");
		if( idx != string::npos ) return true;
	}

	tmp = link;
	idx = tmp.find("&");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("&");
		if( idx != string::npos ) return true;
	}

	tmp = link;
	idx = tmp.find("//");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("//");
		if( idx != string::npos ) return true;
	}

	tmp = link;
	idx = tmp.find("http");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("http");
		if( idx != string::npos ) return true;
	}

	tmp = link;
	idx = tmp.find("misc");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("misc");
		if( idx != string::npos ) return true;
	}

	tmp = link;
	idx = tmp.find("ipb");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("ipb");
		if( idx != string::npos ) return true;
	}

	const char *filter_str[]={
	"cgi-bin",	"htbin",	"linder",	"srs5",		"uin-cgi",  // robots.txt of http://www.expasy.org/
	"uhtbin",	"snapshot",	"=+",		"=-",		"script",
	"gate",		"search",	"clickfile",	"data/scop",	"names",
	"staff/",	"enter",	"user",		"mail",	"pst?",
	"find?",	"ccc?",		"fwd?",		"tcon?",	"&amp",
	"counter?",	"forum",	"cgisirsi",	"{",		"}",
	"proxy",	"login",	"00.pl?",	"sciserv.pl",	"sign.asp",
	"<",		">",		"review.asp?",	"result.asp?",	"keyword",
	"\"",		"'",		"php?s=",	"error",	"showdate",
	"niceprot.pl?",	"volue.asp?id",	".css",		".asp?month",	"prot.pl?",
	"msg.asp",	"register.asp", "database",	"reg.asp",	"qry?u",
	"p?msg",	"tj_all.asp?page", ".plot.",	"comment.php",	"nicezyme.pl?",
	"entr",		"compute-map?", "view-pdb?",	"list.cgi?",	"lists.cgi?",
	"details.pl?",	"aligner?",	"raw.pl?",	"interface.pl?","memcp.php?",
	"member.php?",	"post.php?",	"thread.php",	"bbs/",		"/bbs"
	};
	int filter_str_num = 75;

	
	for(int i=0; i<filter_str_num; i++){
		if( link.find(filter_str[i]) != string::npos)
		return true;
	}	

	return false;
}

/*****************************************************************
** Function name: GetContentLinkInfo
** Input argv:
**      --
** Output argv:
**      --
** Return:
        true: success
        false: fail
** Function Description:  Parse hyperlinks from the web page
** Version: 1.0
** Be careful:
*****************************************************************/
bool Page::GetContentLinkInfo()
{
	if( m_sContent.empty() ) return false;
	
	m_sContentLinkInfo = m_sContent;

	string& s = m_sContentLinkInfo;

	// transform all separation into one space character
	//CStrFun::ReplaceStr(s, "\t", " ");
	//CStrFun::ReplaceStr(s, "\r", " ");
	//CStrFun::ReplaceStr(s, "\n", " ");
	const string delims(" \t\r\n");
	string::size_type idx=0, pre_idx;

	while( (idx = s.find_first_of(delims, idx)) != string::npos ){
		pre_idx = idx;
		s.replace(idx,1,1,' ');
		idx++;
		while( (idx = s.find_first_of(delims, idx)) != string::npos ){
			if( idx-pre_idx == 1 ){
				s.erase(idx, 1);
			} else {
				break;
			}
		}

		idx--;
	}

	// transform all "<br>" into one space character
	replacestr(s, "<br>", " ");

	if( s.size() < 20 ) return false;

	// Keep only <img ...>, <area ...>,<script ...> and <a href ...> tags.
	string::size_type idxHref=0,idxArea=0,idxImg=0;
	string dest;

	do{
		if( s.empty() ) break;

		idxHref = findcase(s, "href");
		idxArea = findcase(s, "<area");
		idxImg = findcase(s, "<img");

		pre_idx = idxHref > idxArea? idxArea: idxHref;
		pre_idx = idxImg > pre_idx? pre_idx: idxImg;
		if( pre_idx == string::npos) break;

		s = s.substr(pre_idx);
		idx = s.find_first_of('<',1);
		if( idx != string::npos ){
			dest = dest + s.substr(0,idx);
		}else{
			break;
		}

		s = s.substr(idx);
		idxHref=0; idxArea=0; idxImg=0;
	}while(1);

	s = dest;

	
	/* erase all '\' character
	 * too avoid the following situations:
	 *      document.write("<A href=\"/~webg/refpaper/index.html\">t2</A>");
	*/
	erasestr(s, "\\");

	if( s.size() < 20 ) return false;

	return true;
}

void Page::CheckUrl(char *srcurl, char *fromurl, char *outurl) {
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
            char baseurl[MAX_URL_LEN] ; baseurl[0] = 0;
            char *tmp = strrchr(fromurl, '?');
            if (tmp) {
                strncpy(baseurl, fromurl, tmp - fromurl);
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
            char baseurl[MAX_URL_LEN] ;baseurl[0] = 0;
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

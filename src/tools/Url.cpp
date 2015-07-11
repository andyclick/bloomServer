/* URL handling
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Url.h"
#include "Http.h"
#include "util.h"
#include "utilcpp.h"

/* Is X "."?  */
#define DOTP(x) ((*(x) == '.') && (!*(x + 1)))
/* Is X ".."?  */
#define DDOTP(x) ((*(x) == '.') && (*(x + 1) == '.') && (!*(x + 2)))

struct scheme_data
{
	char *leading_string;
	int default_port;
	int enabled;
};

static map<string, char> slashCount;

/* Supported schemes: */
static struct scheme_data supported_schemes[] =
{
	{ "http://",  DEFAULT_HTTP_PORT,  1 },
	{ "https://",  DEFAULT_HTTP_PORT,  1 },
/*	{ "ftp://",   DEFAULT_FTP_PORT,   1 }, */

	/* SCHEME_INVALID */
	{ NULL,       -1,                 0 }
};

/* Returns the scheme type if the scheme is supported, or
   SCHEME_INVALID if not.  */
void CUrl::ParseScheme (const char *url)
{
	int i;

	for (i = 0; supported_schemes[i].leading_string; i++)

		if (0 == strncasecmp (url, supported_schemes[i].leading_string,
                          strlen (supported_schemes[i].leading_string))) {

			if (supported_schemes[i].enabled){
				this->m_eScheme = (enum url_scheme) i;
				return;
			}else{
				this->m_eScheme = SCHEME_INVALID;
				return;
			}
		}

	this->m_eScheme = SCHEME_INVALID;
	return;
}

/************************************************************************
 *  Function name: ParseUrlEx
 *  Input argv:
 *  	-- strUrl: url
 *  Output argv:
 *  	--
 *  Return:
   	true: success
   	false: fail
 *  Fucntion Description: break an URL into scheme, host, port and request.
 *  			result as member variants
 *  Be careful:	release the memory by the client
************************************************************************/
bool CUrl::ParseUrlEx(string strUrl)
{
	char protocol[10];
	char host[MAX_HOST_LEN];
	char request[2048];
	int port = -1;

	memset( protocol, 0, sizeof(protocol) );
	memset( host, 0, sizeof(host) );
	memset( request, 0, sizeof(request) );

	this->ParseScheme(strUrl.c_str());
	if( this->m_eScheme != SCHEME_HTTP && this->m_eScheme != SCHEME_HTTPS ){
		return false;
	}

	ParseUrlEx(strUrl.c_str(),
			protocol, sizeof(protocol),
			host, sizeof(host),
			request, sizeof(request),
			&port);

	m_sUrl  = strUrl;
	m_sHost = host;
	m_sPath = request;
    m_sProtocol = protocol;

	if( port > 0 ){
		m_sPort = port;
	}

	return true;
}

/************************************************************************
 *  Function name: ParseUrlEx
 *  Input argv:
 *  	-- url: host name
 *  	-- protocol: result protocol
 *  	-- lprotocol: protocol length
 *  	-- host: result host
 *  	-- lhost: host length
 *  	-- request: result request
 *  	-- lrequest: request length
 *  Output argv:
 *  	--
 *  Return:
   	true: success
   	false: fail
 *  Fucntion Description: break an URL into scheme, host, port and request.
 *  			result as argvs
 *  Be careful:
************************************************************************/
void CUrl::ParseUrlEx(const char *curl,
		char *protocol, int lprotocol,
		char *host, int lhost,
		char *request, int lrequest,
		int *port)
{
	char *work,*ptr,*ptr2;

	*protocol = *host = *request = 0;
	*port = 80;

    string	temp;
    const char *urp = curl;
    while (*urp)
    {
        if (*urp == ' ' && temp.length() > 0)
        {
            // Replace space character with %20 if there's more non-space
            // characters to come...
            const char *s = urp+1;
            while (*s && isspace(*s))
            s++;
            if (*s)
            temp.append("%20");
        } else if (!isspace(*urp)) {
            temp.append(string(1, *urp));
        }
        urp++;
    }
    char	*url = (char *)temp.c_str();

	int len = strlen(url);
	work = new char[len + 1];
	memset(work, 0, len+1);
	strncpy(work, url, len);

	// find protocol if any
	ptr = strchr(work, ':');
	if( ptr != NULL ){
		*(ptr++) = 0;
		strncpy( protocol, work, lprotocol );
	} else {
		strncpy( protocol, "HTTP", lprotocol );
		ptr = work;
	}


	// skip past opening /'s
	if( (*ptr=='/') && (*(ptr+1)=='/') )
		ptr+=2;

    string m_sHost(ptr);
    string::size_type slashMark = m_sHost.find('/');
    string::size_type atMark = m_sHost.find('@');
    if (slashMark != string::npos && atMark != string::npos && atMark < slashMark) {
      m_sUser = m_sHost.substr(0, atMark);
      ptr = &ptr[atMark + 1];
    }
     
	// find host
	ptr2 = ptr;
	while( IsValidHostChar(*ptr2) && *ptr2 )
		ptr2++;
	*ptr2 = 0;
	strncpy( host, ptr, lhost );

	// find the request
	int offset = ptr2 - work;
	const char *pStr = url + offset;
	strncpy( request, pStr, lrequest );

   
	// find the port number, if any
	ptr = strchr( host, ':' );
	if( ptr != NULL ){
		*ptr = 0;
		*port = atoi(ptr+1);
	}

	delete [] work;
	work = NULL;
}

/* scheme://user:pass@host[:port]... 
 *                    ^              
 * We attempt to break down the URL into the components path,
 * params, query, and fragment.  They are ordered like this:
 * scheme://host[:port][/path][;params][?query][#fragment] 
 */

/*
bool CUrl::ParseUrl(string strUrl)
{
	string::size_type idx;

	this->ParseScheme(strUrl.c_str());	
	if( this->m_eScheme != SCHEME_HTTP )
		return false;

	// get host name
	this->m_sHost = strUrl.substr(7);
	idx = m_sHost.find('/');
	if(idx != string::npos){
		m_sHost = m_sHost.substr(0,idx);
	}

	this->m_sUrl = strUrl;

	return true;
}
*/

CUrl::CUrl()
{
	this->m_sUrl = ""; 
	this->m_eScheme= SCHEME_INVALID;
        
	this->m_sHost = "";  
	this->m_sPort = DEFAULT_HTTP_PORT; 
        
	this->m_sPath = "";

}

CUrl::~CUrl()
{
}


/**********************************************************************************
 *  Function name: IsValidHostChar
 *  Input argv:
 *  	-- ch: the character for testing
 *  Output argv:
 *  	-- 
 *  Return:
   	true: is valid
   	false: is invalid
 *  Function Description: test the specified character valid
 *  			for a host name, i.e. A-Z or 0-9 or -.:
**********************************************************************************/
bool CUrl::IsValidHostChar(char ch)
{
	return( isalpha(ch) || isdigit(ch)
		|| ch=='-' || ch=='.' || ch==':' || ch=='_');
}

/**********************************************************************************
 *  Function name: IsValidHost
 *  Input argv:
 *  	-- ch: the character for testing
 *  Output argv:
 *  	-- 
 *  Return:
   	true: is valid
   	false: is invalid
 *  Function Description: test the specified character valid
 *  			for a host name, i.e. A-Z or 0-9 or -.:
 *  Be careful:
**********************************************************************************/
bool CUrl::IsValidHost(const char *host)
{
	if( !host ){
		return false;
	}

	if( strlen(host) < 6 ){ // in case host like "www", "pku", etc.
		return false;
	}

	char ch;
	for(unsigned int i=0; i<strlen(host); i++){
		ch = *(host++);
		if( !IsValidHostChar(ch) ){
			return false;
		}
	}

	return true;
}

/*
 * If it is, return true; otherwise false
 * not very precise
 */
bool CUrl::IsForeignHost(string host)
{
	if( host.empty() ) return true;
	if( host.size() > MAX_HOST_LEN ) return true;

	 string::size_type inaddr = 0;

	inaddr = (unsigned long)inet_addr( host.c_str() );
	if ( inaddr != INADDR_NONE){ // host is just ip
		return false;
	}

	string::size_type idx = host.rfind('.');
	string tmp;
	if( idx != string::npos ){
		tmp = host.substr(idx+1);
	}

	( tmp, tmp.size() );
	const char *homem_sHost[] ={
		"cn","com","net","org","info",
		"biz","tv","cc", "hk", "tw"
	};

	int homem_sHost_num = 10;

	for(int i=0; i<homem_sHost_num; i++){
		if( tmp == homem_sHost[i] )
			return false;
	}

	return true;
}
	
	
bool CUrl::IsImageUrl(string url)
{
	if( url.empty() ) return false;
	if( url.size() > MAX_HOST_LEN ) return false;

	string::size_type idx = url.rfind('.');
	string tmp;
	if( idx != string::npos ){
		tmp = url.substr(idx+1);
	}

	toLowercase(tmp);
	const char *image_type[] ={
		"gif","jpg","jpeg","png","bmp",
		"tif","psd"
	};

	int image_type_num = 7;

	for (int i=0; i<image_type_num; i++)
	{
		if( tmp == image_type[i] )
			return true;
	}

	return false;
}
//must call parseUrlEx first
string &CUrl::signature()
{
    if (_signature.length() == 0) {
        _signature.append(m_sProtocol);
        _signature.append("://");
        if (m_sUser.length())
            _signature.append(m_sUser).append("@");
        _signature.append(m_sHost);

        char portstr[10] ;portstr[0] = 0;
        int2str(m_sPort, portstr, 10, 0);
        _signature.append(":").append(portstr).append("/");
    }
    return _signature;
}
//must call parseUrlEx first
/*void CUrl::constructUrl()
{
    string	temp;
    const char *urp = m_sUrl.c_str();
    while (*urp)
    {
        if (*urp == ' ' && temp.length() > 0)
        {
            // Replace space character with %20 if there's more non-space
            // characters to come...
            const char *s = urp+1;
            while (*s && isspace(*s))
            s++;
            if (*s)
            temp.append("%20");
        } else if (!isspace(*urp)) {
            temp.append(string(1, *urp));
        }
        urp++;
    }
    m_sUrl = temp;
}
*/
/*
 * must call parse first
 */
bool CUrl::IsValidScheme() {
	ParseScheme(m_sProtocol.c_str());
	if(this->m_eScheme != SCHEME_HTTP && this->m_eScheme != SCHEME_HTTPS) {
		return false;
	}
    return true;
}

void CUrl::parse(char *u) 
{
    string url = u;
    parse(url);
}

void CUrl::parse(const string &urlToParse)
{
    if (urlToParse.length() > MAX_URL_LEN) {
        return;
    }

    m_sUrl = urlToParse;
    string u = urlToParse; 
    int  allowspace = 1;
    string	temp;
    //don't open it
    //toLowercase(u);
    const char *urp = u.c_str();
    char *t = NULL;
    while (*urp)
    {
	if (*urp == ' ' && temp.length() > 0 && allowspace)
	{
	    // Replace space character with %20 if there's more non-space
	    // characters to come...
	    const char *s = urp+1;
	    while (*s && isspace(*s))
		s++;
	    if (*s)
		temp.append("%20");
	}
	else
	    temp.append(1, *urp);
	urp++;
    }
    char	*nurl = (char *)temp.c_str();

    //
    // Ignore any part of the URL that follows the '#' since this is just
    // an index into a document.
    //
    char	*p = strchr(nurl, '#');
    if (p)
	*p = '\0';

    // Some members need to be reset.  If not, the caller would
    // have used URL::URL(char *ref, URL &parent)
    // (which may call us, if the URL is found to be absolute).

    //check if schema is valid
	ParseScheme(nurl);
	if(this->m_eScheme != SCHEME_HTTP && this->m_eScheme != SCHEME_HTTPS) {
		return; 
	}

    //
    // Extract the service
    //
    //
    char *buff = NULL;
    p = strchr(nurl, ':');
    if (p)
    {
        t = strtok_r(nurl, ":", &buff);
        if (t != NULL) {
            m_sProtocol.assign(t);
        }
        p = strtok_r(0, "\n", &buff);
    }
    else
    {
        m_sProtocol = "http";
        p = strtok_r(nurl, "\n", &buff);
    }

    toLowercase(m_sProtocol);

    //
    // Extract the host
    //
    if (!p || strncmp(p, "//", 2) != 0)
    {
	// No host specified, it's all a path.
	m_sPort = 0;
	if (p)		// if non-NULL, skip (some) leading slashes in path
	{
	    int i;
	    for (i = slashes (m_sProtocol); i > 0 && *p == '/'; i--)
		p++;
	    if (i)	// if fewer slashes than specified for protocol don't
			// delete any. -> Backwards compatible (necessary??)
		p -= slashes (m_sProtocol) - i;
	}
	m_sPath = p;
	if (strcmp((char*)m_sProtocol.c_str(), "file") == 0 || slashes (m_sProtocol) < 2)
	  m_sHost = "localhost";
    }
    else
    {
        p += 2;

        //
        // p now points to the host
        //
        char	*q = strchr(p, ':');
        char	*slash = strchr(p, '/');
        char	*at = strchr(p, '@');
        m_sPath = "/";
        //cout << " at = " << at << " slash = " << slash << endl;
        if (strcmp((char*)m_sProtocol.c_str(), "file") == 0)
        {
            // These should be of the form file:/// (i.e. no host)
            // if there is a file://host/path then strip the host
            if (strncmp(p, "/", 1) != 0) {
                p = strtok_r(p, "/", &buff);
                t = strtok_r(0, "\n", &buff);
                if (t) {
                    m_sProtocol.append(t);
                }
            } else {
                t = strtok_r(p+1, "\n", &buff);
                if (t) {
                    m_sProtocol.append(t);	// m_sPath is "/" - don't double
                }
            }
            m_sHost = "localhost";
            m_sPort = 0;
        }
        //else if (at && ((slash && slash > q) || !slash))
        else if (at && ((slash && slash > q) || !slash))
        {
            if ((at < slash || !slash)) {
            //if ((at < slash) && at != NULL) {
                m_sUser.assign(p, (at - p) / sizeof(char));
                p = at + 1;
            }
            size_t len = strlen(p);
            t = strtok_r(p, ":", &buff);

            if (t != NULL && len != strlen(p)) {  //find port
                m_sHost = t;
                p = strtok_r(0, "/", &buff);
                if (p)
                    m_sPort = atoi(p);
                if (!p || m_sPort <= 0)
                    m_sPort = DefaultPort();

            } else {
                t = strtok_r(p, "/", &buff);
                if (t) {
                    m_sHost = t;
                }
            }
            //
            // The rest of the input string is the path.
            //
            t = strtok_r(0, "\n", &buff);
            if (t != NULL) {
                m_sPath.append(t);
            }
        } else {
            char *t = strchr(p, ':');
            if (t != NULL && t < slash) {
                t = strtok_r(p, ":", &buff);
                if (t != NULL) {  //find port
                    m_sHost = t;
                    p = strtok_r(0, "/", &buff);
                    if (p)
                        m_sPort = atoi(p);
                    if (!p || m_sPort <= 0)
                        m_sPort = DefaultPort();
                } else {
                    t = strtok_r(p, "/", &buff);
                    if (t) 
                        m_sHost = t;
                    m_sPort = DefaultPort();
                }
            } else {
                t = strtok_r(p, "/", &buff);
                if (t) 
                    m_sHost = t;
                m_sPort = DefaultPort();
            }
            

            /*t = strtok_r(p, "/", &buff);
            if (t != NULL) {
                m_sHost = t;
                chop(m_sHost, " \t");
            }
            m_sPort = DefaultPort();
            */
            

            //
            // The rest of the input string is the path.
            //
            t = strtok_r(0, "\n", &buff);
            if (t != NULL) {
                m_sPath.append(t);
            }

        }

        // Check to see if host contains a user@ portion
        /*int atMark = m_sHost.find('@');
        if (atMark != -1) {
            m_sUser = m_sHost.substr(0, atMark);
            m_sHost = m_sHost.substr(atMark + 1);
        }
        */
    }

    //
    // Get rid of loop-causing constructs in the path
    //
    normalizePath();

    //
    // Build the url.  (Note, the host name has NOT been normalized!)
    //
    constructURL();
}

void CUrl::constructURL()
{
    if (strcmp((char*)m_sProtocol.c_str(), "file") != 0 && m_sHost.length() == 0) {
        _url = "";
        return;
    }

    _url.assign(m_sProtocol);
    _url.append(":");

    // Add correct number of slashes after service name
    int i;
    for (i = slashes (m_sProtocol); i > 0; i--)
    {
        _url.append("/");
    }

    if (slashes (m_sProtocol) == 2)	// services specifying a particular
    {					            // IP host must begin "service://"
        if (strcmp((char*)m_sProtocol.c_str(), "file") != 0)
        {
            if (m_sUser.length())
                _url.append(m_sUser).append(1, '@');
            _url.append(m_sHost);
        }

        if (m_sPort != DefaultPort() && m_sPort != 0)  {// Different than the default port
            char portstr[10] ;portstr[0] = 0;
            int2str(m_sPort, portstr, 10, 0);
            _url.append(1, ':').append(portstr);
        }
    }
    _url.append(m_sPath);
}

int CUrl::DefaultPort()
{
   if (strcmp((char*)m_sProtocol.c_str(), "http") == 0)
      return 80;
   else if (strcmp((char*)m_sProtocol.c_str(), "https") == 0)
      return 443;
   else if (strcmp((char*)m_sProtocol.c_str(), "ftp") == 0)
      return 21;
   else if (strcmp((char*)m_sProtocol.c_str(), "gopher") == 0)
      return 70;
   else if (strcmp((char*)m_sProtocol.c_str(), "file") == 0)
      return 0;
   else if (strcmp((char*)m_sProtocol.c_str(), "news") == 0)
      return NNTP_DEFAULT_PORT;
   else return 80;
}

void CUrl::normalizePath()
{
    //
    // Rewrite the path to be the minimal.
    // Remove "//", "/../" and "/./" components
    //

     string::size_type i, limit;
    int	leadingdotdot = 0;
    string	newPath;
    string::size_type pathend = m_sPath.find('?');	// Don't mess up query strings.
    if (pathend == string::npos)
        pathend = m_sPath.length();

    //
    // get rid of "//" first, or "/foo//../" will become "/foo/" not "/"
    // Some database lookups interpret empty paths (// != /), so give
    // the use the option to turn this off.
    //
    if (1) // don't allow double slashes
    {
        while ((i = m_sPath.find("//")) != string::npos && i < pathend)
        {
            newPath = m_sPath.substr(0, i).c_str();
            newPath.append(m_sPath.substr(i + 1));
            m_sPath = newPath.c_str();
            pathend = m_sPath.find('?');
            if (pathend < 0)
                pathend = m_sPath.length();
        }
    }

    //
    // Next get rid of redundant "/./".  This could cause infinite
    // loops.  Moreover, "/foo/./../" should become "/", not "/foo/"
    //
    while ((i = m_sPath.find("/./")) != string::npos && i < pathend)
    {
        newPath = m_sPath.substr(0, i).c_str();
        newPath.append(m_sPath.substr(i + 2));
        m_sPath = newPath.c_str();
        pathend = m_sPath.find('?');
        if (pathend < 0)
            pathend = m_sPath.length();
    }
    if ((i = m_sPath.find("/.")) != string::npos && i == pathend-2)
    {
        newPath = m_sPath.substr(0, i+1).c_str();		// keep trailing slash
        newPath.append(m_sPath.substr(i + 2));
        m_sPath = newPath.c_str();
        pathend--;
    }

    //
    // Now that "empty" path components are gone, remove ("/../").
    //
    while ((i = m_sPath.find("/../")) != string::npos && i < pathend)
    {
        if (i > 0 && (limit = m_sPath.find_last_of('/', i - 1)) >= 0)
        {
            newPath = m_sPath.substr(0, limit).c_str();
            newPath.append(m_sPath.substr(i + 3));
            m_sPath = newPath.c_str();
        }
        else
        {
            m_sPath = m_sPath.substr(i + 3).c_str();
            leadingdotdot++;
        }
        pathend = m_sPath.find('?');
        if (pathend < 0)
            pathend = m_sPath.length();
    }
    if ((i = m_sPath.find("/..")) != string::npos && i == pathend - 3)
    {
        if (i > 0 && (limit = m_sPath.find_last_of('/', i - 1)) >= 0)
            newPath = m_sPath.substr(0, limit+1).c_str();	// keep trailing slash
        else
        {
            newPath = '/';
            leadingdotdot++;
        }
        newPath.append(m_sPath.substr(i + 3));
        m_sPath = newPath;
        pathend = m_sPath.find('?');
        if (pathend < 0)
            pathend = m_sPath.length();
    }
    // The RFC gives us a choice of what to do when we have .. left and
    // we're at the top level. By principle of least surprise, we'll just
    // toss any "leftovers" Otherwise, we'd have a loop here to add them.

    // Finally change all "%7E" to "~" for sanity
    while ((i = m_sPath.find("%7E")) != string::npos && i < pathend)
    {
        newPath = m_sPath.substr(0, i).c_str();
        newPath.append("~");
        newPath.append(m_sPath.substr(i + 3));
        m_sPath = newPath;
        pathend = m_sPath.find('?');
        if (pathend < 0)
            pathend = m_sPath.length();
    }

    // If the server *isn't* case sensitive, we want to lowercase the path
    /*if (0)
      toLowercase(m_sPath);
      */

    // And don't forget to remove index.html or similar file.
    // if (strcmp((char*)m_sProtocal, "file") != 0)  (check is now internal)
	//removeIndex(m_sPath, m_sProtocol);
}

//*****************************************************************************
// int CUrl::slash(const String &protocol)
// Returns number of slashes folowing the service name for protocol
//
int
CUrl::slashes(const string &tprotocol)
{
    if (!slashCount.size())
    {

        slashCount["mailto"] =  0;
        slashCount["news"] =  0;
        slashCount["http"] =  2;
        slashCount["ftp"] =  2;
        slashCount["file"] =  2;
    }
    map<string, char>::iterator iter; 
    if ((iter = slashCount.find(tprotocol)) != slashCount.end()) {
        return iter->second; 
    } else {
        return 2;
    }
}

string &CUrl::extension() {
    if (_extension.empty()) {
        if (!m_sPath.empty()) {
            unsigned int begin = 0;
            if ((begin = m_sPath.find_last_of(".")) != string::npos) {
                _extension = m_sPath.substr(begin);
            }
        }
    }
    return _extension;
}

void CUrl::dump()
{
    cout << "original = " << m_sUrl << endl;
    cout << "service = " << m_sProtocol << endl;
    cout << "user = " << m_sUser << endl;
    cout << "host = " << m_sHost << endl;
    cout << "port = " << m_sPort << endl;
    cout << "path = " << m_sPath << endl;
    cout << "url = " << _url << endl;
}
string CUrl::getUrl() {
    return _url;
}

#include <ctype.h>
#include "utilcpp.h"
#include "FSBigFile.h"
#include "ic_types.h"
#include "InfoCrawler.h"
#include "iconv.h"

static set<string> charsets;
void toLowercase(string &str) {
    char *s = NULL;
    for (size_t i = 0; i < str.length(); i++) {
        s = &str[i];
        if (s != NULL) {
            *s = tolower((unsigned char) *s);
        }
    }
}

void replacestr(string &str, string srstr, string dsstr)
{
    if( str.size() ==0 || srstr.size() == 0 )
        return;

    string::size_type idx = 0;
    string::size_type sub_length = srstr.length();
    idx = str.find(srstr,idx);
    while( idx != string::npos ) {
        str.replace(idx,sub_length,dsstr);

        if( idx+dsstr.size() > str.size() ) break;

        idx = str.find(srstr,idx+dsstr.size());
    }
}

bool _nocase_compare(char c1, char c2)
{   
    return toupper(c1) == toupper(c2);
}

string::size_type findcase(string haystack, string needle)
{
	if (haystack.empty()) return string::npos;
	if (needle.empty()) return string::npos;

	string::iterator pos;
	pos = search(haystack.begin(), haystack.end(),
			needle.begin(),needle.end(), _nocase_compare);

	if( pos == haystack.end()) {
		return string::npos;
	} else {
		return (pos - haystack.begin());
	}
}

void erasestr(string &str , string substr)
{
    if( str.size() == 0 || substr.size() == 0 )
        return; 

    string::size_type idx = 0;
    string::size_type sub_length = substr.length();
    idx = str.find(substr,idx);
    while( idx != string::npos ) {
        str.erase(idx,sub_length);
        idx = str.find(substr,idx);
    }  
}

int WriteContent(FSBigFile *bigfile, char *key, int keylen, char *data, int datalen) {
    int ret = 0;
    ret += bigfile->write((char *)&keylen, 4); 
    ret += bigfile->write(key, keylen); 
    ret += bigfile->write((char *)&datalen, 4); 
    ret += bigfile->write(data, datalen); 
    return ret;
}
int getlines(FSBigFile *logfile, char* p) 
{
    char *c = (char *)calloc(1, 2);
    int eret = 0; 
    int n = 0;
    int num = 1;
    eret =logfile->read(c,num);
    if (eret != 0 || num == 0)
    {
        free(c);
        return -1;
    }
    while(eret == 0 && c[0] != '\n' && num != 0 && n < 100000)
    {
        p[n] = c[0];  
        n++; 
        eret =logfile->read(c,num);
    }
    p[n] = 0; 
    free(c);
    return n; 
}
int ReadContent(FSBigFile *bigfile, char **key, int *keylen, char **data, int *datalen) {
    int ret = 0;
    int size = 4;
    ret = bigfile->read((char *)keylen, size); 
    if (ret < 0) {
        return -1;
    }
    *key = (char *)malloc(*keylen + 1);
    ret = bigfile->read(*key, *keylen); 
    (*key)[*keylen] = 0;
    if (ret < 0) {
        return -1;
    }
    size = 4;
    ret = bigfile->read((char *)datalen, size); 
    if (ret < 0) {
        return -1;
    }
    *data = (char *)malloc(*datalen + 1);
    ret = bigfile->read(*data, *datalen); 
    (*data)[*datalen] = 0;
    if (ret < 0 && ret != RC_BF_EOF) {
        return -1;
    } else {
        ret = 0;
    }
    return ret;

}

int openConnect(MyConnection &connection, string &servername, int port, 
    int timeout, int retries) {
    connection.Assign_Server((char *)servername.c_str());
    connection.Assign_Port(port);
    connection.Timeout(timeout);
    connection.Retries(retries);

    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    if(connection.IsOpen() && connection.IsConnected()) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Already open and connection is up %s(%d) - %s:%s:%d", servername.c_str(), port,INFO_LOG_SUFFIX);
        return -1; // Already open and connection is up
    }

    // No open connection
    // Let's open a new one
    //
    if(connection.Open() == NOTOK) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Cannot open connection %s(%d) - %s:%s:%d", servername.c_str(), port,INFO_LOG_SUFFIX);
        return 0; // failed
    }
    return 1;
}

int connectServer(MyConnection &connection) {

    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    if (connection.IsConnected()) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Already open and connection is up %s(%d) - %s:%s:%d", connection.Get_Server().c_str(), connection.getPort(),INFO_LOG_SUFFIX);
        return -1; // Already connected
    }

    if (connection.Connect() == NOTOK) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Cannot connect %s(%d) - %s:%s:%d", connection.Get_Server().c_str(), connection.getPort(),INFO_LOG_SUFFIX);
        return 0;  // Connection failed
    }

    return 1;    // Connected
}

string stringstrim(string &src, const char *trim) {
    string ret;
    char *tmp = strdup((char *)src.c_str());
    char *org = tmp;
    tmp = strtrim(org, trim);
    ret = tmp;
    free(org);
    return ret;
}
int initlocalip(vector <string > & localips , char * localip)
{
    char *pToken = NULL;
    char *tokbuff = NULL;
    int ret;
    pToken = strtok_r(localip, ",",&tokbuff);
    int i = 0; 
    while(pToken)
    {
        localips.push_back(pToken);
        pToken = strtok_r(NULL, ",",&tokbuff);
        i++;
    }
    ret = i;
    return ret;
}
bool szReplaceReply(char* szStr,const char* szSrc,const char* szDst)
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
bool szReplaceReply(char* szStr,const char* szSrc,int intDst)
{
    char szDst[64] ;szDst[0] = 0;
    sprintf(szDst, "%d",intDst);
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
void charreplace(char *str, char oldchar, char newchar) {
    int len = strlen(str);
    for(int i = 0; i < len; i++) {
        if (str[i] == oldchar) {
            str[i] = newchar;
        }
    }
}

void charreplace(char *str, int length, char oldchar, char newchar) {
    for(int i = 0; i < length; i++) {
        if (str[i] == oldchar) {
            str[i] = newchar;
        }
    }
}
string GetEncoding(char *html, size_t len, string &charsetByGiven)
{
    string retforheader;
    string retforhtml;

    string newcharsetByGiven = stringstrim(charsetByGiven, NULL);

    retforhtml = _GetEncoding(html , len);
    retforheader = newcharsetByGiven;

    /*if (retforhtml.empty()) {
     * retforhtml = "gb18030";
     * }
     *
     * if (retforheader.empty()) {
     * retforheader = "gb18030";
     * }*/

    toLowercase(retforhtml);
    toLowercase(retforheader);

    retforhtml = ValidateCharsets(retforhtml);
    retforheader = ValidateCharsets(retforheader);


    if (retforhtml.compare("gb2312") == 0 || retforhtml.compare("gbk") == 0 || retforhtml.compare("cn") == 0) {
        retforhtml = "gb18030";
    }

    if (retforheader.compare("gb2312") == 0 || retforheader.compare("gbk") == 0 || retforheader.compare("cn") == 0) {
        retforheader = "gb18030";
    }

    string ret;

    if (retforhtml == "gb18030" || retforhtml == "utf-8" || retforhtml == "iso8859-1") {
        ret = retforhtml;
    } else if (retforheader == "gb18030" || retforheader == "utf-8" || retforheader == "iso8859-1") {
        ret = retforheader;
    } else if (retforhtml.length() >= retforheader.length()) {
        ret = retforhtml;
    } else {
        ret = retforheader;
    }

    if (ret.empty()) {
        ret = "gb18030";
    }

    return ret;
}
string _GetEncoding(const char *html, int len) {
    char *pos = (char *)html;
    char *endpos = pos + 1024;
    char *finalpos = (char *)html + len;
    if (len <= 1024) {
        endpos = (char *)html + len;
        string ret = _GetEncoding1(pos, endpos - pos);
        if (!ret.empty()) {
            return ret;
        }
    } else {
        while(endpos < finalpos) {
            endpos = strchr(endpos, '>');
            if (!endpos) endpos = (char *)html + len;
            string ret = _GetEncoding1(pos, endpos - pos);
            if (!ret.empty()) {
                return ret;
            }
            endpos++;
            pos = endpos;
        }
    }
    return "";
}
string ValidateCharsets(string charset) {
    set<string>::iterator iter;
    if (charset.length() > 64) {
        charset = charset.substr(0, 64);
    }

    if ((iter = charsets.find(charset)) != charsets.end()) {
        return charset;
    } else {
        /*double mindistance = 10000;
         * CDistance m_Distance;
         * string ret;
         * for(iter = charsets.begin(); iter != charsets.end(); ++iter) {
         * double distance = m_Distance.LD((char *)iter->c_str(), (char *)charset.c_str());
         * if (mindistance > distance) {
         * mindistance = distance;
         * ret = *iter;
         * }
         * }*/
        string ret;     
        return ret; 
    }
}
string _GetEncoding1(char *html, int len) {
    string ret;
    char *newhtml = (char *)malloc(len + 1);
    toLowerCase(newhtml, (char *)html, len);
    newhtml[len] = 0;
    char *beginpos = strstr(newhtml, "<meta");
    if (!beginpos) {
        free(newhtml);
        return ret;
    }
    beginpos = beginpos + 5;
    char *endpos = strchr(beginpos, '>');
    if (!endpos) {
        free(newhtml);
        return ret;
    }
    *endpos = 0;

    map<string, string> attributes;

    char *savepos = NULL;
    while (beginpos < endpos) {
        char *tmp = strtok_r(beginpos, "=\"\' ;\t", &savepos);
        int i = 0;
        char *key = NULL;
        char *value = NULL;
        while(tmp) {
            if (i % 2 == 0) {
                key = strtrim(tmp, "\r\n");
                value = NULL;
            } else {
                value = strtrim(tmp, "\r\n");
                if (key) {
                    attributes[key] = value;
                }
                if (strcmp(value, "charset") == 0) {
                    i++;
                    key = value;
                } else {
                    key = NULL;
                }
            }
            tmp = strtok_r(NULL, "=\"\' ;\t", &savepos);
            i++;
        }
        beginpos = strstr(endpos + 1, "<meta");
        if (!beginpos) break;
        beginpos = beginpos + 5;
        endpos = strchr(beginpos, '>');
        if (!endpos) break;
        *endpos = 0;
    }

    free(newhtml);

    map<string, string>::iterator iter;
    if ((iter = attributes.find("http-equiv")) != attributes.end()) {
        if ((iter = attributes.find("content")) != attributes.end()) {
            if ((iter = attributes.find("charset")) != attributes.end()) {
                ret = iter->second; 
            }
        }
    }
    return ret;
}
void toLowerCase(char *des, char *src, size_t len) {
    char distance='A'-'a' ;

    for( int i=0 ; i < len; i++ ) {
        if (src[i]>='A' && src[i]<='Z') {
            des[i] = src[i] - distance ;
        } else {
            des[i] = src[i];
        }
    }
}
int tog(char *inputCharset, char * outputCharset , const char *inbuf, unsigned int inlen, char *outbuf, unsigned int &outlen )
{
    return code_convert(inputCharset, outputCharset, (char *)inbuf, inlen, outbuf, outlen);
}

int tog( string &str ,char *inputCharset, char *outputCharset)
{
    int nLengthofIn = str.size();

    if ( nLengthofIn == 0 )
        return 0;

    unsigned int nLengthlfOut = nLengthofIn * 4 + 1;
    unsigned int nLengthlfOutTmp = nLengthlfOut;
    char *output = new char[nLengthlfOut];
    tog(inputCharset,outputCharset,(char *)str.c_str(), nLengthofIn, output, nLengthlfOutTmp);
    output[nLengthlfOut - nLengthlfOutTmp] = 0;

    str.assign(output);
    delete []output;
    return 0;
}
char *tog( char* str, char *inputCharset ,char * outputCharset)
{
    int nLengthofIn = strlen(str);

    if ( nLengthofIn == 0 )
    {
        char *output = new char[1];
        output[0] = '\0';
        return output;
    }

    unsigned int nLengthlfOut = nLengthofIn * 4 + 1;
    unsigned int nLengthlfOutTmp = nLengthlfOut;
    char *output = new char[nLengthlfOut];
    tog(inputCharset, outputCharset,str, nLengthofIn, output, nLengthlfOutTmp);
    output[nLengthlfOut - nLengthlfOutTmp] = 0;

    return output;
}
int code_convert(const char *from_charset,const char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
{
        iconv_t cd;
        int rc;
        char **pin = &inbuf;
        char **pout = &outbuf;
        cd = iconv_open(to_charset,from_charset);
        if (cd==0) return -1;

        int argument = 1;
        iconvctl(cd, ICONV_SET_DISCARD_ILSEQ, &argument);

        memset(outbuf,0,outlen);
        size_t inlen1=(size_t)inlen;
        size_t outlen1=(size_t)outlen;
        if (iconv(cd,pin,&inlen1,pout,&outlen1)==-1) return -1;
        iconv_close(cd);
        return 0;
}
void InitialCharsets() {
    for(int i = 0; i < 341; i++) {
        if (charsetpool_contents[i][0]) {
            charsets.insert(charsetpool_contents[i]);
        }
    }
}

bool szReplace(char* szStr,const char* szSrc,const char* szDst)
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


int serializeLong(char *&buf, long a) {
    int len = 8;
    memcpy(buf, &len, 4); 
    buf += 4;
    if (sizeof(long) == 4) {
        long long a1 = a;
        memcpy(buf, &a1, len); 
    } else {
        memcpy(buf, &a, len); 
    }
    buf += len;
    return 4 + len;
}

int unserializeLong(char *&buf, long &a) {
    int len = 0;
    memcpy(&len, buf, 4);
    buf += 4;
    if (sizeof(long) == 4) {
        long long a1;
        memcpy(&a1, buf, len); 
        a = a1;
    } else {
        memcpy(&a, buf, len);
    }
    buf += len;
    return len;
}

int serializeULongLong(char *&buf, ulonglong a) {
    int len = 8;
    memcpy(buf, &len, 4); 
    buf += 4;
    memcpy(buf, &a, len); 
    buf += len;
    return 4 + len;
}

int unserializeULongLong(char *&buf, ulonglong &a) {
    int len = 0;
    memcpy(&len, buf, 4); 
    buf += 4;
    memcpy(&a, buf, len);
    buf += len;
    return len;
}

int serializeInt(char *&buf, int a) {
    int len = 4;
    memcpy(buf, &len, 4); 
    buf += 4;
    memcpy(buf, &a, len); 
    buf += len;
    return 4 + len;
}

int unserializeInt(char *&buf, int &a) {
    int len = 0;
    memcpy(&len, buf, 4); 
    buf += 4;
    memcpy(&a, buf,len); 
    buf += len;
    return len;
}

int serializeString(char *&buf, char *a) {
    int len = strlen(a); 
    memcpy(buf, &len, 4); 
    buf += 4;
    memcpy(buf, a, len); 
    buf += len;
    return 4 + len;
}

int unserializeString(char *&buf, char *a) {
    int len = 0;
    memcpy(&len, buf, 4);
    buf += 4;
    memcpy(a, buf, len); 
    a[len] = 0;
    buf += len;
    return len;
}

int serializeChar(char *&buf, char a) {
    int len = 1; 
    memcpy(buf, &len, 4); 
    buf += 4;
    memcpy(buf, &a, len); 
    buf += len;
    return 4 + len;
}

int unserializeChar(char *&buf, char &a) {
    int len = 0; 
    memcpy(&len, buf, 4); 
    buf += 4;
    memcpy(&a, buf, len); 
    buf += len;
    return len;
}

int serializeBool(char *&buf, bool a) {
    int len = 1;
    memcpy(buf, &len, 4);
    buf += 4;
    char a1 = 0;
    if (a) {
        a1 = 1;
    }
    memcpy(buf, &a1, len);
    buf += len;
    return 4 + len;
}

int unserializeBool(char *&buf, bool &a) {
    int len = 0; 
    memcpy(&len, buf, 4);
    buf += 4;
    char a1 = 0;
    memcpy(&a, buf, len);
    buf += len;
    if (a1 == 0) {
        a = false;
    } else {
        a = true;
    }
    return len;
}



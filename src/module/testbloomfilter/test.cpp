#include <iostream>
#include <time.h>
#include <string.h>

#include <mybase.h>
/*#include "bloomfilter/bloom_filter.h"
#include "bloomfilter/murmur.h"
*/
//#include "url_util/util.h"
#include "Time.h"
#include "SiteSourceData.h"
#include "TServerContentDataNet.h"
#include "b_server_common_define.h"
#include "uri.h"
#include "hlink.h"
#include "HtRegex.h"
#include "UrlAnalyzer.h"
#include <boost/regex.hpp>
#include "UrlManager.h" 

//using namespace mybase;
using namespace octopus::common;

void get_time_of_msec();
void get_time_of_nsec();
void test_string_find_last();
string my_find_last_of(string mystr, const char * str, int num);
void test_strtok();
void get_ip_port(string &ip, int16_t &port , char * buf);
void test_map();
void foo(void); 
void foo1(); 
void list_merge();
void test_bloomfilter(); 
void test_parse();
int  onfind(const char *elem, const char *attr, struct uri *uri, void *arg); 
void extract_url();
void delete_urls(list<UrlNode *> &urls);
void test_inf();
void test_json_unescape();
int16_t json_unescape_str(string & content);
string  regexreplace(string  str, char * src, char *desc);
void test_zookeeper();

struct onfindpackage
{   
  void *manager;
  UrlNode *node;
  list<UrlNode *> *urls;
};

typedef struct arynode_
{
  arynode_(int a ,int b)
  {
    a = a;
    b = b;
  };
  int a ;
  int b ;
}arynode;

class Test {
public:
  Test() {
    //nnn = NULL;
  };
  ~Test() {};
public:
  static char *nnn;
  //void print() { nnn = NULL; };
};

char *Test::nnn = NULL;

int main(int argc, char **argv) 
{
  //test_inf();
  //HbaseTest();
  for(int i = 0; i<10; i++)
  {
    extract_url();
  }
  return 0;
  //printf("%d", sizeof(int64_t));
}

void test_contnet()
{
  FILE *f = fopen("1.txt", "rb");
  if (f)
  {
    fseek(f,0,SEEK_END);
    int32_t filelen = ftell(f);
    rewind(f);
    char * urldata = NULL;
    urldata = new char [filelen + 1];
    fread(urldata, 1, filelen, f);
    urldata[filelen]=0;

    std::stringstream  content_stream;
    content_stream.write(urldata, filelen);
    boost::archive::text_iarchive ia(content_stream);
    octopus::common::TServerContentDataNet content;
    ia >> content;

    if (urldata) delete urldata;

    std::deque<octopus::common::ContentToSave>::iterator iter;
    std::deque<octopus::common::ContentToSave> contents;
    contents = content.get_contents();
    iter = contents.begin();
    size_t size = contents.size();
    for(iter = contents.begin(); iter != contents.end(); ++iter) {
      FILE *f = fopen("tests.txt", "wb+");
      if (f) {
        char *tmp = NULL; 
        size_t tmplen = 0;                                           
        inf((char *)iter->content.c_str(), iter->content.length(), &tmp, &tmplen);
        fprintf(f, "url = %s, content = %s\n", iter->url_node->url.c_str(), tmp);
        fclose(f);
      }
      delete iter->url_node;
    }

    fclose(f);

  }
}
void list_merge()
{
  arynode ary[] = {arynode(1,1),arynode(2,2),arynode(3,3),arynode(4,4),arynode(5,5),arynode(6,6),arynode(7,7),arynode(8,8)};
  list<arynode> list1(ary+4,ary+8);
  list<arynode> list2(ary,ary+4);

  list<arynode>::iterator iter;
  /*  for(iter = list.begin();iter != list1.end();iter++)
      {
      cout <<iter<<endl;
      }*/
  /*cout << "list1 : ";
    copy(list1.begin(),list1.end(),ostream_iterator<arynode>(cout," "));
    cout << endl;

    cout << "list2 : ";
    copy(list2.begin(),list2.end(),ostream_iterator<arynode>(cout," "));
    cout << endl << endl;

    cout << "list1 : ";
    copy(list1.begin(),list1.end(),ostream_iterator<arynode>(cout," "));
    cout << endl;

    cout << "list2 : ";
    copy(list2.begin(),list2.end(),ostream_iterator<arynode>(cout," "));
    cout << endl << endl;
    */ 
}
void test_map()
{

  map<string,int > mytest;
  string nodename = "my_lock";
  char lockname[32];
  for( int i=5; i < 20; i++ ) {
    sprintf(lockname,"%s%02d",nodename.c_str(), i);
    mytest[lockname] = i;
  }

  map<string, int>::iterator iter;
  map<string, int>::iterator itertmp;
  string findname = "my_lock19";
  string firstname = mytest.begin()->first;
  if (findname <= firstname )
  {
    cout<< " not find --" <<endl;
  }else
  {
    if ((iter = mytest.find(findname)) != mytest.end())
    {
      itertmp = --iter;
      string a = (itertmp)->first;
      cout<< a <<endl;
    }
  }

}

void test_strtok()
{
  char * pos;
  char * pos1;
  char * pos2;
  string pathtmp;
  char valuetmp[] = "11:33,192.168.1.1:11111";
  int i = 0;
  pos1 = valuetmp;
  char in_ip[32] = "";
  char ou_ip[32] = "";
  pos1 = valuetmp;
  if (pos = strstr(pos1,","))
  {
    *pos = 0;
    string ip;
    int16_t  port;
    get_ip_port(ip, port , pos1);
    cout<<":"<<ip<<":"<<port<<endl;
    get_ip_port(ip, port , pos+1);
    cout<<":"<<ip<<":"<<port;
  }

}
void get_ip_port(string &ip, int16_t &port , char * buf)
{
  char * pos = NULL;
  if (pos = strstr(buf, ":"))
  {
    *pos = 0;
    ip = buf;
    port = atoi(pos+1);
  }

}

void test_string_find_last()
{
  string priority_to_fetch_row = "0000__1111__333__444__555";
  string ret = my_find_last_of(priority_to_fetch_row, "___", 2);
  cout<<ret<<endl;
}
string my_find_last_of(string mystr, const char * str, int num)
{
  string ::size_type ret = string::npos;
  string mystrtmp = mystr;
  string ::size_type pos = string::npos;
  for ( int i = 1; i<= num ;i++)
  {
    if((pos = mystrtmp.find_last_of( str, mystrtmp.length())) != string ::npos)
    {
      if(i==num)
        mystrtmp = mystrtmp.substr(pos+strlen(str));
      else 
        mystrtmp = mystrtmp.substr(0,pos);
    }
    ret =  pos;
  }
  return mystrtmp;
}

void get_time_of_nsec()
{
  /*Time timetmp(1356589053517479);
  cout<<timetmp.toDateTime()<<endl;*/
}
/*void get_time_of_msec()
{
  Time timetmp = Time::now();
  cout<<timetmp.toSeconds()<<endl;
  cout<<timetmp.toMilliSeconds()<<endl;
  cout<<timetmp.toMicroSeconds()<<endl;
  cout<<timetmp.toSecondsDouble()<<endl;
  cout<<timetmp.toMilliSecondsDouble()<<endl;
  cout<<timetmp.toMicroSecondsDouble()<<endl;
  cout<<timetmp.toDateTime()<<endl;
  cout<<timetmp.toDuration()<<endl;

}*/
void foo(void) 
{ 
  unsigned int a = 6; 
  int b = -20; 
  (a+b > 6) ? puts("> 6") : puts("<= 6"); 
}
void foo1()
{
  char str1[] = "abc";
  char str2[] = "abc";
  const char str3[] = "abc"; 
  const char str4[] = "abc"; 
  const char* str5 = "abc";
  const char* str6 = "abc";
  cout << boolalpha << ( str1==str2 ) << endl; // 输出什么？
  cout << boolalpha << ( str3==str4 ) << endl; // 输出什么？
  cout << boolalpha << ( str5==str6 ) << endl; // 输出什么？
}

/*void test_bloomfilter() {
  blooms_init(100000);
  blooms_add("test", 10000, 0.00001);
  int ret = blooms_get("test", "http://www.baidu.com"); 
  printf("ret = %d\n", ret);
  ret = blooms_set("test", "http://www.baidu.com"); 
  printf("ret = %d\n", ret);
  ret = blooms_get("test", "http://www.baidu.com"); 
  printf("ret = %d\n", ret);
  blooms_delete_all();
}*/

void extract_url()
{
  FILE *f = fopen("page.txt", "rb");
  if (f)
  {
    list<UrlNode * > urls;
    UrlNode * url = new UrlNode;
    fseek(f,0,SEEK_END);
    int32_t filelen = ftell(f);
    rewind(f);
    char * urldata = NULL;
    urldata = new char [filelen + 1];
    fread(urldata, 1, filelen, f);
    urldata[filelen]=0;

    struct uri page_uri;

    url->url = "http://hzr8258.1688.com/page/offerlist.htm";
    uri_parse_string(url->url.c_str(), &page_uri);
    struct onfindpackage p = {NULL, url, &urls};
    hlink_detect_string(urldata, &page_uri, onfind, &p);
    uri_destroy(&page_uri);

    if (urldata) delete urldata;
    if (url) delete url;
    delete_urls(urls);
  }
  fclose(f);
}

int  onfind(const char *elem, const char *attr, struct uri *uri, void *arg) {
  struct onfindpackage *p = (struct onfindpackage*)arg;
  char buff[MAX_URL_LEN - 1] ;
  buff[0] = 0;
  list<UrlNode *> *urls = p->urls;

  uri_combine(uri, buff, MAX_URL_LEN - 1, C_SCHEME | C_AUTHORITY | C_PATH | C_QUERY );
  UrlNode *node = new UrlNode;
  node->url=buff;
  char *anchor_text_out = NULL;
  char *tmpanchor_text = NULL;
  char *anchor_text_outtmp = NULL;
  HtRegex regex;
  cout<<buff<<endl;
  if (uri->anchor_text)
  {
    anchor_text_outtmp =  (char*) malloc(strlen(uri->anchor_text) + 10);
    regex.replace(uri->anchor_text, "<[^>]+>",  "", anchor_text_outtmp);
    if (anchor_text_outtmp)
    {
      tmpanchor_text = strtrim(anchor_text_outtmp, NULL);
      node->anchor_text = tmpanchor_text;
    }
  }
  urls->push_back(node);

  if (anchor_text_outtmp) free(anchor_text_outtmp);

  if (anchor_text_out) free (anchor_text_out);

  uri_destroy(uri);

  free(uri);

  return 0;
}

void delete_urls(list<UrlNode *> &urls)
{
  list<UrlNode *>::iterator iter;
  for(iter = urls.begin(); iter != urls.end(); ++iter) {
    UrlNode *newnode = *iter;
    delete newnode;
  }
  urls.clear();
}

void test_inf()
{
  FILE *f = fopen("1.html", "rb");
  if (f)
  {
    fseek(f,0,SEEK_END);
    int32_t filelen = ftell(f);
    rewind(f);
    char * urldata = NULL;
    urldata = new char [filelen + 1];
    fread(urldata, 1, filelen, f);
    urldata[filelen]=0;

    char *tmpcontent = NULL;
    size_t tmpcontentlen = 0;
    int compressret = inf(urldata, filelen, &tmpcontent, &tmpcontentlen);

    if (urldata) delete urldata;
    fclose(f);
  }
}

void test_json_unescape()
{
  FILE *f = fopen("json.txt", "rb");
  if (f)
  {
    fseek(f,0,SEEK_END);
    int32_t filelen = ftell(f);
    rewind(f);
    char * urldata = NULL;
    urldata = new char [filelen + 1];
    fread(urldata, 1, filelen, f);
    urldata[filelen]=0;
    string content(urldata);
    json_unescape_str(content);
    if (urldata) delete urldata;
    fclose(f);
  }
}

int16_t json_unescape_str(string & content)
{
    content = regexreplace(content, "\\\\b", "\b" );
    content = regexreplace(content, "\\\\n", "\n" );
    content = regexreplace(content, "\\\\r", "\r" );
    content = regexreplace(content, "\\\\t", "\t" );
    content = regexreplace(content, "\\\\\\\"", "\"" );
    content = regexreplace(content, "\\\\\\\\", "\\" );
    content = regexreplace(content, "\\\\/", "/" );
}

string  regexreplace(string  str, char * src, char *desc)
{
    boost::regex reg((char *)src,boost::regex::icase|boost::regex::perl);
    string ret =boost::regex_replace(str, reg, desc, boost::match_default | boost::format_all);
    return ret;
}


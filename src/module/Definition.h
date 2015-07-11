#ifndef _MODULE_DEFINITION_H_
#define _MODULE_DEFINITION_H_

#define COMMON_STATE_SUCCESS    0
#define COMMON_STATE_ERROR      1

#define START_TYPE_DEFAULT       0

#define LOGGER_INFO_IDX         0
#define LOGGER_IMPORTANT_IDX    1
#define LOGGER_SOCKET_IDX       2
#define LOGGER_ERROR_IDX        3
#define LOGGER_TIME_IDX         4

#define IS_MAINSERVER   1
#define IS_UNMAINSERVER  0

#define CONFIG_ENCODING   "GB18030"

#define SERVICE_CONFIG_SERVICE  0
#define SERVICE_COMMAND_SERVICE 1
#define SERVICE_STATUS_SERVICE  2

#define MAX_URL_LEN    1024

#define MAX_HOST_LEN    128

#define EXECUTESQL_SUCCESS 0
#define EXECUTESQL_ERROR   1

#define GET_TEMPALTES_ERROR  1
#define GET_SOURCE_ERROR     2
#define UPDATE_SOURCE_ERROR  3
#define UPDATE_TEMPALTES_ERROR 4
#define CREATE_TEMPALTES_AND_SOURCE_ERROR 5

#include <string>
#include <map>
#include <list>
#include <vector>

using namespace std;

class TemplateInfo {
    public:
        int id;    //模板id
        string siteDomain;
        string urlformats;
        string contentPattern;
        string name;
        string templateType;
        string parseClass;
        int source_id;   //数据源id
        int state;   //数据库状态 由 char 转为int
        string md5;

        TemplateInfo() 
        {
           id=0;
           source_id=0;
           state=0;                                  
        }

        ~TemplateInfo()
        {
        }

        void clone(const TemplateInfo &tmpl)
        {
           this->id = tmpl.id;
           this->siteDomain = tmpl.siteDomain.c_str();
           this->urlformats = tmpl.urlformats.c_str();
           this->contentPattern = tmpl.contentPattern.c_str();
           this->name = tmpl.name.c_str();
           this->templateType = tmpl.templateType.c_str();
           this->parseClass = tmpl.parseClass.c_str();
           this->source_id = tmpl.source_id;
           this->state = tmpl.state;
           this->md5 = tmpl.md5;
        }
         
};

typedef struct _SiteSourceInfo
{

        int id;
        int source_id;
        int fetch_interval;
        string name;
        string host;
        string homeurl;
        string pageurl;
        string ultimatepageurl;
        string intxt;
} SiteSourceInfo;


typedef struct _UpdateID
{
   int id;
   unsigned int hashValue;
   time_t time;
   string table;
   int mark;
} UpdateID;





typedef struct _ParsedItem_ 
{
    string name; 
    int length; 
    string stype;
    int mark;
    _ParsedItem_() 
    {  
        length = 0;
        mark = 0;
    }
} ParsedItem;
//static map<string, ParsedItem> ParsedItems;

class Content { 
     public:
           list< map<string,string> >listvalues;  
};

#endif








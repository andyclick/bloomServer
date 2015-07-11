#include <boost/regex.hpp>
#include "CDateTime.h"

#define ISDIGIT(c) ((unsigned) (c) - '0' <= 9)

char *CDateTime::mystrtok_r(char *s, const char *delim, char **save_ptr)
{
	char *token;
	
	if (s == NULL)
		s = *save_ptr;
	
	/* Scan leading delimiters.  */
	s += strspn (s, delim);
	if (*s == '\0')
	{
		*save_ptr = s;
		return NULL;
	}
	
	/* Find the end of the token.  */
	token = s; 
	s = strpbrk (token, delim);
	if (s == NULL)
		/* This token finishes the string.  */
		*save_ptr = strchr (token, '\0');
	else
	{
		/* Terminate the token and make *SAVE_PTR point past it.  */
		*s = '\0';
		*save_ptr = s + 1;
	}
	return token;
}

CDateTime::CDateTime()
{
	sStamp = time(NULL);
	sTm = localtime_r(&sStamp,&sc);
	sTm->tm_isdst = 0;
	sTm->tm_wday = 0;
	sTm->tm_yday = 0;
	sYear = sTm->tm_year;
    sMonth = sTm->tm_mon;
    sDay = sTm->tm_mday;

    sHour = sTm->tm_hour;
    sMinute = sTm->tm_min;
    sSecond = sTm->tm_sec;

	ClearHourMinuteSecond();
}

CDateTime::~CDateTime() {
	::memset(sTm,0,sizeof(sTm));
}

bool CDateTime::ParseDateTime(const char *timeinputstr,const char *format)
{
	if(timeinputstr == NULL || ::strlen(timeinputstr) ==0) return false;
	string timeinput(timeinputstr),strtime;
	if(format != NULL)
	{
		string pattern(format);
		bool ret = parseFormatDate(timeinput,pattern);
		if(ret) return true;
	}

	strtime = timeinput;
	char *str = (char *)malloc(strtime.length()+1);
	
	if(str){
		char *strtmp = str;
		ReplaceSpace(strtime,"点","时");
		str[0] = 0;
		if(FilteTag(strtime.c_str(),str)){
			strtime = str;
			ReplaceSpace(strtime,NULL," ");
			
			if(strtime.empty())
			{
				free(strtmp);
				return false;
			}

			if(defaultparse(strtime.c_str()))
			{
				free(strtmp);
				return true;
			}else if(ParseStepFirst(strtime)){
				free(strtmp);
				return true;
			}else if(ParseStepSecond(strtime)){
				free(strtmp);
				return true;
			}else if(ParseStepThird(strtime)){
				free(strtmp);
				return true;
			}else if(ParseSteFourth(strtime.c_str())){
				free(strtmp);
				return true;
			}else if(ParseSteFifth(strtime.c_str())){
				free(strtmp);
				return true;
			}else{
				if(getHMS(strtime)){
					sTm->tm_year = sYear;
					sTm->tm_mon = sMonth;
					sTm->tm_mday = sDay;
					free(strtmp);
					return true;
				}
				
				free(strtmp);
				return false;
			}
		} else {
			free(strtmp);
			return false;
		}
	}
	return false;
}



void CDateTime::GetTimestamp(time_t *t) {
	*t = this->sStamp;
}

bool CDateTime::GetTimestamp(char *input, time_t *t,bool decision,char * inputformat) {
	bool ret = ParseDateTime(input,inputformat);
	if(decision && sStamp >= time(NULL))
			isOverCurTime();
	if (ret && this->sStamp !=-1) {
		*t = this->sStamp;
	}else{
		*t = time(NULL);
	}
	return ret;
}

bool CDateTime::GetDateStrCurrentTimeAsDefault(char *input, char *output,bool decision,char * inputformat) {
	return GetDateStr(input, "%Y-%m-%d %H:%M:%S", output,decision,inputformat);

}

void CDateTime::GetDefaultDateStr(char *output) {
	GetDateStr("%Y-%m-%d %H:%M:%S", output);
}

bool CDateTime::GetDateStrCurrentTimeAsNoStyle(char *input, char *output,bool decision,char * inputformat) {
    return GetDateStr(input, "%Y%m%d%H%M%S", output,decision,inputformat);
}

bool CDateTime::GetDefaultDateStr(char *input, char *output,bool decision,char * inputformat) {
	return GetDateStr(input, "%Y-%m-%d %H:%M:%S", output,decision,inputformat);
}

void CDateTime::GetDateStr(char *outformat, char *output) {
	strftime(output, 128,outformat, sTm);
}

bool CDateTime::GetDateStr(char *input, char *outformat, char *output,bool decision,char * inputformat) {
	if (ParseDateTime(input,inputformat)) {
		if(decision && sStamp >= time(NULL))
			isOverCurTime();
		strftime(output, 128, outformat, sTm);
		return true;
	} else {
		time_t t = time(NULL);
		struct tm *d = localtime(&t);
		strftime(output, 128,outformat, d);
		return false;
	}
}

#ifdef __FOR_WIN__
time_t CDateTime::getTimeT(COleDateTime &tmptime) {
	sTm->tm_year = tmptime.GetYear() - 1900;
	sTm->tm_mon = tmptime.GetMonth() > 0? tmptime.GetMonth() - 1: 0;
	sTm->tm_mday = tmptime.GetDay();
	sTm->tm_hour = tmptime.GetHour();
	sTm->tm_min = tmptime.GetMinute();
	sTm->tm_sec = tmptime.GetSecond();
	sTm->tm_wday = tmptime.GetDayOfWeek();
	sTm->tm_yday = tmptime.GetDayOfYear();
	sTm->tm_isdst = 0;
	
	time_t sStamp =  mktime(sTm);
	return sStamp;
}
#endif

struct tm * CDateTime::localtime_r (const time_t *t, struct tm *tp)
{
	struct tm *l = localtime (t);
	if (! l)
		return 0;
	*tp = *l;
	return tp;
}
void CDateTime::isOverCurTime()
{
	time_t curtime = time(NULL);
	struct tm ts;
	struct tm *tmtime = localtime_r(&curtime,&ts);
	if(sTm->tm_year > tmtime->tm_year) sTm->tm_year = tmtime->tm_year;
	if(sTm->tm_year == tmtime->tm_year && sTm->tm_mon >tmtime->tm_mon)
	{
		sTm->tm_year = tmtime->tm_year - 1;
	}

	if(sTm->tm_year == tmtime->tm_year && sTm->tm_mon ==  tmtime->tm_mon && sTm->tm_mday > tmtime->tm_mday)
	{			
		sTm->tm_year = tmtime->tm_year - 1;
	}
	sStamp = mktime(sTm);
}
//defaultparse:处理常见时间格式，如时间戳、1980-2-4 am 3:40 、1980-2-4  3:40 am …… 
bool CDateTime::defaultparse(const char *timestring)
{
#ifdef __FOR_WIN__
	time_t times;
	if(IsTime_T(timestring,times))
	{
		this->sStamp = times;
		this->sTm = localtime_r(&sStamp,&sc);
		return true;
	}else{
		COleDateTime tmptime(sStamp);
		try {
			if (tmptime.ParseDateTime(timestring))
			{
				sStamp = getTimeT(tmptime);
				if (sStamp == -1) {
					sStamp = time(NULL);
					return false;
				}
				this->sTm = localtime_r(&sStamp,&sc);
				return true;
			}
		} catch(...) {
			sTm->tm_isdst = 0;
			sTm->tm_wday = 0;
			sTm->tm_yday = 0;
			return false;	
		}
	}
	sTm->tm_isdst = 0;
	sTm->tm_wday = 0;
	sTm->tm_yday = 0;
#endif
	return false;
}

void CDateTime::ClearHourMinuteSecond() {
	sTm->tm_hour = 0;
    sTm->tm_min = 0;
    sTm->tm_sec = 0;
	
    sHour = sTm->tm_hour;
    sMinute = sTm->tm_min;
    sSecond = sTm->tm_sec;
}

//ParseStepFirst: 处理如 1天前、昨天……
bool CDateTime::ParseStepFirst(string timestr)
{
	time_t ltime = 0;
	int i = 0;
	ReplaceSpace(timestr,NULL,NULL);
	if(timestr.find("今天") != string::npos){
		this->sStamp = time(NULL);
		this->sTm = localtime_r(&sStamp,&sc);
		ClearHourMinuteSecond();
		return true;
	}else if(timestr.find("昨天") != string::npos){
		this->sStamp = time(NULL) - dayseconds;
		this->sTm = localtime_r(&sStamp,&sc);
		ClearHourMinuteSecond();
		return true;
	}else if(timestr.find("前天") != string::npos){
		this->sStamp = time(NULL) - dayseconds * 2;
		this->sTm = localtime_r(&sStamp,&sc);
		ClearHourMinuteSecond();
		return true;
	}else if(timestr.find("分钟前") != string::npos){
		return Findstr(timestr,stringminstamp);
	}else if (timestr.find("小时前") != string::npos){
		return Findstr(timestr,stringhourstamp);
	}else if(timestr.find("天前") != string::npos){
		bool ret = Findstr(timestr,stringdaystamp);
		ClearHourMinuteSecond();
		return ret;
	}else if(timestr.find("周前") != string::npos){
		bool ret = Findstr(timestr,stringweekstamp);
		ClearHourMinuteSecond();
		return ret;
	}
	return false;
}

bool CDateTime::ParseStepSecond(string text)
{
	if(parse_english(text)){
		getHMS(text);		//此处不考虑时分秒是否得到。如：2005 may 5 
		this->sStamp = mktime(sTm);
		return true;
	}
	return false;
}
bool CDateTime::ParseStepThird(string text)
{
	if(parseYMD(text)){
		getHMS(text);		//此处不考虑时分秒是否得到。如：2005 may 5 
		this->sStamp = mktime(sTm);
		return true;
	}
	return false;
}
bool CDateTime::ParseSteFourth(string text)
{
	char *pat = "(?:有|剩)\\s*(?:(\\d+)\\s*天)?\\s*(?:(\\d+)\\s*小时)?\\s*(?:(\\d+)\\s*分钟)?";
	std::string::const_iterator start, end;
	start = text.begin();
	end =text.end();
	
	string result;
	bool isfind = false;
	try{
		boost::regex r(pat);

		boost::match_results<std::string::const_iterator> searchwhat;
		if(boost::regex_search(start, end, searchwhat, r)) {
			if(searchwhat[1].matched){
				result = std::string(searchwhat[1].first,searchwhat[1].second);
				sStamp += atoi(result.c_str())*dayseconds;
				isfind = true;
			}
			if(searchwhat[2].matched){
				result = std::string(searchwhat[2].first,searchwhat[2].second);
				sStamp += atoi(result.c_str())*hourseconds;
				isfind = true;
			}
			if(searchwhat[3].matched){
				result = std::string(searchwhat[3].first,searchwhat[3].second);
				sStamp += atoi(result.c_str())*minuteseconds;
				isfind = true;
			}
			this->sTm = localtime_r(&sStamp,&sc);
			return isfind;
		}
	}catch(...){
	}
	return false;
}
bool CDateTime::ParseSteFifth(string text)
{
	return false;
}


CDateTime::TokenType CDateTime::GetTokenType(char c)
{
    if ((c>='0') && (c<='9'))
        return TT_Number;
    else if ('-'==c){
        return TT_Minus;
    }else if ('/'==c){
        return TT_Minus;
    }else if (' ' == c){
        return TT_Blank;
    }else if(':'==c){
        return TT_Colon;
    }else{
        return TT_Null;
    }
}


//IsTime_T：判断是否是时间戳
bool CDateTime::IsTime_T(const char *timestring,time_t &timestamp)
{
	char timestr[128];
	string timetmp(timestring);
	ReplaceSpace(timetmp,NULL,NULL);
	timestamp =(time_t)atol(timetmp.c_str());
	::sprintf(timestr,"%ld",timestamp);
	if(timetmp.length() == ::strlen(timestr))
	{	
		timestamp = ::abs(timestamp);
// 		if(timestamp >= sStamp) //大于当前时间
// 			timestamp= sStamp;
		struct tm *tmResult = localtime(&timestamp);		
		if (tmResult) {
			time_t t = mktime(tmResult);
			if(t != -1 && t == timestamp)
				return true;
			else
				return false;
		} else {
			return false;
		}
	}else{
		return false;
	}	
}

//FilteTag： 过滤html标签，如<div><i>2009年12月31日</i></div><div>09:30</div>
bool CDateTime::FilteTag(const char *strSource,char *strDest)
{
	int strLen = ::strlen(strSource) +1;
	char *buff = (char *)malloc(strLen);
	if(buff){
		char *primitive = buff;
		::memset(buff,0,strLen);
		strncpy(buff,strSource,strLen -1);
		char *p = strchr(buff,'<');
	
		while(p)
		{
			*p = 0;
			::strcat(strDest,buff);
			::strcat(strDest," ");
			buff = ++p;
			p = strchr(buff,'>');
			if(p) buff = ++p;
			p = strchr(buff,'<');
		}
		::strcat(strDest,buff);		
	
		free(primitive);
		return true;
	}
	return false;
}

char *CDateTime::strrtrim(char *str, const char *trim)
{
    char    *end;
	
    if(!str) 
        return NULL;
    
    if(!trim)
        trim = " \t\n\r";
	
    end = str + ::strlen(str);
    
    while(end-- > str)
    {
        if(!strchr(trim, *end))
            return str;
        *end = 0;
    }
    return str;
}   

char *CDateTime::strltrim(char *str, const char *trim)
{
    if(!str)
        return NULL;
	
    if(!trim)
        trim = " \t\r\n";
	
    while(*str)
    {
        if(!strchr(trim, *str))
            return str;
        ++str;
    }
    return str;
}

char *CDateTime::strtrim(char *str, const char *trim)
{
	char *buf = strltrim(strrtrim(str, trim),  trim);
	return buf;
}


void CDateTime::ReplaceSpace(string &str, const char *inputstr, const char *outstr)
{
    if( str.size() ==0)
        return;

	string replaced;
	if (outstr) {
		replaced = outstr;
	}

	if(!inputstr || inputstr[0] == 0)
	{
		inputstr = " |\t|\r|\n|\\(|\\)|\\[|\\]|;|,|；|，|&nbsp|　";
	}
	try{
		boost::regex r(inputstr);
		string output;
		output = boost::regex_replace(str, r, replaced,  boost::match_default | boost::format_all);

		str.assign(output);
	}catch(...){
	}
}

//查找如：1天前、两天前……
bool CDateTime::Findstr(string timestr,const stringToStamp *stringtostamp)
{
	time_t ltime = 0;
	int i = 0;	
	while(::strlen(stringtostamp[i].sstring) > 0)
	{
		if (timestr.compare(stringtostamp[i].sstring) == 0 || timestr.find(stringtostamp[i].sstring) !=string::npos)
		{
			time( &ltime );
			ltime -= stringtostamp[i].stamp + 1;
			this->sStamp = ltime;
			this->sTm = localtime_r(&sStamp,&sc);
			return true;
		}
		i++;
	}
	return false;
}
bool CDateTime::LoadCurrenCDateTime()
{   
    time_t t;
    time(&t);
    //FromUnixDatetime(t);   
    return true;
}

/// <summary>
/// 判定给定的年份是否是润年
/// </summary>
/// <param name="year">需要判定的年份</param>
/// <returns>true:给定的年份是润年。false:给定的年份不是润年。</returns>
bool CDateTime::IsLeapYear(int year)
{
    return ((year % 4 == 0) && (year % 100 != 0) || (year % 400 == 0));                              
}

/// <summary>
/// 判定给定的年份是否有效。
/// </summary>
/// <param name="year">给定的年份</param>
/// <returns>true:有效,false:无效</returns>
bool CDateTime::ValidateDate(int year)
{
    return (year > 0) && (year <= 9999);
}

/// <summary>
/// 判定给定的年月是否有效
/// </summary>
/// <param name="year">给定的年份</param>
/// <param name="month">给定的月份</param>
/// <returns>true:有效。false:无效。</returns>
bool CDateTime::ValidateDate(int year,int month)
{
    if (!ValidateDate(year))
        return false;
    return (month > 0) && (month < 13);
}

/// <summary>
/// 得到一个月份的天数
/// </summary>
/// <param name="year">年</param>
/// <param name="month">月</param>
/// <returns>返回该年该月的总天数，如果给定的参数有错误，则返回0</returns>
int CDateTime::GetDaysOfMonth(int year, int month)
{
    if (!ValidateDate(year, month))
    {
        return 0;
    }

    if (month == 4 || month == 6 || month == 9 || month == 11)
    {
        return 30;
    }
    else if (month == 1 || month == 3 || month == 5
        || month == 7 || month == 8 || month == 10 || month == 12)
    {
        return 31;
    }
    else if (2 == month)
    {
        if (IsLeapYear(year))//如果是闰年
        {
            return 29;
        }
        else
        {
            return 28;
        }
    }

    return 0;
}

/// <summary>
/// 判定给定的年月日是否是一个有效的日期
/// </summary>
/// <param name="year">给定的年份</param>
/// <param name="month">给定的月份</param>
/// <param name="day">给定的日子</param>
/// <returns>true:给定的年月日是一个有效的日期。false:不是一个有效的日期。</returns>
bool CDateTime::ValidateDate(int year, int month, int day)
{
    if (!ValidateDate(year, month))
        return false;

    if ((day < 1) || (day > GetDaysOfMonth(year, month)))
        return false;

    return true;                       
}

/// <summary>
/// 判定给定的小事是否有效
/// </summary>
/// <param name="hour">给定的小时</param>
/// <returns>true:有效;false:无效</returns>
bool CDateTime::ValidateTime(int hour)
{
    return (hour >= 0) && (hour < 24);
}

/// <summary>
/// 判定给定的小时和分钟是否有效。
/// </summary>
/// <param name="hour">给定的小时</param>
/// <param name="minute">给定的分钟</param>
/// <returns>true:有效;false:无效</returns>
bool CDateTime::ValidateTime(int hour,int minute)
{
    if (!ValidateTime(hour))
        return false;
    return (minute >= 0) && (minute < 60);
}

/// <summary>
/// 判定给定的小时、分钟、秒时否有效
/// </summary>
/// <param name="hour">给定的小时</param>
/// <param name="minute">给定的分钟</param>
/// <param name="second">给定的秒</param>
/// <returns>true:有效;false:无效</returns>
bool CDateTime::ValidateTime(int hour, int minute, int second)
{
    if (!ValidateTime(hour,minute))
        return false;
    return (second >= 0) && (second < 60);
}

/// <summary>
/// 判定给定的年月日时分秒是否是一个有效的日期时间
/// </summary>
/// <param name="year">给定的年份</param>
/// <param name="month">给定的月份</param>
/// <param name="day">给定的日子</param>
/// <param name="hour">给定的小时</param>
/// <param name="minute">给定的分钟</param>
/// <param name="second">给定的秒</param>
/// <returns>true:有效;false:无效</returns>
bool CDateTime::ValidateDateTime(int year, int month, int day,
              int hour, int minute, int second)
{
    return ValidateDate(year, month, day)
        && ValidateTime(hour, minute, second);
}

bool CDateTime::Validate()
{
    return Validate(this);
}   

bool CDateTime::Validate(CDateTime *obDateTime)
{
    return ValidateDateTime(obDateTime->sYear,obDateTime->sMonth, obDateTime->sDay,
        obDateTime->sHour, obDateTime->sMinute, obDateTime->sSecond);
}

time_t CDateTime::ToUnixDatetime()
{   
    tm tt;
    tt.tm_year = sYear - 1900;
    tt.tm_mon = sMonth -1;
    tt.tm_mday = sDay;
    tt.tm_hour = sHour;
    tt.tm_min = sMinute;
    tt.tm_sec = sSecond;
    return mktime(&tt);
}

//查找带有英文月份的时间
bool CDateTime::parse_english(string text)
{ 	
	
	bool ret = false;
	//memset(&stm,0,sizeof(stm));
	string month[]={"jan","feb","mar","apr","may","jun","jul","aug","sep","oct","nov","dec"};	

    for(int i=0;i<12;i++)
    { 
		int p=indexOfIgoreCase(text,month[i]);
		if(p>-1)
		{			
			return getYMD(text,i,p);
		}
    }
	return false;
 }

int CDateTime::indexOfIgoreCase(string text, string str)
{
	bool word=false;
	
	for(int i=0;i<text.length();i++)
	{ 
		char c=text.at(i);
		if(c>='a'&&c<='z'||c>='A'&&c<='Z')
		{
			if(word==false)
			{  
				word = true;
				int p=matchWordIgoreCase(text,i,str);
				if(p>=0)
					return p;
			}
			
		}
		else word=false;		
	}
	return -1;
}


int CDateTime::matchWordIgoreCase(string text, int i, string str)
{
    for(int j=0;j<str.length() && text.length()>i+j;j++)
    { 
		char c0=text.at(i+j);
		char c1=str.at(j);
		if(!(c0==c1||c0+32==c1))
			return -1;
    }
    return i;
}
//得到年月日
bool CDateTime::getYMD(string text, int month, int &p)
{ 
	int year=0,day=0,tmp;

	year=getHeaderDigit(text,p);  //提取text的p位置之前的数字

	int p0= findDigit(text,p);
	if(p0 == -1 && year == -1)return false; //only exist month，not year、day

	int p1= findNonDigit(text,p0);
	if(p0 == -1 && p1 == -1)return false;
	if(p1 != 0 && p0 != -1)
		day=atoi(text.substr(p0,text.length()).c_str());
	else
		day = -1;
	
	if(year==-1 && p1!=-1 && p0 !=-1)
	{
		int p2=findDigit(text,p1);
		if(p2!=-1)
		{ 
			int p3 = findNonDigit(text, p2);
			if(p3==-1 && p3 != ':') //如果是 ':'说明这前面找到的数字为小时值
				year=atoi(text.substr(p2,text.length()).c_str());
		}
	}
	if(day>31 ||(year >=0 && day == -1))//当day>31或day==-1交换year和day的值
	{
		tmp = day;
		day = year;
		year = tmp;
	}
	if(year != -1)// year == -1 时说明在串里没有年份
	{
		sTm->tm_year = year<1900 ? year: year-1900;
		sTm->tm_year = sTm->tm_year > 29? sTm->tm_year:sTm->tm_year+100;
	}
	sTm->tm_mon = month;
	if(day != -1) 
		sTm->tm_mday = day;
	else			//当day == -1时说明在串里没有找到day值
		sTm->tm_mday = 1;
	p=p1;
	return true; //成功
	
}
//得到时分秒
bool CDateTime::getHMS(string text)
{   
	int i=0;
	int atlocal;
	bool isam = false,ispm = false;
//	ReplaceSpace(text,1,NULL,"");
	ReplaceSpace(text,"时|分",":");
	ReplaceSpace(text,"秒"," ");
	size_t atcolon = text.find(":");
	
	while(atcolon != string::npos)
	{
		if(atcolon == 0 )
		{
			atcolon = text.find(":",atcolon + 1);
			continue;
		}
		if(atcolon == text.length() -1 ) return false;
		if(ISDIGIT(text.at(atcolon -1)) && ISDIGIT(text.at(atcolon +1)))
		{
			break;
		}else{
			atcolon = text.find(":",atcolon + 1);
		}
	}

	if(atcolon != string::npos)
	{	
		
		atlocal = (int)atcolon - 4;
		while(atlocal<0) atlocal++;
		string str = text.substr(atlocal,20);
		if(str.find("am")!=string::npos || str.find("AM")!=string::npos)
		{
			sTm->tm_hour = -12;
			isam =true;
		}
		if(str.find("pm")!=string::npos || str.find("PM")!=string::npos)
		{
			sTm->tm_hour = 12;	
			ispm =true;
		}
		char *finds = "a|A|p|P|m|M";
		ReplaceSpace(str,finds,"");
		char *strtmp = strdup((char *)str.c_str());
		char *savepos = NULL;
		char *strnum = mystrtok_r(strtmp,":", &savepos);
		while(strnum)
		{
			if(i==0)
			{
				atlocal = ::strlen(strnum)-2;	//去掉：两位前的字符。如去掉asd15:20中的asd
				while(atlocal<0) atlocal++;	
				strnum = strnum + atlocal;
				while(*strnum && (*strnum< '0' || *strnum>'9'))strnum++;
				//if(*strnum>= '0' && *strnum<='9')
				{
					if(isam)
					{
						sTm->tm_hour = atoi(strnum) == 12 ? 0 : atoi(strnum);
						sTm->tm_hour = ::abs(sTm->tm_hour);
					}else if(ispm){
						sTm->tm_hour = atoi(strnum) +12;				
					}else{
						sTm->tm_hour = atoi(strnum);
					}				
				}				
			}else if(i==1){
				sTm->tm_min = atoi(strnum);
			}else if(i==2){
				if (::strlen(strnum) > 2) {
					strnum[2] = 0;				//舍弃：两位后的字符。如：15:20asd中的asd
				}

				sTm->tm_sec = atoi(strnum);
			}
			strnum = mystrtok_r(NULL,":", &savepos);
			++i;
		}
		free(strtmp);
		return true;
	}
	return false;	
}


bool CDateTime::isColon(string text, int b, int e)
{ 	
	for(int i=b;i<e;i++)
    { 
		if(text.at(i)==':')return true;
    }
    return false;
}


int CDateTime::textLen(string text, int b, int e)
{ 
	int n=0;
	//    System.out.println(text.substring(b,e));
    for(int i=b;i<e;i++)
    { 
		char c=text.at(i);
		if(c!=' '&&c!='\n'&&c!='\r')
			n++;
    }
    return n;
}


int CDateTime::findDigit(string text, int begin)
{ 
	//System.out.println("=="+begin);
    for(int i=begin;i<text.length();i++)
    { 
		char c=text.at(i);
		if(c>='0'&&c<='9') 
			return i;
    }
    return -1;
}

int CDateTime::findNonDigit(string text, int begin)
{ 
	for(int i=begin;i<text.length();i++)
	{ 
		char c=text.at(i);
		if(!(c>='0'&&c<='9')) 
			if(c != 58)  //58为 ":号"
				return i;
			else
				return 0;
	}
	return -1;
}


int CDateTime::getHeaderDigit(string text, int p)
{
	if(p<1) return -1;  //前面至少要有一位数字
	text = text.substr(0,p);
	ReplaceSpace(text,NULL,"");
	p = text.length() - 1;
	int i=0;
	for(; p>=0; i++,p--)
	{
		if(text.at(p) < '0' || text.at(p) > '9')
		{
			if(i==0)return -1;	//前面没有数字
			 return atoi(text.substr(p+1).c_str());			 
		}
	}
	return atoi(text.substr(p+1).c_str());
}

bool CDateTime::parse_chinese(string text)
{
	if(text.length() == 0) return  false;
	//System.out.println("========"+text);
	int start=0;
	int g[12];
	int group;
	while(true)
	{
		group=0;
		for(int i=0;i<6;i++)
		{
			g[i*2] = findDigit(text, start); //第一个数字
			 if(g[i*2]==-1) break;
			 group++;
			 g[i*2+1] = findNonDigit(text, g[i*2]);
			 if(g[i*2+1]==-1) 
			 {	 
				 //没有非数字字符或者上一组的非数字到本组数字距离超过一个字符
				 if(i==0|| textLen(text,g[i*2-1],g[i*2])>1)
				 {
					 group--; 
					 break;
				 }
			 }
			 start=g[2*i+1]; //非数字
			 if(start==-1) break;
			 if(i>0)
			 {
				 if( textLen(text,g[i*2-1],g[i*2])>1)
				 {
					 group--; 
					 break;
				 }
			 }
			 
		 }

       if(group>1) break;
       if(group==0) 
		   return false;
       start=g[1];
       if(start==-1)
		   return false;
     } //while
     string strb;
     string datestr;
     switch(group)
     { 
	 case 6:
		 {
			 if(g[1]-g[0]==2)
				 strb.append("yy");
			 else 
				 strb.append("yyyy");
			 strb.append(text.substr(g[1], g[2]-g[1])).append("MM").
				 append(text.substr(g[3], g[4]-g[3])).append("dd").
				 append(text.substr(g[5], g[6]-g[5])).append("HH").
				 append(text.substr(g[7], g[8]-g[7])).append("mm").
				 append(text.substr(g[9], g[10]-g[9])).append("ss");
			 if (g[11] == -1) 
				 datestr=text.substr(g[0],text.length());
			 else 
				 datestr=text.substr(g[0],g[11]-g[0]);
			 break;
		 }
	 case 4:
         { if(isColon(text,g[5],g[6]) )
		 {
             strb.append("MM").
             append(text.substr(g[1], g[2]-g[1])).append("dd").
             append(text.substr(g[3], g[4]-g[3])).append("HH")
             .append(text.substr(g[5], g[6]-g[5])).append("mm");
           }
           else
           {
               if(g[1]-g[0]==2)strb.append("yy");
               else strb.append("yyyy");
               strb.append(text.substr(g[1], g[2]-g[1])).append("MM").
               append(text.substr(g[3], g[4]-g[3])).append("dd")
               .append(text.substr(g[5], g[6]-g[5])).append("HH");
           }
           if (g[7] == -1) datestr=text.substr(g[0],text.length());
           else datestr=text.substr(g[0],g[7]-g[0]);
           break;
         }
        case 5:
        { if(isColon(text,g[5],g[6]) )
          {
            strb.append("MM").
            append(text.substr(g[1], g[2]-g[1])).append("dd").
            append(text.substr(g[3], g[4]-g[3])).append("HH").
            append(text.substr(g[5], g[6]-g[5])).append("mm")
            .append(text.substr(g[7], g[8]-g[7])).append("ss");
          }
          else
          {
              if(g[1]-g[0]==2)strb.append("yy");
              else strb.append("yyyy");
              strb.append(text.substr(g[1], g[2]-g[1])).append("MM").
              append(text.substr(g[3], g[4]-g[3])).append("dd").
              append(text.substr(g[5], g[6]-g[5])).append("HH")
              .append(text.substr(g[7], g[8]-g[7])).append("mm");
          }
          if (g[9] == -1) datestr=text.substr(g[0],text.length());
          else datestr=text.substr(g[0],g[9]-g[0]);
          break;
        }

        case 3:
          { if(isColon(text,g[3],g[4]) )
            {
              strb.append("MM").
              append(text.substr(g[1], g[2]-g[1])).append("dd")
              .append(text.substr(g[3], g[4]-g[3])).append("HH");
            }
            else
            {
                if(g[1]-g[0]==2)strb.append("yy");
                else strb.append("yyyy");
                strb.append(text.substr(g[1], g[2]-g[1])).append("MM")
                    .append(text.substr(g[3], g[4]-g[3])).append("dd");
            }
            if (g[5] == -1) datestr=text.substr(g[0],text.length());
            else datestr=text.substr(g[0],g[5]-g[0]);
            break;
          }
          case 2:
          { strb.append("MM").append(text.substr(g[1], g[2]-g[1])).append("dd");
            if (g[3] == -1) datestr = text.substr(g[0],text.length());
            else datestr = text.substr(g[0], g[3]-g[0]);
            break;
          }

     }
	 //SpiderDateFormat df(strb);
     return 0;//df.parse(datestr,date);
}

int CDateTime::isMatchedDateFormat(string s, string f, int p)
{
	for(int i=p,j=0;i<s.length();i++,j++)
	{ 
		char c0=s.at(i);
		char cf=f.at(j);
		if(cf=='\'')
		{ 
			j++;
			if(j>=f.length())
				return i-p;
			cf=f.at(j);
		}
		switch( cf)
		{ 
		case 'y': 
			if( !ISDIGIT(c0) && c0!=cf)  //c0!=cf ?
				return -1;
			break;
		case 'm':
		case 'M':  //月份
			if(!ISDIGIT(c0) && c0!=cf)
			{ 
				int el= englishMonth(s,i); //判断s前三个字符是不是月份开始字符
				if(el>0) 
					i+=el-1; //i 指向月份之后的第一个字符
				else  
					return -1;
			}
			while(j<f.length()-1&&f.at(j+1)==cf)  //格式串跳过相同的格式字符
				j++;
			if(i<s.length()-1&&ISDIGIT(s.at(i+1))) //内容串跳过连续的数字
				i++;
			break;
		case 'H':
		case 'h':
		case 's':
		case 'S':
		case 'D':
		case 'd':
			if(!ISDIGIT(c0)  && c0!=cf ) return -1;
			if(j<f.length()-1&&f.at(j+1)==cf) j++;
			if(i<s.length()-1&&ISDIGIT(s.at(i+1))) i++;
			break;
		default: 
			if(c0!=cf)  //非通配字符内容串跟格式串要相同
				return -1;
		}
		if(j==f.length()-1)
			return i-p+1; //返回匹配结束长度
	}
	return -1;
}

int CDateTime::englishMonth(string text, int p)
{ 
	string month[]={"jan","feb","mar","apr","may","jun","jul","aug","sep","oct","nov","dec"};
	
	if(text.length()-p<3) 
		return 0;
	
	for(int i=0;i<12;i++)
	{
		if(isHeaderOf(text,p,month[i]))
		{
			for (int j = p + 3; j < text.length(); j++) // j <= text.length()?
			{
				char c = text.at(j);
				if(!isLetter(c)) 
					return j-p;
			}
		}
	}
	return 0;
	
	
}

bool CDateTime::isHeaderOf(string text, int p,string head)
{
	for(int j=0;j<head.length();j++)
	{ 
		char c0=head.at(j);
		char c1=text.at(p+j);
		if(c1!=c0&&c0-32!=c1) 
			return false;
	}
	
	return true;
}

bool CDateTime::isLetter(char c)
{ 
	if(c>='a'&&c<='z')
		return true;
	if(c>='A'&&c<='Z')
		return true;
	return false;
}

bool CDateTime::parseYMD(string text)
{
	int year =0,month = 0, day = 0;
	int date[10] = {0};
	int i = 0;
	int effectivetime = 0; 
	bool istrue = false;
	ReplaceSpace(text," |\\.|-|\\\\|－|年|月|日","/");

	char *str = strdup((char *)text.c_str());
	char *savepos = NULL;
	char *strtmp = mystrtok_r(str,"/", &savepos);
	while(strtmp)
	{
		int p = findDigit(strtmp,0);
		if(p != -1 && i<10 && strchr(strtmp,':') == NULL){
// 			int atlocal = strlen(strtmp)-4;
// 			while(atlocal<0) atlocal++;	
// 			strtmp = strtmp + atlocal;
// 			
// 			while(*strtmp< '0' || *strtmp>'9')
// 			{
// 				if(*strtmp)
// 					strtmp++;
// 				else
// 					break;
// 			}
			date[i] = atoi(strtmp + p);
			if(++effectivetime >= 2)
				istrue= true;
		}			
		strtmp = mystrtok_r(NULL,"/", &savepos);
		i++;
	}
	free(str);
	if(!istrue) return false;
	for(i=0;i<10;i++)
	{
		if(year == 0 && (date[i] > 1900 || (date[i] >0 && date[i] < 99)))	{
			year = date[i] > 1900 ? date[i] -1900 : date[i];
			continue;
		}
		if(month == 0 && date[i]<12 && day ==0){
			month = date[i];
			continue;	
		}
		if(day ==0 && date[i] <31 && (year !=0 || month !=0)){
			day = date[i];
			continue;
		}
		if(year && month && day)break;
	}
	if((year&&month)||(month&&day)){
		sTm->tm_year = year;
		sTm->tm_year = sTm->tm_year > 29? sTm->tm_year:sTm->tm_year+100;
		sTm->tm_mon = month>0?month -1:0;
		sTm->tm_mday = day == 0 ?1:day;
		return true;
	}else if(year&&day){
		sTm->tm_mon = year>0?year -1:0;
		sTm->tm_mday = day == 0 ?1:day;
		return true;
	}
	return false;
}

bool CDateTime::parseFormatDate(string &s,string &pattern)
{
	string year,month,day,hour,minutes,second;
	int slen = s.length();
	int plen = pattern.length();

	for(int i=0,j=0;i<slen;)
	{ 
		char c0=s.at(i++);
		char cf=pattern.at(j++);
		if(cf=='\'')
		{ 
			j++;
			if(j>=plen)
				return false;
			cf=pattern.at(j);
		}
		switch( cf)
		{
		case 'Y':
		case 'y': 
			while(ISDIGIT(c0))
			{
				year += (c0);
				if(i<slen)
					c0 = s.at(i++);
				else
					break;
			}
			while(j<plen && (cf == 'Y' || cf =='y'))
			{
				cf = pattern.at(j++);
			}
				
			break;
		
		case 'M':  //月份
			while(ISDIGIT(c0))
			{
				month += (c0);
				if(i<slen)
					c0 = s.at(i++);
				else
					break;
			}
			while(j<plen && cf == 'M')
			{
				cf = pattern.at(j++);
			}
			
			break;

		case 'D':
		case 'd':
			while(ISDIGIT(c0))
			{
				day += (c0);
				if(i<slen)
					c0 = s.at(i++);
				else
					break;
			}
			while(j<plen && (cf == 'D' || cf == 'd'))
			{
				cf = pattern.at(j++);
			}
			break;
		case 'H':
		case 'h':
			while(ISDIGIT(c0))
			{
				hour += (c0);
				if(i<slen)
					c0 = s.at(i++);
				else
					break;
			}
			while(j<plen && (cf == 'H' || cf == 'h'))
			{
				cf = pattern.at(j++);
			}
			break;

		case 'm': //分钟
			while(ISDIGIT(c0))
			{
				minutes += (c0);
				if(i<slen)
					c0 = s.at(i++);
				else
					break;
			}
			while(j<plen && cf == 'm')
			{
				cf = pattern.at(j++);
			}
			break;
			
		case 's':
		case 'S':
			while(ISDIGIT(c0))
			{
				second += (c0);
				if(i<slen)
					c0 = s.at(i++);
				else
					break;
			}
			while(j<plen && (cf == 'S' || cf =='s'))
			{
				cf = pattern.at(j++);
			}
			break;
		
		default:
			//i++;j++;
			if(c0!=cf)  //非通配字符内容串跟格式串要相同
				return false;
		}
		if(j>=pattern.length())
			break;; //匹配结束
	} //end for

	
	if(year.length()>0)
	{
		sYear = atoi(year.c_str());		
		sTm->tm_year = sYear>1900 ? sYear - 1900 : sYear;
		sTm->tm_year = sTm->tm_year > 29? sTm->tm_year:sTm->tm_year+100;
	}
	if(month.length()>0)
	{
		sMonth = atoi(month.c_str());
		sTm->tm_mon = sMonth > 0 ? sMonth -1 : 0;
	}
	if(day.length()>0)
		sTm->tm_mday = atoi(day.c_str());

	if(hour.length()>0)
		sTm->tm_hour = atoi(hour.c_str());

	if(minutes.length()>0)
		sTm->tm_min = atoi(minutes.c_str());

	if(second.length()>0)
		sTm->tm_sec = atoi(second.c_str());
	sStamp = mktime(sTm);

	return true;
}


#ifndef __CDateTime_H__
#define __CDateTime_H__

//#define __FOR_WIN__

#ifdef __FOR_WIN__
	#include <afxdisp.h>
#endif

#include <time.h>
#include <stdlib.h>
#include <string>
#include "StrToStamp.h"

using namespace std;


 
class CDateTime{
	//属性
private:
    unsigned short sYear;		    //年
    unsigned char sMonth;			//月
    unsigned char sDay;				//日
	
    unsigned char sHour;			//时
    unsigned char sMinute;			//分
    unsigned char sSecond;			//秒

	time_t sStamp;
	struct tm sc;
	struct tm *sTm;
	
private:

    //token类型定义
    typedef enum TokenType
    {
        TT_Null = 0,
		TT_Number =1,
		TT_Minus = 2,
		TT_Colon = 4,
		TT_Blank = 8
			
    };
    //日期时间类型定义
    typedef enum TimePart
    {
        TP_Second = 1,
		TP_Minute = 2,
		TP_Hour = 4,
		TP_Day = 8,
		TP_Month = 16,
		TP_Year = 32
    };
	
private: 
    //根据字符取得该字符所属的类型
    CDateTime::TokenType GetTokenType(char c);
	bool ParseDateTime(const char *timestring,const char *format);

//接口
public:
	CDateTime();
	~CDateTime();

#ifdef __FOR_WIN__
	time_t getTimeT(COleDateTime &tmptime);
#endif

	bool GetDateStrCurrentTimeAsDefault(char *input, char *output,bool decision = true,char * inputformat = NULL);
	bool GetDefaultDateStr(char *input, char *output,bool decision = true,char * inputformat = NULL);
	bool GetDateStr(char *input, char *format, char *output,bool decision = true,char * inputformat = NULL);
	bool GetTimestamp(char *input, time_t *t,bool decision = true,char * inputformat = NULL);
	
	void GetDefaultDateStr(char *output);
	bool GetDateStrCurrentTimeAsNoStyle(char *input, char *output,bool decision = true,char * inputformat = NULL);
	void GetDateStr(char *format, char *output);
	void GetTimestamp(time_t *t);
	void ClearHourMinuteSecond();
private:
    //重新为当前的日期时间
    bool LoadCurrenCDateTime();
    //转化为UNIX形式的time_t时间日期类型
    time_t ToUnixDatetime();
    //重新设定为有time_t类型变量指定的日期时间值
    void FromUnixDatetime(time_t t);
    //校验当前对象的日期时间数据是否正确
    bool Validate();
    //校验一个CDateTime类型变量的日期时间数据是否正确
    bool Validate(CDateTime *obDateTime);
    //检查年份是否是闰年
    bool IsLeapYear(int year);
    //校验给定的年份是否正确
    bool ValidateDate(int year);
    //校验给定的年份和月分是否正确
    bool ValidateDate(int year,int month);
    //取得给定的年份，给定的月份含有的天数
    int GetDaysOfMonth(int year, int month);
    //校验给定的年月日数据是否正确
    bool ValidateDate(int year, int month, int day);
    //检验给定的小时数据，是否正确
    bool ValidateTime(int hour);
    //校验给定的小时分钟数据是否正确
    bool ValidateTime(int hour,int minute);
    //校验给定的时间数据是否正确
    bool ValidateTime(int hour, int minute, int second);
    //校验给定的日期时间数据是否正确
    bool ValidateDateTime(int year, int month, int day, int hour, int minute, int second);
	void isOverCurTime();

private:
	bool defaultparse(const char *timestring);
	bool ParseStepFirst(string timestring);
	bool ParseStepSecond(string timestring);
	bool ParseStepThird(string timestring);
	bool ParseSteFourth(string timestring);
	bool ParseSteFifth(string timestring);

	char *strltrim(char *str, const char *trim);
	char *strrtrim(char *str, const char *trim);
	char *strtrim(char *str, const char *trim);
	bool FilteTag(const char *strSource,char *strDest); //过滤html标签
	void ReplaceSpace(string &input,const char *inputstr,const char *outstr);		//删除串中间的空白以及括号等
	//检验一个字符串是否time_t型时间字符串
	bool IsTime_T(const char *timestring,time_t &timestamp);
	bool parseFormatDate(string &s,string &pattern);

	bool parse_english(string timestring);
	bool parse_chinese(string text);
	int matchWordIgoreCase(string text, int i, string str);
	int indexOfIgoreCase(string text, string str);
	bool getYMD(string text, int month, int &p);		//得到年月日
	bool getHMS(string text);				//得到时分秒
	bool parseYMD(string text);
	int findDigit(string text, int begin);
	int findNonDigit(string text, int begin);
	int getHeaderDigit(string text, int p);
	bool isColon(string text, int b, int e);
	int textLen(string text, int b, int e);
	bool Findstr(string timestr,const stringToStamp *stringtostamp);
	int isMatchedDateFormat(string inputtime, string format, int p);
	int englishMonth(string text, int p);
	bool isHeaderOf(string text, int p,string head);
	bool isLetter(char c);
	static struct tm * localtime_r (const time_t *t, struct tm *tp);
	char *mystrtok_r(char *s, const char *delim, char **save_ptr);

};
#endif 

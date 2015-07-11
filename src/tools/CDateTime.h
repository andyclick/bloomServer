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
	//����
private:
    unsigned short sYear;		    //��
    unsigned char sMonth;			//��
    unsigned char sDay;				//��
	
    unsigned char sHour;			//ʱ
    unsigned char sMinute;			//��
    unsigned char sSecond;			//��

	time_t sStamp;
	struct tm sc;
	struct tm *sTm;
	
private:

    //token���Ͷ���
    typedef enum TokenType
    {
        TT_Null = 0,
		TT_Number =1,
		TT_Minus = 2,
		TT_Colon = 4,
		TT_Blank = 8
			
    };
    //����ʱ�����Ͷ���
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
    //�����ַ�ȡ�ø��ַ�����������
    CDateTime::TokenType GetTokenType(char c);
	bool ParseDateTime(const char *timestring,const char *format);

//�ӿ�
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
    //����Ϊ��ǰ������ʱ��
    bool LoadCurrenCDateTime();
    //ת��ΪUNIX��ʽ��time_tʱ����������
    time_t ToUnixDatetime();
    //�����趨Ϊ��time_t���ͱ���ָ��������ʱ��ֵ
    void FromUnixDatetime(time_t t);
    //У�鵱ǰ���������ʱ�������Ƿ���ȷ
    bool Validate();
    //У��һ��CDateTime���ͱ���������ʱ�������Ƿ���ȷ
    bool Validate(CDateTime *obDateTime);
    //�������Ƿ�������
    bool IsLeapYear(int year);
    //У�����������Ƿ���ȷ
    bool ValidateDate(int year);
    //У���������ݺ��·��Ƿ���ȷ
    bool ValidateDate(int year,int month);
    //ȡ�ø�������ݣ��������·ݺ��е�����
    int GetDaysOfMonth(int year, int month);
    //У������������������Ƿ���ȷ
    bool ValidateDate(int year, int month, int day);
    //���������Сʱ���ݣ��Ƿ���ȷ
    bool ValidateTime(int hour);
    //У�������Сʱ���������Ƿ���ȷ
    bool ValidateTime(int hour,int minute);
    //У�������ʱ�������Ƿ���ȷ
    bool ValidateTime(int hour, int minute, int second);
    //У�����������ʱ�������Ƿ���ȷ
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
	bool FilteTag(const char *strSource,char *strDest); //����html��ǩ
	void ReplaceSpace(string &input,const char *inputstr,const char *outstr);		//ɾ�����м�Ŀհ��Լ����ŵ�
	//����һ���ַ����Ƿ�time_t��ʱ���ַ���
	bool IsTime_T(const char *timestring,time_t &timestamp);
	bool parseFormatDate(string &s,string &pattern);

	bool parse_english(string timestring);
	bool parse_chinese(string text);
	int matchWordIgoreCase(string text, int i, string str);
	int indexOfIgoreCase(string text, string str);
	bool getYMD(string text, int month, int &p);		//�õ�������
	bool getHMS(string text);				//�õ�ʱ����
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

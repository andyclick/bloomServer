#ifndef __StrToStamp_H__
#define __StrToStamp_H__
const int TIMEZONE_8 = 8*60*60;
const int minuteseconds = 60;
const int hourseconds = 60*60;
const int dayseconds = 24*60*60;
const int weekseconds = 7*24*60*60;

typedef struct _stringToStamp{
	char *sstring;
	int stamp;
}stringToStamp;

const stringToStamp stringdaystamp[]={
	{"һ��ǰ", dayseconds * 1},{"1��ǰ", dayseconds * 1},
	{"����ǰ", dayseconds * 2},{"2��ǰ", dayseconds * 2},{"����ǰ", dayseconds * 2},
	{"����ǰ", dayseconds * 3},{"3��ǰ", dayseconds * 3},
	{"����ǰ", dayseconds * 4},{"4��ǰ", dayseconds * 4},
	{"����ǰ", dayseconds * 5},{"5��ǰ", dayseconds * 5},
	{"����ǰ", dayseconds * 6},{"6��ǰ", dayseconds * 6},
	
	{"",0}
};
const stringToStamp stringminstamp[]={	
	{"һ����ǰ", minuteseconds * 1},{"1����ǰ", minuteseconds * 1},
	{"������ǰ", minuteseconds * 2},{"2����ǰ", minuteseconds * 2},
	{"������ǰ", minuteseconds * 3},{"3����ǰ", minuteseconds * 3},
	{"�ķ���ǰ", minuteseconds * 4},{"4����ǰ", minuteseconds * 4},
	{"�����ǰ", minuteseconds * 5},{"5����ǰ", minuteseconds * 5},
	{"������ǰ", minuteseconds * 6},{"6����ǰ", minuteseconds * 6},
	{"�߷���ǰ", minuteseconds * 7},{"7����ǰ", minuteseconds * 7},
	{"�˷���ǰ", minuteseconds * 8},{"8����ǰ", minuteseconds * 8},
	{"�ŷ���ǰ", minuteseconds * 9},{"9����ǰ", minuteseconds * 9},
	
	{"ʮ����ǰ", minuteseconds * 10},{"10����ǰ", minuteseconds * 10},
	{"ʮһ����ǰ", minuteseconds * 11},{"11����ǰ", minuteseconds * 11},
	{"ʮ������ǰ", minuteseconds * 12},{"12����ǰ", minuteseconds * 12},
	{"ʮ������ǰ", minuteseconds * 13},{"13����ǰ", minuteseconds * 13},
	{"ʮ�ķ���ǰ", minuteseconds * 14},{"14����ǰ", minuteseconds * 14},
	{"ʮ�����ǰ", minuteseconds * 15},{"15����ǰ", minuteseconds * 15},
	{"ʮ������ǰ", minuteseconds * 16},{"16����ǰ", minuteseconds * 16},
	{"ʮ�߷���ǰ", minuteseconds * 17},{"17����ǰ", minuteseconds * 17},
	{"ʮ�˷���ǰ", minuteseconds * 18},{"18����ǰ", minuteseconds * 18},
	{"ʮ�ŷ���ǰ", minuteseconds * 19},{"19����ǰ", minuteseconds * 19},
	
	{"��ʮ����ǰ", minuteseconds * 20},{"10����ǰ", minuteseconds * 20},
	{"��ʮһ����ǰ", minuteseconds * 21},{"21����ǰ", minuteseconds * 21},
	{"��ʮ������ǰ", minuteseconds * 22},{"22����ǰ", minuteseconds * 22},
	{"��ʮ������ǰ", minuteseconds * 23},{"23����ǰ", minuteseconds * 23},
	{"��ʮ�ķ���ǰ", minuteseconds * 24},{"24����ǰ", minuteseconds * 24},
	{"��ʮ�����ǰ", minuteseconds * 25},{"25����ǰ", minuteseconds * 25},
	{"��ʮ������ǰ", minuteseconds * 26},{"26����ǰ", minuteseconds * 26},
	{"��ʮ�߷���ǰ", minuteseconds * 27},{"27����ǰ", minuteseconds * 27},
	{"��ʮ�˷���ǰ", minuteseconds * 28},{"28����ǰ", minuteseconds * 28},
	{"��ʮ�ŷ���ǰ", minuteseconds * 29},{"29����ǰ", minuteseconds * 29},
	
	{"��ʮ����ǰ", minuteseconds * 30},{"��Сʱǰ", minuteseconds * 30},{"30����ǰ", minuteseconds * 30},
	{"��ʮһ����ǰ", minuteseconds * 31},{"31����ǰ", minuteseconds * 31},
	{"��ʮ������ǰ", minuteseconds * 32},{"32����ǰ", minuteseconds * 32},
	{"��ʮ������ǰ", minuteseconds * 33},{"33����ǰ", minuteseconds * 33},
	{"��ʮ�ķ���ǰ", minuteseconds * 34},{"34����ǰ", minuteseconds * 34},
	{"��ʮ�����ǰ", minuteseconds * 35},{"35����ǰ", minuteseconds * 35},
	{"��ʮ������ǰ", minuteseconds * 36},{"36����ǰ", minuteseconds * 36},
	{"��ʮ�߷���ǰ", minuteseconds * 37},{"37����ǰ", minuteseconds * 37},
	{"��ʮ�˷���ǰ", minuteseconds * 38},{"38����ǰ", minuteseconds * 38},
	{"��ʮ�ŷ���ǰ", minuteseconds * 39},{"39����ǰ", minuteseconds * 39},

	{"��ʮ����ǰ", minuteseconds * 40},{"40����ǰ", minuteseconds * 40},
	{"��ʮһ����ǰ", minuteseconds * 41},{"41����ǰ", minuteseconds * 41},
	{"��ʮ������ǰ", minuteseconds * 42},{"42����ǰ", minuteseconds * 42},
	{"��ʮ������ǰ", minuteseconds * 43},{"43����ǰ", minuteseconds * 43},
	{"��ʮ�ķ���ǰ", minuteseconds * 44},{"44����ǰ", minuteseconds * 44},
	{"��ʮ�����ǰ", minuteseconds * 45},{"45����ǰ", minuteseconds * 45},
	{"��ʮ������ǰ", minuteseconds * 46},{"46����ǰ", minuteseconds * 46},
	{"��ʮ�߷���ǰ", minuteseconds * 47},{"47����ǰ", minuteseconds * 47},
	{"��ʮ�˷���ǰ", minuteseconds * 48},{"48����ǰ", minuteseconds * 48},
	{"��ʮ�ŷ���ǰ", minuteseconds * 49},{"49����ǰ", minuteseconds * 49},
	

	{"��ʮ����ǰ", minuteseconds * 50},{"50����ǰ", minuteseconds * 50},
	{"��ʮһ����ǰ", minuteseconds * 51},{"51����ǰ", minuteseconds * 51},
	{"��ʮ������ǰ", minuteseconds * 52},{"52����ǰ", minuteseconds * 52},
	{"��ʮ������ǰ", minuteseconds * 53},{"53����ǰ", minuteseconds * 53},
	{"��ʮ�ķ���ǰ", minuteseconds * 54},{"54����ǰ", minuteseconds * 54},
	{"��ʮ�����ǰ", minuteseconds * 55},{"55����ǰ", minuteseconds * 55},
	{"��ʮ������ǰ", minuteseconds * 56},{"56����ǰ", minuteseconds * 56},
	{"��ʮ�߷���ǰ", minuteseconds * 57},{"57����ǰ", minuteseconds * 57},
	{"��ʮ�˷���ǰ", minuteseconds * 58},{"58����ǰ", minuteseconds * 58},
	{"��ʮ�ŷ���ǰ", minuteseconds * 59},{"59����ǰ", minuteseconds * 59},
	
	{"",0}
};
const stringToStamp stringhourstamp[]={	
	{"һСʱǰ", hourseconds * 1},{"1Сʱǰ", hourseconds * 1},
	{"��Сʱǰ", hourseconds * 2},{"2Сʱǰ", hourseconds * 2},
	{"��Сʱǰ", hourseconds * 3},{"3Сʱǰ", hourseconds * 3},
	{"��Сʱǰ", hourseconds * 4},{"4Сʱǰ", hourseconds * 4},
	{"��Сʱǰ", hourseconds * 5},{"5Сʱǰ", hourseconds * 5},
	{"��Сʱǰ", hourseconds * 6},{"6Сʱǰ", hourseconds * 6},
	{"��Сʱǰ", hourseconds * 7},{"7Сʱǰ", hourseconds * 7},
	{"��Сʱǰ", hourseconds * 8},{"8Сʱǰ", hourseconds * 8},
	{"��Сʱǰ", hourseconds * 9},{"9Сʱǰ", hourseconds * 9},
	{"ʮСʱǰ", hourseconds * 10},{"10Сʱǰ", hourseconds * 10},

	{"ʮһСʱǰ", hourseconds * 11},{"11Сʱǰ", hourseconds * 11},
	{"ʮ��Сʱǰ", hourseconds * 12},{"12Сʱǰ", hourseconds * 12},
	{"ʮ��Сʱǰ", hourseconds * 13},{"13Сʱǰ", hourseconds * 13},
	{"ʮ��Сʱǰ", hourseconds * 14},{"14Сʱǰ", hourseconds * 14},
	{"ʮ��Сʱǰ", hourseconds * 15},{"15Сʱǰ", hourseconds * 15},
	{"ʮ��Сʱǰ", hourseconds * 16},{"16Сʱǰ", hourseconds * 16},
	{"ʮ��Сʱǰ", hourseconds * 17},{"17Сʱǰ", hourseconds * 17},
	{"ʮ��Сʱǰ", hourseconds * 18},{"18Сʱǰ", hourseconds * 18},
	{"ʮ��Сʱǰ", hourseconds * 19},{"19Сʱǰ", hourseconds * 19},

	{"��ʮСʱǰ", hourseconds * 20},{"20Сʱǰ", hourseconds * 20},
	{"��ʮһСʱǰ", hourseconds * 21},{"21Сʱǰ", hourseconds * 21},
	{"��ʮ��Сʱǰ", hourseconds * 22},{"22Сʱǰ", hourseconds * 22},
	{"��ʮ��Сʱǰ", hourseconds * 23},{"23Сʱǰ", hourseconds * 23},
	{"",0}
};
const stringToStamp stringweekstamp[]={	
	{"һ��ǰ", weekseconds * 1},{"1��ǰ", weekseconds * 1},
	{"����ǰ", weekseconds * 2},{"2��ǰ", weekseconds * 2},
	{"����ǰ", weekseconds * 3},{"3��ǰ", weekseconds * 3},
	{"����ǰ", weekseconds * 4},{"4��ǰ", weekseconds * 4},
	
	{"",0}
	
};
#endif
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
	{"一天前", dayseconds * 1},{"1天前", dayseconds * 1},
	{"二天前", dayseconds * 2},{"2天前", dayseconds * 2},{"两天前", dayseconds * 2},
	{"三天前", dayseconds * 3},{"3天前", dayseconds * 3},
	{"四天前", dayseconds * 4},{"4天前", dayseconds * 4},
	{"五天前", dayseconds * 5},{"5天前", dayseconds * 5},
	{"六天前", dayseconds * 6},{"6天前", dayseconds * 6},
	
	{"",0}
};
const stringToStamp stringminstamp[]={	
	{"一分钟前", minuteseconds * 1},{"1分钟前", minuteseconds * 1},
	{"二分钟前", minuteseconds * 2},{"2分钟前", minuteseconds * 2},
	{"三分钟前", minuteseconds * 3},{"3分钟前", minuteseconds * 3},
	{"四分钟前", minuteseconds * 4},{"4分钟前", minuteseconds * 4},
	{"五分钟前", minuteseconds * 5},{"5分钟前", minuteseconds * 5},
	{"六分钟前", minuteseconds * 6},{"6分钟前", minuteseconds * 6},
	{"七分钟前", minuteseconds * 7},{"7分钟前", minuteseconds * 7},
	{"八分钟前", minuteseconds * 8},{"8分钟前", minuteseconds * 8},
	{"九分钟前", minuteseconds * 9},{"9分钟前", minuteseconds * 9},
	
	{"十分钟前", minuteseconds * 10},{"10分钟前", minuteseconds * 10},
	{"十一分钟前", minuteseconds * 11},{"11分钟前", minuteseconds * 11},
	{"十二分钟前", minuteseconds * 12},{"12分钟前", minuteseconds * 12},
	{"十三分钟前", minuteseconds * 13},{"13分钟前", minuteseconds * 13},
	{"十四分钟前", minuteseconds * 14},{"14分钟前", minuteseconds * 14},
	{"十五分钟前", minuteseconds * 15},{"15分钟前", minuteseconds * 15},
	{"十六分钟前", minuteseconds * 16},{"16分钟前", minuteseconds * 16},
	{"十七分钟前", minuteseconds * 17},{"17分钟前", minuteseconds * 17},
	{"十八分钟前", minuteseconds * 18},{"18分钟前", minuteseconds * 18},
	{"十九分钟前", minuteseconds * 19},{"19分钟前", minuteseconds * 19},
	
	{"二十分钟前", minuteseconds * 20},{"10分钟前", minuteseconds * 20},
	{"二十一分钟前", minuteseconds * 21},{"21分钟前", minuteseconds * 21},
	{"二十二分钟前", minuteseconds * 22},{"22分钟前", minuteseconds * 22},
	{"二十三分钟前", minuteseconds * 23},{"23分钟前", minuteseconds * 23},
	{"二十四分钟前", minuteseconds * 24},{"24分钟前", minuteseconds * 24},
	{"二十五分钟前", minuteseconds * 25},{"25分钟前", minuteseconds * 25},
	{"二十六分钟前", minuteseconds * 26},{"26分钟前", minuteseconds * 26},
	{"二十七分钟前", minuteseconds * 27},{"27分钟前", minuteseconds * 27},
	{"二十八分钟前", minuteseconds * 28},{"28分钟前", minuteseconds * 28},
	{"二十九分钟前", minuteseconds * 29},{"29分钟前", minuteseconds * 29},
	
	{"三十分钟前", minuteseconds * 30},{"半小时前", minuteseconds * 30},{"30分钟前", minuteseconds * 30},
	{"三十一分钟前", minuteseconds * 31},{"31分钟前", minuteseconds * 31},
	{"三十二分钟前", minuteseconds * 32},{"32分钟前", minuteseconds * 32},
	{"三十三分钟前", minuteseconds * 33},{"33分钟前", minuteseconds * 33},
	{"三十四分钟前", minuteseconds * 34},{"34分钟前", minuteseconds * 34},
	{"三十五分钟前", minuteseconds * 35},{"35分钟前", minuteseconds * 35},
	{"三十六分钟前", minuteseconds * 36},{"36分钟前", minuteseconds * 36},
	{"三十七分钟前", minuteseconds * 37},{"37分钟前", minuteseconds * 37},
	{"三十八分钟前", minuteseconds * 38},{"38分钟前", minuteseconds * 38},
	{"三十九分钟前", minuteseconds * 39},{"39分钟前", minuteseconds * 39},

	{"四十分钟前", minuteseconds * 40},{"40分钟前", minuteseconds * 40},
	{"四十一分钟前", minuteseconds * 41},{"41分钟前", minuteseconds * 41},
	{"四十二分钟前", minuteseconds * 42},{"42分钟前", minuteseconds * 42},
	{"四十三分钟前", minuteseconds * 43},{"43分钟前", minuteseconds * 43},
	{"四十四分钟前", minuteseconds * 44},{"44分钟前", minuteseconds * 44},
	{"四十五分钟前", minuteseconds * 45},{"45分钟前", minuteseconds * 45},
	{"四十六分钟前", minuteseconds * 46},{"46分钟前", minuteseconds * 46},
	{"四十七分钟前", minuteseconds * 47},{"47分钟前", minuteseconds * 47},
	{"四十八分钟前", minuteseconds * 48},{"48分钟前", minuteseconds * 48},
	{"四十九分钟前", minuteseconds * 49},{"49分钟前", minuteseconds * 49},
	

	{"五十分钟前", minuteseconds * 50},{"50分钟前", minuteseconds * 50},
	{"五十一分钟前", minuteseconds * 51},{"51分钟前", minuteseconds * 51},
	{"五十二分钟前", minuteseconds * 52},{"52分钟前", minuteseconds * 52},
	{"五十三分钟前", minuteseconds * 53},{"53分钟前", minuteseconds * 53},
	{"五十四分钟前", minuteseconds * 54},{"54分钟前", minuteseconds * 54},
	{"五十五分钟前", minuteseconds * 55},{"55分钟前", minuteseconds * 55},
	{"五十六分钟前", minuteseconds * 56},{"56分钟前", minuteseconds * 56},
	{"五十七分钟前", minuteseconds * 57},{"57分钟前", minuteseconds * 57},
	{"五十八分钟前", minuteseconds * 58},{"58分钟前", minuteseconds * 58},
	{"五十九分钟前", minuteseconds * 59},{"59分钟前", minuteseconds * 59},
	
	{"",0}
};
const stringToStamp stringhourstamp[]={	
	{"一小时前", hourseconds * 1},{"1小时前", hourseconds * 1},
	{"二小时前", hourseconds * 2},{"2小时前", hourseconds * 2},
	{"三小时前", hourseconds * 3},{"3小时前", hourseconds * 3},
	{"四小时前", hourseconds * 4},{"4小时前", hourseconds * 4},
	{"五小时前", hourseconds * 5},{"5小时前", hourseconds * 5},
	{"六小时前", hourseconds * 6},{"6小时前", hourseconds * 6},
	{"七小时前", hourseconds * 7},{"7小时前", hourseconds * 7},
	{"八小时前", hourseconds * 8},{"8小时前", hourseconds * 8},
	{"九小时前", hourseconds * 9},{"9小时前", hourseconds * 9},
	{"十小时前", hourseconds * 10},{"10小时前", hourseconds * 10},

	{"十一小时前", hourseconds * 11},{"11小时前", hourseconds * 11},
	{"十二小时前", hourseconds * 12},{"12小时前", hourseconds * 12},
	{"十三小时前", hourseconds * 13},{"13小时前", hourseconds * 13},
	{"十四小时前", hourseconds * 14},{"14小时前", hourseconds * 14},
	{"十五小时前", hourseconds * 15},{"15小时前", hourseconds * 15},
	{"十六小时前", hourseconds * 16},{"16小时前", hourseconds * 16},
	{"十七小时前", hourseconds * 17},{"17小时前", hourseconds * 17},
	{"十八小时前", hourseconds * 18},{"18小时前", hourseconds * 18},
	{"十九小时前", hourseconds * 19},{"19小时前", hourseconds * 19},

	{"二十小时前", hourseconds * 20},{"20小时前", hourseconds * 20},
	{"二十一小时前", hourseconds * 21},{"21小时前", hourseconds * 21},
	{"二十二小时前", hourseconds * 22},{"22小时前", hourseconds * 22},
	{"二十三小时前", hourseconds * 23},{"23小时前", hourseconds * 23},
	{"",0}
};
const stringToStamp stringweekstamp[]={	
	{"一周前", weekseconds * 1},{"1周前", weekseconds * 1},
	{"二周前", weekseconds * 2},{"2周前", weekseconds * 2},
	{"三周前", weekseconds * 3},{"3周前", weekseconds * 3},
	{"四周前", weekseconds * 4},{"4周前", weekseconds * 4},
	
	{"",0}
	
};
#endif
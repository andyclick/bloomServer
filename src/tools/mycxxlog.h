//#ifndef _MYLOG_H_
//#define _MYLOG_H_ 
#if defined(_MSC_VER)
	#pragma warning(disable : 4786)
#endif
#if !defined(__MYCXXLOG__)
#define __MYCXXLOG__

#include <string>
#include <stdio.h>
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/xml/domconfigurator.h>
using namespace std;
using namespace log4cxx;
using namespace log4cxx::helpers;

#define ANSI            /* Comment out for UNIX version     */
#ifdef ANSI             /* ANSI compatible version          */
#include <stdarg.h>
#else                   /* UNIX compatible version          */
#include <varargs.h>
#endif

//#define LOG4CPP_HAVE_SNPRINTF
//#if defined(_MSC_VER)
  //  #define VSNPRINTF _vsnprintf
//#else
//#ifdef LOG4CPP_HAVE_SNPRINTF
    #define VSNPRINTF vsnprintf
//#else
/* use alternative snprintf() from http://www.ijs.si/software/snprintf/ */

//#define HAVE_SNPRINTF
//#define PREFER_PORTABLE_SNPRINTF

//#include <stdlib.h>
//#include <stdarg.h>

//extern "C" {
//#include "snprintf.c"
//}

//#define VSNPRINTF portable_vsnprintf

//#endif // LOG4CPP_HAVE_SNPRINTF
//#endif // _MSC_VER

void mylog_info(LoggerPtr logger, const char *stringformat, ...);
void mylog_debug(LoggerPtr logger, const char *stringformat, ...);
void mylog_trace(LoggerPtr logger, const char *stringformat, ...);
void mylog_fatal(LoggerPtr logger, const char *stringformat, ...);
void mylog_error(LoggerPtr logger, const char *stringformat, ...);
void mylog_fatal_errno(LoggerPtr logger,char *s);
void mylog_error_errno(LoggerPtr logger,char * s);
#endif //__MYCXXLOG__
//#endif //MYCXXLOG

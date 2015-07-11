#include <string.h>
#include "mycxxlog.h"
#include <signal.h>
#include "errno.h"

std::string myvalistform(const char* format, va_list args) {
	size_t size = 2048;
	char* buffer = new char[size];

	while (1) {
		//int n = VSNPRINTF(buffer, format, args);
		int n = VSNPRINTF(buffer, size, format, args);

		// If that worked, return a string.
		if ((n > -1) && (static_cast<size_t>(n) < size)) {
	    	std::string s(buffer);
		    delete [] buffer;
		    return s;
		}

		// Else try again with more space.
			size = (n > -1) ?
				n + 1 :   // ISO/IEC 9899:1999
				size * 2; // twice the old size

		delete [] buffer;
		buffer = new char[size];
	}
}

void myexit(LoggerPtr logger, int i) {
    //exit(i);
    int err = errno;
    mylog_error(logger, "due to %d, raise sigsegv - %s:%s:%d\n",err ,__FILE__,__FUNCTION__,__LINE__);
    raise(SIGSEGV);
    //int o = 1/0;
}
void mylog_info(LoggerPtr logger, const char *stringformat, ...) {
	va_list va;
	va_start(va, stringformat);
	std::string buf = myvalistform(stringformat, va);
	va_end(va);
	logger->info(buf);
}

void mylog_debug(LoggerPtr logger, const char *stringformat, ...) {
	va_list va;
	va_start(va, stringformat);
	std::string buf = myvalistform(stringformat, va);
	va_end(va);
	logger->debug(buf);
}

void mylog_trace(LoggerPtr logger, const char *stringformat, ...) {
	va_list va;
	va_start(va, stringformat);
	std::string buf = myvalistform(stringformat, va);
	va_end(va);
	logger->trace(buf);
}

void mylog_fatal(LoggerPtr logger, const char *stringformat, ...) {
	va_list va;
    va_start(va, stringformat);
	std::string buf = myvalistform(stringformat, va);
	va_end(va);
	logger->fatal(buf);
    myexit(logger,1);
}

void mylog_error(LoggerPtr logger, const char *stringformat, ...) {
	va_list va;
	va_start(va, stringformat);
	std::string buf = myvalistform(stringformat, va);
	va_end(va);
	logger->error(buf);
}

void mylog_fatal_errno(LoggerPtr logger,char * s)
{
    mylog_fatal(logger,"%s: %s - %s:%s:%d\n",s, strerror(errno),__FILE__,__FUNCTION__,__LINE__);
}


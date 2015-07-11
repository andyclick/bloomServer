#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include "config.h"
//#include "ic_types.h"
//#include"InfoCrawler.h"

/*defines ********************************************/
#define DIR_BUF_LEN 1024
#define LINE_SEPERATOR " " //the end of the line
#define LINE_SEPERATOR_CHAR ' ' //the end of the line
#define LINE_DELIMS "\n" //the end of the line
#define LINE_DELIMS_CHAR '\n' //the end of the line

#define NullS       (char *) 0
#ifndef NEAR
#define NEAR                /* Who needs segments ? */
#define FAR             /* On a good machine */
#ifndef HUGE_PTR
#define HUGE_PTR
#endif
#endif
#if defined(__IBMC__) || defined(__IBMCPP__)
/* This was  _System _Export but caused a lot of warnings on _AIX43 */
#define STDCALL
#elif !defined( STDCALL)
#define STDCALL
#endif

#if defined(__i386__)
# undef WORDS_BIGENDIAN
# define SIZEOF_CHARP 4
# define SIZEOF_INT 4
# define SIZEOF_LONG 4
# define SIZEOF_LONG_LONG 8
# define SIZEOF_OFF_T 8
# define SIZEOF_SHORT 2
# define HAVE_LONG_LONG
typedef long long __int64;

#elif defined(__ppc__)
# define WORDS_BIGENDIAN
# define SIZEOF_CHARP 4
# define SIZEOF_INT 4
# define SIZEOF_LONG 4
# define SIZEOF_LONG_LONG 8
# define SIZEOF_OFF_T 8
# define SIZEOF_SHORT 2

#elif defined(__x86_64__)
# define WORDS_BIGENDIAN
# define SIZEOF_CHARP 4
# define SIZEOF_INT 4
# define SIZEOF_LONG 8 
# define SIZEOF_LONG_LONG 16     
# define SIZEOF_OFF_T 8
# define SIZEOF_SHORT 2
typedef long int __int64;

#else
# error Building FAT binary for an unknown architecture.
#endif

/* Typdefs for easyier portability */

#if defined(VOIDTYPE)
typedef void    *gptr;      /* Generic pointer */
#else
typedef char    *gptr;      /* Generic pointer */
#endif

#ifndef HAVE_INT_8_16_32
typedef signed char int8;       /* Signed integer >= 8  bits */
typedef short   int16;      /* Signed integer >= 16 bits */
#endif
#ifndef HAVE_UCHAR
typedef unsigned char   uchar;  /* Short for unsigned char */
#endif
typedef unsigned char   uint8;  /* Short for unsigned integer >= 8  bits */
typedef unsigned short  uint16; /* Short for unsigned integer >= 16 bits */

#if SIZEOF_INT == 4
#ifndef HAVE_INT_8_16_32
typedef int     int32;
#endif
typedef unsigned int    uint32; /* Short for unsigned integer >= 32 bits */
#elif SIZEOF_LONG == 4
#ifndef HAVE_INT_8_16_32
typedef long        int32;
#endif
typedef unsigned long   uint32; /* Short for unsigned integer >= 32 bits */
#else
#error "Neither int or long is of 4 bytes width"
#endif

#if !defined(HAVE_ULONG) && !defined(TARGET_OS_LINUX) && !defined(__USE_MISC)
typedef unsigned long   ulong;        /* Short for unsigned long */
#endif
#ifndef longlong_defined

/* 
 *   Using [unsigned] long long is preferable as [u]longlong because we use 
 *     [unsigned] long long unconditionally in many places, 
 *       for example in constants with [U]LL suffix.
 *       */
#if defined(HAVE_LONG_LONG) && SIZEOF_LONG_LONG == 8
typedef unsigned long long int ulonglong; /* ulong or unsigned long long */
typedef long long int   longlong;
#else
typedef unsigned long   ulonglong;    /* ulong or unsigned long long */
typedef long        longlong;
#endif
#endif

#if defined(NO_CLIENT_LONG_LONG)
typedef unsigned long my_ulonglong;
#elif defined (__WIN__)
typedef unsigned __int64 my_ulonglong;
#else
typedef unsigned long long my_ulonglong;
#endif

/****************************************************/

/* Buffer structure for buffer management routines */
struct gbuf {           /* Growing and shrinking buffer */
    char *data;     /* Data buffer */
    int length;     /* Current length of data buffer */
    int size;       /* Size allocated in the Data buffer */
    int default_size;   /* Default size of the Data buffer */
};
typedef struct gbuf Buffer; /* Growing buffer */

#ifndef _PARAMS
#if defined(__STDC__) || defined(__cplusplus) || defined(__STRICT_ANSI__)
#define _PARAMS(ARGS) ARGS
#else /* Traditional C */
#define _PARAMS(ARGS) ()
#endif /* __STDC__ */
#endif /* _PARAMS */


/* Copy one word once and then copy one byte once. */
#define MEMCPY_PLUS_PLUS(dest, src, n) \
({                                                  \
     void *__end = (char *)(dest) + (n);             \
     while ((long *)(dest) + 1 <= (long *)__end){     \
         *((long *)(dest)) = *((long *)(src));   \
         dest = dest + sizeof(long) ; \
         src = src + sizeof(long);}   \
     while ((char *)(dest) + 1 <= (char *)__end){     \
         *((char *)(dest)) = *((char *)(src));   \
         dest = dest + sizeof(char);    \
         src = src +sizeof(char);}   \
     dest;                                           \
 })

#define MEMCPY_PLUS(dest, src, n) \
({                                                  \
     const void *__src = (src);                      \
     MEMCPY_PLUS_PLUS(dest, __src, n);               \
 })

#define FREE_NOT_NULL(ptr) \
do {                                                \
        if (ptr) free(ptr);                             \
} while (0)

#define MIN(x, y)       ((x) < (y) ? (x) : (y))
#define MAX(x, y)       ((x) > (y) ? (x) : (y))


#ifdef USE_NO_DEBUGGING
#define ____BEGINTIMER
#define ____ENDTIMER
#else
#define ____BEGINTIMER \
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl(); \
    int timer_second_begin; \
    int timer_micro_begin; \
    getNow1(&timer_second_begin, &timer_micro_begin);
#define ____ENDTIMER \
    int timer_second_end; \
    int timer_micro_end; \
    getNow1(&timer_second_end, &timer_micro_end); \
    mylog_info(m_pLogGlobalCtrl->infolog, "timer  %d %d %d %d %d %d - %s:%s:%d\n",timer_second_begin, timer_micro_begin, timer_second_end, timer_micro_end,  timer_second_end - timer_second_begin, timer_micro_end - timer_micro_begin,INFO_LOG_SUFFIX);
#define ____BEGINTIMER1 {\
        LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl(); \
        int timer_second_begin; \
        int timer_micro_begin; \
        getNow1(&timer_second_begin, &timer_micro_begin);
#define ____ENDTIMER1 \
        int timer_second_end; \
        int timer_micro_end; \
        getNow1(&timer_second_end, &timer_micro_end);\
        mylog_info(m_pLogGlobalCtrl->infolog, "timer  %d %d %d %d %d %d - %s:%s:%d\n",timer_second_begin, timer_micro_begin, timer_second_end,timer_micro_end, timer_second_end - timer_second_begin, timer_micro_end - timer_micro_begin,INFO_LOG_SUFFIX);
#endif

/* from log.c */
#ifdef  __cplusplus
extern "C" {
#endif

void init_log(const char *a, const char *b, const char *dir);    /* Initialize log routines */
void init_log3(char *,FILE *,FILE *); /* Initialize log routines */
void log_errno(char *);       /* Same as perror(3) */
void log_errno2(char *,int,char *);   /* Same as perror(3) file,line*/
void fatal_errno(char *);     /* Same as perror(3) & exit */
void log_destroy();         /*destroy log*/
FILE *get_fp_log(); 
FILE *get_fp_errs(); 

//#ifdef __STRICT_ANSI__
#include <stdarg.h>
void Log(char *, ...);        /* Log a message */
void errorlog(char *, ...);       /* Log an error message */
void fatal(char *, ...);      /* Log error msg and exit */
//#else
//void Log _PARAMS(());
//void errorlog _PARAMS(());
//void fatal _PARAMS(());
//#endif

/* from strdup.c */
#ifdef NO_STRDUP
char *strdup(char *);         /* Duplicate a string */
#endif
char *xstrdup(char *);        /* Duplicate a string */

/* from system.c */
int do_system(char *);        /* Wrapper for system(3) */
int run_cmd(char *);          /* Simple system(3) */
int do_system_lifetime(char *, int);  /* Limited system(3) */
void close_all_fds(int);      /* Closes all fd's */
void close_all_fds_except(int,int*);  /* Closes all fd's except */
void setsocket_linger(int,int);   /* set SO_LINGER */

/* from buffer.c */
Buffer *create_buffer(int);       /* New buffer */
void grow_buffer(Buffer *);       /* Increase buffer size */
void increase_buffer(Buffer *, int);  /* Increase buffer size */
void shrink_buffer(Buffer *);     /* Reduce buffer size */
void add_buffer(Buffer *, char *, int);/* Add data to a buffer */
void free_buffer(Buffer *);       /* Clean up a buffer */

/* from debug.c */
#ifndef MAX_DEBUG_LEVELS
#define MAX_DEBUG_LEVELS 256
#endif

extern int do_debug;
extern int debug_levels[];

#undef debug_ok_fast
#ifdef USE_NO_DEBUGGING
#define debug_ok_fast(S,L) 0 /* empty */
#else
#define debug_ok_fast(S,L) \
        ( \
                  (do_debug) && \
                  ((debug_levels[S] == -2) || \
                            ((debug_levels[S] != -1) && \
                                    ((L) <= debug_levels[S]))) \
              )

#endif


#undef Debug
#ifdef USE_NO_DEBUGGING
#define Debug(section, level, X) /* empty */;
#else
#define Debug(section, level, X) \
            {if (debug_ok_fast((section),(level))) {Log X;}} /* no parens */
#endif

extern void debug_reset(void);
extern void debug_enable(int, int);
extern void debug_disable(int);
extern void debug_flag(char *);
extern int  debug_ok(int, int);
extern void debug_init(void);

/* from host.c */                                                
char *getfullhostname();      /* Fully qualified hostname */
char *getmylogin();           /* getlogin(3) clone */ 
char *getrealhost(char *);        /* Real DNS hostname */


/* from host_cache.c */
#define HOST_CACHE_TTL 3600

/* Some versions of Solaris with BIND don't define MAXHOSTNAMELEN */
#ifdef MAXHOSTNAMELEN
#undef MAXHOSTNAMELEN
#endif
#define MAXHOSTNAMELEN 254

typedef struct _host {
        char    key[MAXHOSTNAMELEN];    // www.bar.com 
        char        fqdn[MAXHOSTNAMELEN];   // real.bar.com 
        char    dotaddr[16];        // 128.138.213.10 
        char    ipaddr[4];
        time_t      last_t;         // last access of this info 
        int         n;          // # of requests for this host 
        int         addrlen;        // length of 'ipaddr', always 4 
        struct _host *next;
} Host;

extern Host   *thisHost;

int   host_cache_init();
Host  *get_host(char *hostname);
int   delete_host(Host *h);
int   expire_host_cache(time_t timeout);
void  dump_host_cache(int, int);
Host *copy_host(Host *src); 

char *split(char* string, int string_length, char*** splitted, int* count, char separator, int trim);
/*
 *  * String is altered (null replace the separator) and pointers returned point
 *   * to the original string.
 *    */
void split_inplace(char* string, int string_length, char*** splitted, int* count, char separator, int trim);

char* smalloc(int size);
char* srealloc(char* pointer, int size);
void static_alloc(char** tmp, int* tmp_size, int new_size);

char *int2str(long val, char *dst, int radix, int upcase);
void make_path(char *pathname);
void pthread_sleep(int seconds); 
//write pid to file 
pid_t write_pid(const char *pidfile);
pid_t read_pid(const char *pidfile);
//compress  and uncompress
int def(char *src, size_t srclen, char *des, size_t *deslen, int level); 
int inf(char *src, size_t srclen, char **des, size_t *deslen); 

void close_all_fds_except(int start, int *e);
int do_system(char *cmd);
void setsocket_linger(int s, int t);
void close_all_fds(int start);
void close_all_fds(int start);
void sig_phandler();
void sigpipe(int);
void my_sleep(ulong m_seconds);
ulonglong getNow(); 
int my_hash(char *key, int len);
char *strdupn(const char *string, unsigned int n);
char *strtrim(char *str, const char *trim);
void getNow1(int *second, int *micro); 
int URLencode (const char * plain, char * encode, int maxlen);
int MyURLencode (const char * plain, char * encode, int maxlen);
int MyURLBlankencode (const char * plain, char * encode, int maxlen);
void SetCoreDump();
void str_tolower(char *str, size_t len);
void print_result(char *path);
void myrmdir(char * path);
void myrmdir1(const char * name);
void chengtimeTodate(time_t timein, char *dateout);
time_t changeTime(char *date2);
void urlInCode(const char *pD,char* out);

#ifdef WANT_OUR_ATOI
int atoi(const char *src);
int atol(const char *src);
#endif //WANT_OUR_ATOI

#ifdef  __cplusplus
}
#endif


#endif //_UTIL_H_


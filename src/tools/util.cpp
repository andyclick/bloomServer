#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>      
#include <sys/socket.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <zlib.h>
#include <math.h>
#include "util.h"
#include "ic_types.h"
#include <sys/resource.h>
#include <dirent.h>
#include"InfoCrawler.h"

char* split(char* string, int string_length, char*** splitted, int* count, char separator, int trim)
{
    char* split_buffer = 0;
    int split_buffer_size = 0;
    if(split_buffer_size == 0) {
        split_buffer_size = string_length + 512;
        split_buffer = (char*)smalloc(split_buffer_size);
    }
    if(split_buffer_size < string_length) {
        split_buffer_size = string_length + 512;
        split_buffer = (char*)srealloc(split_buffer, split_buffer_size);
    }
    memcpy(split_buffer, string, string_length);
    split_buffer[string_length] = '\0';

    split_inplace(split_buffer, string_length, splitted, count, separator, trim);
    return split_buffer;
}

void split_inplace(char* split_buffer, int split_buffer_length, char*** splitted, int* count, char separator, int trim)
{
    char* first;
    int index;
    char** split_array = 0;
    int split_array_size = 128;
    int split_array_real_size = 0;

    first = split_buffer;
    if(trim) {
        int last = split_buffer_length - 1;
        /*
         * Remove trailing separators.
         */
        while(last >= 0 && split_buffer[last] == separator) {
            split_buffer[last] = '\0';
            last--;
        }
        /*
         * Remove leading separators. 
         */
        while(*first == separator)
            first++;
    }

    static_alloc((char**)&split_array, &split_array_real_size, split_array_size * sizeof(char*));

    index = 0;
    split_array[index++] = first;
    {
        char* p = first;
        while((p = strchr(p, separator))) {
            *p = '\0';
            p++;
            split_array[index++] = p;
            if(index >= split_array_size) {
                split_array_size += 128;
                static_alloc((char**)&split_array, &split_array_real_size, split_array_size * sizeof(char*));
            }
        }
    }
    *count = index;
    *splitted = split_array;
}

char* smalloc(int size)
{
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    char* tmp = (char*)malloc(size);
    if(tmp == 0) {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "cannot malloc %d bytes - %s:%s:%d",size ,INFO_LOG_SUFFIX);
        exit(1);
    }
    tmp[0] = '\0';
    return tmp;
} 

char* srealloc(char* pointer, int size)
{
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    char* tmp = (char*)realloc(pointer, size);
    if(tmp == 0) {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "cannot realloc %d bytes - %s:%s:%d",size ,INFO_LOG_SUFFIX);
        exit(1);
    }
    return tmp;
}

void static_alloc(char** tmp, int* tmp_size, int new_size)
{
    if(*tmp_size == 0) {
        *tmp_size = 16;
        *tmp = (char*)smalloc(*tmp_size);
    }
    if(*tmp_size < new_size) {
        *tmp_size = new_size + 1;
        *tmp = (char*)srealloc(*tmp, *tmp_size);
    }
}



/*
   _dig_vec arrays are public because they are used in several outer places.
   */
char NEAR _dig_vec_upper[] =
"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char NEAR _dig_vec_lower[] =
"0123456789abcdefghijklmnopqrstuvwxyz";


/*
   Convert integer to its string representation in given scale of notation.

   SYNOPSIS
   int2str()
   val     - value to convert
   dst     - points to buffer where string representation should be stored
   radix   - radix of scale of notation
   upcase  - set to 1 if we should use upper-case digits

   DESCRIPTION
   Converts the (long) integer value to its character form and moves it to 
   the destination buffer followed by a terminating NUL. 
   If radix is -2..-36, val is taken to be SIGNED, if radix is  2..36, val is
   taken to be UNSIGNED. That is, val is signed if and only if radix is. 
   All other radixes treated as bad and nothing will be changed in this case.

   For conversion to decimal representation (radix is -10 or 10) one can use
   optimized int10_to_str() function.

   RETURN VALUE
   Pointer to ending NUL character or NullS if radix is bad.
   */

char *
int2str(register long int val, register char *dst, register int radix, 
    int upcase)
{
    char buffer[65];
    register char *p;
    long int new_val;
    char *dig_vec= upcase ? _dig_vec_upper : _dig_vec_lower;

    if (radix < 0)
    {
        if (radix < -36 || radix > -2)
            return NullS;
        if (val < 0)
        {
            *dst++ = '-';
            val = -val;
        }
        radix = -radix;
    }
    else if (radix > 36 || radix < 2)
        return NullS;

    /*
       The slightly contorted code which follows is due to the fact that
       few machines directly support unsigned long / and %.  Certainly
       the VAX C compiler generates a subroutine call.  In the interests
       of efficiency (hollow laugh) I let this happen for the first digit
       only; after that "val" will be in range so that signed integer
       division will do.  Sorry 'bout that.  CHECK THE CODE PRODUCED BY
       YOUR C COMPILER.  The first % and / should be unsigned, the second
       % and / signed, but C compilers tend to be extraordinarily
       sensitive to minor details of style.  This works on a VAX, that's
       all I claim for it.
       */
    p = &buffer[sizeof(buffer)-1];
    *p = '\0';
    new_val=(ulong) val / (ulong) radix;
    *--p = dig_vec[(uchar) ((ulong) val- (ulong) new_val*(ulong) radix)];
    val = new_val;
#ifdef HAVE_LDIV
    while (val != 0)
    {
        ldiv_t res;
        res=ldiv(val,radix);
        *--p = dig_vec[res.rem];
        val= res.quot;
    }
#else
    while (val != 0)
    {
        new_val=val/radix;
        *--p = dig_vec[(uchar) (val-new_val*radix)];
        val= new_val;
    }
#endif
    while ((*dst++ = *p++) != 0) ;
    return dst-1;
}


/*
   Converts integer to its string representation in decimal notation.

   SYNOPSIS
   int10_to_str()
   val     - value to convert
   dst     - points to buffer where string representation should be stored
   radix   - flag that shows whenever val should be taken as signed or not

   DESCRIPTION
   This is version of int2str() function which is optimized for normal case
   of radix 10/-10. It takes only sign of radix parameter into account and 
   not its absolute value.

   RETURN VALUE
   Pointer to ending NUL character.
   */

char *int10_to_str(long int val,char *dst,int radix)
{
    char buffer[65];
    register char *p;
    long int new_val;

    if (radix < 0)				/* -10 */
    {
        if (val < 0)
        {
            *dst++ = '-';
            val = -val;
        }
    }

    p = &buffer[sizeof(buffer)-1];
    *p = '\0';
    new_val= (long) ((unsigned long int) val / 10);
    *--p = '0'+ (char) ((unsigned long int) val - (unsigned long) new_val * 10);
    val = new_val;

    while (val != 0)
    {
        new_val=val/10;
        *--p = '0' + (char) (val-new_val*10);
        val= new_val;
    }
    while ((*dst++ = *p++) != 0) ;
    return dst-1;
}

#define char_val(X) (X >= '0' && X <= '9' ? X-'0' :\
    X >= 'A' && X <= 'Z' ? X-'A'+10 :\
    X >= 'a' && X <= 'z' ? X-'a'+10 :\
    '\177')

char *str2int(register const char *src, register int radix, long int lower,
    long int upper, long int *val)
{
    int sign;			/* is number negative (+1) or positive (-1) */
    int n;			/* number of digits yet to be converted */
    long limit;			/* "largest" possible valid input */
    long scale;			/* the amount to multiply next digit by */
    long sofar;			/* the running value */
    register int d;		/* (negative of) next digit */
    char *start;
    int digits[32];		/* Room for numbers */

    /*  Make sure *val is sensible in case of error  */

    *val = 0;

    /*  Check that the radix is in the range 2..36  */

#ifndef DBUG_OFF
    if (radix < 2 || radix > 36) {
        errno=EDOM;
        return NullS;
    }
#endif

    /*  The basic problem is: how do we handle the conversion of
        a number without resorting to machine-specific code to
        check for overflow?  Obviously, we have to ensure that
        no calculation can overflow.  We are guaranteed that the
        "lower" and "upper" arguments are valid machine integers.
        On sign-and-magnitude, twos-complement, and ones-complement
        machines all, if +|n| is representable, so is -|n|, but on
        twos complement machines the converse is not true.  So the
        "maximum" representable number has a negative representative.
        Limit is set to min(-|lower|,-|upper|); this is the "largest"
        number we are concerned with.	*/

    /*  Calculate Limit using Scale as a scratch variable  */

    if ((limit = lower) > 0) limit = -limit;
    if ((scale = upper) > 0) scale = -scale;
    if (scale < limit) limit = scale;

    /*  Skip leading spaces and check for a sign.
Note: because on a 2s complement machine MinLong is a valid
integer but |MinLong| is not, we have to keep the current
converted value (and the scale!) as *negative* numbers,
so the sign is the opposite of what you might expect.
*/
    while (isspace(*src)) src++;
    sign = -1;
    if (*src == '+') src++; else
        if (*src == '-') src++, sign = 1;

    /*  Skip leading zeros so that we never compute a power of radix
        in scale that we won't have a need for.  Otherwise sticking
        enough 0s in front of a number could cause the multiplication
        to overflow when it neededn't.
        */
    start=(char*) src;
    while (*src == '0') src++;

    /*  Move over the remaining digits.  We have to convert from left
        to left in order to avoid overflow.  Answer is after last digit.
        */

    for (n = 0; (digits[n]=char_val(*src)) < radix && n < 20; n++,src++) ;

    /*  Check that there is at least one digit  */

    if (start == src) {
        errno=EDOM;
        return NullS;
    }

    /*  The invariant we want to maintain is that src is just
        to the right of n digits, we've converted k digits to
        sofar, scale = -radix**k, and scale < sofar < 0.	Now
        if the final number is to be within the original
        Limit, we must have (to the left)*scale+sofar >= Limit,
        or (to the left)*scale >= Limit-sofar, i.e. the digits
        to the left of src must form an integer <= (Limit-sofar)/(scale).
        In particular, this is true of the next digit.  In our
        incremental calculation of Limit,

        IT IS VITAL that (-|N|)/(-|D|) = |N|/|D|
        */

    for (sofar = 0, scale = -1; --n >= 1;)
    {
        if ((long) -(d=digits[n]) < limit) {
            errno=ERANGE;
            return NullS;
        }
        limit = (limit+d)/radix, sofar += d*scale; scale *= radix;
    }
    if (n == 0)
    {
        if ((long) -(d=digits[n]) < limit)		/* get last digit */
        {
            errno=ERANGE;
            return NullS;
        }
        sofar+=d*scale;
    }

    /*  Now it might still happen that sofar = -32768 or its equivalent,
        so we can't just multiply by the sign and check that the result
        is in the range lower..upper.  All of this caution is a right
        pain in the neck.  If only there were a standard routine which
        says generate thus and such a signal on integer overflow...
        But not enough machines can do it *SIGH*.
        */
    if (sign < 0)
    {
        if (sofar < -LONG_MAX || (sofar= -sofar) > upper)
        {
            errno=ERANGE;
            return NullS;
        }
    }
    else if (sofar < lower)
    {
        errno=ERANGE;
        return NullS;
    }
    *val = sofar;
    errno=0;			/* indicate that all went well */
    return (char*) src;
}

/* Theese are so slow compared with ordinary, optimized atoi */

#ifdef WANT_OUR_ATOI

int atoi(const char *src)
{
    long val;
    str2int(src, 10, (long) INT_MIN, (long) INT_MAX, &val);
    return (int) val;
}


long atol(const char *src)
{
    long val;
    str2int(src, 10, LONG_MIN, LONG_MAX, &val);
    return val;
}

#endif /* WANT_OUR_ATOI */

void make_path(char *pathname) {
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    int ret = 0;
    ret = access(pathname, R_OK | W_OK);
    if (ret == -1) {
        ret = mkdir(pathname, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
        if (ret != 0) {
            mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "can not make path %s - %s:%s:%d", pathname , INFO_LOG_SUFFIX);
        }
    }
}

void pthread_sleep(int seconds) {

    struct timespec abstime;
    long int then;
    time_t tDate;

    pthread_mutex_t mutexTimer = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t condTimer = PTHREAD_COND_INITIALIZER;


    tDate = time(NULL);
    then = tDate + seconds;  
    abstime.tv_sec = then;
    abstime.tv_nsec = 0;
    pthread_mutex_lock(&mutexTimer);
    pthread_cond_timedwait(&condTimer, &mutexTimer, &abstime);
    pthread_mutex_unlock(&mutexTimer);
}

pid_t write_pid(const char *pidfile)
{
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    FILE *fp;
    pid_t t;
    /* Write the pid to a file */
    if ((fp = fopen(pidfile, "w")) != NULL) {
        fprintf(fp, "%d\n", t = getpid());
        fclose(fp);
    } else {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "Can not write pidfile %s: %s - %s:%s:%d",(char *)pidfile, strerror(errno), INFO_LOG_SUFFIX);
    }
    return t;
}   

pid_t read_pid(const char *pidfile)
{
    FILE *fp; 
    int pid = -1;

    if ((fp = fopen(pidfile, "r")) != NULL) {
        fscanf(fp, "%d\n", &pid); 
        fclose(fp);
    } else {
        //fatal("%s: %s\n", (char *)pidfile, strerror(errno));
    }
    return pid;
}


int def(char *src, size_t srclen, char *des, size_t *deslen, int level) {
    int ret = -1;
    if (level == -1) {
        level = Z_DEFAULT_COMPRESSION;
    }
    ret = compress2((Bytef *)des, (uLongf *)deslen, (Bytef *)src, (uLongf)srclen, level);
    return ret;
}

int inf(char *src, size_t srclen, char **des, size_t *deslen) {
    int ret = 0;
    if (*des != NULL) {
        return -1;
    }
    if (*deslen == 0) {
        *deslen = srclen * 2;
    }
    size_t realdeslen = *deslen;
    int num = *deslen;
    *des = (char *)malloc(realdeslen + 1);
    for(int i = 1; ret = uncompress((Bytef *)*des, (uLongf *)&realdeslen, (Bytef *)src, (uLongf)srclen); i++) {
        if (ret != Z_BUF_ERROR || i > 5) {
            break;
        } else {
            free(*des);
            int size = num * (int)pow(double(2), i);
            *des = (char *)malloc(size + 1);
            realdeslen = size;
        }
    }
    *deslen = realdeslen;
    return ret;

}


/*
 *  close_all_fds_except() - closes all of the file descriptors starting 
 *  with start, except the fds in e[] which is terminated by a seminal of -1.
 */
void close_all_fds_except(int start, int *e)
{
    int i, j, skip;

    //LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    //mylog_info(m_pLogGlobalCtrl->infolog, "Closing all file descs starting with %d,except - %s:%s:%d\n",start ,INFO_LOG_SUFFIX);

#if   defined(HAVE_GETDTABLESIZE)
    for (i = start; i < getdtablesize(); i++) {
#elif defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
        for (i = start; i < sysconf(_SC_OPEN_MAX); i++) {
#elif defined(OPEN_MAX)
            for (i = start; i < OPEN_MAX; i++) {
#else
                for (i = start; i < 64; i++) {
#endif
                    skip = 0;
                    for (j = 0; e[j] != -1; j++) {
                        if (i == e[j]) {
                            skip = 1;
                            break;
                        }
                    }
                    if (skip == 0) {
                        (void) close(i);
                    }
                }
            }

            void close_all_fds(int start)
            {
                int e[1];

                e[0] = -1;
                close_all_fds_except(start, e);
            }

            /* 
             *  setsocket_linger - sets the LINGER time on the given socket.
             */
            void setsocket_linger(int s, int t)
            {
                LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
                struct linger l;

                l.l_onoff = 1;
                l.l_linger = t;
                if (setsockopt(s, SOL_SOCKET, SO_LINGER, (char *) &l,
                        sizeof(l)) < 0)
                    mylog_fatal_errno(m_pLogGlobalCtrl->veryimportantlog, "setsockopt (SO_LINGER)");
            }

            int do_system(char *cmd)
            {
                LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
                mylog_info(m_pLogGlobalCtrl->infolog, "RUNNING as shell: %s - %s:%s:%d",cmd ,INFO_LOG_SUFFIX);
                return (system(cmd));
            }

            void sig_phandler()
            {
                LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
                struct sigaction action;

                sigemptyset(&action.sa_mask);
                action.sa_handler = sigpipe;
                action.sa_flags = SA_RESTART;
                if (sigaction(SIGPIPE, &action, NULL) != 0)
                    mylog_error(m_pLogGlobalCtrl->errorlog, "Cannot install SIGPIPE handler - %s:%s:%d:%d",ERROR_LOG_SUFFIX);
            }

            void sigpipe(int)
            { 

            } 



            void my_sleep(ulong m_seconds)
            {
#ifdef __NETWARE__
                delay(m_seconds/1000+1);
#elif defined(__WIN__)
                Sleep(m_seconds/1000+1);      /* Sleep() has millisecond arg */
#elif defined(OS2)
                DosSleep(m_seconds/1000+1);
#elif defined(HAVE_SELECT)
                struct timeval t;
                t.tv_sec=  m_seconds / 1000000L;
                t.tv_usec= m_seconds % 1000000L;
                select(0,0,0,0,&t); /* sleep */
#else
                uint sec=(uint)(m_seconds / 1000000L);
                ulong start= (ulong) time((time_t*) 0);
                while ((ulong) time((time_t*) 0) < start+sec);
#endif
            }
            //millisecond
            ulonglong getNow() {
                struct timeval tvtime;
                struct timezone tz;
                gettimeofday(&tvtime, &tz);
                return (ulonglong)tvtime.tv_sec * 1000 + (ulonglong)tvtime.tv_usec / 1000;
            }
            //microsecond
            void getNow1(int *second, int *micro) {
                struct timeval tvtime;
                struct timezone tz;
                gettimeofday(&tvtime, &tz);
                *second = tvtime.tv_sec; 
                *micro = tvtime.tv_usec;
            }

            int my_hash(char *key, int len)
            {
                unsigned long hval;
                const char	*ptr;
                char		c;
                int			i;
                hval = 0;
                for (ptr = key, i = 1; (c = *ptr++); i++)
                    hval += c * i;		// ascii char times its 1-based index 

                return(hval % len);
            }

            char *strdupn(const char *string, unsigned int n)
            {
                char *res = (char *)malloc((n + 1) * sizeof (char));

                if (res)
                {
                    memcpy(res, string, n);
                    *(res + n) = '\0';
                }

                return res;
            }


            char *strrtrim(char *str, const char *trim)
            {
                char    *end;

                if(!str)
                    return NULL;

                if(!trim)
                    trim = " \t\n\r";

                end = str + strlen(str);

                while(end-- > str)
                {
                    if(!strchr(trim, *end))
                        return str;
                    *end = 0;
                }
                return str;
            }

            char *strltrim(char *str, const char *trim)
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

            char *strtrim(char *str, const char *trim)
            {
                return strltrim(strrtrim(str, trim), trim);
            }


            char bin2hex (char ch)
            {               /* return ascii char for hex value of ightmost 4 bits of input */
                ch = ch & 0x0f;     /* mask off right nibble  - & bitwise AND*/
                ch += '0';                  /* make ascii '0' - '9'  */
                if (ch > '9')
                    ch += 7;                /* account for 7 chars between '9' and 'A' */
                return (ch);
            }

            int AlphaNumeric (char ch)
            {
                return ((ch >='a') && (ch <= 'z') ||
                    (ch >='A') && (ch <= 'Z') ||
                    (ch >='0') && (ch <= '9') );
            }

            int URLencode (const char * plain, char * encode, int maxlen)
            {
                char ch;            /* each char, use $t2 */
                char * limit;       /* point to last available location in encode */
                char * start;       /* save start of encode for length calculation */

                limit = encode + maxlen - 4;	/* need to store 3 chars and a zero */
                ch = *plain ++;			/* get first character */
                while (ch != 0)
                {					/* end of string, asciiz */
                    if (ch == ' ')
                        * encode ++ = '+';
                    else if (AlphaNumeric (ch))
                        * encode ++ = ch;
                    else {
                        * encode ++ = '%';
                        * encode ++ = bin2hex (ch >> 4);	/*shift right for left nibble*/
                        * encode ++ = bin2hex (ch);		/* right nibble */
                    }
                    ch = *plain ++;				/* ready for next character */
                    if (encode > limit)
                    {
                        *encode = 0;        /* still room to terminate string */
                        return (-1);        /* get out with error indication */
                    }
                }
                * encode = 0;               /* store zero byte to terminate string */
                return (encode - start);    /* done, return count of characters */
            }   /* URLencode*/

            //just encode chinese charset
            int MyURLencode (const char * plain, char * encode, int maxlen)
            {
                char ch;            /* each char, use $t2 */
                char * limit;       /* point to last available location in encode */
                char * start;       /* save start of encode for length calculation */

                limit = encode + maxlen - 4;	/* need to store 3 chars and a zero */
                ch = *plain ++;			/* get first character */
                while (ch != 0)
                {					/* end of string, asciiz */
                    //if (ch == ' ') {
                    //    * encode ++ = '+';
                    if (ch < 0) {
                        * encode ++ = '%';
                        * encode ++ = bin2hex (ch >> 4);	/*shift right for left nibble*/
                        * encode ++ = bin2hex (ch);		/* right nibble */
                    } else {
                        * encode ++ = ch;
                    }
                    ch = *plain ++;				/* ready for next character */
                    if (encode > limit)
                    {
                        *encode = 0;        /* still room to terminate string */
                        return (-1);        /* get out with error indication */
                    }
                }
                * encode = 0;               /* store zero byte to terminate string */
                return (encode - start);    /* done, return count of characters */
                }

                void SetCoreDump()
                {
                    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
                    int maxcore = 1; 
                    struct rlimit rlim;
                    if (maxcore != 0) { 
                        struct rlimit rlim_new;
                        if (getrlimit(RLIMIT_CORE, &rlim) == 0) {
                            rlim_new.rlim_cur = rlim_new.rlim_max = RLIM_INFINITY;
                            if (setrlimit(RLIMIT_CORE, &rlim_new)!= 0) { 
                                rlim_new.rlim_cur = rlim_new.rlim_max = rlim.rlim_max;
                                (void)setrlimit(RLIMIT_CORE, &rlim_new);
                            }
                        }
                        if ((getrlimit(RLIMIT_CORE, &rlim) != 0) || rlim.rlim_cur == 0) {
                            mylog_error(m_pLogGlobalCtrl->errorlog, "led to ensure corefile creation - %s:%s:%d", INFO_LOG_SUFFIX);
                            exit(0);
                        }   
                    }
                }

                void str_tolower(char *str, size_t len)
                { 
                    for(size_t i = 0; i < len; i++)
                    {
                        if(isupper(str[i])) {
                            tolower(str[i]);
                        }
                    }
                }
                void myrmdir1(const char * name)
                {
                    DIR *dir;
                    struct dirent *read_dir;
                    struct stat st;
                    char buf[DIR_BUF_LEN];

                    if(lstat(name, &st) < 0)
                    {
                        fprintf(stderr, "Lstat Error!\n");
                        exit(1);
                    }

                    if(S_ISDIR(st.st_mode))
                    {
                        if((dir = opendir(name)) == NULL)
                        {
                            fprintf(stderr, "remove [%s] faild\n", name);
                            exit(1);
                        }

                        while((read_dir = readdir(dir)) != NULL)
                        {
                            if(strcmp(read_dir->d_name, ".") == 0 ||
                                strcmp(read_dir->d_name, "..") == 0)
                                continue;
                            sprintf(buf, "%s/%s", name, read_dir->d_name);
                            myrmdir1(buf);
                        }
                    }
                    printf("rm :%s\n", name);
                    if(remove(name) < 0)
                    {
                        fprintf(stderr, "remove [%s] faild\n", name);
                    }
                }
                void print_result(char *path)
                {
                    printf("remove: %s\n",path);
                }

                void myrmdir(char *path)
                {  
                    char next_path[DIR_BUF_LEN];

                    int check = 0;
                    DIR *dir;
                    struct dirent *entry;
                    struct stat  buf;

                    if(lstat(path,&buf)  < 0)
                    { printf("stat  directory or file \"%s\": %s (ERROR %d)\n",     path, strerror(errno), errno);
                        exit(1);
                    }
                    if(!S_ISDIR(buf.st_mode))
                    { 
                        if(unlink(path) < 0)
                            printf("remove %s : %s ,%d\n",path,strerror(errno),errno); 
                        exit(0);
                    }

                    dir = opendir(path);
                    if (dir == NULL) 
                    {
                        printf("$$$$Open directory \"%s\": %s (ERROR %d)\n",
                            path, strerror(errno), errno);
                        exit(1);
                    } 

                    while ((entry = readdir(dir)) != NULL)
                    { 

                        if(strcmp(entry->d_name ,"." ) ==0 || strcmp(entry->d_name ,".." ) ==0 )
                            continue;

                        if (entry->d_type == DT_DIR)
                        { 
                            sprintf(next_path,"%s/%s",path,entry->d_name);
                            printf("*********%s\n",next_path);
                            myrmdir(next_path);
                        }
                        else
                        {
                            sprintf(next_path,"%s/%s",path,entry->d_name);
                            print_result(next_path);
                            if(unlink(next_path)<0)
                            { 
                                printf("remove --%s  error\n",entry->d_name);
                                exit(0);
                            }
                        }
                    }

                    closedir(dir);
                    print_result(path);
                    if(rmdir(path)<0)
                    {
                        printf("remove --%s  error\n",entry->d_name);
                        exit(0);
                    }

                }
//just encode chinese charset and blank
int MyURLBlankencode (const char * plain, char * encode, int maxlen)
{
    char ch;            /* each char, use $t2 */
    char * limit;       /* point to last available location in encode */
    char * start;       /* save start of encode for length calculation */

    limit = encode + maxlen - 4;	/* need to store 3 chars and a zero */
    ch = *plain ++;			/* get first character */
    while (ch != 0)
    {					/* end of string, asciiz */
        //if (ch == ' ') {
        //    * encode ++ = '+';
		if (ch < 0) {
            * encode ++ = '%';
            * encode ++ = bin2hex (ch >> 4);	/*shift right for left nibble*/
            * encode ++ = bin2hex (ch);		/* right nibble */
		} else {
            if (ch == ' ')
            {
                * encode ++ = '%';
                * encode ++ = '2';
                * encode ++ = '0';
            }else
            {
			    * encode ++ = ch;
            }
		}
        ch = *plain ++;				/* ready for next character */
        if (encode > limit)
        {
            *encode = 0;        /* still room to terminate string */
            return (-1);        /* get out with error indication */
        }
    }
    * encode = 0;               /* store zero byte to terminate string */
    return (encode - start);    /* done, return count of characters */
}
void chengtimeTodate(time_t timein, char *dateout)
{
    struct tm   *Nt;
    Nt = localtime(&timein);
    sprintf(dateout,"%d%d%d%d%d%d%d%d%d%d%d",Nt->tm_year+1900,
        (Nt->tm_mon+1)/10,(Nt->tm_mon+1)%10,
        Nt->tm_mday/10,Nt->tm_mday%10,
        Nt->tm_hour/10,Nt->tm_hour%10,
        Nt->tm_min/10,Nt->tm_min%10,
        Nt->tm_sec/10,Nt->tm_sec%10
        );
}
time_t changeTime(char *date2)
{

    struct tm segtime;
    char year[8] = {0},month[8] = {0},day[8] = {0},hour[8] = {0},min[8] = {0},sec[8] = {0};
    int yearnum,monthnum,daynum,hournum,minnum,secnum;

    time_t seconds;

    strncpy(year,date2,4);
    month[0] = date2[4];
    month[1] = date2[5];
    day[0] = date2[6];
    day[1] = date2[7];
    hour[0] = date2[8];
    hour[1] = date2[9];
    min[0] = date2[10];
    min[1] = date2[11];
    sec[0] = date2[12];
    sec[1] = date2[13];

    yearnum = atoi(year);
    monthnum = atoi(month);
    daynum = atoi(day);
    hournum = atoi(hour);
    minnum = atoi(min);
    secnum = atoi(sec);

    memset(&segtime,0,sizeof(segtime));
    segtime.tm_year = yearnum-1900;
    segtime.tm_mon = monthnum-1;
    segtime.tm_mday = daynum;
    segtime.tm_hour = hournum;
    segtime.tm_min = minnum;
    segtime.tm_sec = secnum;
    seconds=mktime(&segtime);

    return(seconds);
}
void urlInCode(const char *pD,char* out)
{
    out[0]=0;
    int i;
    char j,k;

    if(pD==NULL)
        return ;
    char *p=(char *)pD;
    i=0;
    while(*p)
    {/*
        if(*p > 127)
        {
        out[i++]=*p++;
        out[i++]=*p++;
        }
        else */if((*p <='9' && *p >='0')||(*p <='z' && *p >='a')||(*p <='Z' && *p >='A'))
        {
            out[i++]=*p++;
        }
        else
        {
            int pp = (int) *p;
            if (pp < 0)
                pp += 256;

            j= pp/16; k=pp%16;
            if(j > 9) j+='A'-10;
            else j+=48;
            if(k > 9) k+='A'-10;
            else k+=48;
            out[i++]='%';
            out[i++]=j;
            out[i++]=k;
            p++;
        }
    }
    out[i]=0;
}

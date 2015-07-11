#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "sysutil.h"
#include "util.h"
//#include "ic_types.h"
//#include "InfoCrawler.h"
static exitfunc_t s_exit_func;

void
die(const char* p_text)
{
#ifdef DIE_DEBUG
    //LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    //mylog_info(m_pLogGlobalCtrl->infolog, "ERROR: %s - %s:%s:%d\n", p_text,INFO_LOG_SUFFIX);
#endif
    sysutil_exit(1);
}

void*
sysutil_malloc(unsigned int size)
{
    //LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    void* p_ret;
    /* Paranoia - what if we got an integer overflow/underflow? */
    if (size == 0 || size > INT_MAX)
    {
       // mylog_info(m_pLogGlobalCtrl->infolog, "zero or big size in sysutil_malloc - %s:%s:%d\n",INFO_LOG_SUFFIX);
    }
    p_ret = malloc(size);
    if (p_ret == NULL)
    {
        die("malloc");
    }
    return p_ret;
}

void*
sysutil_calloc(size_t nmemb, unsigned int size)
{
    //LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    void* p_ret;
    /* Paranoia - what if we got an integer overflow/underflow? */
    size_t realsize = nmemb * size;
    if (realsize == 0 || realsize > INT_MAX)
    {
        //mylog_info(m_pLogGlobalCtrl->infolog, "zero or big size in sysutil_calloc  - %s:%s:%d\n",INFO_LOG_SUFFIX);
    }
    p_ret = calloc(nmemb, size);
    if (p_ret == NULL)
    {
        die("calloc");
    }
    return p_ret;
}

void
sysutil_exit(int exit_code)
{
    if (s_exit_func)
    {
        exitfunc_t curr_func = s_exit_func;
        /* Prevent recursion */
        s_exit_func = 0;
        (*curr_func)();
    }
    _exit(exit_code);
}

void
sysutil_set_exit_func(exitfunc_t exitfunc)
{
    s_exit_func = exitfunc;
}

int
__os_get_errno()
{
    /*
     * * This routine must be able to return the same value repeatedly.
     * *
     * * We've seen cases where system calls failed but errno was never set.
     * * This version of __os_get_errno() sets errno to EAGAIN if it's not
     * * already set, to work around that problem.  For obvious reasons, we
     * * can only call this function if we know an error has occurred, that
     * * is, we can't test errno for a non-zero value after this call.
     * */
    if (errno == 0)
        __os_set_errno(EAGAIN);

    return (errno);
}

/*
 * * __os_set_errno --
 * *  Set the value of errno.
 * *
 * * PUBLIC: void __os_set_errno __P((int));
 * */
void
__os_set_errno(evalue)
    int evalue;
{
    /*
     * * This routine is called by the compatibility interfaces (DB 1.85,
     * * dbm and hsearch).  Force values > 0, that is, not one of DB 2.X
     * * and later's public error returns.  If something bad has happened,
     * * default to EFAULT -- a nasty return.  Otherwise, default to EINVAL.
     * * As the compatibility APIs aren't included on Windows, the Windows
     * * version of this routine doesn't need this behavior.
     * */
    errno =
        evalue >= 0 ? evalue : 0;
}

/*
 * __os_physwrite --
 * Physical write to a file handle.
 */
int 
__os_physpwrite(int fd, void *addr, size_t len, off_t off)
{
    size_t offset;
    ssize_t nw;
    int ret;
    u_int8_t *taddr;

    ret = 0;

    for (taddr = addr, offset = 0;
        offset < len; taddr += nw, offset += (u_int32_t)nw, off += nw) {
        RETRY_CHK(((nw = pwrite(fd, taddr, len - offset, off)) < 0 ? 1 : 0), ret);
        if (ret != 0)
            break;
    }
    return offset;
}

/*
 * __os_read --
 * Read from a file handle.
 **/
int
__os_pread(int fd, void *addr, size_t len, off_t off) {
    size_t offset;
    ssize_t nr;
    int ret;
    u_int8_t *taddr;

    ret = 0;
    for (taddr = addr, offset = 0;
      offset < len; taddr += nr, offset += (u_int32_t)nr, off += nr) {
      RETRY_CHK(((nr = pread(
                      fd, taddr, len - offset, off)) < 0 ? 1 : 0), ret);
      if (nr == 0 || ret != 0)
          break;
    }
    return offset;
}

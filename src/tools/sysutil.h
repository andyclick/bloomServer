#ifndef KUROBOT_SYSUTIL_H
#define KUROBOT_SYSUTIL_H

#include <errno.h>

typedef void (*exitfunc_t)(void);

#ifdef  __cplusplus
extern "C" {
#endif

#define DB_RETRY    100

#define RETRY_CHK(op, ret) do {                     \
    int __retries = DB_RETRY;                   \
    do {                                \
        (ret) = (op);                       \
    } while ((ret) != 0 && (((ret) = __os_get_errno()) == EAGAIN || \
            (ret) == EBUSY || (ret) == EINTR) && --__retries > 0);  \
} while(0)

    void die(const char* p_text);
    void *sysutil_malloc(unsigned int size);
    void sysutil_exit(int exit_code);
    void sysutil_set_exit_func(exitfunc_t exitfunc);
    void *sysutil_calloc(size_t nmemb, unsigned int size);
    void __os_set_errno(int evalue);
    int __os_get_errno();
    int __os_physpwrite(int, void *, size_t, off_t);
    int __os_pread(int fd, void *addr, size_t len, off_t off); 

#ifdef  __cplusplus
}
#endif

#endif

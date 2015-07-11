#ifdef _DEBUG
#define DEBUG(format, args...) printf("[%s:%d %x] "format, __FILE__, __LINE__, pthread_self(),##args)
#else
#define DEBUG(args...)
#endif


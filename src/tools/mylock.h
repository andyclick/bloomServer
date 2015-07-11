#include <string>
#include <map>
#include "ic_types.h"
#include <pthread.h>

using namespace std;

#define MY_LOCK_HASH_SIZE 1024

class  mylock {

public:
    mylock();
    ~mylock();

    //must call this first
    static void init() {
        instance_ = new mylock;
    }

    static void uninit() {
        delete instance_;
    }

    static mylock * const get_instance() {
        return instance_;
    }

    void get(ulonglong id, int milisecond = 200);
    void put(ulonglong id);

private:
    static mylock *instance_;

    pthread_mutex_t *mutexs_;
    pthread_rwlock_t map_rwlock_;
    map<ulonglong, int> locks_;
};

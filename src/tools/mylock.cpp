#include "mylock.h"
#include "ic_types.h"

mylock *mylock::instance_ = NULL;

mylock::mylock() {
    mutexs_ = new pthread_mutex_t[MY_LOCK_HASH_SIZE];
    for(int i = 0; i < MY_LOCK_HASH_SIZE; i++) {
         pthread_mutex_init(&mutexs_[i], NULL);
    }
    pthread_rwlock_init(&map_rwlock_, NULL);
    //map_rwlock_ = PTHREAD_RWLOCK_INITIALIZER;
}

mylock::~mylock() {
    delete[] mutexs_;
}

void mylock::get(ulonglong id, int milisecond) {
    int hash = id % MY_LOCK_HASH_SIZE;

redo:
    bool locked = false;
    pthread_mutex_lock(&mutexs_[hash]); 

    map<ulonglong, int>::iterator iter;

    pthread_rwlock_rdlock(&map_rwlock_);
    if ((iter = locks_.find(id)) != locks_.end()) {
        if (iter->second > 0) {
            locked = true;
        }
    }
    pthread_rwlock_unlock(&map_rwlock_);

    if (!locked) {
        pthread_rwlock_wrlock(&map_rwlock_);
        locks_[id] = 1;
        pthread_rwlock_unlock(&map_rwlock_);
    }

    pthread_mutex_unlock(&mutexs_[hash]);

    if (locked) {
        my_sleep(milisecond * 1000);
        goto redo;
    }
}

void mylock::put(ulonglong id) {
    int hash = id % MY_LOCK_HASH_SIZE;
    pthread_mutex_lock(&mutexs_[hash]); 

    pthread_rwlock_wrlock(&map_rwlock_);
    locks_.erase(id);
    pthread_rwlock_unlock(&map_rwlock_);

    pthread_mutex_unlock(&mutexs_[hash]);
}

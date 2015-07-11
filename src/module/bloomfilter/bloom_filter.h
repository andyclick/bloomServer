#ifndef __BLOOM_FILTER_H__
#define __BLOOM_FILTER_H__

#include <stdio.h>
#include <pthread.h>
#include "list.h"
#include "debug.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/bitset.hpp>

//const size_t bloom_map_len  = 9062533;
const size_t bloom_map_len  = 36250002;
const size_t blooms_len  = 1048576;

#define ERROR_MALLOC_FAIL 1
#define OK 0

#ifndef MAX_KEY_LEN
#define MAX_KEY_LEN 32
#endif

#define rd_lock(x) pthread_rwlock_rdlock(x)
#define wr_lock(x) pthread_rwlock_wrlock(x)
#define rw_unlock(x) pthread_rwlock_unlock(x)


typedef unsigned long long UINT64_RC;

typedef struct _bloom{
    struct list_head list;
    struct _bloom *hash_next;
    size_t bytes;
    char key[MAX_KEY_LEN];
    UINT64_RC n;
    UINT64_RC m;
    UINT64_RC get_count;
    UINT64_RC set_count;
    UINT64_RC get_miss_count;
    double e;
    double e2;
    int k;
    //int map[0];
    //int map[9062533];
    std::vector<int>* v_hash;
//    int map[1];
    friend class boost::serialization::access;
    template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & list;
            ar & hash_next;
         
            ar & key;
            ar & n;
            ar & m;
            ar & get_count;
            ar & set_count;
            ar & get_miss_count;
            ar & e;
            ar & e2;
            ar & k;
            //for (size_t i = 0; i < ((bytes - 128) / 8); i++)
            //for (size_t i = 0; i < 9062533; i++)
            ar & v_hash;
     /*       for (size_t i = 0; i < bloom_map_len; i++)
            {
                ar & map[i];
            }
            */
 //           ar & map[0];
        }
} bloom_t;

typedef struct {
    struct list_head list;
    size_t bytes;
    size_t max_bytes;
    int count;
    int power;
    UINT64_RC get_count;
    UINT64_RC set_count;
    UINT64_RC get_miss_count;
    //bloom_t *hashtable[0];
    //bloom_t *hashtable[1048576];
    bloom_t *hashtable[blooms_len];
    friend class boost::serialization::access;
    template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & list;
            ar & bytes;
            ar & max_bytes;
            ar & count;
            ar & power;
            ar & get_count;
            ar & set_count;
            ar & get_miss_count;
            for (size_t i = 0; i < blooms_len; i++)
            {
                ar & hashtable[i];
            }
        }
} blooms_t;


int blooms_init (size_t max);
int blooms_add (const char *key, UINT64_RC n, double e);
int blooms_set (const char *key, const char *set_key);
int blooms_get (const char *key, const char *get_key);
void blooms_status(char *buf);
int bloom_status ();
bloom_t *blooms_search(char *key);
void blooms_delete_all(); 

int blooms_delete (const char *key);
void* blooms_dump(void * arg);
void blooms_dump1();
int blooms_load();
size_t calculate (UINT64_RC n, double e, UINT64_RC *m, int *k, double *tmp_e);

extern pthread_rwlock_t bloom_hash_lock;
extern pthread_rwlock_t bloom_dlist_lock;

#endif

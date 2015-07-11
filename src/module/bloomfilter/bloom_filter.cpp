#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>

#include "bloom_filter.h"
#include "murmur.h"

#define BITSPERWORD 32  
#define SHIFT 5  
#define MASK 0x1F  

#define HASH_POWER 20
#define SALT_CONSTANT 0x97c29b3a

#define hashsize(power) ((unsigned int)1<<(power))
#define hashmask(power) (hashsize(power)-1)



#define FORCE_INLINE inline static


blooms_t *blooms = NULL;

pthread_rwlock_t bloom_hash_lock;
pthread_rwlock_t bloom_dlist_lock;

pthread_rwlock_t dump_file_lock;

/* Generic hash function (a popular one from Bernstein).  89  * I tested a few and this was the best. */
unsigned int dictGenHashFunction(const unsigned char *buf, int len) {
    unsigned int hash = 5381;

    while (len--)
        hash = ((hash << 5) + hash) + (*buf++); /* hash * 33 + c */
    return hash;
}

FORCE_INLINE void bitmap_set(int *map, UINT64_RC i) 
{
    map[i>>SHIFT] |= (1<<(i & MASK)); 
}  

FORCE_INLINE void bitmap_clear(int *map, UINT64_RC i) 
{ 
    map[i>>SHIFT] &= ~(1<<(i & MASK)); 
}  

FORCE_INLINE int  bitmap_check(int *map, UINT64_RC i)
{ 
    return map[i>>SHIFT] & (1<<(i & MASK)); 
}  

size_t calculate (UINT64_RC n, double e, UINT64_RC* m, int* k, double* tmp_e)
{
    double te = 1;
    int kk = 0;
    int x;

    for(x = 2; te > e && x < 100; x++) {
        kk = (int)ceil(x * log(2));
        te =  pow((1 - exp((double)(-kk) / x)), kk);
    }

    /* overflow check. */
    if ((double)(x - 1) * n > (UINT64_RC)(-1)) return 0;

    UINT64_RC mm = (x - 1) * n;
    UINT64_RC bytes = (((mm + 0x1F) & (~0x1FULL)) >> 3) + sizeof(bloom_t);
    printf("totalbytes:%d \n", bytes);
    printf("sizeofbloom_t:%d \n", sizeof(bloom_t));
    printf("totalbytes-sizeofbloom_t:%d \n", bytes - sizeof(bloom_t));
    printf("int:%d \n", sizeof(int));

    /* overflow check. */
    if (bytes > (size_t)(-1)) return 0;

    *k = kk;
    *tmp_e = te;
    *m = mm;

    return (size_t)bytes;
}


bloom_t *bloom_init (const char * key, UINT64_RC n, double e)
{

    UINT64_RC m = 0;
    int k = 0;
    double tmp_e = 1;

    size_t bytes = calculate(n, e, &m, &k, &tmp_e);

    printf("bloom size:%d \n", bytes);

    if (!bytes) {
        DEBUG("Error, bad arg n:%llu e:%lf\r\n", n, e);
        return NULL;
    }
    if(bytes > blooms->max_bytes - blooms->bytes){
        DEBUG("Error, bloom over max size(bytes:%lu, x:%llu m:%llu, e:%e)\r\n", (unsigned long)bytes, m/n, m, e);
        return NULL;
    }

    //bloom_t *bloom = (bloom_t *)malloc(bytes); 
    bloom_t *bloom = (bloom_t *)malloc(sizeof(bloom_t)); 
    if(!bloom) {
        DEBUG("Error, bloom init failed (malloc)\r\n");
        return NULL;
    }

    //memset(bloom, 0, bytes);  
    memset(bloom, 0, sizeof(bloom_t));  
    bloom->m = m;
    bloom->e = e;
    bloom->e2 = tmp_e;
    bloom->k = k;
    bloom->n = n;
    bloom->bytes = bytes;
    bloom->v_hash = new std::vector<int>((bytes - sizeof(bloom_t)) / 4);
    // andy: 这个地方要写一个判断是否内存分配成功，new如果分配不成功好像会抛出异常，所以不能用上边的检查是否为空来判断，基本功能完成之后，下一个异常处理。
    strcpy(bloom->key, key);

    return bloom;
}

int bloom_set (bloom_t *bloom, const char *key) 
{
    UINT64_RC checksum[2];

    MurmurHash3_x64_128_for_bloomfilter(key, strlen(key), SALT_CONSTANT, checksum);
    UINT64_RC h1 = checksum[0];
    UINT64_RC h2 = checksum[1];

    int i;
    for (i = 0; i < bloom->k; i++) {
        UINT64_RC hashes = (h1 + i * h2) % bloom->m;
        //bitmap_set(bloom->map, hashes);
        std::vector<int>::iterator iter = (bloom->v_hash)->begin();
        int* addr = &(*iter);
        bitmap_set(addr, hashes);
    }
    bloom->set_count++;
    return 0;
}

        std::vector<int>::iterator bloom_get_iter;
        int* bloom_get_addr;
int bloom_get (bloom_t *bloom, const char *key) 
{
    UINT64_RC checksum[2];
    std::vector<int>::iterator iter = bloom->v_hash->begin();

    MurmurHash3_x64_128_for_bloomfilter(key, strlen(key), SALT_CONSTANT, checksum);
    UINT64_RC h1 = checksum[0];
    UINT64_RC h2 = checksum[1];

    int i;
    for (i = 0; i < bloom->k; i++) {
        UINT64_RC hashes = (h1 + i * h2) % bloom->m;
        //std::vector<int>::iterator iter = (bloom->v_hash)->begin();
        bloom_get_iter = (bloom->v_hash)->begin();
        //int* addr = &(*iter);
        bloom_get_addr = &(*bloom_get_iter);
        if(!bitmap_check(bloom_get_addr, hashes)){
            bloom->get_miss_count++;
            blooms->get_miss_count++;
            return 0;
        }
    }
    bloom->get_count++;
    blooms->get_count++;
    return 1;
}


static void bloom_delete (bloom_t *bloom)
{
    //list_del(&bloom->list);
    blooms->bytes -= bloom->bytes;
    //free(bloom);
    delete bloom->v_hash;
    blooms->count--;
}


bloom_t *blooms_search(char *key)
{
    unsigned int hv = dictGenHashFunction((unsigned char *)key, strlen(key)) & hashmask(blooms->power);

    bloom_t *bloom = blooms->hashtable[hv];

    while (bloom) {
        if (!strcmp(bloom->key, key)) {
            return bloom;
        }
        bloom = bloom->hash_next;
    }

    return NULL;
}

/**
 *1 key exist
 *2 bloom add fail
 *0 success
 *lock
 */

int blooms_add (const char *key, UINT64_RC n, double e)
{    

    unsigned int hv = dictGenHashFunction((unsigned char *)key, strlen(key)) & hashmask(blooms->power);    

    wr_lock(&bloom_hash_lock);
    bloom_t *bloom = blooms->hashtable[hv];

    while (bloom) {
        if (!strcmp(bloom->key, key)) {
            rw_unlock(&bloom_hash_lock);
            return 1;
        }
        bloom = bloom->hash_next;
    }

    bloom = bloom_init(key, n, e);
    if(!bloom) {
        rw_unlock(&bloom_hash_lock);
        DEBUG("Error, bloom init failed\r\n");
        return 2;
    }

    bloom->hash_next = blooms->hashtable[hv];
    blooms->hashtable[hv] = bloom;
    rw_unlock(&bloom_hash_lock);

    blooms->count++;
    blooms->bytes += bloom->bytes;


    wr_lock(&bloom_dlist_lock);
    //list_add_tail(&bloom->list, &blooms->list);
    printf("probe------------------- \n");
    rw_unlock(&bloom_dlist_lock);

    DEBUG("%s add\r\n", key);

    return 0;
}

/**
 *1 key not exist
 *0 success
 */

int blooms_set (const char *key, const char *subkey)
{
    unsigned int hv = dictGenHashFunction((unsigned char *)key, strlen(key)) & hashmask(blooms->power);

    wr_lock(&bloom_hash_lock);    
    bloom_t *bloom = blooms->hashtable[hv];

    while (bloom) {
        if (!strcmp(bloom->key, key)) {
            break;
        }
        bloom = bloom->hash_next;
    }

    if(!bloom) {
        DEBUG("Error, %s not exited\r\n", key);
        rw_unlock(&bloom_hash_lock);
        return 1;
    }

    bloom_set(bloom, subkey);
    blooms->set_count++;
    rw_unlock(&bloom_hash_lock);

    DEBUG("%s:\t%s\tOK\r\n", key, subkey);
    return 0;    
}

/**
 *-1 key not exist
 *1 success subkey exist
 *0 success subkey not exist
 */

int blooms_get (const char *key, const char *subkey)
{

    unsigned int hv = dictGenHashFunction((unsigned char *)key, strlen(key)) & hashmask(blooms->power);

    rd_lock(&bloom_hash_lock);
    bloom_t *bloom = blooms->hashtable[hv];

    while (bloom) {
        if (!strcmp(bloom->key, key)) {
            break;
        }
        bloom = bloom->hash_next;
    }

    if(!bloom) {
        rw_unlock(&bloom_hash_lock);
        DEBUG("Error, %s not exited\r\n", key);
        return -1;
    }

    int result = bloom_get(bloom, subkey);

    rw_unlock(&bloom_hash_lock);
    DEBUG("%s:\t%s\t%d\r\n", key, subkey, result);
    return result;    
}


/**
 *-1 key not exist
 */

int bloom_status (char * key)
{

    unsigned int hv = dictGenHashFunction((unsigned char *)key, strlen(key)) & hashmask(blooms->power);

    rd_lock(&bloom_hash_lock);
    bloom_t *bloom = blooms->hashtable[hv];

    while (bloom) {
        if (!strcmp(bloom->key, key)) {
            break;
        }
        bloom = bloom->hash_next;
    }
    if(!bloom) {
        rw_unlock(&bloom_hash_lock);
        DEBUG("Error, %s not exited\r\n", key);
        return -1;
    }

    printf("-----------------------------\r\n");
    printf("bloom filter name: %s\r\n", bloom->key);
    printf("expected max amount of elements: %llu\r\n", bloom->n);
    printf("expected false positive rate: %e\r\n", bloom->e);
    printf("theoretical false positive rate: %e\r\n", bloom->e2);
    printf("hash functions: %d\r\n", bloom->k);
    printf("table size (bits): %llu\r\n", bloom->m);
    printf("bloom size (bytes): %zu\r\n", bloom->bytes);
    printf("bloom set count: %llu\r\n", bloom->set_count);   
    printf("bloom get count: %llu\r\n", bloom->get_count);     
    printf("bloom get miss count: %llu\r\n", bloom->get_miss_count);    
    printf("-----------------------------\r\n\r\n");

    rw_unlock(&bloom_hash_lock);
    return 0;
}


void blooms_status(char *buf)
{

    rd_lock(&bloom_hash_lock);
    rd_lock(&bloom_dlist_lock);
    printf("-----------------------------\r\n");
    if(!list_empty(&blooms->list)){
        sprintf(buf, "blooms list:\r\n");
        bloom_t * i;
        list_for_each_entry(i, &blooms->list, list) {
            sprintf(buf, "\t\t%s\r\n", i->key);
        }
    }
    sprintf(buf, "blooms count: %d\r\n", blooms->count);    
    sprintf(buf, "blooms total bytes: %zu\r\n", blooms->bytes);
    sprintf(buf, "blooms set count: %llu\r\n", blooms->set_count);   
    sprintf(buf, "blooms get count: %llu\r\n", blooms->get_count);     
    sprintf(buf, "blooms get miss count: %llu\r\n", blooms->get_miss_count);     

    sprintf(buf, "-----------------------------\r\n\r\n");
    rw_unlock(&bloom_hash_lock);
    rw_unlock(&bloom_dlist_lock);
}


void blooms_status ()
{

    rd_lock(&bloom_hash_lock);
    rd_lock(&bloom_dlist_lock);
    printf("-----------------------------\r\n");
    if(!list_empty(&blooms->list)){
        printf("blooms list:\r\n");
        bloom_t * i;
        list_for_each_entry(i, &blooms->list, list){
            printf("\t\t%s\r\n", i->key);
        }
    }
    printf("blooms count: %d\r\n", blooms->count);    
    printf("blooms total bytes: %zu\r\n", blooms->bytes);
    printf("blooms set count: %llu\r\n", blooms->set_count);   
    printf("blooms get count: %llu\r\n", blooms->get_count);     
    printf("blooms get miss count: %llu\r\n", blooms->get_miss_count);     

    printf("-----------------------------\r\n\r\n");
    rw_unlock(&bloom_hash_lock);
    rw_unlock(&bloom_dlist_lock);
}

/**
 *1 key not exist
 *0 success
 *lock
 */

int blooms_delete (const char *key)
{
    wr_lock(&bloom_hash_lock);
    wr_lock(&bloom_dlist_lock);
    unsigned int hv = dictGenHashFunction((unsigned char *)key, strlen(key)) & hashmask(blooms->power);
    bloom_t *bloom = blooms->hashtable[hv];
    if(!bloom) {
        DEBUG("Error, %s not exist\r\n", key);
        rw_unlock(&bloom_hash_lock);
        rw_unlock(&bloom_dlist_lock);
        return 1;
    }


    if (!strcmp(bloom->key, key)) {
        blooms->hashtable[hv] = bloom->hash_next;
        bloom_delete(bloom);
        rw_unlock(&bloom_hash_lock);
        rw_unlock(&bloom_dlist_lock);
        DEBUG("%s deleted\r\n", key);
        return 0;
    } 

    bloom_t *next = bloom->hash_next;
    while (next) {

        if (!strcmp(next->key, key)) {
            bloom->hash_next = next->hash_next;
            bloom_delete(next);
            DEBUG("%s deleted\r\n", key);
            rw_unlock(&bloom_hash_lock);
            rw_unlock(&bloom_dlist_lock);
            return 0;
        }

        bloom = bloom->hash_next;
        next = bloom->hash_next;
    }

    rw_unlock(&bloom_hash_lock);
    rw_unlock(&bloom_dlist_lock);
    DEBUG("Error, %s not exist\r\n", key);
    return 1;
}

void blooms_delete_all() {
    wr_lock(&bloom_hash_lock);
    wr_lock(&bloom_dlist_lock);

    uint32_t num = hashsize(HASH_POWER);

    for(int hv = 0; hv < num; hv++) { 
        bloom_t *bloom = blooms->hashtable[hv];
        if(bloom) {
            bloom_t *next = bloom->hash_next;
            while (next) {
                bloom->hash_next = next->hash_next;
                bloom_delete(next);
                bloom = bloom->hash_next;
                next = bloom->hash_next;
            }
        }
    }

    rw_unlock(&bloom_hash_lock);
    rw_unlock(&bloom_dlist_lock);
}

void* blooms_dump(void* arg)
{
    //printf("ppppppppppppp%d \n", (size_t)(arg));
    while(1)
    {
        printf("sleep........\n");
        sleep((size_t(arg)));
        printf("sleep finished!\n");
        wr_lock(&dump_file_lock);
        std::ofstream fout("file.txt");
        boost::archive::text_oarchive oa(fout);
        rd_lock(&bloom_hash_lock);
        oa << blooms;
        rw_unlock(&bloom_hash_lock);
        fout.close();
        rw_unlock(&dump_file_lock);
    }
    return NULL;
}

void blooms_dump1()
{
        wr_lock(&dump_file_lock);
    std::ofstream fout("file.txt");
    boost::archive::text_oarchive oa(fout);
    rd_lock(&bloom_hash_lock);
    oa << blooms;
    rw_unlock(&bloom_hash_lock);
    fout.close();
        //rw_unlock(&dump_file_lock);
    return;
}

int blooms_load()
{
    /*std::ofstream fout("file.txt");
      boost::archive::text_oarchive oa(fout);
      oa << blooms;
      fout.close();
      blooms_t *blooms = NULL;*/
    pthread_rwlock_init(&dump_file_lock, NULL);
        wr_lock(&dump_file_lock);
    std::ifstream fin("file.txt");
    boost::archive::text_iarchive ia(fin);
    wr_lock(&bloom_hash_lock);
    ia >> blooms;
    rw_unlock(&bloom_hash_lock);
    fin.close();
        rw_unlock(&dump_file_lock);
    return 0;
}


/**
 *1 blooms malloc fail
 *0 success
 */

int blooms_init (size_t max)
{

    size_t blooms_size = /*hashsize(HASH_POWER) * sizeof(blooms_t *) + */sizeof(blooms_t);
    printf("blooms_size:%d\n", blooms_size);
    printf("sizeofblooms_t:%d\n", sizeof(blooms_t));
    printf("blooms_size-sizeofblooms_t:%d\n", blooms_size - sizeof(blooms_t));
    blooms = (blooms_t *)malloc(blooms_size);
    if(!blooms) {
        DEBUG("Error, blooms malloc failed\r\n");
        return 1;
    }
    memset(blooms, 0, blooms_size);  //fail?

    blooms->power = HASH_POWER;
    blooms->count = 0;
    blooms->bytes = 0;
    blooms->max_bytes = max;
    INIT_LIST_HEAD(&blooms->list);

    blooms->get_count = blooms->set_count = blooms->get_miss_count = 0;

    pthread_rwlock_init(&bloom_hash_lock, NULL);
    pthread_rwlock_init(&bloom_dlist_lock, NULL);
    pthread_rwlock_init(&dump_file_lock, NULL);

    return 0;

}

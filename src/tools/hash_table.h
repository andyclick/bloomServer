/* 
 * Copyright Richard Tobin 1995-9.
 */

#ifndef HASH_H
#define HASH_H
#ifdef  __cplusplus
extern "C" {
#endif
typedef struct hash_entry {
    void *key;
    int key_len;
    int value;
    struct hash_entry *next;
} HashEntryStruct;

typedef HashEntryStruct *HashEntry;
typedef struct hash_table *HashTable;

void Free(void *mem);
void CFree(void *mem);
void *Malloc(int bytes);
void *Realloc(void *mem, int bytes);

HashTable create_hash_table(int init_size);
void free_hash_table(HashTable table);
HashEntry hash_find(HashTable table, const void *key, int key_len);
HashEntry hash_find_or_add(HashTable table, const void *key, int key_len, int value, int *foundp);
void hash_remove(HashTable table, const void *key, int key_len);
void hash_map(HashTable table, 
	      void (*function)(const HashEntryStruct *, void *), void *arg);
int hash_count(HashTable table);
#ifdef  __cplusplus
}
#endif

#endif /* HASH_H */


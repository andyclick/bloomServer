#ifndef	__kdb_h
#define	__kdb_h

#include	<sys/types.h>
#include	<sys/stat.h>	/* open() & db_open() mode */
#include	<fcntl.h>		/* open() & db_open() flags */
#include	<stddef.h>		/* NULL */
#include	<pthread.h>		/* NULL */
#include	<errno.h>		/* NULL */
#include    <sys/time.h>
#include	"config.h"
#include	"hash_table.h"

/*#ifndef __USE_UNIX98
#include "pthread_rwlock.h"
#endif
*/
#undef USE_MY_LOCK 

#ifdef  __cplusplus
extern "C" {
#endif

#define FILE_MODE   (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
                        /* default file access permissions for new files */
#define DIR_MODE    (FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)
                        /* default permissions for new directories */

		/* flags for db_store() */
#define	DB_INSERT	1		/* insert new record only */
#define	DB_REPLACE	2		/* replace existing record */

		/* magic numbers */
#define	IDXLEN_SZ	   4	/* #ascii chars for length of index record */
#define	IDXLEN_MIN	   6	/* key, sep, start, sep, length, newline */
#define	IDXLEN_MAX	4096	/* arbitrary */
#define	SEP			 ':'	/* separator character in index record */
#define	DATLEN_MIN	   0	/* data byte, newline */
#define	DATLEN_MAX	1024 * 1024 * 2	/* 1M arbitrary */
//#define DATA_FILELEN_MAX  2000000000 /* max size of data file length 2 G*/
#define DATA_FILELEN_MAX  2000000000 /* max size of data file length 2 G*/
//#define DATA_FILELEN_MAX  200 /* max size of data file length 2 G*/

		/* following definitions are for hash chains and free list chain
		   in index file */
#define	PTR_SZ		   9	/* size of ptr field in hash chain */
#define	PTR_MAX   999999999	/* max offset (file size) = 10**PTR_SZ - 1 */
//#define	PTR_SZ		   2	/* size of ptr field in hash chain */
//#define	PTR_MAX        99   /* max offset (file size) = 10**PTR_SZ - 1 */
#define	PTR_THRESHOLD   PTR_MAX * 0.75 /* 4/3 of PTR_MAX */
#define	PTR_THRESHOLD_MAX   PTR_MAX * 0.9 /* 4/3 of PTR_MAX */
//#define	NHASH_DEF	 1000000	/* default hash table size */
#define	NHASH_DEF	 1572869ul	/* default hash table size */
//#define	NHASH_DEF	 10	/* default hash table size */
#define	FREE_OFF	   0	/* offset of ptr to free list in index file */
#define	HASH_OFF  PTR_SZ	/* offset of hash table in index file */
#define END_OFF       -1

#define IDX_LEN_CONF 10

#define IDX_RANGE 1000
#define DAT_RANGE 1024 * 3

#define IS_FREE_SPACE 1
#define IS_NOT_FREE_SPACE 0

#define DB_IDX_EXTENSION ".idx"
#define DB_DAT_EXTENSION ".dat"

#define DB_IDX_TMP_FILE_NAME "idx_template"

#define DB_LOCK_RDLOCK 1
#define DB_LOCK_WRLOCK 2
/*
 * free space list struct
 */
typedef struct _FSDBD {
   off_t offset; 
   size_t datlen;
   size_t idxlen_a;
   struct _FSDBD *next;
   struct _FSDBD *prev; 
} FSDBD;

typedef struct {	/* our internal structure */
  int	idxfd;	/* fd for index file */
  int	datfd;	/* fd for data file */
  int	oflag;	/* flags for open()/db_open(): O_xxx */
  char	*name;	/* name db was opened under */
  off_t	hashoff;/* offset in index file of hash table */
  int	nhash;	/* current hash table size */
  long	cnt_delok;	/* delete OK */
  long	cnt_delerr;	/* delete error */
  long	cnt_fetchok;/* fetch OK */
  long	cnt_fetcherr;/* fetch error */
  long	cnt_nextrec;/* nextrec */
  long	cnt_stor1;	/* store: DB_INSERT, no empty, appended */
  long	cnt_stor2;	/* store: DB_INSERT, found empty, reused */
  long	cnt_stor3;	/* store: DB_REPLACE, diff data len, appended */
  long	cnt_stor4;	/* store: DB_REPLACE, same data len, overwrote */
  long	cnt_storerr;/* store error */

  off_t idx_len; /*length of index file including free space and used space */
  off_t dat_len; /*length of data file including free space and used space */

  FSDBD *free_list;
  pthread_mutex_t *lock;
#ifndef USE_MY_LOCK 
  pthread_rwlock_t *freelock;
  pthread_rwlock_t *endlock;
  pthread_rwlock_t *commonlock;
#endif
  HashTable  locks;
} DB;
/*
 * for db data
 */
typedef struct {
    char	*idxbuf;/* malloc'ed buffer for index record */
    char	*datbuf;/* malloc'ed buffer for data record*/
    off_t	idxoff;	/* offset in index file of index record */
				/* actual key is at (idxoff + PTR_SZ + IDXLEN_SZ) */
    off_t	datoff;	/* offset in data file of data record */
    off_t	chainoff;/* offset of hash chain for this index record */
    off_t	ptroff;	/* offset of chain ptr that points to this index record */
    off_t	ptrval;	/* contents of chain ptr in index record */
    size_t  datlen;/* length of data record */
				/* includes newline at end */
    size_t  idxlen;/* length of index record */
                /* excludes IDXLEN_SZ bytes at front of index record */
                /* includes newline at end of index record */
    int     isfree;

    size_t datlen_u; /* used data */
    size_t idxlen_u; /* used  length of idx */

    size_t idxlen_a; /* all idx field length */
} DBD;


/*
 * for record position
 */
typedef struct {
    off_t offset; /* record data offset*/
    //off_t length; /* length of record */
} POS;

typedef	unsigned long	hash_t;	/* hash values */
			/* user-callable functions */
DB		*db_open(const char *, int, int, char *);
void	 db_close(DB *);
DBD	    *db_fetch(DB *, const char *, POS *pos);
DBD     *db_fetch_by_chainoff(DB *db, off_t chainoff); 
DBD     *db_fetch_directly(DB *db, const char *key, const POS *pos);


#ifdef __kdb_bin_
int     db_store(DB *db, const char *key, const void *data, size_t dat_len, int flag, POS *pos);
#else
int     db_store(DB *db, const char *key, const char *data, int flag, POS *pos);
#endif
int		 db_delete(DB *, const char *, POS *pos);
void	 db_rewind(DB *, DBD *);
DBD	*    db_nextrec(DB *, DBD *, char *, off_t *);
void	 db_stats(DB *);
int      dbd_free(DBD *dbd);
int      dbd_free_data(DBD *dbd);
DBD      *dbd_alloc(DB *db);

			/* internal functions */
DB		*_db_alloc(int);
int		 _db_checkfree(DB *);
int		 _db_dodelete(DB *, DBD *);
//int		 _db_emptykey(char *);
int		 _db_find(DB *, DBD *dbd, const char *, int, off_t);
int		 _db_findfree(DB *, DBD *, int, int);
int      _db_find_by_chainoff(DB *db, DBD *dbd, int writelock, off_t chainoff); 
int		 _db_free(DB *);
hash_t	 _db_hash(DB *, const char *);
//char	*_db_nextkey(DB *);
char	*_db_readdat(DB *, DBD *);
char    *_db_readdat_bin(DB *db, DBD *dbd);
off_t	 _db_readidx(DB *, DBD *, off_t, off_t *);
off_t	 _db_readptr(DB *, off_t);
void	 _db_writedat(DB *, DBD *, const char *, off_t, int);
void     _db_writedat_bin(DB *db, DBD *dbd, const void *data, size_t length, off_t offset, int whence);
int	     _db_writeidx(DB *, DBD *, const char *, off_t, int, off_t);
void	 _db_writeptr(DB *, off_t, off_t);
off_t    _get_file_size(int fd); 
void     _db_copy(DBD *dbd_src, DBD *dbd_des); 
void     _db_dump_dbd(DBD *dbd); 
int      _db_findfree_cache(DB *db, DBD *dbd, int keylen, int datlen);
void     _db_cache_free(FSDBD *item); 
void     _db_cache_delete(DB *db, FSDBD *item); 
FSDBD   *_db_cache_add(DB *db, DBD *dbd); 
void     _db_cache_dump(DB *db); 
void    _db_dump_db_size(DB *db); 

FSDBD   *_db_retrieve_free_space(DB *db, int *count_of_free_space);
void    *_db_retrieve_used_space(DB *db, size_t *idx_size_of_free_space, 
    int *count_of_free_space, size_t *dat_size_of_free_space);
int     _db_is_enough_file_space(DB *db, off_t idxlen, off_t datlen); 
int     _db_delete(DB *db, const char *key, POS *pos, int lock);
off_t   _db_chain_off(DB *db, const char *key); 
void    _db_create_db_template_files(char *path); 
void    _db_create_db_files(DB *db, char *dir); 
void    _db_rwlock_rdlock(DB *db, off_t chainoff); 
void    _db_rwlock_wrlock(DB *db, off_t chainoff); 
void    _db_rwlock_unlock(DB *db, off_t chainoff); 
int     _db_rwlock_checkwrlock(DB *db, off_t chainoff); 
int     _db_rwlock_checkrdlock(DB *db, off_t chainoff); 
void    _db_pthread_sleep(int useconds); 
int     ultoa(unsigned long num, char **str);


#ifdef  __cplusplus
}
#endif

#endif  /* __kdb_h */

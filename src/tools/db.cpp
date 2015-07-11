#include <sys/uio.h>		/* struct iovec */
#include <stdio.h>
#include "db.h"
#include "sysutil.h"
#include "util.h"
#include "InfoCrawler.h"

static pthread_mutex_t db_tmp_file_lock = PTHREAD_MUTEX_INITIALIZER;
static int tmp_db_fd  = -1;

/* Allocate & initialize a DB structure, and all the buffers it needs  */
DB *
_db_alloc(int namelen)
{
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    DB		*db;

    /* Use calloc, to init structure to zero */
    if ( (db = (DB *)calloc(1, sizeof(DB))) == NULL)
        mylog_error(m_pLogGlobalCtrl->errorlog, "calloc error for DB - %s:%s:%d", INFO_LOG_SUFFIX);

    db->idxfd = db->datfd = -1;				/* descriptors */

    /* Allocate room for the name.
       +5 for ".idx" or ".dat" plus null at end. */

    if ( (db->name = (char *)malloc(namelen + 5)) == NULL)
        mylog_error(m_pLogGlobalCtrl->errorlog, "malloc error for name for DB - %s:%s:%d", INFO_LOG_SUFFIX);

    /*
     *initialize lock
     */
    db->lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(db->lock, NULL);
#ifndef USE_MY_LOCK 
    db->freelock = (pthread_rwlock_t *)malloc(sizeof(pthread_rwlock_t));
    pthread_rwlock_init(db->freelock, NULL);
    db->commonlock = (pthread_rwlock_t *)malloc(sizeof(pthread_rwlock_t));
    pthread_rwlock_init(db->commonlock, NULL);
    db->endlock = (pthread_rwlock_t *)malloc(sizeof(pthread_rwlock_t));
    pthread_rwlock_init(db->endlock, NULL);
#endif
    /*
     *initialize locks table
     */
    db->locks = create_hash_table(1000);

    /* Allocate an index buffer and a data buffer.
       +2 for newline and null at end. */

    /*if ( (db->idxbuf = malloc(IDXLEN_MAX + 2)) == NULL)
      errorlog("malloc error for index buffer");
      if ( (db->datbuf = malloc(DATLEN_MAX + 2)) == NULL)
      errorlog("malloc error for data buffer");
      */

    return(db);
}
DBD *dbd_alloc(DB *db) {
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    DBD *dbd;
    if ( (dbd = (DBD *)calloc(1, sizeof(DBD))) == NULL)
        mylog_error(m_pLogGlobalCtrl->errorlog, "calloc error for DB - %s:%s:%d", INFO_LOG_SUFFIX);
    if ( (dbd->idxbuf = (char *)malloc(IDXLEN_MAX + 2)) == NULL)
        mylog_error(m_pLogGlobalCtrl->errorlog, "malloc error for index buffer for DB - %s:%s:%d", INFO_LOG_SUFFIX);

    return dbd;
}

void
db_close(DB *db)
{
    _db_dump_db_size(db);
    _db_free(db);	/* closes fds, free buffers & struct */
}

/* Delete the specified record */

int
_db_delete(DB *db, const char *key, POS *pos, int lock)
{
    //must set pos = NULL
    pos = NULL;

    int		rc;
    DBD *dbd = dbd_alloc(db);

    off_t offset = 0;
    if (pos != NULL) {
        offset = pos->offset;
    }
    /*if (lock)
      pthread_mutex_lock(db->lock);
      */

    if (_db_find(db, dbd, key, 1, offset) == 0) {

        rc = _db_dodelete(db, dbd);	/* record found */
        db->cnt_delok++;
    } else {
        rc = -1;				/* not found */
        db->cnt_delerr++;
    }
    _db_rwlock_unlock(db, dbd->chainoff);
    /*
       if (un_lock(db->idxfd, dbd->chainoff, SEEK_SET, 1) < 0)
       errorlog("un_lock error");
       */

    /*if (lock)
      pthread_mutex_unlock(db->lock);
      */
    dbd_free(dbd);

    return(rc);
}

int
db_delete(DB *db, const char *key, POS *pos)
{
    return _db_delete(db, key, pos, 1);
}

/* Delete the current record specified by the DB structure.
 * This function is called by db_delete() and db_store(),
 * after the record has been located by _db_find(). */

int
_db_dodelete(DB *db, DBD *dbd)
{
    char	*ptr;
    off_t	freeptr, saveptr;

    /* Set data buffer to all blanks */
    /*dbd->datbuf = (char *)calloc(dbd->datlen + 2, sizeof(char));
      for (ptr = dbd->datbuf, i = 0; i < dbd->datlen - 1; i++)
     *ptr++ = ' ';
     *ptr = 0;	*//* null terminate for _db_writedat() */

    /* Set key to blanks */

    ptr = dbd->idxbuf;
    while (*ptr)
        *ptr++ = ' ';

    /* We have to lock the free list */
    _db_rwlock_wrlock(db, FREE_OFF);
    /*
       if (writew_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0)
       errorlog("writew_lock error");
       */

    /* Write the data record with all blanks */
    /*
     * no need to write it with blanks
     */
    //_db_writedat(db, dbd, dbd->datbuf, dbd->datoff, SEEK_SET);

    /* Read the free list pointer.  Its value becomes the
       chain ptr field of the deleted index record.  This means
       the deleted record becomes the head of the free list. */
    freeptr = _db_readptr(db, FREE_OFF);

    /* Save the contents of index record chain ptr,
       before it's rewritten by _db_writeidx(). */
    saveptr = dbd->ptrval;

    /* Rewrite the index record.  This also rewrites the length
       of the index record, the data offset, and the data length,
       none of which has changed, but that's OK. */
    /*
     *set state 
     */
    //dbd->idxlen_u = 0; 
    //dbd->datlen_u = 0; 
    dbd->isfree = IS_FREE_SPACE;

    _db_writeidx(db, dbd, dbd->idxbuf, dbd->idxoff, SEEK_SET, freeptr);
    /* Write the new free list pointer */
    _db_writeptr(db, FREE_OFF, dbd->idxoff);

    /* Rewrite the chain ptr that pointed to this record
       being deleted.  Recall that _db_find() sets db->ptroff
       to point to this chain ptr.  We set this chain ptr
       to the contents of the deleted record's chain ptr,
       saveptr, which can be either zero or nonzero. */
    _db_writeptr(db, dbd->ptroff, saveptr);

    /*
     * Add a new free space to list
     */
    _db_cache_add(db, dbd);

    _db_rwlock_unlock(db, FREE_OFF);
    /*
       if (un_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0)
       errorlog("un_lock error");
       */
    return(0);
}

/* Fetch a specified record.
 * We return a pointer to the null-terminated data. */
DBD *
db_fetch_directly(DB *db, const char *key, const POS *pos)
{
    char	*ptr;
    DBD *dbd = dbd_alloc(db);
    int not_found = 0;

    dbd->datoff = pos->offset;
    dbd->chainoff = _db_chain_off(db, key);
    //dbd->datlen = pos->length;

    //pthread_mutex_lock(db->lock);
    _db_rwlock_rdlock(db, dbd->chainoff); 

    /*
       if (readw_lock(db->idxfd, dbd->chainoff, SEEK_SET, 1) < 0)
       errorlog("readw_lock error");
       */

#ifdef __kdb_bin_
    ptr = _db_readdat_bin(db, dbd);	/* return pointer to data */
#else
    ptr = _db_readdat(db, dbd);	/* return pointer to data */
#endif
    db->cnt_fetchok++;
    /* Unlock the hash chain that _db_find() locked */

    /*
       if (un_lock(db->idxfd, dbd->chainoff, SEEK_SET, 1) < 0)
       errorlog("un_lock error");
       */

    _db_rwlock_unlock(db, dbd->chainoff); 
    //pthread_mutex_unlock(db->lock);

    if (not_found) {
        dbd_free(dbd);
        dbd = NULL;
    }
    return(dbd);
}

DBD *
db_fetch_by_chainoff(DB *db, off_t chainoff) {
    char	*ptr;
    DBD *dbd = dbd_alloc(db);
    int not_found = 0;
    if (_db_find_by_chainoff(db, dbd, 0, chainoff) < 0) {
        not_found = 1;
    } else {
#ifdef __kdb_bin_
        ptr = _db_readdat_bin(db, dbd);	/* return pointer to data */
#else
        ptr = _db_readdat(db, dbd);	/* return pointer to data */
#endif
    }
    _db_rwlock_unlock(db, dbd->chainoff);

    if (not_found) {
        dbd_free(dbd);
        dbd = NULL;
    }
    return(dbd);
}

DBD *
db_fetch(DB *db, const char *key, POS *pos)
{
    char	*ptr;
    DBD *dbd = dbd_alloc(db);
    int not_found = 0;

    off_t offset = 0;
    if (pos != NULL) {
        offset = pos->offset;
    }

    //pthread_mutex_lock(db->lock);

    if (_db_find(db, dbd, key, 0, offset) < 0) {
        not_found = 1;
        db->cnt_fetcherr++;
    } else {
#ifdef __kdb_bin_
        ptr = _db_readdat_bin(db, dbd);	/* return pointer to data */
#else
        ptr = _db_readdat(db, dbd);	/* return pointer to data */
#endif
        if (pos != NULL) {
            pos->offset = dbd->idxoff;
        }
        db->cnt_fetchok++;
    }
    _db_rwlock_unlock(db, dbd->chainoff);
    /* Unlock the hash chain that _db_find() locked */
    /*if (un_lock(db->idxfd, dbd->chainoff, SEEK_SET, 1) < 0)
      errorlog("un_lock error");
      */

    //pthread_mutex_unlock(db->lock);

    if (not_found) {
        dbd_free(dbd);
        dbd = NULL;
    }
    return(dbd);
}

int
_db_find_by_chainoff(DB *db, DBD *dbd, int writelock, off_t chainoff) {

    off_t	offset;
    int ret = 0;

    dbd->chainoff = chainoff;
    dbd->ptroff = dbd->chainoff;

    if (writelock) {
        _db_rwlock_wrlock(db, dbd->chainoff);
    } else {
        _db_rwlock_rdlock(db, dbd->chainoff);
    }

    offset = _db_readptr(db, dbd->ptroff);
    DBD *dbd_new = dbd_alloc(db);
    if (offset > 0) {
        ret = _db_readidx(db, dbd_new, offset, NULL);
        if (ret >= 0) { /* found it*/
            _db_copy(dbd_new, dbd);
        }
    }

    dbd_free(dbd_new);

    if (offset <= 0 || ret < 0) {
        return(-1);		/* error, record not found */
    }

    return(0);
}

/* Find the specified record.
 * Called by db_delete(), db_fetch(), and db_store(). */

int
_db_find(DB *db, DBD *dbd, const char *key, int writelock, off_t idx_offset)
{
    off_t	offset, nextoffset;

    /* Calculate hash value for this key, then calculate byte
       offset of corresponding chain ptr in hash table.
       This is where our search starts. */

    /* calc offset in hash table for this key */
    dbd->chainoff = _db_chain_off(db, key);
    dbd->ptroff = dbd->chainoff;

    if (writelock) {
        _db_rwlock_wrlock(db, dbd->chainoff);
    } else {
        _db_rwlock_rdlock(db, dbd->chainoff);
    }
    /*
     *If already know the idx_offset
     */
    DBD *dbd_new = dbd_alloc(db);
    if (idx_offset != 0) {
        if (_db_readidx(db, dbd_new, idx_offset, NULL) >= 0) {
            if (strcmp(dbd_new->idxbuf, key) == 0) {
                _db_copy(dbd_new, dbd);
                dbd_free(dbd_new);
                return 0;
            }
        }
    }

    /* Here's where we lock this hash chain.  It's the
       caller's responsibility to unlock it when done.
       Note we lock and unlock only the first byte. */
    /*
       if (writelock) {
       if (writew_lock(db->idxfd, dbd->chainoff, SEEK_SET, 1) < 0)
       errorlog("writew_lock error");
       } else {
       if (readw_lock(db->idxfd, dbd->chainoff, SEEK_SET, 1) < 0)
       errorlog("readw_lock error");
       }
       */
    /* Get the offset in the index file of first record
       on the hash chain (can be 0) */
    offset = _db_readptr(db, dbd->ptroff);
    while (offset > 0) {
        nextoffset = _db_readidx(db, dbd_new, offset, NULL);
        if (strcmp(dbd_new->idxbuf, key) == 0) {
            _db_copy(dbd_new, dbd);
            break;		/* found a match */
        }

        dbd->ptroff = offset;	/* offset of this (unequal) record */
        offset = nextoffset;	/* next one to compare */
    }

    dbd_free(dbd_new);

    if (offset <= 0) {
        return(-1);		/* error, record not found */
    }

    /* We have a match.  We're guaranteed that db->ptroff contains
       the offset of the chain ptr that points to this matching
       index record.  _db_dodelete() uses this fact.  (The chain
       ptr that points to this matching record could be in an
       index record or in the hash table.) */
    return(0);
}

/* Try to find a free index record and accompanying data record
 * of the correct sizes.  We're only called by db_store(). */

int
_db_findfree(DB *db, DBD *dbd, int keylen, int datlen)
{
    int		rc;
    off_t	offset, nextoffset, saveoffset;
    /* Lock the free list */
    //pthread_rwlock_wrlock(&rwlock_db);
    /*
       if (writew_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0)
       errorlog("writew_lock error");
       */

    /* Read the free list pointer */
    _db_rwlock_wrlock(db, FREE_OFF);

    saveoffset = FREE_OFF;
    offset = _db_readptr(db, saveoffset);
    DBD *dbd_new = dbd_alloc(db);
    while (offset > 0) {
        nextoffset = _db_readidx(db, dbd_new, offset, NULL);
        if (nextoffset >= 0) {
            if (dbd_new->idxlen_a >= keylen && dbd_new->idxlen_a < (keylen + IDX_RANGE) && dbd_new->datlen >= datlen && dbd_new->datlen < (datlen + DAT_RANGE))  {
                //if (strlen(dbd->idxbuf) == keylen && dbd->datlen == datlen)
                _db_copy(dbd_new, dbd);
                break;		/* found a match */
            }
        }
        saveoffset = offset;
        offset = nextoffset;
    }

    dbd_free(dbd_new);

    if (offset <= 0)
        rc = -1;	/* no match found */
    else {
        /* Found a free record with matching sizes.
           The index record was read in by _db_readidx() above,
           which sets db->ptrval.  Also, saveoffset points to
           the chain ptr that pointed to this empty record on
           the free list.  We set this chain ptr to db->ptrval,
           which removes the empty record from the free list. */

        _db_writeptr(db, saveoffset, dbd->ptrval);
        rc = 0;

        /* Notice also that _db_readidx() set both db->idxoff
           and db->datoff.  This is used by the caller, db_store(),
           to write the new index record and data record. */
    }
    /* Unlock the free list */
    /*
       if (un_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0)
       errorlog("un_lock error");
       */
    //
    _db_rwlock_unlock(db, FREE_OFF);
    //pthread_rwlock_unlock(&rwlock_db);
    return(rc);
}


int
_db_findfree_cache(DB *db, DBD *dbd, int keylen, int datlen)
{
    int		rc;
    off_t	offset = 0, saveoffset = FREE_OFF;
    FSDBD *item = NULL;
    _db_rwlock_wrlock(db, FREE_OFF);
    item = db->free_list;
    DBD *dbd_new = dbd_alloc(db);
    while (item != NULL) {
        if (item->idxlen_a >= keylen && item->idxlen_a < (keylen + IDX_RANGE) && item->datlen >= datlen && item->datlen < (datlen + DAT_RANGE))  {
            offset = item->offset;
            if (_db_readidx(db, dbd_new, item->offset, NULL) >= 0) {
                _db_copy(dbd_new, dbd);
                break;		/* found a match */
            } else { //record error
                _db_cache_delete(db, item);
                _db_cache_free(item);
            }
        }
        saveoffset = item->offset;
        item = item->next;
    }

    dbd_free(dbd_new);

    if (offset == 0)
        rc = -1;	/* no match found */
    else {
        /*
         * Remove free space from list and memory
         */
        _db_writeptr(db, saveoffset, dbd->ptrval);
        _db_cache_delete(db, item);
        _db_cache_free(item);
        rc = 0;
    }
    _db_rwlock_unlock(db, FREE_OFF);
    return(rc);
}
/* Free up a DB structure, and all the malloc'ed buffers it
 * may point to.  Also close the file descriptors if still open. */

int
_db_free(DB *db)
{
    /*
     * free free space list
     */
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    FSDBD *item = db->free_list;
    while(item != NULL) {
        FSDBD *tmp = item;
        item = item->next;
        free(tmp);
    }
    //

    if (db->idxfd >= 0 && close(db->idxfd) < 0)
        mylog_error(m_pLogGlobalCtrl->errorlog, "index close error for DB - %s:%s:%d", INFO_LOG_SUFFIX);
    if (db->datfd >= 0 && close(db->datfd) < 0)
        mylog_error(m_pLogGlobalCtrl->errorlog, "data close error for DB - %s:%s:%d", INFO_LOG_SUFFIX);
    db->idxfd = db->datfd = -1;

    /*if (db->idxbuf != NULL)
      free(db->idxbuf);
      if (db->datbuf != NULL)
      free(db->datbuf);
      */
    if (db->name != NULL) {
        free(db->name);
    }
    free(db->lock);
    free(db->freelock);
    free(db->endlock);
    free(db->commonlock);
    free_hash_table(db->locks);
    free(db);
    return(0);
}

int
dbd_free(DBD *dbd) {
    if (dbd != NULL) {
        if (dbd->idxbuf != NULL)
            free(dbd->idxbuf);
        dbd->idxbuf = NULL;
        if (dbd->datbuf != NULL)
            free(dbd->datbuf);
        dbd->datbuf = NULL;
        free(dbd);
    }
    return 0;
}
int dbd_free_data(DBD *dbd) {
    if (dbd != NULL) {
        /*if (dbd->idxbuf != NULL)
          free(dbd->idxbuf);
          dbd->idxbuf = NULL;
          */
        if (dbd->datbuf != NULL)
            free(dbd->datbuf);
        dbd->datbuf = NULL;
    }
    return 0;
}

/* Calculate the hash value for a key. */

hash_t
_db_hash(DB *db, const char *key)
{
    /*    unsigned int n=0; 
          char* b=(char*)&n; 
          size_t i = 0;
          for (i=0; i<strlen(key); i++) {
          b[i%4]^=key[i]; 
          }
          return n%db->nhash; 
          */
    hash_t		hval;
    const char	*ptr;
    char		c;
    int			i;
    hval = 0;
    for (ptr = key, i = 1; (c = *ptr++); i++)
        hval += c * i;		// ascii char times its 1-based index 

    return(hval % db->nhash);

}


/* Return the next sequential record.
 * We just step our way through the index file, ignoring deleted
 * records.  db_rewind() must be called before this is function
 * is called the first time.
 */
/*
 * Should free DBD
 */
DBD *
db_nextrec(DB *db, DBD *dbd_last, char *key, off_t *last_offset)
{
    char *ptr;
    DBD *dbd = NULL;
    if (dbd_last != NULL) {
        dbd_free_data(dbd_last);
        dbd = dbd_last;
    } else {
        dbd = dbd_alloc(db);
        db_rewind(db, dbd);
        *last_offset = dbd->idxoff;
    }

    //pthread_mutex_lock(db->lock);
    /* We read lock the free list so that we don't read
       a record in the middle of its being deleted. */
    _db_rwlock_rdlock(db, FREE_OFF);
    /*
       if (readw_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0)
       errorlog("readw_lock error");
       */

    do {
        /* read next sequential index record */
        off_t offset = *last_offset;
        if (_db_readidx(db, dbd, offset, last_offset) < 0) {
            dbd_free(dbd);
            dbd = NULL;		/* end of index file, EOF */
            goto doreturn;
        }
        /* check if key is all blank (empty record) */
        /*ptr = dbd->idxbuf;
          while ( (c = *ptr++) != 0  &&  c == ' ')
          ;	*//* skip until null byte or nonblank */
        //} while (c == 0);	/* loop until a nonblank key is found */
} while (dbd->isfree);	/* loop until a nonblank key is found */

if (key != NULL)
    strcpy(key, dbd->idxbuf);	/* return key */
#ifdef __kdb_bin_
    ptr = _db_readdat_bin(db, dbd);	/* return pointer to data */
#else
    ptr = _db_readdat(db, dbd);	/* return pointer to data */
#endif

    db->cnt_nextrec++;
doreturn:
/*
   if (un_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0)
   errorlog("un_lock error");
   */

_db_rwlock_unlock(db, FREE_OFF);
//pthread_mutex_unlock(db->lock);

return(dbd);
}

/* Open or create a database.  Same arguments as open(). */

DB *
db_open(const char *pathname, int oflag, int mode, char *dir)
{
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    DB			*db;
    int len;
    //int			i, len;
    /*	char		asciiptr[PTR_SZ + 1],
        hash[(NHASH_DEF + 1) * PTR_SZ + 2];
        */
    /* +2 for newline and null */
    struct stat	statbuff;

    /* Allocate a DB structure, and the buffers it needs */
    len = strlen(pathname);
    if ( (db = _db_alloc(len)) == NULL)
        mylog_error(m_pLogGlobalCtrl->errorlog, "_db_alloc error for DB - %s:%s:%d", INFO_LOG_SUFFIX);

    db->oflag = oflag;		/* save a copy of the open flags */

    /* Open index file */
    strcpy(db->name, pathname);
    strcat(db->name, ".idx");
    int ret = access(db->name, R_OK | W_OK);
    if (ret == 0) {
        if ( (db->idxfd = open(db->name, O_RDWR, mode)) < 0) {
            mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, " can not open db %s %d for DB - %s:%s:%d:%d",db->name , ERROR_LOG_SUFFIX);
            _db_free(db);
            return(NULL);
        }

        strcpy(db->name + len, ".dat");
        if ( (db->datfd = open(db->name, O_RDWR, mode)) < 0) {
            mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, " can not open db %s %d for DB - %s:%s:%d:%d",db->name , ERROR_LOG_SUFFIX);
            _db_free(db);
            return(NULL);
        }
    } else {

        if ( (db->idxfd = open(db->name, oflag, mode)) < 0) {
            mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, " can not open db %s %d for DB - %s:%s:%d:%d",db->name , ERROR_LOG_SUFFIX);
            _db_free(db);
            return(NULL);
        }
        /* Open data file */
        strcpy(db->name + len, ".dat");
        if ( (db->datfd = open(db->name, oflag, mode)) < 0) {
            mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, " can not open db %s %d for DB - %s:%s:%d:%d",db->name , ERROR_LOG_SUFFIX);
            _db_free(db);
            return(NULL);
        }

        /* If the database was created, we have to initialize it */
        if ((oflag & (O_CREAT | O_TRUNC)) == (O_CREAT | O_TRUNC)) {

            /* Write lock the entire file so that we can stat
               the file, check its size, and initialize it,
               as an atomic operation. */
            /*
               if (writew_lock(db->idxfd, 0, SEEK_SET, 0) < 0)
               errorlog("writew_lock error");
               */

            if (fstat(db->idxfd, &statbuff) < 0)
                mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "fstat error for DB - %s:%s:%d",db->name , INFO_LOG_SUFFIX);
            if (statbuff.st_size == 0) {
                /* We have to build a list of (NHASH_DEF + 1) chain
                   ptrs with a value of 0.  The +1 is for the free
                   list pointer that precedes the hash table. */
                /*sprintf(asciiptr, "%*d", PTR_SZ, 0);
                  hash[0] = 0;
                  for (i = 0; i < (NHASH_DEF + 1); i++)
                  strcat(hash, asciiptr);
                  strcat(hash, "\n");

                  i = strlen(hash);
                  if (write(db->idxfd, hash, i) != i)
                  errorlog("write error initializing index file");
                  */
                _db_create_db_files(db, dir);
            }
            /*
               if (un_lock(db->idxfd, 0, SEEK_SET, 0) < 0)
               errorlog("un_lock error");
               */
        }
    }
    db->nhash   = NHASH_DEF;/* hash table size */
    db->hashoff = HASH_OFF;	/* offset in index file of hash table */
    /* free list ptr always at FREE_OFF */
    //db_rewind(db);
    /*
     * retrieve free space 
     */
    int count_of_free_space = 0;
    FSDBD *list_head = _db_retrieve_free_space(db, &count_of_free_space);
    db->dat_len = _get_file_size(db->datfd);
    db->idx_len = _get_file_size(db->idxfd);

    _db_dump_db_size(db);

    if (list_head != NULL && list_head->next != NULL) {
        db->free_list = list_head->next;
        db->free_list->prev = NULL;
    }
    if (list_head) free(list_head);

    return(db);
}

/* Read the current data record into the data buffer.
 * Return a pointer to the null-terminated data buffer. */

char *
_db_readdat(DB *db, DBD *dbd)
{
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    /*if (lseek(db->datfd, db->datoff, SEEK_SET) == -1)
      errorlog("lseek error");
      */
    dbd->datbuf = (char *)calloc(dbd->datlen_u + 2, sizeof(char));
    if (__os_pread(db->datfd, dbd->datbuf, dbd->datlen_u, dbd->datoff) != dbd->datlen_u) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "pread error for DB - %s:%s:%d", INFO_LOG_SUFFIX);
        return NULL;
    }
    if (dbd->datbuf[dbd->datlen_u - 1] != '\n') {/* sanity check */
        mylog_error(m_pLogGlobalCtrl->errorlog, "missing newline for DB - %s:%s:%d", INFO_LOG_SUFFIX);
        return NULL;
    }
    dbd->datbuf[dbd->datlen_u - 1] = 0;		/* replace newline with null */

    return(dbd->datbuf);		/* return pointer to data record */
}

/* Read the current data record into the data buffer.
 * Return a pointer to the null-terminated data buffer. */

char *
_db_readdat_bin(DB *db, DBD *dbd)
{
    LogGlobalCtrl * m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    dbd->datbuf = (char *)calloc(dbd->datlen_u + 2, sizeof(char));
    if (__os_pread(db->datfd, dbd->datbuf, dbd->datlen_u, dbd->datoff) != dbd->datlen_u) {
        mylog_info(m_pLogGlobalCtrl->infolog, "pread error for DB - %s:%s:%d", INFO_LOG_SUFFIX);        
        return NULL;
    }

    dbd->datbuf[dbd->datlen_u] = 0;		/* to be compatible with ascii data */
    return(dbd->datbuf);		/* return pointer to data record */
}

/* Read the next index record.  We start at the specified offset in
 * the index file.  We read the index record into db->idxbuf and
 * replace the separators with null bytes.  If all is OK we set
 * dbd->datoff and dbd->datlen to the offset and length of the
 * corresponding data record in the data file.  */

off_t
_db_readidx(DB *db, DBD *dbd, off_t offset, off_t *next_key_offset)
{

    int				i;
    char			*ptr1, *ptr2, *ptr3, *ptr4, *ptr5;
    char			asciiptr[PTR_SZ + 1], asciilen[IDXLEN_SZ + 1];
    struct iovec	iov[2];
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    /* Position index file and record the offset.  db_nextrec()
       calls us with offset==0, meaning read from current offset.
       We still need to call lseek() to record the current offset. */
    /*
       if ( (db->idxoff = lseek(db->idxfd, offset,
       offset == 0 ? SEEK_CUR : SEEK_SET)) == -1)
       errorlog("lseek error");
       */

    /* Read the ascii chain ptr and the ascii length at
       the front of the index record.  This tells us the
       remaining size of the index record. */
    dbd->idxoff = offset;
    iov[0].iov_base = asciiptr;
    iov[0].iov_len  = PTR_SZ;
    iov[1].iov_base = asciilen;
    iov[1].iov_len  = IDXLEN_SZ;
    if ( (i = __os_pread(db->idxfd, iov[0].iov_base,iov[0].iov_len,  offset)) != PTR_SZ) {
        if (i == 0 && offset >= _get_file_size(db->idxfd))
            return(-1);		/* EOF for db_nextrec() */
        mylog_error(m_pLogGlobalCtrl->errorlog, "pread error of index record for DB - %s:%s:%d", INFO_LOG_SUFFIX);
        return(-1);
    }
    if ( (i = __os_pread(db->idxfd, iov[1].iov_base,iov[1].iov_len,  offset + PTR_SZ)) != IDXLEN_SZ) {
        if (i == 0 && offset >= _get_file_size(db->idxfd))
            return(-1);		/* EOF for db_nextrec() */
        mylog_error(m_pLogGlobalCtrl->errorlog, "pread error of index record for DB - %s:%s:%d", INFO_LOG_SUFFIX);
        return -1;
    }

    asciiptr[PTR_SZ] = 0;			/* null terminate */
    dbd->ptrval = atol(asciiptr);	/* offset of next key in chain */
    /* this is our return value; always >= 0 */
    asciilen[IDXLEN_SZ] = 0;		/* null terminate */
    if ( (dbd->idxlen = atoi(asciilen)) < IDXLEN_MIN ||
        dbd->idxlen > IDXLEN_MAX) {
        mylog_info(m_pLogGlobalCtrl->infolog, "invalid length length = %d for DB - %s:%s:%d",dbd->idxlen ,INFO_LOG_SUFFIX);
        return -1;
    }

    /* Now read the actual index record.  We read it into the key
       buffer that we malloced when we opened the database. */
    if ( (i = __os_pread(db->idxfd, dbd->idxbuf, dbd->idxlen, offset + PTR_SZ + IDXLEN_SZ)) != dbd->idxlen) {
        mylog_info(m_pLogGlobalCtrl->infolog, "read error of indexc record for DB - %s:%s:%d",INFO_LOG_SUFFIX);
        return -1;
    }
    if (next_key_offset != NULL) {
        *next_key_offset = offset + PTR_SZ + IDXLEN_SZ + i;
    }
    if (dbd->idxbuf[dbd->idxlen-1] != '\n') {
        mylog_info(m_pLogGlobalCtrl->infolog, "missing newline for DB - %s:%s:%d",INFO_LOG_SUFFIX);
        return -1;
    }
    dbd->idxbuf[dbd->idxlen-1] = 0;		/* replace newline with null */
    /* Find the separators in the index record */
    if ( (ptr5 = strrchr(dbd->idxbuf, SEP)) == NULL) {
        mylog_info(m_pLogGlobalCtrl->infolog, "missing second separator for DB - %s:%s:%d",INFO_LOG_SUFFIX);
        return -1;
    }
    *ptr5++ = 0;				/* replace SEP with null */
    if ( (ptr4 = strrchr(dbd->idxbuf, SEP)) == NULL) {
        mylog_info(m_pLogGlobalCtrl->infolog, "missing second separator for DB - %s:%s:%d",INFO_LOG_SUFFIX);
        return -1;
    }
    *ptr4++ = 0;				/* replace SEP with null */
    if ( (ptr3 = strrchr(dbd->idxbuf, SEP)) == NULL) {
        mylog_info(m_pLogGlobalCtrl->infolog, "missing second separator for DB - %s:%s:%d",INFO_LOG_SUFFIX);
        return -1;
    }
    *ptr3++ = 0;				/* replace SEP with null */
    if ( (ptr1 = strrchr(dbd->idxbuf, SEP)) == NULL) {
        mylog_info(m_pLogGlobalCtrl->infolog, "missing first separator for DB - %s:%s:%d",INFO_LOG_SUFFIX);
        return -1;
    }
    *ptr1++ = 0;				/* replace SEP with null */
    if ( (ptr2 = strrchr(dbd->idxbuf, SEP)) == NULL) {
        mylog_info(m_pLogGlobalCtrl->infolog, "missing second separator for DB - %s:%s:%d",INFO_LOG_SUFFIX);
        return -1;
    }
    *ptr2++ = 0;				/* replace SEP with null */

    /*if (strchr(ptr2, SEP) != NULL)
      errorlog("too many separators");
      */

    /* Get the starting offset and length of the data record */
    if ( (dbd->datoff = atol(ptr2)) < 0) {
        mylog_info(m_pLogGlobalCtrl->infolog, "starting offset < 0 for DB - %s:%s:%d",INFO_LOG_SUFFIX);
        return -1;
    }
    if ( (dbd->datlen = atol(ptr1)) < DATLEN_MIN || dbd->datlen > DATLEN_MAX) {
        mylog_info(m_pLogGlobalCtrl->infolog, "invalid length for DB - %s:%s:%d",INFO_LOG_SUFFIX);
        return -1;
    }
    if ( (dbd->idxlen_u = atol(ptr3)) < 0) {
        mylog_info(m_pLogGlobalCtrl->infolog, "starting offset < 0 for DB - %s:%s:%d",INFO_LOG_SUFFIX);
        return -1;
    }
    if ( (dbd->datlen_u = atol(ptr4)) < 0) {
        mylog_info(m_pLogGlobalCtrl->infolog, "starting offset < 0 for DB - %s:%s:%d",INFO_LOG_SUFFIX);
        return -1;
    }
    if ( (dbd->isfree = atol(ptr5)) < 0) {
        mylog_info(m_pLogGlobalCtrl->infolog, "starting offset < 0 for DB - %s:%s:%d",INFO_LOG_SUFFIX);
        return -1;
    }
    /*
     * get real idx length
     */
    dbd->idxlen_a = strlen(dbd->idxbuf);
    /*
     * set used idx buf
     */
    dbd->idxbuf[dbd->idxlen_u] = '\0';

    return(dbd->ptrval);		/* return offset of next key in chain */
}

off_t _get_file_size(int fd) {
    struct stat file_stat;
    fstat(fd, &file_stat);
    off_t size = file_stat.st_size; 
    return size;
}


/* Read a chain ptr field from anywhere in the index file:
 * the free list pointer, a hash table chain ptr, or an
 * index record chain ptr.  */

off_t
_db_readptr(DB *db, off_t offset)
{
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    char	asciiptr[PTR_SZ + 1];

    /*if (lseek(db->idxfd, offset, SEEK_SET) == -1)
      errorlog("lseek error to ptr field");
      */
    if (__os_pread(db->idxfd, asciiptr, PTR_SZ, offset) != PTR_SZ)
        mylog_error(m_pLogGlobalCtrl->errorlog, "read error of ptr field for DB - %s:%s:%d", INFO_LOG_SUFFIX);

    asciiptr[PTR_SZ] = 0;		/* null terminate */
    return(atol(asciiptr));
}

/* Rewind the index file for db_nextrec().
 * Automatically called by db_open().
 * Must be called before first db_nextrec().
 */

void
db_rewind(DB *db, DBD *dbd)
{
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    off_t	offset;

    offset = (db->nhash + 1) * PTR_SZ;		/* +1 for free list ptr */

    /* We're just setting the file offset for this process
       to the start of the index records; no need to lock.
       +1 below for newline at end of hash table. */
    dbd->idxoff = offset+1;
    if ( (dbd->idxoff = lseek(db->idxfd, offset+1, SEEK_SET)) == -1)
        mylog_error(m_pLogGlobalCtrl->errorlog, "lseek error for DB - %s:%s:%d", INFO_LOG_SUFFIX);
}

/* Print and reset the accumulated statistics. */

void
db_stats(DB *db)
{
    fprintf(stderr, "%d: del=%ld+%ld, fetch=%ld+%ld, next=%ld, "
        "store=%ld(%ld+%ld+%ld+%ld+%ld)\n", getpid(),
        db->cnt_delok, db->cnt_delerr,
        db->cnt_fetchok, db->cnt_fetcherr,
        db->cnt_nextrec,
        (db->cnt_stor1 + db->cnt_stor2 + db->cnt_stor3 + 
         db->cnt_stor4 + db->cnt_storerr),
        db->cnt_stor1, db->cnt_stor2, db->cnt_stor3, 
        db->cnt_stor4, db->cnt_storerr);

    db->cnt_delok = db->cnt_delerr =
        db->cnt_fetchok = db->cnt_fetcherr = db->cnt_nextrec =
        db->cnt_stor1 = db->cnt_stor2 = db->cnt_stor3 =
        db->cnt_stor4 = db->cnt_storerr = 0;
}

/* Store a record in the database.
 * Return 0 if OK, 1 if record exists and DB_INSERT specified,
 * -1 if record doesn't exist and DB_REPLACE specified. 
 *  2 if idx file or data file reach its max size
 *  3 if data reach its max length
 *  */
#ifdef __kdb_bin_
int db_store(DB *db, const char *key, const void *data, size_t dat_len, int flag, POS *pos)
#else
int db_store(DB *db, const char *key, const char *data, int flag, POS *pos)
#endif
{
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    int		rc, keylen, datlen;
    off_t	ptrval;

    DBD *dbd = dbd_alloc(db);
    keylen = strlen(key);
#ifdef __kdb_bin_
    datlen = dat_len;
#else
    datlen = strlen(data) + 1;		/* +1 for newline at end */
#endif
    /*
     * initial dbd by kurt
     */
    dbd->isfree = IS_NOT_FREE_SPACE;

    if (datlen < DATLEN_MIN || datlen > DATLEN_MAX) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "invalid data length %d for DB - %s:%s:%d\n",datlen,INFO_LOG_SUFFIX);
        dbd_free(dbd);
        return 3;
    }

    /* _db_find() calculates which hash table this new record
       goes into (db->chainoff), regardless whether it already
       exists or not.  The calls to _db_writeptr() below
       change the hash table entry for this chain to point to
       the new record.  This means the new record is added to
       the front of the hash chain. */

    off_t idx_offset = 0;
    if (pos != NULL) {
        idx_offset  = pos->offset;
    }

    //pthread_mutex_lock(db->lock);

    if (_db_find(db, dbd, key, 1, idx_offset) < 0) {		/* record not found */
        if (flag & DB_REPLACE) {
            rc = -1;
            db->cnt_storerr++;
            goto doreturn;		/* error, record does not exist */
        }

        /* _db_find() locked the hash chain for us; read the
           chain ptr to the first index record on hash chain */
        ptrval = _db_readptr(db, dbd->chainoff);
        if (_db_findfree_cache(db, dbd, keylen, datlen) < 0) {
            /* An empty record of the correct size was not found.
               We have to append the new record to the ends of
               the index and data files */
            if (_db_is_enough_file_space(db, keylen, datlen) != 0) {
                rc = 2;
                goto doreturn;
            }
            dbd->idxlen_u = keylen;

#ifdef __kdb_bin_
            _db_writedat_bin(db, dbd, data, dat_len, 0, SEEK_END);
#else
            _db_writedat(db, dbd, data, 0, SEEK_END);
#endif
            _db_writeidx(db, dbd, key, 0, SEEK_END, ptrval);
            /* db->idxoff was set by _db_writeidx().  The new
               record goes to the front of the hash chain. */
            _db_writeptr(db, dbd->chainoff, dbd->idxoff);
            db->cnt_stor1++;
        } else {
            /* We can reuse an empty record.
               _db_findfree() removed the record from the free
               list and set both db->datoff and db->idxoff. */
#ifdef __kdb_bin_
            _db_writedat_bin(db, dbd, data, dat_len, dbd->datoff, SEEK_SET);
#else
            _db_writedat(db, dbd, data, dbd->datoff, SEEK_SET);
#endif
            _db_writeidx(db, dbd, key, dbd->idxoff, SEEK_SET, ptrval);
            /* reused record goes to the front of the hash chain. */
            _db_writeptr(db, dbd->chainoff, dbd->idxoff);

            db->cnt_stor2++;
        }

    } else {						/* record found */
        if (flag & DB_INSERT) {
            rc = 1;
            db->cnt_storerr++;
            goto doreturn;		/* error, record already in db */
        }

        if (datlen > dbd->datlen) {
            /*
             *check if have free space
             */
            /*
               DBD *dbd_old = dbd_alloc(db);
               _db_copy(dbd, dbd_old);

               dbd_old->ptroff = dbd->ptroff; // for we don't copy ptroff
               _db_dodelete(db, dbd_old);	// delete the existing record 
               dbd_free(dbd_old);
               */
            _db_rwlock_unlock(db, dbd->chainoff);

            _db_delete(db, key, NULL, 0);

            _db_rwlock_wrlock(db, dbd->chainoff);
            /* Reread the chain ptr in the hash table
               (it may change with the deletion). */
            if (_db_findfree_cache(db, dbd, keylen, datlen) < 0) {

                //ptrval = _db_readptr(db, dbd->chainoff);
                ptrval = _db_readptr(db, _db_chain_off(db, key));
                if (_db_is_enough_file_space(db, keylen, datlen) != 0) {
                    rc = 2;
                    goto doreturn;
                }
                //ptrval = _db_readptr(db, dbd->chainoff);
                /* An empty record of the correct size was not found.
                   We have to append the new record to the ends of
                   the index and data files */
                dbd->idxlen_u = keylen;
#ifdef __kdb_bin_
                _db_writedat_bin(db, dbd, data, dat_len, 0, SEEK_END);
#else
                _db_writedat(db, dbd, data, 0, SEEK_END);
#endif
                _db_writeidx(db, dbd, key, 0, SEEK_END, ptrval);
                /* db->idxoff was set by _db_writeidx().  The new
                   record goes to the front of the hash chain. */
                _db_writeptr(db, dbd->chainoff, dbd->idxoff);

                db->cnt_stor3++;
            } else {
                /* We can reuse an empty record.
                   _db_findfree() removed the record from the free
                   list and set both db->datoff and db->idxoff. */
#ifdef __kdb_bin_
                _db_writedat_bin(db, dbd, data, dat_len, dbd->datoff, SEEK_SET);
#else
                _db_writedat(db, dbd, data, dbd->datoff, SEEK_SET);
#endif
                ptrval = _db_readptr(db, _db_chain_off(db, key));
                _db_writeidx(db, dbd, key, dbd->idxoff, SEEK_SET, ptrval);
                /* reused record goes to the front of the hash chain. */
                _db_writeptr(db, dbd->chainoff, dbd->idxoff);

                db->cnt_stor2++;
            }
        } else {
            /* no greater than length of data, just replace data record */
#ifdef __kdb_bin_
            _db_writedat_bin(db, dbd, data, dat_len, dbd->datoff, SEEK_SET);
#else
            _db_writedat(db, dbd, data, dbd->datoff, SEEK_SET);
#endif
            _db_writeidx(db, dbd, key, dbd->idxoff, SEEK_SET, dbd->ptrval);
            db->cnt_stor4++;
        }
    }
    rc = 0;		/* OK */
    if (pos != NULL) {
        pos->offset = dbd->idxoff;
    }
doreturn:	/* unlock the hash chain that _db_find() locked */
    /*
       if (un_lock(db->idxfd, dbd->chainoff, SEEK_SET, 1) < 0)
       errorlog("un_lock error");
       */
    _db_rwlock_unlock(db, dbd->chainoff);
    //pthread_mutex_unlock(db->lock); 
    dbd_free(dbd);
    return(rc);
}

/* Write a data record.  Called by _db_dodelete() (to write
   the record with blanks) and db_store(). */
/*
 * Return 0 if ok, 1 out of max file size
 */

void
_db_writedat(DB *db, DBD *dbd, const char *data, off_t offset, int whence)
{
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    struct iovec	iov[2];
    static char		newline = '\n';

    /* If we're appending, we have to lock before doing the lseek()
       and write() to make the two an atomic operation.  If we're
       overwriting an existing record, we don't have to lock. */
    if (whence == SEEK_END)	{	/* we're appending, lock entire file */
        offset = db->dat_len;
        _db_rwlock_wrlock(db, END_OFF);       
        /*
           if (writew_lock(db->datfd, 0, SEEK_SET, 0) < 0)
           errorlog("writew_lock error");
           */

        dbd->datlen = strlen(data) + 1;	/* datlen includes newline */
        dbd->datlen_u = dbd->datlen;
        db->dat_len += dbd->datlen; /* new file size */
    } else {
        dbd->datlen_u = strlen(data) + 1;	/* datlen includes newline */
    }

    /*if ( (db->datoff = lseek(db->datfd, offset, whence)) == -1)
      errorlog("lseek error");
      */
    dbd->datoff = offset;
    iov[0].iov_base = (char *) data;
    iov[0].iov_len  = dbd->datlen_u - 1;
    iov[1].iov_base = &newline;
    iov[1].iov_len  = 1;
    int ret = 0;
    if ((ret = __os_physpwrite(db->datfd, iov[0].iov_base, iov[0].iov_len, offset)) != iov[0].iov_len)
        mylog_error(m_pLogGlobalCtrl->errorlog, "writev error of data record %d %d for DB - %s:%s:%d",ret, iov[0].iov_len,INFO_LOG_SUFFIX);
    ret = 0;
    if ((ret = __os_physpwrite(db->datfd, iov[1].iov_base, iov[1].iov_len, offset + iov[0].iov_len)) != iov[1].iov_len)
        mylog_error(m_pLogGlobalCtrl->errorlog, "writev error of data record %d %d for DB - %s:%s:%d", ret, iov[1].iov_len,INFO_LOG_SUFFIX);

    if (whence == SEEK_END) {
        _db_rwlock_unlock(db, END_OFF);       
    }
    /*
       if (whence == SEEK_END)
       if (un_lock(db->datfd, 0, SEEK_SET, 0) < 0)
       errorlog("un_lock error");
       */
}

void
_db_writedat_bin(DB *db, DBD *dbd, const void *data, size_t length, off_t offset, int whence)
{
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    /* If we're appending, we have to lock before doing the lseek()
       and write() to make the two an atomic operation.  If we're
       overwriting an existing record, we don't have to lock. */
    if (whence == SEEK_END)	{	/* we're appending, lock entire file */
        offset = db->dat_len;
        /*
           if (writew_lock(db->datfd, 0, SEEK_SET, 0) < 0)
           errorlog("writew_lock error");
           */

        dbd->datlen = length;	/* datlen includes newline */
        dbd->datlen_u = dbd->datlen;
        db->dat_len += dbd->datlen; /* new file size */
        _db_rwlock_wrlock(db, END_OFF);       
    } else {
        dbd->datlen_u = length;	/* datlen includes newline */
    }

    dbd->datoff = offset;
    /*
     * write data
     */
    int ret = 0;
    if (length > 0) {
        if ((ret = __os_physpwrite(db->datfd, (void *)data, length, offset)) != length)
            mylog_error(m_pLogGlobalCtrl->errorlog, "writev error of data record %d %d for DB - %s:%s:%d",ret, length,INFO_LOG_SUFFIX);
    }
    if (whence == SEEK_END) {
        _db_rwlock_unlock(db, END_OFF);       
    }
    /*
       if (whence == SEEK_END)
       if (un_lock(db->datfd, 0, SEEK_SET, 0) < 0)
       errorlog("un_lock error");
       */

}

/* Write an index record.
 * _db_writedat() is called before this function, to set the fields
 * datoff and datlen in the DB structure, which we need to write
 * the index record. */
/*
 * Return 0 ok, -1 idx length exceed max length, -2 error
 */
int
_db_writeidx(DB *db, DBD *dbd, const char *key,
    off_t offset, int whence, off_t ptrval)
{
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    struct iovec	iov[2];
    char			asciiptrlen[PTR_SZ + IDXLEN_SZ +1];
    int				len;

    if ( (dbd->ptrval = ptrval) < 0 || ptrval > PTR_MAX) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "invalid ptr: %d for DB - %s:%s:%d",ptrval,INFO_LOG_SUFFIX);
        return -2;
    }

    sprintf(dbd->idxbuf, "%-*s%c%ld%c%d%c%d%c%d%c%d\n",
        dbd->idxlen_a, key, SEP, dbd->datoff, SEP, dbd->datlen, SEP, strlen(key), SEP, dbd->datlen_u, SEP, dbd->isfree);
    if ( (len = strlen(dbd->idxbuf)) < IDXLEN_MIN || len > IDXLEN_MAX) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "invalid length for DB - %s:%s:%d",INFO_LOG_SUFFIX);
        return -1;
    }
    if (dbd->idxlen > len) {
        memset(&dbd->idxbuf[len - 1], ' ', dbd->idxlen - len);
        len = dbd->idxlen;
        dbd->idxbuf[len - 1] = '\n';
    } else if (dbd->idxlen < len) {
        whence = SEEK_END;
    }
    sprintf(asciiptrlen, "%*ld%*d", PTR_SZ, ptrval, IDXLEN_SZ, len);

    /* If we're appending, we have to lock before doing the lseek()
       and write() to make the two an atomic operation.  If we're
       overwriting an existing record, we don't have to lock. */
    if (whence == SEEK_END)	{	/* we're appending */
        _db_rwlock_wrlock(db, END_OFF);       
        /*
           if (writew_lock(db->idxfd, ((db->nhash+1)*PTR_SZ)+1,
           SEEK_SET, 0) < 0)
           errorlog("writew_lock error");
           */
        offset = db->idx_len;
        db->idx_len += (len + PTR_SZ + IDXLEN_SZ); /* new file size */
    }

    /* Position the index file and record the offset */
    /*if ( (db->idxoff = lseek(db->idxfd, offset, whence)) == -1)
      errorlog("lseek error");
      */
    dbd->idxoff = offset;
    iov[0].iov_base = asciiptrlen;
    iov[0].iov_len  = PTR_SZ + IDXLEN_SZ;
    iov[1].iov_base = dbd->idxbuf;
    iov[1].iov_len  = len;
    int ret = 0;
    if ((ret = __os_physpwrite(db->idxfd, iov[0].iov_base, iov[0].iov_len, offset)) != PTR_SZ + IDXLEN_SZ)
        mylog_error(m_pLogGlobalCtrl->errorlog, "writev error of data record %d %d for DB - %s:%s:%d",ret, PTR_SZ + IDXLEN_SZ,INFO_LOG_SUFFIX);

    ret = 0;
    if ((ret = __os_physpwrite(db->idxfd, iov[1].iov_base, iov[1].iov_len, offset + iov[0].iov_len)) != len)
        mylog_error(m_pLogGlobalCtrl->errorlog, "writev error of data record %d %d for DB - %s:%s:%d",ret, len,INFO_LOG_SUFFIX);

    if (whence == SEEK_END) {
        _db_rwlock_unlock(db, END_OFF);       
    }
    /*
       if (whence == SEEK_END)
       if (un_lock(db->idxfd, ((db->nhash+1)*PTR_SZ)+1, SEEK_SET, 0) < 0)
       errorlog("un_lock error");
       */
    return 0;
}

/* Write a chain ptr field somewhere in the index file:
 * the free list, the hash table, or in an index record. */

void
_db_writeptr(DB *db, off_t offset, off_t ptrval)
{
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    char	asciiptr[PTR_SZ + 1];

    if (ptrval < 0 || ptrval > PTR_MAX)
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "invalid ptr: %d for DB - %s:%s:%d",ptrval, INFO_LOG_SUFFIX);
    sprintf(asciiptr, "%*ld", PTR_SZ, ptrval);

    /*if (lseek(db->idxfd, offset, SEEK_SET) == -1)
      errorlog("lseek error to ptr field");
      */
    int ret = 0;
    if ((ret = __os_physpwrite(db->idxfd, asciiptr, PTR_SZ, offset)) != PTR_SZ)
        mylog_error(m_pLogGlobalCtrl->errorlog, "writev error of data record %d %d for DB - %s:%s:%d",ret, PTR_SZ,INFO_LOG_SUFFIX);
}

/*
 * Read free space list from db file into struct list
 */
FSDBD * 
//_db_retrieve_free_space(DB *db, size_t *idx_size_of_free_space, int *count_of_free_space, size_t *dat_size_of_free_space) 
_db_retrieve_free_space(DB *db, int *count_of_free_space) 
{
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    off_t   offset, nextoffset, saveoffset;
    int     count;

    /* Read the free list pointer */
    saveoffset = FREE_OFF;
    offset = _db_readptr(db, saveoffset);
    if (offset == 0) {
        return NULL;
    }

    /* Loop through the free list */
    count = 0;
    DBD *dbd = dbd_alloc(db);
    FSDBD *list_head = NULL, *item_last = NULL, *item = NULL;
    list_head = (FSDBD *)calloc(sizeof(FSDBD), 1);
    item_last = list_head;
    while (offset != 0) {
        item = (FSDBD *)calloc(sizeof(FSDBD), 1);
        count++;
        nextoffset = _db_readidx(db, dbd, offset, NULL);
        item->offset = offset;
        item->datlen = dbd->datlen;   /*Total length of data*/
        item->idxlen_a = dbd->idxlen_a; /*Total length of idx*/
        item->prev = item_last;
        item->prev->next = item;

        saveoffset = offset;
        offset = nextoffset;
        item_last = item;
        if (offset < 0) {
            mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "DB Record ERROR for DB - %s:%s:%d:%d",ERROR_LOG_SUFFIX);
        }

    }
    dbd_free(dbd);
    *count_of_free_space = count;
    return(list_head);
}
/*
 * read used space list from db file into struct list
 */
void *
_db_retrieve_used_space(DB *db, size_t *idx_size_of_free_space, int *count_of_free_space, size_t *dat_size_of_free_space)
{
    DBD *dbd = dbd_alloc(db);
    db_rewind(db, dbd);
    off_t last_offset = dbd->idxoff;
    /* We read lock the free list so that we don't read
       a record in the middle of its being deleted. */
    off_t offset = last_offset;
    while(offset != 0) {
        do {
            /* read next sequential index record */
            offset = last_offset;
            if (_db_readidx(db, dbd, offset, &last_offset) < 0) {
                goto doreturn;
            }
        } while (!dbd->isfree);	
        *idx_size_of_free_space += dbd->idxlen;
        *dat_size_of_free_space += dbd->datlen;
        (*count_of_free_space)++;
    }
doreturn:
    dbd_free(dbd);
    return NULL;
}

void
_db_copy(DBD *dbd_src, DBD *dbd_des) {
    dbd_des->datlen = dbd_src->datlen;
    dbd_des->idxlen = dbd_src->idxlen;
    dbd_des->datlen_u = dbd_src->datlen_u;
    dbd_des->idxlen_u = dbd_src->idxlen_u;
    dbd_des->idxlen_a = dbd_src->idxlen_a;
    dbd_des->idxoff = dbd_src->idxoff;
    dbd_des->datoff = dbd_src->datoff;
    dbd_des->ptrval = dbd_src->ptrval;
    strncpy(dbd_des->idxbuf, dbd_src->idxbuf, dbd_src->idxlen_a);
    dbd_des->idxbuf[dbd_src->idxlen_a] = '\0';
    //dbd_des->datbuf = dbd_src->datbuf;
}

/*
 * Not very accurate, for idxlen is only key length
 * Return 0 OK, 1 data not enough, 2 idx not enough
 */
int
_db_is_enough_file_space(DB *db, off_t idxlen, off_t datlen) {
    /*
     * test if data file is enough
     */
    if ((db->dat_len  + datlen) > DATA_FILELEN_MAX) {
        //printf(" out of data size %ld > %d\n", db->dat_len  + datlen, DATA_FILELEN_MAX);
        return 1;
    } 
    if ((db->idx_len + idxlen) > PTR_MAX) {
        //printf(" out of idx size %ld > %d\n", db->idx_len + idxlen, PTR_MAX);
        return 2;
    }
    return 0;
}

void
_db_dump_dbd(DBD *dbd) {
    printf("------------- dump dbd ------------\n");
    printf("datlen = %d, idxlen = %d, idxoff = %ld, datoff = %ld, isfree = %d, ptroff = %ld, ptrval = %ld, chainoff = %ld, idxlen_u = %d, datlen_u = %d, idxlen_a = %d\n", dbd->datlen, dbd->idxlen, dbd->idxoff, dbd->datoff, dbd->isfree, dbd->ptroff, dbd->ptrval, dbd->chainoff, dbd->idxlen_u, dbd->datlen_u, dbd->idxlen_a);
}

void
_db_dump_db_size(DB *db) {
    //printf("------------- dump db size ------------\n");
    //printf("idxlen = %d, datlen = %d\n", db->idx_len, db->dat_len);
}

FSDBD *
_db_cache_add(DB *db, DBD *dbd) {
    FSDBD *item = (FSDBD *)calloc(sizeof(FSDBD), 1);
    item->offset = dbd->idxoff;
    item->datlen = dbd->datlen;   /*Total length of data*/
    item->idxlen_a = dbd->idxlen_a; /*Total length of idx*/
    item->prev = NULL;
    if (db->free_list != NULL) {
        /*
         * add the new item to the top of the list
         */
        db->free_list->prev = item; 
        item->next = db->free_list;
    }
    db->free_list = item;
    return item;
}
void
_db_cache_delete(DB *db, FSDBD *item) {
    if (item->prev != NULL) {
        item->prev->next = item->next;

        //printf("get in there item->prev->offset = %d, item->next->offset = %d\n", item->prev->offset, item->next->offset);
        if (item->next != NULL) {
            item->next->prev = item->prev;
        }
    } else{ /* top of the list*/
        db->free_list = item->next;
        if (db->free_list != NULL) {
            db->free_list->prev = NULL;
        }
    }
}

void 
_db_cache_free(FSDBD *item) {
    if (item != NULL) {
        free(item);
        item = NULL;
    }
}
void
_db_cache_dump(DB *db) {
    printf("------------- dump cache ------------\n");
    if (db != NULL) {
        FSDBD *fsdbd = db->free_list;
        while(fsdbd != NULL) {
            fsdbd = fsdbd->next;
            printf("fsdbd");
        }
    }
}
off_t _db_chain_off(DB *db, const char *key) {
    return (_db_hash(db, key) * PTR_SZ) + db->hashoff;
}

void
_db_create_db_template_files(char *path) {
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    int i;
    char		asciiptr[PTR_SZ + 1];
    //				hash[(NHASH_DEF + 1) * PTR_SZ + 2];
    tmp_db_fd = open(path, O_RDWR | O_CREAT | O_TRUNC, FILE_MODE); 
    if (tmp_db_fd  == -1) {
        mylog_error(m_pLogGlobalCtrl->errorlog, "can not open db template file for DB - %s:%s:%d",INFO_LOG_SUFFIX);
    }
    sprintf(asciiptr, "%*d", PTR_SZ, 0);
    //   hash[0] = 0;
    for (i = 0; i < (NHASH_DEF + 1); i++) {
        int len = strlen(asciiptr);
        if (write(tmp_db_fd, asciiptr, len) != len)
            mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "write error initializing template file for DB - %s:%s:%d:%d", ERROR_LOG_SUFFIX);
    }

    if (write(tmp_db_fd, "\n", 1) != 1)
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "write error initializing template file for DB - %s:%s:%d:%d", ERROR_LOG_SUFFIX);

    //strcat(hash, asciiptr);
    //strcat(hash, "\n");

    //i = strlen(hash);
    /*if (write(tmp_db_fd, hash, i) != i)
      errorlog("write error initializing template file");
      */
}

void    
_db_create_db_files(DB *db, char *dir) {
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    static const int TMP_BUFSIZE = 4096;
    pthread_mutex_lock(&db_tmp_file_lock);
    if (tmp_db_fd < 0) {
        if (tmp_db_fd < 0) {
            char tmp_file_name[100];
            tmp_file_name[0] = 0;
            strcat(tmp_file_name, dir);
            strcat(tmp_file_name, "/");
            strcat(tmp_file_name, DB_IDX_TMP_FILE_NAME);
            tmp_db_fd = open(tmp_file_name, O_RDONLY , FILE_MODE); 
            if (tmp_db_fd  == -1) {
                mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "Can not open db template file for DB - %s:%s:%d:%d", ERROR_LOG_SUFFIX);
            }
        }
    }
    char buf[TMP_BUFSIZE];
    memset(buf, '\0', TMP_BUFSIZE);
    int length = 0;
    lseek(tmp_db_fd, SEEK_SET, 0);
    int size = 0;
    while ((length = read(tmp_db_fd, buf, TMP_BUFSIZE)) != 0) {
        if (length == -1)
            mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "write error initializing index file for DB - %s:%s:%d:%d", ERROR_LOG_SUFFIX);
        if (write(db->idxfd, buf, length) != length)
            mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "write error initializing index file for DB - %s:%s:%d:%d", ERROR_LOG_SUFFIX);
        size += length;
        //Debug(0, 1, ("copy idxtemplate bytes %d current thread %d\n", size, pthread_self()));
    }
    pthread_mutex_unlock(&db_tmp_file_lock);
    if (size != 14155831) {
        mylog_info(m_pLogGlobalCtrl->infolog, "copy idxtemplate bytes %d failed %d - %s:%s:%d",  size, errno,INFO_LOG_SUFFIX);
    } else {
        mylog_info(m_pLogGlobalCtrl->infolog, "copy idxtemplate bytes %d successfully - %s:%s:%d",  size, INFO_LOG_SUFFIX);
    }
}

void 
_db_rwlock_rdlock(DB *db, off_t chainoff) {
#ifdef USE_MY_LOCK 
    while(!_db_rwlock_checkrdlock(db, chainoff)) {
        _db_pthread_sleep(10000);
    }
#else
    if (chainoff == FREE_OFF) {
        pthread_rwlock_rdlock(db->freelock);
    } else if (chainoff == END_OFF) {
        pthread_rwlock_rdlock(db->endlock);
    } else {
        pthread_rwlock_rdlock(db->commonlock);
    }
#endif
}

int
_db_rwlock_checkrdlock(DB *db, off_t chainoff) {
    char *key = NULL;
    ultoa(chainoff, &key);
    pthread_mutex_lock(db->lock);
    HashEntry rwlock = hash_find(db->locks, key, strlen(key));
    if (rwlock == NULL) {
        hash_find_or_add(db->locks, key, strlen(key), DB_LOCK_RDLOCK, 0);
        pthread_mutex_unlock(db->lock);
        free(key);
        return 1;
    }
    free(key);
    /*if (rwlock->value == DB_LOCK_RDLOCK) {
      pthread_mutex_unlock(db->lock);
      return 1;
      }
      if (rwlock->value == DB_LOCK_WRLOCK) {
      pthread_mutex_unlock(db->lock);
      return 0;
      }
      */
    pthread_mutex_unlock(db->lock);
    return 0;
}

void 
_db_rwlock_wrlock(DB *db, off_t chainoff) {
#ifdef USE_MY_LOCK 
    while(!_db_rwlock_checkwrlock(db, chainoff)) {
        _db_pthread_sleep(10000);
    }
#else
    if (chainoff == FREE_OFF) {
        pthread_rwlock_wrlock(db->freelock);
    } else if (chainoff == END_OFF) {
        pthread_rwlock_wrlock(db->endlock);
    } else {
        pthread_rwlock_wrlock(db->commonlock);
    }
#endif
}

int 
_db_rwlock_checkwrlock(DB *db, off_t chainoff) {
    char *key = NULL;
    ultoa(chainoff, &key);
    pthread_mutex_lock(db->lock);
    HashEntry rwlock = hash_find(db->locks, key, strlen(key));
    if (rwlock == NULL) {
        hash_find_or_add(db->locks, key, strlen(key), DB_LOCK_WRLOCK, 0);
        pthread_mutex_unlock(db->lock);
        free(key);
        return 1;
    }
    free(key);
    pthread_mutex_unlock(db->lock);
    return 0;
}

void
_db_rwlock_unlock(DB *db, off_t chainoff) {
#ifdef USE_MY_LOCK 
    char *key = NULL;
    ultoa(chainoff, &key);
    pthread_mutex_lock(db->lock);
    hash_remove(db->locks, key, strlen(key));
    pthread_mutex_unlock(db->lock);
    free(key);
#else
    if (chainoff == FREE_OFF) {
        pthread_rwlock_unlock(db->freelock);
    } else if (chainoff == END_OFF) {
        pthread_rwlock_unlock(db->endlock);
    } else {
        pthread_rwlock_unlock(db->commonlock);
    }
#endif
}

void 
_db_pthread_sleep(int useconds) {

    struct timeval now;
    struct timespec timeout;

    //just for timer
    pthread_mutex_t mutexTimer = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t condTimer = PTHREAD_COND_INITIALIZER;
    //

    gettimeofday(&now, NULL);
    timeout.tv_sec = now.tv_sec;
    timeout.tv_nsec = (now.tv_usec + useconds) * 1000;
    pthread_mutex_lock(&mutexTimer);
    pthread_cond_timedwait(&condTimer, &mutexTimer, &timeout);
    pthread_mutex_unlock(&mutexTimer);
}
int ultoa(unsigned long num, char **str)
{
    int ret = asprintf(str, "%lu", num);
    return ret;
}

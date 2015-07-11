#include "DBAccess.h"
#include "InfoCrawler.h"
DBAccess *DBAccess::_instance = NULL;

pthread_mutex_t DBAccess::mutex_open = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t DBAccess::mutex_load = PTHREAD_MUTEX_INITIALIZER;

DBAccess::DBAccess():dir(NULL) {
    m_pLogGlobalCtrl = NULL;
    if (m_pLogGlobalCtrl == NULL)
    {
        m_pLogGlobalCtrl= InfoCrawler::getInstance()->getLogGlobalCtrl();
    }
}

DBAccess::~DBAccess() {
    free(this->dir);
    /*
     * free DBAccessType
     */
    vector<DBAccessType *>::iterator iterator;
    for(iterator = dbs.begin(); iterator != dbs.end(); ++iterator) {
        //DBAccessType *tmp = *iterator;
        delete *iterator;
    }
}

DBAccess* const DBAccess::getInstance()
{
    if(_instance == NULL) {
        _instance = new DBAccess();
    } 
    return _instance;
} 

void DBAccess::destroydbs() {
   /* vector<DBAccessType *>::iterator iterator;
    for(iterator = dbs.begin(); iterator != dbs.end(); ++iterator) {
        //DBAccessType *tmp = *iterator;
        delete *iterator;
    }*/
/*    for(int i=0 ; i<dbs.size() ; i++)
    {
        unload(i);
    }
    dbs.clear();*/
    vector<DBAccessType *>::iterator iterator;
    for(iterator = dbs.begin(); iterator != dbs.end(); ++iterator) {
        delete *iterator;
    }
    dbsuffix.clear();
    dbs.clear();
}
void DBAccess::destroy() {
    delete this;
}

void DBAccess::setDir(char *dir) {
    if (this->dir != NULL) {
        free(this->dir);
    }
    this->dir = strdup(dir);
    make_path(this->dir);
}

int DBAccess::load(char *tablename) {
    //vector<DBAccessType *>::iterator iter; 
    map<string, int>::iterator iter; 
    pthread_mutex_lock(&mutex_load);
    /*for(iter =  dbs.begin(); iter != dbs.end(); ++iter) {
        if (strcmp((*iter)->filename.c_str(), tablename) == 0) {
            pthread_mutex_unlock(&mutex_load);
            return (*iter)->curPos;
        }
    }*/
    if ((iter = dbsuffix.find(tablename)) != dbsuffix.end()) {
        int ret = iter->second; 
        pthread_mutex_unlock(&mutex_load);
        return ret;
    }
    //if not yet loaded, load it
    int ret = _load(tablename);
    pthread_mutex_unlock(&mutex_load);
    return ret;
}

int DBAccess::_load(char *tablename) {
    dbs.push_back(new DBAccessType(tablename));
    int suffix = dbs.size() - 1;
    mylog_info(m_pLogGlobalCtrl->infolog, "tablename %s,suffix %d,dbs.size,%d - %s:%s:%d",tablename, suffix ,dbs.size() ,INFO_LOG_SUFFIX);
    DBAccessType *db_used = dbs[suffix];
    db_used->curPos = suffix; 
    string dbname(dbs[suffix]->filename);
    dbsuffix.insert(make_pair(tablename, suffix));
    if ((db_used->db_conf = db_open((char *)(getPath(dbname)).c_str(), 
                O_RDWR | O_CREAT | O_TRUNC, FILE_MODE, dir)) == NULL) {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "db_open error - %s:%s:%d",INFO_LOG_SUFFIX);
    }
    DBD *dbd = NULL;
    off_t last_offset = 0;
    while((dbd = db_nextrec(db_used->db_conf, dbd, NULL, &last_offset)) != NULL) {
        string data(dbd->datbuf, dbd->datlen_u);
        db_used->databases_conf.insert(make_pair(string(dbd->idxbuf), data));
        Format f(data);
        f.parse();
        //char **record = f.getRecord();
        if (f.getCount() != DB_CONF_FIELD_COUNT) {
            mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "db_open error, config error - %s:%s:%d", INFO_LOG_SUFFIX);
        }
        string fileno = dbd->idxbuf;
        openExistDB(suffix, fileno);
        db_used->count_of_database++;
    }
    dbd_free(dbd);
    return suffix;
}

void DBAccess::unload(int suffix) {
    pthread_mutex_lock(&mutex_load);
    remove(suffix);
    pthread_mutex_unlock(&mutex_load);
    mylog_info(m_pLogGlobalCtrl->infolog, "suffix %d,dbs.size,%d - %s:%s:%d",suffix ,dbs.size() ,INFO_LOG_SUFFIX);
}
/*
 * must unload first;
 */
void DBAccess::remove(int suffix) {
    DBAccessType *db_used = dbs[suffix];
    dbs[suffix] = NULL;
    dbsuffix.erase(db_used->filename);
    string dbname(db_used->filename);
    if ((db_used->db_conf = db_open((char *)(getPath(dbname)).c_str(), 
                O_RDWR | O_CREAT | O_TRUNC, FILE_MODE, dir)) == NULL) {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "db_open error - %s:%s:%d", INFO_LOG_SUFFIX);
    }
    DBD *dbd = NULL;
    off_t last_offset = 0;
    vector<string> db_to_remove;
    while((dbd = db_nextrec(db_used->db_conf, dbd, NULL, &last_offset)) != NULL) {
        string data(dbd->datbuf, dbd->datlen_u);
        Format f(data);
        f.parse();
        //char **record = f.getRecord();
        if (f.getCount() != DB_CONF_FIELD_COUNT) {
            mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "db_open error, can not get size of db - %s:%s:%d", INFO_LOG_SUFFIX);
        }
        string fileno = dbd->idxbuf;
        string dbfile=  getPath(fileno, (char *)db_used->filename.c_str()).c_str();
        string idxfile = dbfile + DB_IDX_EXTENSION;
        string datfile = dbfile + DB_DAT_EXTENSION;
        //remove db file 
        int ret = ::remove(idxfile.c_str());
        ret = ::remove(datfile.c_str());
        //remove db from config
        db_delete(db_used->db_conf, (char *)fileno.c_str(), NULL);
    }
    dbd_free(dbd);
    delete db_used;
}

DB *DBAccess::openExistDB(int suffix, string &fn) {
    DBAccessType *db_used = dbs[suffix];
    DBD *dbd = db_fetch(db_used->db_conf, (char *)fn.c_str(), NULL);
    DB *db;
    if (dbd != NULL) {
        string data(dbd->datbuf, dbd->datlen_u);
        Format f(data);
        f.parse();
        char **record = f.getRecord();
        char timestr[20] ;timestr[0] = 0;
        int2str(time(NULL), timestr, 10, 0);
        record[0] = timestr;
        string newdata = f.toString();
#ifdef __kdb_bin_
        db_store(db_used->db_conf, (char *)fn.c_str(), newdata.c_str(), data.length(), DB_REPLACE, NULL);
#else
        db_store(db_used->db_conf, (char *)fn.c_str(), newdata.c_str(), DB_REPLACE, NULL);
#endif
        string fileno = fn;
        if ((db = db_open((char *)(getPath(fileno, (char *)db_used->filename.c_str())).c_str(), O_RDWR | O_CREAT | O_TRUNC,
                                            FILE_MODE, dir)) == NULL) {
            mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "db_open error - %s:%s:%d", INFO_LOG_SUFFIX);
        }
        _open(suffix, fn, db);
        dbd_free(dbd);
        return db;
    }
    return NULL;
}
DB *DBAccess::openNewDB(int suffix, string &fn) {
    DBAccessType *db_used = dbs[suffix];
    pthread_mutex_lock(&mutex_open);

    DB *db;
    //open new
    int dir = (db_used->count_of_database + 1) / MAX_DATABASES_PER_DIR;
    char dirstr[20] ;dirstr[0] = 0;
    int2str(dir, dirstr, 10, 0);
    string fileno =  dirstr;
    fileno.append("/");
    fileno.append(db_used->filename);
    fileno.append(".");
    int2str(db_used->count_of_database, dirstr, 10, 0);
    fileno.append(dirstr);

    char *key = (char *)fileno.c_str();
    string filename = getPath(fileno, (char *)db_used->filename.c_str());
    int pos = filename.find_last_of("/");
    string path = filename.substr(0, pos);
    make_path((char *)path.c_str());

    if ((db = db_open((char *)filename.c_str(), O_RDWR | O_CREAT | O_TRUNC,
                                        FILE_MODE, this->dir)) == NULL) {
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "db_open error - %s:%s:%d", INFO_LOG_SUFFIX);
    }

    string data;
    char timestr[20] ;timestr[0] = 0;
    int2str(time(NULL), timestr, 10, 0);
    data = timestr;
    
    db_used->databases_conf.insert(make_pair(string(key), data));

#ifdef __kdb_bin_
    if (db_store(db_used->db_conf, key, data.c_str(), data.length(), DB_INSERT, NULL) != 0) {
#else
    if (db_store(db_used->db_conf, key, data.c_str(), DB_INSERT, NULL) != 0) {
#endif
        mylog_fatal(m_pLogGlobalCtrl->veryimportantlog, "db_open error - %s:%s:%d", INFO_LOG_SUFFIX);
    }
    _open(suffix, fileno, db);
    db_used->count_of_database++;

    fn.assign(key);
    pthread_mutex_unlock(&mutex_open);
    return db;
}

void DBAccess::_open(int suffix, string &fileno, DB *db) {
    DBAccessType *db_used = dbs[suffix];
    if (db_used->dbs.size() > TOP_DATABASES) {
        db_close(db_used->dbs.end()->second); //close db
        db_used->dbs.erase(db_used->dbs.end());
    }
    db_used->dbs[fileno] = db;
}

string DBAccess::getPath(string &filename, char *hashfilename) {
    string path;
    int d = 0;
    if (hashfilename) {
        d = my_hash(hashfilename, MAX_DB_DIR); 
    } else {
        d = my_hash((char *)filename.c_str(), MAX_DB_DIR); 
    }
    char dirstr[10] ;dirstr[0] = 0;
    sprintf(dirstr, "%d", d);
    path.append(dir).append("/").append(dirstr);

    if (access(path.c_str(), R_OK | W_OK) == -1) {
        make_path((char *)path.c_str());
    }

    path.append("/").append(filename);
    return path;
}

string DBAccess::getPath(char * filename, char *hashfilename) {
    string path;
    int d = 0;
    if (hashfilename) {
        d = my_hash(hashfilename, MAX_DB_DIR); 
    } else {
        d = my_hash(filename, MAX_DB_DIR); 
    }

    char dirstr[10] ;dirstr[0] = 0;
    sprintf(dirstr, "%d", d);
    path.append(dir).append("/").append(dirstr);

    if (access(path.c_str(), R_OK | W_OK) == -1) {
        make_path((char *)path.c_str());
    }
    path.append("/").append(filename);
    return path;
}

/*
 * 0: ok
 * 1: ok, but return a new fileno
 * 2: error
 * 3: data length exceed, key is null
 */
int DBAccess::store(int suffix, char *key, const char *data, string &fileno, POS *pos) {
    return store(suffix, key, data, strlen(data), fileno, pos);
}

int DBAccess::store(int suffix, char *key, const void *data, size_t datlen, string &fileno, POS *pos) {
    if (strlen(key) == 0) {
        return 3;
    }
    DBAccessType *db_used = dbs[suffix];
    DB *db = NULL;
    string new_fileno;
    int returnValue = 0, ret = 0;
    bool saved = false;

    pthread_rwlock_wrlock(&(db_used->rwlock_dbs));
    if (!fileno.empty() && (db = db_used->dbs[fileno]) == NULL) {
        db = openExistDB(suffix, fileno);
        fileno.assign(fileno);
        returnValue = 0;
    }
    pthread_rwlock_unlock(&(db_used->rwlock_dbs));

    //struct timeval now, now1;
    if (db != NULL) {
        //gettimeofday(&now, NULL);
        // Log("INFO: DBAccess db_store1 = %d %d\n", now.tv_sec, now.tv_usec);
#ifdef __kdb_bin_
        ret = db_store(db, key, (const char *)data, datlen, DB_REPLACE, pos);
#else
        ret = db_store(db, key, (const char *)data, DB_REPLACE, pos);
#endif
        //gettimeofday(&now1, NULL);
        // Log("INFO: DBAccess db_store1 end = %d %d\n", now1.tv_sec - now.tv_sec, now1.tv_usec - now.tv_usec);
        if (ret == -1) { //data not exist
        //gettimeofday(&now, NULL);
         //Log("INFO: DBAccess db_store2 = %d %d\n", now.tv_sec, now.tv_usec);
#ifdef __kdb_bin_
            ret = db_store(db, key, data, datlen, DB_INSERT, pos);
#else
            ret = db_store(db, key, (char *)data, DB_INSERT, pos);
#endif
        //gettimeofday(&now1, NULL);
        // Log("INFO: DBAccess db_store2 end = %d %d\n", now1.tv_sec - now.tv_sec, now1.tv_usec - now.tv_usec);
        } else if (ret == 2) { //exists but can not be saved
            db_delete(db, key, pos);
        }
        returnValue = 0;
        saved = true;
    } else {
        ret = 2; //should find an available db
    }

    /*
     * If cannot be saved in original db file or have not been saved, 
     * remove it from original(already been saved) and then save it in a new db file.
     */
    if (ret == 2) {
        saved = false;
        map<string, string>::iterator iterator;

        for(iterator = db_used->databases_conf.begin(); iterator != db_used->databases_conf.end(); ++iterator) {
            string fileno_tmp = iterator->first;

            map<string, DB *>::iterator dbs_iterator;

            pthread_rwlock_wrlock(&(db_used->rwlock_dbs));
            if ((dbs_iterator = db_used->dbs.find(fileno_tmp)) == db_used->dbs.end()) {
                string fileno = fileno_tmp;
                db = openExistDB(suffix, fileno);
            } else {
                db = dbs_iterator->second;
            }
            pthread_rwlock_unlock(&(db_used->rwlock_dbs));
            if (db != NULL) {
                //gettimeofday(&now, NULL);
                 //Log("INFO: DBAccess db_store3 = %d %d\n", now.tv_sec, now.tv_usec);
#ifdef __kdb_bin_
                ret = db_store(db, key, data, datlen, DB_REPLACE, pos);
#else
                ret = db_store(db, key, (char *)data, DB_REPLACE, pos);
#endif
                //gettimeofday(&now1, NULL);
                // Log("INFO: DBAccess db_store3 end = %d %d\n", now1.tv_sec - now.tv_sec, now1.tv_usec - now.tv_usec);
                if (ret == -1) { //data not exist
                //gettimeofday(&now, NULL);
                // Log("INFO: DBAccess db_store4  = %d %d\n", now.tv_sec, now.tv_usec);
#ifdef __kdb_bin_
                    ret = db_store(db, key, data, datlen, DB_INSERT, pos);
#else
                    ret = db_store(db, key, (char *)data, DB_INSERT, pos);
#endif
                //gettimeofday(&now1, NULL);
                // Log("INFO: DBAccess db_store4 end = %d %d\n", now1.tv_sec - now.tv_sec, now1.tv_usec - now.tv_usec);
                }
                if (ret == 0) {
                    fileno.assign(fileno_tmp);
                    returnValue = 1;
                    saved = true;
                    break;
                } else if (ret == 3) { //error 
                    returnValue = 2;
                    saved = true;
                    break;
                }
            }
        }
    }

    if (!saved) {
        if (pthread_rwlock_trywrlock(&(db_used->rwlock_dbs)) != 0) { //other thread is opening new db 
            return store(suffix, key, data, datlen, fileno, pos);
        }
        db = openNewDB(suffix, new_fileno);
        pthread_sleep(5); //in order that other thread open new db
        pthread_rwlock_unlock(&(db_used->rwlock_dbs));
        fileno.assign(new_fileno);
        returnValue = 1;

#ifdef __kdb_bin_
        ret = db_store(db, key, data, datlen, DB_REPLACE, pos);
#else
        ret = db_store(db, key, data, DB_REPLACE, pos);
#endif
        if (ret == -1) { //data not exist
#ifdef __kdb_bin_
            ret = db_store(db, key, data, datlen, DB_INSERT, pos);
#else
            ret = db_store(db, key, data, DB_INSERT, pos);
#endif
        }

        if (ret != 0) {
            returnValue = 2;
        }
    }

    return returnValue;
}
/*
 *0, ok
 *1, fail
 *-1, not found 
 */
int DBAccess::erase(int suffix, char *key, string &fileno, POS *pos)  {
    DBAccessType *db_used = dbs[suffix];
    map<string, string>::iterator iterator; 
    DB *db = NULL;

    //If key found, return it, otherwise go through all database to find it
    if (!fileno.empty()) {
        db = db_used->dbs[fileno];
        if (db) {
            int ret = db_delete(db, key, pos);
            return ret;
        }
    }
    
    for(iterator = db_used->databases_conf.begin(); iterator != db_used->databases_conf.end(); ++iterator) {
        string fileno_tmp = iterator->first;

        map<string, DB *>::iterator dbs_iterator;

        pthread_rwlock_wrlock(&(db_used->rwlock_dbs));
        if ((dbs_iterator = db_used->dbs.find(iterator->first)) == db_used->dbs.end()) {
            string fno = fileno_tmp;
            db = openExistDB(suffix, fno);
        } else {
            db = dbs_iterator->second;
        }
        pthread_rwlock_unlock(&(db_used->rwlock_dbs));

        if (db != NULL) {
            int ret = db_delete(db, key, pos);
            return ret;
        }
    }
    return 1;
}

DBD *DBAccess::get(int suffix, string &fileno, char *key, POS *pos)  {
    DBAccessType *db_used = dbs[suffix];
    map<string, string>::iterator iterator; 
    DB *db = NULL;
    DBD *dbd = NULL;

    //If key found, return it, otherwise go through all database to find it
    
    if (!fileno.empty() && pos != NULL) {
        dbd = this->get_by_pos(suffix, key, fileno, pos);
        if (dbd) {
            return dbd;
        }
    }

    for(iterator = db_used->databases_conf.begin(); iterator != db_used->databases_conf.end(); ++iterator) {
        string fileno_tmp = iterator->first;

        map<string, DB *>::iterator dbs_iterator;

        pthread_rwlock_wrlock(&(db_used->rwlock_dbs));
        if ((dbs_iterator = db_used->dbs.find(fileno_tmp)) == db_used->dbs.end()) {
            string fno = fileno_tmp;
            db = openExistDB(suffix, fno);
        } else {
            db = dbs_iterator->second;
        }
        pthread_rwlock_unlock(&(db_used->rwlock_dbs));

        if (db != NULL) {
            dbd = db_fetch(db, key, NULL);
            if (dbd != NULL) {
                fileno.assign(fileno_tmp);
                return dbd;
            }
        }
    }
    return NULL;
}

DBD *DBAccess::get_by_pos(int suffix, char *key, string &fileno, POS *pos) {

    DBAccessType *db_used = dbs[suffix];
    DB *db = NULL;
    DBD *dbd = NULL;

    pthread_rwlock_wrlock(&(db_used->rwlock_dbs));
    db = db_used->dbs[fileno];
    if (!fileno.empty() && db == NULL) {
        db = openExistDB(suffix, fileno);
        fileno.assign(fileno);
    }
    pthread_rwlock_unlock(&(db_used->rwlock_dbs));

    if (db != NULL) {
        dbd = db_fetch(db, key, pos);
    } else {
        mylog_info(m_pLogGlobalCtrl->infolog, "DB %s does not exists - %s:%s:%d",(char *)fileno.c_str(),INFO_LOG_SUFFIX);
    }

    return dbd;
}

off_t DBAccess::getDBSize(int fd) {
    struct stat file_stat;
    fstat(fd, &file_stat);
    off_t size = file_stat.st_size; 
    return size;
}

DBD *DBAccess::get_random(int suffix, string &fileno) {
    DBAccessType *db_used = dbs[suffix];
    map<string, string>::iterator iterator; 

    struct timeval now;
    gettimeofday(&now, NULL);
    srandom(now.tv_sec + now.tv_usec);
    long ran = random();

    //random datafile
    if (fileno.empty()) {
        size_t size = db_used->databases_conf.size();
        if (size > 0) {
            int count = ran % size;
            int i = -1;
            for(iterator = db_used->databases_conf.begin(); iterator != db_used->databases_conf.end() && i < count; i++) {
                fileno.assign(iterator->first);
            }
        }
    }


    map<string, DB *>::iterator dbs_iterator;
    DB *db = NULL;

    pthread_rwlock_wrlock(&(db_used->rwlock_dbs));
    if ((dbs_iterator = db_used->dbs.find(fileno)) == db_used->dbs.end()) {
        db = openExistDB(suffix, fileno);
    } else {
        db = dbs_iterator->second;
    }
    pthread_rwlock_unlock(&(db_used->rwlock_dbs));

    DBD *dbd = NULL;
    if (db != NULL) {

        //random chainoff
        off_t chainoff = ((ran % db->nhash) * PTR_SZ) + db->hashoff;
        while(chainoff == 0)  {
            ran = random();
            chainoff = ((ran % db->nhash) * PTR_SZ) + db->hashoff;
        }
        dbd = db_fetch_by_chainoff(db, chainoff); 
    }

    return dbd;
}

DBD* DBAccess::gets(int suffix, DBD **last_dbd, off_t *last_offset, string &fileno) {
    DBAccessType *db_used = dbs[suffix];
    map<string, string>::iterator iterator; 

    if (fileno.empty()) {
        if ((iterator = db_used->databases_conf.begin()) != db_used->databases_conf.end()) {
            fileno.assign(iterator->first);
        }
    }
    
    map<string, DB *>::iterator dbs_iterator;
    DB *db = NULL;

    pthread_rwlock_wrlock(&(db_used->rwlock_dbs));
    if ((dbs_iterator = db_used->dbs.find(fileno)) == db_used->dbs.end()) {
        db = openExistDB(suffix, fileno);
    } else {
        db = dbs_iterator->second;
    }
    pthread_rwlock_unlock(&(db_used->rwlock_dbs));

    if (db != NULL) {
        *last_dbd = db_nextrec(db, *last_dbd, NULL, last_offset);
        if (*last_dbd == NULL) {
            pthread_rwlock_rdlock(&(db_used->rwlock_dbs));
            dbs_iterator = db_used->dbs.find(fileno);
            if (++dbs_iterator != db_used->dbs.end()) {//next db
                *last_offset = 0;
                fileno.assign(dbs_iterator->first);
                pthread_rwlock_unlock(&(db_used->rwlock_dbs));
                return gets(suffix, last_dbd, last_offset, fileno);
            } else {
                pthread_rwlock_unlock(&(db_used->rwlock_dbs));
                return NULL;
            }
        }
    }

    return *last_dbd;
}


#ifndef KUROBOT_DBACCESS_H
#define KUROBOT_DBACCESS_H

#include <string>
#include <vector>
#include <pthread.h>
#include "db.h"
#include "DBAccessType.h"
#include "Format.h"
#include "util.h"

using namespace std;

/*
 * db format:
 *  size of file
 *  open date
 */
#define TOP_DATABASES 100
#define DB_CONF_FIELD_COUNT 1
#define MAX_DATABASES_PER_DIR 1000
#define MAX_DB_DIR 30

class DBAccess {
public:
    DBAccess();
    ~DBAccess();
    static DBAccess* const getInstance();
    void destroy(); 
    void destroydbs();
public:

    int store(int suffix, char *key, const void *data, size_t datlen, string &fileno, POS *); 
    int store(int suffix, char *key, const char *data, string &fileno, POS *pos); 
    int storeURLNodeTmp(char *key, char *data); 
    DBD* gets(int suffix, DBD **last_dbd, off_t *last_offset, string &fileno);
    //DBD *getHTML(char *key, string &fileno, POS *pos); 
    int  load(char *tablename); //must call setDir first
    void setDir(char *dir);  
    void loadUrlnodeTmpDB(); 
    void closeUrlnodeTmpDB(); 
    DB* getUrlnodeTmp();
    off_t getDBSize(int fd); 
    DBD *get(int suffix, string &fileno, char *key, POS *pos);
    DBD *get_by_pos(int suffix, char *key, string &fileno, POS *pos); 
    DBD *get_random(int suffix, string &fileno); 
    void unload(int suffix); 
    void remove(int suffix); 
    int erase(int suffix, char *key, string &fileno, POS *pos); 

    static pthread_mutex_t mutex_open;
    static pthread_mutex_t mutex_load;
private:
    vector<DBAccessType *> dbs; 
    map<string, int> dbsuffix; 
    static DBAccess *_instance;
    char *dir;
private:
    string getPath(string &filename, char *hashfilename = NULL);
    string getPath(char *filename, char *hashfilename = NULL);
    
    DB *openExistDB(int suffix, string &fn);
    DB *openNewDB(int suffix, string &fn); 
    int _load(char *tablename);
    void _open(int suffix, string &fileno, DB * db);

    LogGlobalCtrl       *m_pLogGlobalCtrl;
};

#endif

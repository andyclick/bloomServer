#ifndef KUROBOT_DBACCESSTYPE_H
#define KUROBOT_DBACCESSTYPE_H

#include <string>
#include <map>
#include <pthread.h>
#include "db.h"
#include "util.h"
#include "ic_types.h"

using namespace std;

class DBAccessType {
public:
    map<string, string> databases_conf; 
    int count_of_database;
    map<string, DB *> dbs;
    DB *db_conf;
    pthread_rwlock_t rwlock_dbs;
    pthread_mutex_t mutex_store;
    string filename;
    int curPos; //current pos in <vector>dbs which is in DBAccess class
public:
    DBAccessType(char *);
    ~DBAccessType();
    void destroy(); 
    LogGlobalCtrl       *m_pLogGlobalCtrl;    

};

#endif

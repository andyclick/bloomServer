#include "DBAccessType.h"

DBAccessType::DBAccessType(char *filename):count_of_database(0), db_conf(NULL) {
    pthread_mutex_init(&mutex_store, NULL);
    pthread_rwlock_init(&rwlock_dbs, NULL);
    this->filename = filename;
    curPos = -1;
}

DBAccessType::~DBAccessType() {
    destroy();
}

void DBAccessType::destroy() {
    map<string, DB *>::iterator iter; 

    if (db_conf != NULL) {
        db_close(db_conf);
        db_conf = NULL;
    }
    //close dbs
    for (iter = dbs.begin(); iter != dbs.end(); iter++) {
        if (iter->second != NULL) {
            db_close(iter->second);
            iter->second = NULL;
        }
    }
    count_of_database = 0;
}

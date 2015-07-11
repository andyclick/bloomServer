#ifndef _MODULE_TEST_H_
#define _MODULE_TEST_H_
#include "util.h"
#include "ic_types.h"
void testDB();
char * get_final_content(UrlNode *urlnode);
void get_decompress(char * content);
void getContentDBName1(UrlNode *urlnode, char *dbname);
void getRecordKeyName1(UrlNode *urlnode, char *recordname);
void getUrlDBName1(UrlNode *urlnode, char *dbname);
#endif

#ifndef CLENT_H
#define CLENT_H
#include <iostream>
#include <fstream>
#include <string.h>
#include <sstream>
#include "ThriftBroker/Client.h"
#include "ThriftBroker/gen-cpp/HqlService.h"
#include "ThriftBroker/ThriftHelper.h"
//#include "ThriftBroker/SerializedCellsReader.h"
//
#include "util.h"
#include "cmsinfo.h"
#include "ic_types.h"
#include "MyConnection.h"
#include "utilcpp.h"
#include "UrlAnalyseManager.h"
#include "LocalDbManager.h"
#include "Http.h"
#include "uri.h"
#include "Url.h"
#include "hlink.h"
#include "ic_types.h"
#include <zlib.h>
#include "FSBigFile.h"
#include "HtRegex.h"
#include "DBAccess.h"
#include "DBAccess.h"
#include "InfoCrawler.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "mycxxlog.h"
//

using namespace Hypertable;
using namespace Hypertable::ThriftGen;
using namespace std;
void run(Thrift::Client *client);
void test_rename_alter(Thrift::Client *client, std::ostream &out);
void test_guid(Thrift::Client *client, std::ostream &out);
void test_unique(Thrift::Client *client, std::ostream &out);
void test_hql(Thrift::Client *client, std::ostream &out);
void test_scan(Thrift::Client *client, std::ostream &out);
void test_scan_keysonly(Thrift::Client *client, std::ostream &out);
void test_set(Thrift::Client *client);
void test_schema(Thrift::Client *client, std::ostream &out);
void test_put(Thrift::Client *client);
void test_async(Thrift::Client *client, std::ostream &out);
void test_error(Thrift::Client *client, std::ostream &out);
int test();
void insert_data_to_hypertable(char * url,char * content);
void get_content_hypertable();
string format_timestamp(struct tm tm);
#endif

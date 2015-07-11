#include "DbMongodbManager.h"
#include "DbHbaseManager.h"
#include "mongo/client/dbclientinterface.h"
#include "mongo/client/connpool.h"

#define RED_B  "\033[31m"
#define RED_E  "\033[0m"
#define GREEN_B "\033[32m"
#define GREEN_E "\033[0m"
#define INSERT_NUMBER   300
#define THREAD_NUMBER   10

int wrong_num = 0;

using namespace std;

#define EXPECT_EQUAL(expect, result, ...) \
  { \
    if (expect == result) { \
      cout<<"=="<<GREEN_B<<"RIGHT"<<GREEN_E<<"==in "<<__FUNCTION__<<" at "<<setw(4)<<__LINE__<<", when "<<__VA_ARGS__<<endl; \
    } else { \
      wrong_num++; \
      cout<<"=="<<RED_B<<"WRONG"<<RED_E<<"==in "<<__FUNCTION__<<" at "<<setw(4)<<__LINE__<<", when "<<__VA_ARGS__<<endl<<"expect is "<<expect<<endl<<"result is "<<result<<endl; \
    } \
  }

  using namespace octopus::b_server;
  using namespace mongo;

  void* mongo_thread(void *param);
  void* hbase_thread(void *param);
  void test_mongo_pool(void);
  void test_hbase_pool(void);
  void case_duplicate_mongo(void);
  void case_batch_insert_url_mongo(void);
  void case_insert_get_fetchedurl_mongo(void);
  void case_get_update_position_mongo(void);
  void case_insert_get_entrance_mongo(void);
  void case_insert_get_priorityurl_mongo(void);
  void case_insert_get_normalurl_mongo(void);

  void test_mongo_(void) {
    printf("-----BEGIN: %s\n", __FUNCTION__);
    CoreManager *core_ = NULL;
    DbMongodbManager *mongodb = new DbMongodbManager(core_);
    ScopedDbConnection *mongopool = ScopedDbConnection::getScopedDbConnection("192.168.19.11:10001", 0);
    string table_name_temp = "admin.t_source_state_tbl";
    bool ret = false;
    ret = mongopool->conn().dropCollection(table_name_temp);
    EXPECT_EQUAL(true, ret, "drop t_source_state_tbl table");
    string table_name = "admin.test_table";
    ret = mongopool->conn().dropCollection(table_name);
    EXPECT_EQUAL(true, ret, "drop table");
    ret = mongopool->conn().createCollection(table_name);
    EXPECT_EQUAL(true, ret, "create table");
    ret = mongopool->conn().ensureIndex(table_name, mongo::fromjson("{Uid:1}"), false, "uid_index");
    EXPECT_EQUAL(true, ret, "create uid index");
    ret = mongopool->conn().ensureIndex(table_name, mongo::fromjson("{Tid:1}"), false, "tid_index");
    EXPECT_EQUAL(true, ret, "create tid index");
    mongodb->ConnectMongodb();
      try {
        mongopool->conn().insert("admin.test_table", BSON("Uid" << "4" << "Tid" << "44"));
        mongopool->conn().insert("admin.test_table", BSON("Uid" << "5" << "Tid" << "55"));
        mongopool->conn().insert("admin.test_table", BSON("Uid" << "6" << "Tid" << "66"));
        string errmsg = mongopool->conn().getLastError();
        if (errmsg.empty()) {
          printf("*******Insert OK*******\n");
        } else {
          printf("***Error: insert*************%s**\n", errmsg.c_str());
        }
        string return_column = "Uid";
        int get_number = 0;
        std::vector<string> vector_row;

        mongo::BSONArrayBuilder display_ids;
        display_ids.append("5");
        display_ids.append("6");
        display_ids.append("7");

        mongo::BSONObjBuilder in_condition;
        in_condition.append("$in", display_ids.arr());
                
        mongo::BSONObjBuilder update_condition;
        //update_condition.append("Uid", "4");
        update_condition.append("Uid", in_condition.obj());
        mongo::Query up_query(update_condition.obj());
        mongo::BSONObjBuilder update;
        update.append("Tid", "11");
        mongo::BSONObjBuilder record;
        record.append("$set", update.obj());
        mongopool->conn().update(table_name, up_query, record.obj(), false, true);
        mongo::BSONArrayBuilder display_ids2;
        display_ids2.append("4");
        display_ids2.append("6");
        display_ids2.append("8");

        mongo::BSONObjBuilder in_condition2;
        in_condition2.append("$in", display_ids2.arr());
                
        mongo::BSONObjBuilder message_condition2;
        message_condition2.append("Uid", in_condition2.obj());
        
        ret = mongodb->GetFromCollection("test_table", &message_condition2, return_column, get_number, vector_row);
        for(size_t i = 0; i < vector_row.size(); i++) {
          printf(">>>>>%lu>>>>>>>>>%s\n", i, vector_row[i].c_str());
        }
        //////////////////////////////////////////////////////
        mongo::BSONObjBuilder query;
        query.append("Uid", "5");
        std::vector<string> vector_row1;
        vector_row1.push_back("Tid");
        vector_row1.push_back("Uid");
        std::vector< std::vector<string> > vector_result;
        vector_result.push_back(vector_row1);
        ret = mongodb->GetFromCollection("test_table", &query, get_number, vector_result);
        for (size_t i = 1; i < vector_result.size(); i++) {
          printf("**********%s=%s*********\n", vector_result[i][0].c_str(), vector_result[i][1].c_str());
        }
      } catch (mongo::DBException &e) {
        printf("===Error: insert============\n");
      } catch (...) {
        printf("---Error: insert------------\n");
      }
  }


  int main(int argc, char **argv) {
//    test_mongo_pool();
//    test_mongo_();
//    test_hbase_pool();
//    case_duplicate_mongo();
    case_batch_insert_url_mongo();
//    case_insert_get_fetchedurl_mongo();
//    case_get_update_position_mongo();
//    case_insert_get_entrance_mongo();
//    case_insert_get_priorityurl_mongo();
//    case_insert_get_normalurl_mongo();
    if (0 < wrong_num) {
        printf("%sThere are %d error!%s\n", RED_B, wrong_num, RED_E);
    }
    return 0;
  }

  void getNow1(int *second, int *micro) {
    struct timeval tvtime;
    struct timezone tz;
    gettimeofday(&tvtime, &tz);
    *second = tvtime.tv_sec;
    *micro = tvtime.tv_usec;
  }

  void* mongo_thread(void *param) {
    ScopedDbConnection *mongopool = ScopedDbConnection::getScopedDbConnection("192.168.19.11:10001", 0);
    DBClientBase *client = mongopool->get();
    int timer_second_begin; 
    int timer_micro_begin; 
    getNow1(&timer_second_begin, &timer_micro_begin);
    int mycount = 0;
    for (int i = 0; i < INSERT_NUMBER; i++) {
      try {
        client->insert("admin.test_table", BSON("Uid" << "asdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdf" << "Tid" << "asdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdf"));
        string errmsg = client->getLastError();
        if (errmsg.empty()) {
          mycount++;
        } else {
          printf("***Error: insert***********%d**%s**\n", i, errmsg.c_str());
        }
      } catch (mongo::DBException &e) {
        printf("===Error: insert==========%d==\n", i);
      } catch (...) {
        printf("---Error: insert----------%d--\n", i);
      }
    } 
    int timer_second_end; 
    int timer_micro_end; 
    getNow1(&timer_second_end, &timer_micro_end);
    printf("!!!!!cost %d, i=%d\n", (timer_second_end - timer_second_begin) * 1000000 + timer_micro_end - timer_micro_begin, mycount);
    mongopool->done();
    return NULL;
  }

  void test_mongo_pool(void) {
    printf("-----BEGIN: %s\n", __FUNCTION__);
    ScopedDbConnection *mongopool = ScopedDbConnection::getScopedDbConnection("192.168.19.11:10001", 0);
    string table_name = "admin.test_table";
    bool ret = false;
    ret = mongopool->conn().dropCollection(table_name);
    EXPECT_EQUAL(true, ret, "drop table");
    ret = mongopool->conn().createCollection(table_name);
    EXPECT_EQUAL(true, ret, "create table");
    ret = mongopool->conn().ensureIndex(table_name, mongo::fromjson("{Uid:1}"), false, "uid_index");
    EXPECT_EQUAL(true, ret, "create uid index");
    ret = mongopool->conn().ensureIndex(table_name, mongo::fromjson("{Tid:1}"), false, "tid_index");
    EXPECT_EQUAL(true, ret, "create tid index");

    pthread_t *tid = (pthread_t*)malloc(THREAD_NUMBER * sizeof(pthread_t));
    if (tid == NULL) {
      printf("malloc error\n");
    }
    void *param = NULL;
    for (int i = 0; i < THREAD_NUMBER; i++) {
      if(pthread_create(&tid[i], NULL, mongo_thread, param)) {
        printf("create threads error\n");
      }
    }
    for (int j = 0; j < THREAD_NUMBER; j++) {
      pthread_join(tid[j], NULL);
    }
    free(tid);

    try {
      int count = 0;
      int timer_second_begin; 
      int timer_micro_begin; 
      getNow1(&timer_second_begin, &timer_micro_begin);
      mongo::BSONObjBuilder query;
      query.append("Uid", "asdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdf");
      query.append("Tid", "asdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdf");
      count = mongopool->conn().count("admin.test_table", BSON("Uid" << "asdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdf" << "Tid" << "asdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdf"));
      int timer_second_end; 
      int timer_micro_end; 
      getNow1(&timer_second_end, &timer_micro_end);
      string errmsg = mongopool->conn().getLastError();
      if (errmsg.empty()) {
        printf("!!!count cost %d, count=%d\n", (timer_second_end - timer_second_begin) * 1000000 + timer_micro_end - timer_micro_begin, count);
      } else {
        printf("***Error: count*************%s***\n", errmsg.c_str());
      }
      count = 0;
      mongo::Query condision(query.obj());
      mongo::BSONObj columns = BSON("Uid" << 1);
      auto_ptr<mongo::DBClientCursor> cursor = mongopool->conn().query("admin.test_table", condision.sort("Uid"), 0, 0, &columns);
      if (!cursor->more()) {
        printf("***Error: get****************\n");
      }
      while (cursor->more()) {
        mongo::BSONObj p = cursor->next();
        count++;
      }
      getNow1(&timer_second_begin, &timer_micro_begin);
      printf("!!!!!get cost %d, count=%d\n", (timer_second_begin - timer_second_end) * 1000000 + timer_micro_begin - timer_micro_end, count);
    } catch (mongo::DBException &e) {
      printf("===Error: count=========\n");
    } catch (...) {
      printf("---Error: count---------\n");
    }
    mongopool->done();
    printf("-----END: %s\n", __FUNCTION__);
    return;
  }

  void* hbase_thread(void *param) {
    return NULL;
  }

  void test_hbase_pool(void) {
    printf("-----BEGIN: %s\n", __FUNCTION__);
    CoreManager *core_ = NULL;
    DbHbaseManager *hbase = new DbHbaseManager(core_);
    int taskid = 2;
    bool ret = false;
    octopus::common::UrlNode urlnode;
    urlnode.task_id = taskid;
    urlnode.source_id = 2;
    urlnode.batch_id = 3;
    urlnode.id = 1122;
    urlnode.url = "123456789";
    urlnode.father_url = "www.baidufff.com";
    urlnode.dup_mode = URL_DUPLICATE_MODE_BY_SOURCE | URL_DUPLICATE_MODE_BY_BATCH;
    string content = "this is hbase test";
    ret = hbase->initialize_table();
    EXPECT_EQUAL(true, ret, "connect hbase");
    ret = hbase->create_task_tbl(taskid);
    EXPECT_EQUAL(true, ret, "create hbase table");
    ret = hbase->StoreContentToHbase(&urlnode, content);
    EXPECT_EQUAL(true, ret, "insert into hbase table");
    char row_key[32] = {0};
    sprintf(row_key, "%s:%ld", urlnode.url.c_str(), urlnode.id);
    char *result = NULL;
    ret = hbase->GetContentFromTable(urlnode.task_id, row_key, &result);
    EXPECT_EQUAL(true, ret, "get from hbase table");
    EXPECT_EQUAL(0, strcmp(content.c_str(), result), "check content");
    ret = hbase->DeleteContentFromHbase(urlnode.task_id, row_key);
    EXPECT_EQUAL(true, ret, "delete from hbase table");
    char *result2 = NULL;
    ret = hbase->GetContentFromTable(urlnode.task_id, row_key, &result2);
    EXPECT_EQUAL(false, ret, "get from hbase table again");
    EXPECT_EQUAL(NULL, result2, "check content again");
    printf("-----END: %s\n", __FUNCTION__);
    return;
  }

  void case_duplicate_mongo(void) {
    printf("-----BEGIN: %s\n", __FUNCTION__);
    CoreManager *core_ = NULL;
    DbMongodbManager *mongodb = new DbMongodbManager(core_);
    mongodb->ConnectMongodb();
    octopus::common::UrlNode urlnode;
    urlnode.task_id = 0;
    urlnode.source_id = 2;
    urlnode.batch_id = 3;
    urlnode.id = 1122;
    urlnode.url = "www.baidu.com";
    urlnode.father_url = "www.baidufff.com";
    urlnode.dup_mode = URL_DUPLICATE_MODE_BY_SOURCE | URL_DUPLICATE_MODE_BY_BATCH;
    std::vector<int> batch_ids;
    batch_ids.push_back(urlnode.batch_id);
    bool ret = mongodb->IsUrlRepeated(&urlnode);
    if (ret) {
      ret = mongodb->DeleteUrlFromDuplicateTbl(urlnode.task_id, batch_ids);
      EXPECT_EQUAL(true, ret, "delete url from duplicate");
    }
    ret = mongodb->IsUrlRepeated(&urlnode);
    EXPECT_EQUAL(false, ret, "check url repeated");
    ret = mongodb->InsertIntoDuplicate(&urlnode);
    EXPECT_EQUAL(true, ret, "InsertIntoDuplicate");
    ret = mongodb->IsUrlRepeated(&urlnode);
    EXPECT_EQUAL(true, ret, "check url repeated");
    ret = mongodb->DeleteUrlFromDuplicateTbl(urlnode.task_id, batch_ids);
    EXPECT_EQUAL(true, ret, "delete url from duplicate");
    ret = mongodb->IsUrlRepeated(&urlnode);
    EXPECT_EQUAL(false, ret, "check url repeated");
    delete mongodb;
    mongodb = NULL;
    printf("-----END: %s\n", __FUNCTION__);
    return;
  }

  void case_insert_get_fetchedurl_mongo(void) {
    printf("-----BEGIN: %s\n", __FUNCTION__);
    CoreManager *core_ = NULL;
    DbMongodbManager *mongodb = new DbMongodbManager(core_);
    mongodb->ConnectMongodb();
    octopus::common::UrlNode urlnode;
    urlnode.task_id = 0;
    urlnode.source_id = 2;
    urlnode.batch_id = 3;
    urlnode.id = 112233;
    urlnode.url = "www.baidu.com";
    urlnode.father_url = "www.baidufff.com";
    urlnode.dup_mode = URL_DUPLICATE_MODE_BY_SOURCE | URL_DUPLICATE_MODE_BY_BATCH;
    urlnode.page = 1;
    urlnode.total_page = 1;
    std::vector<string> result_row;
    bool ret = mongodb->DeleteUrlFromFetchedTbl(urlnode.task_id, 0, result_row);
    EXPECT_EQUAL(true, ret, "delete url from fetched table first");
    string startrow = "";
    std::vector<string> result;
    ret = mongodb->GetFetchedUrlId(urlnode.task_id, urlnode.source_id, urlnode.batch_id, startrow, 0, 0, result);
    EXPECT_EQUAL(false, ret, "get fetched url id");
    EXPECT_EQUAL(0, result.size(), "get fetched url id size");
    ret = mongodb->InsertIntoContentPage(&urlnode);
    EXPECT_EQUAL(true, ret, "insert url to content page table");
    ret = mongodb->GetFetchedUrlId(urlnode.task_id, urlnode.source_id, urlnode.batch_id, startrow, 0, 0, result);
    EXPECT_EQUAL(true, ret, "get fetched url id");
    EXPECT_EQUAL(1, result.size(), "get fetched url id size");
    char *pos = strrchr((char *)result[0].c_str(), ':');
    EXPECT_EQUAL(112233, atoi(pos + 1), "check url id");
    ret = mongodb->UpdateFetchedUrl(result[0], urlnode.source_id, urlnode.batch_id, 1);
    EXPECT_EQUAL(true, ret, "update fetched url id state");
    result.clear();
    ret = mongodb->GetFetchedUrlId(urlnode.task_id, urlnode.source_id, urlnode.batch_id, startrow, 0, 0, result);
    EXPECT_EQUAL(false, ret, "get fetched url id state=0");
    result.clear();
    ret = mongodb->GetFetchedUrlId(urlnode.task_id, urlnode.source_id, urlnode.batch_id, startrow, 1, 0, result);
    EXPECT_EQUAL(true, ret, "get fetched url id state=1");
    EXPECT_EQUAL(1, result.size(), "get fetched url id size state=1");
    std::vector<string> result_id;
    ret = mongodb->GetContentPageUrl(urlnode.task_id, urlnode.id, 0, result_id);
    EXPECT_EQUAL(true, ret, "get content page url");
    for (uint32_t i = 0; i < result_id.size(); i++) {
      char row[1024] = {0};
      sprintf(row, "%s:%lu", urlnode.url.c_str(), urlnode.id);
      EXPECT_EQUAL(0, strcmp(row, result_id[i].c_str()), "check content page row");
    }
    result_id.clear();
    ret = mongodb->DeleteUrlFromFetchedTbl(urlnode.task_id, 0, result_row);
    EXPECT_EQUAL(true, ret, "delete url from fetched table again");
    std::vector<string> result_id2;
    ret = mongodb->GetContentPageUrl(urlnode.task_id, urlnode.id, 0, result_id2);
    EXPECT_EQUAL(false, ret, "get content page url again");
    delete mongodb;
    mongodb = NULL;
    printf("-----END: %s\n", __FUNCTION__);
    return;
  }

  void case_batch_insert_url_mongo(void) {
    printf("-----BEGIN: %s\n", __FUNCTION__);
    CoreManager *core_ = NULL;
    DbMongodbManager *mongodb = new DbMongodbManager(core_);
    mongodb->ConnectMongodb();
    std::map<string, octopus::common::UrlNode *> urls_map;
    for (int i = 0; i < INSERT_NUMBER; i++) {
      octopus::common::UrlNode *urlnode = new octopus::common::UrlNode();
      urlnode->task_id = 0;
      urlnode->source_id = 2;
      urlnode->batch_id = 3;
      urlnode->id = 1000 + i;
      char temp[128] = {0};
      sprintf(temp, "www.baidu.com%lu", urlnode->id);
      urlnode->url = temp;
      urlnode->father_url = "www.baidufff.com";
      urlnode->dup_mode = URL_DUPLICATE_MODE_BY_SOURCE | URL_DUPLICATE_MODE_BY_BATCH;
      urlnode->priority = 0;
      urls_map.insert(pair<string, octopus::common::UrlNode *>(urlnode->url, urlnode));
    }
    std::vector<string> vector_row;
    bool ret = mongodb->GetNormalToFetchKey(0, 2, 3, 0, 0, vector_row);
    EXPECT_EQUAL(false, ret, "get row from normal tbl");
    EXPECT_EQUAL(0, vector_row.size(), "check row number");
    UrlQueueNode urlnode_queue;
    urlnode_queue.task_id = 0;
    urlnode_queue.source_id = 2;
    urlnode_queue.batch_id = 3;
    urlnode_queue.dup_mode = URL_DUPLICATE_MODE_BY_SOURCE | URL_DUPLICATE_MODE_BY_BATCH;
    urlnode_queue.priority_type = 0;
    urlnode_queue.urls = &urls_map;
    ret = mongodb->InsertUrlsIntoMongo(&urlnode_queue);
    EXPECT_EQUAL(true, ret, "insert urls into mongo");
    vector_row.clear();
    ret = mongodb->GetNormalToFetchKey(0, 2, 3, 0, 0, vector_row);
    EXPECT_EQUAL(true, ret, "get row from normal tbl");
    EXPECT_EQUAL(300, vector_row.size(), "check row number");
    vector_row.clear();
    std::map<int32_t, int32_t> source_ids;
    source_ids.insert(pair<int32_t, int32_t>(urlnode_queue.source_id, urlnode_queue.source_id));
    std::vector<octopus::b_server::UrlnodeToFetch_t> urlnodeInfos;
    ret = mongodb->GetNormalUrlToFetch(0, source_ids, 0, urlnodeInfos);
    EXPECT_EQUAL(true, ret, "get url from normal tbl");
    EXPECT_EQUAL(300, urlnodeInfos.size(), "check url number");

    vector_row.clear();
    ret = mongodb->GetNormalToFetchKey(0, 2, 3, 0, 0, vector_row);
    EXPECT_EQUAL(false, ret, "get row from normal tbl again");
    EXPECT_EQUAL(0, vector_row.size(), "check row number again");
    
    std::map<string, octopus::common::UrlNode *> urls_map2;
    for (int i = 0; i < INSERT_NUMBER; i++) {
      octopus::common::UrlNode *urlnode = new octopus::common::UrlNode();
      urlnode->task_id = 0;
      urlnode->source_id = 2;
      urlnode->batch_id = 3;
      urlnode->id = 1000 + i + INSERT_NUMBER / 2;
      char temp[128] = {0};
      sprintf(temp, "www.baidu.com%lu", urlnode->id);
      urlnode->url = temp;
      urlnode->father_url = "www.baidufff.com";
      urlnode->dup_mode = 0;
      urlnode->priority = 0;
      urls_map2.insert(pair<string, octopus::common::UrlNode *>(urlnode->url, urlnode));
    }
    urlnode_queue.task_id = 0;
    urlnode_queue.source_id = 2;
    urlnode_queue.batch_id = 3;
    urlnode_queue.dup_mode = URL_DUPLICATE_MODE_BY_SOURCE | URL_DUPLICATE_MODE_BY_BATCH;
    urlnode_queue.priority_type = 0;
    urlnode_queue.urls = &urls_map2;
    ret = mongodb->InsertUrlsIntoMongo(&urlnode_queue);
    EXPECT_EQUAL(true, ret, "insert urls into mongo again");

    vector_row.clear();
    urlnodeInfos.clear();
    ret = mongodb->GetNormalUrlToFetch(0, source_ids, 1, urlnodeInfos);
    EXPECT_EQUAL(true, ret, "get url from normal tbl");
    vector_row.clear();
    urlnodeInfos.clear();
    ret = mongodb->GetNormalUrlToFetch(0, source_ids, 0, urlnodeInfos);
    EXPECT_EQUAL(true, ret, "get url from normal tbl");
    EXPECT_EQUAL(INSERT_NUMBER / 2, vector_row.size(), "check url number again");
//    ret = mongodb->UpdateToFetchUrlStateToSended(2, 0, vector_row);
    EXPECT_EQUAL(true, ret, "update url state to sended");
    vector_row.clear();
    urlnodeInfos.clear();
    ret = mongodb->GetNormalUrlToFetch(0, source_ids, 2, urlnodeInfos);
    EXPECT_EQUAL(true, ret, "get url from normal tbl");
    EXPECT_EQUAL(INSERT_NUMBER / 2, vector_row.size(), "check url number again");

    vector_row.clear();
    ret = mongodb->GetNormalToFetchKey(0, 2, 3, 0, 0, vector_row);
    EXPECT_EQUAL(false, ret, "get row from normal tbl again");
    EXPECT_EQUAL(0, vector_row.size(), "check row number again");
    ret = mongodb->DeleteNormalUrlFromFetchTbl(0, 0);
    EXPECT_EQUAL(true, ret, "delete url from normal tbl");
    delete mongodb;
    mongodb = NULL;
    ScopedDbConnection *mongopool = ScopedDbConnection::getScopedDbConnection("192.168.19.11:10001", 0);
    string table_name = "admin.t_url_to_fetch_tbl";
    ret = mongopool->conn().dropCollection(table_name);
    EXPECT_EQUAL(true, ret, "drop table");
    table_name = "admin.t_duplicate_url_tbl";
    ret = mongopool->conn().dropCollection(table_name);
    EXPECT_EQUAL(true, ret, "drop table");
    printf("-----END: %s\n", __FUNCTION__);
    return;
  }

  void case_get_update_position_mongo(void) {
    printf("-----BEGIN: %s\n", __FUNCTION__);
    CoreManager *core_ = NULL;
    DbMongodbManager *mongodb = new DbMongodbManager(core_);
    mongodb->ConnectMongodb();
    std::vector<string> result_row;
    bool ret = mongodb->DeleteUrlFromFetchedTbl(0, 0, result_row);
    EXPECT_EQUAL(true, ret, "delete url from fetched table first");
    octopus::common::UrlNode urlnode;
    urlnode.task_id = 0;
    urlnode.source_id = 2;
    urlnode.batch_id = 3;
    urlnode.url = "www.baidu.com";
    urlnode.father_url = "www.baidufff.com";
    urlnode.dup_mode = URL_DUPLICATE_MODE_BY_SOURCE | URL_DUPLICATE_MODE_BY_BATCH;
    urlnode.page = 1;
    urlnode.total_page = 1;
    string startrow = "";
    std::vector<string> result;
    for (int i = 0; i < 3; i++) {
      result.clear();
      urlnode.id = i + 1000;
      ret = mongodb->InsertIntoContentPage(&urlnode);
      EXPECT_EQUAL(true, ret, "insert url to content page table");
      if (1 == i) {
        ret = mongodb->GetFetchedUrlId(urlnode.task_id, urlnode.source_id, urlnode.batch_id, startrow, 0, 0, result);
        EXPECT_EQUAL(true, ret, "get fetched url id");
        EXPECT_EQUAL(2, result.size(), "get fetched url id size");
        char *pos = strrchr((char *)result[1].c_str(), ':');
        EXPECT_EQUAL(1001, atoi(pos + 1), "check url id");
        startrow = result[1];
      }
    }
    ret = mongodb->InsertIntoFetchedPosition(urlnode.task_id, urlnode.source_id, urlnode.batch_id, startrow);
    EXPECT_EQUAL(true, ret, "insert fetched position");
    result.clear();
    ret = mongodb->GetFetchedPosition(urlnode.task_id, urlnode.source_id, urlnode.batch_id, 0, 0, result);
    EXPECT_EQUAL(true, ret, "get fetched position state=0");
    EXPECT_EQUAL(1, result.size(), "get fetched position size");
    char *pos = strrchr((char *)result[0].c_str(), ':');
    EXPECT_EQUAL(1001, atoi(pos + 1), "check url id");
    startrow = result[0];
    result.clear();
    ret = mongodb->GetFetchedUrlId(urlnode.task_id, urlnode.source_id, urlnode.batch_id, startrow, 0, 0, result);
    EXPECT_EQUAL(true, ret, "get fetched url id state=0");
    EXPECT_EQUAL(1, result.size(), "get fetched url id size state=0");
    pos = strrchr((char *)result[0].c_str(), ':');
    EXPECT_EQUAL(1002, atoi(pos + 1), "check url id");
    result.clear();
    ret = mongodb->UpdateFetchedPosition(startrow, urlnode.source_id, urlnode.batch_id, 1);
    EXPECT_EQUAL(true, ret, "update fetched position state=1");
    result.clear();
    ret = mongodb->GetFetchedPosition(urlnode.task_id, urlnode.source_id, urlnode.batch_id, 0, 0, result);
    EXPECT_EQUAL(false, ret, "get fetched position state=0 again");
    EXPECT_EQUAL(0, result.size(), "get fetched position size again");
    delete mongodb;
    mongodb = NULL;
    printf("-----END: %s\n", __FUNCTION__);
    return;
  }

  void case_insert_get_entrance_mongo(void) {
    printf("-----BEGIN: %s\n", __FUNCTION__);
    CoreManager *core_ = NULL;
    DbMongodbManager *mongodb = new DbMongodbManager(core_);
    mongodb->ConnectMongodb();
    octopus::common::UrlNode urlnode;
    urlnode.task_id = 0;
    urlnode.source_id = 2;
    urlnode.batch_id = 3;
    urlnode.id = 1000;
    urlnode.url = "www.baidu.com";
    urlnode.father_url = "www.baidufff.com";
    urlnode.dup_mode = URL_DUPLICATE_MODE_BY_SOURCE | URL_DUPLICATE_MODE_BY_BATCH;
    urlnode.page = 1;
    urlnode.total_page = 1;
    string table_name = T_SITE_ENTRANCE_URL_TBL;
    mongo::BSONObjBuilder insert;
    struct timeval tvtime;
    struct timezone tz;
    gettimeofday(&tvtime, &tz);
    char row_key[64] = {0};
    sprintf(row_key, "%ld%06ld:%d", tvtime.tv_sec, tvtime.tv_usec, urlnode.source_id);
    insert.append("Sid", row_key);
    insert.append("Url", urlnode.url.c_str());
    insert.append("Bid", 1);
    insert.append("Tid", urlnode.task_id);
    bool ret = mongodb->InsertCollection(table_name, &insert);
    EXPECT_EQUAL(true, ret, "insert entrance url");
    std::vector<string> result_row;
    ret = mongodb->GetSiteEntranceRow(urlnode.task_id, 0, 0, result_row);
    EXPECT_EQUAL(false, ret, "get entrance row key with batchid=0");
    result_row.clear();
    ret = mongodb->GetSiteEntranceRow(urlnode.task_id, urlnode.batch_id, 0, result_row);
    EXPECT_EQUAL(true, ret, "get entrance row key with batchid=3");
    char *pos = strrchr((char *)result_row[0].c_str(), ':');
    EXPECT_EQUAL(2, atoi(pos + 1), "check url id");
    string row = result_row[0];
    result_row.clear();
    ret = mongodb->GetSiteEntranceUrl(urlnode.task_id, urlnode.batch_id, 0, result_row);
    EXPECT_EQUAL(true, ret, "get entrance urls and update batchid");
    EXPECT_EQUAL(0, strcmp(urlnode.url.c_str(), result_row[0].c_str()), "check entrance url");
    result_row.clear();
    ret = mongodb->GetSiteEntranceRow(urlnode.task_id, urlnode.batch_id, 0, result_row);
    EXPECT_EQUAL(false, ret, "get entrance row key with batchid=0");
    EXPECT_EQUAL(0, result_row.size(), "get entrance row key with batchid=0");
    mongo::BSONObjBuilder delete_url;
    delete_url.append("Sid", row_key);
    ret = mongodb->DeleteFromCollection(T_SITE_ENTRANCE_URL_TBL, &delete_url);
    delete mongodb;
    mongodb = NULL;
    printf("-----END: %s\n", __FUNCTION__);
    return;
  }

  void case_insert_get_priorityurl_mongo(void) {
    printf("-----BEGIN: %s\n", __FUNCTION__);
    CoreManager *core_ = NULL;
    DbMongodbManager *mongodb = new DbMongodbManager(core_);
    mongodb->ConnectMongodb();
    octopus::common::UrlNode urlnode;
    urlnode.task_id = 0;
    urlnode.source_id = 2;
    urlnode.batch_id = 3;
    urlnode.id = 1011;
    urlnode.priority = 3;
    urlnode.url = "www.baidu.com";
    urlnode.father_url = "www.baidufff.com";
    urlnode.dup_mode = URL_DUPLICATE_MODE_BY_SOURCE | URL_DUPLICATE_MODE_BY_BATCH;
    urlnode.page = 1;
    urlnode.total_page = 1;
    bool ret = mongodb->InsertIntoPriorityUrlToFetch(urlnode.task_id, urlnode.source_id, urlnode.batch_id, &urlnode, urlnode.priority);
    EXPECT_EQUAL(true, ret, "insert url to fetch");
    std::vector<string> result_row;
    ret = mongodb->GetPriorityToFetchKey(urlnode.task_id, urlnode.source_id, urlnode.batch_id, 1, 0, result_row);
    EXPECT_EQUAL(false, ret, "get url to fetch row key with state=1");
    result_row.clear();
    ret = mongodb->GetPriorityToFetchKey(urlnode.task_id, urlnode.source_id, urlnode.batch_id, 0, 0, result_row);
    EXPECT_EQUAL(true, ret, "get url to fetch row key with state=0");
    string row = result_row[0];
    result_row.clear();
    std::map<int32_t, int32_t> source_ids;
    source_ids.insert(pair<int32_t, int32_t>(urlnode.source_id, urlnode.source_id));
    std::vector<UrlnodeToFetch_t> urlnodeInfos;
    ret = mongodb->GetPriorityUrlToFetch(0, source_ids, 0, urlnodeInfos);
    EXPECT_EQUAL(true, ret, "get urls and update state");
    EXPECT_EQUAL(0, strcmp(urlnode.url.c_str(), result_row[0].c_str()), "check to fetch url");
    result_row.clear();
    ret = mongodb->GetPriorityToFetchKey(urlnode.task_id, urlnode.source_id, urlnode.batch_id, 0, 0, result_row);
    EXPECT_EQUAL(false, ret, "get url to fetch row key with state=0 again");
    EXPECT_EQUAL(0, result_row.size(), "get url to fetch row key with state=0 again");
    delete mongodb;
    mongodb = NULL;
    printf("-----END: %s\n", __FUNCTION__);
    return;
  }

  void case_insert_get_normalurl_mongo(void) {
    printf("-----BEGIN: %s\n", __FUNCTION__);
    CoreManager *core_ = NULL;
    DbMongodbManager *mongodb = new DbMongodbManager(core_);
    mongodb->ConnectMongodb();
    octopus::common::UrlNode urlnode;
    urlnode.task_id = 0;
    urlnode.source_id = 34;
    urlnode.batch_id = 31;
    urlnode.id = 1011;
    urlnode.priority = 0;
    urlnode.url = "www.baidu.com";
    urlnode.father_url = "www.baidufff.com";
    urlnode.dup_mode = URL_DUPLICATE_MODE_BY_SOURCE | URL_DUPLICATE_MODE_BY_BATCH;
    urlnode.page = 1;
    urlnode.total_page = 1;
    bool ret = mongodb->InsertIntoNormalUrlToFetch(urlnode.task_id, urlnode.source_id, urlnode.batch_id, &urlnode);
    EXPECT_EQUAL(true, ret, "insert url to fetch");
    std::vector<string> result_row;
    ret = mongodb->GetNormalToFetchKey(urlnode.task_id, urlnode.source_id, urlnode.batch_id, 1, 0, result_row);
    EXPECT_EQUAL(false, ret, "get url to fetch row key with state=1");
    result_row.clear();
    ret = mongodb->GetNormalToFetchKey(urlnode.task_id, urlnode.source_id, urlnode.batch_id, 0, 0, result_row);
    EXPECT_EQUAL(true, ret, "get url to fetch row key with state=0");
    string row = result_row[0];
    result_row.clear();
    std::map<int32_t, int32_t> source_ids;
    source_ids.insert(pair<int32_t, int32_t>(urlnode.source_id, urlnode.source_id));
    source_ids.insert(pair<int32_t, int32_t>(4, 4));
    std::vector<UrlnodeToFetch_t> urlnodeInfos;
    ret = mongodb->GetNormalUrlToFetch(0, source_ids, 0, urlnodeInfos);
    EXPECT_EQUAL(true, ret, "get urls and update state");
    EXPECT_EQUAL(0, strcmp(urlnode.url.c_str(), result_row[0].c_str()), "check to fetch url");
    result_row.clear();
    ret = mongodb->GetNormalToFetchKey(urlnode.task_id, urlnode.source_id, urlnode.batch_id, 0, 0, result_row);
    EXPECT_EQUAL(false, ret, "get url to fetch row key with state=0 again");
    EXPECT_EQUAL(0, result_row.size(), "get url to fetch row key with state=0 again");
    result_row.clear();
    result_row.push_back(urlnode.url);
    //ret = mongodb->UpdateToFetchUrlStateToSended(urlnode.source_id, 0, result_row);
    EXPECT_EQUAL(true, ret, "update state");
    result_row.clear();
    result_row.push_back(urlnode.url);
//    ret = mongodb->UpdateToFetchUrlStateToSended(urlnode.source_id, 1, result_row);
    EXPECT_EQUAL(true, ret, "update state");
    result_row.clear();
    ret = mongodb->GetNormalToFetchKey(urlnode.task_id, urlnode.source_id, urlnode.batch_id, 2, 0, result_row);
    EXPECT_EQUAL(true, ret, "get url to fetch row key with state=2 again");

    string temp_table_name = T_URL_TO_FETCH_TBL;
    string timestr = mongodb->get_time_str(60 * 0);
    mongo::BSONObjBuilder query;
    query.append("Key", result_row[0].c_str());
    query.append("State", MONGO_URL_STATE_SENDED);
    mongo::BSONObj normal_ts = BSON("ts" << mongo::LT << timestr.c_str());
    query.appendElements(normal_ts);
    string temp = "Url";
    result_row.clear();

    std::vector<string> vector_sourceid;
    ret = mongodb->GetFinishedTidBidSId(vector_sourceid);
    char result_str[128] = {0};
    sprintf(result_str, "%d:%d:%d", urlnode.task_id, urlnode.batch_id, urlnode.source_id);
    EXPECT_EQUAL(result_str, vector_sourceid[0], "get sourceid");
    mongo::BSONObjBuilder delete_url;
    delete_url.append("Key", row.c_str());
    ret = mongodb->DeleteFromCollection(T_URL_TO_FETCH_TBL, &delete_url);
    mongo::BSONObjBuilder delete_url2;
    delete_url2.append("Key", row.c_str());
    ret = mongodb->DeleteFromCollection(T_PRIORITY_URL_TO_FETCH_TBL, &delete_url2);
    delete mongodb;
    mongodb = NULL;
    printf("-----END: %s\n", __FUNCTION__);
    return;
  }

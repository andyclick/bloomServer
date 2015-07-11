#ifndef _OCTOPUS_COMMON_DEFINE_H_
#define _OCTOPUS_COMMON_DEFINE_H_

#include <string>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#ifdef _MSC_VER
typedef     unsigned char        	uint8_t;
//typedef     char	      					int8_t;
typedef     unsigned short       	uint16_t;
typedef     short	       					int16_t;
typedef     unsigned int 					uint32_t;
typedef     int		       					int32_t;

#ifdef _WIN64
typedef     unsigned long       	uint64_t;
typedef     long       						int64_t;
#else
typedef     unsigned long long      	uint64_t;
typedef     long long      						int64_t;
#endif //_WIN64
#endif //_MSC_VER

//common state
#define MY_STATE_OK                         0
#define MY_STATE_NOT_OK                     1
//bloom filter state 
#define BLOOF_FILTER_KEY_NOT_EXIST          -1
#define BLOOF_FILTER_SUBKEY_EXIST           1
#define BLOOF_FILTER_SUBKEY_NOT_EXIST       0 
#define CHECK_KEY_EXIST                     "check_key_exist"
 
//http ret
#define HTTP_FETCH_OK                       0
#define HTTP_FETCH_RET_REDIRECT             201
#define HTTP_FETCH_RET_ERROR                202
#define HTTP_FETCH_STATUS_ERROR             203
#define HTTP_FETCH_RET_ERROR_UNACCEPTED     204

//url send state 
#define URL_TO_FETCH_UNREAD                 "0"
#define URL_TO_FETCH_ALREADY_READ           "1" 
#define URL_TO_FETCH_ALREADY_SEND           "2"

// is main server
#define IS_MAIN_MAIN_SERVER                 1
#define IS_MAIN_READY_TO_SERVER             2
#define IS_MAIN_NOT_MAIN_SERVER             3
#define IS_MAIN_UNDETERMINED                4

//system state
#define SYSTEM_STATE_RUNNING                1
#define SYSTEM_STATE_SHUTDOWN               2
#define SYSTEM_STATE_PAUSE                  4
#define SYSTEM_STATE_INTERNAL_ERROR_OCCURED 8

//system sub state
#define RUNNING_STATE_RUNNING                0
#define RUNNING_STATE_NEED_TO_SHUTDOWN       1
#define RUNNING_STATE_SHUTDOWN               2

//split of row
#define SPLIT_OF_COLON                      ":" 
#define SPLIT_OF_UNDERLINE                  "_"

//url mode
#define URL_MODE_NO_MODE                    0
#define URL_MODE_FINAL_PAGE                 1
#define URL_MODE_LIST_PAGE                  2
#define URL_MODE_ENTRANCE_PAGE              4
#define URL_MODE_NEED_TO_SAVE               128
#define URL_MODE_NEED_TO_ANALYSE            256
#define URL_MODE_NEED_TO_ADD_NEW            512

//config
#define CONF_SN_PUBLIC                      "public"
#define CONF_SN_MYSQL                       "mysql"
#define CONF_SN_HYPERTABLE                  "hypertable"
#define CONF_SN_HEARTBEAT                   "heartbeat"
#define CONF_SN_TSERVER                     "tserverthrift"
#define CONF_SN_BSERVER                     "bserverthrift"
#define CONF_SN_DSERVER                     "dserverthrift"
#define CONF_WORK_DIR                       "work_dir"
#define CONF_PORT                           "port"
#define CONF_THREAD_COUNT                   "thread_count"
#define CONF_IP_ADDR                        "ip_addr"
#define CONF_DEV_NAME                       "dev_name"
#define CONF_LISTEN_THREAD_NUM              "listen_thread_num"
#define CONF_B_SENDCONTENT                  "b_server_content_thrift"
#define CONF_B_SERVER_ZOOKEEPER             "zookeeper"
#define CONF_B_SERVER_BLOOMFILTER           "bloomfilter"
#define CONF_T_SERVER_ID                    "t_server_id"

//task state
#define TASK_STATE_EFFECTIVE                 1
#define TASK_STATE_INVALID                   0 

//source state
#define SOURCE_STATE_EFFECTIVE               1
#define SOURCE_STATE_INVALID                 0

#define URL_DUPLICATE_MODE_BY_BATCH           1 
#define URL_DUPLICATE_MODE_BY_SOURCE          2 
#define URL_DUPLICATE_MODE_BY_TASK            4 

//for URL
#define MAX_URL_LEN                           1024 
#define MAX_HOST_LEN                          128

//for http request
#define HTTP_REQUEST_TYPE_GET                 0
#define HTTP_REQUEST_TYPE_POST                1


//for zookeeper 
#define ZOOKEEPER_NONE                          0 
#define ZOOKEEPER_NEED_CREATE_ZNODE             1

#define SPLIT_DB_SQL                            ";"

#define TASK_TABLE_NAME                         "task_tbl" 
#define SOURCE_TABLE_NAME                       "site_source_config_tbl"

#define URL_TYPE_PRIORITY                       1 
#define URL_TYPE_UNPRIORITY                     0 

#define INSERT_URL_FORCED                       1 
#define INSERT_URL_UNFORCED                     0

namespace octopus {
  namespace common {

    class IPStruct {
    public:
      std::string ip;
      uint32_t port;
    };
  }
}

#define PRIORITY_FETCH_TBL_LOCK_NAME           "priority_fetch_tbl_lock"
#define FETCH_TBL_LOCK_NAME                    "fetch_tbl_lock"
#define CONTENT_PAGE_TBL_LOCK_NAME             "content_page_tbl_lock"

#define MY_TRUE                                  "true"
#define MY_FALSE                                 "false"

//the method for getting custom data from entrance
#define GET_CUSTOM_DATA_FROM_ENTRANCE_URL    1
#define GET_CUSTOM_DATA_FROM_ENTRANCE_PAGE   2

#endif //_OCTOPUS_COMMON_DEFINE_H_

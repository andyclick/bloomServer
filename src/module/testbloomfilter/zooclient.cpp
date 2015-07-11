#include <time.h>
#include <iostream>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <zookeeper/zookeeper.h>
#include "ZookeeperManager.h"
#include "CoreManager.h"
#include <assert.h>
#include <mybase.h>
#include "Singleton.h"
#include "CoreManager.h"
#include "BodyServerConfig.h"
#include <string>

using namespace std;
//define
//#define CREATE_FLAG   ZOO_EPHEMERAL
//#define CREATE_FLAG   ZOO_EPHEMERAL|ZOO_SEQUENCE
#define CREATE_FLAG   ZOO_EPHEMERAL
//#define CREATE_FLAG  0 
#define CREATE_LEADER_FLAG   ZOO_SEQUENCE 
#define _LL_CAST_ (long long)
zhandle_t *zh;
clientid_t myid;

//char hostPort[]="192.168.40.141:2181,192.168.40.142:2181,192.168.40.143:2181,192.168.40.144:2181";
<<<<<<< .mine
char hostPort[]="192.168.245.38:2181";
=======
char hostPort[]="127.0.0.1:2181";
>>>>>>> .r27676
//static const char *hostPort;
char myservername[] = "b_server_";
char LeaderPath[] = "/octopus/leader";
char GroupMemberShiprootPath[] = "/octopus";
static const char *clientIdFile = 0;
static int shutdownThisThing=0;
struct timeval startTime;
static char cmd[1024];
static int batchMode=0;
static int verbose = 0;

void watcher(zhandle_t *zzh, int type, int state, const char *path,void* context);
void my_data_completion(int rc, const char *value, int value_len,const struct Stat *stat, const void *data);
void dumpStat(const struct Stat *stat); 
void my_stat_completion(int rc, const struct Stat *stat, const void *data); 
void my_leader_state_watcher(zhandle_t *zzh, int type, int state, const char *path,void* context);
int leader_awexists(zhandle_t *zh, string path);
int myClientStart();
string create_mynode();
void  findLeader();
int startsWith(const char *line, const char *prefix);
static const char* type2String(int state);
static const char* state2String(int state);
void get_child();
void get_child_completion(int rc, const struct String_vector *strings, const void *data);
void get_child_watcher(zhandle_t *zzh, int type, int state, const char *path,void* context);
string all_path;
void my_string_completion(int rc, const char *name, const void *data) {
    fprintf(stderr, "[%s]: rc = %d\n", (char*)(data==0?"null":data), rc);
    if (!rc) {
        fprintf(stderr, "\tname = %s\n", name);
    }
    if(batchMode)
      shutdownThisThing=1;
    //all_path = name;
}

void testZookeeperLock(); 
void testZookeeperDbChanged(); 

int main(int argc, char **argv) {

<<<<<<< .mine
  myClientStart();
  return 0;
=======
  myClientStart();
>>>>>>> .r27676
  log4cxx::xml::DOMConfigurator::configureAndWatch("log4cxx.xml", 3000);

  //initailize mybase log
  mybase::initialize_mybase_log(log4cxx::Logger::getLogger("in")
    , log4cxx::Logger::getLogger("er")
    , log4cxx::Logger::getLogger("in")
    , log4cxx::Logger::getLogger("in")
    , log4cxx::Logger::getLogger("in")
    );

  mylog_init(mybase::get_err_log());
  mylog_init(mybase::get_info_log());

  if (argc > 1) {
    int idx = atoi(argv[1]);
    switch(idx) {
    case 1:testZookeeperLock();break;
    case 2:testZookeeperDbChanged();break;
    default:printf("can not find case\n");
    }
  } else {
    printf("please input case idx\n");
  }

  sleep(100000);

	return 0;
}

void testZookeeperDbChanged() {
  octopus::b_server::CoreManager core_manager(NULL);

  MYBASE_CONFIG.load("b_server.conf"); 
  octopus::b_server::BodyServerConfig::instance().initialize();
  core_manager.get_zookeeper_manager()->init_zookeeper();
  core_manager.get_zookeeper_manager()->db_changed();
  sleep(100000);
}

void testZookeeperLock() {
  octopus::b_server::CoreManager core_manager(NULL);

  MYBASE_CONFIG.load("b_server.conf"); 
  octopus::b_server::BodyServerConfig::instance().initialize();
  core_manager.get_zookeeper_manager()->init_zookeeper();
  octopus::b_server::Completion completion;
  core_manager.get_zookeeper_manager()->init_zookeeper_lock(&completion, PRIORITY_FETCH_TBL_LOCK_NAME);
  if (core_manager.get_zookeeper_manager()->zookeeper_mutex_lock(&completion) == MY_STATE_OK) {
    //core_manager_->get_db_manager()->get_db_mongodb_manager()->GetPriorityUrlToFetch(priority_url_num, source_ids, 0 , urls);
  }
  { 
    octopus::b_server::Completion completion;
    core_manager.get_zookeeper_manager()->init_zookeeper_lock(&completion, PRIORITY_FETCH_TBL_LOCK_NAME);
    core_manager.get_zookeeper_manager()->zookeeper_mutex_lock(&completion); 
  }
  core_manager.get_zookeeper_manager()->zookeeper_mutex_unlock(&completion);

}

/*int main(int argc, char **argv) {
    int rc = 0;    
    zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
    zoo_deterministic_conn_order(1); // enable deterministic order
    hostPort = argv[1];
    zh = zookeeper_init(hostPort, watcher, 30000, &myid, 0, 0);
    if (!zh) {
        return errno;
    }
    create_mynode();
    sleep(5);  
    zookeeper_close(zh);
    return 0;
}*/
int myClientStart()
{
	cout<<"begin test ....."<<endl;
	zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
	zoo_deterministic_conn_order(1); // enable deterministic order
	
  zh = zookeeper_init(hostPort, watcher, 30000, 0, 0, 0);
	if (!zh) {
		fprintf(stderr, "error when connecing to zookeeper servers.... %d \n",errno);
		return errno;
  }

  //  get_child();
  string path = create_mynode();
<<<<<<< .mine
    findLeader();
  	leader_awexists(zh);
  while(true)
=======
  //findLeader();
  sleep(1);
  leader_awexists(zh, all_path);
  /*while(true)
>>>>>>> .r27676
  {
    fprintf(stderr, "my servers is running .... %d \n",errno);
    // test seq max val
    sleep(20);
  }*/
}
void get_child()
{
   char path[] = "/octopus";
   int rc = zoo_awget_children(zh, path,
            get_child_watcher, path,
            get_child_completion, strdup(path));

   //int  rc= zoo_aget_children(zh, path, 1, get_child_completion, strdup(path));
   if (rc) {
     fprintf(stderr, "Error %d for %s\n", rc, path);
   }
	fprintf(stderr, "after get_child\n");
}

void get_child_watcher(zhandle_t *zzh, int type, int state, const char *path,void* context)
{
	fprintf(stderr, "begin get_child in watcher\n");
  sleep(10);
	get_child();
  fprintf(stderr, "after get_child in watcher\n");
}

void get_child_completion(int rc, const struct String_vector *strings, const void *data)
{
  fprintf(stderr, "before get_child in completion\n");
  if (strings){
    for (int i=0; i < strings->count; i++) {
      fprintf(stderr, "\t%s\n", strings->data[i]);
    }
  }
  fprintf(stderr, "after get_child in completion\n");
  free((void *)data);
}

string create_mynode()
{
  char mypath[1024];
	try{
		char value[1024*1024];
		sprintf(value, "%s_%d", myservername, 1111);
		sprintf(mypath, "%s/%s%d", GroupMemberShiprootPath,myservername, 1111);
    all_path = mypath;
		//sprintf(mypath, "%s", "/liujingtesttask");
		int len = strlen(value);
		cout<<"begin create node :"<< mypath << " value :" <<value<<endl;
		int rc = zoo_acreate(zh, mypath, value, len, &ZOO_OPEN_ACL_UNSAFE, CREATE_FLAG, my_string_completion, strdup(mypath));
		//int rc = zoo_acreate(zh, mypath, value, len, &ZOO_OPEN_ACL_UNSAFE, 0, my_string_completion, strdup(mypath));
		cout<<"Error "<< rc << " path " <<mypath <<endl;
	}catch(...)
	{
		fprintf(stderr, "create_mynode error \n");
	}
  return mypath;
}

void  findLeader()
{
	fprintf(stderr, "%s_%d begin find leader\n", myservername, getpid());
	int rc;
        rc = zoo_aget(zh, LeaderPath, 1, my_data_completion, strdup(LeaderPath));
	if (rc == ZOK) {
		fprintf(stderr, "leader already exsit %d for %s\n", rc, LeaderPath);
	}
}

void watcher(zhandle_t *zzh, int type, int state, const char *path,void* context)
{
    // Be careful using zh here rather than zzh - as this may be mt code
     // the client lib may call the watcher before zookeeper_init returns 

    fprintf(stderr, "initializing Watcher %s state = %s", type2String(type), state2String(state));
    if (path && strlen(path) > 0) {
      fprintf(stderr, " for path %s", path);
    }
    fprintf(stderr, "\n");

    if (type == ZOO_SESSION_EVENT) {
        if (state == ZOO_CONNECTED_STATE) {
            const clientid_t *id = zoo_client_id(zzh);
            if (myid.client_id == 0 || myid.client_id != id->client_id) {
                myid = *id;
                fprintf(stderr, "Got a new session id: 0x%llx\n",
                        _LL_CAST_ myid.client_id);
                if (clientIdFile) {
                    FILE *fh = fopen(clientIdFile, "w");
                    if (!fh) {
                        perror(clientIdFile);
                    } else {
                        int rc = fwrite(&myid, sizeof(myid), 1, fh);
                        if (rc != sizeof(myid)) {
                            perror("writing client id");
                        }
                        fclose(fh);
                    }
                }
            }
        } else if (state == ZOO_AUTH_FAILED_STATE) {
            fprintf(stderr, "Authentication failure. Shutting down...\n");
            zookeeper_close(zzh);
            shutdownThisThing=1;
            zh=0;
        } else if (state == ZOO_EXPIRED_SESSION_STATE) {
            fprintf(stderr, "Session expired. Shutting down...\n");
            zookeeper_close(zzh);
            shutdownThisThing=1;
            zh=0;

            printf("zoo session expired reconnect");
            bool connected = false;
            //while(!completion->zookeepermanger_->need_to_shutdown_ && !connected) {
            zh = zookeeper_init(hostPort, watcher, 30000, 0, 0, 0);
            /*if (!completion->zookeepermanger_->my_zh_)
            {
              printf("error when connecing to zookeeper servers: %d", errno);
              usleep(500 * 1000);
            } else {
              connected = true;
            }
            */
            //zookeeper_close(zzh);
        }
    }
}
void my_data_completion(int rc, const char *value, int value_len,const struct Stat *stat, const void *data) 
{
	struct timeval tv;
	int sec;
	int usec;
	gettimeofday(&tv, 0);
	sec = tv.tv_sec - startTime.tv_sec;
	usec = tv.tv_usec - startTime.tv_usec;
	fprintf(stderr, "time = %d msec\n", sec*1000 + usec/1000);
	fprintf(stderr, "%s: rc = %d\n", (char*)data, rc);
	if (value) {
		fprintf(stderr, " value_len = %d\n", value_len);
		assert(write(2, value, value_len) == value_len);
	}
	fprintf(stderr, "\nStat:\n");
	dumpStat(stat);
	if (startsWith((const char *)data, "/octopers/leader") && rc== ZNONODE)
	{
		fprintf(stderr, "Error %d for %s\n", rc, LeaderPath);
		fprintf(stderr, "Creating [%s] node\n", LeaderPath);
		char value[1024];
		sprintf(value, "%s_%d", myservername, getpid());
		rc = zoo_acreate(zh, LeaderPath, value, strlen(value), &ZOO_OPEN_ACL_UNSAFE, CREATE_FLAG, my_string_completion, strdup(LeaderPath));
		if (rc)
		{
			fprintf(stderr, "Error %d for %s\n", rc, LeaderPath);
		}
	}
	free((void*)data);
}
void dumpStat(const struct Stat *stat) {
    char tctimes[40];
    char tmtimes[40];
    time_t tctime;
    time_t tmtime;

    if (!stat) {
        fprintf(stderr,"null\n");
        return;
    }
    tctime = stat->ctime/1000;
    tmtime = stat->mtime/1000;

    ctime_r(&tmtime, tmtimes);
    ctime_r(&tctime, tctimes);

    fprintf(stderr, "\tctime = %s\tczxid=%llx\n"
    "\tmtime=%s\tmzxid=%llx\n"
    "\tversion=%x\taversion=%x\n"
    "\tephemeralOwner = %llx\n",
     tctimes, _LL_CAST_ stat->czxid, tmtimes,
    _LL_CAST_ stat->mzxid,
    (unsigned int)stat->version, (unsigned int)stat->aversion,
    _LL_CAST_ stat->ephemeralOwner);
}
int leader_awexists(zhandle_t *zh, string path)
{
	int ret =  zoo_awexists(zh, (char *)path.c_str(), my_leader_state_watcher, (void*) 0, my_stat_completion, strdup(LeaderPath));
  printf("awexists ret = %d %s\n", ret, path.c_str());
  return ret;
}
void my_leader_state_watcher(zhandle_t *zzh, int type, int state, const char *path,void* context)
{
	fprintf(stderr, "Watcher %s state = %s", type2String(type), state2String(state));
	if (state == ZOO_CONNECTED_STATE) {
		if (type == ZOO_CREATED_EVENT)
		{
			fprintf(stderr, "leader already started....\n", type2String(type), state2String(state));
		}else if (type == ZOO_DELETED_EVENT)
		{
			//findLeader();
		}	
    leader_awexists(zh, (char *)all_path.c_str());
	} else if (state == ZOO_EXPIRED_SESSION_STATE) {
    int ret = leader_awexists(zh, (char *)all_path.c_str());
    /*while(ret == ZINVALIDSTATE) {
      ret = leader_awexists(zh, (char *)all_path.c_str());
      sleep(10);
    }*/
  }
}
void my_stat_completion(int rc, const struct Stat *stat, const void *data) {
	fprintf(stderr, "%s: rc = %d Stat:\n", (char*)data, rc);
	dumpStat(stat);
	free((void*)data);
}
static const char* type2String(int state){
  if (state == ZOO_CREATED_EVENT)
    return "CREATED_EVENT";
  if (state == ZOO_DELETED_EVENT)
    return "DELETED_EVENT";
  if (state == ZOO_CHANGED_EVENT)
    return "CHANGED_EVENT";
  if (state == ZOO_CHILD_EVENT)
    return "CHILD_EVENT";
  if (state == ZOO_SESSION_EVENT)
    return "SESSION_EVENT";
  if (state == ZOO_NOTWATCHING_EVENT)
    return "NOTWATCHING_EVENT";

  return "UNKNOWN_EVENT_TYPE";
}
static const char* state2String(int state){
  if (state == 0)
    return "CLOSED_STATE";
  if (state == ZOO_CONNECTING_STATE)
    return "CONNECTING_STATE";
  if (state == ZOO_ASSOCIATING_STATE)
    return "ASSOCIATING_STATE";
  if (state == ZOO_CONNECTED_STATE)
    return "CONNECTED_STATE";
  if (state == ZOO_EXPIRED_SESSION_STATE)
    return "EXPIRED_SESSION_STATE";
  if (state == ZOO_AUTH_FAILED_STATE)
    return "AUTH_FAILED_STATE";

  return "INVALID_STATE";
}
int startsWith(const char *line, const char *prefix) {
    int len = strlen(prefix);
    return strncmp(line, prefix, len) == 0;
}

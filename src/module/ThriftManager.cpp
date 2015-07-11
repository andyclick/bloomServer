#include "ThriftManager.h"
#include "bloomfilter/bloom_filter.h"
#include "bloomfilter/murmur.h"
#include "bloomfilter/common_define.h"
#include "thrift_filter/Serv.h"
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <server/TNonblockingServer.h>
#include <fstream>
#include "readconf.h"
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace apache::thrift::concurrency;

using boost::shared_ptr;

using namespace std;

class BloomFilter{
public:
    virtual int16_t initialize(size_t size);
    int16_t get(const char *bloom_key, const char *key);
    int16_t add (const char *key, int64_t n, double e);
    int16_t set(const char *bloom_key, const char *key);
    int16_t remove(char *bloom_key);
    int16_t remove_all();
    int16_t start_dump_thread();
    int16_t dump_to_file();
};

int16_t BloomFilter::initialize(size_t size)
{
    if (access("file.txt", R_OK) == -1)
    {
        printf("Can not find file \"file.txt\"");
        blooms_init(size);
    }
    else
    {
        blooms_load();
    }
    return 0;
}

char bloom_key_str[128];
char bloom_key_str1[128];
int16_t BloomFilter::get(const char* bloom_key, const char *key)
{
    //char bloom_key_str[128];
    //char bloom_key_str1[128];
    //strcpy(bloom_key_str, bloom_key);
    //strcpy(bloom_key_str1, key);
    return blooms_get(bloom_key_str, bloom_key_str1);
}

int16_t BloomFilter::add(const char* key, int64_t n, double e)
{
    //char bloom_key_str[64];
    strcpy(bloom_key_str,key);
    return blooms_add(bloom_key_str, n, e);
}
int16_t BloomFilter::set(const char *bloom_key, const char *key)
{
    //char bloom_key_str[128];
    char bloom_key_str1[128];
    strcpy(bloom_key_str, bloom_key);
    strcpy(bloom_key_str1, key);
    if (blooms_set(bloom_key_str, bloom_key_str1) == 0) {
        return 0;
    }
    return -1;
}
int16_t BloomFilter::remove(char *bloom_key)
{
    if (blooms_delete(bloom_key) == 0) {
        return 0;
    }
    return -1;
}
int16_t BloomFilter::remove_all()
{
    blooms_delete_all();

    return 0;
}


int16_t BloomFilter::start_dump_thread()
{
    char tmpStr[16];
    getconfigstr("public",  "dump_time", tmpStr, 16, "spider1.conf");
    printf("tmpStr:%s \n", tmpStr);
    size_t dump_seconds;
    dump_seconds = strtoul(tmpStr,0,10);
    printf("dump_seconds:%d \n", dump_seconds);
    pthread_t *tid = (pthread_t*)malloc(1 * sizeof(pthread_t));
    if (tid == NULL) {
        //mylog_fatal_errno(m->m_pLogGlobalCtrl->veryimportantlog, "malloc error for startGetTask\n");
    }
    //void* tmpArg = NULL;
    if(pthread_create(tid, NULL, blooms_dump, (void*)dump_seconds)) {
        //mylog_fatal_errno(m->m_pLogGlobalCtrl->veryimportantlog, "create DeleteTaskContentThread error\n");
    }
    printf("----------------------------------- thread return.\n");

    pthread_detach(*tid);
    free(tid);
    return 0;
}

int16_t BloomFilter::dump_to_file()
{
    blooms_dump1();
}


BloomFilter bloomfilter;
class ServHandler : virtual public ServIf {
public:
    ServHandler() {
        // Your initialization goes here
        //bloomfilter.initialize(400000000);
        bloomfilter.initialize(4000000000);
        bloomfilter.start_dump_thread();
    }

    /**
     * @brief   add a bloom filter.
     *
     * @param key   sourceId.
     * @param max_elements  longing element sum.
     * @param false_rate    longing impact rate.
     *
     * @return  0 for success, 1 for bloom exist, and 2 for add failed.
     */
    int16_t add(const std::string& key, const int32_t max_elements, const double false_rate) {
        return blooms_add(key.c_str(), max_elements, false_rate);
    }

    void fill(std::vector<element> & _return, const std::string& key, const std::vector<element> & vector_url) {
        std::vector<element>::const_iterator iter;
        int16_t ret = 0;
        element element_ret;
        for (iter = vector_url.begin(); iter != vector_url.end(); iter++)
        {
            ret = blooms_set(key.c_str(), ((*iter).url).c_str());
            if (ret == 0)
            {
                element_ret.is_exist = 0;
                element_ret.url = (*iter).url;
                _return.push_back(element_ret);
            }
            else if (ret == 1)
            {
                element_ret.is_exist = 1;
                element_ret.url = (*iter).url;
                _return.push_back(element_ret);
            }
        }
    }

    void get(std::vector<element> & _return, const std::string& key, const std::vector<element> & vector_url) {
        printf("vector_url'size: %d \n", vector_url.size());
        std::vector<element>::const_iterator iter;
        int16_t ret = 0;
        element element_ret;
        for (iter = vector_url.begin(); iter != vector_url.end(); iter++)
        {
            ret = blooms_get(key.c_str(), ((*iter).url).c_str());
            if(ret == 1)
            {
                element_ret.is_exist = 1;
                element_ret.url = (*iter).url;
                _return.push_back(element_ret);
            }
            else if (ret == 0)
            {
                element_ret.is_exist = 0;
                element_ret.url = (*iter).url;
                _return.push_back(element_ret);
            }
            else if (ret == -1)
            {
                element_ret.is_exist = -1;
                element_ret.url = (*iter).url;
                _return.push_back(element_ret);
            }
        }
    }

    void get_stats(std::vector<memInfo> & _return) {
        // Your implementation goes here
        //blooms_dump((void*)("dump"));
        printf("Dump bloomfilter test!\n");
        printf("get_stats\n");
    }
    void get_blooms_stats(std::vector<memInfo> & _return) {
        // Your implementation goes here
        printf("get_blooms_stats\n");
    }
    void get_bloom_stats(std::vector<memInfo> & _return, const std::string& key) {
        // Your implementation goes here
        printf("get_bloom_stats\n");
    }

    int16_t set_mem(const int32_t size) {
        // Your implementation goes here
        printf("set_mem\n");
    }

    /*
     * @param return. 1 for success.0 for key exist.-1 for key not exist.
     *
     */

    int16_t get_one(const std::string& key, const std::string& url) 
    {
        return blooms_get(key.c_str(), url.c_str());
    }

    int16_t fill_one(const std::string& key, const std::string& url) {
        return blooms_set(key.c_str(), url.c_str());
    }
    int16_t delete_blooms(const std::string& key) {
        return blooms_delete(key.c_str());
    }

    int16_t delete_blooms_all() {
        printf("blooms_delete_all\n");
    }

private:


};

ThriftManager::ThriftManager(LogGlobalCtrl * pLogGlobalCtrl)
{
    m_pLogGlobalCtrl = pLogGlobalCtrl;
}
int ThriftManager::start() {
    Manager::start();

    return 0;
}
void ThriftManager::retrieve()
{
    return;
}
void ThriftManager::dump()
{
    bloomfilter.dump_to_file();
    return;
}
void *ThriftManager::start_thrift_server()
{
    shared_ptr<ServHandler> handler(new ServHandler());
    shared_ptr<TProcessor> processor(new ServProcessor(handler));
    //shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    //shared_ptr<TProtocolFactory> protocol_factory(new TBinaryProtocolFactory(0, 0, false, false));
    shared_ptr<TProtocolFactory> protocol_factory(new TBinaryProtocolFactory());
    //shared_ptr<ThreadManager> thread_manager;
    // andy: 这个地方刘静是通过函数获取的要建立的最大线程数，我这里先简单的设成10回来完善的时候再修改。
    //thread_manager = ThreadManager::newSimpleThreadManager(10000);
    shared_ptr<ThreadManager> thread_manager(ThreadManager::newSimpleThreadManager(10));
    shared_ptr<PosixThreadFactory> thread_factory(new PosixThreadFactory());
    thread_manager->threadFactory(thread_factory);
    thread_manager->start();
    // andy:同样，端口号完善的时候也需要改。
#if 0
    shared_ptr<TNonblockingServer> server(new TNonblockingServer(processor, protocol_factory, /*GFactory::get_server_config().bserver_service_thrift_config_.port*/9090, thread_manager));
#endif
    TNonblockingServer server(processor, protocol_factory, /*GFactory::get_server_config().bserver_service_thrift_config_.port*/7001, thread_manager);
    server.serve();
    int16_t mark = 1;
    return NULL;
}

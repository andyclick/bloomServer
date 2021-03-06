// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "Serv.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ;

class ServHandler : virtual public ServIf {
 public:
  ServHandler() {
    // Your initialization goes here
  }

  int16_t add(const std::string& key, const int32_t max_elements, const double false_rate) {
    // Your implementation goes here
    printf("add\n");
  }

  void fill(std::vector<element> & _return, const std::string& key, const std::vector<element> & vector_url) {
    // Your implementation goes here
    printf("fill\n");
  }

  void get(std::vector<element> & _return, const std::string& key, const std::vector<element> & vector_url) {
    // Your implementation goes here
    printf("get\n");
  }

  void get_stats(std::vector<memInfo> & _return) {
    // Your implementation goes here
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

  int16_t get_one(const std::string& key, const std::string& url) {
    // Your implementation goes here
    printf("get_one\n");
  }

  int16_t fill_one(const std::string& key, const std::string& url) {
    // Your implementation goes here
    printf("fill_one\n");
  }

  int16_t delete_blooms(const std::string& key) {
    // Your implementation goes here
    printf("delete_blooms\n");
  }

  int16_t delete_blooms_all() {
    // Your implementation goes here
    printf("delete_blooms_all\n");
  }

};

int main(int argc, char **argv) {
  int port = 9090;
  shared_ptr<ServHandler> handler(new ServHandler());
  shared_ptr<TProcessor> processor(new ServProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}


#include "Serv.h"  // 替换成你的.h  
#include <transport/TSocket.h>  
#include <transport/TBufferTransports.h>  
#include <protocol/TBinaryProtocol.h>  
using namespace apache::thrift;  
using namespace apache::thrift::protocol;  
using namespace apache::thrift::transport;  
using boost::shared_ptr;  

int main(int argc, char **argv) {  
    boost::shared_ptr<TSocket> socket(new TSocket("localhost", 9090));  
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));  
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));  
    transport->open();  
    ITEM item;
    item.url = "bloomfilter test!";
    std::vector <ITEM> bloom_vector;
    bloom_vector.clear();
    bloom_vector.push_back(item);
    ServClient client(protocol);
    client.send_url_list(bloom_vector);
    std::cout << "Mark!" << std::endl;
    // 我们的代码写在这里  
    //   
    //       transport->close();  
    //         
    //             return 0;  
    //             }  
    }

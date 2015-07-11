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
    std::vector <ITEM> bloom_vector_returned;
    std::vector <ITEM> bloom_vector;
    ServClient client(protocol);
    bloom_vector_returned.clear();
    bloom_vector_returned.push_back(item);
    bloom_vector_returned.push_back(item);
    bloom_vector_returned.push_back(item);
    bloom_vector_returned.push_back(item);
    bloom_vector_returned.push_back(item);
    bloom_vector_returned.push_back(item);
    bloom_vector.clear();
    bloom_vector.push_back(item);
  
    client.send_url_list(bloom_vector_returned, bloom_vector);
    int a = 0;
    std::vector<ITEM>::iterator iter = bloom_vector_returned.begin();
    std::cout << (*iter).isExist << std::endl;
    std::cout << (*iter).url << std::endl;
    std::cout << (*iter).url << std::endl;
    std::vector<ITEM>::iterator iter1 = bloom_vector_returned.end();
    iter1--;
    std::cout << (*iter1).url << std::endl;
    std::cout << "Mark!" << std::endl;
    // 我们的代码写在这里  
    //   
    //       transport->close();  
    //         
    //             return 0;  
    //             }  
    }

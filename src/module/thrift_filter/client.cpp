#include "Serv.h"  // Ìæ»»³ÉÄãµÄ.h  
#include <transport/TSocket.h>  
#include <transport/TBufferTransports.h>  
#include <protocol/TBinaryProtocol.h>  
using namespace apache::thrift;  
using namespace apache::thrift::protocol;  
using namespace apache::thrift::transport;  
using boost::shared_ptr;  

int main(int argc, char **argv) {  
    boost::shared_ptr<TSocket> socket(new TSocket("localhost", 9096));  
    boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));  

    // 500 s
    socket->setConnTimeout(500*1000*1000);
    socket->setRecvTimeout(500*1000*1000);
    socket->setSendTimeout(500*1000*1000);
    transport->open();  
    ServClient client(protocol);
    ServClient client1(protocol);
    int ret;
    //ret = client.add("alibaba", 30000001, 0.00001);
    int i;
    //ret = client.fill_one("alibaba", "http://laputa.1688.com/offer/ajax/OfferDesc.do?offerId=11d&callback=rnd");
    std::vector <element> url_elements;
    std::vector <element> url_elements_return;
    element url_element;
    url_element.is_exist = -2;
    int ii;
    char url[1024];
    for(ii = 0; ii < 1000000; ii++)
    {
        sprintf(url,"http://laputa.1688.com/offer/ajax/OfferDesc.do?offerId=%010d&callback=rnd", ii);
        url_element.url = url;
        url_elements.push_back(url_element);
    }
    //client.fill(url_elements_return, "alibaba", url_elements);
    ret = client.get_one("tecent", "http://laputa.1688.com/offer/ajax/OfferDesc.do?offerId=10d&callback=rnd");
    client.delete_blooms("tecent");
    ret = client.get_one("tecent", "http://laputa.1688.com/offer/ajax/OfferDesc.do?offerId=10d&callback=rnd");
    int mark;
#endif
}

g++ -g -I/usr/local/include/thrift -L/usr/local/lib/ -lthrift Serv.cpp
g++ -g -I/usr/local/include/thrift -L/usr/local/lib/ -lthrift Serv.cpp -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H
g++ -g -I/usr/local/include/thrift -L/usr/local/lib/ -lthrift Serv.cpp msg_types.cpp msg_constants.cpp Serv_server.skeleton.cpp -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H
会报错，需要修改Serv_server.skeleton.cpp里的using namespace，之后就可以编译通过了。
参考刘静以前的源码，可以考虑把Serv.cpp加上几层命名空间，然后得修改Serv_server.skeleton.cpp里相应的using 声明。

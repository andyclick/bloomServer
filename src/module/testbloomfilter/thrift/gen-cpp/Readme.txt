g++ -g -I/usr/local/include/thrift -L/usr/local/lib/ -lthrift Serv.cpp
g++ -g -I/usr/local/include/thrift -L/usr/local/lib/ -lthrift Serv.cpp -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H
g++ -g -I/usr/local/include/thrift -L/usr/local/lib/ -lthrift Serv.cpp msg_types.cpp msg_constants.cpp Serv_server.skeleton.cpp -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H
�ᱨ����Ҫ�޸�Serv_server.skeleton.cpp���using namespace��֮��Ϳ��Ա���ͨ���ˡ�
�ο�������ǰ��Դ�룬���Կ��ǰ�Serv.cpp���ϼ��������ռ䣬Ȼ����޸�Serv_server.skeleton.cpp����Ӧ��using ������

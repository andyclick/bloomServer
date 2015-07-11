all:
	g++ -g -I/usr/local/include/thrift -L/usr/local/lib/ -lthrift -lm -pthread -lz -lrt -lssl Serv.cpp msg_types.cpp msg_constants.cpp client.cpp -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H -o client	
	g++ -g -I/usr/local/include/thrift -L/usr/local/lib/ -lthrift Serv.cpp msg_types.cpp msg_constants.cpp Serv_server.skeleton.cpp -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H -o server

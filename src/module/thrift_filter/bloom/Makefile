all:
	g++ -g -I/usr/local/include/thrift -L/usr/local/lib/ -lthrift -lm -pthread -lz -lrt -lssl Serv.cpp bloomfilter_types.cpp bloomfilter_constants.cpp -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H BloomfilterConnectionPool.cpp pool.cpp -o pool 
#g++ -g -I/usr/local/include/thrift -L/usr/local/lib/ -lthrift Serv.cpp bloomfilter_types.cpp bloomfilter_constants.cpp Serv_server.skeleton.cpp -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H -o server

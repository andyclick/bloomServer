SUBDIRS=module common tools
bin_PROGRAMS=bloomfilter
bloomfilter_SOURCES=main.cpp
BOOST_INCLUDE_DIR=/usr/local/include/boost
BOOST_THREAD_LIB=boost_thread
BOOST_IOSTREAMS_LIB=boost_iostreams
BOOST_PROGRAM_OPTIONS_LIB=boost_program_options
BOOST_SYSTEM_LIB=boost_system
SIGAR_LIB=sigar-amd64-linux
HYPERTABLE_INSTALL_DIR=/backup/bigtable/hypertable-0.9.6.5-linux-x86_64/opt/hypertable/0.9.6.5
THRIFT_DIR=/backup/bigtable/hypertable-0.9.6.5-linux-x86_64/opt/hypertable/0.9.6.5/include_back/thrift
COMMON_DIR=/backup/bigtable/hypertable-0.9.6.5-linux-x86_64/opt/hypertable/0.9.6.5/include_back/Common
bloomfilter_CFLAGS=-g -I$(BOOST_INCLUDE_DIR)  -I$(THRIFT_DIR) -I$(COMMON_DIR)
#bloomfilter_LDFLAGS=-D_GNU_SOURCE -L/usr/local/bloomfilter/lib -L$(HYPERTABLE_INSTALL_DIR)/lib  -g -I$(BOOST_INCLUDE_DIR) -I$(HYPERTABLE_INSTALL_DIR)/include  -I$(THRIFT_DIR) -I$(COMMON_DIR) -DHAVE_NETINET_IN_H -DHAVE_INTTYPES_H
bloomfilter_LDADD= module/libspidermodule.a tools/libspidertools.a -lz  -lrt -lcoredumper -lcurl -levent  -llog4cxx -liconv -lboost_regex -lsqlite3   -llog4cpp -lthriftnb -lunwind  -lexpat  -lcom_err -lre2 -ltcmalloc_minimal  -lboost_filesystem  -l$(BOOST_THREAD_LIB) -l$(BOOST_IOSTREAMS_LIB) -l$(BOOST_PROGRAM_OPTIONS_LIB) -l$(BOOST_SYSTEM_LIB) -l$(SIGAR_LIB)  -lz -lcurses   -lthrift -lmysqlcppconn -lboost_serialization
bloomfilter_LDFLAGS=-D_GNU_SOURCE -L/usr/local/bloomfilter/lib   -g -I$(BOOST_INCLUDE_DIR)  -I$(THRIFT_DIR) -I$(COMMON_DIR) -DHAVE_NETINET_IN_H -DHAVE_INTTYPES_H

confdir=$(prefix)
conf_DATA=bloomfilter.conf

tnsdir=$(prefix)
tns_DATA=tnsnames.ora

logconfdir=$(prefix)
logconf_DATA=log4cxx.xml

EXTRA_DIST=$(conf_DATA) $(tns_DATA) $(logconf_DATA)

noinst_PROGRAMS=test set_delete_flag
test_SOURCES=test.cpp 
test_LDADD= module/libspidermodule.a tools/libspidertools.a  -lz -lociei -lrt -lcoredumper  -lcurl -llog4cxx -liconv -lboost_regex -lmysqlcppconn 
test_LDFLAGS=-D_GNU_SOURCE -L/usr/local/bloomfilter/lib

#mydb_archive_SOURCES=mydb_archive.cpp
#mydb_archive_LDADD= module/libspidermodule.a tools/libspidertools.a -lz -lociei -lrt -lcoredumper 
#mydb_archive_LDFLAGS=-D_GNU_SOURCE -L/usr/local/bloomfilter/lib

set_delete_flag_SOURCES=set_delete_flag.cpp
#set_delete_flag_LDADD= module/libspidermodule.a tools/libspidertools.a -lz -lociei -lrt -lcoredumper 
set_delete_flag_LDFLAGS=-D_GNU_SOURCE -L/usr/local/bloomfilter/lib

#testlog_SOURCES=testlog.cpp 
#testlog_LDADD= -llog4cxx
#testlog_LDFLAGS=-D_GNU_SOURCE -L/usr/local/bloomfilter/lib

DEFS=
#LIBS=-lpthread
export INCLUDES

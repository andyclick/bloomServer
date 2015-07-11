SUBDIRS=src
#CURRENTPATH=/backup/spider1_ali_free
CURRENTPATH=$(shell /bin/pwd)
#HYPERTABLE_DIR=/backup/bigtable/hypertable-0.9.6.5-linux-x86_64/opt/hypertable/0.9.6.5/include_back
#HYPERTABLE_THRIFT_DIR=/backup/bigtable/hypertable-0.9.6.5-linux-x86_64/opt/hypertable/0.9.6.5/include_back/thrift
INCLUDES=-I"$(CURRENTPATH)/src/module/"\
		 -I"/usr/include/libxml2/"\
	     -I"$(CURRENTPATH)/src/tools/"\
         -I"$(CURRENTPATH)/src/threadpool/"\
	     -I"/usr/local/include/thrift/"
DEFS=-D_GNU_SOURCE -DVOIDTYPE

#CPFLAG:=$(DEFS)

LFLAGS=-Pyy
export INCLUDES 
export CURRENTPATH
export LFLAGS

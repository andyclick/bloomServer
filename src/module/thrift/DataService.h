/**
 * Autogenerated by Thrift Compiler (0.7.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 */
#ifndef DataService_H
#define DataService_H

#include <TProcessor.h>
#include "DataService_types.h"

namespace DataService {

class DataServiceIf {
 public:
  virtual ~DataServiceIf() {}
  virtual void get_content(PageData& _return, const std::string& column_name, const int32_t num, const std::string& now_value, const std::string& tablename) = 0;
  virtual void update_read_flag(const std::string& column_name, const std::string& set_val, const std::string& now_value, const std::string& table_name) = 0;
};

class DataServiceNull : virtual public DataServiceIf {
 public:
  virtual ~DataServiceNull() {}
  void get_content(PageData& /* _return */, const std::string& /* column_name */, const int32_t /* num */, const std::string& /* now_value */, const std::string& /* tablename */) {
    return;
  }
  void update_read_flag(const std::string& /* column_name */, const std::string& /* set_val */, const std::string& /* now_value */, const std::string& /* table_name */) {
    return;
  }
};

typedef struct _DataService_get_content_args__isset {
  _DataService_get_content_args__isset() : column_name(false), num(false), now_value(false), tablename(false) {}
  bool column_name;
  bool num;
  bool now_value;
  bool tablename;
} _DataService_get_content_args__isset;

class DataService_get_content_args {
 public:

  DataService_get_content_args() : column_name(""), num(0), now_value(""), tablename("") {
  }

  virtual ~DataService_get_content_args() throw() {}

  std::string column_name;
  int32_t num;
  std::string now_value;
  std::string tablename;

  _DataService_get_content_args__isset __isset;

  void __set_column_name(const std::string& val) {
    column_name = val;
  }

  void __set_num(const int32_t val) {
    num = val;
  }

  void __set_now_value(const std::string& val) {
    now_value = val;
  }

  void __set_tablename(const std::string& val) {
    tablename = val;
  }

  bool operator == (const DataService_get_content_args & rhs) const
  {
    if (!(column_name == rhs.column_name))
      return false;
    if (!(num == rhs.num))
      return false;
    if (!(now_value == rhs.now_value))
      return false;
    if (!(tablename == rhs.tablename))
      return false;
    return true;
  }
  bool operator != (const DataService_get_content_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const DataService_get_content_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class DataService_get_content_pargs {
 public:


  virtual ~DataService_get_content_pargs() throw() {}

  const std::string* column_name;
  const int32_t* num;
  const std::string* now_value;
  const std::string* tablename;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _DataService_get_content_result__isset {
  _DataService_get_content_result__isset() : success(false) {}
  bool success;
} _DataService_get_content_result__isset;

class DataService_get_content_result {
 public:

  DataService_get_content_result() {
  }

  virtual ~DataService_get_content_result() throw() {}

  PageData success;

  _DataService_get_content_result__isset __isset;

  void __set_success(const PageData& val) {
    success = val;
  }

  bool operator == (const DataService_get_content_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const DataService_get_content_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const DataService_get_content_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _DataService_get_content_presult__isset {
  _DataService_get_content_presult__isset() : success(false) {}
  bool success;
} _DataService_get_content_presult__isset;

class DataService_get_content_presult {
 public:


  virtual ~DataService_get_content_presult() throw() {}

  PageData* success;

  _DataService_get_content_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _DataService_update_read_flag_args__isset {
  _DataService_update_read_flag_args__isset() : column_name(false), set_val(false), now_value(false), table_name(false) {}
  bool column_name;
  bool set_val;
  bool now_value;
  bool table_name;
} _DataService_update_read_flag_args__isset;

class DataService_update_read_flag_args {
 public:

  DataService_update_read_flag_args() : column_name(""), set_val(""), now_value(""), table_name("") {
  }

  virtual ~DataService_update_read_flag_args() throw() {}

  std::string column_name;
  std::string set_val;
  std::string now_value;
  std::string table_name;

  _DataService_update_read_flag_args__isset __isset;

  void __set_column_name(const std::string& val) {
    column_name = val;
  }

  void __set_set_val(const std::string& val) {
    set_val = val;
  }

  void __set_now_value(const std::string& val) {
    now_value = val;
  }

  void __set_table_name(const std::string& val) {
    table_name = val;
  }

  bool operator == (const DataService_update_read_flag_args & rhs) const
  {
    if (!(column_name == rhs.column_name))
      return false;
    if (!(set_val == rhs.set_val))
      return false;
    if (!(now_value == rhs.now_value))
      return false;
    if (!(table_name == rhs.table_name))
      return false;
    return true;
  }
  bool operator != (const DataService_update_read_flag_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const DataService_update_read_flag_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class DataService_update_read_flag_pargs {
 public:


  virtual ~DataService_update_read_flag_pargs() throw() {}

  const std::string* column_name;
  const std::string* set_val;
  const std::string* now_value;
  const std::string* table_name;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class DataService_update_read_flag_result {
 public:

  DataService_update_read_flag_result() {
  }

  virtual ~DataService_update_read_flag_result() throw() {}


  bool operator == (const DataService_update_read_flag_result & /* rhs */) const
  {
    return true;
  }
  bool operator != (const DataService_update_read_flag_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const DataService_update_read_flag_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class DataService_update_read_flag_presult {
 public:


  virtual ~DataService_update_read_flag_presult() throw() {}


  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class DataServiceClient : virtual public DataServiceIf {
 public:
  DataServiceClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) :
    piprot_(prot),
    poprot_(prot) {
    iprot_ = prot.get();
    oprot_ = prot.get();
  }
  DataServiceClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) :
    piprot_(iprot),
    poprot_(oprot) {
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void get_content(PageData& _return, const std::string& column_name, const int32_t num, const std::string& now_value, const std::string& tablename);
  void send_get_content(const std::string& column_name, const int32_t num, const std::string& now_value, const std::string& tablename);
  void recv_get_content(PageData& _return);
  void update_read_flag(const std::string& column_name, const std::string& set_val, const std::string& now_value, const std::string& table_name);
  void send_update_read_flag(const std::string& column_name, const std::string& set_val, const std::string& now_value, const std::string& table_name);
  void recv_update_read_flag();
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class DataServiceProcessor : virtual public ::apache::thrift::TProcessor {
 protected:
  boost::shared_ptr<DataServiceIf> iface_;
  virtual bool process_fn(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, std::string& fname, int32_t seqid, void* callContext);
 private:
  std::map<std::string, void (DataServiceProcessor::*)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*)> processMap_;
  void process_get_content(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_update_read_flag(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  DataServiceProcessor(boost::shared_ptr<DataServiceIf> iface) :
    iface_(iface) {
    processMap_["get_content"] = &DataServiceProcessor::process_get_content;
    processMap_["update_read_flag"] = &DataServiceProcessor::process_update_read_flag;
  }

  virtual bool process(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot, void* callContext);
  virtual ~DataServiceProcessor() {}
};

class DataServiceMultiface : virtual public DataServiceIf {
 public:
  DataServiceMultiface(std::vector<boost::shared_ptr<DataServiceIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~DataServiceMultiface() {}
 protected:
  std::vector<boost::shared_ptr<DataServiceIf> > ifaces_;
  DataServiceMultiface() {}
  void add(boost::shared_ptr<DataServiceIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void get_content(PageData& _return, const std::string& column_name, const int32_t num, const std::string& now_value, const std::string& tablename) {
    size_t sz = ifaces_.size();
    for (size_t i = 0; i < sz; ++i) {
      if (i == sz - 1) {
        ifaces_[i]->get_content(_return, column_name, num, now_value, tablename);
        return;
      } else {
        ifaces_[i]->get_content(_return, column_name, num, now_value, tablename);
      }
    }
  }

  void update_read_flag(const std::string& column_name, const std::string& set_val, const std::string& now_value, const std::string& table_name) {
    size_t sz = ifaces_.size();
    for (size_t i = 0; i < sz; ++i) {
      ifaces_[i]->update_read_flag(column_name, set_val, now_value, table_name);
    }
  }

};

} // namespace

#endif

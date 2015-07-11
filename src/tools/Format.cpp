#include "Format.h"

Format::Format(string &str) {
    this->str = (char *)str.c_str();
    this->length = str.size();
    record = NULL;
    count = 0;
}

Format::Format(char *str, size_t length) {
    this->str = str;
    this->length = length;
    record = NULL;
    count = 0;
}
Format::~Format() {
    free(str_buffer);
    free(record);
}
void Format::parse() {
   char delims = (char)*LINE_DELIMS;
   str_buffer = split(str, length, &record, &count, delims, 1); 
}
void Format::parse(char delims) {
   str_buffer = split(str, length, &record, &count, delims, 1); 
}
char ** Format::getRecord() {
    return record; 
}
int Format::getCount() {
    return count; 
}
string Format::toString() {
    string ret; 
    for(int i = 0; i < count; i++) {
        if (i > 0) {
            ret.append(LINE_DELIMS);
        }
        ret.append(record[i]);
    }
    return ret;
}

#ifndef KUROBOT_FORMAT_H
#define KUROBOT_FORMAT_H

#include <string>
#include "util.h"

using namespace std;

class Format {
public:
    Format(string &str);
    Format(char *str, size_t length); 
    ~Format();
    string toString();
    char ** getRecord(); 
    void parse(); 
    void parse(char delimes); 
    int getCount(); 
private:
    char ** record;
    int count;
    size_t length;
    char *str;
    char *str_buffer;
};
#endif

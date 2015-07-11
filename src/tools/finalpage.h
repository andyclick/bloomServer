#ifndef _FINALPAGE_H_
#define _FINALPAGE_H_

#include <stdlib.h>
#include "util.h"
#include "ic_types.h"
#include <string>
#include <set>
#include <list>

using namespace std;

class FinalPage {
public:
    FinalPage();
    ~FinalPage();
    static FinalPage * const getInstance();
    int retrieve(); 
    int nextPage(UrlNode *fatherurlnode, list<UrlNode *> &urls, char *url); 
    void destroy(); 
private:
    void genNextPage(UrlNode *, set<string> &urls); 
    string  have_suffix(string fatherurl, string urltemplet, string::size_type urlbegin, string::size_type urlend,string fatherpage,string nextpage);
    string  no_suffix(string fatherurl, string urltemplet, string::size_type urlbegin,string  fatherpage,string nextpage);
    string have_questionmark(string fatherurl,string urltemplet,string fatherpage,string nextpage);
    static FinalPage *_instance;
    list<string> pagesuffix;
};

#endif //_FINALPAGE_H_

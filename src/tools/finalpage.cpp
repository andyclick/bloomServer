#include "InfoCrawler.h"
#include "finalpage.h"

FinalPage *FinalPage::_instance = NULL;

FinalPage::FinalPage() {

}

FinalPage::~FinalPage() {

}

FinalPage* const FinalPage::getInstance()
{
    if(_instance == NULL) {
        _instance = new FinalPage();
    } 
    return _instance;
} 

void FinalPage::destroy() {
    delete this;
}

int FinalPage::retrieve() {
    return 0;
}

int FinalPage::nextPage(UrlNode *fatherurlnode, list<UrlNode *> &urls, char *url) {
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    set<string> nextpageurls; 
    list<UrlNode *>::iterator iter; 
    set<string>::iterator nextpageiter; 

    genNextPage(fatherurlnode, nextpageurls);
    //Debug(0, 1, ("Debug: nextpage urls' size = %d\n", nextpageurls.size()));

    for(iter = urls.begin(); iter != urls.end(); iter++) {
        if ((nextpageiter = nextpageurls.find((*iter)->url)) != nextpageurls.end()) {
            mylog_info(m_pLogGlobalCtrl->infolog, "find nextpage url %s in %s - %s:%s:%d\n",fatherurlnode->url, nextpageiter->c_str(),INFO_LOG_SUFFIX);
            strcpy(url, nextpageiter->c_str());
            return 1;
        }
    }
    return 0;
}

//generate new page url in terms of suffix
void FinalPage::genNextPage(UrlNode *fatherurlnode, set<string> &urls) {
    string newurl;
    char urltemplet[1024] ;urltemplet[0] = 0;
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    strcpy(urltemplet, icconfig->urltemplate);
    string fatherurl = fatherurlnode -> url;


     string::size_type urlbegin = 0;
     string::size_type urlend = 0;
     string::size_type urlend_questionmark = 0;

    int  fatherpage=1;
    char  page[10] ;page[0] = 0;
    char nextpage[10];nextpage[0] = 0;
    sprintf(page,"%d",fatherurlnode->page);
    sprintf(nextpage,"%d",fatherurlnode->page+1);

    urlend = fatherurl.rfind(".");
    urlbegin = fatherurl.rfind("/");
    urlend_questionmark =  fatherurl.rfind("?");
    if (urlbegin == (fatherurl.length() - 1))
    {
        urlend = urlbegin;
        urlbegin = fatherurl.substr(0,urlend).rfind("/");
    }
    char *pToken = NULL;
    char *tokbuff = NULL;
    pToken = strtok_r(urltemplet, " ", &tokbuff);
    while(pToken)
    {
        if (urlend_questionmark != string::npos)
        {
            string url_front = fatherurl.substr(0,urlend_questionmark);
            string url_back = fatherurl.substr(urlend_questionmark);
            if (strstr(pToken,"&"))
            {
                newurl = url_front;
                newurl.append(have_questionmark( url_back, pToken, page, nextpage));
            }else
            {
                urlend = url_front.rfind(".");
                urlbegin = url_front.rfind("/");
                if ((urlbegin != string::npos) && (urlend != string ::npos) && (urlend > urlbegin))
                {
                    string newurl =have_suffix(url_front,pToken,urlbegin,urlend,page,nextpage);
                    newurl.append(url_back);

                }else if ((urlbegin != string ::npos) && (urlend < urlbegin))
                {
                    string newurl = no_suffix( url_front,  pToken,  urlbegin,page,nextpage);
                    newurl.append(url_back);
                }
            }
        } else if ((urlbegin != string::npos) && (urlend != string ::npos) && (urlend > urlbegin))
        {
            string newurl =have_suffix(fatherurl,pToken,urlbegin,urlend,page,nextpage);

        }else if ((urlbegin != string ::npos) && (urlend < urlbegin))
        {
            string newurl = no_suffix( fatherurl,  pToken,  urlbegin,page,nextpage);
        }else
        {
            return;
        }
        
        if (!newurl.empty())
        {
            urls.insert(newurl);
        }
        pToken = strtok_r(NULL, " ", &tokbuff);
    }
}
string  FinalPage::have_suffix(string fatherurl, string urltemplet, string::size_type urlbegin, string::size_type urlend,string   fatherpage,string nextpage)
{
    string newurl = "";
     string::size_type templetbegin = 0;
     string::size_type tmp_templebegin = 0;
    string templet = "";
    string templet1 = "";
    string tmpurl = fatherurl.substr(urlbegin+1);
    if (((templetbegin = urltemplet.find("[n]") ) != string::npos ) && (urltemplet.empty()) && (!tmpurl.empty()))
    {
        templet = urltemplet.substr(0,templetbegin);
        templet1 = templet;
        templet.append(fatherpage);
        templet.append(fatherurl.substr(urlend,fatherurl.length() - urlend));
        if (!templet.empty())
        {
            if ((tmp_templebegin = tmpurl.rfind(templet)) != string::npos )
            {
                newurl = fatherurl.substr(0,urlbegin+1);
                newurl.append(tmpurl.substr(0,tmp_templebegin));
                newurl.append(templet1);
                newurl.append(nextpage);
                newurl.append(fatherurl.substr(urlend,fatherurl.length() - urlend));
            }else
            {
                newurl = fatherurl.substr(0,urlend);
                newurl.append(templet1);
                newurl.append(nextpage);
                newurl.append(fatherurl.substr(urlend,fatherurl.length() - urlend));
            }
        }
    }
    return newurl;
}
string  FinalPage::no_suffix(string fatherurl, string urltemplet, string::size_type  urlbegin,string fatherpage,string nextpage)
{
    string newurl = "";
     string::size_type templetbegin = 0;
     string::size_type tmp_templebegin = 0;
    string templet = "";
    string templet1 = "";
    string tmpurl = fatherurl.substr(urlbegin+1,fatherurl.length() - urlbegin - 1);
    if (((templetbegin = urltemplet.find("[n]") ) != string::npos ) && (urltemplet.empty()) && (!tmpurl.empty()))
    {
        templet = urltemplet.substr(0,templetbegin);
        templet1 = templet;
        templet.append(fatherpage);
        if ((tmpurl.length() > templet.length()) && (tmpurl.substr(tmpurl.length() - templet.length()) ==  templet))
        {
            newurl = fatherurl.substr(0,urlbegin+1);
            newurl.append(tmpurl.substr(0,tmpurl.length() - templet.length()));
            newurl.append(templet1);
            newurl.append(nextpage);
        }else
        {
            newurl = fatherurl;
            newurl.append(templet1);
            newurl.append(nextpage);

        }
    }
    return newurl;
}
string FinalPage::have_questionmark(string fatherurl,string urltemplet,string fatherpage,string nextpage)
{
    string newurl;
     string::size_type templetbegin = 0;
     string::size_type urlbegin =0,urlend = 0;
    string templet = "";
    string templet1 = "";
    if (((templetbegin = urltemplet.find("[n]") ) != string::npos) && (urltemplet.empty())) 
    {
        templet = urltemplet.substr(1,templetbegin-1);
        templet1 = templet;
        templet.append(fatherpage);
        if((urlbegin = fatherurl.find(templet)) != string::npos)
        {
            newurl = fatherurl.substr(0,urlbegin)+templet1+nextpage+fatherurl.substr(urlbegin+templet.length());
        }else
        {
            newurl = fatherurl + "&"+templet1+nextpage;
        }

    }
    return newurl;
}
/*void FinalPage::genNextPage(UrlNode *fatherurlnode, set<string> &urls) {
    string fatherurl = fatherurlnode -> url;
    char urltemplet[1024] ;urltemplet[0] = 0;
    string templet = "";
    string templet1="";
    unsigned int urlbegin = 0;
    unsigned int urlend = 0;
    char  page[10] ;page[0] = 0;
    sprintf(page,"%d", fatherurlnode->page + 1);
    urlend = fatherurl.rfind(".");
    urlbegin = fatherurl.rfind("/");
    string newurl = "";
    ICCONFIG *icconfig = InfoCrawler::getInstance()->getConf();
    strcpy(urltemplet, icconfig->urltemplate);
    if ((urlbegin != string::npos) && (urlend != string ::npos) && (urlend > urlbegin))
    {
        //string tmpurl = fatherurl.substr(urlbegin+1,urlend - urlbegin -1);
        string tmpurl = fatherurl.substr(urlbegin+1);
        char * pToken = NULL;
        pToken = strtok(urltemplet, " ");
        while(pToken)
        {
            unsigned int templetbegin = 0;
            unsigned int tmp_templebegin = 0;
            string Token = pToken;
            templetbegin = Token.find("[n]");
            if ((templetbegin = Token.find("[n]") ) != string::npos )
            {
                templet = Token.substr(0,templetbegin);
                templet1 = templet;
                char pagestr[16] ;pagestr[0]= 0;
                sprintf(pagestr, "%d", fatherurlnode->page);
                templet.append(pagestr);
                templet.append(fatherurl.substr(urlend,fatherurl.length() - urlend));
                if (!templet.empty())
                {
                    if ((tmp_templebegin = tmpurl.rfind(templet)) != string::npos )
                    {
                       newurl = fatherurl.substr(0,urlbegin+1);
                       newurl.append(tmpurl.substr(0,tmp_templebegin));
                       newurl.append(templet1);
                       newurl.append(page);
                       newurl.append(fatherurl.substr(urlend,fatherurl.length() - urlend));
                    }else
                    {
                        newurl = fatherurl.substr(0,urlend);
                        newurl.append(templet1);
                        newurl.append(page);
                        newurl.append(fatherurl.substr(urlend,fatherurl.length() - urlend));
                    }
                }else
                {
                    continue;
                }
            }
            urls.insert(newurl); 
            pToken = strtok(NULL, " ");
        }

    }else if ((urlbegin != string ::npos) && (urlend < urlbegin))
    {
       string tmpurl = fatherurl.substr(urlbegin+1,fatherurl.length() - urlbegin - 1);
       char * pToken = NULL;
       pToken = strtok(urltemplet, " ");
       while(pToken)
       {
           unsigned int templetbegin = 0;
           unsigned int tmp_templebegin = 0;
           string Token = pToken;
           templetbegin = Token.find("[n]");
           if ((templetbegin = Token.find("[n]") ) != string::npos )
           {
              // if ((tmp_templebegin = tmpurl.rfind(templet)) != string::npos )
               templet = Token.substr(0,templetbegin);
               templet1 = templet;
                char pagestr[16] ;pagestr[0] = 0;
                sprintf(pagestr, "%d", fatherurlnode->page);
               templet.append(pagestr);
               if (tmpurl.substr(tmpurl.length() - templet.length()) ==  templet)
               {
                    newurl = fatherurl.substr(0,urlbegin+1);
                    newurl.append(tmpurl.substr(0,tmpurl.length() - templet.length()));
                    newurl.append(templet1);
                    newurl.append(page);
               }else
               {
                   newurl = fatherurl;
                   newurl.append(templet1);
                   newurl.append(page);

               }
               
           }else
           {
               continue;
           }
           urls.insert(newurl);
           pToken = strtok(NULL, " ");
       }
    }
}*/

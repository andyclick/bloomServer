//
// HtRegex.cc
//
// HtRegex: A simple C++ wrapper class for the system regex routines.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtRegex.cpp,v 1.1.1.1 2006/05/11 07:52:13 root Exp $
//

#include "HtRegex.h"
#include <locale.h>


HtRegex::HtRegex() : compiled(0) { }

HtRegex::HtRegex(const char *str, int case_sensitive) : compiled(0)
{
        set(str, case_sensitive);
}

HtRegex::~HtRegex()
{
	if (compiled != 0) regfree(&re);
	compiled = 0;
}

const string &HtRegex::lastError()
{
	return lastErrorMessage;
}

int
HtRegex::set(const char * str, int case_sensitive)
{
  if (compiled != 0) regfree(&re);

  int err;
  compiled = 0;
  if (str == NULL) return 0;
  if (strlen(str) <= 0) return 0;
  if (err = regcomp(&re, str, case_sensitive ? REG_EXTENDED : (REG_EXTENDED|REG_ICASE)), err == 0)
    {
      compiled = 1;
    }
  else
    {
      size_t len = regerror(err, &re, 0, 0);
      char *buf = new char[len];
      regerror(err, &re, buf, len);
      lastErrorMessage = buf;
      delete []buf;
    }
  return compiled;
}

int
HtRegex::match(const char * str, int nullpattern, int nullstr)
{
	int	rval;
	
	if (compiled == 0) return(nullpattern);
	if (str == NULL) return(nullstr);
	if (strlen(str) <= 0) return(nullstr);
	rval = regexec(&re, str, (size_t) 0, NULL, 0);
	if (rval == 0) return(1);
	else return(0);
}
int HtRegex::replace(const char * str,const char * Pattern, const char * replacestr ,char * result)
{
    int slen = strlen(str);
    size_t pos=0,len=strlen(replacestr);
    regmatch_t match;
    const char * t = str;
    result [0] = 0;
    set(Pattern);
    if (compiled == 0) return -1;
	if (str == NULL) return -1;
    if (strlen(str) <= 0) return -1;
    while(regexec(&re,t,1,&match,0)==0)
    {
        strncpy(result+pos,t,match.rm_so);//first copy the string that doesn't match
        pos+=match.rm_so;//change pos
        strncpy(result+pos,replacestr,len);//then replace 
        pos+=len;
        t+=match.rm_eo;//for another match
    }
    strncpy(result+pos,t,strlen(t));//don't forget the last unmatch string
    pos +=strlen(t);
    result[pos] = 0;
    return 0;
}
/*
int
HtRegex::getmatchs(const char * str, int nullpattern, int nullstr, int sublen, regmatch_t *subs)
{
	int	rval;
	
	if (compiled == 0) return(nullpattern);
	if (str == NULL) return(nullstr);
	if (strlen(str) <= 0) return(nullstr);
	rval = regexec(&re, str, (size_t) sublen, subs, 0);
	if (rval == 0) return(1);
	else return(0);
}
*/

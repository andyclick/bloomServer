//
// HtRegex.h
//
// HtRegex: A simple C++ wrapper class for the system regex routines.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtRegex.h,v 1.1.1.1 2006/05/11 07:52:13 root Exp $
//
//

#ifndef	_HtRegex_h_
#define	_HtRegex_h_

// This is an attempt to get around compatibility problems 
// with the included regex

#include <regex.h>
#include <sys/types.h>

#include <string.h>
#include <fstream>
#include <string>
using namespace std;

class HtRegex 
{
public:
    //
    // Construction/Destruction
    //
    HtRegex();
    HtRegex(const char *str, int case_sensitive = 0);
    virtual ~HtRegex();

    //
    // Methods for setting the pattern
    //
    int set(string& str, int case_sensitive = 0) { return set(str.c_str(), case_sensitive); }
    int set(const char *str, int case_sensitive = 0);

	virtual const string &lastError();	// returns the last error message

    //
    // Methods for checking a match
    //
    int		match(string& str, int nullmatch, int nullstr) { return match(str.c_str(), nullmatch, nullstr); }
    int		match(const char *str, int nullmatch, int nullstr);
    //int     getmatchs(const char * str, int nullpattern, int nullstr, int sublen, regmatch_t *subs);
    int replace(const char * str,const char * Pattern, const char * replacestr ,char * result);
    regex_t		re;

protected:
    int			compiled;

    string		lastErrorMessage;
};

#endif

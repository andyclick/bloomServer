/*
 * uri.l -- Routines dealing with URI, mainly parsing and merging.
 * Created: Xie Han, OS lab of Peking University. <me@pku.edu>
 *
 * The grammar of URI is quite simply so yacc is not needed.
 *
 * Created: Sep 25 04:15am 2003. version 0.1.1
 *		# Can read URI from standard input, parse it and print every
 *		  part of the URI.
 *
 * Updated: Sep 25 05:10pm 2003. version 0.1.2
 *
 * Updated: Sep 25 10:30pm 2003. version 0.1.3
 *		# A lot of changes in the program structure, much more clear.
 *		# Now the result is written into a uri structure.
 *		# A little bug. String "?" is accepted as a valid URI.
 *
 * Updated: Sep 26 02:16am 2003. version 0.1.4
 *		# 2 states are added to solve the bug in version 0.1.3.
 *		# The uri structure was redesigned.
 *
 * Updated: Sep 27 05:21am 2003. version 1.0.0!!!!
 *		# All states are changed from exclusive to shared to make
 *		  the lex program more compatible because some lexer do not
 *		  support exclusive start condition.
 *		# "unput" is substituted by "yyless" when putting back excess
 *		  text, because the latter cost less.
 *		# 4 parsing functions are completed, each of which has different
 *		  characteristic.
 *		# Function "uri_rally_parse" is added, which cooperates with
 *		  other lex programs.
 *
 * Updated: Sep 28 1:00pm 2003. version 1.1.0
 *		# The function "uri_rel_to_abs" is under construction.
 *		# The URI structure is changed again: "path_type" field is remove,
 *		  and "abs_path", "rel_path", "opaque_path" are substituted by
 *		  "path" because the original design is too sophisticated.
 *
 * Updated: National Day 5:06am 2003. version 1.1.1
 *		# The function "__uri_path_merge" which merges the base path and
 *		  the relative path is finished and tested and proved to be a
 *		  nice job! This function is the core of merging a base URI and
 *		  a relative URI into an absolute URI.
 *		# The previous function "uri_rel_to_abs" is commented out. It's
 *		  terrible.
 *
 * Updated: National Day 5:00pm 2003. version 1.1.2
 *		# Now we distinguish between "no" and "empty". Related fields
 *		  include "userinfo", "port", "query", and "fragment". The previous
 *		  version treats "no" and "empty" equally, which is not right.
 *
 * Updated: Oct 2 2:42am 2003. version 1.1.3
 *		# Solve the conflict of sever and reg_name by adding a start
 *		  condition and some ugly codes that are normally not reached.
 *		# The "uri_rel_to_abs" is rewritten but has not been tested.
 *
 * Updated: Oct 4 6:33pm 2003. version 1.2.0
 *		# Function "uri_path_merge" now returns the length of the result
 *		  path while the result path is returned by a pointer.
 *		# Function "uri_rel_to_abs" is renamed "uri_merge", and it will
 *		  return the string length of the result URI.
 *		# Function "uri_recombine" is coded, which recombine the a URI
 *		  structure into a URI string, and the string length is returned.
 *		# The URI merging algorithm in RFC 2396 is found not flawless, and
 *		  the problem is fixed in "uri_merge" with not very graceful means.
 *
 * Updated: Oct 5 2:09am 2003. version 1.2.1
 *		# From this version function "uri_parse_string" is implemented by
 *		  "uri_parse_bytes" which from now on is dependent on
 *		  "uri_parse_buffer".
 *		# "uri_validate_string" and "uri_validate_bytes" were added. Their
 *		  names explain what they do.
 *		# Key word "static" was taken off from the front of function
 *		  "__uri_path_merge" while it is not declared in the .h file. I
 *		  mean to make it a secret interface ^_^
 *
 * Updated: Oct 5 4:43am 2003. version 1.2.2
 *		# Cross the first change log of version 1.2.1!
 *		# "uri_validate_string" is no longer implemented by
 *		  "uri_validate_bytes".
 *
 * Updated: Oct 9 4:49am 2003. version 1.2.3
 *		# Function "uri_rally_parse" no longer exists, because it cannot
 *		  work. The original desire that it can cooperate with other lex
 *		  programs is indeed out of the question.
 *		# Macro URI_INIT changed slightly.
 *
 * Updated: Oct 10 4:28am 2003. version 1.2.4
 *		# The arguments sequence of "rel_xxx" and "base_xxx" of functions
 *		  "__uri_path_merge" and "uri_merge" are exchanged. "rel_xxx" from
 *		  now on precedes "base_xxx".
 *
 * Updated: Oct 11 3:31am 2003. version 1.2.5
 *		# URI merging algorithm is changed again. We check if the path of
 *		  the relative URI is defined before merging path, and if it's not
 *		  defined, we take the base URI's path directly other than merging
 *		  the paths. I thinks that the URI merging algorithm is now at the
 *		  edge of perfect. (The recommended merging algorithm of RFC 2396
 *		  has a lot of problems.)
 *		# The rule of "<REG_NAME>{reg_name}" is optimized, because "strcat"
 *		  is slow.
 *
 * Updated: Oct 12 4:49am 2003. version 1.2.6
 *		# "goto" statements are substituted by "do ... while (0)" and
 *		  "break" in function "uri_merge" and "uri_recombine". No real
 *		  improvement is made but showing off continues.
 *
 * Updated: Oct 12 4:55pm 2003. version 1.2.7
 *		# URIs with chinese character are supported in an obviously
 *		  not perfect way: the "unreserved" BNF allows non-US-ASCII
 *		  characters, i.e., characters with value that ranges from
 *		  128 to 255.
 *
 * Updated: Oct 13 1:06am 2003. version 1.2.8
 *		# Some codes of "__uri_path_merge" was rewritten because the
 *		  interface of stack module changed.
 *		# NOTE!!! NULL can no long be passed to "__uri_path_merge" as
 *		  a relative path or a base path, and the function from now on
 *		  won't set NULL as the result path. If the result path is
 *		  empty, the "abs_path" will point to a pointer that points to
 *		  an empty string ("\0"). So you can always "free(*abs_path)"
 *		  if "__uri_path_merge" succeeded.
 *
 * Updated: Oct 15 1:13am 2003. version 1.4.0
 *		# The update of 1.x.x version continues because version 2.0.x
 *		  runs slower than 1.x.x (half speed), though 2.0.x is written
 *		  in much shorter and very nice codes.
 *		# The new uri structure is derived from 1.3.x. 1.3.x is a test
 *		  version which has been updated into 2.0.0. Indeed, 1.4.x is
 *		  compasible with 2.0.x completely, but 2.0.x is NOT compasible
 *		  with 1.4.x because 2.0.x has only one parsing functions:
 *		  "uri_parse_bytes". I am unwilling to say that 2.0.x is 
 *		  failure because the its codes is very nice. It's slow because
 *		  it scan the string more than once: first time determine the
 *		  length of the URI, and second time break down every components.
 *		  And when breaking down authority it must scan the authority
 *		  again to solve the conflict of server and reg_name. Indeed the
 *		  authority is scanned 3 times.
 *		# "__uri_path_merge", "uri_destroy", "uri_merge", "uri_recombine"
 *		  are taken from 2.0.0 because the uri structure changed.
 *		# "__uri_path_merge" once again supports "no" path. Forget the
 *		  second change log of version 1.2.8.
 *		# Some state names changed.
 *
 * Updated: Oct 15 7:51am 2003. version 1.4.1
 *		# Fix a bug in "__uri_path_merge": when encountered a "../",
 *		  the stack won't pop when the stack top segment is "/". This
 *		  is not right because "/" is not necessarily the root path.
 *		  Now, when encountered a "../", the stack won't pop when and
 *		  only when the stack is empty or the stack has 1 segment and
 *		  it's a "/", i.e., when the following is FALSE:
 *			stack_height(stack) > sizeof (char *) + sizeof (int) ||
 *			!stack_empty(stack) && stack_peep(int, stack) != 1
 *		# Now the parsing process is very neat with "state stack"
 *		  introduced. All URIs will pass all states when being parsed. I
 *		  think it nicer than the 2.0.x parsing process. The performance
 *		  is almost equal to 1.4.0, twice the performance of 2.0.x.
 *		# Some internal macros and functions change. Some parentheses
 *		  are added. From now on "{" followes "do" immediately if the
 *		  corresponding "while" is "while (0)". The aim is to let the
 *		  readers know that indeed it's not an iteration.
 *
 * Updated: Oct 18 11:43am 2003. version 1.4.2
 *		# Fix the bug that port can exist without host. If no host found,
 *		  we should BEGIN REG_NAME instead of BEGIN PORT.
 *		# The statement in the second change log of version 1.4.1 that
 *		  "All URIs will pass all states when being parse." is not the
 *		  truth. Forget it.
 *		# We now check the return value of "yy_scan_xxx" and
 *		  "yy_create_xxx" because there's no garuantee that they would
 *		  not fail. (Only "yy_scan_buffer" may fail practically.)
 *
 * Updated: Oct 18 22:52pm 2003. version 1.4.3
 *		# Function "uri_length_string" and "uri_length_bytes" are added.
 *		  They return the the same value as "uri_parse_string" and
 *		  "uri_parse_bytes", but they do not break down the URI so they
 *		  are faster (hoped). And, from now on "uri_validate_string" and
 *		  "uri_validate_bytes" are implemented by these two function
 *		  respectively.
 *		# State "FINISH" is renamed "ACCEPT", meaning a URI string is
 *		  accepted.
 *
 * Updated: Oct 20 1:28am 2003. version 1.4.4
 *		# New version uses stack 2.2.x, and now we must tell the stack
 *		  what data type we wanna push or pop. A slight mistake in
 *		  "__uri_path_merge" is fixed: pushing or pop a segment name,
 *		  the data type should be "const char *", not "char *".
 *
 * Updated: Oct 20 6:10am 2003. version 1.4.5
 *		# Some code blocks are moved to make the codes look clear.
 *
 * Updated: Oct 23 9:31pm 2003. version 1.4.6
 *		# I forgot to destroy the stack when the result path is "no path".
 *		  Fixed.
 *		# Some names of internal functions and macros are changed to make
 *		  everything simplier. "__uri_path_merge" -> "__path_merge",
 *		  "URI_AUTH_INIT" -> "AUTH_INIT", "__URI_STRDUP" -> "__STRDUP",
 *		  "__URI_AUTH_DUP" -> "__AUTH_DUP".
 *		# Fix the bug that opaque_part cannot have a fragment.
 *		# No longer supports Non-US-ASCII characters in URI string. Because
 *		  the way that we used is imperfect.
 *
 * Updated: Oct 27 4:13am 2003. version 1.5.0
 *		# Hmmmmmmmmm. A better way to support non-ASCII characters is out.
 *		  Function "uri_escape" is added to turn some bytes into a string
 *		  in escaped form, i.e., all non-ASCII characters and excluded
 *		  ASCII characters are turned into %xx. "xx" is the hex value of
 *		  that character.
 *		# Macro uri_isexcl is added for users to examine whether a
 *		  character is an excluded ASCII character. But notice that this
 *		  macro is as dangerous as every "isxxx" macro provided by
 *		  "ctype.h". Make sure that the character's value ranges from
 *		  0 - 127, or your program may crash.
 *
 * Updated: Oct 28 4:49am 2003. version 1.5.1
 *		# Macro "uri_isexcl" is no longer available for users, because it
 *		  is not perfect.
 *		# Two arguments is added to function "uri_escape".
 *		  @const char *ignore: Octets that should be treated as if they
 *		  do not exist;
 *		  @int ignlen: How many octets in argument "ignore". Note that
 *		  "ignore" is NOT necessarily a '\0' terminated string.
 *
 * Updated: Oct 30 7:09am 2003. version 1.5.2
 *		# Function "uri_recombine" is updated by "uri_combine". The latter
 *		  take one more argument: "int flags" indicating which components
 *		  you are interested and want to appear in the URI string. "flags"
 *		  can be any C_xxx macro or their "|" combination.
 *
 * Updated: Nov 4 4:30am 2003. version 1.6.0
 *		# Function "uri_escape" can no longer ignore some characters.
 *		  Arguments "ignore" and "ignlen" are removed.
 *
 * Updated: Nov 4 3:45am 2003. version 1.6.1
 *		# Macro "is_uri_chr" is added for users to check whether a character
 *		  allowed in a URI string.
 *		# Function "uri_escape" is rewriten using macro "is_uri_chr".
 */

/* The followings are BNFs generating URI-refernce, taken from RFC 2396. */
URI-reference	({absoluteURI}|{relativeURI})?("#"{fragment})?
absoluteURI		{scheme}":"({hier_part}|{opaque_part})
relativeURI		({net_path}|{abs_path}|{rel_path})("?"{query})?
hier_part		({net_path}|{abs_path})("?"{query})?
opaque_part		{uric_no_slash}{uric}*
uric_no_slash	{unreserved}|{escaped}|";"|"?"|":"|"@"|"&"|"="|"+"|"$"|","
net_path		"//"{authority}{abs_path}?
abs_path		"/"{path_segments}
rel_path		{rel_segment}{abs_path}?
rel_segment		({unreserved}|{escaped}|";"|"@"|"&"|"="|"+"|"$"|",")+
scheme			{alpha}({alpha}|{digit}|"+"|"-"|".")*
authority		{server}|{reg_name}
reg_name		({unreserved}|{escaped}|"$"|","|";"|":"|"@"|"&"|"="|"+")+
server			(({userinfo}"@")?{hostport})?
userinfo		({unreserved}|{escaped}|";"|":"|"&"|"="|"+"|"$"|",")*
hostport		{host}(":"{port})?
host			{hostname}|{IPv4address}
hostname		({domainlabel}".")*{toplabel}"."?
domainlabel		{alphanum}|{alphanum}({alphanum}|"-")*{alphanum}
toplabel		{alpha}|{alpha}({alphanum}|"-")*{alphanum}
IPv4address		{digit}+"."{digit}+"."{digit}+"."{digit}+
port			{digit}*

path			({abs_path}|{opaque_part})?
path_segments	{segment}("/"{segment})*
segment			{pchar}*(";"{param})*
param			{pchar}*
pchar			{unreserved}|{escaped}|":"|"@"|"&"|"="|"+"|"$"|","

query			{uric}*

fragment		{uric}*

uric			{reserved}|{unreserved}|{escaped}
reserved		";"|"/"|"?"|":"|"@"|"&"|"="|"+"|"$"|","
unreserved		{alphanum}|{mark}
mark			"?"|"-"|"_"|"."|"!"|"~"|"*"|"'"|"("|")"

escaped			"%"{hex}{hex}
hex				{digit}|[A-Fa-f]

alphanum		{alpha}|{digit}
alpha			{lowalpha}|{upalpha}

lowalpha		[a-z]
upalpha			[A-Z]
digit			[0-9]

%option stack

%s SCHEME REL_PATH AUTHORITY USERINFO HOST PORT REG_NAME ABS_PATH
%s OPAQUE_PART QUERY FRAGMENT ACCEPT

%{
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stack.h"
#include "uri.h"

#define URI_INIT(uri) \
do {											\
	(uri)->scheme = NULL;						\
	(uri)->authority = NULL;					\
	(uri)->path = NULL;							\
	(uri)->query = NULL;						\
    (uri)->fragment = NULL;						\
    (uri)->anchor_text = NULL;						\
} while (0)

#define AUTH_INIT(auth, at) \
do {											\
	if (((auth)->type = (at)) == AT_SERVER)		\
	{											\
		(auth)->userinfo = NULL;				\
		(auth)->host = NULL;					\
		(auth)->port = NULL;					\
	}											\
	else										\
		(auth)->reg_name = NULL;				\
} while (0)

#define URI_AUTH_DESTROY(auth) \
do {											\
	if ((auth)->type == AT_SERVER)				\
	{											\
		FREE_NOT_NULL((auth)->userinfo);		\
		FREE_NOT_NULL((auth)->host);			\
		FREE_NOT_NULL((auth)->port);			\
	}											\
	else										\
		FREE_NOT_NULL((auth)->reg_name);		\
} while (0)

static int __length;
static struct uri *__uri;
%}

%%

<SCHEME>{scheme}":"/"/"|{uric_no_slash}		{
	if (__uri->scheme = strdupn(yytext, yyleng - 1))
	{
		__length += yyleng;
		yy_push_state(AUTHORITY);
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<SCHEME>.|\n		{
	yyless(0);
	BEGIN REL_PATH;
}

<SCHEME><<EOF>>		BEGIN REL_PATH;

<REL_PATH>{rel_path}	{
	if (__uri->path = strdupn(yytext, yyleng))
	{
		__length += yyleng;
		BEGIN QUERY;
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<REL_PATH>.|\n		{
	yyless(0);
	yy_push_state(AUTHORITY);
}

<REL_PATH><<EOF>>	yy_push_state(AUTHORITY);

	/* Authority and abs_path have conflict! If the following is "//",
	 * we assume that it's an authority; if the following is "/", it's
	 * an abs_path. */
<AUTHORITY>"//"		{
	yy_pop_state();
	__uri->authority = (struct authority *)malloc(sizeof (struct authority));
	if (__uri->authority)
	{
		AUTH_INIT(__uri->authority, AT_SERVER);
		__length += yyleng;
		BEGIN USERINFO;
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<AUTHORITY>.|\n		{
	yyless(0);
	yy_push_state(ABS_PATH);
}

<AUTHORITY><<EOF>>	yy_push_state(ABS_PATH);

<USERINFO>{userinfo}"@"		{
	if (__uri->authority->userinfo = strdupn(yytext, yyleng - 1))
	{
		__length += yyleng;
		BEGIN HOST;
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<USERINFO>.|\n		{
	yyless(0);
	BEGIN HOST;
}

<USERINFO><<EOF>>	BEGIN HOST;

<HOST>{host}		{
	if (__uri->authority->host = strdupn(yytext, yyleng))
	{
		__length += yyleng;
		BEGIN PORT;
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<HOST>.|\n			{
	yyless(0);
	BEGIN REG_NAME;
}

<HOST><<EOF>>		BEGIN REG_NAME;

<PORT>":"{port}		{
	if (__uri->authority->port = strdupn(yytext + 1, yyleng - 1))
	{
		__length += yyleng;
		BEGIN REG_NAME;
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<PORT>.|\n			{
	yyless(0);
	BEGIN REG_NAME;
}

<PORT><<EOF>>		BEGIN REG_NAME;

<REG_NAME>{reg_name}	{
	/* We have assumed that the authority is a server, but it seems that
	 * we are wrong: it's a reg_name. We should join the userinfo, host
	 * and the port together with this yytext into a reg_name. This case
	 * seldom happens, almost never. */
	char *reg_name, *curpos;
	int len = yyleng;

	if (__uri->authority)
	{
		if (__uri->authority->userinfo)
			len += strlen(__uri->authority->userinfo) + 1;
		if (__uri->authority->host)
			len += strlen(__uri->authority->host);
		if (__uri->authority->port)
			len += strlen(__uri->authority->port) + 1;
	}

	if (reg_name = (char *)malloc((len + 1) * sizeof (char)))
	{
		curpos = reg_name;
		if (__uri->authority)
		{
			if (__uri->authority->userinfo)
			{
				MEMCPY_PLUS(curpos, __uri->authority->userinfo,
							strlen(__uri->authority->userinfo));
				*curpos++ = '@';
			}

			if (__uri->authority->host)
				MEMCPY_PLUS(curpos, __uri->authority->host,
							strlen(__uri->authority->host));

			if (__uri->authority->port)
			{
				*curpos++ = ':';
				MEMCPY_PLUS(curpos, __uri->authority->port,
							strlen(__uri->authority->port));
			}
		}

		MEMCPY_PLUS(curpos, yytext, yyleng);
		*curpos = '\0';

		URI_AUTH_DESTROY(__uri->authority);
		AUTH_INIT(__uri->authority, AT_REG_NAME);
		__uri->authority->reg_name = reg_name;

		__length += yyleng;
		yy_push_state(ABS_PATH);
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<REG_NAME>.|\n		{
	yyless(0);
	yy_push_state(ABS_PATH);
}

<REG_NAME><<EOF>>	yy_push_state(ABS_PATH);

<ABS_PATH>{abs_path}	{
	yy_pop_state();
	if (YY_START == AUTHORITY)
		yy_pop_state();
	if (__uri->path = strdupn(yytext, yyleng))
	{
		__length += yyleng;
		BEGIN QUERY;
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<ABS_PATH>.|\n		|
<ABS_PATH><<EOF>>	{
	/* When encountered an EOF we can not yyless. */
	if (yyleng == 1)
		yyless(0);
	yy_pop_state();

	/* The previous state is "AUTHORITY" indicates the URI
	 * has NO authority. */
	if (YY_START == AUTHORITY)
	{
		yy_pop_state();
		/* The previous state is "SCHEME" indicates the URI
		 * HAS a scheme. It's a little confusing. */
		if (YY_START == SCHEME)
			BEGIN OPAQUE_PART;
		else
			BEGIN FRAGMENT;
	}
	else
		BEGIN QUERY;
}

<OPAQUE_PART>{opaque_part}	{
	if (__uri->path = strdupn(yytext, yyleng))
	{
		__length += yyleng;
		BEGIN FRAGMENT;
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<OPAQUE_PART>.|\n		{
	yyless(0);
	BEGIN FRAGMENT;
}

<OPAQUE_PART><<EOF>>	BEGIN FRAGMENT;

<QUERY>"?"{query}	{
	if (__uri->query = strdupn(yytext + 1, yyleng - 1))
	{
		__length += yyleng;
		BEGIN FRAGMENT;
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<QUERY>.|\n			{
	yyless(0);
	BEGIN FRAGMENT;
}

<QUERY><<EOF>>		BEGIN FRAGMENT;

<FRAGMENT>"#"{fragment}		{
	if (__uri->fragment = strdupn(yytext + 1, yyleng - 1))
	{
		__length += yyleng;
		BEGIN ACCEPT;
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<FRAGMENT>.|\n		{
	yyless(0);
	BEGIN ACCEPT;
}

<FRAGMENT><<EOF>>	BEGIN ACCEPT;

<ACCEPT>.|\n		{
	yyless(0);
	return __length;
}

<ACCEPT><<EOF>>		return __length;

<INITIAL>{URI-reference}	return yyleng;

<INITIAL>.|\n		{
	yyless(0);
	return 0;
}

<INITIAL><<EOF>>	return 0;

%%

int yywrap(void)
{
	return 1;
}

char __uri_chr[] =
{
	0x00, 0x00, 0x00, 0x00,
	0x5b, 0xff, 0xff, 0xf5,
	0xff, 0xff, 0xff, 0xe1,
	0x7f, 0xff, 0xff, 0xe2
};

static int __uri_parse(struct uri *uri)
{
	__uri = uri;
	__length = 0;
	URI_INIT(__uri);
	BEGIN SCHEME;
	return yylex();
}

/* Scan a file and return the length of the uri. Return negative number
 * when and only when failed to allocate memory. */
int uri_parse_file(FILE *file, struct uri *uri)
{
	YY_BUFFER_STATE buf;
	int n = -1;

	yyin = file;
	if (buf = yy_create_buffer(yyin, YY_BUF_SIZE))
	{
		yy_switch_to_buffer(buf);
		n = __uri_parse(uri);
		yy_delete_buffer(buf);
	}

	return n;
}

/* Scan a string ('\0' terminated). */
int uri_parse_string(const char *string, struct uri *uri)
{
	YY_BUFFER_STATE buf;
	int n = -1;

	if (buf = yy_scan_string(string))
	{
		yy_switch_to_buffer(buf);
		n = __uri_parse(uri);
		yy_delete_buffer(buf);
	}

	return n;
}

/* Scan some memory bytes. */
int uri_parse_bytes(const char *bytes, int len, struct uri *uri)
{
	YY_BUFFER_STATE buf;
	int n = -1;

	if (buf = yy_scan_bytes(bytes, len))
	{
		yy_switch_to_buffer(buf);
		n = __uri_parse(uri);
		yy_delete_buffer(buf);
	}

	return n;
}

/* Scan some memory bytes. The last two bytes of the memory MUST be '\0', or
 * the function will return -1 indicating a failure. This function has better
 * performance than "uri_parse_bytes", but note there is NO "const" key
 * word before the "base" argument, which means the content of memory may
 * be changed. */
int uri_parse_buffer(char *base, unsigned int size, struct uri *uri)
{
	YY_BUFFER_STATE buf;
	int n = -1;

	if (buf = yy_scan_buffer(base, size))
	{
		yy_switch_to_buffer(buf);
		n = __uri_parse(uri);
		yy_delete_buffer(buf);
	}

	return n;
}

void uri_destroy(struct uri *uri)
{
	FREE_NOT_NULL(uri->scheme);
	if (uri->authority)
	{
		URI_AUTH_DESTROY(uri->authority);
		free(uri->authority);
	}
	FREE_NOT_NULL(uri->path);
	FREE_NOT_NULL(uri->query);
	FREE_NOT_NULL(uri->fragment);
	FREE_NOT_NULL(uri->anchor_text);
    //FREE_NOT_NULL(uri->super_connect);
}

int __uri_length(void)
{
	BEGIN INITIAL;
	return yylex();
}

int uri_length_string(const char *string)
{
	YY_BUFFER_STATE buf;
	int n = -1;

	if (buf = yy_scan_string(string))
	{
		yy_switch_to_buffer(buf);
		n = __uri_length();
		yy_delete_buffer(buf);
	}

	return n;
}

int uri_length_bytes(const char *bytes, int len)
{
	YY_BUFFER_STATE buf;
	int n = -1;

	if (buf = yy_scan_bytes(bytes, len))
	{
		yy_switch_to_buffer(buf);
		n = __uri_length();
		yy_delete_buffer(buf);
	}

	return n;
}

/* Validate a string ('\0' terminated). */
int uri_validate_string(const char *string)
{
	int n = uri_length_string(string);
	return n >= 0 ? n == strlen(string) : n;
}

/* Validate some memory bytes. */
int uri_validate_bytes(const char *bytes, int len)
{
	int n = uri_length_bytes(bytes, len);
	return n >= 0 ? n == len : n;
}

/* Merge two path. It sounds easy but indeed quite troublesome if you take
 * everything into consideration. Core of merging two URIs. The function will
 * allocate memory for you, which is NOT a very good programming style you
 * should not follow. */
int __path_merge(const char *rel_path, const char *base_path, char **result)
{
	int len;
	stack_t *stack;
	const char *curpos;
	int seglen;
	const char *next_slash;
    char * base_path_tmp = NULL;
    int i;

	/* This merging algorithm is different from RFC 2396, which uses string,
	 * while this algorithm uses stack. */
	if (!(stack = stack_create(STACK_INITIAL_SIZE)))
		return -1;

	/* The "base_path" and the "rel_path" are divided into segments and push
	 * all these segments and their length into the stack. If a segment
	 * is ".", ignore it; if a segment is "..", pop one segment out. */
	len = 0;
	for (i = 0; i < 2; i++)
	{
		/* Both "rel_path" and "base_path" can be NULL. */
		if (curpos = base_path)
		{
            if ((i==0) && (next_slash = strchr(curpos, '?')))
            {
                int lentmp = strlen(curpos) ;
                base_path_tmp = (char *) malloc ((lentmp +1 ) * sizeof (char));
                strncpy(base_path_tmp , curpos , lentmp);
                base_path_tmp[lentmp] = '\0';
                if (next_slash = strchr(base_path_tmp, '?'))
                {
                    base_path_tmp[next_slash - base_path_tmp] = '\0';
                }
                curpos = base_path_tmp;
            }
			while (next_slash = strchr(curpos, '/'))
			{
				if (strncmp(curpos, "../", next_slash - curpos + 1) == 0)
				{
					if (stack_height(stack) > sizeof (char *) + sizeof (int) ||
						!stack_empty(stack) && stack_top(int, stack) != 1)
					{
						len -= stack_pop(int, stack);
						stack_pop(const char *, stack);
					}
				}
				else if (strncmp(curpos, "./", next_slash - curpos + 1) != 0)
				{
					len += next_slash - curpos + 1;
					if (stack_push(curpos, const char *, stack) < 0 ||
						stack_push(next_slash - curpos + 1, int, stack) < 0)
					{
						stack_destroy(stack);
                        if ( base_path_tmp ) free ( base_path_tmp);
		                base_path_tmp = NULL;
						return -1;
					}
				}

				curpos = next_slash + 1;
			}
            if (i == 0 && rel_path[0] == '?')
            {
                if (next_slash = strchr(curpos, '?'))
                {
                    len += next_slash - curpos;
                    if (stack_push(curpos, const char *, stack) < 0 ||
                        stack_push(next_slash - curpos, int, stack) < 0)
                    {
                        stack_destroy(stack);
                        if ( base_path_tmp ) free ( base_path_tmp);
		                base_path_tmp = NULL;
                        return -1;
                    }
                }else
                {

                    len += strlen(curpos);
                    if (stack_push(curpos, const char *, stack) < 0 ||
                        stack_push(strlen(curpos), int, stack) < 0)
                    {
                        stack_destroy(stack);
                        if ( base_path_tmp ) free ( base_path_tmp);
		                base_path_tmp = NULL;
                        return -1;
                    }
                }
            }
            base_path = rel_path;
        }
	}

	/* This part deals with the "filename", which may be empty, may be "..",
	 * may be ".", and may be something else like "index.html". */
	if (curpos)
	{
		if (strcmp(curpos, "..") == 0)
		{
			if (stack_height(stack) > sizeof (char *) + sizeof (int) ||
				!stack_empty(stack) && stack_top(int, stack) != 1)
			{
				len -= stack_pop(int, stack);
				stack_pop(const char *, stack);
			}
		}
		else if (strcmp(curpos, ".") != 0)
		{
			len += strlen(curpos);
			if (stack_push(curpos, const char *, stack) < 0 ||
				stack_push(strlen(curpos), int, stack) < 0)
			{
				stack_destroy(stack);
                if ( base_path_tmp ) free ( base_path_tmp);
		        base_path_tmp = NULL;
				return -1;
			}
		}
	}

	/* Example:
	 * rel_path: "../././../game/../document/rfc/rfc2616.pdf"
	 * base_path: "/pub/incoming/./software/linux/nasm.tar.gz",
	 * Now the stack is:
	 *
	 *	+---------------+		<-- stack top
	 *	|	11			|
	 *	+---------------|
	 *	|	rfc2616.pdf	|
	 *	+---------------|
	 *	|	4			|
	 *	|---------------|
	 *	|	rfc/		|
	 *	|---------------|
	 *	|	9			|
	 *	|---------------|
	 *	|	document/	|
	 *	|---------------|
	 *	|	9			|
	 *	|---------------|
	 *	|	incoming/	|
	 *	|---------------|
	 *	|	4			|
	 *	|---------------|
	 *	|	pub/		|
	 *	|---------------|
	 *	|	1			|
	 *	|---------------|
	 *	|	/			|
	 *	+---------------+		<-- stack base
	 *
	 * len = 1 + 4 + 9 + 9 + 4 + 11 = ??
	 *
	 * Note that we do NOT copy the segments into the stack, we just push the
	 * pointers into the stack.
	 *
	 * All the information we need to compose the result path has been here.
	 */

	/* The result path is an "empty path". We should turn it into "no path".
	 * "no path" is allowed while "empty path" is illegal. */
	if (len == 0)
		*result = NULL;
	else if (*result = (char *)malloc((len + 1) * sizeof (char)))
	{
		*result = *result + len;
		**result = '\0';
		while (!stack_empty(stack))
		{
			seglen = stack_pop(int, stack);
			*result -= seglen;
			memcpy(*result, stack_pop(const char *, stack), seglen);
		}
	}
	else
		len = -1;

    if ( base_path_tmp ) free ( base_path_tmp);
    base_path_tmp = NULL;
	stack_destroy(stack);
	return len;
}

int uri_merge(const struct uri *rel_uri, const struct uri *base_uri,
			  struct uri *result)
{
	int len, n;

	/* I am lazy. */
	#define __STRDUP(str) \
	({																	\
		int __n;														\
		char *__res;													\
		if (str)														\
		{																\
			__n = strlen(str);											\
			if (__res = strdupn(str, __n))								\
				len += __n;												\
			else														\
				break;													\
		}																\
		else															\
			__res = NULL;												\
		__res;															\
	})

	/* The following macro is sooooooo big but it's only extended twice
	 * and does not matter much. */
	#define __AUTH_DUP(auth) \
	({																	\
		struct authority *__res;										\
		if (auth)														\
		{																\
			if (__res = (struct authority *)							\
						malloc(sizeof (struct authority)))				\
			{															\
				AUTH_INIT(__res, (auth)->type);							\
				if ((auth)->type == AT_SERVER)							\
				{														\
					if (__res->userinfo = __STRDUP((auth)->userinfo))	\
						len++;											\
					__res->host = __STRDUP((auth)->host);				\
					if (__res->port = __STRDUP((auth)->port))			\
						len++;											\
				}														\
				else													\
					__res->reg_name = __STRDUP((auth)->reg_name);		\
				len += 2;												\
			}															\
			else														\
				break;													\
		}																\
		else															\
			__res = NULL;												\
		__res;															\
	})

	URI_INIT(result);
	len = 0;
	do {
		/* If the relative URI has a scheme, take it; else take the scheme
		 * of the base URI. */
		if (rel_uri->scheme)
		{
			result->scheme = __STRDUP(rel_uri->scheme);
			len++;
		}
		else if (result->scheme = __STRDUP(base_uri->scheme))
			len++;

		/* If the relative URI has a scheme or an authority, take it's
		 * authority; else take the authority of the base URI. */
		if (rel_uri->scheme || rel_uri->authority)
			result->authority = __AUTH_DUP(rel_uri->authority);
		else
			result->authority = __AUTH_DUP(base_uri->authority);

		/* If the relative URI has a scheme or an authority or an absolute
		 * path, take it's path; else if the relative URI does not have a
		 * path, take the base URI's path; else if base URI has a path,
		 * merge the relative URI's path with the base URI's path, and take
		 * the result; else if the base URI has no path, merge the relative
		 * URI's path with path "/" and take the result; no else. */
		if (rel_uri->scheme || rel_uri->authority ||
									rel_uri->path && *rel_uri->path == '/')
			result->path = __STRDUP(rel_uri->path);
		else if (!rel_uri->path)
			result->path = __STRDUP(base_uri->path);
		else if ((n = __path_merge(rel_uri->path, base_uri->path ?
								   base_uri->path : "/", &result->path)) >= 0)
			len += n;
		else
			break;

		/* Query is taken from relative URI. */
		if (result->query = __STRDUP(rel_uri->query))
			len++;

		/* Fragment is taken from relative URI. */
		if (result->fragment = __STRDUP(rel_uri->fragment))
			len++;

		return len;
	} while (0);

	#undef __AUTH_DUP
	#undef __STRDUP
	uri_destroy(result);
	return -1;
}

/* Recombine a URI structure into a URI string. "flags" indicates what
 * component(s) would you like to appear in the result string. Note that
 * the result string is NOT necessarily a legal URI string (When you mask
 * some components) though the second argument has the name "uristr". */
int uri_combine(const struct uri *uri, char *uristr, unsigned int n, int flags)
{
	char *curpos = uristr;
	char *end = curpos + n;

	do {
		if (flags & C_SCHEME && uri->scheme)
		{
			n = strlen(uri->scheme);
			if (curpos + n + 1 < end)
			{
				MEMCPY_PLUS(curpos, uri->scheme, n);
				*curpos++ = ':';
			}
			else
				break;
		}

		if (flags & C_AUTHORITY && uri->authority)
		{
			if (curpos + 2 < end)
			{
				*curpos++ = '/';
				*curpos++ = '/';
			}
			else
				break;

			if (uri->authority->type == AT_SERVER)
			{
				if (flags & C_USERINFO && uri->authority->userinfo)
				{
					n = strlen(uri->authority->userinfo);
					if (curpos + n + 1 < end)
					{
						MEMCPY_PLUS(curpos, uri->authority->userinfo, n);
						*curpos++ = '@';
					}
					else
						break;
				}

				if (flags & C_HOST && uri->authority->host)
				{
					n = strlen(uri->authority->host);
					if (curpos + n < end)
						MEMCPY_PLUS(curpos, uri->authority->host, n);
					else
						break;
				}

				if (flags & C_PORT && uri->authority->port)
				{
					n = strlen(uri->authority->port);
					if (curpos + n + 1 < end)
					{
						*curpos++ = ':';
						MEMCPY_PLUS(curpos, uri->authority->port, n);
					}
					else
						break;
				}
			}
			else if (flags & C_REG_NAME && uri->authority->reg_name)
			{
				n = strlen(uri->authority->reg_name);
				if (curpos + n < end)
					MEMCPY_PLUS(curpos, uri->authority->reg_name, n);
				else
					break;
			}
		}

		if (flags & C_PATH && uri->path)
		{
			n = strlen(uri->path);
			if (curpos + n < end)
				MEMCPY_PLUS(curpos, uri->path, n);
			else
				break;
		}

		if (flags & C_QUERY && uri->query)
		{
			n = strlen(uri->query);
			if (curpos + n + 1 < end)
			{
				*curpos++ = '?';
				MEMCPY_PLUS(curpos, uri->query, n);
			}
			else
				break;
		}

		if (flags & C_FRAGMENT && uri->fragment)
		{
			n = strlen(uri->fragment);
			if (curpos + n + 1 < end)
			{
				*curpos++ = '#';
				MEMCPY_PLUS(curpos, uri->fragment, n);
			}
			else
				break;
		}

		if (curpos < end)
			*curpos = '\0';
		else
			break;

		return curpos - uristr;
	} while (0);

	errno = ENOSPC;
	return -1;
}

/* Turn some bytes into a string of escaped form. */
int uri_escape(const char *bytes, int len, char *escstr, unsigned int n)
{
	const char *tmp = bytes + len;
	char *curpos = escstr;
	char *end = escstr + n;

	while (1)
	{
		if (bytes == tmp)
		{
			if (curpos < end)
			{
				*curpos = '\0';
				return curpos - escstr;
			}
			else
				break;
		}

		if (is_uri_chr(*bytes) && curpos < end)
			*curpos++ = *bytes;
		else if (curpos + 2 < end)
		{
			sprintf(curpos, "%%%X%X",
					(unsigned char)*bytes >> 4, *bytes & 0x0f);
			curpos += 3;
		}
		else
			break;

		bytes++;
	}

	errno = ENOSPC;
	return -1;
}


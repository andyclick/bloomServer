#ifndef _URI_H_
#define _URI_H_

#include <stdio.h>
#include "list.h"
#include "util.h"

#define AT_SERVER		0
#define AT_REG_NAME		1

#define C_SCHEME		0001
#define C_AUTHORITY		0036
#define C_USERINFO		0002
#define C_HOST			0004
#define C_PORT			0010
#define C_REG_NAME		0020
#define C_PATH			0040
#define C_QUERY			0100
#define C_FRAGMENT		0200
#define C_URI			0377

struct authority
{
	int type;
	union
	{
		struct
		{
			char *userinfo;
			char *host;
			char *port;
		};
		char *reg_name;
	};
};

struct uri
{
	struct list_head list;
	char *scheme;
	char *path;
	struct authority *authority;
	char *query;
	char *fragment;
    char *anchor_text;
};

extern char __uri_chr[];

#ifdef __cplusplus
extern "C"
{
#endif

int uri_parse_file(FILE *file, struct uri *uri);
int uri_parse_string(const char *string, struct uri *uri);
int uri_parse_bytes(const char *bytes, int len, struct uri *uri);
int uri_parse_buffer(char *buffer, unsigned int size, struct uri *uri);
void uri_destroy(struct uri *uri);
int uri_length_string(const char *string);
int uri_length_bytes(const char *bytes, int len);
int uri_validate_string(const char *string);
int uri_validate_bytes(const char *bytes, int len);
int uri_merge(const struct uri *rel_uri, const struct uri *base_uri,
			  struct uri *result);
int uri_combine(const struct uri *uri, char *uristr, unsigned int n,
				int flags);
int uri_escape(const char *bytes, int len, char *escstr, unsigned int n);
#define is_uri_chr(c) \
({																\
	unsigned char tmp = (c);									\
	tmp < 128 && __uri_chr[tmp >> 3] & 0x80 >> (tmp & 0x07);	\
})

#ifdef __cplusplus
}
#endif

#endif


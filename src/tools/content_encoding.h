/***************************************************************************
 *                                  _   _ ____  _     
 *  Project                     ___| | | |  _ \| |    
 *                             / __| | | | |_) | |    
 *                            | (__| |_| |  _ <| |___ 
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2003, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 * 
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * $Id: content_encoding.h,v 1.6 2003/05/12 12:45:14 bagder Exp $
 ***************************************************************************/
#include "util.h"
#include "zlib.h"

typedef enum {
  GZIP_OK,
  GZIP_BAD,
  GZIP_UNDERFLOW
} GZIPNUM;

typedef enum {
  CURLE_OK = 0,
  CURLE_UNSUPPORTED_PROTOCOL,    /* 1 */
  CURLE_FAILED_INIT,             /* 2 */
  CURLE_URL_MALFORMAT,           /* 3 */
  CURLE_URL_MALFORMAT_USER,      /* 4 */
  CURLE_COULDNT_RESOLVE_PROXY,   /* 5 */
  CURLE_COULDNT_RESOLVE_HOST,    /* 6 */
  CURLE_COULDNT_CONNECT,         /* 7 */
  CURLE_FTP_WEIRD_SERVER_REPLY,  /* 8 */
  CURLE_FTP_ACCESS_DENIED,       /* 9 */
  CURLE_FTP_USER_PASSWORD_INCORRECT, /* 10 */
  CURLE_FTP_WEIRD_PASS_REPLY,    /* 11 */
  CURLE_FTP_WEIRD_USER_REPLY,    /* 12 */
  CURLE_FTP_WEIRD_PASV_REPLY,    /* 13 */
  CURLE_FTP_WEIRD_227_FORMAT,    /* 14 */
  CURLE_FTP_CANT_GET_HOST,       /* 15 */
  CURLE_FTP_CANT_RECONNECT,      /* 16 */
  CURLE_FTP_COULDNT_SET_BINARY,  /* 17 */
  CURLE_PARTIAL_FILE,            /* 18 */
  CURLE_FTP_COULDNT_RETR_FILE,   /* 19 */
  CURLE_FTP_WRITE_ERROR,         /* 20 */
  CURLE_FTP_QUOTE_ERROR,         /* 21 */
  CURLE_HTTP_RETURNED_ERROR,     /* 22 */
  CURLE_WRITE_ERROR,             /* 23 */
  CURLE_MALFORMAT_USER,          /* 24 - user name is illegally specified */
  CURLE_FTP_COULDNT_STOR_FILE,   /* 25 - failed FTP upload */
  CURLE_READ_ERROR,              /* 26 - could open/read from file */
  CURLE_OUT_OF_MEMORY,           /* 27 */
  CURLE_OPERATION_TIMEOUTED,     /* 28 - the timeout time was reached */
  CURLE_FTP_COULDNT_SET_ASCII,   /* 29 - TYPE A failed */
  CURLE_FTP_PORT_FAILED,         /* 30 - FTP PORT operation failed */
  CURLE_FTP_COULDNT_USE_REST,    /* 31 - the REST command failed */
  CURLE_FTP_COULDNT_GET_SIZE,    /* 32 - the SIZE command failed */
  CURLE_HTTP_RANGE_ERROR,        /* 33 - RANGE "command" didn't work */
  CURLE_HTTP_POST_ERROR,         /* 34 */
  CURLE_SSL_CONNECT_ERROR,       /* 35 - wrong when connecting with SSL */
  CURLE_BAD_DOWNLOAD_RESUME,     /* 36 - couldn't resume download */
  CURLE_FILE_COULDNT_READ_FILE,  /* 37 */
  CURLE_LDAP_CANNOT_BIND,        /* 38 */
  CURLE_LDAP_SEARCH_FAILED,      /* 39 */
  CURLE_LIBRARY_NOT_FOUND,       /* 40 */
  CURLE_FUNCTION_NOT_FOUND,      /* 41 */
  CURLE_ABORTED_BY_CALLBACK,     /* 42 */
  CURLE_BAD_FUNCTION_ARGUMENT,   /* 43 */
  CURLE_BAD_CALLING_ORDER,       /* 44 */
  CURLE_HTTP_PORT_FAILED,        /* 45 - HTTP Interface operation failed */
  CURLE_BAD_PASSWORD_ENTERED,    /* 46 - my_getpass() returns fail */
  CURLE_TOO_MANY_REDIRECTS ,     /* 47 - catch endless re-direct loops */
  CURLE_UNKNOWN_TELNET_OPTION,   /* 48 - User specified an unknown option */
  CURLE_TELNET_OPTION_SYNTAX ,   /* 49 - Malformed telnet option */
  CURLE_OBSOLETE,            /* 50 - removed after 7.7.3 */
  CURLE_SSL_PEER_CERTIFICATE,    /* 51 - peer's certificate wasn't ok */
  CURLE_GOT_NOTHING,             /* 52 - when this is a specific error */
  CURLE_SSL_ENGINE_NOTFOUND,     /* 53 - SSL crypto engine not found */
  CURLE_SSL_ENGINE_SETFAILED,    /* 54 - can not set SSL crypto engine as
                                    default */
  CURLE_SEND_ERROR,              /* 55 - failed sending network data */
  CURLE_RECV_ERROR,              /* 56 - failure in receiving network data */
  CURLE_SHARE_IN_USE,            /* 57 - share is in use */
  CURLE_SSL_CERTPROBLEM,         /* 58 - problem with the local certificate */
  CURLE_SSL_CIPHER,              /* 59 - couldn't use specified cipher */
  CURLE_SSL_CACERT,              /* 60 - problem with the CA cert (path?) */
  CURLE_BAD_CONTENT_ENCODING,    /* 61 - Unrecognized transfer encoding */

  CURL_LAST /* never use! */
} CURLcode;

/*
 * Comma-separated list all supported Content-Encodings ('identity' is implied)
 *
 */

#define ALL_CONTENT_ENCODINGS "deflate, gzip"


CURLcode deflate_write(char *data, z_stream *k, size_t nread, Buffer *buffer);

CURLcode gzip_write(char *data, z_stream *z, size_t nread, int & zlib_init,  Buffer *buffer);


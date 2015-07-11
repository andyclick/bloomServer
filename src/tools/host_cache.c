//static char rcsid[] = "host_cache.c,v 1.20 1996/01/05 20:28:34 duane Exp";
/*
 *  host_cache.c - An IP/DNS cache to avoid frequent gethostbyname() calls
 * 
 *  DEBUG: section  86, level 1         Common utilities DNS host cache
 *
 *  Duane Wessels, wessels@cs.colorado.edu,  March 1995
 *
 *  ----------------------------------------------------------------------
 *  Copyright (c) 1994, 1995.  All rights reserved.
 *  
 *    The Harvest software was developed by the Internet Research Task
 *    Force Research Group on Resource Discovery (IRTF-RD):
 *  
 *          Mic Bowman of Transarc Corporation.
 *          Peter Danzig of the University of Southern California.
 *          Darren R. Hardy of the University of Colorado at Boulder.
 *          Udi Manber of the University of Arizona.
 *          Michael F. Schwartz of the University of Colorado at Boulder.
 *          Duane Wessels of the University of Colorado at Boulder.
 *  
 *    This copyright notice applies to software in the Harvest
 *    ``src/'' directory only.  Users should consult the individual
 *    copyright notices in the ``components/'' subdirectories for
 *    copyright information about other software bundled with the
 *    Harvest source code distribution.
 *  
 *  TERMS OF USE
 *    
 *    The Harvest software may be used and re-distributed without
 *    charge, provided that the software origin and research team are
 *    cited in any use of the system.  Most commonly this is
 *    accomplished by including a link to the Harvest Home Page
 *    (http://harvest.cs.colorado.edu/) from the query page of any
 *    Broker you deploy, as well as in the query result pages.  These
 *    links are generated automatically by the standard Broker
 *    software distribution.
 *    
 *    The Harvest software is provided ``as is'', without express or
 *    implied warranty, and with no support nor obligation to assist
 *    in its use, correction, modification or enhancement.  We assume
 *    no liability with respect to the infringement of copyrights,
 *    trade secrets, or any patents, and are not responsible for
 *    consequential damages.  Proper use of the Harvest software is
 *    entirely the responsibility of the user.
 *  
 *  DERIVATIVE WORKS
 *  
 *    Users may make derivative works from the Harvest software, subject 
 *    to the following constraints:
 *  
 *      - You must include the above copyright notice and these 
 *        accompanying paragraphs in all forms of derivative works, 
 *        and any documentation and other materials related to such 
 *        distribution and use acknowledge that the software was 
 *        developed at the above institutions.
 *  
 *      - You must notify IRTF-RD regarding your distribution of 
 *        the derivative work.
 *  
 *      - You must clearly notify users that your are distributing 
 *        a modified version and not the original Harvest software.
 *  
 *      - Any derivative product is also subject to these copyright 
 *        and use restrictions.
 *  
 *    Note that the Harvest software is NOT in the public domain.  We
 *    retain copyright, as specified above.
 *  
 *  HISTORY OF FREE SOFTWARE STATUS
 *  
 *    Originally we required sites to license the software in cases
 *    where they were going to build commercial products/services
 *    around Harvest.  In June 1995 we changed this policy.  We now
 *    allow people to use the core Harvest software (the code found in
 *    the Harvest ``src/'' directory) for free.  We made this change
 *    in the interest of encouraging the widest possible deployment of
 *    the technology.  The Harvest software is really a reference
 *    implementation of a set of protocols and formats, some of which
 *    we intend to standardize.  We encourage commercial
 *    re-implementations of code complying to this set of standards.  
 *  
 */

#include <memory.h>
#include <ctype.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/* According to POSIX 1003.1-2001 */
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "util.h"

#define HASHTABLE_N 1000
#define HASHTABLE_M 9

static Host HostTable[HASHTABLE_N];
static pthread_mutex_t init_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t get_mutex = PTHREAD_MUTEX_INITIALIZER;

static int hash_index(buf)
     char *buf;
{
    static int n = HASHTABLE_N;
    static int m = HASHTABLE_M;
    register int val = 0;
    register char *s = NULL;

    for (s = buf; *s; s++)
	val += (int) (*s * m);
    val %= n;
    return val;
    /* 
    unsigned int n=0; 
    char* b=(char*)&n; 
    size_t i=0;
    for (;i<strlen(buf);i++) 
        b[i%4]^=buf[i]; 
    return n%HASHTABLE_N; 
    */
}

int host_cache_init ();
static Host *new_host (char *hostname);
static void Tolower (char *);
void dump_host_cache (int, int);
static int cache_inited = 0;

/* ========== PUBLIC FUNCTIONS ============================================= */

int host_cache_init()
{
    memset(HostTable, '\0', HASHTABLE_N * sizeof(Host));
    cache_inited = 1;
    char *h = getfullhostname();
    if (!get_host(h)) {
        //Log("Can't get my own host info!?\n");
        //exit(1);
    }
    //Debug(86, 1, ("host_cache: initialized\n"));
    return 1;
}

Host *get_host(hostname)
     char *hostname;
{
    char hn[MAXHOSTNAMELEN];
    Host *h = 0;
    int idx;
    time_t now = time(0);

    if (hostname == (char *) 0)
	return 0;

    //Debug(86, 1, ("host_cache: get_host (%s)\n", hostname));

    if (!cache_inited) {
        pthread_mutex_lock(&init_mutex);
        if (!cache_inited) {
            host_cache_init();
        }
        pthread_mutex_unlock(&init_mutex);
    }

    strncpy(hn, hostname, MAXHOSTNAMELEN - 1);
    hn[MAXHOSTNAMELEN - 1] = 0;
    Tolower(hn);

    idx = hash_index(hn);
    //Debug(86, 1, ("host_cache: hash index = %d\n", idx));

    pthread_mutex_lock(&get_mutex);
    if (!strcmp(HostTable[idx].key, hn)) {
        h = &HostTable[idx];
    }

    if (!h) {
        pthread_mutex_unlock(&get_mutex);
        h = new_host(hostname);
    }
    if (!h) {
        pthread_mutex_unlock(&get_mutex);
        return 0;
    }

    h->n++;
    h->last_t = now;
    Host *ret = copy_host(h);
    pthread_mutex_unlock(&get_mutex);

    return ret;
}

static Host *new_host(hostname)
     char *hostname;
{
    Host *h = NULL;
    char *hn = NULL;
    struct hostent *H = NULL;
    struct in_addr ina;
    unsigned long ip;
    int idx;
    char x[64];

    //Debug(86, 1, ("new_host: Adding %s\n", hostname));
    hn = strdup(hostname);
    if (strlen(hn) > (MAXHOSTNAMELEN - 1))
	*(hn + MAXHOSTNAMELEN - 1) = 0;
    Tolower(hn);

    idx = hash_index(hn);
    h = &HostTable[idx];

    if (sscanf(hn, "%[0-9].%[0-9].%[0-9].%[0-9]%s", x, x, x, x, x) == 4) {
        ip = inet_addr(hn);
        //Debug(86, 1, ("new_host: numeric address %s, trying gethostbyaddr()\n", hn));
        H = gethostbyaddr((char *) &ip, 4, AF_INET);
        if (!H) {		/* special hack for DNS's which don't work */
            /* unknown if this works                   */
            //Debug(86, 1, ("new_host: gethostbyaddr() failed.  Trying hack.\n"));
            pthread_mutex_lock(&get_mutex);
            memset(h, '\0', sizeof(Host));
            strncpy(h->key, hn, MAXHOSTNAMELEN - 1);
            strncpy(h->fqdn, hn, MAXHOSTNAMELEN - 1);
            memcpy(h->ipaddr, &ip, h->addrlen = 4);
            strcpy(h->dotaddr, hn);
            free(hn);
            return h;
        }
    } else {

        int ret = 0; 
        int my_err = 1; 
        char buf[8192]; 
        struct hostent host_ent, *host_ent_result = NULL; 
        ret = gethostbyname_r(hn, &host_ent, buf, 8192, &host_ent_result, &my_err); 
        H = host_ent_result;
        //H = gethostbyname(hn);

        if (ret != 0 || host_ent_result == NULL) {
           //Log("INFO: gethostbyname(%s) failed.\n", hn);
           free(hn);
           return 0;
        }
    }

    if (H == (struct hostent *) NULL) {
        ///Debug(86, 1, ("new_host: %s: unknown host\n", hn));
        free(hn);
        return 0;
    }
    pthread_mutex_lock(&get_mutex);
    memset(h, '\0', sizeof(Host));
    strncpy(h->key, hn, MAXHOSTNAMELEN - 1);
    strncpy(h->fqdn, H->h_name, MAXHOSTNAMELEN - 1);
    Tolower(h->fqdn);
    memcpy(h->ipaddr, *H->h_addr_list, h->addrlen = 4);
    memcpy(&ina.s_addr, *H->h_addr_list, 4);
    strcpy(h->dotaddr, inet_ntoa(ina));

    //Debug(86, 1, ("new_host: successfully added host %s\n", h->key));
    //Debug(86, 1, ("new_host:   FQDN=%s\n", h->fqdn));
    //Debug(86, 1, ("new_host:     IP=%s\n", h->dotaddr));

    free(hn);
    return h;
}

void dump_host_cache(d_sec, d_lvl)
     int d_sec;
     int d_lvl;
{
    int i;
    Host *h = NULL;

    //Debug(d_sec, d_lvl, ("HostTable:\n"));
    for (i = 0; i < HASHTABLE_N; i++) {
	h = &HostTable[i];
	if (*h->fqdn) {
	    ///Debug(d_sec, d_lvl, ("key: %-30s = [%s] %s\n",h->key, h->dotaddr, h->fqdn));
	}
    }
}

Host *copy_host(Host *src) {
   Host *des = (Host *)malloc(sizeof(Host));
   memcpy(des, src, sizeof(Host)); 
   return des;
}

/* ========== MISC UTIL FUNCS ============================================== */

static void Tolower(q)
     char *q;
{
    char *s = q;
    while (*s) {
        *s = tolower((unsigned char) *s);
        s++;
    }
}


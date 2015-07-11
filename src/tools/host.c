//static char rcsid[] = "host.c,v 1.26 1996/01/05 20:28:33 duane Exp";
/*
 *  host.c - Retrieves full DNS name of the current host
 *
 *  DEBUG: section  84, level 1         Common utilities hostname processing
 *
 *  Darren Hardy, hardy@cs.colorado.edu, April 1994
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "util.h"

/* Make sure this is defined, some systems don't have this in netdb.h */
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN  254
#endif

/*
 *  getfullhostname() - Returns the fully qualified name of the current 
 *  host, or NULL on error.  Pointer is only valid until the next call
 *  to the gethost*() functions.
 */
char *getfullhostname()
{
    struct hostent *hp = NULL;
    char *buf = (char *)malloc(MAXHOSTNAMELEN + 1);

    if (gethostname(buf, MAXHOSTNAMELEN) < 0) {
        //log_errno("gethostname");
        return (NULL);
    }

    if ((hp = gethostbyname(buf)) == NULL) {
        //Debug(0, 1, ("getfullhostname: gethostbyname(%s) returned NULL.\n", buf));
        return NULL;
    }
    return (hp->h_name);
}

/*
 *  getmylogin() - Returns the login for the pid of the current process,
 *  or "nobody" if there is not login associated with the pid of the
 *  current process.  Intended to be a replacement for braindead getlogin(3).
 */
char *getmylogin()
{
    char *nobody_str = "nobody";
    uid_t myuid = getuid();
    struct passwd *pwp = NULL;

    pwp = getpwuid(myuid);
    if (pwp == NULL || pwp->pw_name == NULL)
	return (nobody_str);
    return (pwp->pw_name);
}

#ifdef USE_HOST_CACHE
char *getrealhost(s)
     char *s;
{
    Host *H = get_host(s);
    return H ? strdup(H->fqdn) : 0;
    /* We rely on new_host() to do the right thing with IP numbers */
}
#else
/*
 *  getrealhost() - Returns the real fully qualified name of the given
 *  host or IP number, or NULL on error.    If it's an IP number and
 *  the DNS doesn't know about it, then just return the IP number.
 *  Returns a string that was allocated with malloc(3).
 */
char *getrealhost(s)
     char *s;
{
    char *q;
    struct hostent *hp;
    int is_octet = 1, ndots = 0;
    unsigned int addr = 0;

    if (s == NULL || *s == '\0')
	return (NULL);

    for (q = s; *q; q++) {
	if (*q == '.') {
	    ndots++;
	    continue;
	} else if (!isdigit((unsigned char) *q)) {
	    /* [^0-9] is a name */
	    is_octet = 0;
	    break;
	}
    }

    if (ndots != 3)
	is_octet = 0;

    if (is_octet) {
	addr = inet_addr(s);
	hp = gethostbyaddr((char *) &addr, sizeof(unsigned int), AF_INET);
	if (hp == (struct hostent *) NULL) {
	    return strdup(s);
	}
    } else {
	hp = gethostbyname(s);
    }
    return (hp != NULL ? strdup(hp->h_name) : NULL);
}
#endif

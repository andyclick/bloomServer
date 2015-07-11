/*
 *  buffer.c - Simple dynamic buffer management.
 * 
 *  DEBUG: section  81, level 1         Common utilities buffer routines
 *
 *  Darren Hardy, hardy@cs.colorado.edu, February 1994
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
#include <string.h>
#include "util.h"

/*
 *  create_buffer() - Creates a buffer of default_size bytes allocated.
 */
Buffer *create_buffer(default_size)
     int default_size;
{
    Buffer *b = NULL;

    b = malloc(sizeof(Buffer));
    b->size = b->default_size = default_size;
    b->data = malloc(b->size);
    b->length = 0;
    //mylog_info(m_pLogGlobalCtrl->infolog, "Creating buffer of %d bytes - %s:%s:%d\n",b->size ,INFO_LOG_SUFFIX);
    return (b);
}

/*
 *  increase_buffer() - Increase the buffer so that it holds sz more bytes.
 */
void increase_buffer(b, sz)
     Buffer *b;
     int sz;
{
    b->size += sz;
    b->data = realloc(b->data, b->size);
    //mylog_info(m_pLogGlobalCtrl->infolog, "Growing buffer by %d bytes to %d bytes - %s:%s:%d\n",sz ,b->size ,INFO_LOG_SUFFIX);
}

/*
 *  grow_buffer() - increases the buffer size by the default size
 */
void grow_buffer(b)
     Buffer *b;
{
    increase_buffer(b, b->default_size);
}

/*
 *  shrink_buffer() - restores a buffer back to its original size.
 *  all data is lost.
 */
void shrink_buffer(b)
     Buffer *b;
{
    b->length = 0;
    if (b->size == b->default_size)	/* nothing to do */
	return;

    if (b->data)
	free(b->data);
    b->size = b->default_size;
    b->data = malloc(b->size);
    //mylog_info(m_pLogGlobalCtrl->infolog, "Shrinking buffer to %d bytes - %s:%s:%d\n",b->size ,INFO_LOG_SUFFIX);
}

/*
 *  free_buffer() - Cleans up after a buffer.
 */
void free_buffer(b)
     Buffer *b;
{
    if (b == NULL)
        return;
    //mylog_info(m_pLogGlobalCtrl->infolog, "Freeing buffer of %d bytes - %s:%s:%d\n",b->size ,INFO_LOG_SUFFIX);
    if (b->data)
        free(b->data);
    free(b);
}


/*
 *  add_buffer() - Adds the sz bytes of s to the Buffer b.
 */
void add_buffer(b, s, sz)
     Buffer *b;
     char *s;
     int sz;
{
    if (sz < 1)
	return;
    if (b->length + sz + 1 > b->size)
	increase_buffer(b, sz);
    if (sz > 1)
	memcpy(&b->data[b->length], s, sz);
    else
	b->data[b->length] = *s;
    b->length += sz;
    b->data[b->length] = '\0';	/* add NULL to current position */
}

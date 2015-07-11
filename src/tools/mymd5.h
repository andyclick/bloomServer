#ifndef KUROBOT_MYMD5_H 
#define KUROBOT_MYMD5_H 
/* md5.h,v 1.3 1995/02/01 21:42:47 hardy Exp
 ***********************************************************************
 ** md5.h -- header file for implementation of MD5                    **
 ** RSA Data Security, Inc. MD5 Message-Digest Algorithm              **
 ** Created: 2/17/90 RLR                                              **
 ** Revised: 12/27/90 SRD,AJ,BSK,JT Reference C version               **
 ** Revised (for MD5): RLR 4/27/91                                    **
 ***********************************************************************
 */

/*
 ***********************************************************************
 ** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved.  **
 **                                                                   **
 ** License to copy and use this software is granted provided that    **
 ** it is identified as the "RSA Data Security, Inc. MD5 Message-     **
 ** Digest Algorithm" in all material mentioning or referencing this  **
 ** software or this function.                                        **
 **                                                                   **
 ** License is also granted to make and use derivative works          **
 ** provided that such works are identified as "derived from the RSA  **
 ** Data Security, Inc. MD5 Message-Digest Algorithm" in all          **
 ** material mentioning or referencing the derived work.              **
 **                                                                   **
 ** RSA Data Security, Inc. makes no representations concerning       **
 ** either the merchantability of this software or the suitability    **
 ** of this software for any particular purpose.  It is provided "as  **
 ** is" without express or implied warranty of any kind.              **
 **                                                                   **
 ** These notices must be retained in any copies of any part of this  **
 ** documentation and/or software.                                    **
 ***********************************************************************
 */


/* From:
 * ntp_types.h,v 3.1 1993/07/06 01:07:00 jbj Exp
 *  ntp_types.h - defines how LONG and U_LONG are treated.  For 64 bit systems
 *  like the DEC Alpha, they has to be defined as int and u_int.  for 32 bit
 *  systems, define them as long and u_long
 */

/*
 * DEC Alpha systems need LONG and U_LONG defined as int and u_int
 */
#ifdef __alpha
#ifndef LONG
#define LONG int
#endif /* LONG */
#ifndef U_LONG
#define U_LONG u_int
#endif /* U_LONG */
/*
 *  All other systems fall into this part
 */
#else /* __alpha */
#ifndef LONG
#define LONG long
#endif /* LONG */
#ifndef U_LONG
#define U_LONG u_long
#endif /* U_LONG */
#endif /* __alpha */

/* typedef a 32-bit type */
typedef unsigned LONG UINT4;

#ifdef  __cplusplus
extern "C" {
#endif
void MD5Init ();
void MD5Update ();
void MD5Final ();
char *get_md5();
char *get_md5_string(char *dat, int datlen);

#ifdef  __cplusplus
}
#endif

/*
 ***********************************************************************
 ** End of md5.h                                                      **
 ******************************** (cut) ********************************
 */

#endif //KUROBOT_MYMD5_H 

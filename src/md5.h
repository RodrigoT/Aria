/*
 *	Copyright (C) 1998 Free Software Foundation, Inc.
 *
 * This file is part of GNUPG.
 *
 * GNUPG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GNUPG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

/* modified by Tatsuhiro Tsujikawa <tujikawa@rednoah.com> */

#include <stdlib.h>
#include "config.h"
#ifndef G10_TYPES_H
#define G10_TYPES_H


/* The AC_CHECK_SIZEOF() in configure fails for some machines.
 * we provide some fallback values here */
#if !SIZEOF_UNSIGNED_SHORT
  #undef SIZEOF_UNSIGNED_SHORT
  #define SIZEOF_UNSIGNED_SHORT 2
#endif
#if !SIZEOF_UNSIGNED_INT
  #undef SIZEOF_UNSIGNED_INT
  #define SIZEOF_UNSIGNED_INT 4
#endif
#if !SIZEOF_UNSIGNED_LONG
  #undef SIZEOF_UNSIGNED_LONG
  #define SIZEOF_UNSIGNED_LONG 4
#endif


#include <sys/types.h>


#ifndef HAVE_BYTE_TYPEDEF
  #undef byte	    /* maybe there is a macro with this name */
  typedef unsigned char byte;
  #define HAVE_BYTE_TYPEDEF
#endif

#ifndef HAVE_USHORT_TYPEDEF
  #undef ushort     /* maybe there is a macro with this name */
  typedef unsigned short ushort;
  #define HAVE_USHORT_TYPEDEF
#endif

#ifndef HAVE_ULONG_TYPEDEF
  #undef ulong	    /* maybe there is a macro with this name */
  typedef unsigned long ulong;
  #define HAVE_ULONG_TYPEDEF
#endif

#ifndef HAVE_U32_TYPEDEF
  #undef u32	    /* maybe there is a macro with this name */
  #if SIZEOF_UNSIGNED_INT == 4
    typedef unsigned int u32;
  #elif SIZEOF_UNSIGNED_LONG == 4
    typedef unsigned long u32;
  #else
    #error no typedef for u32
  #endif
  #define HAVE_U32_TYPEDEF
#endif

typedef union {
    int a;
    short b;
    char c[1];
    long d;
  #ifdef HAVE_U64_TYPEDEF
    u64 e;
  #endif
    float f;
    double g;
} PROPERLY_ALIGNED_TYPE;

typedef struct string_list {
    struct string_list *next;
    unsigned int flags;
    char d[1];
} *STRLIST;


#endif /*G10_TYPES_H*/

/****************
 * Rotate a 32 bit integer by n bytes
 */
#if defined(__GNUC__) && defined(__i386__)
static inline u32
rol( u32 x, int n)
{
        __asm__("roll %%cl,%0"
                :"=r" (x)
                :"0" (x),"c" (n));
        return x;
}
#else
  #define rol(x,n) ( ((x) << (n)) | ((x) >> (32-(n))) )
#endif


/******** some macros ************/
#ifndef STR
  #define STR(v) #v
#endif
#define STR2(v) STR(v)
#define DIM(v) (sizeof(v)/sizeof((v)[0]))
#define DIMof(type,member)   DIM(((type *)0)->member)

/******** export md5 struct and functions ********/
typedef struct {
    u32 A,B,C,D;	  /* chaining variables */
    u32  nblocks;
    byte buf[64];
    int  count;
} MD5_CONTEXT;

void md5_init( MD5_CONTEXT *ctx );
void md5_write( MD5_CONTEXT *hd, byte *inbuf, size_t inlen);
void md5_final( MD5_CONTEXT *hd );

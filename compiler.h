//
//
// Copyright (C) 2023-2024 Frenkel Smeijers
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __COMPILER__
#define __COMPILER__

#if defined _M_I86
//16-bit
#include <i86.h>

#define D_MK_FP  MK_FP
#define D_FP_SEG FP_SEG
#if defined __WATCOMC__
#define D_FP_OFF FP_OFF
#else
#define D_FP_OFF(p) ((uint16_t)((uint32_t)(p)))
#endif

typedef uint16_t segment_t;
#define SIZE_OF_SEGMENT_T 2

#ifndef _fmemcpy
#include <stddef.h>
void __far* _fmemcpy(void __far* destination, const void __far* source, size_t num);
#endif

#ifndef _fmemset
#include <stddef.h>
void __far* _fmemset(void __far* str, int c, size_t n);
#endif

#else
//32-bit
#define D_MK_FP(s,o) (void*)((s<<4)+o)
#define D_FP_SEG(p)  (((uint32_t)p)>>4)
#define D_FP_OFF(p)  (((uint32_t)p)&15)

typedef uint32_t segment_t;
#define SIZE_OF_SEGMENT_T 4

#define __far

#define _fmemcpy	memcpy
#define _fmemset	memset

#endif



#if defined __DJGPP__
//DJGPP
#include <sys/nearptr.h>

//DJGPP doesn't inline inp, outp and outpw,
//but it does inline inportb, outportb and outportw
#define inp(port)			inportb(port)
#define outp(port,data)		outportb(port,data)


#else
//Watcom and gcc-ia16
#define __djgpp_nearptr_enable()
#define __djgpp_conventional_base 0


#endif

#endif

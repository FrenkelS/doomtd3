/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2024 Frenkel Smeijers
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Code specific to ELKS
 *
 *-----------------------------------------------------------------------------*/

#include <stdarg.h>
#include <stdint.h>
#include <sys/time.h>

#include "compiler.h"

#include "d_main.h"
#include "i_system.h"
#include "m_random.h"
#include "r_defs.h"
#include "v_video.h"
#include "w_wad.h"

#include "globdata.h"


#define PLANEWIDTH			 80
#define SCREENHEIGHT_CGA	200


extern const int16_t CENTERY;

static uint8_t _s_viewwindow[VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT];
static uint8_t __far* _s_statusbar;
static uint8_t __far* videomemory_view;
static uint8_t __far* videomemory_statusbar;

static boolean isGraphicsModeSet = false;


static void I_SetScreenMode(uint8_t mode)
{
   // SI, DI, BP, ES and probably DS are to be saved
   // cli and sti are used to make a proper BIOS call from ELKS
   __asm__(
  "push %%si;"
  "push %%di;"
  "push %%bp;"
  "push %%es;"
  "cli;"
  "mov %%ah,0;" 
  "mov %%al,%0;" 
  "int $0x10;"
  "sti;"
  "pop %%es;"
  "pop %%bp;"
  "pop %%di;"
  "pop %%si;"
     : /* no outputs */
     : "r" (mode)
     : ); //list of modified registers
}


void I_InitGraphics(void)
{
	I_SetScreenMode(6);

	videomemory_view      = D_MK_FP(0xb800, ((PLANEWIDTH - VIEWWINDOWWIDTH)     / 2) + (((SCREENHEIGHT_CGA - SCREENHEIGHT) / 2) * PLANEWIDTH) / 2                                     + __djgpp_conventional_base);
	videomemory_statusbar = D_MK_FP(0xb800, ((PLANEWIDTH - SCREENWIDTH * 2 / 8) / 2) + (((SCREENHEIGHT_CGA - SCREENHEIGHT) / 2) * PLANEWIDTH) / 2 + VIEWWINDOWHEIGHT * PLANEWIDTH / 2 + __djgpp_conventional_base);

	_s_statusbar = Z_MallocStatic(SCREENWIDTH * ST_HEIGHT);

	isGraphicsModeSet = true;
}


static void I_ShutdownGraphics(void)
{
	if (isGraphicsModeSet)
		I_SetScreenMode(3);
}


void I_SetPalette(int8_t pal)
{
}


#define B0 (0 << 0)
#define B1 (1 << 0)
#define B2 (3 << 0)

static const uint8_t VGA_TO_BW_LUT_0[256] =
{
	B0, B0, B0, B1, B2, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2,
	B2, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B1, B1, B1, B0, B0, B1, B1, B1, B1, B1, B1, B0, B0,
	B2, B2, B2, B2, B1, B1, B1, B0, B2, B2, B2, B2, B2, B2, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1,
	B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B0, B0, B0, B0, B0,
	B0, B0, B0, B0, B0, B0, B0, B0, B2, B2, B2, B1, B1, B0, B0, B1
};

#undef B0
#undef B1
#undef B2

#define B0 (0 << 0)
#define B1 (2 << 0)
#define B2 (3 << 0)

static const uint8_t VGA_TO_BW_LUT_0b[256] =
{
	B0, B0, B0, B1, B2, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2,
	B2, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B1, B1, B1, B0, B0, B1, B1, B1, B1, B1, B1, B0, B0,
	B2, B2, B2, B2, B1, B1, B1, B0, B2, B2, B2, B2, B2, B2, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1,
	B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B0, B0, B0, B0, B0,
	B0, B0, B0, B0, B0, B0, B0, B0, B2, B2, B2, B1, B1, B0, B0, B1
};

#undef B0
#undef B1
#undef B2

#define B0 (0 << 2)
#define B1 (1 << 2)
#define B2 (3 << 2)

static const uint8_t VGA_TO_BW_LUT_1[256] =
{
	B0, B0, B0, B1, B2, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2,
	B2, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B1, B1, B1, B0, B0, B1, B1, B1, B1, B1, B1, B0, B0,
	B2, B2, B2, B2, B1, B1, B1, B0, B2, B2, B2, B2, B2, B2, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1,
	B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B0, B0, B0, B0, B0,
	B0, B0, B0, B0, B0, B0, B0, B0, B2, B2, B2, B1, B1, B0, B0, B1
};

#undef B0
#undef B1
#undef B2

#define B0 (0 << 2)
#define B1 (2 << 2)
#define B2 (3 << 2)

static const uint8_t VGA_TO_BW_LUT_1b[256] =
{
	B0, B0, B0, B1, B2, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2,
	B2, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B1, B1, B1, B0, B0, B1, B1, B1, B1, B1, B1, B0, B0,
	B2, B2, B2, B2, B1, B1, B1, B0, B2, B2, B2, B2, B2, B2, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1,
	B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B0, B0, B0, B0, B0,
	B0, B0, B0, B0, B0, B0, B0, B0, B2, B2, B2, B1, B1, B0, B0, B1
};

#undef B0
#undef B1
#undef B2

#define B0 (0 << 4)
#define B1 (1 << 4)
#define B2 (3 << 4)

static const uint8_t VGA_TO_BW_LUT_2[256] =
{
	B0, B0, B0, B1, B2, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2,
	B2, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B1, B1, B1, B0, B0, B1, B1, B1, B1, B1, B1, B0, B0,
	B2, B2, B2, B2, B1, B1, B1, B0, B2, B2, B2, B2, B2, B2, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1,
	B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B0, B0, B0, B0, B0,
	B0, B0, B0, B0, B0, B0, B0, B0, B2, B2, B2, B1, B1, B0, B0, B1
};

#undef B0
#undef B1
#undef B2

#define B0 (0 << 4)
#define B1 (2 << 4)
#define B2 (3 << 4)

static const uint8_t VGA_TO_BW_LUT_2b[256] =
{
	B0, B0, B0, B1, B2, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2,
	B2, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B1, B1, B1, B0, B0, B1, B1, B1, B1, B1, B1, B0, B0,
	B2, B2, B2, B2, B1, B1, B1, B0, B2, B2, B2, B2, B2, B2, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1,
	B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B0, B0, B0, B0, B0,
	B0, B0, B0, B0, B0, B0, B0, B0, B2, B2, B2, B1, B1, B0, B0, B1
};

#undef B0
#undef B1
#undef B2

#define B0 (0 << 6)
#define B1 (1 << 6)
#define B2 (3 << 6)

static const uint8_t VGA_TO_BW_LUT_3[256] =
{
	B0, B0, B0, B1, B2, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2,
	B2, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B1, B1, B1, B0, B0, B1, B1, B1, B1, B1, B1, B0, B0,
	B2, B2, B2, B2, B1, B1, B1, B0, B2, B2, B2, B2, B2, B2, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1,
	B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B0, B0, B0, B0, B0,
	B0, B0, B0, B0, B0, B0, B0, B0, B2, B2, B2, B1, B1, B0, B0, B1
};

#undef B0
#undef B1
#undef B2

#define B0 (0 << 6)
#define B1 (2 << 6)
#define B2 (3 << 6)

static const uint8_t VGA_TO_BW_LUT_3b[256] =
{
	B0, B0, B0, B1, B2, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2,
	B2, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B1, B1, B1, B0, B0, B1, B1, B1, B1, B1, B1, B0, B0,
	B2, B2, B2, B2, B1, B1, B1, B0, B2, B2, B2, B2, B2, B2, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1,
	B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B0, B0, B0, B0, B0,
	B0, B0, B0, B0, B0, B0, B0, B0, B2, B2, B2, B1, B1, B0, B0, B1
};

#undef B0
#undef B1
#undef B2


static boolean refreshStatusBar;


void I_FinishUpdate(void)
{
	// view window
	uint8_t *src = &_s_viewwindow[0];
	uint8_t __far* dst = videomemory_view;

	for (uint_fast8_t y = 0; y < VIEWWINDOWHEIGHT / 2; y++) {
		_fmemcpy(dst, src, VIEWWINDOWWIDTH);

		dst += 0x2000;
		src += VIEWWINDOWWIDTH;

		_fmemcpy(dst, src, VIEWWINDOWWIDTH);

		dst -= 0x2000 - PLANEWIDTH;
		src += VIEWWINDOWWIDTH;
	}

	// status bar
	if (refreshStatusBar)
	{
		refreshStatusBar = false;

		uint8_t __far* src = _s_statusbar;
		dst = videomemory_statusbar;
		for (uint_fast8_t y = 0; y < ST_HEIGHT / 2; y++) {
			for (uint_fast8_t x = 0; x < (SCREENWIDTH * 2 / 8); x++) {
				*dst++ = (VGA_TO_BW_LUT_3[*src++] | VGA_TO_BW_LUT_2[*src++] | VGA_TO_BW_LUT_1[*src++] | VGA_TO_BW_LUT_0[*src++]);
			}

			dst += 0x2000 - (SCREENWIDTH * 2 / 8);

			for (uint_fast8_t x = 0; x < (SCREENWIDTH * 2 / 8); x++) {
				*dst++ = (VGA_TO_BW_LUT_3b[*src++] | VGA_TO_BW_LUT_2b[*src++] | VGA_TO_BW_LUT_1b[*src++] | VGA_TO_BW_LUT_0b[*src++]);
			}

			dst -= 0x2000 - (PLANEWIDTH - (SCREENWIDTH * 2 / 8));
		}
	}
}


void R_InitColormaps(void)
{
	fullcolormap = W_GetLumpByName("COLORMAP"); // Never freed
}


#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)

static uint8_t nearcolormap[256];

#define L_FP_OFF D_FP_OFF
static uint16_t nearcolormapoffset = 0xffff;

static const uint8_t __far* source;
static const uint8_t *dest;


static void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count)
{
	uint8_t __far* dst = (uint8_t __far*)dest;
	int16_t l = count >> 4;

	while (l--)
	{
		*dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		*dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		*dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		*dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;

		*dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		*dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		*dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		*dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;

		*dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		*dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		*dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		*dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;

		*dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		*dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		*dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		*dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
	}

	switch (count & 15)
	{
		case 15: *dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		case 14: *dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		case 13: *dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		case 12: *dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		case 11: *dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		case 10: *dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		case  9: *dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		case  8: *dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		case  7: *dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		case  6: *dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		case  5: *dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		case  4: *dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		case  3: *dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		case  2: *dst = nearcolormap[source[frac>>COLBITS]]; dst += VIEWWINDOWWIDTH; frac += fracstep;
		case  1: *dst = nearcolormap[source[frac>>COLBITS]];
	}
}


void R_DrawColumn(const draw_column_vars_t *dcvars)
{
	const int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	source = dcvars->source;

	if (nearcolormapoffset != L_FP_OFF(dcvars->colormap))
	{
		_fmemcpy(nearcolormap, dcvars->colormap, 256);
		nearcolormapoffset = L_FP_OFF(dcvars->colormap);
	}

	dest = &_s_viewwindow[(dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x];

	const uint16_t fracstep = (dcvars->iscale >> COLEXTRABITS);
	uint16_t frac = (dcvars->texturemid + (dcvars->yl - CENTERY) * dcvars->iscale) >> COLEXTRABITS;

	R_DrawColumn2(fracstep, frac, count);
}


void R_DrawColumnFlat(int16_t texture, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	if (count <= 0)
		return;

	const uint8_t color1 = texture;
	const uint8_t color2 = (color1 << 4 | color1 >> 4);
	const uint8_t colort = color1 + color2;
	      uint8_t color  = (dcvars->yl & 1) ? color1 : color2;

	uint8_t *dest = &_s_viewwindow[(dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x];

	while (count--)
	{
		*dest = color;
		dest += VIEWWINDOWWIDTH;
		color = colort - color;
	}
}


#define FUZZOFF (VIEWWINDOWWIDTH)
#define FUZZTABLE 50

static const int8_t fuzzoffset[FUZZTABLE] =
{
	FUZZOFF,-FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
	FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
	FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,
	FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
	FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,
	FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,
	FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF
};


void R_DrawFuzzColumn(const draw_column_vars_t *dcvars)
{
	int16_t dc_yl = dcvars->yl;
	int16_t dc_yh = dcvars->yh;

	// Adjust borders. Low...
	if (dc_yl <= 0)
		dc_yl = 1;

	// .. and high.
	if (dc_yh >= VIEWWINDOWHEIGHT - 1)
		dc_yh = VIEWWINDOWHEIGHT - 2;

	int16_t count = (dc_yh - dc_yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	if (nearcolormapoffset != L_FP_OFF(&fullcolormap[6 * 256]))
	{
		_fmemcpy(nearcolormap, &fullcolormap[6 * 256], 256);
		nearcolormapoffset = L_FP_OFF(&fullcolormap[6 * 256]);
	}

	uint8_t *dest = &_s_viewwindow[(dc_yl * VIEWWINDOWWIDTH) + dcvars->x];

	static int16_t fuzzpos = 0;

	do
	{
		*dest = nearcolormap[dest[fuzzoffset[fuzzpos]]];
		dest += VIEWWINDOWWIDTH;

		fuzzpos++;
		if (fuzzpos >= FUZZTABLE)
			fuzzpos = 0;

	} while(--count);
}


void V_DrawRaw(int16_t num, uint16_t offset)
{
	refreshStatusBar = true;

	const uint8_t __far* lump = W_TryGetLumpByNum(num);

	if (lump != NULL)
	{
		uint16_t lumpLength = W_LumpLength(num);
		_fmemcpy(&_s_statusbar[offset - (SCREENHEIGHT - ST_HEIGHT) * SCREENWIDTH], lump, lumpLength);
		Z_ChangeTagToCache(lump);
	}
	else
		W_ReadLumpByNum(num, &_s_statusbar[offset - (SCREENHEIGHT - ST_HEIGHT) * SCREENWIDTH]);
}


void ST_Drawer(void)
{
	if (ST_NeedUpdate())
		ST_doRefresh();
}


void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	y -= patch->topoffset;
	x -= patch->leftoffset;

	byte __far* desttop = _s_statusbar + (y * SCREENWIDTH) + x - (SCREENHEIGHT - ST_HEIGHT) * SCREENWIDTH;

	int16_t width = patch->width;

	for (int16_t col = 0; col < width; col++, desttop++)
	{
		const column_t __far* column = (const column_t __far*)((const byte __far*)patch + (uint16_t)patch->columnofs[col]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			const byte __far* source = (const byte __far*)column + 3;
			byte __far* dest = desttop + (column->topdelta * SCREENWIDTH);

			uint16_t count = column->length;

			while (count--)
			{
				*dest = *source++;
				dest += SCREENWIDTH;
			}

			column = (const column_t __far*)((const byte __far*)column + column->length + 4);
		}
	}
}


segment_t I_ZoneBase(uint32_t *size)
{
	uint32_t paragraphs = 550 * 1024L / PARAGRAPH_SIZE;
	uint8_t __far* ptr = fmemalloc(paragraphs * PARAGRAPH_SIZE);
	while (!ptr)
	{
		paragraphs--;
		ptr = fmemalloc(paragraphs * PARAGRAPH_SIZE);
	}

	// align ptr
	uint32_t m = (uint32_t) ptr;
	if ((m & (PARAGRAPH_SIZE - 1)) != 0)
	{
		paragraphs--;
		while ((m & (PARAGRAPH_SIZE - 1)) != 0)
			m = (uint32_t) ++ptr;
	}

	*size = paragraphs * PARAGRAPH_SIZE;
	return D_FP_SEG(ptr);
}


segment_t I_ZoneAdditional(uint32_t *size)
{
	*size = 0;
	return 0;
}


static int32_t clock()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_sec * 1000L + now.tv_usec * 1000L;
}


static int32_t starttime;


void I_StartClock(void)
{
	starttime = clock();
}


uint32_t I_EndClock(void)
{
	int32_t endtime = clock();
	return ((endtime - starttime) * TICRATE) / 1000L;
}


void I_Error2(const char *error, ...)
{
	va_list argptr;

	I_ShutdownGraphics();

	va_start(argptr, error);
	vfprintf(stdout, error, argptr);
	va_end(argptr);
	printf("\n");
	exit(1);
}


void __far* _fmemcpy(void __far* destination, const void __far* source, size_t num)
{
	uint8_t __far* s = (uint8_t __far*)source;
	uint8_t __far* d = (uint8_t __far*)destination;

	for (size_t i = 0; i < num; i++)
		*d++ = *s++;

	return NULL;
}


void __far* _fmemset(void __far* str, int c, size_t n)
{
	uint8_t __far* d = (uint8_t __far*)str;

	for (size_t i = 0; i < n; i++)
		*d++ = c;

	return NULL;
}


int16_t abs(int16_t v)
{
	return v < 0 ? -v : v;
}


int32_t labs(int32_t v)
{
	return v < 0 ? -v : v;
}


int main(int argc, const char * const * argv)
{
	D_DoomMain();
	return 0;
}

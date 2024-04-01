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
 *      Code specific to the Amiga 500
 *
 *-----------------------------------------------------------------------------*/

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include <stdarg.h>
#include <stdint.h>
#include <time.h>

#include "compiler.h"

#include "d_main.h"
#include "i_system.h"
#include "m_random.h"
#include "r_defs.h"
#include "v_video.h"
#include "w_wad.h"

#include "globdata.h"


#define PLANEWIDTH			 80

extern struct GfxBase *GfxBase;
extern struct Custom custom;

extern const int16_t CENTERY;

static uint16_t viewwindowtop;
static uint8_t *_s_viewwindow;
static uint8_t *_s_statusbar;

static uint32_t screenpaget;
static uint8_t *screenpage;

static boolean isGraphicsModeSet = false;


#define FMODE	0x1fc

#define DDFSTRT			0x092
#define DDFSTOP			0x094
#define DDFSTRT_VALUE	0x003c
#define DDFSTOP_VALUE	0x00d4

#define DIWSTRT				0x08e
#define DIWSTOP				0x090
#define DIWSTRT_VALUE		0x2c81
#define DIWSTOP_VALUE_PAL	0x2cc1
#define DIWSTOP_VALUE_NTSC	0xf4c1

#define BPLCON0			0x100
#define BPLCON0_VALUE	0b1001000000000000

#define BPL1MOD	0x108

#define COLOR00	0x180
#define COLOR01	0x182

#define BPL1PTH	0x0e0
#define BPL1PTL	0x0e2

#define COPLIST_IDX_DIWSTOP_VALUE 9
#define COPLIST_IDX_COLOR00_VALUE 15
#define COPLIST_IDX_BPL1PTH_VALUE 19
#define COPLIST_IDX_BPL1PTL_VALUE 21


static uint16_t __chip coplist[] = {
	FMODE,   0,
	DDFSTRT, DDFSTRT_VALUE,
	DDFSTOP, DDFSTOP_VALUE,
	DIWSTRT, DIWSTRT_VALUE,
	DIWSTOP, DIWSTOP_VALUE_PAL,
	BPLCON0, BPLCON0_VALUE,
	BPL1MOD, 0,

	COLOR00, 0x000,
	COLOR01, 0xfff,

	BPL1PTH, 0,
	BPL1PTL, 0,

	0xffff, 0xfffe, // COP_WAIT_END
	0xffff, 0xfffe  // COP_WAIT_END
};


static const int16_t colors[14] =
{
	0x000,													// normal
	0x800, 0x900, 0xa00, 0xb00, 0xc00, 0xd00, 0xe00, 0xf00,	// red
	0x440, 0x550, 0x660, 0x770,								// yellow
	0x070													// green
};


static void I_UploadNewPalette(int8_t pal)
{
	coplist[COPLIST_IDX_COLOR00_VALUE] = colors[pal];
}


void I_InitGraphics(void)
{
	LoadView(NULL);
	WaitTOF();
	WaitTOF();

	boolean pal = (((struct GfxBase *) GfxBase)->DisplayFlags & PAL) == PAL;
	int16_t screenHeightAmiga;
	if (pal) {
		coplist[COPLIST_IDX_DIWSTOP_VALUE] = DIWSTOP_VALUE_PAL;
		screenHeightAmiga = 256;
	} else {
		coplist[COPLIST_IDX_DIWSTOP_VALUE] = DIWSTOP_VALUE_NTSC;
		screenHeightAmiga = 200;
	}

	uint8_t *screenpage0 = Z_MallocStatic(PLANEWIDTH * screenHeightAmiga * 2);	
	memset(screenpage0, 0, PLANEWIDTH * screenHeightAmiga * 2);

	uint8_t *screenpage1 = screenpage0 + PLANEWIDTH * screenHeightAmiga;
	screenpaget = (uint32_t)screenpage0 + (uint32_t)screenpage1;
	screenpage = screenpage1;

	uint32_t addr = (uint32_t) screenpage1;
	coplist[COPLIST_IDX_BPL1PTH_VALUE] = addr >> 16;
	coplist[COPLIST_IDX_BPL1PTL_VALUE] = addr;

	I_UploadNewPalette(0);

	custom.dmacon = 0x0020;
	custom.cop1lc = (uint32_t) coplist;

	viewwindowtop = ((PLANEWIDTH - VIEWWINDOWWIDTH) / 2) + ((screenHeightAmiga - SCREENHEIGHT) / 2) * PLANEWIDTH;
	_s_viewwindow = screenpage + viewwindowtop;

	_s_statusbar  = Z_MallocStatic(SCREENWIDTH * ST_HEIGHT);

	OwnBlitter();
	WaitBlit();
	custom.bltcon0 = 0b0000100111110000;
	custom.bltcon1 = 0;

	custom.bltamod = PLANEWIDTH - VIEWWINDOWWIDTH;
	custom.bltdmod = PLANEWIDTH - VIEWWINDOWWIDTH;

	custom.bltafwm = 0xffff;
	custom.bltalwm = 0xffff;

	custom.bltbdat = 0xffff;
	custom.bltcdat = 0xffff;

	isGraphicsModeSet = true;
}


static void I_ShutdownGraphics(void)
{
	if (isGraphicsModeSet)
	{
		DisownBlitter();
		LoadView(((struct GfxBase *) GfxBase)->ActiView);
		WaitTOF();
		WaitTOF();
		custom.cop1lc = (uint32_t) ((struct GfxBase *) GfxBase)->copinit;
		RethinkDisplay();
	}
}


static int8_t newpal;


void I_SetPalette(int8_t pal)
{
	newpal = pal;
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


#define NO_PALETTE_CHANGE 100

static uint16_t st_needrefresh = 0;

void I_FinishUpdate(void)
{
	// palette
	if (newpal != NO_PALETTE_CHANGE)
	{
		I_UploadNewPalette(newpal);
		newpal = NO_PALETTE_CHANGE;
	}

	// status bar
	if (st_needrefresh == 2)
	{
		st_needrefresh--;

		uint8_t *src = _s_statusbar;
		uint8_t *dst = _s_viewwindow + PLANEWIDTH * VIEWWINDOWHEIGHT;
		for (uint_fast8_t y = 0; y < ST_HEIGHT / 2; y++) {
			for (uint_fast8_t x = 0; x < VIEWWINDOWWIDTH; x++) {
				*dst++ = VGA_TO_BW_LUT_3[*src++] | VGA_TO_BW_LUT_2[*src++] | VGA_TO_BW_LUT_1[*src++] | VGA_TO_BW_LUT_0[*src++];
			}

			dst += PLANEWIDTH - VIEWWINDOWWIDTH;

			for (uint_fast8_t x = 0; x < VIEWWINDOWWIDTH; x++) {
				*dst++ = VGA_TO_BW_LUT_3b[*src++] | VGA_TO_BW_LUT_2b[*src++] | VGA_TO_BW_LUT_1b[*src++] | VGA_TO_BW_LUT_0b[*src++];
			}

			dst += PLANEWIDTH - VIEWWINDOWWIDTH;
		}
	}
	else if (st_needrefresh) // st_needrefresh == 1
	{
		st_needrefresh = 0;

		WaitBlit();

		custom.bltapt = (uint8_t*)(screenpaget - (uint32_t)screenpage) + viewwindowtop + PLANEWIDTH * VIEWWINDOWHEIGHT;
		custom.bltdpt = _s_viewwindow + PLANEWIDTH * VIEWWINDOWHEIGHT;

		custom.bltsize = (ST_HEIGHT << 6) | (VIEWWINDOWWIDTH / 2);
	}

	// page flip
	uint32_t addr = (uint32_t) screenpage;
	coplist[COPLIST_IDX_BPL1PTH_VALUE] = addr >> 16;
	coplist[COPLIST_IDX_BPL1PTL_VALUE] = addr;
	screenpage = (uint8_t*)(screenpaget - (uint32_t)screenpage);
	_s_viewwindow = screenpage + viewwindowtop;
}


void R_InitColormaps(void)
{
	fullcolormap = W_GetLumpByName("COLORMAP"); // Never freed
}


#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)

void R_DrawColumn(const draw_column_vars_t *dcvars)
{
	const int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	const uint8_t *source = dcvars->source;

	const uint8_t *nearcolormap = dcvars->colormap;

	uint8_t *dest = _s_viewwindow + (dcvars->yl * PLANEWIDTH) + dcvars->x;

	const uint16_t fracstep = (dcvars->iscale >> COLEXTRABITS);
	uint16_t frac = (dcvars->texturemid + (dcvars->yl - CENTERY) * dcvars->iscale) >> COLEXTRABITS;

	int16_t l = count >> 4;

	while (l--)
	{
		*dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;

		*dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;

		*dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;

		*dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
	}

	switch (count & 15)
	{
		case 15: *dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 14: *dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 13: *dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 12: *dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 11: *dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 10: *dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  9: *dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  8: *dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  7: *dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  6: *dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  5: *dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  4: *dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  3: *dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  2: *dest = nearcolormap[source[frac>>COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  1: *dest = nearcolormap[source[frac>>COLBITS]];
	}
}


void R_DrawColumnFlat(int16_t texture, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	const uint8_t color1 = texture;
	const uint8_t color2 = (color1 << 4 | color1 >> 4);
	const uint8_t colort = color1 + color2;
	      uint8_t color  = (dcvars->yl & 1) ? color1 : color2;

	uint8_t *dest = _s_viewwindow + (dcvars->yl * PLANEWIDTH) + dcvars->x;

	while (count--)
	{
		*dest = color;
		dest += PLANEWIDTH;
		color = colort - color;
	}
}


#define FUZZOFF (PLANEWIDTH)
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

	const uint8_t *nearcolormap = &fullcolormap[6 * 256];

	uint8_t *dest = _s_viewwindow + (dc_yl * PLANEWIDTH) + dcvars->x;

	static int16_t fuzzpos = 0;

	do
	{
		*dest = nearcolormap[dest[fuzzoffset[fuzzpos]]];
		dest += PLANEWIDTH;

		fuzzpos++;
		if (fuzzpos >= FUZZTABLE)
			fuzzpos = 0;

	} while(--count);
}


void V_DrawRaw(int16_t num, uint16_t offset)
{
	const uint8_t *lump = W_TryGetLumpByNum(num);

	if (lump != NULL)
	{
		uint16_t lumpLength = W_LumpLength(num);
		memcpy(&_s_statusbar[offset - (SCREENHEIGHT - ST_HEIGHT) * SCREENWIDTH], lump, lumpLength);
		Z_ChangeTagToCache(lump);
	}
	else
		W_ReadLumpByNum(num, &_s_statusbar[offset - (SCREENHEIGHT - ST_HEIGHT) * SCREENWIDTH]);
}


void ST_Drawer(void)
{
	if (ST_NeedUpdate())
	{
		ST_doRefresh();
		st_needrefresh = 2; //2 screen pages
	}
}


void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t *patch)
{
	y -= patch->topoffset;
	x -= patch->leftoffset;

	byte *desttop = _s_statusbar + (y * SCREENWIDTH) + x - (SCREENHEIGHT - ST_HEIGHT) * SCREENWIDTH;

	int16_t width = patch->width;

	for (int16_t col = 0; col < width; col++, desttop++)
	{
		const column_t *column = (const column_t*)((const byte*)patch + (uint16_t)patch->columnofs[col]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			const byte *source = (const byte*)column + 3;
			byte *dest = desttop + (column->topdelta * SCREENWIDTH);

			uint16_t count = column->length;

			while (count--)
			{
				*dest = *source++;
				dest += SCREENWIDTH;
			}

			column = (const column_t*)((const byte*)column + column->length + 4);
		}
	}
}


segment_t I_ZoneBase(uint32_t *size)
{
	uint32_t paragraphs = 560 * 1024L / PARAGRAPH_SIZE;
	uint8_t *ptr = malloc(paragraphs * PARAGRAPH_SIZE);
	while (!ptr)
	{
		paragraphs--;
		ptr = malloc(paragraphs * PARAGRAPH_SIZE);
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
	printf("%ld bytes allocated for zone\n", *size);
	return D_FP_SEG(ptr);
}


segment_t I_ZoneAdditional(uint32_t *size)
{
	*size = 0;
	return 0;
}


static clock_t starttime;


void I_StartClock(void)
{
	starttime = clock();
}


uint32_t I_EndClock(void)
{
	clock_t endtime = clock();
	return ((endtime - starttime) * TICRATE) / CLOCKS_PER_SEC;
}


void I_Error2(const char *error, ...)
{
	va_list argptr;

	I_ShutdownGraphics();

	va_start(argptr, error);
	vprintf(error, argptr);
	va_end(argptr);
	printf("\n");
	exit(1);
}


int main(int argc, const char * const * argv)
{
	UNUSED(argc);
	UNUSED(argv);

	D_DoomMain();
	return 0;
}

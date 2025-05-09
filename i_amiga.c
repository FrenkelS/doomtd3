/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2024-2025 Frenkel Smeijers
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


#define HORIZONTAL_RESOLUTION_LO	320
#define HORIZONTAL_RESOLUTION_HI	640

#if !defined HORIZONTAL_RESOLUTION
#define HORIZONTAL_RESOLUTION HORIZONTAL_RESOLUTION_HI
#endif

#define PLANEWIDTH			 		(HORIZONTAL_RESOLUTION/8)

#if HORIZONTAL_RESOLUTION == HORIZONTAL_RESOLUTION_LO
#define DDFSTRT_VALUE	0x0038
#define DDFSTOP_VALUE	0x00d0
#define BPLCON0_VALUE	0b0001000000000000
#else
#define DDFSTRT_VALUE	0x003c
#define DDFSTOP_VALUE	0x00d4
#define BPLCON0_VALUE	0b1001000000000000
#endif

#define DW	(HORIZONTAL_RESOLUTION/HORIZONTAL_RESOLUTION_LO)

#if defined VERTICAL_RESOLUTION_DOUBLED
#define DH	2
#else
#define DH	1
#endif

extern struct GfxBase *GfxBase;
extern struct Custom custom;

extern const int16_t CENTERY;

static uint16_t viewwindowtop;
static uint16_t statusbartop;
static uint8_t *_s_viewwindow;
static uint8_t *_s_statusbar;

static uint32_t screenpaget;
static uint8_t *screenpage;

static boolean isGraphicsModeSet = false;


#define FMODE	0x1fc

#define DDFSTRT	0x092
#define DDFSTOP	0x094

#define DIWSTRT				0x08e
#define DIWSTOP				0x090
#define DIWSTRT_VALUE		0x2c81
#define DIWSTOP_VALUE_PAL	0x2cc1
#define DIWSTOP_VALUE_NTSC	0xf4c1

#define BPLCON0	0x100

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
	0x100, 0x300, 0x500, 0x700, 0x800, 0xa00, 0xc00, 0xe00,	// red
	0x110, 0x321, 0x541, 0x652,								// yellow
	0x020													// green
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

	viewwindowtop = ((PLANEWIDTH - VIEWWINDOWWIDTH)      / 2) + ((screenHeightAmiga - (VIEWWINDOWHEIGHT * DH + ST_HEIGHT)) / 2) * PLANEWIDTH;
	statusbartop  = ((PLANEWIDTH - SCREENWIDTH * DW / 8) / 2) + ((screenHeightAmiga - (VIEWWINDOWHEIGHT * DH + ST_HEIGHT)) / 2) * PLANEWIDTH + VIEWWINDOWHEIGHT * DH * PLANEWIDTH;
	_s_viewwindow = screenpage + viewwindowtop;

	_s_statusbar  = Z_MallocStatic(SCREENWIDTH * ST_HEIGHT);

	OwnBlitter();
	WaitBlit();
	custom.bltcon0 = 0b0000100111110000;
	custom.bltcon1 = 0;

	custom.bltamod = PLANEWIDTH - SCREENWIDTH * DW / 8;
	custom.bltdmod = PLANEWIDTH - SCREENWIDTH * DW / 8;

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



#if HORIZONTAL_RESOLUTION == HORIZONTAL_RESOLUTION_LO

#define B0 0
#define B1 1

static const uint8_t VGA_TO_BW_LUT[256] =
{
	B0, B0, B0, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0, B0,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0,
	B1, B1, B1, B1, B1, B1, B0, B0, B1, B1, B1, B1, B1, B1, B0, B0,
	B1, B1, B1, B1, B1, B1, B1, B0, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B0, B0, B0, B0, B0, B0, B0, B0, B1, B1, B1, B1, B1, B0, B0, B1
};

#undef B0
#undef B1
#undef B2

#else

#define B0 0
#define B1 1
#define B2 3

static const uint8_t VGA_TO_BW_LUT_e[256] =
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

#define B0 0
#define B1 2
#define B2 3

static const uint8_t VGA_TO_BW_LUT_o[256] =
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

#endif


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
	if (st_needrefresh)
	{
		st_needrefresh--;

		if (st_needrefresh)
		{
			uint8_t *src = _s_statusbar;
			uint8_t *dst = screenpage + statusbartop;
#if HORIZONTAL_RESOLUTION == HORIZONTAL_RESOLUTION_LO
			for (uint_fast8_t y = 0; y < ST_HEIGHT; y++) {
				for (uint_fast8_t x = 0; x < SCREENWIDTH * DW / 8; x++) {
					uint8_t c =    VGA_TO_BW_LUT[*src++];
					c = (c << 1) | VGA_TO_BW_LUT[*src++];
					c = (c << 1) | VGA_TO_BW_LUT[*src++];
					c = (c << 1) | VGA_TO_BW_LUT[*src++];
					c = (c << 1) | VGA_TO_BW_LUT[*src++];
					c = (c << 1) | VGA_TO_BW_LUT[*src++];
					c = (c << 1) | VGA_TO_BW_LUT[*src++];
					c = (c << 1) | VGA_TO_BW_LUT[*src++];
					*dst++ = c;
				}

				dst += PLANEWIDTH - SCREENWIDTH * DW / 8;
			}
#else
			for (uint_fast8_t y = 0; y < ST_HEIGHT / 2; y++) {
				for (uint_fast8_t x = 0; x < SCREENWIDTH * DW / 8; x++) {
					uint8_t c =    VGA_TO_BW_LUT_e[*src++];
					c = (c << 2) | VGA_TO_BW_LUT_e[*src++];
					c = (c << 2) | VGA_TO_BW_LUT_e[*src++];
					c = (c << 2) | VGA_TO_BW_LUT_e[*src++];
					*dst++ = c;
				}

				dst += PLANEWIDTH - SCREENWIDTH * DW / 8;

				for (uint_fast8_t x = 0; x < (SCREENWIDTH * DW / 8); x++) {
					uint8_t c =    VGA_TO_BW_LUT_o[*src++];
					c = (c << 2) | VGA_TO_BW_LUT_o[*src++];
					c = (c << 2) | VGA_TO_BW_LUT_o[*src++];
					c = (c << 2) | VGA_TO_BW_LUT_o[*src++];
					*dst++ = c;
				}

				dst += PLANEWIDTH - SCREENWIDTH * DW / 8;
			}
#endif
		}
		else
		{
			WaitBlit();

			custom.bltapt = (uint8_t*)(screenpaget - (uint32_t)screenpage) + statusbartop;
			custom.bltdpt = screenpage + statusbartop;

			custom.bltsize = (ST_HEIGHT << 6) | ((SCREENWIDTH * DW / 8) / 2);
		}
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

	uint8_t *dest = _s_viewwindow + (dcvars->yl * PLANEWIDTH * DH) + dcvars->x;

	const uint16_t fracstep = dcvars->fracstep;
	uint16_t frac = (dcvars->texturemid >> COLEXTRABITS) + (dcvars->yl - CENTERY) * fracstep;

	int16_t l = count >> 4;

#if defined VERTICAL_RESOLUTION_DOUBLED
	uint8_t c;
	while (l--)
	{
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;

		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;

		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;

		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
	}

	switch (count & 15)
	{
		case 15: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case 14: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case 13: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case 12: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case 11: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case 10: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case  9: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case  8: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case  7: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case  6: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case  5: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case  4: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case  3: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case  2: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case  1: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c;
	}
#else
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
#endif
}


static uint8_t swapNibbles(uint8_t color)
{
	return (color << 4) | (color >> 4);
}


void R_DrawColumnFlat(uint8_t color, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	if (count <= 0)
		return;

	uint8_t *dest = _s_viewwindow + (dcvars->yl * PLANEWIDTH * DH) + dcvars->x;

	uint8_t color0;
	uint8_t color1;

	if (dcvars->yl & 1)
	{
		color0 = swapNibbles(color);
		color1 = color;
	}
	else
	{
		color0 = color;
		color1 = swapNibbles(color);
	}

#if defined VERTICAL_RESOLUTION_DOUBLED
	int16_t l = count >> 4;

	while (l--)
	{
		*dest = color0; dest += PLANEWIDTH; *dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH; *dest = color1; dest += PLANEWIDTH;
		*dest = color0; dest += PLANEWIDTH; *dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH; *dest = color1; dest += PLANEWIDTH;

		*dest = color0; dest += PLANEWIDTH; *dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH; *dest = color1; dest += PLANEWIDTH;
		*dest = color0; dest += PLANEWIDTH; *dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH; *dest = color1; dest += PLANEWIDTH;

		*dest = color0; dest += PLANEWIDTH; *dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH; *dest = color1; dest += PLANEWIDTH;
		*dest = color0; dest += PLANEWIDTH; *dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH; *dest = color1; dest += PLANEWIDTH;

		*dest = color0; dest += PLANEWIDTH; *dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH; *dest = color1; dest += PLANEWIDTH;
		*dest = color0; dest += PLANEWIDTH; *dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH; *dest = color1; dest += PLANEWIDTH;
	}

	switch (count & 15)
	{
		case 15: dest[PLANEWIDTH * 29] = color0; dest[PLANEWIDTH * 28] = color0;
		case 14: dest[PLANEWIDTH * 27] = color1; dest[PLANEWIDTH * 26] = color1;
		case 13: dest[PLANEWIDTH * 25] = color0; dest[PLANEWIDTH * 24] = color0;
		case 12: dest[PLANEWIDTH * 23] = color1; dest[PLANEWIDTH * 22] = color1;
		case 11: dest[PLANEWIDTH * 21] = color0; dest[PLANEWIDTH * 20] = color0;
		case 10: dest[PLANEWIDTH * 19] = color1; dest[PLANEWIDTH * 18] = color1;
		case  9: dest[PLANEWIDTH * 17] = color0; dest[PLANEWIDTH * 16] = color0;
		case  8: dest[PLANEWIDTH * 15] = color1; dest[PLANEWIDTH * 14] = color1;
		case  7: dest[PLANEWIDTH * 13] = color0; dest[PLANEWIDTH * 12] = color0;
		case  6: dest[PLANEWIDTH * 11] = color1; dest[PLANEWIDTH * 10] = color1;
		case  5: dest[PLANEWIDTH *  9] = color0; dest[PLANEWIDTH *  8] = color0;
		case  4: dest[PLANEWIDTH *  7] = color1; dest[PLANEWIDTH *  6] = color1;
		case  3: dest[PLANEWIDTH *  5] = color0; dest[PLANEWIDTH *  4] = color0;
		case  2: dest[PLANEWIDTH *  3] = color1; dest[PLANEWIDTH *  2] = color1;
		case  1: dest[PLANEWIDTH *  1] = color0; dest[PLANEWIDTH *  0] = color0;
	}
#else
	int16_t l = count >> 4;

	while (l--)
	{
		*dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH;
		*dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH;

		*dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH;
		*dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH;

		*dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH;
		*dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH;

		*dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH;
		*dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH;
	}

	switch (count & 15)
	{
		case 15: dest[PLANEWIDTH * 14] = color0;
		case 14: dest[PLANEWIDTH * 13] = color1;
		case 13: dest[PLANEWIDTH * 12] = color0;
		case 12: dest[PLANEWIDTH * 11] = color1;
		case 11: dest[PLANEWIDTH * 10] = color0;
		case 10: dest[PLANEWIDTH *  9] = color1;
		case  9: dest[PLANEWIDTH *  8] = color0;
		case  8: dest[PLANEWIDTH *  7] = color1;
		case  7: dest[PLANEWIDTH *  6] = color0;
		case  6: dest[PLANEWIDTH *  5] = color1;
		case  5: dest[PLANEWIDTH *  4] = color0;
		case  4: dest[PLANEWIDTH *  3] = color1;
		case  3: dest[PLANEWIDTH *  2] = color0;
		case  2: dest[PLANEWIDTH *  1] = color1;
		case  1: dest[PLANEWIDTH *  0] = color0;
	}
#endif
}


#define FUZZCOLOR1 0x00
#define FUZZCOLOR2 0x02
#define FUZZCOLOR3 0x20
#define FUZZCOLOR4 0x22
#define FUZZTABLE 50

static const int8_t fuzzcolors[FUZZTABLE] =
{
	FUZZCOLOR1,FUZZCOLOR2,FUZZCOLOR3,FUZZCOLOR4,FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR2,
	FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR4,FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR1,FUZZCOLOR2,
	FUZZCOLOR3,FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR4,FUZZCOLOR2,FUZZCOLOR4,FUZZCOLOR2,
	FUZZCOLOR1,FUZZCOLOR4,FUZZCOLOR2,FUZZCOLOR3,FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR1,FUZZCOLOR4,
	FUZZCOLOR3,FUZZCOLOR2,FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR4,FUZZCOLOR2,FUZZCOLOR1,
	FUZZCOLOR3,FUZZCOLOR4,FUZZCOLOR2,FUZZCOLOR4,FUZZCOLOR2,FUZZCOLOR1,FUZZCOLOR3,
	FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR4,FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR2,FUZZCOLOR1
};


void R_DrawFuzzColumn(const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	uint8_t *dest = _s_viewwindow + (dcvars->yl * PLANEWIDTH * DH) + dcvars->x;

	static int16_t fuzzpos = 0;

	do
	{
#if defined VERTICAL_RESOLUTION_DOUBLED
		dest[PLANEWIDTH * 0] =
		dest[PLANEWIDTH * 1] = fuzzcolors[fuzzpos];
#else
		dest[PLANEWIDTH * 0] = fuzzcolors[fuzzpos];
#endif
		dest += PLANEWIDTH * DH;

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
	uint32_t paragraphs = 480 * 1024L / PARAGRAPH_SIZE;
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

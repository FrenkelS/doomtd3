/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2023-2024 Frenkel Smeijers
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
 *      Video code for the IBM PC
 *
 *-----------------------------------------------------------------------------*/
 
#include <conio.h>
#include <dos.h>
#include <stdint.h>

#include "compiler.h"

#include "i_system.h"
#include "i_video.h"
#include "m_random.h"
#include "r_defs.h"
#include "v_video.h"
#include "w_wad.h"

#include "globdata.h"


#define PLANEWIDTH 80

 
extern const int16_t CENTERY;

static uint8_t __far* _s_viewwindow;
static uint8_t __far* _s_statusbar;
static uint8_t __far* videomemory;


static void I_SetScreenMode(uint16_t mode)
{
	union REGS regs;
	regs.w.ax = mode;
	int86(0x10, &regs, &regs);
}


void I_InitGraphicsHardwareSpecificCode(void)
{
	I_SetScreenMode(6);

	__djgpp_nearptr_enable();
	videomemory = D_MK_FP(0xb800, ((SCREENWIDTH_VGA - SCREENWIDTH) / 2) / 4 + (((SCREENHEIGHT_VGA - SCREENHEIGHT) / 2) * PLANEWIDTH) / 2 + __djgpp_conventional_base);

	_s_viewwindow = Z_MallocStatic(VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT);
	_s_statusbar = Z_MallocStatic(SCREENWIDTH * ST_HEIGHT);
}


void I_ShutdownGraphics(void)
{
	I_SetScreenMode(3);
}


void I_SetPalette(int8_t pal)
{

}


void I_FinishUpdate(void)
{
	uint8_t __far* src = _s_viewwindow;
	uint8_t __far* dst = videomemory;

	for (uint_fast8_t y = 0; y < (SCREENHEIGHT - ST_HEIGHT) / 2; y++) {
		_fmemcpy(dst, src, VIEWWINDOWWIDTH);
		dst += 0x2000;
		src += VIEWWINDOWWIDTH;

		for (uint_fast8_t x = 0; x < VIEWWINDOWWIDTH; x++) {
			uint8_t b = *src++;
			*dst++ = (b << 4 | b >> 4);
		}

		dst -= 0x2000 - (PLANEWIDTH - VIEWWINDOWWIDTH);
	}

#if !defined DISABLE_STATUS_BAR
	src = _s_statusbar;
	for (uint_fast8_t y = (SCREENHEIGHT - ST_HEIGHT) / 2; y < SCREENHEIGHT / 2; y++) {
		for (uint_fast8_t x = 0; x < VIEWWINDOWWIDTH; x++) {
			*dst++ = *src;
			src += 4;
		}

		dst += 0x2000 - VIEWWINDOWWIDTH;

		for (uint_fast8_t x = 0; x < VIEWWINDOWWIDTH; x++) {
			uint8_t b = *src;
			*dst++ = (b << 4 | b >> 4);
			src += 4;
		}

		dst -= 0x2000 - (PLANEWIDTH - VIEWWINDOWWIDTH);
	}
#endif
}


#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)

static uint8_t nearcolormap[256];
static uint16_t nearcolormapoffset = 0xffff;


void R_DrawColumn(const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	const uint8_t __far* source = dcvars->source;

	if (nearcolormapoffset != D_FP_OFF(dcvars->colormap))
	{
		_fmemcpy(nearcolormap, dcvars->colormap, 256);
		nearcolormapoffset = D_FP_OFF(dcvars->colormap);
	}

	uint8_t __far* dest = _s_viewwindow + (dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x;

	const uint16_t fracstep = (dcvars->iscale >> COLEXTRABITS);
	uint16_t frac = (dcvars->texturemid + (dcvars->yl - CENTERY) * dcvars->iscale) >> COLEXTRABITS;

	while (count--)
	{
		*dest = nearcolormap[source[frac>>COLBITS]];
		dest += VIEWWINDOWWIDTH;
		frac += fracstep;
	}
}


void R_DrawColumnFlat(int16_t texture, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	const uint8_t color = texture;

	uint8_t __far* dest = _s_viewwindow + (dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x;

	while (count--)
	{
		*dest = color;
		dest += VIEWWINDOWWIDTH;
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

	if (nearcolormapoffset != D_FP_OFF(&fullcolormap[6 * 256]))
	{
		_fmemcpy(nearcolormap, &fullcolormap[6 * 256], 256);
		nearcolormapoffset = D_FP_OFF(&fullcolormap[6 * 256]);
	}

	uint8_t __far* dest = _s_viewwindow + (dc_yl * VIEWWINDOWWIDTH) + dcvars->x;

	static int16_t fuzzpos = 0;

	do
	{
		*dest = nearcolormap[dest[fuzzoffset[fuzzpos] * 4]];
		dest += VIEWWINDOWWIDTH;

		fuzzpos++;
		if (fuzzpos >= FUZZTABLE)
			fuzzpos = 0;

	} while(--count);
}


void V_FillRect(byte colour)
{

}


void V_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
{

}


void V_DrawBackground(void)
{

}


void V_DrawRaw(int16_t num, uint16_t offset)
{
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


void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	y -= patch->topoffset;
	x -= patch->leftoffset;

	byte __far* desttop = _s_statusbar + (y * SCREENWIDTH) + x - (SCREENHEIGHT - ST_HEIGHT) * SCREENWIDTH;

	int16_t width = patch->width;

	for (int16_t col = 0; col < width; col++, desttop++)
	{
		const column_t __far* column = (const column_t __far*)((const byte __far*)patch + patch->columnofs[col]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			const byte __far* source = (const byte __far*)column + 3;
			byte __far* dest = desttop + (column->topdelta * SCREENWIDTH);

			uint16_t count = column->length;

			uint16_t l = count >> 2;
			while (l--)
			{
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
			}

			switch (count & 3)
			{
				case 3: *dest = *source++; dest += SCREENWIDTH;
				case 2: *dest = *source++; dest += SCREENWIDTH;
				case 1: *dest = *source++;
			}

			column = (const column_t __far*)((const byte __far*)column + column->length + 4);
		}
	}
}


void V_DrawPatchScaled(int16_t x, int16_t y, const patch_t __far* patch)
{

}


void wipe_StartScreen(void)
{

}


void D_Wipe(void)
{

}

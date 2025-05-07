/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2025 Frenkel Smeijers
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
 *      Code specific to the Atari ST
 *
 *-----------------------------------------------------------------------------*/

#include <osbind.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>

#include "compiler.h"

#include "d_main.h"
#include "i_system.h"
#include "w_wad.h"


#define PLANEWIDTH	80


extern const int16_t CENTERY;

static uint8_t _s_viewwindow[VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT];
static uint16_t *videomemory;

static boolean isGraphicsModeSet = false;


static const uint16_t colors[14] =
{
	0x000,													// normal
	0x100, 0x300, 0x500, 0x700, 0x800, 0xa00, 0xc00, 0xe00,	// red
	0x110, 0x321, 0x541, 0x652,								// yellow
	0x020													// green
};


static void I_UploadNewPalette(int8_t pal)
{
	Setcolor(0, colors[pal]);
}


void I_InitGraphics(void)
{
	Setscreen(-1L, -1L, 1);
	Setcolor(0, 0x000);
	Setcolor(1, 0x777);

	videomemory = Physbase();

	isGraphicsModeSet = true;
}


static void I_ShutdownGraphics(void)
{
	if (isGraphicsModeSet)
	{
		Setscreen(-1L, -1L, 0);
		Setcolor(0, 0x777);
		Setcolor(1, 0x000);
	}
}


static int8_t newpal;


void I_SetPalette(int8_t pal)
{
	newpal = pal;
}


#define NO_PALETTE_CHANGE 100


void I_FinishUpdate(void)
{
	// palette
	if (newpal != NO_PALETTE_CHANGE)
	{
		I_UploadNewPalette(newpal);
		newpal = NO_PALETTE_CHANGE;
	}

	// view window
	uint16_t *src = (uint16_t*)_s_viewwindow;
	uint16_t *dst = videomemory;

	for (uint_fast8_t y = 0; y < VIEWWINDOWHEIGHT; y++) {
		for (uint_fast16_t x = 0; x < VIEWWINDOWWIDTH / 2; x++) {
			*dst = *src++;
			dst+=2;
		}

		dst += 20;
	}
}


void R_InitColormaps(void)
{
	fullcolormap = W_GetLumpByName("COLORMAP"); // Never freed
}


#define COLEXTRABITS	(8 - 1)
#define COLBITS			(8 + 1)


void R_DrawColumn(const draw_column_vars_t *dcvars)
{
	const int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	const uint8_t *source = dcvars->source;

	const uint8_t *colormap = dcvars->colormap;

	uint8_t *dest = &_s_viewwindow[(dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x];

	const uint16_t fracstep = dcvars->fracstep;
	uint16_t frac = (dcvars->texturemid >> COLEXTRABITS) + (dcvars->yl - CENTERY) * fracstep;

	int16_t l = count >> 4;

	while (l--)
	{
		*dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;

		*dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;

		*dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;

		*dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
	}

	switch (count & 15)
	{
		case 15: *dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 14: *dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 13: *dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 12: *dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 11: *dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 10: *dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  9: *dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  8: *dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  7: *dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  6: *dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  5: *dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  4: *dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  3: *dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  2: *dest = colormap[source[frac>>COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  1: *dest = colormap[source[frac>>COLBITS]];
	}
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

	uint8_t *dest = &_s_viewwindow[(dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x];

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

	int16_t l = count >> 4;

	while (l--)
	{
		*dest = color0; dest += VIEWWINDOWWIDTH;
		*dest = color1; dest += VIEWWINDOWWIDTH;
		*dest = color0; dest += VIEWWINDOWWIDTH;
		*dest = color1; dest += VIEWWINDOWWIDTH;

		*dest = color0; dest += VIEWWINDOWWIDTH;
		*dest = color1; dest += VIEWWINDOWWIDTH;
		*dest = color0; dest += VIEWWINDOWWIDTH;
		*dest = color1; dest += VIEWWINDOWWIDTH;

		*dest = color0; dest += VIEWWINDOWWIDTH;
		*dest = color1; dest += VIEWWINDOWWIDTH;
		*dest = color0; dest += VIEWWINDOWWIDTH;
		*dest = color1; dest += VIEWWINDOWWIDTH;

		*dest = color0; dest += VIEWWINDOWWIDTH;
		*dest = color1; dest += VIEWWINDOWWIDTH;
		*dest = color0; dest += VIEWWINDOWWIDTH;
		*dest = color1; dest += VIEWWINDOWWIDTH;
	}

	switch (count & 15)
	{
		case 15: dest[VIEWWINDOWWIDTH * 14] = color0;
		case 14: dest[VIEWWINDOWWIDTH * 13] = color1;
		case 13: dest[VIEWWINDOWWIDTH * 12] = color0;
		case 12: dest[VIEWWINDOWWIDTH * 11] = color1;
		case 11: dest[VIEWWINDOWWIDTH * 10] = color0;
		case 10: dest[VIEWWINDOWWIDTH *  9] = color1;
		case  9: dest[VIEWWINDOWWIDTH *  8] = color0;
		case  8: dest[VIEWWINDOWWIDTH *  7] = color1;
		case  7: dest[VIEWWINDOWWIDTH *  6] = color0;
		case  6: dest[VIEWWINDOWWIDTH *  5] = color1;
		case  5: dest[VIEWWINDOWWIDTH *  4] = color0;
		case  4: dest[VIEWWINDOWWIDTH *  3] = color1;
		case  3: dest[VIEWWINDOWWIDTH *  2] = color0;
		case  2: dest[VIEWWINDOWWIDTH *  1] = color1;
		case  1: dest[VIEWWINDOWWIDTH *  0] = color0;
	}
}


void R_DrawFuzzColumn(const draw_column_vars_t *dcvars)
{

}


void V_DrawRaw(int16_t num, uint16_t offset)
{

}


void ST_Drawer(void)
{

}


void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t *patch)
{

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

	Setcolor(0, 0x777);

	va_start(argptr, error);
	vfprintf(stdout, error, argptr);
	va_end(argptr);

	printf("\n");
	while (!Bconstat(2)) {}

	I_ShutdownGraphics();

	exit(0);
}


int main(int argc, const char * const * argv)
{
	UNUSED(argc);
	UNUSED(argv);

	D_DoomMain();
	return 0;
}

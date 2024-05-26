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
 *      Code specific to the HP 95LX 240x128 graphics mode
 *
 *-----------------------------------------------------------------------------*/

#include <dos.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>

#include "compiler.h"

#include "d_main.h"
#include "i_system.h"
#include "w_wad.h"


extern const int16_t CENTERY;

static uint8_t _s_viewwindow[VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT];
static uint8_t __far* videomemory;

static boolean isGraphicsModeSet = false;


static void I_SetScreenMode(uint16_t mode)
{
	union REGS regs;
	regs.w.ax = mode;
	int86(0x10, &regs, &regs);
}


void I_InitGraphics(void)
{
#if defined EGA_DEBUG
	I_SetScreenMode(0x0d);

	videomemory = D_MK_FP(0xa000, 0);
#else
	I_SetScreenMode(0x20);

	videomemory = D_MK_FP(0xb000, 0);
#endif

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


void I_FinishUpdate(void)
{
	// view window
#if defined EGA_DEBUG
#define PLANEWIDTH 40
	uint8_t *src = &_s_viewwindow[0];
	uint8_t __far* dst = videomemory;

	for (uint_fast8_t y = 0; y < VIEWWINDOWHEIGHT; y++) {
		_fmemcpy(dst, src, VIEWWINDOWWIDTH);

		dst += PLANEWIDTH;
		src += VIEWWINDOWWIDTH;
	}
#else
	_fmemcpy(videomemory, &_s_viewwindow[0], VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT);
#endif
}


void R_InitColormaps(void)
{
	int16_t num = W_GetNumForName("COLORMAP");
	fullcolormap = W_GetLumpByNum(num); // Never freed

#if !defined EGA_DEBUG
	uint16_t length = W_LumpLength(num);
	uint8_t __far* ptr = (uint8_t __far*) fullcolormap;
	for (int i = 0; i < length; i++)
	{
		uint8_t b = *ptr;
		*ptr++ = ~b;
	}
#endif
}


#define COLEXTRABITS (8 - 1)

uint8_t nearcolormap[256];

static uint16_t nearcolormapoffset = 0xffff;

const uint8_t __far* source;
const uint8_t *dest;


void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count);


void R_DrawColumn(const draw_column_vars_t *dcvars)
{
	const int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	source = dcvars->source;

	if (nearcolormapoffset != D_FP_OFF(dcvars->colormap))
	{
		_fmemcpy(nearcolormap, dcvars->colormap, 256);
		nearcolormapoffset = D_FP_OFF(dcvars->colormap);
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

	if (nearcolormapoffset != D_FP_OFF(&fullcolormap[6 * 256]))
	{
		_fmemcpy(nearcolormap, &fullcolormap[6 * 256], 256);
		nearcolormapoffset = D_FP_OFF(&fullcolormap[6 * 256]);
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

}


void ST_Drawer(void)
{

}


void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch)
{

}


segment_t I_ZoneBase(uint32_t *size)
{
	unsigned int max, segment;
	_dos_allocmem(0xffff, &max);
	_dos_allocmem(max, &segment);
	*size = (uint32_t)max * PARAGRAPH_SIZE;
	printf("Standard: %ld bytes allocated for zone\n", *size);
	return segment;
}


#define	EMS_INT			0x67

#define	EMS_STATUS		0x40
#define	EMS_GETFRAME	0x41
#define	EMS_GETPAGES	0x42
#define	EMS_ALLOCPAGES	0x43
#define	EMS_MAPPAGE		0x44
#define	EMS_FREEPAGES	0x45
#define	EMS_VERSION		0x46

static uint16_t emsHandle;

segment_t I_ZoneAdditional(uint32_t *size)
{
	*size = 0;

	segment_t __far* emsInterruptVectorSegment = D_MK_FP(0, EMS_INT * 4 + 2);
	uint64_t __far* actualEmsDeviceName = D_MK_FP(*emsInterruptVectorSegment, 0x000a);
	uint64_t expectedEmsDeviceName = *(uint64_t*)"EMMXXXX0";
	if (*actualEmsDeviceName != expectedEmsDeviceName)
		return 0;

	// EMS detected

	union REGS regs;
	regs.h.ah = EMS_STATUS;
	int86(EMS_INT, &regs, &regs);
	if (regs.h.ah)
		return 0;

	// EMS status is successful

	regs.h.ah = EMS_VERSION;
	int86(EMS_INT, &regs, &regs);
	if (regs.h.ah || regs.h.al < 0x32)
		return 0;

	// EMS v3.2 or higher detected

	regs.h.ah = EMS_GETFRAME;
	int86(EMS_INT, &regs, &regs);
	if (regs.h.ah)
		return 0;

	// EMS page frame address
	segment_t emsSegment = regs.w.bx;

	regs.h.ah = EMS_GETPAGES;
	int86(EMS_INT, &regs, &regs);
	if (regs.h.ah || regs.w.bx < 4)
		return 0;

	// There are at least 4 unallocated pages

	regs.h.ah = EMS_ALLOCPAGES;
	regs.w.bx = 4;
	int86(EMS_INT, &regs, &regs);
	if (regs.h.ah)
		return 0;

	// 4 logical pages are allocated

	emsHandle = regs.w.dx;

	for (int16_t pageNumber = 0; pageNumber < 4; pageNumber++)
	{
		regs.h.ah = EMS_MAPPAGE;
		regs.h.al = pageNumber;	// physical page number
		regs.w.bx = pageNumber;	//  logical page number
		regs.w.dx = emsHandle;
		int86(EMS_INT, &regs, &regs);
		if (regs.h.ah)
			return 0;
	}
	
	// 64 kB of expanded memory is mapped
	*size = 65536;
	printf("Expanded:  65536 bytes allocated for zone\n");
	return emsSegment;
}


static void I_ShutdownEMS(void)
{
	if (emsHandle)
	{
		union REGS regs;
		regs.h.ah = EMS_FREEPAGES;
		regs.w.dx = emsHandle;
		int86(EMS_INT, &regs, &regs);
	}
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
	I_ShutdownEMS();

	va_start(argptr, error);
	vprintf(error, argptr);
	va_end(argptr);
	printf("\n");
	exit(1);
}


int main(int argc, const char * const * argv)
{
	D_DoomMain();
	return 0;
}

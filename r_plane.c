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
 *      Render floor and ceilings
 *
 *-----------------------------------------------------------------------------*/

#include <stdint.h>

#include "compiler.h"
#include "r_defs.h"
#include "r_main.h"

#include "globdata.h"


int16_t skyflatnum;


static int16_t firstflat;
static int16_t  animated_flat_basepic;
static int16_t __far* flattranslation;             // for global animation


//
// R_DrawSpan
// With DOOM style restrictions on view orientation,
//  the floors and ceilings consist of horizontal slices
//  or spans with constant z depth.
// However, rotation around the world z axis is possible,
//  thus this mapping, while simpler and faster than
//  perspective correct texture mapping, has to traverse
//  the texture at an angle in all but a few cases.
// In consequence, flats are not stored by column (like walls),
//  and the inner loop has to step in texture space u and v.
//

byte R_GetPlaneColor(int16_t picnum, int16_t lightlevel)
{
	const uint8_t __far* colormap = R_LoadColorMap(lightlevel);
	return colormap[flattranslation[picnum]];
}


//
// R_InitFlats
//

void R_InitFlats(void)
{
	firstflat        = W_GetNumForName("F_START") + 1;

	int16_t lastflat = W_GetNumForName("F_END")   - 1;
	int16_t numflats = lastflat - firstflat + 1;

	// Create translation table for global animation.

	flattranslation = Z_MallocStatic((numflats + 1) * sizeof(*flattranslation));

	for (int16_t i = 0; i < numflats; i++)
	{
		byte b;
		W_ReadLumpByNum(firstflat + i, &b);
		flattranslation[i] = b;
	}
}


void R_InitSky(void)
{
	// First thing, we have a dummy sky texture name,
	//  a flat. The data is in the WAD only because
	//  we look for an actual index, instead of simply
	//  setting one.
	skyflatnum = R_FlatNumForName("F_SKY1");
}


//
// R_FlatNumForName
// Retrieval, get a flat number for a flat name.
//
//

int16_t R_FlatNumForName(const char *name)
{
	int16_t i = W_GetNumForName(name);
	return i - firstflat;
}

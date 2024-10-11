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


#define FLAT_NUKAGE1_COLOR 122


const int16_t skyflatnum = -2;


byte R_GetPlaneColor(int16_t picnum, int16_t lightlevel)
{
	const uint8_t __far* colormap = R_LoadColorMap(lightlevel);

	if (picnum == -3)
		picnum = FLAT_NUKAGE1_COLOR;

	return colormap[picnum];
}

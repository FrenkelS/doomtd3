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
 *      Scrollers code
 *
 *-----------------------------------------------------------------------------*/

#include "globdata.h"


typedef struct {
	thinker_t thinker;				// Thinker structure for scrolling
	int16_t __far* textureoffset;	// Affected textureoffset
} scroll_t;


static void T_Scroll(scroll_t __far* s)
{
	(*s->textureoffset)++;
}


//
// Add_Scroller()
//
// Add a generalized scroller to the thinker list.
//
// affectee: the index of the affected sidedef
//

static void Add_Scroller(int16_t affectee)
{
	scroll_t __far* s = Z_CallocLevSpec(sizeof *s);
	s->thinker.function = T_Scroll;
	s->textureoffset = &_g_sides[affectee].textureoffset;
	P_AddThinker(&s->thinker);
}


// Initialize the scrollers
void P_SpawnScrollers(void)
{
	const line_t __far* line = _g_lines;

	for (int16_t i = 0; i < _g_numlines; i++)
	{
		if (LN_SPECIAL(line) == 48)
			Add_Scroller(line->sidenum[0]);
		line++;
	}
}

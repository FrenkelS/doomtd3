/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *  Copyright 2023, 2024 by
 *  Frenkel Smeijers
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
 *      System specific interface stuff.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __I_SYSTEM__
#define __I_SYSTEM__

#include "config.h"

#include "r_defs.h"
#include "r_main.h"


#define PARAGRAPH_SIZE 16


#if defined __m68k__
#define I_Error(...) I_Error2("\p"__VA_ARGS__)
#else
#define I_Error(...) I_Error2(__VA_ARGS__)
#endif
void NORETURN_PRE I_Error2(const char *error, ...) NORETURN_POST;

void I_InitGraphics(void);
void I_SetPalette(int8_t pal);
void I_FinishUpdate(void);


void R_DrawColumn(const draw_column_vars_t *dcvars);
void R_DrawColumnFlat(int16_t texture, const draw_column_vars_t *dcvars);
void R_DrawFuzzColumn(const draw_column_vars_t *dcvars);


void V_DrawRaw(int16_t num, uint16_t offset);
void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch);


unsigned int I_ZoneBase(unsigned int *size);


void I_StartClock(void);
uint32_t I_EndClock(void);

#endif

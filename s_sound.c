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
 * DESCRIPTION:  Platform-independent sound code
 *
 *-----------------------------------------------------------------------------*/

// killough 3/7/98: modified to allow arbitrary listeners in spy mode
// killough 5/2/98: reindented, removed useless code, beautified

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "d_player.h"
#include "s_sound.h"
#include "i_system.h"
#include "d_main.h"
#include "r_main.h"
#include "m_random.h"
#include "w_wad.h"

#include "globdata.h"


typedef struct
{
  const sfxinfo_t *sfxinfo;  // sound information (if null, channel avail.)
  int32_t tickend;
  void __far* origin;        // origin of sound
  int16_t handle;          // handle of the sound being played
  boolean is_pickup;       // whether sound is a player's weapon
} channel_t;


// when to clip out sounds
// Does not fit the large outdoor areas.
#define S_CLIPPING_DIST (1200L<<FRACBITS)

// Distance tp origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// Originally: (200*0x10000).

#define S_CLOSE_DIST (160L<<FRACBITS)
#define S_ATTENUATOR ((S_CLIPPING_DIST-S_CLOSE_DIST)>>FRACBITS)

// Adjustable by menu.
#define NORM_PRIORITY 64
#define NORM_SEP      128

#define S_STEREO_SWING		96


// These are not used, but should be (menu).
// Maximum volume of a sound effect.
// Internal default is max out of 0-15.
int16_t snd_SfxVolume = 15;

// Maximum volume of music. Useless so far.
int16_t snd_MusicVolume = 15;


void S_StartSound(mobj_t __far* origin, sfxenum_t sfx_id)
{
	UNUSED(origin);
	UNUSED(sfx_id);
}

void S_StartSound2(degenmobj_t __far* origin, sfxenum_t sfx_id)
{
	UNUSED(sfx_id);

    //Look at this mess.

    //Originally, the degenmobj_t had
    //a thinker_t at the start of the struct
    //so that it could be passed around and
    //cast to a mobj_t* in the sound code
    //for non-mobj sound makers like doors.

    //This also meant that each and every sector_t
    //struct has 24 bytes wasted. I can't afford
    //to waste memory like that so we have a seperate
    //function for these cases which cobbles toget a temp
    //mobj_t-like struct to pass to the sound code.


    static struct fake_mobj
    {
        thinker_t ununsed;
        degenmobj_t origin;
    } __far fm;

    fm.origin.x = origin->x;
    fm.origin.y = origin->y;
}

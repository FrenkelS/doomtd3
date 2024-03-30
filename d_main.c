/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2004 by
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
 *  DOOM main program (D_DoomMain) and game loop (D_DoomLoop),
 *  plus functions to
 *  parse command line parameters, configure game parameters (turbo),
 *  and call the startup functions.
 *
 *-----------------------------------------------------------------------------
 */

#include <stdint.h>

#include "doomdef.h"
#include "doomtype.h"
#include "d_player.h"
#include "d_englsh.h"
#include "sounds.h"
#include "z_zone.h"
#include "w_wad.h"
#include "v_video.h"
#include "i_system.h"
#include "i_timer.h"
#include "g_game.h"
#include "st_stuff.h"
#include "p_setup.h"
#include "r_main.h"
#include "d_main.h"
#include "globdata.h"


static int32_t maketic;


static void D_BuildNewTiccmds(void)
{
    static int32_t lastmadetic = 0;
    int32_t newtics = I_GetTime() - lastmadetic;
    lastmadetic += newtics;

    while (newtics--)
    {
        if (maketic - _g_gametic > 3)
            break;

        G_BuildTiccmd();
        maketic++;
    }
}


//
// D_Display
//  draw current display, possibly wiping it from the previous
//

static void D_Display (void)
{
    if (_g_gametic != _g_basetic)
    { // In a level

        // Now do the drawing
        R_RenderPlayerView (&_g_player);

        ST_doPaletteStuff();
        ST_Drawer();
    }

    D_BuildNewTiccmds();

    // normal update
    I_FinishUpdate ();              // page flip or blit buffer
}


//
//  D_DoomLoop()
//
// Not a globally visible function,
//  just included for source reference,
//  called by D_DoomMain, never exits.
// Manages timing and IO,
//  calls all ?_Responder, ?_Ticker, and ?_Drawer,
//  calls I_GetTime
//
static void NORETURN_PRE D_DoomLoop(void) NORETURN_POST;
static void D_DoomLoop(void)
{
    for (;;)
    {
        // frame syncronous IO operations

        // process one or more tics
        G_BuildTiccmd ();

        G_Ticker ();

        _g_gametic++;
        maketic++;

        // Update display, next frame, with current state.
        D_Display();
    }
}


static void D_InitNetGame (void)
{
    _g_playeringame = true;
}


//
// D_DoomMainSetup
//
// CPhipps - the old contents of D_DoomMain, but moved out of the main
//  line of execution so its stack space can be freed

static void D_DoomMainSetup(void)
{
    // init subsystems

    Z_Init();

    G_ReloadDefaults();    // killough 3/4/98: set defaults just loaded.

    // CPhipps - move up netgame init
    D_InitNetGame();

    W_Init(); // CPhipps - handling of wadfiles init changed

    R_Init();

    P_Init();

    ST_Init();

    I_InitGraphics();

    G_DeferedPlayDemo();
}

//
// D_DoomMain
//

void D_DoomMain(void)
{
    D_DoomMainSetup(); // CPhipps - setup out of main execution stack

    D_DoomLoop ();  // never returns
}

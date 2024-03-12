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
 * DESCRIPTION:  none
 *  The original Doom description was none, basically because this file
 *  has everything. This ties up the game logic, linking the menu and
 *  input code to the underlying game by creating & respawning players,
 *  building game tics, calling the underlying thing logic.
 *
 *-----------------------------------------------------------------------------
 */

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "compiler.h"
#include "d_player.h"
#include "doomtype.h"
#include "m_random.h"
#include "p_setup.h"
#include "p_tick.h"
#include "p_map.h"
#include "d_main.h"
#include "st_stuff.h"
#include "w_wad.h"
#include "r_main.h"
#include "p_map.h"
#include "s_sound.h"
#include "d_englsh.h"
#include "sounds.h"
#include "r_data.h"
#include "m_fixed.h"
#include "p_inter.h"
#include "g_game.h"
#include "i_system.h"

#include "globdata.h"


static const byte __far* demobuffer;   /* cph - only used for playback */
static uint16_t demolength; // check for overrun (missing DEMOMARKER)

static const byte __far* demo_p;

static gameaction_t    _s_gameaction;
gamestate_t     _g_gamestate;
skill_t         _g_gameskill;

static int16_t             _s_gamemap;

player_t        _g_player;

int32_t             _g_gametic;
int32_t             _g_basetic;       /* killough 9/29/98: for demo sync */
int32_t             _g_totalkills, _g_totallive, _g_totalitems, _g_totalsecret;    // for intermission
wbstartstruct_t _g_wminfo;               // parms for world map / intermission
static int32_t             totalleveltimes;      // CPhipps - total time for all completed levels


static boolean gamekeydown[NUMKEYS];

boolean         _g_playeringame;
boolean         _g_demoplayback;


//
// controls (have defaults)
//

static const int16_t key_right       = KEYD_RIGHT;
static const int16_t key_left        = KEYD_LEFT;
static const int16_t key_up          = KEYD_UP;
static const int16_t key_down        = KEYD_DOWN;
static const int16_t key_strafeleft  = KEYD_L;
static const int16_t key_straferight = KEYD_R;
static const int16_t key_fire        = KEYD_B; 
static const int16_t key_use         = KEYD_A;
static const int16_t key_weapon_up   = KEYD_BRACKET_RIGHT;
static const int16_t key_weapon_down = KEYD_BRACKET_LEFT;


#define MAXPLMOVE   (forwardmove[1])
#define SLOWTURNTICS  6

static const int8_t forwardmove[2] = {0x19, 0x32};
static const int8_t sidemove[2]    = {0x18, 0x28};
static const int16_t angleturn[3]  = {640, 1280, 320};  // + slow turn

static void G_DoPlayDemo(void);
static void G_InitNew(skill_t skill, int16_t map);
static void G_ReadDemoTiccmd (void);


//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs
// or reads it from the demo buffer.
// If recording a demo, write it out
//
static inline int8_t fudgef(int8_t b)
{
    static int16_t c;
    if (!b) return b;
    if (++c & 0x1f) return b;
    b |= 1; if (b>2) b-=2;
    return b;
}

static ticcmd_t netcmd;

void G_BuildTiccmd(void)
{
    static int16_t     turnheld = 0;       // for accelerative turning

    int16_t speed;
    int16_t tspeed;
    int16_t forward;
    int16_t side;
    weapontype_t newweapon;
    /* cphipps - remove needless I_BaseTiccmd call, just set the ticcmd to zero */
    memset(&netcmd,0,sizeof(ticcmd_t));

    //Use button negates the always run setting.
    speed = gamekeydown[key_use];

    forward = side = 0;

    // use two stage accelerative turning
    // on the keyboard
    if (gamekeydown[key_right] || gamekeydown[key_left])
        turnheld++;
    else
        turnheld = 0;

    if (turnheld < SLOWTURNTICS)
        tspeed = 2;             // slow turn
    else
        tspeed = speed;

    // let movement keys cancel each other out

    if (gamekeydown[key_right])
        netcmd.angleturn -= angleturn[tspeed];
    if (gamekeydown[key_left])
        netcmd.angleturn += angleturn[tspeed];

    if (gamekeydown[key_up])
        forward += forwardmove[speed];
    if (gamekeydown[key_down])
        forward -= forwardmove[speed];

    if (gamekeydown[key_straferight])
        side += sidemove[speed];

    if (gamekeydown[key_strafeleft])
        side -= sidemove[speed];

    if (gamekeydown[key_fire])
        netcmd.buttons |= BT_ATTACK;

    if (gamekeydown[key_use])
    {
        netcmd.buttons |= BT_USE;
    }

    if(gamekeydown[key_weapon_up])
        newweapon = P_WeaponCycleUp(&_g_player);
    else if(gamekeydown[key_weapon_down])
        newweapon = P_WeaponCycleDown(&_g_player);
    else if ((_g_player.attackdown && !P_CheckAmmo(&_g_player)))
        newweapon = P_SwitchWeapon(&_g_player);
    else
        newweapon = wp_nochange;

    if (newweapon != wp_nochange)
    {
        netcmd.buttons |= BT_CHANGE;
        netcmd.buttons |= newweapon<<BT_WEAPONSHIFT;
    }

    if (forward > MAXPLMOVE)
        forward = MAXPLMOVE;
    else if (forward < -MAXPLMOVE)
        forward = -MAXPLMOVE;
    if (side > MAXPLMOVE)
        side = MAXPLMOVE;
    else if (side < -MAXPLMOVE)
        side = -MAXPLMOVE;

    netcmd.forwardmove += fudgef((int8_t)forward);
    netcmd.sidemove += side;
}


//
// G_DoLoadLevel
//

static void G_DoLoadLevel (void)
{
    _g_gamestate = GS_LEVEL;


    if (_g_playeringame && _g_player.playerstate == PST_DEAD)
        _g_player.playerstate = PST_REBORN;


    // initialize the msecnode_t freelist.                     phares 3/25/98
    // any nodes in the freelist are gone by now, cleared
    // by Z_FreeTags() when the previous level ended or player
    // died.

    P_SetSecnodeFirstpoolToNull();


    P_SetupLevel (_s_gamemap);

    _s_gameaction = ga_nothing;
    Z_CheckHeap ();

    // clear cmd building stuff
    memset(gamekeydown, 0, sizeof(gamekeydown));

    // killough 5/13/98: in case netdemo has consoleplayer other than green
    ST_Start();
}


//
// G_Ticker
// Make ticcmd_ts for the players.
//

void G_Ticker (void)
{
    static gamestate_t prevgamestate = 0;

    P_MapStart();

    P_MapEnd();

    // do things to change the game state
    while (_s_gameaction != ga_nothing)
    {
        G_DoPlayDemo();
    }


    if (_g_playeringame)
    {
        memcpy(&_g_player.cmd, &netcmd, sizeof(ticcmd_t));
   
        if (_g_demoplayback)
            G_ReadDemoTiccmd ();
    }


    // cph - if the gamestate changed, we may need to clean up the old gamestate
    prevgamestate = _g_gamestate;

    // do main actions
    P_Ticker ();
    ST_Ticker ();
}


//
// G_PlayerReborn
// Called after a player dies
// almost everything is cleared and initialized
//

void G_PlayerReborn (void)
{
    player_t *p;
    int16_t i;
    int16_t killcount;
    int16_t itemcount;
    int16_t secretcount;

    killcount   = _g_player.killcount;
    itemcount   = _g_player.itemcount;
    secretcount = _g_player.secretcount;

    p = &_g_player;

    memset (p, 0, sizeof(*p));

    _g_player.killcount = killcount;
    _g_player.itemcount = itemcount;
    _g_player.secretcount = secretcount;

    p->usedown = p->attackdown = true;  // don't do anything immediately
    p->playerstate = PST_LIVE;
    p->health = initial_health;  // Ty 03/12/98 - use dehacked values
    p->readyweapon = p->pendingweapon = wp_pistol;
    p->weaponowned[wp_fist] = true;
    p->weaponowned[wp_pistol] = true;
    p->ammo[am_clip] = initial_bullets; // Ty 03/12/98 - use dehacked values

    for (i=0 ; i<NUMAMMO ; i++)
        p->maxammo[i] = maxammo[i];
}


void G_ExitLevel (void)
{
    _s_gameaction = ga_completed;
}


// killough 3/1/98: function to reload all the default parameter
// settings before a new game begins

void G_ReloadDefaults(void)
{
    // killough 3/1/98: Initialize options based on config file
    // (allows functions above to load different values for demos
    // and savegames without messing up defaults).
    _g_demoplayback = false;
}


//
// G_InitNew
// Can be called by the startup code or the menu task,
// consoleplayer, displayplayer, playeringame[] should be set.
//

static void G_InitNew(skill_t skill, int16_t map)
{
    if (skill > sk_hard)
        skill = sk_hard;

    if (map < 1)
        map = 1;
    if (map > 9)
        map = 9;

    M_ClearRandom();

    _g_player.playerstate = PST_REBORN;

    _s_gamemap = map;
    _g_gameskill = skill;

    totalleveltimes = 0;

    G_DoLoadLevel ();
}

//
// DEMO RECORDING
//

static void G_CheckDemoStatus (void);

#define DEMOMARKER    0x80

static void G_ReadDemoTiccmd (void)
{
    uint8_t at; // e6y: tasdoom stuff

    if (*demo_p == DEMOMARKER)
        G_CheckDemoStatus();      // end of demo data stream
    else if (_g_demoplayback && demo_p + 4 > demobuffer + demolength)
    {
        printf("G_ReadDemoTiccmd: missing DEMOMARKER\n");
        G_CheckDemoStatus();
    }
    else
    {
        ticcmd_t* cmd = &_g_player.cmd;
        cmd->forwardmove = ((int8_t)*demo_p++);
        cmd->sidemove = ((int8_t)*demo_p++);
        cmd->angleturn = ((uint8_t)(at = *demo_p++))<<8;
        cmd->buttons = (uint8_t)*demo_p++;
    }
}


//
// G_PlayDemo
//

void G_DeferedPlayDemo (void)
{
    _s_gameaction = ga_playdemo;
}


//e6y: Check for overrun
static void CheckForOverrun(const byte __far* start_p, const byte __far* current_p, size_t size)
{
    size_t pos = current_p - start_p;
    if (pos + size > demolength)
    {
        I_Error("CheckForOverrun: wrong demo header\n");
    }
}


// killough 2/28/98: A ridiculously large number
// of players, the most you'll ever need in a demo
// or savegame. This is used to prevent problems, in
// case more players in a game are supported later.

#define MIN_MAXPLAYERS 4


static const byte __far* G_ReadDemoHeader(const byte __far* demo_p)
{
    // e6y
    // The local variable should be used instead of demobuffer,
    // because demobuffer can be uninitialized
    const byte __far* header_p = demo_p;

    _g_basetic = _g_gametic;  // killough 9/29/98

    // killough 2/22/98, 2/28/98: autodetect old demos and act accordingly.
    // Old demos turn on demo_compatibility => compatibility; new demos load
    // compatibility flag, and other flags as well, as a part of the demo.

    //e6y: check for overrun
    CheckForOverrun(header_p, demo_p, 1);

    demo_p++;

    // killough 3/2/98: force these variables to be 0 in demo_compatibility

    // killough 3/6/98: rearrange to fix savegame bugs (moved fastparm,
    // respawnparm, nomonsters flags to G_LoadOptions()/G_SaveOptions())

    //e6y: check for overrun
    CheckForOverrun(header_p, demo_p, 8);

    skill_t skill = *demo_p++;
    demo_p++;
    int16_t map = *demo_p++;
    demo_p++;
    demo_p++;
    demo_p++;
    demo_p++;
    demo_p++;

    //e6y: check for overrun
    CheckForOverrun(header_p, demo_p, MAXPLAYERS);

    _g_playeringame = *demo_p++;
    demo_p += MIN_MAXPLAYERS - MAXPLAYERS;


    if (_s_gameaction != ga_loadgame) { /* killough 12/98: support -loadgame */
        G_InitNew(skill, map);
    }

    return demo_p;
}


static void G_DoPlayDemo(void)
{
    int16_t demolumpnum = W_GetNumForName("DEMO3");
    demobuffer = W_GetLumpByNum(demolumpnum);
    demolength = W_LumpLength(demolumpnum);

    demo_p = G_ReadDemoHeader(demobuffer);

    _s_gameaction = ga_nothing;

    _g_demoplayback = true;

    clock();
}

/* G_CheckDemoStatus
 *
 * Called after a death or level completion to allow demos to be cleaned up
 */
static void G_CheckDemoStatus (void)
{
    uint32_t realtics = (clock() * TICRATE) / CLOCKS_PER_SEC;
    uint32_t resultfps = TICRATE * 1000L * _g_gametic / realtics;
    I_Error ("Timed %lu gametics in %lu realtics = %lu.%.3lu frames per second",
             (uint32_t) _g_gametic,realtics,
             resultfps / 1000, resultfps % 1000);
}


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
 *  DOOM selection menu, options etc. (aka Big Font menus)
 *  Sliders and icons. Kinda widget stuff.
 *  Setup Menus.
 *  Extended HELP screens.
 *  Dynamic HELP screen.
 *
 *-----------------------------------------------------------------------------*/

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>

#include "compiler.h"
#include "doomdef.h"
#include "d_player.h"
#include "d_englsh.h"
#include "d_main.h"
#include "v_video.h"
#include "w_wad.h"
#include "r_main.h"
#include "hu_stuff.h"
#include "g_game.h"
#include "s_sound.h"
#include "sounds.h"
#include "m_menu.h"
#include "doomtype.h"
#include "i_system.h"

#include "globdata.h"


//
// MENU TYPEDEFS
//

typedef struct
{
  int16_t status; // 0 = no cursor here, 1 = ok, 2 = arrows ok
  char  name[10];

  // choice = menu item #.
  // if status = 2,
  //   choice=0:leftarrow,1:rightarrow
  void  (*routine)(int16_t choice);
} menuitem_t;

typedef struct menu_s
{
  int16_t           numitems;     // # of menu items
  const menuitem_t* menuitems;    // menu items
  void            (*routine)(); // draw routine
  int16_t           x;
  int16_t           y;            // x,y of menu
  const struct menu_s*	prevMenu;	// previous menu
  int16_t previtemOn;
} menu_t;


//
// defaulted values
//
int16_t _g_alwaysRun;

uint16_t _g_gamma;

static boolean messageToPrint;  // true = message to be printed

static const char* messageString; // ...and here is the message string!

static boolean messageLastMenuActive;


static const menu_t* currentMenu; // current menudef

static int16_t itemOn;           // menu item skull is on (for Big Font menus)
static int16_t skullAnimCounter; // skull animation counter
static int16_t whichSkull;       // which skull to draw (he blinks)

boolean _g_menuactive;    // The menus are up
static boolean messageNeedsInput; // timed message = no input from user

char _g_savegamestrings[8][8];


int16_t showMessages = 0;


static void (*messageRoutine)(boolean affirmative);

// we are going to be entering a savegame string

#define SKULLXOFF  -32
#define LINEHEIGHT  16


// end of externs added for setup menus

//
// PROTOTYPES
//
static void M_NewGame(int16_t choice);
static void M_ChooseSkill(int16_t choice);
static void M_LoadGame(int16_t choice);
static void M_Options(int16_t choice);
static void M_EndGame(int16_t choice);


static void M_ChangeMessages(int16_t choice);
static void M_ChangeAlwaysRun(int16_t choice);
static void M_ChangeGamma(int16_t choice);
static void M_SfxVol(int16_t choice);
static void M_MusicVol(int16_t choice);

static void M_LoadSelect(int16_t choice);
static void M_ReadSaveStrings(void);

static void M_DrawMainMenu(void);
static void M_DrawNewGame(void);
static void M_DrawOptions(void);
static void M_DrawSound(void);
static void M_DrawLoad(void);

static void M_SetupNextMenu(const menu_t *menudef);
static void M_DrawThermo(int16_t x, int16_t y, int16_t thermWidth, int16_t thermDot);
static void M_WriteText(int16_t x, int16_t y, const char __far* string);
static int16_t M_StringWidth(const char __far* string);
static int16_t M_StringHeight(const char *string);
static void M_StartMessage(const char *string,void (*routine)(boolean));
static void M_ClearMenus (void);


// end of prototypes added to support Setup Menus and Extended HELP screens

/////////////////////////////////////////////////////////////////////////////
//
// DOOM MENUS
//

/////////////////////////////
//
// MAIN MENU
//

// main_e provides numerical values for which Big Font screen you're on

enum
{
  newgame = 0,
  options,
  loadgame,
  main_end
};

//
// MainMenu is the definition of what the main menu Screen should look
// like. Each entry shows that the cursor can land on each item (1), the
// built-in graphic lump (i.e. "M_NGAME") that should be displayed,
// and the program which takes over when an item is selected.
//

static const menuitem_t MainMenu[]=
{
  {1,"M_NGAME", M_NewGame},
  {1,"M_OPTION",M_Options},
  {1,"M_LOADG", M_LoadGame},
};

static const menu_t MainDef =
{
  main_end,       // number of menu items
  MainMenu,       // table that defines menu items
  M_DrawMainMenu, // drawing routine
  97,64,          // initial cursor position
  NULL,0,
};

//
// M_DrawMainMenu
//

static void M_DrawMainMenu(void)
{

}


// numerical values for the New Game menu items

enum
{
  killthings,
  toorough,
  hurtme,
  violence,
  nightmare,
  newg_end
};

// The definitions of the New Game menu

static const menuitem_t NewGameMenu[]=
{
  {1,"M_JKILL", M_ChooseSkill},
  {1,"M_ROUGH", M_ChooseSkill},
  {1,"M_HURT",  M_ChooseSkill},
  {1,"M_ULTRA", M_ChooseSkill},
  {1,"M_NMARE", M_ChooseSkill}
};

static const menu_t NewDef =
{
  newg_end,       // # of menu items
  NewGameMenu,    // menuitem_t ->
  M_DrawNewGame,  // drawing routine ->
  48,63,          // x,y
  &MainDef,0,
};


//
// M_NewGame
//

static void M_DrawNewGame(void)
{

}

static void M_NewGame(int16_t choice)
{
	UNUSED(choice);

	M_SetupNextMenu(&NewDef);
	itemOn = 2; //Set hurt me plenty as default difficulty
}


static void M_VerifyNightmare(boolean affirmative)
{
    if (affirmative)
        G_DeferedInitNew(nightmare);
}

static void M_ChooseSkill(int16_t choice)
{
    if (choice == nightmare)
    {
        M_StartMessage(NIGHTMARE, M_VerifyNightmare);
		itemOn = 0;
    }
    else
    {
        G_DeferedInitNew(choice);
		M_ClearMenus ();
    }    
}

/////////////////////////////
//
// LOAD GAME MENU
//

// numerical values for the Load Game slots

enum
{
    load1,
    load2,
    load3,
    load4,
    load5,
    load6,
    load7,
    load8,
    load_end
};

// The definitions of the Load Game screen

static const menuitem_t LoadMenue[]=
{
    {1,"", M_LoadSelect},
    {1,"", M_LoadSelect},
    {1,"", M_LoadSelect},
    {1,"", M_LoadSelect},
    {1,"", M_LoadSelect},
    {1,"", M_LoadSelect},
    {1,"", M_LoadSelect},
    {1,"", M_LoadSelect},
};

static const menu_t LoadDef =
{
  load_end,
  LoadMenue,
  M_DrawLoad,
  64,34, //jff 3/15/98 move menu up
  &MainDef,2,
};

#define LOADGRAPHIC_Y 8

//
// M_LoadGame & Cie.
//

static void M_DrawSaveLoad(const char* name)
{
	UNUSED(name);

	int8_t i, j;

	const patch_t __far* lpatch = W_GetLumpByName("M_LSLEFT");
	const patch_t __far* mpatch = W_GetLumpByName("M_LSCNTR");
	const patch_t __far* rpatch = W_GetLumpByName("M_LSRGHT");

	for (i = 0; i < load_end; i++)
	{
		//
		// Draw border for the savegame description
		//
		int16_t x = LoadDef.x;
		const int16_t y = 27 + 13 * i + 7;
		V_DrawPatchNotScaled(x - 8, y, lpatch);
		for (j = 0; j < 12; j++)
		{
			V_DrawPatchNotScaled(x, y, mpatch);
			x += 8;
		}
		V_DrawPatchNotScaled(x, y, rpatch);

		M_WriteText(LoadDef.x, y - 7, _g_savegamestrings[i]);
	}

	Z_ChangeTagToCache(lpatch);
	Z_ChangeTagToCache(mpatch);
	Z_ChangeTagToCache(rpatch);
}

static void M_DrawLoad(void)
{
	M_DrawSaveLoad("M_LOADG");
}


//
// User wants to load this game
//

static void M_LoadSelect(int16_t choice)
{
  // CPhipps - Modified so savegame filename is worked out only internal
  //  to g_game.c, this only passes the slot.

  G_LoadGame(choice); // killough 3/16/98: add slot

  M_ClearMenus ();
}

//
// Selected from DOOM menu
//

static void M_LoadGame (int16_t choice)
{
	UNUSED(choice);

	/* killough 5/26/98: exclude during demo recordings
	 * cph - unless a new demo */

	M_SetupNextMenu(&LoadDef);
	M_ReadSaveStrings();
}

/////////////////////////////
//
// SAVE GAME MENU
//


//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
static void M_ReadSaveStrings(void)
{

}


/////////////////////////////
//
// OPTIONS MENU
//

// numerical values for the Options menu items

enum
{                                                   // phares 3/21/98
  endgame,
  messages,
  alwaysrun,
  gamma,
  opt_end
};

// The definitions of the Options menu

static const menuitem_t OptionsMenu[]=
{
  // killough 4/6/98: move setup to be a sub-menu of OPTIONs
  {1,"M_ENDGAM", M_EndGame},
  {1,"M_MESSG",  M_ChangeMessages},
  {1,"M_ARUN",   M_ChangeAlwaysRun},
  {2,"M_GAMMA",  M_ChangeGamma},
};

static const menu_t OptionsDef =
{
  opt_end,
  OptionsMenu,
  M_DrawOptions,
  60,37,
  &MainDef,1,
};

//
// M_Options
//

static void M_DrawOptions(void)
{
  M_DrawThermo(OptionsDef.x + 158, OptionsDef.y+LINEHEIGHT*gamma+2,6,_g_gamma);
}

static void M_Options(int16_t choice)
{
	UNUSED(choice);

	M_SetupNextMenu(&OptionsDef);
}

/////////////////////////////
//
// SOUND VOLUME MENU
//

// numerical values for the Sound Volume menu items
// The 'empty' slots are where the sliding scales appear.

enum
{
  sfx_vol,
  sfx_empty1,
  music_vol,
  sfx_empty2,
  sound_end
};

// The definitions of the Sound Volume menu

static const menuitem_t SoundMenu[]=
{
  {2,"M_SFXVOL",M_SfxVol},
  {-1,"",0},
  {2,"M_MUSVOL",M_MusicVol},
  {-1,"",0}
};

static const menu_t SoundDef =
{
  sound_end,
  SoundMenu,
  M_DrawSound,
  80,64,
  &OptionsDef,4,
};

//
// Change Sfx & Music volumes
//

static void M_DrawSound(void)
{
  M_DrawThermo(SoundDef.x, SoundDef.y + LINEHEIGHT * (sfx_vol   + 1), 16, snd_SfxVolume);

  M_DrawThermo(SoundDef.x, SoundDef.y + LINEHEIGHT * (music_vol + 1), 16, snd_MusicVolume);
}


static void M_SfxVol(int16_t choice)
{
  switch(choice)
    {
    case 0:
      if (snd_SfxVolume)
        snd_SfxVolume--;
      break;
    case 1:
      if (snd_SfxVolume < 15)
        snd_SfxVolume++;
      break;
    }

  G_SaveSettings();
}

static void M_MusicVol(int16_t choice)
{
  switch(choice)
    {
    case 0:
      if (snd_MusicVolume)
        snd_MusicVolume--;
      break;
    case 1:
      if (snd_MusicVolume < 15)
        snd_MusicVolume++;
      break;
    }

  G_SaveSettings();
}

/////////////////////////////
//
// M_EndGame
//

static void M_EndGameResponse(boolean affirmative)
{
  if (!affirmative)
    return;

  // killough 5/26/98: make endgame quit if recording or playing back demo
  if (_g_singledemo)
    G_CheckDemoStatus();

  M_ClearMenus ();
  D_StartTitle ();
}

static void M_EndGame(int16_t choice)
{
	UNUSED(choice);

	M_StartMessage(ENDGAME, M_EndGameResponse);
}

/////////////////////////////
//
//    Toggle messages on/off
//

static void M_ChangeMessages(int16_t choice)
{
  UNUSED(choice);

  showMessages = 1 - showMessages;

  _g_player.message = showMessages ? MSGON : MSGOFF;

  _g_message_dontfuckwithme = true;

  G_SaveSettings();
}


static void M_ChangeAlwaysRun(int16_t choice)
{
    UNUSED(choice);

    _g_alwaysRun = 1 - _g_alwaysRun;

    if (!_g_alwaysRun)
      _g_player.message = RUNOFF; // Ty 03/27/98 - externalized
    else
      _g_player.message = RUNON ; // Ty 03/27/98 - externalized

    G_SaveSettings();
}

static void M_ChangeGamma(int16_t choice)
{
	switch(choice)
    {
		case 0:
		  if (_g_gamma)
			_g_gamma--;
		  break;
		case 1:
		  if (_g_gamma < 5)
			_g_gamma++;
		  break;
    }
	I_SetPalette(0);

    G_SaveSettings();
}

//
// End of Original Menus
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////
//
// General routines used by the Setup screens.
//

//
// M_InitDefaults()
//
// killough 11/98:
//
// This function converts all setup menu entries consisting of cfg
// variable names, into pointers to the corresponding default[]
// array entry. var.name becomes converted to var.def.
//

static void M_InitDefaults(void)
{

}

//
// End of Setup Screens.
//
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//
// General Routines
//
// This displays the Main menu and gets the menu screens rolling.
// Plus a variety of routines that control the Big Font menu display.
// Plus some initialization for game-dependant situations.

static char __far* Z_Strdup(const char* s)
{
    const size_t len = strlen(s);

    if(!len)
        return NULL;

    char __far* ptr = Z_MallocStatic(len+1);

    if(ptr)
        _fstrcpy(ptr, s);

    return ptr;
}


//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
// killough 9/29/98: Significantly reformatted source
//

void M_Drawer (void)
{
    // Horiz. & Vertically center string and print it.
    // killough 9/29/98: simplified code, removed 40-character width limit
    if (messageToPrint)
    {
        /* cph - strdup string to writable memory */
        char __far* ms = Z_Strdup(messageString);
        char __far* p = ms;

        int16_t y = 80 - M_StringHeight(messageString)/2;
        while (*p)
        {
            char __far* string = p;
            char c;
            while ((c = *p) && *p != '\n')
                p++;
            *p = 0;
            M_WriteText(120 - M_StringWidth(string)/2, y, string);
            y += HU_FONT_HEIGHT;
            if ((*p = c))
                p++;
        }
        Z_Free(ms);
    }
    else
        if (_g_menuactive)
        {
            int16_t x,y,max,i;

            if (currentMenu->routine)
                currentMenu->routine();     // call Draw routine

            // DRAW MENU

            x = currentMenu->x;
            y = currentMenu->y;
            max = currentMenu->numitems;

            for (i=0;i<max;i++)
            {
                y += LINEHEIGHT;
            }
        }
}

//
// M_ClearMenus
//
// Called when leaving the menu screens for the real world

static void M_ClearMenus (void)
{
  _g_menuactive = false;
  itemOn = 0;
}

//
// M_SetupNextMenu
//
static void M_SetupNextMenu(const menu_t *menudef)
{
  currentMenu = menudef;
  itemOn = 0;
}

/////////////////////////////
//
// M_Ticker
//
void M_Ticker (void)
{
  if (--skullAnimCounter <= 0)
    {
      whichSkull ^= 1;
      skullAnimCounter = 8;
    }
}

/////////////////////////////
//
// Message Routines
//

static void M_StartMessage (const char* string, void (*routine)(boolean))
{
	messageLastMenuActive = _g_menuactive;
	messageToPrint        = true;
	messageString         = string;
	messageRoutine        = routine;
	messageNeedsInput     = routine != NULL;
	_g_menuactive         = true;
}


/////////////////////////////
//
// Thermometer Routines
//

//
// M_DrawThermo draws the thermometer graphic for Mouse Sensitivity,
// Sound Volume, etc.
//
// proff/nicolas 09/20/98 -- changed for hi-res
// CPhipps - patch drawing updated
//
static void M_DrawThermo(int16_t x, int16_t y, int16_t thermWidth, int16_t thermDot )
{
	UNUSED(y);
	UNUSED(thermDot);

    int16_t          xx;
    int16_t           i;
    /*
   * Modification By Barry Mead to allow the Thermometer to have vastly
   * larger ranges. (the thermWidth parameter can now have a value as
   * large as 200.      Modified 1-9-2000  Originally I used it to make
   * the sensitivity range for the mouse better. It could however also
   * be used to improve the dynamic range of music and sound affect
   * volume controls for example.
   */
    int16_t horizScaler; //Used to allow more thermo range for mouse sensitivity.
    thermWidth = (thermWidth > 200) ? 200 : thermWidth; //Clamp to 200 max
    horizScaler = (thermWidth > 23) ? (200 / thermWidth) : 8; //Dynamic range
    xx = x;

    xx += 8;
    for (i=0;i<thermWidth;i++)
    {
        xx += horizScaler;
    }

    xx += (8 - horizScaler);  /* make the right end look even */
}

/////////////////////////////
//
// String-drawing Routines
//

static int16_t font_lump_offset;

//
// Find string width from hu_font chars
//

static int16_t M_StringWidth(const char __far* string)
{
	int16_t	w = 0;

	for (size_t i = 0; i < _fstrlen(string); i++)
	{
		char c = string[i];
		c = toupper(c);
		if (HU_FONTSTART <= c && c <= HU_FONTEND)
		{
			const patch_t __far* patch = W_GetLumpByNum(c + font_lump_offset);
			w += patch->width;
			Z_ChangeTagToCache(patch);
		} else
			w += HU_FONT_SPACE_WIDTH;
	}

	return w;
}

//
//    Find string height from hu_font chars
//

static int16_t M_StringHeight(const char* string)
{
	int16_t i, h = HU_FONT_HEIGHT;
	for (i = 0; string[i]; i++)            // killough 1/31/98
		if (string[i] == '\n')
			h += HU_FONT_HEIGHT;
	return h;
}

//
//    Write a string using the hu_font
//
static void M_WriteText (int16_t x, int16_t y, const char __far* string)
{
	const char __far* ch = string;
	int16_t cx = x;
	int16_t cy = y;

	while (true) {
		char c = *ch++;
		if (!c)
			break;

		if (c == '\n') {
			cx = x;
			cy += 12;
			continue;
		}

		c = toupper(c);
		if (HU_FONTSTART <= c && c <= HU_FONTEND)
		{
			const patch_t __far* patch = W_GetLumpByNum(c + font_lump_offset);
			V_DrawPatchNotScaled(cx, cy, patch);
			cx += patch->width;
			Z_ChangeTagToCache(patch);
		} else {
			cx += HU_FONT_SPACE_WIDTH;
		}
	}
}

/////////////////////////////
//
// Initialization Routines to take care of one-time setup
//

//
// M_Init
//
void M_Init(void)
{
	M_InitDefaults();
	currentMenu           = &MainDef;
	_g_menuactive         = false;
	whichSkull            = 0;
	skullAnimCounter      = 10;
	messageToPrint        = false;
	messageString         = NULL;
	messageLastMenuActive = _g_menuactive;

	font_lump_offset = W_GetNumForName(HU_FONTSTART_LUMP) - HU_FONTSTART;

	G_UpdateSaveGameStrings();
}

//
// End of General Routines
//
/////////////////////////////////////////////////////////////////////////////

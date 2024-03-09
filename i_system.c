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
 *      DOS implementation of i_system.h
 *
 *-----------------------------------------------------------------------------*/

#include <conio.h>
#include <dos.h>
#include <stdarg.h>
#include <time.h>

#include "doomdef.h"
#include "doomtype.h"
#include "compiler.h"
#include "a_taskmn.h"
#include "d_main.h"
#include "i_system.h"
#include "globdata.h"


void I_InitGraphicsHardwareSpecificCode(void);
static void NORETURN_PRE I_Quit(void) NORETURN_POST;


static boolean isGraphicsModeSet = false;


//**************************************************************************************
//
// Screen code
//

void I_SetScreenMode(uint16_t mode)
{
	union REGS regs;
	regs.w.ax = mode;
	int86(0x10, &regs, &regs);
}


void I_InitGraphics(void)
{
	I_InitGraphicsHardwareSpecificCode();
	isGraphicsModeSet = true;
}


//**************************************************************************************
//
// Keyboard code
//

void I_InitScreen(void)
{
	I_SetScreenMode(3);
}


void I_StartTic(void)
{

}


//**************************************************************************************
//
// Returns time in 1/35th second tics.
//

static volatile int32_t ticcount;


int32_t I_GetTime(void)
{
    return ticcount++;
}


void I_InitTimer(void)
{

}


//**************************************************************************************
//
// Exit code
//

static void I_Shutdown(void)
{
	if (isGraphicsModeSet)
		I_SetScreenMode(3);

	I_ShutdownSound();

	Z_Shutdown();
}


static void I_Quit(void)
{
	I_Shutdown();

	int16_t lumpnum = W_GetNumForName("ENDOOM");
	W_ReadLumpByNum(lumpnum, D_MK_FP(0xb800, __djgpp_conventional_base));

	union REGS regs;
	regs.h.ah = 2;
	regs.h.bh = 0;
	regs.h.dl = 0;
	regs.h.dh = 23;
	int86(0x10, &regs, &regs);

	printf("\n");
	exit(0);
}


void I_Error (const char *error, ...)
{
	va_list argptr;

	I_Shutdown();

	va_start(argptr, error);
	vprintf(error, argptr);
	va_end(argptr);
	printf("\n");
	exit(1);
}

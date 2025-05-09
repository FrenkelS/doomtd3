/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2001 by
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
 *      Handles WAD file header, directory, lump I/O.
 *
 *-----------------------------------------------------------------------------
 */

// use config.h if autoconf made one -- josh
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdint.h>

#include "compiler.h"
#include "d_player.h"
#include "doomtype.h"
#include "i_system.h"

#include "w_wad.h"

#include "globdata.h"


//
// TYPES
//

typedef struct
{
  int32_t  filepos;
  uint16_t size;
  int16_t  filler;        // always zero
  char name[8];
} filelump_t;


//
// GLOBALS
//

static FILE* fileWAD;

static int16_t numlumps;

static filelump_t __far* fileinfo;

static void __far*__far* lumpcache;

//
// LUMP BASED ROUTINES.
//

#define BUFFERSIZE 512

static void _ffread(void __far* ptr, uint16_t size, FILE* fp)
{
	uint8_t __far* dest = ptr;
	uint8_t buffer[BUFFERSIZE];

	while (size >= BUFFERSIZE)
	{
		fread(&buffer[0], BUFFERSIZE, 1, fp);
		_fmemcpy(dest, &buffer[0], BUFFERSIZE);
		dest += BUFFERSIZE;
		size -= BUFFERSIZE;
	}

	if (size > 0)
	{
		fread(&buffer[0], size, 1, fp);
		_fmemcpy(dest, &buffer[0], size);
	}
}

typedef struct
{
  char identification[4]; // Should be "IWAD" or "PWAD".
  int16_t  numlumps;
  int16_t  filler;        // always zero
  int32_t  infotableofs;
} wadinfo_t;

void W_Init(void)
{
#if BYTE_ORDER == LITTLE_ENDIAN
#define WAD_UPPERCASE "DOOMTD3L.WAD"
#define WAD_LOWERCASE "doomtd3l.wad"
#elif BYTE_ORDER == BIG_ENDIAN
#define WAD_UPPERCASE "DOOMTD3B.WAD"
#define WAD_LOWERCASE "doomtd3b.wad"
#else
#error unknown byte order
#endif

	fileWAD = fopen(WAD_UPPERCASE, "rb");
	if (fileWAD == NULL)
	{
		fileWAD = fopen(WAD_LOWERCASE, "rb");
		if (fileWAD == NULL)
			I_Error("Can't open " WAD_UPPERCASE);
	}

	wadinfo_t header;
	fseek(fileWAD, 0, SEEK_SET);
	fread(&header, sizeof(header), 1, fileWAD);

	fileinfo = Z_MallocStatic(header.numlumps * sizeof(filelump_t));
	fseek(fileWAD, header.infotableofs, SEEK_SET);
	_ffread(fileinfo, sizeof(filelump_t) * header.numlumps, fileWAD);

	lumpcache = Z_MallocStatic(header.numlumps * sizeof(*lumpcache));
	_fmemset(lumpcache, 0, header.numlumps * sizeof(*lumpcache));

	numlumps = header.numlumps;
}


const char __far* PUREFUNC W_GetNameForNum(int16_t num)
{
	return fileinfo[num].name;
}


//
// W_LumpLength
// Returns the buffer size needed to load the given lump.
//

uint16_t PUREFUNC W_LumpLength(int16_t num)
{
	return fileinfo[num].size;
}


// W_GetNumForName
// bombs out if not found.
//
int16_t PUREFUNC W_GetNumForName(const char *name)
{
	char name8[8];
	strncpy(name8, name, 8);
	uint32_t name_int1 = *(uint32_t*)&name8[0];
	uint32_t name_int2 = *(uint32_t*)&name8[4];

	for (int16_t i = 0; i < numlumps; i++)
	{
		if (name_int1 == *(uint32_t __far*)&fileinfo[i].name[0]
		 && name_int2 == *(uint32_t __far*)&fileinfo[i].name[4])
		{
			return i;
		}
	}

	I_Error("W_GetNumForName: %.8s not found", name);
	return -1;
}


void W_ReadLumpByNum(int16_t num, void __far* ptr)
{
	const filelump_t __far* lump = &fileinfo[num];
	fseek(fileWAD, lump->filepos, SEEK_SET);
	_ffread(ptr, lump->size, fileWAD);
}


const void __far* PUREFUNC W_GetLumpByNumAutoFree(int16_t num)
{
	const filelump_t __far* lump = &fileinfo[num];

	void __far* ptr = Z_MallocLevel(lump->size, NULL);

	fseek(fileWAD, lump->filepos, SEEK_SET);
	_ffread(ptr, lump->size, fileWAD);
	return ptr;
}


static void __far* PUREFUNC W_GetLumpByNumWithUser(int16_t num, void __far*__far* user)
{
	const filelump_t __far* lump = &fileinfo[num];

	void __far* ptr = Z_MallocStaticWithUser(lump->size, user);

	fseek(fileWAD, lump->filepos, SEEK_SET);
	_ffread(ptr, lump->size, fileWAD);
	return ptr;
}


int16_t W_GetFirstInt16(int16_t num)
{
	const filelump_t __far* lump = &fileinfo[num];

	int16_t firstInt16;

	fseek(fileWAD, lump->filepos, SEEK_SET);
	fread(&firstInt16, sizeof(int16_t), 1, fileWAD);
	return firstInt16;
}


const void __far* PUREFUNC W_GetLumpByNum(int16_t num)
{
	if (lumpcache[num])
		Z_ChangeTagToStatic(lumpcache[num]);
	else
		lumpcache[num] = W_GetLumpByNumWithUser(num, &lumpcache[num]);

	return lumpcache[num];
}


boolean PUREFUNC W_IsLumpCached(int16_t num)
{
	return lumpcache[num] != NULL;
}


const void __far* PUREFUNC W_TryGetLumpByNum(int16_t num)
{
	if (lumpcache[num])
	{
		Z_ChangeTagToStatic(lumpcache[num]);
		return lumpcache[num];
	}
	else if (Z_IsEnoughFreeMemory(W_LumpLength(num)))
		return W_GetLumpByNum(num);
	else
		return NULL;
}


void W_CacheLumps(void)
{
	int16_t cachelumpnum = W_GetNumForName("CACHE");
	int16_t numlumps = W_LumpLength(cachelumpnum) / 8;
	const char __far* lumpsToCache = W_GetLumpByNum(cachelumpnum);
	const void __far* __far* lumps = (const void __far* __far*)lumpsToCache;
	uint32_t freeBlockSize = Z_GetLargestFreeBlockSize();
	int16_t cachecount = 0;
	for (int16_t i = 0; i < numlumps; i++)
	{
		char name[8];
		_fmemcpy(name, &lumpsToCache[i * 8], 8);
#if defined DISABLE_STATUS_BAR
		if ((name[0] != 'S' && name[1] != 'T') || (name[0] == 'S' && name[1] == 'T' && name[2] == 'E' && name[3] == 'P'))
#endif
		{
			int16_t num = W_GetNumForName(name);
			uint16_t length = W_LumpLength(num);

			if (length > freeBlockSize)
			{
				freeBlockSize = Z_GetLargestFreeBlockSize();
				if (length > freeBlockSize)
				{
					break;
				}
			}

			freeBlockSize -= length;
			*lumps++ = W_GetLumpByNum(num);
			cachecount++;
		}
	}

	lumps = (const void __far* __far*)lumpsToCache;
	for (int16_t j = 0; j < cachecount; j++)
		Z_ChangeTagToCache(*lumps++);

	Z_Free(lumpsToCache);
}

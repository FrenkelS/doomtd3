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
 *  General plane mover and floor mover action routines
 *  Floor motion, pure changer types, raising stairs. donuts, elevators
 *
 *-----------------------------------------------------------------------------*/

#include "d_player.h"
#include "r_main.h"
#include "m_random.h"
#include "p_inter.h"
#include "p_map.h"
#include "p_spec.h"
#include "p_tick.h"
#include "sounds.h"

#include "globdata.h"


static boolean nofit;


//
// P_ThingHeightClip
// Takes a valid thing and adjusts the thing->floorz,
// thing->ceilingz, and possibly thing->z.
// This is called for all nearby monsters
// whenever a sector changes height.
// If the thing doesn't fit,
// the z will be set to the lowest value
// and false will be returned.
//

static boolean P_ThingHeightClip(mobj_t __far* thing)
{
  boolean   onfloor;

  onfloor = (thing->z == thing->floorz);

  P_CheckPosition (thing, thing->x, thing->y);

  /* what about stranding a monster partially off an edge?
   * killough 11/98: Answer: see below (upset balance if hanging off ledge)
   */

  thing->floorz = _g_tmfloorz;
  thing->ceilingz = _g_tmceilingz;
  thing->dropoffz = _g_tmdropoffz;    /* killough 11/98: remember dropoffs */

  if (onfloor)
    {

    // walking monsters rise and fall with the floor

    thing->z = thing->floorz;
    }
  else
    {

  // don't adjust a floating monster unless forced to

    if (thing->z+thing->height > thing->ceilingz)
      thing->z = thing->ceilingz - thing->height;
    }

  return thing->ceilingz - thing->floorz >= thing->height;
}


//
// SECTOR HEIGHT CHANGING
// After modifying a sectors floor or ceiling height,
// call this routine to adjust the positions
// of all things that touch the sector.
//
// If anything doesn't fit anymore, true will be returned.
// If crunch is true, they will take damage
//  as they are being crushed.
// If Crunch is false, you should set the sector height back
//  the way it was and call P_ChangeSector again
//  to undo the changes.
//


//
// PIT_ChangeSector
//

static void PIT_ChangeSector(mobj_t __far* thing)
  {
  if (P_ThingHeightClip (thing))
    return;

  // crunch bodies to giblets

  if (thing->health <= 0)
    {
    P_SetMobjState (thing, S_GIBS);

    thing->flags &= ~MF_SOLID;
    thing->height = 0;
    thing->radius = 0;
    return;
    }

  // crunch dropped items

  if (thing->flags & MF_DROPPED)
    {
    P_RemoveMobj (thing);

    return;
    }

  if (! (thing->flags & MF_SHOOTABLE) )
    {
    // assume it is bloody gibs or something
    return;
    }

  nofit = true;
  }


//
// P_CheckSector
// jff 3/19/98 added to just check monsters on the periphery
// of a moving sector instead of all in bounding box of the
// sector. Both more accurate and faster.
//

static boolean P_CheckSector(sector_t __far* sector)
  {
  msecnode_t *n;

  nofit = false;

  // killough 4/4/98: scan list front-to-back until empty or exhausted,
  // restarting from beginning after each thing is processed. Avoids
  // crashes, and is sure to examine all things in the sector, and only
  // the things which are in the sector, until a steady-state is reached.
  // Things can arbitrarily be inserted and removed and it won't mess up.
  //
  // killough 4/7/98: simplified to avoid using complicated counter

  // Mark all things invalid

  for (n=sector->touching_thinglist; n; n=n->m_snext)
    n->visited = false;

  do
    for (n=sector->touching_thinglist; n; n=n->m_snext)  // go through list
      if (!n->visited)               // unprocessed thing found
        {
        n->visited  = true;          // mark thing as processed
        if (!(n->m_thing->flags & MF_NOBLOCKMAP)) //jff 4/7/98 don't do these
          PIT_ChangeSector(n->m_thing);    // process it
        break;                 // exit and start over
        }
  while (n);  // repeat from scratch until all things left are marked valid

  return nofit;
  }


///////////////////////////////////////////////////////////////////////
//
// Plane (floor or ceiling), Floor motion and Elevator action routines
//
///////////////////////////////////////////////////////////////////////

//
// T_MovePlane()
//
// Move a plane (floor or ceiling) and check for crushing. Called
// every tick by all actions that move floors or ceilings.
//
// Passed the sector to move a plane in, the speed to move it at,
// the dest height it is to achieve, whether it crushes obstacles,
// whether it moves a floor or ceiling, and the direction up or down
// to move.
//
// Returns a result_e:
//  ok - plane moved normally, has not achieved destination yet
//  pastdest - plane moved normally and is now at destination height
//  crushed - plane encountered an obstacle, is holding until removed
//
result_e T_MovePlaneFloor(sector_t __far* sector, fixed_t speed, fixed_t dest, int16_t direction)
{
  boolean       flag;
  fixed_t       lastpos;
  fixed_t       destheight; //jff 02/04/98 used to keep floors/ceilings
                            // from moving thru each other

      switch(direction)
      {
        case -1:
          // Moving a floor down
          if (sector->floorheight - speed < dest)
          {
            lastpos = sector->floorheight;
            sector->floorheight = dest;
            flag = P_CheckSector(sector);
            if (flag == true)
            {
              sector->floorheight =lastpos;
              P_CheckSector(sector);
            }
            return pastdest;
          }
          else
          {
            lastpos = sector->floorheight;
            sector->floorheight -= speed;
            flag = P_CheckSector(sector);
      /* cph - make more compatible with original Doom, by
       *  reintroducing this code. This means floors can't lower
       *  if objects are stuck in the ceiling */
          }
          break;

        case 1:
          // Moving a floor up
          // jff 02/04/98 keep floor from moving thru ceilings
          // jff 2/22/98 weaken check to demo_compatibility
          destheight = (dest<sector->ceilingheight)?
                          dest : sector->ceilingheight;
          if (sector->floorheight + speed > destheight)
          {
            lastpos = sector->floorheight;
            sector->floorheight = destheight;
            flag = P_CheckSector(sector);
            if (flag == true)
            {
              sector->floorheight = lastpos;
              P_CheckSector(sector);
            }
            return pastdest;
          }
          else
          {
            // crushing is possible
            lastpos = sector->floorheight;
            sector->floorheight += speed;
            flag = P_CheckSector(sector);
            if (flag == true)
            {

              sector->floorheight = lastpos;
              P_CheckSector(sector);
              return crushed;
            }
          }
          break;
      }

  return ok;
}


result_e T_MovePlaneCeiling(sector_t __far* sector, fixed_t speed, fixed_t dest, int16_t direction)
{
  boolean       flag;
  fixed_t       lastpos;
  fixed_t       destheight; //jff 02/04/98 used to keep floors/ceilings
                            // from moving thru each other

      switch(direction)
      {
        case -1:
          // moving a ceiling down
          // jff 02/04/98 keep ceiling from moving thru floors
          // jff 2/22/98 weaken check to demo_compatibility
          destheight = (dest>sector->floorheight)?
                          dest : sector->floorheight;
          if (sector->ceilingheight - speed < destheight)
          {
            lastpos = sector->ceilingheight;
            sector->ceilingheight = destheight;
            flag = P_CheckSector(sector);

            if (flag == true)
            {
              sector->ceilingheight = lastpos;
              P_CheckSector(sector);
            }
            return pastdest;
          }
          else
          {
            // crushing is possible
            lastpos = sector->ceilingheight;
            sector->ceilingheight -= speed;
            flag = P_CheckSector(sector);

            if (flag == true)
            {
              sector->ceilingheight = lastpos;
              P_CheckSector(sector);
              return crushed;
            }
          }
          break;

        case 1:
          // moving a ceiling up
          if (sector->ceilingheight + speed > dest)
          {
            lastpos = sector->ceilingheight;
            sector->ceilingheight = dest;
            flag = P_CheckSector(sector);
            if (flag == true)
            {
              sector->ceilingheight = lastpos;
              P_CheckSector(sector);
            }
            return pastdest;
          }
          else
          {
            lastpos = sector->ceilingheight;
            sector->ceilingheight += speed;
            flag = P_CheckSector(sector);
          }
          break;
      }

  return ok;
}


//
// T_MoveFloor()
//
// Move a floor to it's destination (up or down).
// Called once per tick for each moving floor.
//
// Passed a floormove_t structure that contains all pertinent info about the
// move. See P_SPEC.H for fields.
// No return.
//
// jff 02/08/98 all cases with labels beginning with gen added to support
// generalized line type behaviors.

static void T_MoveFloor(floormove_t __far* floor)
{
  result_e      res;

  // move the floor
  res = T_MovePlaneFloor(floor->sector, floor->speed, floor->floordestheight, floor->direction);

  if (res == pastdest)    // if destination height is reached
  {
    if (floor->direction == 1)       // going up
    {
      switch(floor->type) // handle texture/type changes
      {
        case donutRaise:
          floor->sector->special  = 0;
          floor->sector->floorpic = floor->texture;
          break;
        default:
          break;
      }
    }

    floor->sector->floordata = NULL; //jff 2/22/98
    P_RemoveThinker(&floor->thinker);//remove this floor from list of movers
  }
}


//
// P_FindNextHighestFloor()
//
// Passed a sector, returns the fixed point value
// of the smallest floor height in a surrounding sector larger than
// the floor height passed. If no such height exists the floorheight
// of the passed sector is returned.
//
// Rewritten by Lee Killough to avoid fixed array and to be faster
//
fixed_t P_FindNextHighestFloor(sector_t __far* sec)
{
  fixed_t currentheight = sec->floorheight;
  sector_t __far* other;
  int16_t i;

  for (i=0 ;i < sec->linecount ; i++)
    if ((other = getNextSector(sec->lines[i],sec)) &&
         other->floorheight > currentheight)
    {
      fixed_t height = other->floorheight;
      while (++i < sec->linecount)
        if ((other = getNextSector(sec->lines[i],sec)) &&
            other->floorheight < height &&
            other->floorheight > currentheight)
          height = other->floorheight;
      return height;
    }
  /* cph - my guess at doom v1.2 - 1.4beta compatibility here.
   * If there are no higher neighbouring sectors, Heretic just returned
   * heightlist[0] (local variable), i.e. noise off the stack. 0 is right for
   * RETURN01 E1M2, so let's take that. */
  return currentheight;
}


///////////////////////////////////////////////////////////////////////
//
// Floor motion linedef handlers
//
///////////////////////////////////////////////////////////////////////

//
// EV_DoFloor()
//
// Handle regular and extended floor types
//
// Passed the line that activated the floor and the type of floor motion
// Returns true if a thinker was created.
//
boolean EV_DoFloor(const line_t __far* line, floor_e floortype)
{
  int16_t           secnum;
  boolean           rtn;
  sector_t __far*     sec;
  floormove_t __far*  floor;

  secnum = -1;
  rtn = false;
  // move all floors with the same tag as the linedef
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &_g_sectors[secnum];

    // Don't start a second thinker on the same floor
    if (sec->floordata != NULL)
      continue;

    // new floor thinker
    rtn = true;
    floor = Z_CallocLevSpec(sizeof(*floor));
    P_AddThinker (&floor->thinker);
    sec->floordata = floor; //jff 2/22/98
    floor->thinker.function = T_MoveFloor;
    floor->type = floortype;

    // setup the thinker according to the linedef type
    switch(floortype)
    {
      case lowerFloor:
        floor->direction = -1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = P_FindHighestFloorSurrounding(sec);
        break;

      case lowerFloorToLowest:
        floor->direction = -1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = P_FindLowestFloorSurrounding(sec);
        break;

      case turboLower:
        floor->direction = -1;
        floor->sector = sec;
        floor->speed = FLOORSPEED * 4;
        floor->floordestheight = P_FindHighestFloorSurrounding(sec);
        if (floor->floordestheight != sec->floorheight)
          floor->floordestheight += 8*FRACUNIT;
        break;

      case raiseFloor:
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = P_FindLowestCeilingSurrounding(sec);
        if (floor->floordestheight > sec->ceilingheight)
          floor->floordestheight = sec->ceilingheight;
        break;

      case raiseFloorToNearest:
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = P_FindNextHighestFloor(sec);
        break;

      default:
        break;
    }
  }
  return rtn;
}

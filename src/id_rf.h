/* Catacomb 3-D Source Code
 * Copyright (C) 1993-2014 Flat Rock Software
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

// ID_RF.H

#define __ID_RF__

#ifndef __ID_MM__
#include "id_mm.h"
#endif

/*
=============================================================================

							CONSTANTS

=============================================================================
*/

#define	MINTICS				2
#define	MAXTICS				6
#define DEMOTICS			3

#define	MAPBORDER			2		// map border must be at least 1

#define	MAXSPRITES			50		// max tracked sprites
#define	MAXANIMTILES		90		// max animating tiles on screen
#define MAXANIMTYPES		50		// max different unique anim tiles on map

#define	MAXMAPHEIGHT		200

#define	PRIORITIES			4
#define	MASKEDTILEPRIORITY	3		// planes go: 0,1,2,MTILES,3

#define TILEGLOBAL			256
#define PIXGLOBAL			16

#define	G_T_SHIFT			8		// global >> ?? = tile
#define	G_P_SHIFT			4		// global >> ?? = pixels
#define P_T_SHIFT			4		// pixels >> ?? = tile

#define	PORTTILESWIDE		21      // all drawing takes place inside a
#define	PORTTILESHIGH		14		// non displayed port of this size

//#define	PORTGLOBALWIDE		(21*TILEGLOBAL)
//#define	PORTGLOBALHIGH		(14*TILEGLOBAL)

#define UPDATEWIDE			(PORTTILESWIDE+1)
#define UPDATEHIGH			PORTTILESHIGH


//===========================================================================

typedef enum {spritedraw,maskdraw} drawtype;

/*
=============================================================================

						 PUBLIC VARIABLES

=============================================================================
*/


extern	id0_boolean_t		compatability;			// crippled refresh for wierdo SVGAs

extern	id0_unsigned_t	tics;
extern	id0_long_t		lasttimecount;

extern	id0_unsigned_t	originxglobal,originyglobal;
extern	id0_unsigned_t	originxtile,originytile;
extern	id0_unsigned_t	originxscreen,originyscreen;

extern	id0_unsigned_t	mapwidth,mapheight,mapbyteswide,mapwordswide
					,mapbytesextra,mapwordsextra;
extern	id0_unsigned_t	mapbwidthtable[MAXMAPHEIGHT];

extern	id0_unsigned_t	originxmin,originxmax,originymin,originymax;

extern	id0_unsigned_t	masterofs;

//
// the floating update window is also used by the view manager for
// double buffer tracking
//

extern	id0_byte_t		*updateptr;				// current start of update window

#if GRMODE == CGAGR
extern	id0_byte_t		*baseupdateptr;
#endif

extern id0_unsigned_t	blockstarts[UPDATEWIDE*UPDATEHIGH];
extern id0_unsigned_t	updatemapofs[UPDATEWIDE*UPDATEHIGH];
extern id0_unsigned_t	uwidthtable[UPDATEHIGH];		// lookup instead of multiple

#define	UPDATETERMINATE	0x0301

/*
=============================================================================

						 PUBLIC FUNCTIONS

=============================================================================
*/

void RF_Startup (void);
void RF_Shutdown (void);

void RF_FixOfs (void);
void RF_NewMap (void);
void RF_MarkTileGraphics (void);
void RF_SetScrollBlock (id0_int_t x, id0_int_t y, id0_boolean_t horizontal);
void RF_NewPosition (id0_unsigned_t x, id0_unsigned_t y);
void RF_Scroll (id0_int_t x, id0_int_t y);

void RF_MapToMap (id0_unsigned_t srcx, id0_unsigned_t srcy,
				  id0_unsigned_t destx, id0_unsigned_t desty,
				  id0_unsigned_t width, id0_unsigned_t height);
void RF_MemToMap (id0_unsigned_t id0_far *source, id0_unsigned_t plane,
				  id0_unsigned_t destx, id0_unsigned_t desty,
				  id0_unsigned_t width, id0_unsigned_t height);

void RF_ClearBlock (id0_int_t	x, id0_int_t y, id0_int_t width, id0_int_t height);
void RF_RedrawBlock (id0_int_t x, id0_int_t y, id0_int_t width, id0_int_t height);

void RF_PlaceSprite (void **user,id0_unsigned_t globalx,id0_unsigned_t globaly,
	id0_unsigned_t spritenumber, drawtype draw, id0_int_t priority);
void RF_RemoveSprite (void **user);

void RF_CalcTics (void);

void RF_Refresh (void);
void RF_ForceRefresh (void);
void RF_SetRefreshHook (void (*func) (void) );

id0_unsigned_t RF_FindFreeBuffer (void);


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

 * ID_RF_A.C
 */

// Ported from ASM

#include "id_heads.h"
#include "id_asm.h"
#include "id_rf.h" // Had to add it, probably because it's unused in Cat3D

// TODO (Chocolate Cat3D): IMPLEMENT EGA SUPPORT!
// (EGA read/write modes can have unexpected effects - anything that
// involves the address A000:0000 or so is probably under effect.)

//============================================================================

#define TILESWIDE 21
#define TILESHIGH 14

//#define UPDATESIZE ((TILESWIDE+1)*TILESHIGH+1)

extern id0_unsigned_t screenstart[3]; // starts of three screens (0/1/master) in EGA mem
extern id0_unsigned_t originmap;
extern id0_byte_t id0_seg *tinf;

id0_byte_t planemask;
id0_byte_t planenum;

id0_unsigned_t screenstartcs; // in code segment for accesability




#if GRMODE == CGAGR
//============================================================================
//
// CGA refresh routines
//
//============================================================================

#define TILEWIDTH 4

//=================
//
// RFL_NewTile
//
// Draws a composit two plane tile to the master screen and sets the update
// spot to 1 in both update pages, forcing the tile to be copied to the
// view pages the next two refreshes
//
// Called to draw newlly scrolled on strips and animating tiles
//
//=================

void RFL_NewTile (id0_unsigned_t updateoffset)
{
	updateptr[updateoffset] = 1;
	// offset in map from origin
	id0_unsigned_t currmapoffset = updatemapofs[updateoffset]+originmap;
	// screen location for tile
	id0_unsigned_t currtileoffset = blockstarts[updateoffset]+masterofs;
	// destination pointer on screen
	uint8_t *destPtr = &screenseg[currtileoffset];
	// If either of the tile numbers is 0xFFFF, the tile does not need to
	// be masked together; as one of the planes totally eclipses the other
	id0_unsigned_t foretilenum = mapsegs[1][currmapoffset/2];
	id0_unsigned_t backtilenum = mapsegs[0][currmapoffset/2];
	if (!foretilenum)
	{
		//=============
		//
		// Draw single background tile from main memory
		//
		//=============
		uint8_t *backSrcPtr = grsegs[STARTTILE16+backtilenum];
		
		for (int loopVar = 15; loopVar; --loopVar, backSrcPtr += TILEWIDTH, destPtr += SCREENWIDTH)
		{
			memcpy(destPtr, backSrcPtr, TILEWIDTH);
		}
		memcpy(destPtr, backSrcPtr, TILEWIDTH);
		return;
	}
	//=========
	//
	// Draw a masked tile combo
	//
	//=========
	uint8_t *foreSrcPtr = grsegs[STARTTILE16M+foretilenum];
	uint8_t *backSrcPtr = grsegs[STARTTILE16+backtilenum];

	for (int loopVar = 16; loopVar; --loopVar, backSrcPtr += TILEWIDTH, foreSrcPtr += TILEWIDTH, destPtr += SCREENWIDTH)
	{
		// backSrcPtr - background tile
		// foreSrcPtr - mask
		// &foreSrcPtr[64] - masked data
		*(uint32_t *)destPtr = ((*(uint32_t *)backSrcPtr) & (*(uint32_t *)foreSrcPtr))
		                       | ((uint32_t *)foreSrcPtr)[16];
	}
}
#endif



#if GRMODE == EGAGR
;===========================================================================
;
; EGA refresh routines
;
;===========================================================================

TILEWIDTH	=	2

;=================
;
; RFL_NewTile
;
; Draws a composit two plane tile to the master screen and sets the update
; spot to 1 in both update pages, forcing the tile to be copied to the
; view pages the next two refreshes
;
; Called to draw newlly scrolled on strips and animating tiles
;
; Assumes write mode 0
;
;=================

PROC	RFL_NewTile	updateoffset:WORD
PUBLIC	RFL_NewTile
USES	SI,DI

;
; mark both update lists at this spot
;
	mov	di,[updateoffset]

	mov	bx,[updatestart]		;page 0 pointer
	mov	[BYTE bx+di],1
	mov	bx,[updatestart+2]		;page 1 pointer
	mov	[BYTE bx+di],1

;
; set screenstartcs to the location in screenseg to draw the tile
;
	shl	di,1
	mov	si,[updatemapofs+di]	;offset in map from origin
	add	si,[originmap]
	mov	di,[blockstarts+di]		;screen location for tile
	add	di,[masterofs]
	mov	[cs:screenstartcs],di

;
; set BX to the foreground tile number and SI to the background number
; If either BX or SI = 0xFFFF, the tile does not need to be masked together
; as one of the planes totally eclipses the other
;
	mov	es,[mapsegs+2]			;foreground plane
	mov	bx,[es:si]
	mov	es,[mapsegs]			;background plane
	mov	si,[es:si]

	mov	es,[screenseg]
	mov	dx,SC_INDEX				;for stepping through map mask planes

	or	bx,bx
	jz	@@singletile
	jmp	@@maskeddraw			;draw both together

;=========
;
; No foreground tile, so draw a single background tile.
;
;=========
@@singletile:

	mov	bx,SCREENWIDTH-2		;add to get to start of next line
	shl	si,1

	mov	ax,[cs:screenstartcs]
	mov	ds,[grsegs+STARTTILE16*2+si]

	xor	si,si					;block is segment aligned

	mov	ax,SC_MAPMASK+0001b*256	;map mask for plane 0

	mov	cx,4					;draw four planes
@@planeloop:
	mov	dx,SC_INDEX
	WORDOUT

	mov	di,[cs:screenstartcs]	;start at same place in all planes

REPT	15
	movsw
	add	di,bx
ENDM
	movsw

	shl	ah,1					;shift plane mask over for next plane
	loop	@@planeloop

	mov	ax,ss
	mov	ds,ax					;restore turbo's data segment
	ret


;=========
;
; Draw a masked tile combo
; Interupts are disabled and the stack segment is reassigned
;
;=========
@@maskeddraw:
	cli							; don't allow ints when SS is set
	shl	bx,1
	mov	ss,[grsegs+STARTTILE16M*2+bx]
	shl	si,1
	mov	ds,[grsegs+STARTTILE16*2+si]

	xor	si,si					;first word of tile data

	mov	ax,SC_MAPMASK+0001b*256	;map mask for plane 0

	mov	di,[cs:screenstartcs]
@@planeloopm:
	WORDOUT
tileofs		=	0
lineoffset	=	0
REPT	16
	mov	bx,[si+tileofs]			;background tile
	and	bx,[ss:tileofs]			;mask
	or	bx,[ss:si+tileofs+32]	;masked data
	mov	[es:di+lineoffset],bx
tileofs		=	tileofs + 2
lineoffset	=	lineoffset + SCREENWIDTH
ENDM
	add	si,32
	shl	ah,1					;shift plane mask over for next plane
	cmp	ah,10000b
	je	@@done					;drawn all four planes
	jmp	@@planeloopm

@@done:
	mov	ax,@DATA
	mov	ss,ax
	sti
	mov	ds,ax
	ret
ENDP

#endif

#if GRMODE == VGAGR
;============================================================================
;
; VGA refresh routines
;
;============================================================================


#endif


//============================================================================
//
// reasonably common refresh routines
//
//============================================================================


//=================
//
// RFL_UpdateTiles
//
// Scans through the update matrix pointed to by updateptr, looking for 1s.
// A 1 represents a tile that needs to be copied from the master screen to the
// current screen (a new row or an animated tiled).  If more than one adjacent
// tile in a horizontal row needs to be copied, they will be copied as a group.
//
// Assumes write mode 1
//
//=================

void RFL_UpdateTiles (void)
{
	// TODO (Chocolate Cat3D): IMPLEMENT FOR EGA! (write mode 1)
	id0_byte_t *scanPtr = updateptr;
	id0_byte_t *scanEndPtr = updateptr + (TILESWIDE+1)*TILESHIGH+1;
	id0_word_t iterationsToDo = 0xFFFF; // definitely scan the entire thing

	do
	{
		/*
		 * scan for a 1 in the update list, meaning a tile needs
		 * to be copied from the master screen to the current screen
		 */
		for (; iterationsToDo && (*scanPtr != 1); --iterationsToDo, ++scanPtr);
		if (scanPtr == scanEndPtr)
		{
			return; // Nothing left
		}
		if (*scanPtr != 1)
		{
			//============
			//
			// copy a single tile
			//
			//============
			++scanPtr; // we know the next tile is nothing
			uint16_t tileLoc = blockstarts[scanPtr-updateptr-2]; // start of tile location on screen
			uint8_t *destPtr = &screenseg[tileLoc+bufferofs]; // dest in current screen
			uint8_t *srcPtr = &screenseg[tileLoc+masterofs]; // source in master screen
			// Originally there were separate codepaths for EGA
			// and CGA only; But we can use TILEWIDTH now
			// (4 for CGA, 2 for EGA)
#if (GRMODE == CGAGR) || (GRMODE == EGACR)
			for (int loopVar = 15; loopVar; --loopVar)
			{
				memcpy(destPtr, srcPtr, TILEWIDTH);
				srcPtr += SCREENWIDTH;
				destPtr += SCREENWIDTH;
			}
			memcpy(destPtr, srcPtr, TILEWIDTH);
			srcPtr += TILEWIDTH;
			destPtr += TILEWIDTH;
			continue;

#endif
		}
		//============
		//
		// more than one tile in a row needs to be updated, so do it as a group
		//
		//============
		id0_byte_t *rowScanStartPtr = scanPtr; // hold starting position + 1
		++scanPtr; // we know the next tile also gets updated
		// see how many more in a row
		for (; iterationsToDo && (*scanPtr == 1); --iterationsToDo, ++scanPtr);
		uint16_t bytesPerRow = (scanPtr - rowScanStartPtr) << 1;

		uint16_t tileLoc = blockstarts[rowScanStartPtr-updateptr-1]; // start of tile location
		uint8_t *destPtr = &screenseg[tileLoc+bufferofs]; // dest in current screen
		uint8_t *srcPtr = &screenseg[tileLoc+masterofs]; // source in master screen
#if (GRMODE == CGAGR)
		uint16_t bytesToSkip = SCREENWIDTH-2*bytesPerRow; // words wide in CGA tiles
#else
		uint16_t bytesToSkip = SCREEENWIDTH-bytesPerRow;
#endif
		for (int loopVar = 15; loopVar; --loopVar)
		{
			iterationsToDo = bytesPerRow;
#if (GRMODE == CGAGR) || (GRMODE == EGACR)
			memcpy(destPtr, srcPtr, iterationsToDo*(TILEWIDTH/2));
			srcPtr += iterationsToDo*(TILEWIDTH/2);
			destPtr += iterationsToDo*(TILEWIDTH/2);
			iterationsToDo = 0;
#endif
			srcPtr += bytesToSkip;
			destPtr += bytesToSkip;
		}
		iterationsToDo = bytesPerRow;
#if (GRMODE == CGAGR) || (GRMODE == EGACR)
		memcpy(destPtr, srcPtr, iterationsToDo*(TILEWIDTH/2));
		srcPtr += iterationsToDo*(TILEWIDTH/2);
		destPtr += iterationsToDo*(TILEWIDTH/2);
		iterationsToDo = 0;
#endif
		// was 0, now 0xFFFF for above loop
		// WARNING: This should be UNSIGNED, or else we get undefined behaviors
		--iterationsToDo;
	} while (true);
}

//============================================================================


//=================
//
// RFL_MaskForegroundTiles
//
// Scan through update looking for 3's.  If the foreground tile there is a
// masked foreground tile, draw it to the screen
//
//=================

void RFL_MaskForegroundTiles (void)
{
	id0_byte_t *scanPtr = updateptr;
	id0_byte_t *scanEndPtr = updateptr + (TILESWIDE+1)*TILESHIGH+2;
	id0_word_t iterationsToDo = 0xFFFF; // definitely scan the entire thing
	do
	{
		/*
		 * scan for a 3 in the updates list
		 */
		for (; iterationsToDo && (*scanPtr != 3); --iterationsToDo, ++scanPtr);
		if (scanPtr == scanEndPtr)
		{
			return; // Nothing left
		}

		//============
		//
		// found a tile, see if it needs to be masked on
		//
		//============

		uint16_t offsettedoriginmap = updatemapofs[scanPtr-updateptr-1] + originmap;
		uint16_t foreTileNum = mapsegs[1][offsettedoriginmap/2];
		if (!foreTileNum) // 0 = no foreground tile
		{
			continue;
		}
		if (!(tinf[foreTileNum+INTILE] & 0x80)) // high bit = masked tile
		{
			continue;
		}

#if (GRMODE == CGAGR)
		//=================
		//
		// mask the tile CGA
		//
		//=================

		uint16_t tileLoc = blockstarts[scanPtr-updateptr-1];
		uint8_t *destPtr = &screenseg[tileLoc + bufferofs];
		uint8_t *srcPtr = grsegs[STARTTILE16M+foreTileNum];

		for (int loopVar = 16, lineoffset = 0; loopVar; --loopVar, srcPtr += 4, destPtr += SCREENWIDTH)
		{
			*((uint32_t *)destPtr) =
				((*((uint32_t *)destPtr)) // background
				 & (*(uint32_t *)srcPtr) // mask
				) | (*(uint32_t *)(srcPtr+64)); // masked data
			srcPtr += 4;
		}
#endif

#if (GRMODE == EGAGR)
		;=================
		;
		; mask the tile
		;
		;=================

		mov	[BYTE planemask],1
		mov	[BYTE planenum],0

		mov	di,[blockstarts-2+di]
		add	di,[bufferofs]
		mov	[cs:screenstartcs],di
		mov	es,[screenseg]
		shl	si,1
		mov	ds,[grsegs+STARTTILE16M*2+si]

		mov	bx,32					;data starts 32 bytes after mask

	@@planeloopm:
		mov	dx,SC_INDEX
		mov	al,SC_MAPMASK
		mov	ah,[ss:planemask]
		WORDOUT
		mov	dx,GC_INDEX
		mov	al,GC_READMAP
		mov	ah,[ss:planenum]
		WORDOUT

		xor	si,si
		mov	di,[cs:screenstartcs]
	lineoffset	=	0
	REPT	16
		mov	cx,[es:di+lineoffset]	;background
		and	cx,[si]					;mask
		or	cx,[si+bx]				;masked data
		inc	si
		inc	si
		mov	[es:di+lineoffset],cx
	lineoffset	=	lineoffset + SCREENWIDTH
	ENDM
		add	bx,32					;the mask is now further away
		inc	[ss:planenum]
		shl	[ss:planemask],1		;shift plane mask over for next plane
		cmp	[ss:planemask],10000b	;done all four planes?
		je	@@drawn					;drawn all four planes
		jmp	@@planeloopm

	@@drawn:
#endif
		iterationsToDo = 0xFFFF; // definitely scan the entire thing
	} while (true);
}

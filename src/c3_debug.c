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

// C3_DEBUG.C

#include "C3_DEF.H"
#pragma hdrstop

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#define VIEWTILEX	20
#define VIEWTILEY	(VIEWHEIGHT/16)

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/


/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/


id0_int_t	maporgx;
id0_int_t	maporgy;
enum {mapview,tilemapview,actoratview,visview}	viewtype;

void ViewMap (void);

//===========================================================================



/*
==================
=
= DebugMemory
=
==================
*/

void DebugMemory (void)
{
	id0_int_t	i;
	id0_char_t    scratch[80],str[10];
	id0_long_t	mem;
	spritetype id0_seg	*block;

	VW_FixRefreshBuffer ();
	US_CenterWindow (16,7);

#if 0
	CA_OpenDebug ();
	for (i=0;i<NUMCHUNKS;i++)
	{
		if (grsegs[i])
		{
			strcpy (scratch,"Chunk:");
			itoa (i,str,10);
			strcat (scratch,str);
			strcat (scratch,"\n");
			write (debughandle,scratch,strlen(scratch));
		}
	}
	CA_CloseDebug ();
#endif

	US_CPrint ("Memory Usage");
	US_CPrint ("------------");
	US_Print ("Total     :");
	US_PrintUnsigned (mminfo.mainmem/1024);
	US_Print ("k\nFree      :");
	US_PrintUnsigned (MM_UnusedMemory()/1024);
	US_Print ("k\nWith purge:");
	US_PrintUnsigned (MM_TotalFree()/1024);
	US_Print ("k\n");
	VW_UpdateScreen();
	IN_Ack ();
}

//===========================================================================

/*
================
=
= PicturePause
=
================
*/

void PicturePause (void)
{
	id0_int_t	y;
	id0_unsigned_t	source;

	source = displayofs+panadjust;

	VW_ColorBorder (15);
	VW_SetLineWidth (40);
	VW_SetScreen (0,0);

	if (source<0x10000l-200*64)
	{
	//
	// copy top line first
	//
		for (y=0;y<200;y++)
			VW_ScreenToScreen (source+y*64,y*40,40,1);
	}
	else
	{
	//
	// copy bottom line first
	//
		for (y=199;y>=0;y--)
			VW_ScreenToScreen (source+y*64,y*40,40,1);
	}

	IN_Shutdown ();

	VW_WaitVBL(70);
	bioskey(0);
	VW_WaitVBL(70);
	Quit (NULL);
}


//===========================================================================

/*
================
=
= ShapeTest
=
================
*/

void ShapeTest (void)
{

}


//===========================================================================

#define	sc_1			0x02
#define	sc_2			0x03
#define	sc_3			0x04
#define	sc_4			0x05
#define	sc_5			0x06
#define	sc_6			0x07
#define	sc_7			0x08
#define	sc_8			0x09
#define	sc_9			0x0a
#define	sc_0			0x0b



/*
================
=
= DebugKeys
=
================
*/

id0_int_t DebugKeys (void)
{
	id0_boolean_t esc;
	id0_int_t level,i;

	if (Keyboard[sc_B])		// B = border color
	{
		CenterWindow(24,3);
		PrintY+=6;
		US_Print(" Border color (0-15):");
		VW_UpdateScreen();
		esc = !US_LineInput (px,py,str,NULL,true,2,0);
		if (!esc)
		{
			level = atoi (str);
			if (level>=0 && level<=15)
				VW_ColorBorder (level);
		}
		return 1;
	}

#if 0

	if (Keyboard[sc_C])		// C = count objects
	{
		CountObjects();
		return 1;
	}


	if (Keyboard[sc_D])		// D = start / end demo record
	{
		if (DemoMode == demo_Off)
			StartDemoRecord ();
		else if (DemoMode == demo_Record)
		{
			EndDemoRecord ();
			playstate = ex_completed;
		}
		return 1;
	}

#endif

	if (Keyboard[sc_E])		// E = quit level
	{
		if (tedlevel)
			TEDDeath();
		playstate = ex_warped;
		gamestate.mapon++;
	}

	if (Keyboard[sc_F])		// F = facing spot
	{
		CenterWindow (12,4);
		US_Print ("X:");
		US_PrintUnsigned (player->x);
		US_Print ("Y:");
		US_PrintUnsigned (player->y);
		US_Print ("A:");
		US_PrintUnsigned (player->angle);
		VW_UpdateScreen();
		IN_Ack();
		return 1;
	}

	if (Keyboard[sc_G])		// G = god mode
	{
		CenterWindow (12,2);
		if (godmode)
		  US_PrintCentered ("God mode OFF");
		else
		  US_PrintCentered ("God mode ON");
		VW_UpdateScreen();
		IN_Ack();
		godmode ^= 1;
		return 1;
	}
	if (Keyboard[sc_H])		// H = hurt self
	{
		TakeDamage (5);
	}
	else if (Keyboard[sc_I])			// I = item cheat
	{
		CenterWindow (12,3);
		US_PrintCentered ("Free items!");
		VW_UpdateScreen();
		for (i=0;i<4;i++)
		{
			GiveBolt ();
			GiveNuke ();
			GivePotion ();
			if (!gamestate.keys[i])
				GiveKey (i);
		}
		for (i=0;i<8;i++)
			GiveScroll (i,false);

		IN_Ack ();
		return 1;
	}
	else if (Keyboard[sc_M])			// M = memory info
	{
		DebugMemory();
		return 1;
	}
	else if (Keyboard[sc_O])			// O = overhead
	{
		ViewMap();
		return 1;
	}
	else if (Keyboard[sc_P])			// P = pause with no screen disruptioon
	{
		PicturePause ();
		return 1;
	}
	else if (Keyboard[sc_S])	// S = slow motion
	{
		singlestep^=1;
		CenterWindow (18,3);
		if (singlestep)
			US_PrintCentered ("Slow motion ON");
		else
			US_PrintCentered ("Slow motion OFF");
		VW_UpdateScreen();
		IN_Ack ();
		return 1;
	}
	else if (Keyboard[sc_S])	// T = shape test
	{
		ShapeTest ();
		return 1;
	}
	else if (Keyboard[sc_V])			// V = extra VBLs
	{
		CenterWindow(30,3);
		PrintY+=6;
		US_Print("  Add how many extra VBLs(0-8):");
		VW_UpdateScreen();
		esc = !US_LineInput (px,py,str,NULL,true,2,0);
		if (!esc)
		{
			level = atoi (str);
			if (level>=0 && level<=8)
				extravbls = level;
		}
		return 1;
	}
	else if (Keyboard[sc_W])	// W = warp to level
	{
		CenterWindow(26,3);
		PrintY+=6;
		US_Print("  Warp to which level(1-21):");
		VW_UpdateScreen();
		esc = !US_LineInput (px,py,str,NULL,true,2,0);
		if (!esc)
		{
			level = atoi (str);
			if (level>0 && level<21)
			{
				gamestate.mapon = level-1;
				playstate = ex_warped;
			}
		}
		return 1;
	}
	else if (Keyboard[sc_X])			// X = item cheat
	{
		CenterWindow (12,3);
		US_PrintCentered ("Extra stuff!");
		VW_UpdateScreen();
		for (i=0;i<4;i++)
		{
			GiveBolt ();
			GiveNuke ();
			GivePotion ();
		}
		IN_Ack ();
		return 1;
	}
	else if (Keyboard[sc_Z])			// Z = game over
	{

	}
	else if (LastScan >= sc_1 && LastScan <= sc_8)	// free scrolls
	{
		GiveScroll (LastScan-sc_1,false);
		IN_ClearKeysDown ();
	}

	return 0;
}


/*
=====================
=
= LatchDrawChar
=
=====================
*/

void LatchDrawChar (id0_unsigned_t x, id0_unsigned_t y, id0_unsigned_t picnum)
{
	id0_unsigned_t	source, dest;

	dest = bufferofs + ylookup[y]+x;
	source = latchpics[0]+picnum*8;

	EGAWRITEMODE(1);
	EGAMAPMASK(15);

asm	mov	bx,[linewidth]
asm	dec	bx

asm	mov	ax,[screenseg]
asm	mov	es,ax
asm	mov	ds,ax

asm	mov	si,[source]
asm	mov	di,[dest]

asm	movsb
asm	add	di,bx
asm	movsb
asm	add	di,bx
asm	movsb
asm	add	di,bx
asm	movsb
asm	add	di,bx
asm	movsb
asm	add	di,bx
asm	movsb
asm	add	di,bx
asm	movsb
asm	add	di,bx
asm	movsb

asm	mov	ax,ss
asm	mov	ds,ax					// restore turbo's data segment

	EGAWRITEMODE(0);
}


/*
=====================
=
= LatchDrawTile
=
=====================
*/

void LatchDrawTile (id0_unsigned_t x, id0_unsigned_t y, id0_unsigned_t picnum)
{
	id0_unsigned_t	source, dest;

	dest = bufferofs + ylookup[y]+x;
	source = tileoffsets[picnum];

	EGAWRITEMODE(1);
	EGAMAPMASK(15);

asm	mov	bx,[linewidth]
asm	sub	bx,2

asm	mov	ax,[screenseg]
asm	mov	es,ax
asm	mov	ds,ax

asm	mov	si,[source]
asm	mov	di,[dest]
asm	mov	dx,16

lineloop:
asm	movsb
asm	movsb
asm	add	di,bx

asm	dec	dx
asm	jnz	lineloop

asm	mov	ax,ss
asm	mov	ds,ax					// restore turbo's data segment

	EGAWRITEMODE(0);
}


/*
===================
=
= OverheadRefresh
=
===================
*/

void OverheadRefresh (void)
{
	id0_unsigned_t	x,y,endx,endy,sx,sy;
	id0_unsigned_t	tile;


	if (++screenpage == 3)
		screenpage = 0;

	bufferofs = screenloc[screenpage];

	endx = maporgx+VIEWTILEX;
	endy = maporgy+VIEWTILEY;

	for (y=maporgy;y<endy;y++)
		for (x=maporgx;x<endx;x++)
		{
			sx = (x-maporgx)*2;
			sy = (y-maporgy)*16;

			switch (viewtype)
			{
			case mapview:
				tile = *(mapsegs[0]+farmapylookup[y]+x);
				break;

			case tilemapview:
				tile = tilemap[x][y];
				break;

			case actoratview:
				tile = (id0_unsigned_t)actorat[x][y];
				break;

			case visview:
				tile = spotvis[x][y];
				break;

			}

			if (tile<NUMTILE16)
				LatchDrawTile(sx,sy,tile);
			else
			{
				LatchDrawChar(sx,sy,NUMBERCHARS+((tile&0xf000)>>12));
				LatchDrawChar(sx+1,sy,NUMBERCHARS+((tile&0x0f00)>>8));
				LatchDrawChar(sx,sy+8,NUMBERCHARS+((tile&0x00f0)>>4));
				LatchDrawChar(sx+1,sy+8,NUMBERCHARS+(tile&0x000f));
			}
		}

	VW_SetScreen (bufferofs,0);
	displayofs = bufferofs;
}


/*
===================
=
= ViewMap
=
===================
*/

void ViewMap (void)
{
	id0_boolean_t		button0held;

	viewtype = actoratview;
	button0held = false;


	maporgx = player->tilex - VIEWTILEX/2;
	if (maporgx<0)
		maporgx = 0;
	maporgy = player->tiley - VIEWTILEY/2;
	if (maporgy<0)
		maporgy = 0;

	do
	{
//
// let user pan around
//
		IN_ReadControl(0,&c);
		if (c.xaxis == -1 && maporgx>0)
			maporgx--;
		if (c.xaxis == 1 && maporgx<mapwidth-VIEWTILEX)
			maporgx++;
		if (c.yaxis == -1 && maporgy>0)
			maporgy--;
		if (c.yaxis == 1 && maporgy<mapheight-VIEWTILEY)
			maporgy++;

		if (c.button0 && !button0held)
		{
			button0held = true;
			viewtype++;
			if (viewtype>visview)
				viewtype = mapview;
		}
		if (!c.button0)
			button0held = false;


		OverheadRefresh ();

	} while (!Keyboard[sc_Escape]);

	IN_ClearKeysDown ();
	DrawPlayScreen ();
}



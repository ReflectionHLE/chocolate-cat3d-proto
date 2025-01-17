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

#include "ID_HEADS.H"
#include <MATH.H>
#include <VALUES.H>

//#define PROFILE

/*
=============================================================================

						 GLOBAL CONSTANTS

=============================================================================
*/

#define NAMESTART	180


#define UNMARKGRCHUNK(chunk)	(grneeded[chunk]&=~ca_levelbit)

#define MOUSEINT	0x33

#define EXPWALLSTART	8
#define NUMEXPWALLS		7
#define WALLEXP			15
#define NUMFLOORS		36

#define NUMFLOORS	36

#define NUMLATCHPICS	100
#define NUMSCALEPICS	100
#define NUMSCALEWALLS	30


#define FLASHCOLOR	5
#define FLASHTICS	4


#define NUMLEVELS	20

#define VIEWX		0		// corner of view window
#define VIEWY		0
#define VIEWWIDTH	(33*8)		// size of view window
#define VIEWHEIGHT	(18*8)
#define VIEWXH		(VIEWX+VIEWWIDTH-1)
#define VIEWYH		(VIEWY+VIEWHEIGHT-1)

#define CENTERX		(VIEWX+VIEWWIDTH/2-1)	// middle of view window
#define CENTERY		(VIEWY+VIEWHEIGHT/2-1)

#define GLOBAL1		(1l<<16)
#define TILEGLOBAL  GLOBAL1
#define TILESHIFT	16l

#define MINDIST		(2*GLOBAL1/5)
#define FOCALLENGTH	(TILEGLOBAL)	// in global coordinates

#define ANGLES		360		// must be divisable by 4

#define MAPSIZE		64		// maps are 64*64 max
#define MAXACTORS	150		// max number of tanks, etc / map

#define NORTH	0
#define EAST	1
#define SOUTH	2
#define WEST	3

#define SIGN(x) ((x)>0?1:-1)
#define ABS(x) ((id0_int_t)(x)>0?(x):-(x))
#define LABS(x) ((id0_long_t)(x)>0?(x):-(x))

#define	MAXSCALE	(VIEWWIDTH/2)


#define MAXBODY			64
#define MAXSHOTPOWER	56

#define SCREEN1START	0
#define SCREEN2START	8320

#define PAGE1START		0x900
#define PAGE2START		0x2000
#define	PAGE3START		0x3700
#define	FREESTART		0x4e00

#define PIXRADIUS		512

#define STATUSLINES		(200-VIEWHEIGHT)

enum bonusnumbers {B_BOLT,B_NUKE,B_POTION,B_RKEY,B_YKEY,B_GKEY,B_BKEY,B_SCROLL1,
 B_SCROLL2,B_SCROLL3,B_SCROLL4,B_SCROLL5,B_SCROLL6,B_SCROLL7,B_SCROLL8,
 B_GOAL,B_CHEST};


/*
=============================================================================

						   GLOBAL TYPES

=============================================================================
*/

enum {BLANKCHAR=9,BOLTCHAR,NUKECHAR,POTIONCHAR,KEYCHARS,SCROLLCHARS=17,
	NUMBERCHARS=25};

typedef id0_long_t fixed;

typedef struct {id0_int_t x,y;} tilept;
typedef struct {fixed x,y;} globpt;

typedef struct
{
  id0_int_t	x1,x2,leftclip,rightclip;// first pixel of wall (may not be visable)
  id0_unsigned_t	height1,height2,color,walllength,side;
	id0_long_t	planecoord;
} walltype;

typedef enum
  {nothing,playerobj,bonusobj,orcobj,batobj,skeletonobj,trollobj,demonobj,
  mageobj,pshotobj,bigpshotobj,mshotobj,inertobj,bounceobj,grelmobj
  ,gateobj} classtype;

typedef enum {north,east,south,west,northeast,southeast,southwest,
		  northwest,nodir} dirtype;		// a catacombs 2 carryover


typedef struct	statestruct
{
	id0_int_t		shapenum;
	id0_int_t		tictime;
	void	(*think) ();
	struct	statestruct	*next;
} statetype;


typedef struct objstruct
{
  enum {no,yes}	active;
  id0_int_t		ticcount;
  classtype	obclass;
  statetype	*state;

  id0_boolean_t	shootable;
  id0_boolean_t	tileobject;		// true if entirely inside one tile

  id0_long_t		distance;
  dirtype	dir;
  fixed 	x,y;
  id0_unsigned_t	tilex,tiley;
  id0_int_t	 	viewx;
  id0_unsigned_t	viewheight;

  id0_int_t 		angle;
  id0_int_t		hitpoints;
  id0_long_t		speed;

  id0_unsigned_t	size;			// global radius for hit rect calculation
  fixed		xl,xh,yl,yh;	// hit rectangle

  id0_int_t		temp1,temp2;
  struct	objstruct	*next,*prev;
} objtype;


typedef	struct
{
	id0_int_t		difficulty;
	id0_int_t		mapon;
	id0_int_t		bolts,nukes,potions,keys[4],scrolls[8];
	id0_long_t	score;
	id0_int_t		body,shotpower;
} gametype;

typedef	enum	{ex_stillplaying,ex_died,ex_warped,ex_resetgame
	,ex_loadedgame,ex_victorious,ex_abort} exittype;


/*
=============================================================================

						 C3_MAIN DEFINITIONS

=============================================================================
*/

extern	id0_char_t		str[80],str2[20];
extern	id0_unsigned_t	tedlevelnum;
extern	id0_boolean_t		tedlevel;
extern	gametype	gamestate;
extern	exittype	playstate;


void NewGame (void);
id0_boolean_t	SaveTheGame(id0_int_t file);
id0_boolean_t	LoadTheGame(id0_int_t file);
void ResetGame(void);
void ShutdownId (void);
void InitGame (void);
void Quit (id0_char_t *error);
void TEDDeath(void);
void DemoLoop (void);
void SetupScalePic (id0_unsigned_t picnum);
void SetupScaleWall (id0_unsigned_t picnum);
void SetupScaling (void);
void main (void);

/*
=============================================================================

						 C3_GAME DEFINITIONS

=============================================================================
*/

extern	id0_unsigned_t	latchpics[NUMLATCHPICS];
extern	id0_unsigned_t	tileoffsets[NUMTILE16];
extern	id0_unsigned_t	textstarts[27];


#define	L_CHARS		0
#define L_NOSHOT	1
#define L_SHOTBAR	2
#define L_NOBODY	3
#define L_BODYBAR	4


void ScanInfoPlane (void);
void ScanText (void);
void SetupGameLevel (void);
void Victory (void);
void Died (void);
void NormalScreen (void);
void DrawPlayScreen (void);
void LoadLatchMem (void);
void FizzleFade (id0_unsigned_t source, id0_unsigned_t dest,
	id0_unsigned_t width,id0_unsigned_t height, id0_boolean_t abortable);
void FizzleOut (id0_int_t showlevel);
void FreeUpMemory (void);
void GameLoop (void);


/*
=============================================================================

						 C3_PLAY DEFINITIONS

=============================================================================
*/

extern	ControlInfo	c;
extern	id0_boolean_t		running,slowturn;

extern	id0_int_t			bordertime;

extern	id0_byte_t		tilemap[MAPSIZE][MAPSIZE];
extern	objtype		*actorat[MAPSIZE][MAPSIZE];
extern	id0_byte_t		spotvis[MAPSIZE][MAPSIZE];

extern	objtype 	objlist[MAXACTORS],*new,*obj,*player;

extern	id0_unsigned_t	farmapylookup[MAPSIZE];
extern	id0_byte_t		*nearmapylookup[MAPSIZE];
extern	id0_byte_t		update[];

extern	id0_boolean_t		godmode,singlestep;
extern	id0_int_t			extravbls;

extern	id0_int_t			mousexmove,mouseymove;
extern	id0_int_t			pointcount,pointsleft;


void CenterWindow(id0_word_t w,id0_word_t h);
void DebugMemory (void);
void PicturePause (void);
id0_int_t  DebugKeys (void);
void CheckKeys (void);
void InitObjList (void);
void GetNewObj (id0_boolean_t usedummy);
void RemoveObj (objtype *gone);
void PollControlls (void);
void PlayLoop (void);


/*
=============================================================================

						 C3_STATE DEFINITIONS

=============================================================================
*/

void SpawnNewObj (id0_unsigned_t x, id0_unsigned_t y, statetype *state, id0_unsigned_t size);
void SpawnNewObjFrac (id0_long_t x, id0_long_t y, statetype *state, id0_unsigned_t size);
id0_boolean_t CheckHandAttack (objtype *ob);
void T_DoDamage (objtype *ob);
id0_boolean_t Walk (objtype *ob);
void ChaseThink (objtype *obj, id0_boolean_t diagonal);
void MoveObj (objtype *ob, id0_long_t move);
id0_boolean_t Chase (objtype *ob, id0_boolean_t diagonal);

extern	dirtype opposite[9];

/*
=============================================================================

						 C3_TRACE DEFINITIONS

=============================================================================
*/

id0_int_t FollowTrace (fixed tracex, fixed tracey, id0_long_t deltax, id0_long_t deltay, id0_int_t max);
id0_int_t BackTrace (id0_int_t finish);
void ForwardTrace (void);
id0_int_t FinishWall (void);
void InsideCorner (void);
void OutsideCorner (void);
void FollowWalls (void);

extern	id0_boolean_t	aborttrace;

/*
=============================================================================

						 C3_DRAW DEFINITIONS

=============================================================================
*/

#define MAXWALLS	50
#define DANGERHIGH	45

#define	MIDWALL		(MAXWALLS/2)

//==========================================================================

extern	tilept	tile,lasttile,focal,left,mid,right;

extern	globpt	edge,view;

extern	id0_unsigned_t screenloc[3];
extern	id0_unsigned_t freelatch;

extern	id0_int_t screenpage;

extern	id0_boolean_t		fizzlein;

extern	id0_long_t lasttimecount;

extern	id0_int_t firstangle,lastangle;

extern	fixed prestep;

extern	id0_int_t traceclip,tracetop;

extern	fixed sintable[ANGLES+ANGLES/4],*costable;

extern	fixed	viewx,viewy,viewsin,viewcos;			// the focal point
extern	id0_int_t	viewangle;

extern	fixed scale,scaleglobal;
extern	id0_unsigned_t slideofs;

extern	id0_int_t zbuffer[VIEWXH+1];

extern	walltype	walls[MAXWALLS],*leftwall,*rightwall;


extern	fixed	tileglobal;
extern	fixed	focallength;
extern	fixed	mindist;
extern	id0_int_t		viewheight;
extern	fixed scale;

extern	id0_int_t	walllight1[NUMFLOORS];
extern	id0_int_t	walldark1[NUMFLOORS];
extern	id0_int_t	walllight2[NUMFLOORS];
extern	id0_int_t	walldark2[NUMFLOORS];

//==========================================================================

void	DrawLine (id0_int_t xl, id0_int_t xh, id0_int_t y,id0_int_t color);
void	DrawWall (walltype *wallptr);
void	TraceRay (id0_unsigned_t angle);
fixed	FixedByFrac (fixed a, fixed b);
void	TransformPoint (fixed gx, fixed gy, id0_int_t *screenx, id0_unsigned_t *screenheight);
fixed	TransformX (fixed gx, fixed gy);
id0_int_t	FollowTrace (fixed tracex, fixed tracey, id0_long_t deltax, id0_long_t deltay, id0_int_t max);
void	ForwardTrace (void);
id0_int_t	FinishWall (void);
id0_int_t	TurnClockwise (void);
id0_int_t	TurnCounterClockwise (void);
void	FollowWall (void);

void	NewScene (void);
void	BuildTables (void);


/*
=============================================================================

						 C3_SCALE DEFINITIONS

=============================================================================
*/


#define COMPSCALECODESTART	(65*6)		// offset to start of code in comp scaler

typedef struct
{
	id0_unsigned_t	codeofs[65];
	id0_unsigned_t	start[65];
	id0_unsigned_t	width[65];
	id0_byte_t		code[];
}	t_compscale;

typedef struct
{
	id0_unsigned_t	width;
	id0_unsigned_t	codeofs[64];
}	t_compshape;


extern id0_unsigned_t	scaleblockwidth,
		scaleblockheight,
		scaleblockdest;

extern	id0_byte_t	plotpix[8];
extern	id0_byte_t	bitmasks1[8][8];
extern	id0_byte_t	bitmasks2[8][8];


extern	t_compscale id0_seg *scaledirectory[MAXSCALE+1];
extern	t_compshape id0_seg *shapedirectory[NUMSCALEPICS];
extern	memptr			walldirectory[NUMSCALEWALLS];
extern	id0_unsigned_t	shapesize[MAXSCALE+1];

void 		DeplanePic (id0_int_t picnum);
void ScaleShape (id0_int_t xcenter, t_compshape id0_seg *compshape, id0_unsigned_t scale);
id0_unsigned_t	BuildCompShape (t_compshape id0_seg **finalspot);


/*
=============================================================================

						 C3_ASM DEFINITIONS

=============================================================================
*/

extern	id0_unsigned_t	wallheight	[VIEWWIDTH];
extern	id0_unsigned_t	wallwidth	[VIEWWIDTH];
extern	id0_unsigned_t	wallseg		[VIEWWIDTH];
extern	id0_unsigned_t	wallofs		[VIEWWIDTH];
extern	id0_unsigned_t	screenbyte	[VIEWWIDTH];
extern	id0_unsigned_t	screenbit	[VIEWWIDTH];
extern	id0_unsigned_t	bitmasks	[64];

extern	id0_long_t		wallscalecall;

void	ScaleWalls (void);

/*
=============================================================================

						 C3_WIZ DEFINITIONS

=============================================================================
*/

#define MAXHANDHEIGHT	72

extern	id0_long_t	lastnuke;
extern	id0_int_t		handheight;
extern	id0_int_t		boltsleft;

/*
=============================================================================

						 C3_ACT1 DEFINITIONS

=============================================================================
*/

extern	statetype s_trollouch;
extern	statetype s_trolldie1;


extern	statetype s_orcpause;

extern	statetype s_orc1;
extern	statetype s_orc2;
extern	statetype s_orc3;
extern	statetype s_orc4;

extern	statetype s_orcattack1;
extern	statetype s_orcattack2;
extern	statetype s_orcattack3;

extern	statetype s_orcouch;

extern	statetype s_orcdie1;
extern	statetype s_orcdie2;
extern	statetype s_orcdie3;


extern	statetype s_demonouch;
extern	statetype s_demondie1;

extern	statetype s_mageouch;
extern	statetype s_magedie1;

extern	statetype s_grelouch;
extern	statetype s_greldie1;

extern	statetype s_batdie1;

/*		cake++ - a checkers engine
 *
 *		Copyright (C) 2001 by Martin Fierz
 *
 *		contact: checkers@fierz.ch
 */
/* structs.h: data structures for cake++ */

/*definitions for platform-independence*/
#define int32 unsigned int
#define int16 unsigned short
#define int8  unsigned char
#define sint32 signed int
#define sint16 signed short
#define sint8  signed char


struct coor             /* coordinate structure for board coordinates */
	{
	int x;
	int y;
	};

struct CBmove				/* all the information you need about a move */
	{
	int ismove; 		 /* kind of superfluous: is 0 if the move is not a valid move */
	int newpiece;		/* what type of piece appears on to */
	int oldpiece;		/* what disappears on from */
	struct coor from,to; /* coordinates of the piece - in 8x8 notation!*/
	struct coor path[12]; /* intermediate path coordinates of the moving piece */
	struct coor del[12]; /* squares whose pieces are deleted after the move */
	int delpiece[12];	/* what is on these squares */
	};


struct pos
	{
	int32 bm;
	int32 bk;
	int32 wm;
	int32 wk;
	};

struct move
	{
	int32 bm;
	int32 bk;
	int32 wm;
	int32 wk;
	};
/* old
struct hashentry
	// struct hashentry needs 8 bytes 
	{
	int32  lock;
	unsigned int best:6;
	int value:12;
	unsigned int color:2;
	unsigned int depth:10;
	unsigned int valuetype:2;
	};
*/

struct hashentry
	// struct hashentry needs 8 bytes 
	{
	int32  lock;
	unsigned int best:6;
	int value:12;
	unsigned int color:1;
	unsigned int ispvnode:1;
	unsigned int depth:10;
	unsigned int valuetype:2;
	};


struct bookhashentry
	// bookhashentry is the struct used for the book hashtable
	// we can store 3 moves per position.
	{
	int32  lock;

	unsigned int color:2;
	unsigned int best:6;
	int value:14;
	unsigned int depth:10;
	
	/*unsigned int best2:6; 
	unsigned int value2:4;			// value2 is the difference in value to the best value.
	unsigned int depth2:6;			// can be up to 15, and if it's worse than that, we don't have a move.
	
	unsigned int best3:6;
	unsigned int value3:4;
	unsigned int depth3:6;*/
	};

struct dbhashentry
	{
	unsigned int lock: 28;
	unsigned int color: 2;
	unsigned int result: 2;
	};

struct hashtable
	{
	struct hashentry *pointer;
	int32 size;
	int bucketsize;
	};
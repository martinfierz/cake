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

struct CBmove            	/* all the information you need about a move */
	{
	int ismove;          /* kind of superfluous: is 0 if the move is not a valid move */
   int newpiece;        /* what type of piece appears on to */
   int oldpiece;        /* what disappears on from */
	struct coor from,to; /* coordinates of the piece - in 8x8 notation!*/
   struct coor path[12]; /* intermediate path coordinates of the moving piece */
	struct coor del[12]; /* squares whose pieces are deleted after the move */
   int delpiece[12];    /* what is on these squares */
	} GCBmove;

struct move
	{
	int32 bm;
	int32 bk;
	int32 wm;
	int32 wk;
	sint32 info;
	//int32 n;
	};

struct pos
	{
	int32 bm;
	int32 bk;
	int32 wm;
	int32 wk;
	};

struct hashentry
	/* struct hashentry needs 12 bytes - thats a bit inefficient!*/
	{
	int32  lock;
	int32  best;
	int value:16;
	unsigned int color:2;
	unsigned int depth:12;
	unsigned int valuetype:2;
	};
// dblookup.h
// all you need.

// include 
#include "switches.h"

// choose which db you want to use

#define PRELOAD // preload (parts) of db in cache? 

#define AUTOLOADSIZE 0

#define DB_BLACK 0
#define DB_WHITE 1


typedef __int64 int64;

typedef struct 
	{
	unsigned int bm;
	unsigned int bk;
	unsigned int wm;
	unsigned int wk;
	} position;

#define DB_UNKNOWN 0
#define DB_WIN 1
#define DB_LOSS 2
#define DB_DRAW 3
#define DB_NOT_LOOKED_UP 4

// either SIX or EIGHT has to be defined in switches.h
// the db code will use either up to 6 or up to 8-piece dbs respectively.
#ifdef EIGHT
	// 8-piece db
	#define SPLITSIZE 8
	#define BLOCKNUM 4600000		// enough for 8-piece db.
	#define CACHESIZE 786432 // 835328 //393216//786432// 1572864// 196608//786432//393216//196608 //786432		//786432 // 393216 //750000 // this is the amount of 1K blocks it gets 0.74GB
	#define MAXIDX 44000			// maximal number of indexes in any index file (42313 should work too)
	#define MAXPIECES 8
	#define MAXPIECE 4
#endif

#ifdef SIX
	// 6-piece db
	#define SPLITSIZE 7
	#define MAXPIECES 6
	#define BLOCKNUM 42000		// enough for 6-piece db.
	#define CACHESIZE 42000		//42000 // this is the amount of 1K blocks it gets
	#define MAXIDX 1000			// maximal number of indexes in any index file
	#define MAXPIECE 4
#endif


int initdblookup(char str[256]);
int dblookup(position *p, int color,int cl);


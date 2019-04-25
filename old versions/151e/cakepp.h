/*		cake++ 1.22 - a checkers engine			*/
/*															*/
/*		Copyright (C) 2001 by Martin Fierz		*/
/*															*/
/*		contact: checkers@fierz.ch					*/

/* cake++.h */
#include "switches.h"
/* prototypes for all functions in cake++ */
int initcake(int logging, char str[1024]);
int exitcake(void);
int cake_getmove(struct pos *position,int color, int how,double maxtime, int depthtosearch,int32 maxnodes, char str[1024], int *playnow, int logging,int reset);
/* returns the value of the position */
static void countmaterial(struct pos *p);
static void initboard(void);
static int firstnegamax(int d, int color, int alpha, int beta, struct move *best);
static int negamax(struct pos *p,int depth, int color, int alpha, int beta, int32 *bestproto, int truncationdepth, int truncationdepth2);
int evaluation(struct pos *p,int color, int alpha, int beta,int *noprune);
static int mtdf(int firstguess,int depth, int color, struct move *best);
static int windowsearch(int depth, int color, int guess, struct move *best);

static int fineevaluation(struct pos *p,int color, int *noprune);

static int bitcount(int32 n);
int recbitcount(int32 n);
int LSB(int32 x);
static void absolutehashkey(struct pos *p);
void updatehashkey(struct move *m);
void hashstore(struct pos *p, int value, int alpha, int beta, int depth, struct move *best, int color, int32 bestindex);
int hashlookup(int *value, int *valuetype, int depth, int32 *bestindex, int color, int *ispvnode);
int hashreallocate(int MB);
static int booklookup(int *value, int depth, int32 *best, int color, int *bestindex);
static void movetonotation(struct pos position,struct move m, char *str, int color);
static void getpv(char *str, int color);
int testcapture(struct pos *p,int color);
int isforced(struct pos *p,int color);
#ifdef USEDB
int dbwineval(struct pos *p,int color);
int dblosseval(struct pos *p,int color);
#endif
void printboardtofile(struct pos p);
void printboard(struct pos p);
#ifdef OPLIB
static void boardtobitboard(int b[8][8], struct pos *position);
#endif
static int32 myrand(void);
#ifdef ETC
int ETClookup(struct move movelist[MAXMOVES], int d, int n,int color);
#endif

#ifdef BOOKGEN
static booknegamax(struct pos *position, int color, int depth, int alpha, int beta, int values[MAXMOVES]);
bookgen(struct pos *p, int color, int *n, int values[MAXMOVES], int how, int depth, double time);
#endif

static int32 attack_forwardjump(int32 x, int32 free);
static int32 attack_backwardjump(int32 x, int32 free);
static int32 forwardjump(int32 x, int32 free,int32 other);
static int32 backwardjump(int32 x, int32 free,int32 other);

// pattern matching macros
#define match1(a,b) ( ((a)&(b)) == (b) )
#define match2(a,b,c,d) ( (((a)&(c))|((b)&(d))) ==((c)|(d)))
#define match3(a,b,c,d,e,f) ( (((a)&(d))|((b)&(e))|((c)&(f))) ==((d)|(e)|(f)))

/* cake++.h */

/* prototypes for all functions in cake++ */
int initcake(int logging);
int exitcake(void);
int cake_getmove(struct pos *position,int color, int how,double maxtime, int depthtosearch,int32 maxnodes, char str[255], int *playnow, int logging,int reset);
/* returns the value of the position */
void countmaterial(void);
void initboard(void);
int firstnegamax(int d, int color, int alpha, int beta, struct move *best);
int negamax(int depth, int color, int alpha, int beta, int32 *bestproto, int truncationdepth);
int evaluation(int color, int alpha, int beta);
int mtdf(int firstguess,int depth, int color, struct move *best);
int windowsearch(int depth, int color, int guess, struct move *best);

int fineevaluation(int color);

int bitcount(int32 n);
int recbitcount(int32 n);
int lastbit(int32 x);
void absolutehashkey(void);
void updatehashkey(struct move m);
void hashstore(int value, int alpha, int beta, int depth, struct move best, int color);
int hashlookup(int *value, int *valuetype, int depth, int32 *best, int color);
void movetonotation(struct pos position,struct move m, char *str, int color);
void getpv(char *str, int color);
int testcapture(int color);
#ifdef USEDB
int dbwineval(int color);
int dblosseval(int color);
#endif
void printboardtofile(struct pos p);
void printboard(struct pos p);
#ifdef OPLIB
void boardtobitboard(int b[8][8], struct pos *position);
#endif
int32 myrand(void);


long DBLookup(struct pos p, int turn);


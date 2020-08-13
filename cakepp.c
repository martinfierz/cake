/*
 *		cake - a checkers engine			
 *															
 *		Copyright (C) 2000...2011 by Martin Fierz	
 *														
 *		contact: checkers@fierz.ch		
 *		TODO: check which late moves lead to cutoffs to try and determine whether we can selectively improve
 *		TODO: cakepp.c is 2600 lines of code, much too much
 */



#include "switches.h"
#include <stdio.h>
#include <stdlib.h> // malloc() 
#include <string.h> // memset() 
#include <time.h>
#include <assert.h>
#include <conio.h>
#include <windows.h>
//#include <pgobootrun.h>

// cake-specific includes - structs defines structures, consts defines constants,
// xxx.h defines function prototypes for xxx.c
#include "structs.h"
#include "consts.h"
#include "cakepp.h"
#include "move_gen.h"
#include "dblookup.h"
#include "initcake.h"
#include "cake_misc.h"
#include "cake_eval.h"
#include "boolean.h"
#include "cake_book.h"
#include "app_instance.h"

// define the two following for running the optimizer, undef them for normal operation
#undef NOHASHRESET
#undef NOLOGFILE


//-------------------------------------------------------------------------------//
// globals below here are shared - even with these, cake *should* be thread-safe //
//-------------------------------------------------------------------------------//
int hashmegabytes = 512;				// default hashtable size in MB if no value in registry is found
int dbmegabytes = 1536;				// default db cache size in MB if no value in registry is found
int usethebook = 3;					// default: use best moves if no value in the registry is found
int hashsize = HASHSIZE;
int maxNdb=0;						// the largest number of stones which is still in the database 
int bookentries = 0; // number of entries in book hashtable
int bookmovenum = 0; // number of used entries in book

static HASHENTRY *hashtable;		// pointer to the hashtable, is allocated at startup
static HASHENTRY *book;				// pointer to the book hashtable, is allocated at startup
static int iscapture[MAXDEPTH];		// tells whether move at realdepth was a capture
#ifdef PROMOTEEXTEND
static int ispromote[MAXDEPTH];		// tells whether move at realdepth was a promotion
#endif
#ifdef SPA
static SPA_ENTRY *spatable;
#endif
static int cakeisinit = 0;			// is set to 1 after cake is initialized, i.e. initcake has been called
static int32 hashxors[2][4][32];			// this is initialized to constant hashxors stored in the code.
static int  norefresh;

//FILE* cake_main_fp; 

//int32 killer1[MAXDEPTH+10], killer2[MAXDEPTH+10];
#ifdef GLOBALMOVESTACK
MOVE movestack[(MAXDEPTH + 10) * MAXMOVES];
#endif

//int ifs[100]; 

#ifdef THREADSAFEHT
CRITICAL_SECTION hash_access;		// is locked if a thread is accessing the hashtable
#endif

/*----------------------------------------------------------------------------*/
/*																			  */
/*                                initialization                              */
/*																			  */
/*----------------------------------------------------------------------------*/

int initcake(char str[1024])
	//  initcake must be called before any calls to cake_getmove can be made.
	//	it initializes the database and some constants, like the arrays for lastone
	//	and bitsinword, plus some evaluation tables 
	//	str is the buffer it writes output to
	{
	char dirname[512];
	//TCHAR tdirname[512]; 
	int returnvalue;
	char s[256];
	FILE* fp; 
	int g_app_instance = 0;					/* 0, 1, 2, ... */
	char g_app_instance_suffix[16] =""; 

	// first of all, set application instance to allow different logfiles for different instances
	/* To support running multiple instances of CB, use suffixes in logfilenames. */
	get_app_instance("Cake", &g_app_instance);
	if (g_app_instance > 0)
		sprintf(g_app_instance_suffix, "[%d]", g_app_instance);
	setsuffix(g_app_instance_suffix); 



	returnvalue = GetCurrentDirectory(512, dirname);
	//returnvalue = GetCurrentDirectory(512, tdirname); 
	
	//clearlogfile();
	fp = getlogfile(1);
	
	printf("initcake with dir %s", dirname); 

	logtofile(fp, "working directory is:"); 
	logtofile(fp, dirname);
	sprintf(s,"characters returned: %i \n\n",returnvalue);
	logtofile(fp, s);
	SetCurrentDirectory(dirname);

	
	// TODO: here working dir is still ok
	// do some loads. first, the endgame database.

#ifdef USEDB
	sprintf(str, "initializing database");
	maxNdb = db_init(dbmegabytes,str, fp);
	//printf("\nfound %i-piece database in %s", maxNdb, dirname); 
#endif // USEDB

#ifdef BOOK
	// next, load the book 
	SetCurrentDirectory(dirname);
	sprintf(str,"loading book...");
	//printf("loading book...");
	book = loadbook(&bookentries, &bookmovenum, fp);
#endif

	// allocate hashtable
	sprintf(str,"allocating hashtable (%i MB)", hashsize/(1024*1024));
	printf("\n%s", str); 
	hashtable = inithashtable(hashsize, fp);
	
	// initialize xors 
	initxors((int*)hashxors);
	
	initeval();
#ifdef SPA
	// allocate memory for SPA
	initspa();
	// load spa positions
	loadspa(); 
#endif

#ifdef BOOKHT
	initbookht();
#endif

#ifdef THREADSAFEHT
	InitializeCriticalSection(&hash_access);
#endif

	cakeisinit=1;
	logtofile(fp, "\ninitcake done!");
	if(fp != NULL)
		fclose(fp); 

	// create Cake folder in personal documents
	//createCakeFolder(); 

	/*for (int i = 0; i < 100; i++)
		ifs[i] = 0;*/

	return 1;
	}

int hashreallocate(int newMB)  // call with hashmegabytes
	{
	// TODO: move to little-used functions file
	// reallocates the hashtable to x MB
	
	static int currenthashsize;
	char Lstr[256];

	HASHENTRY *pointer = NULL;
	int newsize;

	hashmegabytes = newMB;
	newsize = ((long)hashmegabytes)*(long)1048576/sizeof(HASHENTRY);

	sprintf(Lstr,"reallocating hashtable to %i MB", newMB);
	//logtofile(Lstr);

	// TODO: this must be coerced to a power of 2. luckily this is true
	// with sizeof(HASHENTRY) == 8 bytes, but if i ever change that it will
	// fail

#ifdef WINMEM
	VirtualFree(hashtable, 0, MEM_RELEASE);
	pointer = VirtualAlloc(0, (newsize+HASHITER)*sizeof(HASHENTRY), MEM_COMMIT, PAGE_READWRITE);
#else
	_aligned_free(hashtable);
	

	while(pointer == NULL) {
		printf("\ntrying to allocate %i bytes", newsize); 
		pointer = (HASHENTRY*)malloc(((long)newsize + (long)HASHITER) * sizeof(HASHENTRY) + 64L);
		//pointer = (HASHENTRY*)_aligned_malloc((newsize + (int)HASHITER) * sizeof(HASHENTRY) + 64, 64);
		
		// todo: use _aligned_malloc() instead to align it on 64-byte boundary
		newsize = newsize/2;
	}
	hashsize = newsize *2;
	hashtable = pointer;
	//pointer = (HASHENTRY*) realloc(hashtable,(newsize+HASHITER)*sizeof(HASHENTRY));
#endif
	/*if(pointer != NULL)
		{
		hashsize = newsize;
		hashtable = pointer;
		}*/
	// TODO: do something if pointer == NULLbut what do we do when pointer == 0 here??
	if(pointer == NULL) {
		//logtofile("hash reallocation failed");
		exit(0);
	}
	return 1;
	}

#ifdef BOOKGEN
int bookgen(OLDPOSITION *q, int color, int *numberofmoves, int values[MAXMOVES], int how, int depth, double searchtime)
	{
	// TODO: this function uses too much space from stack, should put data on heap (malloc MOVE mlarray...)
	// this function analyzes a position, and returns the number of possible moves (*numberofmoves) and
	// an array with the evaluation of each move. to speed up things, it only looks at "sensible" values,
	// i.e. if a move is 120 points worse than the best move, it stops there and doesn't determine exactly
	// how bad it is. 
	// bookgen is only used by the book generator, not in testcake or checkerboard.
	int playnow=0;
	int d;
	int value=0,lastvalue=0,n,i,j, guess;
	MOVE best = { .bm = 0,.bk = 0,.wm = 0,.wk = 0 };
	MOVE last, movelist[MAXMOVES];
	MOVE mlarray[MAXMOVES][MAXMOVES];
	REPETITION dummy;
	char Lstr[1024],str[1024];
	int bookfound=1,bookbestvalue=-MATE;
	int bookequals=0;
	int bookdepth=0;
	int bookindex=-1;
	int reset=1;
	int forcedmove = 0; // indicates if a move is forced, then cake++ will play at depth 1.
	char pvstring[1024];
	char valuestring[256];
	char beststring[1024];
	int issearched[MAXMOVES];
	int bestvalue;
	int index = 0;
	int bestnewvalue;
	double t;
	int zeroevals = 0;
	// since this is an alternate entry point to cake.dll, we need to have a searchinfo structure ready here
	SEARCHINFO si;

	POSITION p;
	
	// initialize module if necessary
	if(!cakeisinit) 
		initcake(str);
	
	// reset all counters, nodes, database lookups etc 
	resetsearchinfo(&si);

	// allocate memory for repcheck array
	si.repcheck = malloc((MAXDEPTH+HISTORYOFFSET) * sizeof(REPETITION));

	
	si.play = &playnow;
	si.out = str;
	si.aborttime = 40000*searchtime;
	si.maxtime = searchtime;
	si.searchmode = how;

	// change from old position struct to new one, because oplib is using the old one.
	p.bk = q->bk;
	p.bm = q->bm;
	p.wk = q->wk;
	p.wm = q->wm;
	p.color = color;

	// initialize material 
	countmaterial(&p, &(si.matcount));
	// initialize hash key 
	absolutehashkey(&p, &(si.hash));

#ifdef MOHISTORY
	// reset history table					
	memset(si.history,0,32*32*sizeof(int));
#endif

	printboard(&p);

	// clear the hashtable TODO doesnt look threadsafe!
	// memset(hashtable,0,(hashsize+HASHITER)*sizeof(HASHENTRY)); // TODO: check if this is ok
	
	norefresh=0;

	n = makecapturelist(&p, movelist, values, 0);
	if(!n)
		n = makemovelist(&si, &p, movelist, values, 0, 0);
	*numberofmoves = n;
#ifdef REPCHECK
	// initialize history list: holds the last few positions of the current game 
	si.repcheck[HISTORYOFFSET].hash = si.hash.key;
	si.repcheck[HISTORYOFFSET].irreversible = 1;
	
	dummy.hash = 0;
	dummy.irreversible = 1;

	for(i=0; i<HISTORYOFFSET; i++)  // was i<historyoffset + 8 - doesn't make much sense to me now
		si.repcheck[i] = dummy;
#endif
	
	si.start = clock();
	guess = 0;

	// initialize mlarray:
	for(i = 0; i<n; i++)
		{
		togglemove((&p), movelist[i]);
		getorderedmovelist(&p, &mlarray[i][0]);
		togglemove((&p), movelist[i]);
		}

	for(d=1; d<MAXDEPTH; d+=2)
		{
		si.leaf = 0;
		si.leafdepth = 0;
		value = -MATE;
		printf("\n");

		for(i=0;i<n;i++)
			issearched[i] = 0;

		bestnewvalue = -MATE;
		for(i=0;i<n;i++)
			{
			// get move with next best value to search, search moves from best to worst
			bestvalue = -MATE;
			for(j=0;j<n;j++)
				{
				if(issearched[j])
					continue;
				if(values[j]>bestvalue)
					{
					index = j;
					bestvalue=values[j];
					}
				}

			issearched[index]=1;

			togglemove((&p),movelist[index]);
			absolutehashkey((&p), &(si.hash));
			countmaterial((&p), &(si.matcount));

#ifndef MTD
			values[index]=-windowsearch(p,FRAC*(d-1), 0 /*guess*/, &best);
			// set value for next search 
			//guess=value[i];	
#endif
#ifdef MTD
			// MTD(F) search 
			values[index] = -bookmtdf(&si, &p, &mlarray[index][0], -values[index],FRAC*(d-1),&best,-bestnewvalue);
			bestnewvalue = max(bestnewvalue,values[index]);
#endif
			value = max(value, values[index]);
			togglemove((&p),movelist[index]);
			movetonotation(&p,&movelist[index],Lstr);
			printf("[%s %i] ",Lstr,values[index]);
			if(value == values[index])
				sprintf(beststring,"%s",Lstr);
			}

		// zeroevals: count how many times we have seen 0, and stop the analysis early if we see too many.
		if(bestnewvalue == 0)
			zeroevals++;
		else
			zeroevals = 0;

		// generate output 
		sprintf(Lstr,"%s",beststring);
		getpv(&si, &p, pvstring);
		
		// check for clock tick overflow
		// this should not happen - reset the start time
		t = clock();
		if(t-si.start < 0)
			si.start = clock(); 
		
		sprintf(valuestring, "value=%i", value);
		searchinfotostring(str, d*FRAC, (t-si.start)/CLK_TCK, valuestring, pvstring, &si);
		printf("\n%s",str);	
		// iterative deepening loop break conditions: depend on search mode, 'how':
		//	how=0: time mode
		if(d>1)
			{
			if( si.searchmode == TIME_BASED && ((clock() - si.start)/CLK_TCK>(si.maxtime/2)) ) 
				break;
			
			// early break in case of many zeros and enough search depth
			if( si.searchmode == TIME_BASED && ((clock()- si.start)/CLK_TCK>(si.maxtime/8)) && (zeroevals>4) && (d>22)) 
				{
				printf("\n* early break");
				break;
				}
			
			if( si.searchmode == DEPTH_BASED && d>=depth)
				break;
			
			if(abs(value)>MATE-100) break;
			}

		lastvalue=value;	// save the value for this iteration 
		last=best;			// save the best move on this iteration 
		norefresh=1;
		}
	
	free(si.repcheck);

	return value;
}

int hashclear() {
	 memset(hashtable,0,(hashsize+HASHITER)*sizeof(HASHENTRY)); // TODO: check if this is ok
	 return 1; 
}

int bookmtdf(SEARCHINFO *si, POSITION *p, MOVE movelist[MAXMOVES], int firstguess,int depth, MOVE *best, int bestvalue)
	{
	int g,lowerbound, upperbound,beta;
	double time;
	char Lstr1[1024],Lstr2[1024];

#define CUTOFF 120
	
	upperbound=MATE;
	lowerbound=-MATE;
	
	g=firstguess;
	g=0;			// a bit weird, firstguess should be better, but 0 seems to help

	while(lowerbound<upperbound)
		{
		if(g==lowerbound)
			beta=g+1;
		else beta=g;
		g = firstnegamax(si, p, movelist, depth, beta-1, beta, best);
		
		if(g<beta)
			{
			upperbound=g;
			sprintf(Lstr1,"value<%i",beta);
			}
		else
			{
			lowerbound=g;
			sprintf(Lstr1,"value>%i",beta-1);
			// termination if this node is much worse than the best so far:
			if(g - CUTOFF > bestvalue)
				return g;
			}
		
		time = ( (clock()- si->start)/CLK_TCK);
		getpv(si, p, Lstr2);

		sprintf(si->out,"depth %i/%i/%.1f  time %.2fs  %s  nodes %i  %ikN/s  %s", 
			(depth/FRAC),si->maxdepth,(float)si->leafdepth/((float)si->leaf+0.001) , time, Lstr1, 
			si->negamax, (int)((float)si->negamax/time/1000),Lstr2); 
#ifdef FULLLOG
		//logtofile(si->out);
#endif		
		}

	
	return g;
	}

#endif //bookgen

int analyze(POSITION *p, int color, int how, int depth, double searchtime, int *stat_eval)
	{
	// is a direct-access function for external analysis
	int playnow=0;
	int d;
	int value=0,lastvalue=0, guess;
	int dummy;
	MOVE best,last;
	char Lstr[1024],str[1024];
	int staticeval;
	// si for the analyze function so that it can reset HT after X runs
	static int analyzedpositions; 
	double t;
	// analyze is an entry point to cake, so we need a searchinfo here
	SEARCHINFO si;
	MOVE movelist[MAXMOVES];

	d = getorderedmovelist(p, movelist);

	resetsearchinfo(&si);

	// allocate memory for repcheck array
	si.repcheck = malloc((MAXDEPTH+HISTORYOFFSET) * sizeof(REPETITION));



	si.play = &playnow;
	si.out = str;
	analyzedpositions++;
	
	// set time limits
	si.aborttime = 40000*searchtime;
	si.maxtime = searchtime;
	si.searchmode = how;

	// initialize module if necessary
	if(!cakeisinit) 
		initcake(str);


#ifdef MOHISTORY
	// reset history table 
	memset(si.history,0,32*32*sizeof(int));
#endif


	// clear the hashtable 
	// no - not in this mode
	// or more precisely, nearly never. reason: after a long time, the hashtable is
	// filled with positions which have a ispvnode attribute.
	
	//if((analyzedpositions % 100000) == 0)
	//	{
		printf("\nresetting hashtable");
		memset(hashtable,0,(hashsize+HASHITER)*sizeof(HASHENTRY));
	//	}
	
	norefresh=0;
	
	// initialize hash key 
	absolutehashkey(p, &(si.hash));
	countmaterial(p, &(si.matcount));

	si.start = clock();
	guess=0;

	staticeval = evaluation(p,&(si.matcount),0,&dummy,0,maxNdb);

	// set return value of static eval.
	*stat_eval = staticeval;
	//printf("\nstatic eval %i",staticeval);

	for(d=1;d<MAXDEPTH;d+=2)
		{
		si.leaf  = 0;
		si.leafdepth = 0;
		// choose which search algorithm to use: 
		// non - windowed search 
		// value = firstnegamax(FRAC*d, -MATE, MATE, &best);
#ifndef MTD
		// windowed search
		value = windowsearch(&si, p, movelist, FRAC*d, guess, &best);
		// set value for next search 
		guess=value;	
#endif

#ifdef MTD
		// MTD(F) search 
		value = mtdf(&si, p, movelist, guess, FRAC*d, &best);
		guess = value;	
#endif
	
		// generate output 
		movetonotation(p,&best,Lstr);
		t = clock();
		
		// iterative deepening loop break conditions: depend on search mode, 'how':
		//	how=0: time mode;
		if(d>1)
			{
			if( si.searchmode == TIME_BASED && ((clock()- si.start)/CLK_TCK>(si.maxtime/2)) ) 
				break;
			if( si.searchmode == DEPTH_BASED && d>=depth)
				break;
			if(abs(value)>MATE-100) break;
			}

		if( *(si.play))
			{
			// the search has been aborted, either by the user or by cake++ because 
			// abort time limit was exceeded
			// stop the search. don't use the best move & value because they might be rubbish 
			best=last;
			movetonotation(p,&best,Lstr);
			value=lastvalue;
			sprintf(str,"interrupt: best %s value %i",Lstr,value);
			break;
			}

		lastvalue=value;	// save the value for this iteration 
		last=best;			// save the best move on this iteration 
		norefresh=1;
		}

	free(si.repcheck);
	return value;
	}


int cake_getmove(SEARCHINFO *si, POSITION *p, int how,double maximaltime,
					  int depthtosearch, int32 maxnodes,
					  char str[1024], int *playnow, int log, int info)
	{

	/*----------------------------------------------------------------------------*/
	/*
	/*		cake_getmove is the entry point to cake++
	/*		give a pointer to a position and you get the new position in
	/*		this structure after cake++ has calculated.
	/*		color is BLACK or WHITE and is the side to move.
	/*		how is TIME_BASED for time-based search and DEPTH_BASED for depth-based search and 
	/*		NODE_BASED for node-based search
	/*		maximaltime and depthtosearch and maxnodes are used for these three search modes.
	/*		cake++ prints information in str
	/*		if playnow is set to a value != 0 cake++ aborts the search.
	/*		if (log&1) cake will write information into "cakelog.txt"
	/*		if(log&2) cake will also print the information to stdout.
	/*		if reset!=0 cake++ will reset hashtables and repetition checklist
	/*		reset==0 generally means that the normal course of the game was disturbed
	// info currently has uses for its first 4 bits:
	// info & 1  means reset repcheck array
	// info & 2  means exact time level
	// info & 4  means increment time level
	// info & 8  means allscore search
	// info & 16 means increase aborttime to 10x search time (why??)
	// info & 32 means reset hash table


	/*
	/*----------------------------------------------------------------------------*/


	// ed's comments in his code:

	/* If this board has more pieces than the previous board,
	 * or this board represents a conversion move from the previous board,
	 * or this board has the same color to move as the previous board,
	 * then reset the list of game moves.
	 */

	 /* If the position is identical to the second most recent, then he must be researching the same
	  * position as the last time in Analysis mode. Delete the last position and return.
	  */

	  /* In autoplay the position to search will be identical to last one. */

	int d;
	int value=0,lastvalue=0,n,i, guess;
	MOVE best,last;
	REPETITION dummy;
	char Lstr[1024];
	int bookfound=1,bookbestvalue=-MATE;
	MOVE bookmove;
	int bookequals=0;
	int bookdepth=0;
	int bookindex=-1,booklookupvalue;
	int values[MAXMOVES];
	int reset = (info & 1);
	int forcedmove = 0; // indicates if a move is forced, then cake++ will play at depth 1.
	int zeroes = 0;		// number of zero evals returned
	int winslosses = 0;
	static int age = 0; 
	double t;
	char pvstring[1024];
	MOVE movelist[MAXMOVES];	// we make a movelist here which we pass on, so that it can be ordered
								// and remain so during iterative deeping.

	FILE *fp = 0; 

	static POSITION lastsearchpos; 
	static int newgamestarts = 0; 
	int32 difference = 0; 
	int32 identical = 0; 
	int irreversible = 0; 

	/*
	if (log) {
		lfp = fopen("c:\\code\\checkerspositions.txt", "a");
		//fseek(lfp, 0, SEEK_END);
		if (lfp != NULL) {
			fprintf(lfp, "%i %i %i %i %i\n", p->bm, p->bk, p->wm, p->wk, p->color);
			fclose(lfp);
		}
	}*/



	
	// initialize module if necessary
	if (!cakeisinit) {
		printf("\ninitializing Cake");
		initcake(str);
		// TODO: move getlogfile code here once, so that we know the name of 
		// the logfile once and for all
	}

	// open the log file at the start of a search
#ifndef NOLOGFILE
	if (log & 1) {
		fp = getlogfile(0);
		//logtofile(fp, "\ngetmove called");
	}
#else
	fp = NULL; 
#endif

	//logtofile(fp, "repcheck array at very start of cake_getmove");
	//for (i = 0; i <= HISTORYOFFSET; i++) {
	//	sprintf(Lstr, "(%i, %i, %i)", i, si->repcheck[i].hash, si->repcheck[i].irreversible);
	//	logtofile(fp, Lstr);
	//}

	resetsearchinfo(si);

	// set "age" telling which hashentries can be overwritten (all those with different age)
	age = (age + 1) % 4; 
	si->age = age; 


	memset(iscapture, 0, MAXDEPTH * sizeof(int)); 

	*playnow = 0;
	si->play = playnow;
	si->out = str;
	si->searchmode = how;

	// set time limits
	si->maxtime = maximaltime;
#ifdef TIMEOPTIMIZED
	si->aborttime = 2.5 * maximaltime;
#else
	//si->aborttime = 4 * maximaltime;
	si->aborttime = 3 * maximaltime; 
#endif
	// if exact:
	if(info&2)
		si->aborttime = maximaltime;
	// allscores:
	if(info&8)
		si->allscores = 1;
	// if noabort
	if (info & 16)
		si->aborttime = 10 * maximaltime; 
	if (info & 32)
		hashclear(); 
	if(log & 1)
		printboardtofile(p, fp);
	// print current directory to see whether CB is getting confused at some point.
	//GetCurrentDirectory(256, pvstring);
	// initialize material 
	countmaterial(p, &(si->matcount));

#ifdef MOHISTORY // reset history table 
	memset(si->history,0,32*32*sizeof(int));
#endif

	// check if position we are searching resembles last searched position or not
	// this is good for deciding on hash clear, and for deciding on repetition checks
	difference = p->bm ^ lastsearchpos.bm;
	difference |= (p->wm ^ lastsearchpos.wm);
	difference |= (p->bk ^ lastsearchpos.bk);
	difference |= (p->wk ^ lastsearchpos.wk);
	if (bitcount(difference) > 6)
		difference = 1;
	else
		difference = 0;

	if (!p->bk && !p->wk)
		irreversible = 1; 
	if (p->bm != lastsearchpos.bm)
		irreversible = 1; 
	if (p->wm != lastsearchpos.wm)
		irreversible = 1; 

	// check if position we are searching is identical to the last one searched
	//if (p->bm == lastsearchpos.bm && p->bk == lastsearchpos.bk &&
	//	p->wm == lastsearchpos.wm && p->wk == lastsearchpos.wk)
	//	identical = 1; 

	// clear the hashtable 
	//if(reset)
	//memset(hashtable,0,(hashsize+HASHITER)*sizeof(HASHENTRY));
	// initialize hash key 
	absolutehashkey(p, &(si->hash));
	//sprintf(Lstr, "hash key before Cake's move is %i", si->hash.key);
	//logtofile(fp, Lstr);
	

#ifdef LEARNUSE //stuff learned positions in the hashtable
	stufflearnpositions();
#endif

	// what is this doing at all?
	norefresh=0;

	// what is this doing here?
	//n = makecapturelist(p, movelist, values, 0);

#ifdef REPCHECK
	//logtofile(fp, "repcheck array at start of cake_getmove"); 
	//for (i = 0; i <= HISTORYOFFSET; i++) {
	//	sprintf(Lstr, "(%i, %i, %i)", i, si->repcheck[i].hash, si->repcheck[i].irreversible);
	//	logtofile(fp, Lstr);
	//}
	// initialize history list: holds the last few positions of the current game 
	dummy.hash = 0;
	dummy.irreversible = 0;
	if(reset != 0)
		{
		//logtofile(fp, "\nreset indicated by CB, resetting rep array\n"); 
		for (i = 0; i < HISTORYOFFSET; i++) {
			si->repcheck[i] = dummy;
			}
		}
#ifdef NEWREPDETECTION
	// no reset indicated by CB; check if we have to shift history array
	else {
		if (si->repcheck[HISTORYOFFSET].hash != si->hash.key) {
			//logtofile(fp, "\nNo reset, shifting repcheck array by 1");
			for (i = 0; i < HISTORYOFFSET; i++) {
				si->repcheck[i] = si->repcheck[i + 1];
			}
			si->repcheck[HISTORYOFFSET].hash = si->hash.key;
			si->repcheck[HISTORYOFFSET].irreversible = irreversible;  
		}
		//else
		//	logtofile(fp, "\nlast position found in repcheck array, not shifting it");
	}

	/*if (reset != 0)
		logtofile(fp, "\nReset indicated by calling function, resetting repcheck array"); 
	logtofile(fp, "repcheck array after init in cake_getmove");
	for (i = 0; i <= HISTORYOFFSET; i++) {
		sprintf(Lstr, "(%i, %i, %i)", i, si->repcheck[i].hash, si->repcheck[i].irreversible);
		logtofile(fp, Lstr);
	}*/
#endif

	// repcheck disabled by deleting everything
	//for (i = 0; i < HISTORYOFFSET; i++) 
	//	si->repcheck[i] = dummy;



	if (difference != 0) {
		//if(log & 1)
		//	logtofile(fp, "\n******very different position found, assuming new game");
		newgamestarts++;
		//printf("\nhash reset"); 

		/*if ((newgamestarts % 500) == 0) {
			logtofile(fp, "\nclearing hashtable due to game number");
			memset(hashtable, 0, (hashsize + HASHITER) * sizeof(HASHENTRY));
		}*/
		//for (i = 0; i < hashsize + HASHITER; i++)
		//	hashtable[i].depth = 0; 
	}
	/*else {
		if(log & 1)
			logtofile(fp, "\n******very similar position found");
	}*/



#endif

#ifdef TESTPERF
	for(i=0;i<MAXMOVES;i++)
		cutoffsat[i]=0;
#endif
	
	// do a book lookup TODO: put this all in a small function in book.c
	if(usethebook)
		{
		bookfound=0;
		bookindex=0;
		if(log & 1)
			logtofile(fp, "\nbook lookup");
		if(booklookup(p,&booklookupvalue,0,&bookdepth,&bookindex,str, book, bookentries, usethebook, fp))
			{
			if(log & 1)
				logtofile(fp, "...success");
			// booklookup was successful, it sets bookindex to the index of the move in movelist that it wants to play
			bookfound = 1;
			n = makecapturelist(p, movelist, values, 0);
			if(!n)
				n = makemovelist(si, p,movelist,values,0,0);
			// set best value
			bookmove = movelist[bookindex];
			}
		else {
			if(log & 1)
				logtofile(fp, "...not found");
		}
		
		// if the remaining depth is too small, we dont use the book move 
		if(bookdepth < BOOKMINDEPTH) 
			bookfound = 0;
		if(bookfound)
			{
			movetonotation(p,&bookmove,Lstr);
			//sprintf(str,"found position in book, value=%i, move is %s (depth %i)\n",booklookupvalue,Lstr,bookdepth);
			best=bookmove;
			value=0;
			}
		else
			{
			sprintf(str,"%X %X not found in book\n",si->hash.key,si->hash.lock);
			value=0;
			}
		if(log & 1)
			logtofile(fp, str);
		}
	else
		bookfound=0;

	//absolutehashkey(p, &(si->hash));
	//countmaterial(p, &(si->matcount));
	
	// check if the move on the board is forced - if yes, we don't waste time on it.
	forcedmove = isforced(p);

	// get a movelist which we pass around to mtdf() and firstnegamax() - do this here
	// because we want to keep it ordered during iterative deepening.
	memset(movelist, 0, MAXMOVES * sizeof(MOVE));
	n = getorderedmovelist(p, movelist);

	//printf("\nnumber of moves is %i", n);

	si->start = clock();
	guess=0;
	
	if(!bookfound)
		{
#ifdef FIXEDDEPTH  // check if this version is set to search to a fixed depth
		how = DEPTH_BASED;
		depthtosearch = FIXEDDEPTH;
#endif
		for(d=1; d<MAXDEPTH; d+=2)
			{
			si->leaf=0;
			si->leafdepth=0;
			si->depth = d; 
#ifndef MTD
			// windowed search
			value = windowsearch(si, p, movelist, FRAC*d, guess, &best);
			// set value for next search 
			guess=value;	
#endif
#ifdef MTD
			// MTD(F) search 
			if(si->allscores == 0)
				{
				value = mtdf(si, p, movelist, guess, FRAC*d, &best);   
				// best move comes from firstnegamax on a fail high, i.e. when MTD driver which is called
				// here produces a fail high in its firstnegamax call, we get a new move. 
				guess = value;	
				}
			else
				value = allscoresearch(si, p, movelist, FRAC*d, &best);
#endif
			if(log & 1)
				logtofile(fp, si->out); 

			// count zero evals
			if(value == 0)
				zeroes++;
			else 
				zeroes = 0;
			// count winning evals
			// do not count evals > 400 (db wins) as this would confuse cake when it's in a db win!
			if(abs(value)>=100 && abs(value)<400)
				winslosses++;
			else
				winslosses=0;

			// generate output 
			movetonotation(p,&best,Lstr);
			getpv(si, p, pvstring);
			t = clock();

			// iterative deepening loop break conditions: depend on search mode
			// todo: make time based search mode smarter by looking at 
			// ratio of last two iterations
			if(d>0)
				{
#ifdef TIMEOPTIMIZED
				if (si->searchmode == TIME_BASED && ((clock() - si->start) / CLK_TCK > (si->maxtime * 0.57)))
					break;
				// don't allow going two 1-ply steps.
				if (si->searchmode == TIME_BASED && ((d % 2) == 0))
					break; 
				if (si->searchmode == TIME_BASED && ((clock() - si->start) / CLK_TCK > (si->maxtime *0.38)))
					d--;
#else
				//if (si->searchmode == TIME_BASED && ((clock() - si->start) / CLK_TCK > (si->maxtime / 2)))
				if (si->searchmode == TIME_BASED && ((clock() - si->start) / CLK_TCK > (si->maxtime * 0.4)))
					break;
#endif
				if( si->searchmode == DEPTH_BASED && d>=depthtosearch)
					break;
				if(si->searchmode == NODE_BASED && si->negamax>maxnodes)
					break;
#ifdef IMMEDIATERETURNONFORCED
				// do not search if only one move is possible 
				if(forcedmove) // this move was forced! 
					{
					// set return value to 0 so no win/loss claims hamper engine matches*/
					value=0;
					break;
					}
#endif
				if(abs(value)>MATE-100) 
					break;
				}

			if(*(si->play))
				{
				// the search has been aborted, either by the user or by cake++ because 
				// abort time limit was exceeded
				// stop the search. don't use the best move & value because they might be rubbish 
				//logtofile(fp, "\ncalculation interrupt! replace ");
				//movetonotation(p, &best, Lstr);
				//logtofile(fp, Lstr);
				//logtofile(fp, "with");
				
#ifndef FASTUPDATE
				best = last;
#endif
				movetonotation(p,&best,Lstr);
				logtofile(fp, Lstr); 
				value=lastvalue;
				sprintf(str,"interrupt: best %s value %i",Lstr,value);
				break;
				}
			lastvalue = value;	// save the value for this iteration 
			last = best;			// save the best move on this iteration 
			norefresh = 1;
			}		// end of for depth move
		}

	si->value = value; 
	if (log & 2)
		printf("\n%s", si->out); 
	// check if we can learn this position
#ifdef LEARNSAVE
	dolearn = 0;
	if(!(*(si.play)))
		{
		// we only learn if we had a bit of a search
		if(d>=13)
			{
			stones = bitcount(p->bm | p->bk | p->wm |p->wk);
			if(stones > maxNdb) 
				{
				// draws if zero values and few stones or
				if((zeroes>=3) && (stones <= maxNdb+4) && repetitiondraw==0)
					dolearn = 1;
				// draws if lots of zero values and a few more stones.
				if((zeroes>=5) && (stones <= maxNdb+6) && repetitiondraw==0)
					dolearn = 1;
				}
			// we learn wins/losses
			if(winslosses>1)
				dolearn = 1;
			}
		}
	// now if dolearn is set, we save the position into the learnfile
	if(dolearn)
		learn(p, value, d, si.Gbestindex);
#endif // LEARNSAVE

	// TODO: for new cake 1.85, mtdf should return a proper best move, always (and defend
	// against interrupts itself
	// comment out the next two lines! TODO: sems this should not be necessary anyway because
	// it's done in loop above!
#ifndef FASTUPDATE
	if (*(si->play)) {
		best = last;
		si->aborted = 1;
	}
#endif
	togglemove(p,best);


	// calculate new hash key

	absolutehashkey(p, &(si->hash));

	// find out if move was likely reversible
	if (p->bm != lastsearchpos.bm ||
		p->wm != lastsearchpos.wm)
		irreversible = 1;
	else
		irreversible = 0; 


	// remember this position 
	lastsearchpos.color = p->color;
	lastsearchpos.bm = p->bm;
	lastsearchpos.bk = p->bk;
	lastsearchpos.wm = p->wm;
	lastsearchpos.wk = p->wk;

#ifdef NEWREPDETECTION
	// Cake made a move, so we certainly shift history array

	//sprintf(Lstr, "hash key after Cake's move is %i", si->hash.key); 
	//logtofile(fp, Lstr); 
	//logtofile(fp, "\nShifted repcheck array by 1 at end of search\nnew history array after Cake's move is");
	for (i = 0; i < HISTORYOFFSET; i++) {
		si->repcheck[i] = si->repcheck[i + 1];
	}
	si->repcheck[HISTORYOFFSET].hash = si->hash.key;
	si->repcheck[HISTORYOFFSET].irreversible = irreversible;
	/*for (i = 0; i <= HISTORYOFFSET; i++) {
		sprintf(Lstr, "(%i, %i, %i)", i, si->repcheck[i].hash, si->repcheck[i].irreversible);
		logtofile(fp, Lstr);
	}*/

#endif

	// we are done searching, close the log file
	if (fp != NULL)
		fclose(fp); 

	//PgoAutoSweep("");


	// return value: WIN / LOSS / DRAW / UNKNOWN
	// if this position occurred before, return a rep draw
	int repetitions = 0; 
	for(i= HISTORYOFFSET; i>=0; i--) {
		if(si->repcheck[i].hash == si->hash.key && p->bk && p->wk) {
			repetitions++; 
			if (repetitions > 1) {
				strcat(str, " Cake claims repetition draw");
				return DRAW;
			}
		}
	}
	if(value > WINLEVEL) {
		strcat(str, " Cake claims a win"); 
		return WIN;
	}
	if(value < -WINLEVEL) {
		strcat(str, " Cake claims a loss"); 
		return LOSS;
	}
#ifdef USEDB
	countmaterial(p, &(si->matcount));
	if(isdbpos(p, &(si->matcount)))
		{
		value = dblookup(p, 0, &(si->matcount));
		if(value == DB_DRAW) {
			strcat(str, " Cake claims a database draw"); 
			return DRAW;
			}
		}
#endif
	return UNKNOWN;
}


#ifdef USEDB
int isdbpos(POSITION *p, MATERIALCOUNT *m)
	{
	if(m->bm + m->bk + m->wm + m->wk <= maxNdb && max(m->bm + m->bk, m->wm + m->wk) <= MAXPIECE)
		{
		if(testcapture(p))
			return 0;
		p->color ^= CC;
		if(testcapture(p))
			{
			p->color^=CC;
			return 0;
			}
		p->color ^= CC;
		return 1;
		}
	return 0;
	}
#endif


	/*--------------------------------------------------------------------------
	|																			|
	|		driver routines for search: MTD(f) and windowsearch					|
	|																			|
	 --------------------------------------------------------------------------*/

int mtdf(SEARCHINFO *si, POSITION *p, MOVE movelist[MAXMOVES], int firstguess,int depth, MOVE *best)
	{
	// todo: test "correct return values", 
	// also look if the best move is always the same as the last fail-high - 
	// perhaps that's what mtdf should return: the fail-high move with the highest
	// value?!
	int g, lowerbound, upperbound, beta;
#ifdef FASTUPDATE
	MOVE lastbest = { .bm = 0,.bk = 0,.wm = 0,.wk = 0 };
#endif
	double time;
	char Lstr1[1024]="",Lstr2[1024]="";
	int failedhigh = 0; 
	
	g = firstguess;  // or g = 0?
	upperbound = MATE;
	lowerbound = -MATE;
	while(lowerbound<upperbound)
		{
		if(g==lowerbound)
			beta=g+1;
		else beta=g;
		g = firstnegamax(si, p, movelist, depth, beta-1, beta, best);
	
		if(g<beta)
			{
			// minimal window search failed low - perhaps not good to trust this result?
			upperbound=g;
			// TODO: move sprintfs into a #ifdef FULLOG clause
			sprintf(Lstr1,"value<%i",beta);
			}
		else
			{
			// minimal window search failed high, perhaps OK to trust this result?
			lowerbound=g;
			failedhigh = 1; 
			//lowerbound = beta;
			sprintf(Lstr1,"value>%i",beta-1);
			// TODO: if fastupdate I should set best here if si->play is not set!
			}
		
		time = ( (clock() - si->start)/CLK_TCK);
		getpv(si, p, Lstr2);

#ifdef TO_MTD
		if (si->searchmode == TIME_BASED && time > si->maxtime && failedhigh)
			break;
		// TODO: only do this if we ever had a fail high; and/or check what happens if si->play?
#endif



		searchinfotostring(si->out, depth, time, Lstr1, Lstr2, si);
#ifdef FULLLOG
		sprintf(Lstr,"\n -> %s",si->out);
		logtofile(Lstr);
#endif	

		// new in cake 185
		// return if calculation interrupt is requested but also remember which was last best move before interrupt
		// verify!!!
#ifdef FASTUPDATE
		if(*(si->play) != 0) {
			(*best) = lastbest;
			break;
		}
		else  // normal search, store best move
			lastbest = (*best); 
#endif
		}
	sprintf(Lstr1,"value=%i",g);
	searchinfotostring(si->out, depth, time, Lstr1, Lstr2, si);
//#ifndef NEWLOG
//	logtofile(si->out);
//#endif
	//printf("%s\n", si->out); 
	 
	return g;
	}
	

int allscoresearch(SEARCHINFO *si, POSITION *p, MOVE movelist[MAXMOVES], int d, MOVE *best)
	{
	int i, j, n, m;
	static int values[MAXMOVES];
	int bestindex = 0;
	char str[256], tmpstr[256], movestr[256];
	int tmpvalue;
	MOVE tmpmove;
	MATERIALCOUNT Lmatcount;
	HASH localhash;
	MOVE ml[MAXMOVES];
	
	if(*(si->play)) 
		return 0;
	
	// get number of moves in movelist
	n = numberofmoves(movelist);
	// save old hashkey 
	localhash = si->hash;
	// save old material balance 
	Lmatcount = si->matcount;
	*best = movelist[0];
	
	for(i=0;i<n;i++)
		{
		// domove
		togglemove(p,movelist[i]);
		countmaterial(p, &(si->matcount));
		si->realdepth++;
		updatehashkey(&movelist[i], &(si->hash));
		m = getorderedmovelist(p, ml);
#ifdef REPCHECK
		si->repcheck[si->realdepth + HISTORYOFFSET].hash = si->hash.key;
		si->repcheck[si->realdepth + HISTORYOFFSET].irreversible = movelist[i].bm|movelist[i].wm;
#endif
		
		/********************recursion********************/
		values[i] = -mtdf(si, p, ml, 0, d, best);
		//		guess = value;	
		/******************* end recursion ***************/
		
		// undomove
		si->realdepth--;
		togglemove(p,movelist[i]);
		si->hash = localhash;
		si->matcount = Lmatcount;

		// create output string
		sprintf(str,"d%i  ",d/FRAC);
		for(j=0;j<n;j++)		
			{
			if(j<=i)
				{
				movetonotation(p, &movelist[j], movestr);
				sprintf(tmpstr, "%s: %i   ", movestr, values[j]);
				strcat(str, tmpstr);
				}
			else
				{
				movetonotation(p, &movelist[j], movestr);
				sprintf(tmpstr, "%s:(%i)  ", movestr, values[j]);
				strcat(str, tmpstr);
				}
			}
		sprintf(si->out,"%s", str);
		}

	//logtofile(str);

	// now, order the list according to values
	for(j=0;j<n;j++)
		{
		for(i=0;i<n-1;i++)
			{
			if(values[i] < values[i+1])
				{
				// swap
				tmpvalue = values[i];
				values[i] = values[i+1];
				values[i+1] = tmpvalue;
				tmpmove = movelist[i];
				movelist[i] = movelist[i+1];
				movelist[i+1] = tmpmove;
				}
			}
		}
	*best = movelist[0];
	return 1;
	}



int firstnegamax(SEARCHINFO *si, POSITION *p, MOVE movelist[MAXMOVES], int d, int alpha, int beta, MOVE *best)
	{

	/*--------------------------------------------------------------------------
	|																			|
	|		firstnegamax: first instance of negamax which returns a move		|
	|																			|
	 --------------------------------------------------------------------------*/
	int i,value,swap=0,bestvalue=-MATE;
	int n;					
	MOVE ml2[MAXMOVES];		
	MATERIALCOUNT Lmatcount;
	int Lalpha=alpha;
	int32 forcefirst=0;
	int32 Lkiller=0;
	MOVE tmpmove;
	int refnodes[MAXMOVES];		/* number of nodes searched to refute a move; used to order move list */
	int statvalues[MAXMOVES];	
	int tmpnodes;				
	int bestindex=0;
	HASH localhash;

	if(*(si->play)) 
		return 0;
	si->negamax++;

	assert((p->bm & p->bk) | (p->wm & p->wk) == 0);


	for(i=0;i<MAXMOVES;i++)
		refnodes[i] = 0;
	
	// get number of moves in movelist
	n = numberofmoves(movelist);

	if(n==0)
		return -MATE+si->realdepth;
	
	//save old hashkey 
	localhash = si->hash;

	// save old material balance 
	Lmatcount = si->matcount;
	*best = movelist[0];

	// For all moves do...
	for(i=0;i<n;i++)
		{
		togglemove(p,movelist[i]);
		countmaterial(p, &(si->matcount));
#ifdef NEW_NOMATERIAL_CHECK
		if (p->color == WHITE && si->matcount.wm + si->matcount.wk == 0) {
			si->matcount = Lmatcount;
			togglemove(p, movelist[i]);
			*best = movelist[i];
			bestvalue = MATE; 
			break; 
		}

		if (p->color == BLACK && si->matcount.bm + si->matcount.bk == 0) {
			si->matcount = Lmatcount;
			togglemove(p, movelist[i]);
			*best = movelist[i];
			bestvalue = MATE;
			break;
		}

#endif

		si->realdepth++;
		updatehashkey(&movelist[i], &(si->hash));
#ifdef REPCHECK
		si->repcheck[si->realdepth + HISTORYOFFSET].hash = si->hash.key;
		si->repcheck[si->realdepth + HISTORYOFFSET].irreversible = movelist[i].bm|movelist[i].wm;
#endif
		tmpnodes = si->negamax;
		/********************recursion********************/
		assert((p->bm & p->bk) | (p->wm & p->wk) == 0);

#ifdef LATEMOVEREDUCTIONROOT
		// late move reduction: the assumption is that late moves will fail low, so we search
		// them with a bit less depth and let them fail low. if they don't fail low, then we have to research!

		// bestmovevalue takes care that hash and killer move are never reduced
		// testcapture (&q) makes sure that moves leading to captures are not reduced, but is this sensible?

		// v 185 b that was good hat LATEMOVEMINDEPTH 2*FRAC, i<1 and testcapture

		if (d <= LATEMOVEMINDEPTH ||  i<3 /*||  testcapture(&q)*/)
			value = -negamax(si, p, d - FRAC, -(alpha + 1), &Lkiller, &forcefirst, 0, 0, 0);
		else
			// we have a candidate for reduction
			{
			value = -negamax(si, p, d - FRAC - LATEMOVEDEPTH, -(alpha + 1), &Lkiller, &forcefirst, 0, 0, 0);
			if (value > alpha)
				value = -negamax(si, p, d - FRAC, -(alpha + 1), &Lkiller, &forcefirst, 0, 0, 0);
			}
#else
		value = -negamax(si, p,d-FRAC,-beta, &Lkiller, &forcefirst, 0,0,0);
#endif


		/******************* end recursion ***************/
		refnodes[i] = si->negamax-tmpnodes;
		si->realdepth--;
		togglemove(p,movelist[i]);
		// restore the old hash key
		si->hash = localhash;
		// restore the old material balance 
		si->matcount = Lmatcount;
		bestvalue = max(bestvalue,value);

		if(value >= beta) 
			{
			*best = movelist[i];/*Lalpha=value;*/
			swap=i;
			break;
			}
		if(value > Lalpha) 
			{
			*best=movelist[i];
			Lalpha=value;
			swap=i;
			}
		}
	/* save the position in the hashtable */
	/* a bit complicated because we need to save best move as index */
	// what if we don't set forcefirst here, i.e. set it to 0?
	n = makecapturelist(p, ml2, statvalues, 0);
	
	iscapture[si->realdepth] = n;
	
	if(n==0)
		n = makemovelist(si, p, ml2, statvalues, 0,0);
	
	// search the best move
	for(i=0; i<n; i++)
		{
		if(ml2[i].bm==best->bm && ml2[i].bk==best->bk && ml2[i].wm==best->wm && ml2[i].wk==best->wk)
			break;
		}
	bestindex = i;

#ifdef THREADSAFEHT
	EnterCriticalSection(&hash_access);
#endif	
	hashstore(si, p, bestvalue, alpha, d, best, bestindex);
#ifdef THREADSAFEHT
	LeaveCriticalSection(&hash_access);
#endif


	// set global Gbestindex for learn function
	si->Gbestindex = bestindex;

	// order movelist according to the number of nodes it took
	if(swap!=0)
		{
		tmpmove = movelist[swap];
		tmpnodes = refnodes[swap];
		for(i=swap;i>0;i--)
			{
			movelist[i] = movelist[i-1];
			refnodes[i] = refnodes[i-1];
			}
		movelist[0] = tmpmove;
		refnodes[0] = tmpnodes;
		}
	return bestvalue;
}



#ifdef QSEARCH
int qsearch(SEARCHINFO* si, POSITION* p, int alpha, int beta, int qsdepth)
{
	// qsearch checks "violent" moves, i.e. all moves that lead to a possible
	// capture for side not to move. 
	// qsearch assumes that there is no capture at the moment for the side to move!

	// todo: also find squeeze moves, generate these and drop back into negamax!

	// if we find any move with value >= beta we can return immediately.
	int i, n, value;
	int maxvalue = -MATE;
	int32 Lkiller = 0;
	HASH h;

	MOVE movelist[MAXMOVES];
	POSITION q;
	int forcefirst = MAXMOVES - 1;
	MATERIALCOUNT mc;
	int killer = 0;
	int bestmoveindex = 0;

	si->qsearch++;
	si->negamax++;

	// check material:
	if (p->bm + p->bk == 0)
		return evaluation_nomaterial_black(p, si->realdepth);
	if (p->wm + p->wk == 0)
		return evaluation_nomaterial_white(p, si->realdepth);

	assert((p->bm & p->bk) | (p->wm & p->wk) == 0);

	/* what does qs do?
	 * 1) generate moves that lead to a capture for the opponent (taking care there is a recapture??)
	 * 2) if no moves found, return
	 * 3) else recursion over loops to csearch()
	 * 4) return maximal value
	 */


	n = makeQSmovelist(p, movelist);
	// if we have no potential shots, return

	// TODOTODO: no, if we have no moves, we should check for squeezes


	if (n > 0) {
		// now do recursion over all moves. note that we either have a capture, then we certainly
		// have moves, or we already 

		// store old material balance and hash
		h = si->hash;
		mc = si->matcount;

		for (i = 0; i < n; i++)
		{
			domove(q, p, movelist[i]);

			// could assert testcapture here
			// TODO: why can this assert fail?
			assert((p->bm & p->bk) | (p->wm & p->wk) == 0);

			/*if (testcapture(&q) == 0) {
				printboard(p);
				printboard(&q);
				printf("\n no capture in qsearch!!");
				getch();
			}*/
			assert(testcapture(&q));

			// if we arrive here, the opponent has a capture now - so
			// we search this move.
			si->realdepth++;

			// update the hash key
			updatehashkey(&(movelist[i]), &(si->hash));

			// updatematerial 
			if (p->color == BLACK)
			{
				if (movelist[i].bk && movelist[i].bm)
				{
					si->matcount.bk++; si->matcount.bm--;
				}
			}
			else
			{
				if (movelist[i].wk && movelist[i].wm)
				{
					si->matcount.wk++; si->matcount.wm--;
				}
			}

			//recursion
			// here, we only increase qsdepth if we do a real qs:
			value = -csearch(si, &q, -beta, -alpha);

			// restore the hash key and material count
			si->hash = h;
			si->matcount = mc;
			si->realdepth--;

			/* update best value so far */
			maxvalue = max(value, maxvalue);
			/* and set alpha and beta bounds */
			if (maxvalue > alpha)
			{
				si->qsearchsuccess++;
				// we found a shot, print it:

				/*
				printboard(p);
				printint32(movelist[i].bm|movelist[i].bk|movelist[i].wm|movelist[i].wk);
				printf("\nalpha: %i\tvalue: %i", alpha, maxvalue);
				_getch();
				*/

				return maxvalue;

			}

			// TODO: in MTD formulation this is not necessary
			//if(maxvalue>alpha) 
			//	alpha=maxvalue;

		} /* end main recursive loop of forallmoves */
	}
	// check for squeezes:
	// add IFDEF QS_SQUEEZE here

	n = makeSqueezemovelist(p, movelist);

	if (n == 0) {
		si->qsearchfail++;
		return maxvalue;
	}

	// store old material balance and hash
	h = si->hash;
	mc = si->matcount;


	for (i = 0; i < n; i++)
	{
		domove(q, p, movelist[i]);

		// could assert testcapture for side not to move here
		// wow, this assert fails... how is this possible
		//assert(testcapture(&q));

		si->realdepth++;

		// update the hash key
		updatehashkey(&(movelist[i]), &(si->hash));

		//recursion
		// here, we only increase qsdepth if we do a real qs:
		//value = -csearch(si, &q, -beta, -alpha);
		value = -negamax(si, &q, 0, -(alpha + 1), &killer, &bestmoveindex, 0, 0, 0);

		// restore the hash key and material count
		si->hash = h;
		si->realdepth--;

		/* update best value so far */
		maxvalue = max(value, maxvalue);
		/* and set alpha and beta bounds */
		if (maxvalue > alpha)
		{
			si->qsearchsuccess++;
			// we found a squeeze that helped, print it:
			/*printboard(p);
			printint32(movelist[i].bm|movelist[i].bk|movelist[i].wm|movelist[i].wk);
			printf("\nsqueeze! alpha: %i\tvalue: %i", alpha, maxvalue);
			_getch();*/
			return maxvalue;
		}
	}

	si->qsearchfail++;
	return maxvalue;
}


int csearch(SEARCHINFO *si, POSITION *p, int alpha, int beta)
	{
	// csearch plays out captures
	// it expects to get only positions with captures!
	int i, n, value;
	int maxvalue = -MATE;
	int32 Lkiller = 0;
	HASH h; 

	MOVE movelist[MAXMOVES];
	POSITION q;	
	int forcefirst = MAXMOVES-1;
	int values[MAXMOVES];
	int delta;
	MATERIALCOUNT mc;

	//si->qsearch++;
	si->negamax++;

	// check material:
// check material:
	if(p->bm + p->bk == 0)
		return evaluation_nomaterial_black(p, si->realdepth); 
	if(p->wm + p->wk == 0)
		return evaluation_nomaterial_white(p, si->realdepth); 

	assert((p->bm & p->bk) | (p->wm & p->wk) == 0);
	/* what does cs do?
	 * 1) recursion over all capture moves
	 * 2) return the value
	 */

	// TODO: check what is faster: testcapture followed by makecapturelist or just makecapturelist
	// TODO: csearch is called by qsearch when we KNOW we have a capture, so this 
	// testcapture call is always 1 when called by qsearch!

	if(testcapture(p) == 0)
		return evaluation(p, &(si->matcount), alpha, &delta, 0, maxNdb);	
	
	n = makecapturelist(p, movelist, values, forcefirst);
	
	
	// if no more captures, return eval
	// here would be the place to call qsearch again if we want to make qsearch recursive
	//if(n == 0)
		
		
	
	// now do recursion over all moves. note that we either have a capture, then we certainly
	// have moves, or we already 

	// store old material balance and hash
	h = si->hash;
	mc = si->matcount;

	for(i=0;i<n;i++)
		{
		domove(q,p,movelist[i]);
		assert((p->bm & p->bk) | (p->wm & p->wk) == 0);

		// if we arrive here, the opponent has a capture now - so
		// we search this move.
		si->realdepth++;

		// update the hash key
		updatehashkey(&(movelist[i]), &(si->hash));  

		// updatematerial if capture
		if(p->color == BLACK)
			{
			si->matcount.wm -= bitcount(movelist[i].wm);
			si->matcount.wk -= bitcount(movelist[i].wk);
			
			if(movelist[i].bk && movelist[i].bm) 
				{si->matcount.bk++; si->matcount.bm--;}
			}
		else
			{
			si->matcount.bm -= bitcount(movelist[i].bm);
			si->matcount.bk -= bitcount(movelist[i].bk);
				
			if(movelist[i].wk && movelist[i].wm)
				{si->matcount.wk++; si->matcount.wm--;}
			}

		
		//recursion
		// here, we only increase qsdepth if we do a real qs:
		value = -csearch(si, &q, -beta, -alpha); 
		
		
		// restore the hash key and material count
		si->hash = h;
		si->matcount = mc;
		si->realdepth--;
		
		/* update best value so far */
		maxvalue = max(value,maxvalue);
		
		if(maxvalue > alpha)
			return maxvalue;

		} /* end main recursive loop of forallmoves */
	return maxvalue;	
	}
#endif

#ifdef SAFE
int safemoves(POSITION *p)
	{
	// safemoves returns the number of "safe" moves for
	// color - given that there are no capture moves!
	// it is only called when there are no kings on the board, therefore there is
	// nothing about kings in the code below.

	int32 tmp;
	int32 free;
	int32 safe;
	// TODO: improve with safe2:
	//int32 safe2;
	int safemoves = 0;

	// can compute free only with men, since safemoves cannot be called with kings.
	free = ~(p->bm | p->wm);
	
	if(p->color == BLACK)
		{
		// mark all black men that can move right forward
		safe = p->bm & leftbackward(free);
		// remove those that walk into a white man that can capture them
		tmp = safe & twoleftbackward(p->wm);
		safe ^= tmp;
		// remove those running into a white man that can capture them "diagonally"
		tmp = safe & twobackward(p->wm) & left(free);
		safe ^= tmp;
		// remove those protecting another man
		tmp = safe & rightbackward(p->bm) & tworightbackward(p->wm);
		safe ^= tmp;

		//safe2 = safe;
		safemoves += bitcount(safe);

		// other direction
		safe = p->bm & rightbackward(free);
		tmp = safe & tworightbackward(p->wm);
		safe ^= tmp;
		tmp = safe & twobackward(p->wm) & right(free);
		safe ^= tmp;
		tmp = safe & leftbackward(p->bm) & twoleftbackward(p->wm);
		safe ^= tmp;

		// safemoves = bitcount(safe | safe2)
		safemoves += bitcount(safe);
		}
	else
		{
		// mark all white men that can move right backward
		safe = p->wm & leftforward(free);
		// remove those that walk into a black man that can capture them
		tmp = safe & twoleftforward(p->bm);
		safe ^= tmp;
		// remove those running into a black man that can capture them "diagonally"
		tmp = safe & twoforward(p->bm) & left(free);
		safe ^= tmp;
		// remove those protecting another man
		tmp = safe & rightforward(p->wm) & tworightforward(p->bm);
		safe ^= tmp;

		safemoves += bitcount(safe);

		// other direction
		safe = p->wm & rightforward(free);
		tmp = safe & tworightforward(p->bm);
		safe ^= tmp;
		tmp = safe & twoforward(p->bm) & right(free);
		safe ^= tmp;
		tmp = safe & leftforward(p->wm) & twoleftforward(p->bm);
		safe ^= tmp;

		safemoves += bitcount(safe);
		}
	return safemoves;
	}
#endif  // SAFE

#ifdef TESTEVALSYMMETRY
int reverseposition(POSITION* p, POSITION* q)
{
	// takes position p, and produces q which is reversed

	q->bm = revert(p->wm);
	q->bk = revert(p->wk);
	q->wm = revert(p->bm);
	q->wk = revert(p->bk);
	q->color = p->color ^ CC;
	return 1;
}
#endif

int negamax(SEARCHINFO *si, POSITION *p, int d, int alpha, int32 *protokiller, int *bestmoveindex, int truncationdepth, int truncationdepth2, int iid)
	/*----------------------------------------------------------------------------
	|		TODO: incredibly, negamax is 540 lines of code!						  |
	|		negamax: the basic recursion routine of cake++						  |
	|					returns the negamax value of the current position		  |
	|					sets protokiller to a compressed version of a move,		  |
	|					if it's black's turn, protokiller = best.bm|best.bk		  |
	|					*bestindex is set to the best move index, and is only     |
	|					used for IID: there, it returns the best index, and then  |
	|					we continue setting forcefirst to that index.             |
	 ----------------------------------------------------------------------------*/
	{
	int value;
	int valuetype = -1;// set to something unknown (0/1 are lower/upper)
	int32 forcefirst = MAXMOVES-1;	// initialize to crazy value to show it's not valid
	int n, localeval = MATE;
	int maxvalue = -MATE;
	int delta=0;
#ifdef ETC
	int bestETCvalue;
#endif
	int i,j;
	int index, bestmovevalue;
	int bestindex;
	int32 Lkiller=0;
	int stmcapture = 0;
	int sntmcapture = 0;
#ifdef USEDB
	int dbresult;
#endif
	int isdbpos = 0;
	int cl = 0;				// conditional lookup - predefined as no. if CLDEPTH>d then it's 1.
	int ispvnode = 0;		// is set to 1 in hashlookup if this position was part of a previous pv.
#ifdef SAFE
	int safemovenum = 16;  // initialize to a large number so if not calculated it looks safe
#endif
#ifdef STOREEXACTDEPTH
	int originaldepth = d;
#endif
	HASH localhash;			// hash for copy/make
	POSITION q;				// board for copy/make 
	MATERIALCOUNT Lmatcount;// materialcount for copy/make

#ifdef GLOBALMOVESTACK
	MOVE* movelist = &(movestack[si->realdepth*MAXMOVES]);
#else
	MOVE movelist[MAXMOVES];// movelist
#endif
	
	
	int values[MAXMOVES];   // move ordering values
	
#ifdef LATEMOVEREDUCTION
	int reduction = 0; 
	//	char movestring[16]; 
	//char bestmovestring[16];
	//int lmrvalue; 
#endif

	//int futile = 0; 
	

	/* 
	 * negamax does the following:
	 * 1) abort search if time is elapsed
	 * 2) return MATE value if one side has no material
	 * 3) abort search if playnow is true
	 * 4) return with evaluation if MAXDEPTH is reached (this practically never happens)
	 * 5) hashlookup and return if successful
	 * 6) IID if hashlookup unsuccessful and remaining depth sufficient
	 * 7) check for repetition, return 0 if repetition
	 * 8) get capture status of current position (stmcapture, sntmcapture)
	 * 9) dblookup if no capture and stones < maxNdb
	 * 10) extension and pruning decisions
	 * 11) compute # of safe moves in position
	 * 12) if depth small enough, no capture, and enough safe moves: return evaluation
	 * 13) makemovelist & SINGLEEXTEND if n==1.
	 * 14) ETClookup
	 * 15) for all moves: domove, recursion, undomove
	 * 16) hashstore
	 * 17) return value
	 */


#ifdef TESTEVALSYMMETRY
	MATERIALCOUNT revmc;
	//EVALUATION re;
	int normaleval, reverseeval;


	stmcapture = testcapture(p);
	p->color ^= CC;
	sntmcapture = testcapture(p);
	p->color ^= CC;

	revmc.bk = si->matcount.wk;
	revmc.bm = si->matcount.wm;
	revmc.wm = si->matcount.bm;
	revmc.wk = si->matcount.bk;

	reverseposition(p, &q);

	//return evaluation(p, &(si->matcount), alpha, &delta, 0, maxNdb);
	normaleval = evaluation(p, &(si->matcount), alpha, &delta, stmcapture | sntmcapture, maxNdb);
	reverseeval = evaluation(&q, &revmc, -alpha, &delta, stmcapture | sntmcapture, maxNdb);

	if (normaleval != reverseeval)
	{
		printboard(p);
		printboard(&q); 
		printf("\n(%i) normal: %i\treversed: %i", si->negamax, normaleval, reverseeval);
		//printf("\nnormal: %i br, %i comp, %i cramp, %i hold, %i king, %i king-man, %i material, %i men, %i RA, %i ST", e.backrank, e.compensation, e.cramp, e.hold, e.king, e.king_man, e.material, e.men, e.runaway, e.selftrap);
		//printf("\nreverse: %i br, %i comp, %i cramp, %i hold, %i king, %i king-man, %i material, %i men, %i RA, %i ST", re.backrank, re.compensation, re.cramp, re.hold, re.king, re.king_man, re.material, re.men, re.runaway, re.selftrap);
		getch();
		///normaleval = evaluation(p, &(si->matcount), alpha, &delta, stmcapture | sntmcapture, maxNdb);
		//reverseeval = evaluation(&q, &revmc, -alpha, &delta, stmcapture | sntmcapture, maxNdb);
		//printf("!");
	}
	else
		{printf("ok");}

#endif // TESTEVALSYMMETRY

	// time check: abort search if we exceed the time given by aborttime
#ifdef CHECKTIME
	if( (si->negamax & 0xFFFF)==0 && si->searchmode == TIME_BASED)
		if( (clock()- si->start)/CLK_TCK > (si->aborttime)) 
			*(si->play)=1;
#endif
	si->negamax++;


#ifndef NEW_NOMATERIAL_CHECK
	// check material:  // todo: only execute this after capture moves directly without calling negamax again?
	if(p->bm + p->bk == 0)
		return evaluation_nomaterial_black(p, si->realdepth); 
	if(p->wm + p->wk == 0)
		return evaluation_nomaterial_white(p, si->realdepth); 
#endif

	// return if calculation interrupt is requested
	if(*(si->play) != 0) 
		return 0;

	assert((p->bm& p->bk) | (p->wm & p->wk) == 0);

	// stop search if maximal search depth is reached - this should basically never happen
	if(si->realdepth >= MAXDEPTH) 
		{
		si->leaf++;
		si->leafdepth += si->realdepth;
		return evaluation(p,&(si->matcount),alpha,&delta,0,maxNdb);
		}
	

#ifdef SPA
	//------------------------------------------------//
	// search the current position in the spa table   //
	//------------------------------------------------//
	si->spalookups++;
	if (spalookup(si, &value, d, p->color))
	{
		si->spasuccess++;
		//printboard(p); 
		//printf("\nspa lookup successful, value %i", value);
		// return value 1: the position is in the spa table and the depth
		// is sufficient. its value and valuetype are used to cutoff 
		return value; 
	}
#endif


	//------------------------------------------------//
	// search the current position in the hashtable   //
	// only if there is still search depth left!      //
	// should test this without d>0 !?                //
	//------------------------------------------------//
	// TODO: check dblookup first!

	if(d >= 0 && !iid)
		{
		si->hashlookup++;
#ifdef THREADSAFEHT
		EnterCriticalSection(&hash_access);
#endif	
		if(hashlookup(si, &value, &valuetype, d, &forcefirst, p->color, &ispvnode)) {
#ifdef THREADSAFEHT
		LeaveCriticalSection(&hash_access);
#endif	
			si->hashlookupsuccess++;
			// the position is in the HT with sufficient depth. its value and valuetype are used to cutoff 
			if(valuetype == LOWER)
				{
				if(value > alpha) 
					return value;
				}
			else	//if(valuetype==UPPER)		
				{
				if(value <= alpha) 
					return value;
				}
			}
		}
	


#ifdef REPCHECK
	// check for repetitions  
	// TODO: maybe add repcheck[i-1].irreversible to the break statement to break faster
	// TODO: move this before IITERD above because if we return here we don't need to do the IITERD part!
	if(p->bk && p->wk) {
		for(i = si->realdepth + HISTORYOFFSET-2; i >= 0; i-=2) {
			// stop repetition search if move with a man is detected 
			if(si->repcheck[i].irreversible)
				break;
			if(si->repcheck[i].hash == si->hash.key)			
				return 0;                      
			}
		}
#endif

#ifdef IITERD // TODO: if we do the dblookup first, then we haven't counted the pieces properly?!
#ifdef USEDB
	/*
	if (forcefirst == MAXMOVES - 1)
		ifs[0]++; 
	if (d > IIDDEPTH)
		ifs[1]++; 
	if (si->matcount.bm + si->matcount.bk + si->matcount.wm + si->matcount.wk > maxNdb)
		ifs[2]++;

		ifs[0]: 611963291
		ifs[1]: 8515074
		ifs[2]: 600260589
		*/

	//if (forcefirst == MAXMOVES - 1 && d > IIDDEPTH && si->matcount.bm + si->matcount.bk + si->matcount.wm + si->matcount.wk > maxNdb)
	if (d > IIDDEPTH && forcefirst == MAXMOVES - 1  && si->matcount.bm + si->matcount.bk + si->matcount.wm + si->matcount.wk > maxNdb)
#else
	if (forcefirst == MAXMOVES - 1 && d > IIDDEPTH)
#endif
	{
		// TODO: make this MAXMOVES instead of MAXMOVES - 1 - theoretically it could be MAXMOVES
		// forcefirst == MAXMOVES-1 means no hashmove available
		// get a good first move with a shallower search, 4 ply less, fixed
		value = negamax(si, p, d - IIDREDUCE, alpha, protokiller, &forcefirst, truncationdepth, truncationdepth2, 1);
		assert(forcefirst >= 0 && forcefirst < MAXMOVES);
	}
#endif //IITERD

	
	// get info on captures: can side to move or side not to move capture now?
	stmcapture = testcapture(p);
	
	// get info on capture by opponent (if there is a capture on the board, then the database
	// contains no valid data
// NOTE TODO do we need this here yet? delay evaluation?

	// TODO: put this if in the else below!
	if(!stmcapture)	// NOTE: if we have no capture and opp has one we don't see it!
		{
		p->color ^= CC;
		sntmcapture = testcapture(p);
		p->color ^= CC;
		}

	if(stmcapture)
		n = makecapturelist(p,movelist,values,forcefirst);
    else
		n = 0;
	assert(si->realdepth < 99);
	iscapture[si->realdepth] = n;

	//--------------------------------------------//
	// check for database use                     // 
	// conditions: -> #of men < maxNdb            //
	//			   -> no capture for either side  //
	//                else result is incorrect    //
	//--------------------------------------------//
#ifdef USEDB
	if(si->matcount.bm + si->matcount.bk + si->matcount.wm + si->matcount.wk <= maxNdb && max(si->matcount.bm + si->matcount.bk, si->matcount.wm + si->matcount.wk) <= MAXPIECE)
		{
		isdbpos = 1;
		if(!(stmcapture|sntmcapture))
			{
			// this position can be looked up! only if neither side has a capture
			si->dblookup++;
			if(d<CLDEPTH*FRAC && !ispvnode)
				cl=1;
			dbresult = dblookup(p,cl, &(si->matcount));

			// statistics
			if(dbresult == DB_NOT_LOOKED_UP)
				si->dblookupfail++;
			else if(dbresult != DB_UNKNOWN)
				{
				si->dblookupsuccess++;			

				if(dbresult == DB_DRAW)
					return 0;
				if(dbresult == DB_WIN)
					{
					// we only need accurate value of db win if we are searching dbwin starting positions!
					if(alpha < 300)
						return 400; 
					value = dbwineval(p, &(si->matcount));
					if(value > alpha)
						return value;
					localeval = value;
					}
				if(dbresult == DB_LOSS)
					{
					if (alpha > -300)
						return -400;
					
					p->color ^= CC;
					value = -dbwineval(p, &(si->matcount));
					p->color ^= CC;
					
					if(value <= alpha)
						return value;
					localeval = value;
					}
				}
			}
		}
#endif // USEDB


#ifdef BOOKHT
	if(d>BOOKHTMINDEPTH && d<BOOKHTMAXDEPTH)
		{
		if(bookht_lookup(Gkey, Glock, &value, p->color))
			return value;
		}
#endif
#ifndef MARKPV
	ispvnode=0;
#endif
#ifdef EXTENDPV
	// TODO: maybe it's a tiny bit faster to do d+=EXTENDPVDEPTH*ispvnode?
	if (ispvnode)
		{d += EXTENDPVDEPTH;/*printf("*");*/}     // extension by half a ply
#endif

	/*-----------------------------------------------------
	|	pruning decisions:									|
	|	get local eval here and reduce search depth if it   |
	|   is too far away from (alpha, beta).                 |
	|   on the other hand, if the eval is close to the      |
	|   window again, we undo previous pruning              |
	\------------------------------------------------------*/
	// we don't do any truncation if:
	//  -> the side to move is capturing - we may be way off the score
	//  -> we are in a db position - there the huge scores confuse the pruning
	//  -> we are in a previous pv - we want to look at that carefully!
	
	// TODO: there used to be &!sntmcapture here - good or not?
	
	/*
	if (!stmcapture)
		ifs[4]++; 
	if (!isdbpos)
		ifs[5]++; 
	if (!ispvnode)
		ifs[6]++; 
		ifs[4]: 369847505
ifs[5]: 619939913
ifs[6]: 648698573*/
	if(!stmcapture & !isdbpos & !ispvnode)
		{
		// only get evaluation here if we don't have it already!
		assert(localeval == MATE);
		// get whole evaluation TODO: warning: evaluation can be called here with sntmcapture!
		localeval = evaluation(p,&(si->matcount),alpha,&delta, stmcapture|sntmcapture,maxNdb);
		delta = 0; 
		// eval will tell us if it thinks this eval is not stable and should not be pruned as much by returning delta large
		// todo: sntmcapture could mean we should prune less? maybe add something to delta if sntmcapture? or just not call
		// prune function because calling it will make search re-extend...?

		/* tested while making cake 1.88 13.1.2020 - such a nice idea but tested badly...
		// check if we can improve on local eval due to hashtable bounds
		if (valuetype == LOWER && value > localeval) {
			//printf("\nlocal eval %i, improved by HT to %i", localeval, value); 
			localeval = value;
		}
		if (valuetype == UPPER && value < localeval) {
			localeval = value;
		}
		*/


		d = prune(max(alpha-localeval, localeval-(alpha+1)), p, delta, d, &truncationdepth, &truncationdepth2); 
		}


	
#ifdef SAFE
	/*
	if (!stmcapture)
		ifs[7]++; 
	if (localeval > alpha)
		ifs[8]++; 
	if (p->bk + p->wk == 0)
		ifs[9]++; 
		ifs[7]: 369847505
ifs[8]: 453057992
ifs[9]: 382939399
*/

	if((!stmcapture) & (localeval > alpha) & ((p->bk+p->wk)==0))
		safemovenum = safemoves(p);
#endif // SAFE

	// futility pruning 
	// if we are far above alpha and think there is no reason to be worried, return localeval
#ifdef FUTILITY
	// todo: move less likely ifs forward (at least the captures in front of the ispvnode...)
	// try to understand this code: it's not a pv node, there is no capture for either side, side to move has enough safe moves
	// depth remaining is < 10 ply, and local evaluation is 40 above alpha, respectively 40 + depth*2 > alpha then we stop.
	// with a depth of 2 ply remaining, that gives 2*FRAC = 8 *2 = 16 so total 56 above alpha then we cut. That seems kind
	// of optimisitc

	// TODO take this if and the next if together - they both need !stmcapture && safemovnum > SAFEMOVENUM
	// TODO: !iid should also be here, right? because the point of having iid is to generate a best move, and if
	// we return here we don't get a best move
	/*
	if (!iid)
		ifs[10]++; 
	if (!ispvnode)
		ifs[11]++; 
	if (!stmcapture)
		ifs[12]++; 
	if (!sntmcapture)
		ifs[13]++; 
	if (safemovenum > SAFEMOVENUM)
		ifs[14]++; 
	if (originaldepth < 10 * FRAC)
		ifs[15]++; 
	if (localeval - 40 - (max(originaldepth, 0)) * 2 > alpha)
		ifs[16]++; 
	if (localeval != MATE)
		ifs[17]++; 
		ifs[10]: 648756494
		ifs[11]: 648698573
		ifs[12]: 369847505
		ifs[13]: 588609816
		ifs[14]: 642778444
		ifs[15]: 648436520
		ifs[16]: 361701578
		ifs[17]: 355706825
*/

	//if (!iid && !ispvnode && !stmcapture && !sntmcapture && safemovenum > SAFEMOVENUM && originaldepth < 10 * FRAC &&
	//	(localeval - 40 - (max(originaldepth, 0)) * 2 > alpha) && localeval != MATE) {
	if (!stmcapture && localeval != MATE && !sntmcapture  && !iid && !ispvnode && safemovenum > SAFEMOVENUM && originaldepth < 10 * FRAC &&
		(localeval - 40 - (max(originaldepth, 0)) * 2 > alpha)) {
			//futile = 1; 
		//printf("!"); 
		return localeval;
	}
#endif

	
	//---------------------------------------------------//
	// check if we can return the evaluation             //
	// depth must be <=0, and sidetomove has no capture  //
	//---------------------------------------------------//

	/*
	if (d <= 0)
		ifs[18]++; 
	if (!stmcapture)
		ifs[19]++; 
	if (safemovenum > SAFEMOVENUM)
		ifs[20]++; 
		ifs[18]: 427980362
ifs[19]: 311624538
ifs[20]: 584555477
*/
	if((d<=0) & (!stmcapture) & (!iid) & (safemovenum >SAFEMOVENUM)) // // TODO: safemovenum computation has changed, need to revisit "2"
		{
		// normally, we'd be stopping here, but if the side to move "has something", we shouldn't do this
		if(!sntmcapture)
			{
			// no capture in this position - return
			// first, do some statistics:
			if(si->realdepth > si->maxdepth) 
				si->maxdepth = si->realdepth;
			si->leaf++;
			si->leafdepth += si->realdepth;
			
			// localeval is initialized to MATE; if we already computed it, we don't have to again:
#ifndef QSEARCH
			if(localeval != MATE)
				return localeval;
			else
				return evaluation(p,&(si->matcount),alpha,&delta,0,maxNdb);
#else
			// optimize a bit
			if(localeval != MATE && ((localeval > alpha) || (localeval < alpha-QSEARCHLEVEL)))
				return localeval;
			else
				{
				if(localeval == MATE) { // eval not yet computed
					localeval = evaluation(p,&(si->matcount),alpha,&delta,0,maxNdb);
					if((localeval > alpha) || (localeval < alpha-QSEARCHLEVEL))
						return localeval;
					}
				return max(localeval, qsearch(si, p, alpha, alpha+1, 0));
				}
#endif
			}
		else
			{
			// returning a value with sntmcapture is risky, but probably good.
			// however, if this position is in the database, we don't want to 
			// do this.
			if(!isdbpos)
				{
				if(localeval != MATE)
					value = localeval;
				else
					value = evaluation(p,&(si->matcount),alpha,&delta,1,maxNdb);
				// side not to move has a capture. what does this mean? it means that
				// possibly the value of this position will drop dramatically. therefore
				// this should be done asymmetrically: if value is already low, don't worry
				// too much. but if value is high, think about it again! value > localbeta+X*QLEVEL maybe?
				// QLEVEL:200, QLEVEL2:60. 
				// this takes care of this, i hope. at about 5% price opening & midgame, 2% endgame.
				// compared to 60/60
				if(value > alpha+1+QLEVEL || value < alpha-QLEVEL2)
					{
					si->leaf++;
					si->leafdepth += si->realdepth;
					return value;
					}
				}
			}
		}


	// we already made a capture list - if we have moves from there, we don't need to make a movelist.
	if(!n)
		n = makemovelist(si, p, movelist, values, forcefirst, *protokiller);
		//n = makemovelist(si, p, movelist, values, forcefirst, killer1[si->realdepth]);

	if(n==0)  // this can happen if side to move is stalemated.
		return evaluation_nomove(si->realdepth); 

	// check for single move and extend appropriately 
	// TODO: move this up to extension decisions? or not, because we have to wait
	// for makemovelist? but isn't this here rather about extending captures?
	// and do we really want to extend all captures? shouldn't we just be extending
	// "sensible" captures, for instance, find "recaptures" and extend them by a full ply?
	
#ifndef NEW_SINGLEEXTEND
	// original
	if(n==1)
		d += SINGLEEXTEND;
#else
	// new version
	// TODO: a non-capture single move gets a 1/2 ply extension, whereas a recapture gets a 1/2 ply extension too
	// these should be tested separately
	if(n==1) {
		if (!iscapture[si->realdepth])
			d += SINGLEEXTEND;	
		else if (iscapture[si->realdepth - 1])
			d += SINGLEEXTEND; 
		} 

	
#endif

#ifdef PROMOTEEXTEND
	if (ispromote[si->realdepth - 1]) {
		if (p->color == BLACK) {
			if (si->matcount.wk == 1) {
				if (max(alpha - localeval, localeval - (alpha + 1)) < 100) {
					d += SINGLEEXTEND;
					//printboard(p);
					//printf("\n white just promoted a first king and the position is interesting");
					//getch();
				}
			}
		}
		else // color == white  
		{
			if (si->matcount.bk == 1) {
				if (max(alpha - localeval, localeval - (alpha + 1)) < 100) {
					d += SINGLEEXTEND;
					//printboard(p);
					//printf("\n black just promoted a first king and the position is interesting");
					//getch();
				}
			}
		}
	}
#endif

	
#ifdef ETC
	if(d>ETCDEPTH)
		{
		bestETCvalue = ETClookup(si, p, movelist, d, n, &i);
		if(bestETCvalue > alpha) 
			{
			*bestmoveindex = i;
			// TODO: should i store this in the hashtable or not?
			// who knows: perhaps this isn't really the best move, just one that was good enough
			return bestETCvalue;
			}
		}
#endif
	// save old hashkey and old material balance
	localhash = si->hash;
	Lmatcount = si->matcount;
	// for all moves: domove, update hashkey&material, recursion, restore
	//	material balance and hashkey, undomove, do alphabetatest
	// set best move in case of only fail-lows 
	for(i=0;i<n;i++)
		{
		// todo: this loop may be slow...? perhaps if a hash move is available then we know which move we want to search
		// first and we don't have to do this loop?
		index=0;
		bestmovevalue=-1;
		for(j=0; j<n; j++)
			{
			if(values[j]>bestmovevalue)
				{
				bestmovevalue = values[j];
				index = j;
				}
			}
			
		// TODO: think about swapping: put move at position n-1-i to position index
		// movelist[index] now holds the best move 
		// set values[index] to -1, that means that this move will no longer be considered 
		values[index]=-1;
		
		// domove 
		domove(q,p,movelist[index]);

		assert((p->bm& p->bk) | (p->wm & p->wk) == 0);
		
		// if we had no hashmove, we have to set bestindex on the first iteration of this loop
		
		if(i == 0)		
			bestindex = index;
			
		// inline material count 
		if(p->color == BLACK)
			{
			if(stmcapture)
				{
				si->matcount.wm -= bitcount(movelist[index].wm);
				si->matcount.wk -= bitcount(movelist[index].wk);
#ifdef NEW_NOMATERIAL_CHECK
				if (si->matcount.wm + si->matcount.wk == 0)
					return MATE - si->realdepth; 
#endif
				}
			if (movelist[index].bk && movelist[index].bm)
			{
				si->matcount.bk++; si->matcount.bm--;
				//ispromote[si->realdepth] = 1;
			}
			//else ispromote[si->realdepth] = 0; 
			}
		else
			{
			if(stmcapture)
				{
				si->matcount.bm -= bitcount(movelist[index].bm);
				si->matcount.bk -= bitcount(movelist[index].bk);
#ifdef NEW_NOMATERIAL_CHECK
				if (si->matcount.bm + si->matcount.bk == 0)
					return MATE - si->realdepth;
#endif
				}
			if (movelist[index].wk && movelist[index].wm)
			{
				si->matcount.wk++; si->matcount.wm--;
				//ispromote[si->realdepth] = 1;
			}
			//else ispromote[si->realdepth] = 0; 
			}
		//the above is equivalent to: countmaterial();

		si->realdepth++;
		// update the hash key
		updatehashkey(&movelist[index], &(si->hash));

#ifdef REPCHECK
		si->repcheck[si->realdepth + HISTORYOFFSET].hash = si->hash.key;
		si->repcheck[si->realdepth + HISTORYOFFSET].irreversible = movelist[index].bm | movelist[index].wm;
#endif
	
		/********************recursion********************/
#ifdef LATEMOVEREDUCTION
		// late move reduction: the assumption is that late moves will fail low, so we search
		// them with a bit less depth and let them fail low. if they don't fail low, then we have to research!

		// bestmovevalue takes care that hash and killer move are never reduced
		// testcapture (&q) makes sure that moves leading to captures are not reduced, but is this sensible?

		// v 185 b that was good hat LATEMOVEDEPTH 2, LATEMOVEMINDEPTH 2*FRAC, i<1 and testcapture

		//if (d <= LATEMOVEMINDEPTH || bestmovevalue >= 128 || i<3 /*||  testcapture(&q)*/)
#ifdef LMR_AGGRESSIVE
		if (d <= LATEMOVEMINDEPTH ||  i < 2 ||  testcapture(&q) || ispvnode)
#else
		if (d <= LATEMOVEMINDEPTH || i < LATEMOVENUM || ispvnode)
#endif
			value = -negamax(si, &q,d-FRAC,-(alpha+1),&Lkiller, &forcefirst, truncationdepth,truncationdepth2,0);
		else
			// we have a candidate for reduction
			{
			//reduction = i / 2; 
			//if (reduction > 4)
			reduction = 4; 
			//if (i >= LATEMOVENUM2)
			//	reduction = 8; 
			value = -negamax(si, &q, d - FRAC - reduction, -(alpha + 1), &Lkiller, &forcefirst, truncationdepth, truncationdepth2, 0);
			/* below is 1.85e
			if(i<3*n/4)
				value = -negamax(si, &q,d-FRAC-LATEMOVEDEPTH,-(alpha+1), &Lkiller, &forcefirst, truncationdepth,truncationdepth2,0);
			else
				value = -negamax(si, &q, d - FRAC - 2*LATEMOVEDEPTH, -(alpha + 1), &Lkiller, &forcefirst, truncationdepth, truncationdepth2, 0);
			*/
			// if LMR fails high, re-search with full depth
			if (value > alpha) {
				//printboard(p);
				//printint32(movelist[index].wm | movelist[index].wk | movelist[index].bm | movelist[index].bk); 
				//printf("\nlate move fail at index %i/%i", i, n); 
				//printf("!");
				//getch(); 
				value = -negamax(si, &q, d - FRAC, -(alpha + 1), &Lkiller, &forcefirst, truncationdepth, truncationdepth2, 0);
				}
			else {
				//printboard(p); 
				//printf("\late move success at index %i/%i", i, n); 
				//printf(".");
				}
			}
#else
		value = -negamax(si, &q, d-FRAC,-(alpha+1),&Lkiller, &forcefirst, truncationdepth,truncationdepth2,0);
#endif
	
		/*************************************************/

		//----------------undo the move------------------/
		si->realdepth--;
		si->hash = localhash;
		si->matcount = Lmatcount;
		//----------------end undo move------------------/
		
		// update best value so far 
		maxvalue = max(value,maxvalue);
		// check cutoff
		if(maxvalue > alpha)
			{
			bestindex = index;
			if(i == 0)
				si->cutoffsatfirst++;
			si->cutoffs++;
			break;
			}
		} // end main recursive loop of forallmoves 
	// set forcefirst in the calling negamax to the index of the best move- used for IID only
	*bestmoveindex = bestindex;
	// save the position in the hashtable 
	// we can/should restore the depth with which negamax was originally entered 
	// since this is the depth with which it would have been compared 
#ifdef STOREEXACTDEPTH
	d = originaldepth;
#endif
	if(d>=0)
		{
#ifdef THREADSAFEHT
		EnterCriticalSection(&hash_access);
#endif	
		hashstore(si, p, maxvalue, alpha, d, &movelist[bestindex], bestindex); 
#ifdef THREADSAFEHT
		LeaveCriticalSection(&hash_access);
#endif	
		}
#ifdef MOKILLER
	// set the killer move - maybe change this: only if cutoff move or only if no capture?
	if(p->color == BLACK)
		*protokiller = movelist[bestindex].bm|movelist[bestindex].bk;
	else
		*protokiller = movelist[bestindex].wm|movelist[bestindex].wk;
	// TODO: do I ever use killer2 and killer1??
	//killer2[si->realdepth] = killer1[si->realdepth];
	//killer1[si->realdepth] = *protokiller;
#endif

	// show futility failures
	/*if (value <= alpha && futile) {
		printf("!"); 
		printboard(p);
		printf("\nthis should have been futile (alpha=%i, value = %i, localeval =%i, depth %i)", 
			alpha, maxvalue, localeval, originaldepth); 
		getch(); 
	}*/

	// return best value
	return maxvalue;
	}


int	prune(int v1, POSITION *p, int delta, int d, int *truncationdepth, int *truncationdepth2)
	{
	// v1 is the distance of local eval to window
	int v2; 
	if(selfstalemate(p))
		{
		v1 = 0;
		v2 = 0;
		d += FRAC/2;
		return d; 
		}
	// some compile options to test pruning.
#ifndef NOPRUNE
	delta = 0;
#endif
#ifdef NEVERPRUNE
	v1=0;
	v2=0;
#endif
	v2 = max(0, v1-TRUNCLEVELSOFT-delta);
	v1 = max(0, v1-TRUNCLEVELHARD-delta);
	// now, if v1 != 0 it means it was more than TRUNCLEVELHARD
	// outside the window. example: 100 outside -> 2 ply penalty - seems sound.
	// this only happens if the value is outside trunclevel.
	if(v1)
		{
		v1 += TRUNCLEVELHARD; 
		// limit maximum truncation amount to MAXTRUNC:
		v1 = min(v1, MAXTRUNC*TRUNCDIVHARD); 
		*truncationdepth += v1/TRUNCDIVHARD; 
		d-= v1/TRUNCDIVHARD;  
		}
	else //not outside window with strong truncation - reexpand and check if we also have to reexpand the weak truncation.
		{
		// reexpand
		d += *truncationdepth;
		*truncationdepth = 0;
		//new double trunc code
		if(v2)
			{
			v2 += TRUNCLEVELSOFT;
			*truncationdepth2 += v2/TRUNCDIVSOFT;
			d -= v2/TRUNCDIVSOFT; 
			}
		else
			{
			d += *truncationdepth2;
			*truncationdepth2 = 0;
			}
		// end new double trunc code
		}
	return d; 
	}


int selfstalemate(POSITION *p)
	{
	// ridiculous strangle / smother detection
	if(p->bk == WBR)
		{
		if(match1(p->bm,SQ25|SQ26|SQ27))
			{
			if(SQ28 & (p->bm|p->wm|p->wk))
				return 1;
			}
		}
	if(p->wk == BBR)
		{
		if(match1(p->wm,SQ6|SQ7|SQ8))
			{
			if(SQ5 & (p->bm|p->wm|p->bk))
				return 1;
			}
		}
	return 0;
	}

int testcapture(POSITION *p)
	{
	// testcapture returns 1 if the side to move has a capture.
	int32 black,white,free,m;
	
	black = p->bm|p->bk;
	white = p->wm|p->wk;
	free = ~(black|white);
	if (p->color == BLACK)
		{
		m = ((((black & LFJ2) << 4) & white) << 3);
		m |= (((((black & LFJ1) << 3)| ((black & RFJ2) << 5)) & white) << 4);
		m |= ((((black & RFJ1) << 4) & white) << 5);
		if(p->bk)
			{
			m |= (((((p->bk & LBJ1) >> 5)| ((p->bk & RBJ2) >> 3)) & white) >> 4);
			m |= ((((p->bk & LBJ2) >> 4) & white) >> 5);
			m |= ((((p->bk & RBJ1) >> 4) & white) >> 3);
			}
		}
	else
		{
		m = (((((white & LBJ1) >> 5) | ((white & RBJ2) >> 3) )& black) >> 4);
		m |= ((((white & LBJ2) >> 4) & black) >> 5);
		m |= ((((white & RBJ1) >> 4) & black) >> 3);
		if(p->wk)
			{
			m|=((((p->wk&LFJ2)<<4)&black)<<3);
			m|=(((((p->wk&LFJ1)<<3)| ((p->wk & RFJ2) << 5) )&black)<<4);
			m|=((((p->wk&RFJ1)<<4)&black)<<5);
			}
		}
	if(m & free)
		return 1;
	return 0;
	}


#ifdef ETC
int ETClookup(SEARCHINFO *si, POSITION *p, MOVE movelist[MAXMOVES], int d, int n, int *bestindex)
	{
	// i could also pass beta as a parameter, and return the best value/best move index
	// immediately once i reach beta (ETCvalue >= localbeta from negamax). however, that
	// doesn't seem to be good: searching all moves is not a big waste of time, and perhaps i get a better 
	// bound/move.

	int i;
	HASH localhash = si->hash;
	int ETCvalue, ETCvaluetype,ETCdummy,bestETCvalue=-MATE;
	int dummy;
	int color = p->color ^CC;

	*bestindex = -1;

#ifdef THREADSAFEHT
	EnterCriticalSection(&hash_access);
#endif	

	for(i=0;i<n;i++)
		{
		// update hashkey - we don't need to actually execute the move! 
		updatehashkey(movelist, &(si->hash) ); 
		movelist++;

		// do the ETC lookup, with reduced depth and changed color 
		if(hashlookup(si, &ETCvalue,&ETCvaluetype,d-FRAC,&ETCdummy,color,&dummy))
			{
			// the position we searched had sufficient depth 
			ETCvalue = -ETCvalue;  // negamax-framework! 
			
			if(ETCvaluetype != LOWER)					// this works both with mtdf (only LOWER and UPPER are possible) and window where also EXACT is possible as valuetype		
				{
				assert(ETCvaluetype == UPPER);
				// if it was an upper bound one iteration on it is a lower bound
				// on this iteration level. therefore, in these two cases ETCvalue
				// is a lower bound on the value of our position here 
				if(ETCvalue > bestETCvalue) 
					{
					bestETCvalue = ETCvalue;
					*bestindex = i;
					// here we could return if we passed beta and checked against it - see above.
					}
				}
			}
		// restore the hash key
		si->hash = localhash;
		}

#ifdef THREADSAFEHT
	LeaveCriticalSection(&hash_access);
#endif	

	return bestETCvalue;
	}
#endif

/*void printhashstats() {
	int i; 
	int deepest = 0; 
	long int depthsum = 0;
	int entries = 0;
	int age[4] = { 0,0,0,0 };
	int pvnodes; 

	for (i = 0; i < hashsize; i++) {
		if (hashtable[i].lock != 0) {
			entries++; 
			if (hashtable[i].depth > deepest)
				deepest = hashtable[i].depth;
			age[hashtable[i].age] ++;
			depthsum += hashtable[i].depth; 
			if (hashtable[i].ispvnode)
				pvnodes++; 
		}
	}
} */


void hashstore(SEARCHINFO *si, POSITION *p, int value, int alpha, int depth, MOVE *best, int32 bestindex)
	{
	// store position in hashtable
	// based on the search window, alpha&beta, the value is assigned a valuetype
	// UPPER or LOWER (no EXACT because of MTD(f)). the best move is stored in a reduced form 
	
	int32 index,minindex;
	int mindepth = 10000,iter=0;
	int from,to;
	int add; 
	//int increment = 1; 

	//si->hashstores += 1; 
	if (value > alpha) {
		si->hashstores += (depth); // +FRAC);
#ifdef MOHISTORY
	// update history table 
		if (p->color == BLACK)
		{
			if ((best->bm | best->bk) != 0) {
				//from = (best->bm | best->bk) & (p->bm | p->bk);    // bit set on square from 
				//to = (best->bm | best->bk) & (~(p->bm | p->bk));
				//si->history[LSB(from)][LSB(to)] += 1;
				_BitScanForward(&from, (best->bm | best->bk) & (p->bm | p->bk));
				_BitScanForward(&to, (best->bm | best->bk) & (~(p->bm | p->bk)));
				si->history[(from)][(to)] += (depth); // +FRAC);	// candidate for faster LSB
				//si->history[LSB(from)][LSB(to)] += (depth); // +FRAC);	// candidate for faster LSB
			}
		}
		else
		{
			if ((best->wm | best->wk) != 0) {
				//from = (best->wm | best->wk) & (p->wm | p->wk);
				//to = (best->wm | best->wk) & (~(p->wm | p->wk));
				//si->history[LSB(from)][LSB(to)] += 1;
				//si->history[LSB(from)][LSB(to)] += (depth); // +FRAC);	// candidate for faster LSB
				_BitScanForward(&from, (best->wm | best->wk) & (p->wm | p->wk));
				_BitScanForward(&to, (best->wm | best->wk) & (~(p->wm | p->wk)));
				si->history[(from)][(to)] += (depth); // +FRAC);	// candidate for faster LSB

			}
		}
#endif
	}
	index = si->hash.key & (hashsize-1);

	// new alignment of hashtable index to 4-boundary
	index = index & (0xFFFFFFFC);

	minindex = index;

	while(iter<HASHITER)
		{
		if(hashtable[index].lock == si->hash.lock || hashtable[index].lock==0) // vtune: use | instead of ||
			// found an index where we can write the entry 
			{
			hashtable[index].lock = si->hash.lock;
			hashtable[index].depth =(int16) (depth);
			hashtable[index].best = bestindex;
			hashtable[index].color = (p->color>>1);
			hashtable[index].value =( sint16)value;
			// determine valuetype 
			if(value > alpha) 
				hashtable[index].valuetype = LOWER;
			else
				hashtable[index].valuetype = UPPER;

			return;
			}
		else
			{
			// have to overwrite 
			// alternative:
			//printf("si->age is %i, ht %i", si->age, hashtable[index].age); 
			add = 4 - ((si->age + HASHITER - hashtable[index].age) % HASHITER);
			add *= (120 * FRAC);

			// very first search: entire hashtable has age 0 (everything set to 0), but search has age 1.
			// add = 4 - (( 1 + 4 - 0) %4 ) = 4 - 1%4 = 3.
			// so add should end up being 300*FRAC. aha and that is 1200 and that is bigger than mindepth, so it 
			// overwrites its own searches instead of empty entries, how dumb...

			// e.g. if si->age is 3 and hashtable.age is 2, then this give 7-2=5 % 4 = 1
			// if si->age is 3 and hashtable.age is 0, this gives 3
			// if si->age is 0 and hashtable.age is 2, this gives 2
			// the larger the difference is, the rather you want to overwrite
			// add = (120-40*add*FRAC)* calc above

			
			/*if (si->age == hashtable[index].age)	// current search, pretend it is more important
				add = 120*FRAC;
			else 
				add = 0;*/ 
			
			//printf("%i ", add); 
			
			if((int) hashtable[index].depth+add < mindepth)
				{
				minindex = index;
				mindepth = hashtable[index].depth + add;
				}
			}
		iter++;
		index++;
		}
		// if we arrive here it means we have gone through all hashiter
		//entries and all were occupied. in this case, we write the entry	to minindex 

	// if alwaysstore is not defined, and we have found no entry with equal or
	// lower importance, we don't overwrite it.
#ifndef ALWAYSSTORE
	if(mindepth>(depth)) 
		return;
#endif

	hashtable[minindex].lock = si->hash.lock;
	hashtable[minindex].depth = depth;
	hashtable[minindex].best = bestindex;
	hashtable[minindex].color = (p->color>>1);
	hashtable[minindex].value = value;
	hashtable[minindex].age = si->age; 
	hashtable[minindex].ispvnode = 0; // new 26.1.2020!
	/* determine valuetype */
	if(value > alpha)
		hashtable[minindex].valuetype = LOWER;
	else
		hashtable[minindex].valuetype = UPPER;
	return;
	}

int hashlookup(SEARCHINFO *si, int *value, int *valuetype, int depth, int32 *forcefirst, int color, int *ispvnode)
	{
	/* searches for a position in the hashtable.
	if the position is found and
	if (the stored depth is >= depth to search),
	hashlookup returns 1, indicating that the value and valuetype have
	useful information.	else,
	forcefirst is set to the best move found previously, and 0 returned
	if the position is not found at all, 0 is returned and forcefirst is
	left unchanged (at MAXMOVES-1). */

	int32 index;
	int iter=0;

	index = si->hash.key & (hashsize-1); // expects that hashsize is a power of 2!
	index = index & (0xFFFFFFFC);

	while(iter<HASHITER && hashtable[index].lock)  // if hashtable[index].lock is 0 then nothing is here.
		{
		if(hashtable[index].lock == si->hash.lock && ((int)hashtable[index].color==(color>>1)))
			{
			// we have found the position 
			*ispvnode=hashtable[index].ispvnode;
			
			// move ordering 
			*forcefirst=hashtable[index].best;

			// value bounds

			//*value = hashtable[index].value;
			//*valuetype = hashtable[index].valuetype;

			// use value if depth in hashtable >= current depth
			if ((int)hashtable[index].depth >= depth)
			{
				*value=hashtable[index].value;
				*valuetype=hashtable[index].valuetype;
				return 1;
			}
			else
				return 0; 
			}
		iter++;
		index++;
		}
	return 0;
	}

void updatehashkey(MOVE *m, HASH *h)
	{
	// given a move m, updatehashkey updates the HASH structure h
	int32 x,y;
	x=m->bm;
	while(x)
		{
		_BitScanForward(&y, x); 
		h->key ^=hashxors[0][0][y];
		h->lock^=hashxors[1][0][y];
		x&=(x-1);
		}
	x=m->bk;
	while(x)
		{
		_BitScanForward(&y, x);
		h->key ^=hashxors[0][1][y];
		h->lock^=hashxors[1][1][y];
		x&=(x-1);
		}
	x=m->wm;
	while(x)
		{
		_BitScanForward(&y, x);
		h->key ^=hashxors[0][2][y];
		h->lock^=hashxors[1][2][y];
		x&=(x-1);
		}
	x=m->wk;
	while(x)
		{
		_BitScanForward(&y, x);
		h->key ^=hashxors[0][3][y];
		h->lock^=hashxors[1][3][y];
		x&=(x-1);
		}
	}


int pvhashlookup(SEARCHINFO *si, int *value, int *valuetype, int depth, int32 *forcefirst, int color, int *ispvnode)
	{
	// pvhashlookup is like hashlookup,except that it marks the entry in the hashtable as being part of the PV
	int32 index;
	int iter=0;

	index = si->hash.key & (hashsize-1);
	index = index & (0xFFFFFFFC);

#ifdef THREADSAFEHT
	EnterCriticalSection(&hash_access);
#endif
	while(iter<HASHITER && hashtable[index].lock) 
		{
		if(hashtable[index].lock == si->hash.lock && ((int)hashtable[index].color==(color>>1)))
			{
			// here's the only difference to the normal hashlookup!
			hashtable[index].ispvnode=1;
			// move ordering 
			*forcefirst = hashtable[index].best;
			// use value if depth in hashtable >= current depth)
			if((int)hashtable[index].depth>=depth)
				{
				*value=hashtable[index].value;
				*valuetype=hashtable[index].valuetype;
#ifdef THREADSAFEHT
				LeaveCriticalSection(&hash_access);
#endif				
				return 1;
				}
			}
		iter++;
		index++;
		}
#ifdef THREADSAFEHT
	LeaveCriticalSection(&hash_access);
#endif
	return 0;
	}


void getpv(SEARCHINFO *si, POSITION *p, char *str)
	{
	//----------------------------------------------------------------------
	// retrieves the principal variation from the hashtable:				|
	// looks up the position, finds the hashtable move, plays it, looks     |
	// up the position again etc.                                           |
	// color is the side to move, the whole PV gets written in *str         |
	// getpv also marks pv nodes in the hashtable, so they won't be pruned! |
	//----------------------------------------------------------------------

	MOVE movelist[MAXMOVES];
	int32 forcefirst;
	int dummy=0;
	int i,n;
	char Lstr[1024];
	POSITION Lp;
	int values[MAXMOVES];

	Lp=*p;
	absolutehashkey(&Lp, &(si->hash));
	sprintf(str,"pv ");
	for(i=0;i<MAXDEPTH;i++)
		{
		forcefirst=100;
		// pvhashlookup also stores the fact that these nodes are pv nodes in the hashtable
		pvhashlookup(si, &dummy,&dummy,0, &forcefirst,Lp.color,&dummy);
		if(forcefirst==100)
			break;
		
		// TODO: replace below with generatemovelist?
		n = makecapturelist(&Lp,movelist,values,forcefirst);
		if(!n)
			n = makemovelist(si, &Lp, movelist, values, forcefirst, 0);
		if(!n) 
			{
			absolutehashkey(p,&(si->hash));
			return;
			}
	
		if(i<MAXPV)
			{
#ifdef FULLLOG
			staticeval = evaluation(&Lp,&(si->matcount),0,&dummy,0,maxNdb);
			sprintf(Lstr,"[%i]",staticeval);
			strcat(str,Lstr);
#endif // FULLLOG
			movetonotation(&Lp,&movelist[forcefirst],Lstr);
			strcat(str,Lstr);
			strcat(str," ");
			}
		
		togglemove((&Lp),movelist[forcefirst]);
		countmaterial(&Lp, &(si->matcount));
		absolutehashkey(&Lp,&(si->hash));
		}
	
	absolutehashkey(p,&(si->hash));
	countmaterial(p, &(si->matcount));
	return;
	}


void absolutehashkey(POSITION *p, HASH *hash)
	{
	/* absolutehashkey calculates the hashkey completely. slower than
		using updatehashkey, but useful sometimes */
	int32 x;
	hash->lock=0;
	hash->key=0;
	x=p->bm;
	while(x) {
		hash->key ^=hashxors[0][0][LSB(x)];
		hash->lock^=hashxors[1][0][LSB(x)];
		x&=(x-1);
		}
	x=p->bk;
	while(x) {
		hash->key ^=hashxors[0][1][LSB(x)];
		hash->lock^=hashxors[1][1][LSB(x)];
		x&=(x-1);
		}
	x=p->wm; 
	while(x) {
		hash->key ^=hashxors[0][2][LSB(x)];
		hash->lock^=hashxors[1][2][LSB(x)];
		x&=(x-1);
		}
	x=p->wk;
	while(x) {
		hash->key ^=hashxors[0][3][LSB(x)];
		hash->lock^=hashxors[1][3][LSB(x)];
		x&=(x-1);
		}
	}

void countmaterial(POSITION *p, MATERIALCOUNT *m)
	{
	m->bm = bitcount(p->bm);
	m->bk = bitcount(p->bk);
	m->wm = bitcount(p->wm);
	m->wk = bitcount(p->wk);
	}

int perft(SEARCHINFO *si, POSITION *p, int depth)
	{
	int i;
	char str[256];
	double t = clock(); 
	
	//logtofile("");
	//logtofile("perft");
	for(i=1; i<=depth; i++)
		{
		resetsearchinfo(si);
		si->start = clock(); 
		perftrec(si, p, i);
		t = clock(); 
		t = (t-si->start)/CLK_TCK; 
		sprintf(str, "depth %i positions %i  time %f   %.1f kN/s", i, si->negamax, t, si->negamax/(t*1000));
		//logtofile(str);
		}
	return si->negamax; 
	}

int perftrec(SEARCHINFO *si, POSITION *p, int depth)
	{
	MOVE movelist[MAXMOVES];
	int values[MAXMOVES];
	int stmcapture; 
	int i,n; 
	POSITION q;
	
	// get info on captures: can side to move or side not to move capture now?
	stmcapture = testcapture(p);
	if(stmcapture)
		n = makecapturelist(p,movelist,values,0);
    else
		n = makemovelist(si, p, movelist, values, 0, 0);

	if(depth <= 1)
		{
		si->negamax += n;
		return 0;
		}

	for(i=0; i<n; i++)
		{
		domove(q,p,movelist[i]);
		perftrec(si, &q, depth-1);
		}
	return 0;
	}

/*
 *		cake - a checkers engine			
 *															
 *		Copyright (C) 2000...2007 by Martin Fierz	
 *														
 *		contact: checkers@fierz.ch				
 */


#include "switches.h"
#include <stdio.h>
#include <stdlib.h> // malloc() 
#include <string.h> // memset() 
#include <time.h>
#include <assert.h>

#include <conio.h>
#include <windows.h>

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


//-------------------------------------------------------------------------------//
// globals below here are shared - even with these, cake *should* be thread-safe //
//-------------------------------------------------------------------------------//

static int iscapture[MAXDEPTH];		// tells whether move at realdepth was a capture

int hashmegabytes = 64;				// default hashtable size in MB if no value in registry is found
int dbmegabytes = 128;				// default db cache size in MB if no value in registry is found
int usethebook = 3;					// default: use best moves if no value in the registry is found

static HASHENTRY *hashtable;		// pointer to the hashtable, is allocated at startup
static HASHENTRY *book;				// pointer to the book hashtable, is allocated at startup

int hashsize = HASHSIZE;

#ifdef SPA
static SPA_ENTRY *spatable;
#endif


int maxNdb=0;						// the largest number of stones which is still in the database 


static int cakeisinit = 0;			// is set to 1 after cake is initialized, i.e. initcake has been called

// history table array
#ifdef MOHISTORY
int32 history[32][32];
#endif

// hashxors array.
int32 hashxors[2][4][32];			// this is initialized to constant hashxors stored in the code.

// bit tables for  number of set bits in word 
static unsigned char bitsinword[65536];
static unsigned char bitsinbyte[256];
static unsigned char LSBarray[256];

static int   norefresh;

int bookentries = 0; // number of entries in book hashtable
int bookmovenum = 0; // number of used entries in book

#ifdef HEURISTICDBSAVE
FILE *fp_9pc_black, *fp_9pc_white;
FILE *fp_10pc_black, *fp_10pc_white;
FILE *fp_11pc_black, *fp_11pc_white;
FILE *fp_12pc_black, *fp_12pc_white;
FILE *fp_9pc_blackK, *fp_9pc_whiteK;
FILE *fp_10pc_blackK, *fp_10pc_whiteK;
FILE *fp_11pc_blackK, *fp_11pc_whiteK;
FILE *fp_12pc_blackK, *fp_12pc_whiteK;
#endif

#ifdef THREADSAFEHT
CRITICAL_SECTION hash_access;		// is locked if a thread is accessing the hashtable
#endif
#ifdef THREADSAFEDB
CRITICAL_SECTION db_access;			// is locked if a thread is accessing the database
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
	char dirname[256];
	FILE *fp;

	// create a new logfile / delete old logfile 
	sprintf(str,"creating logfile");

	// delete old logfile
	fp = fopen("cakelog.txt","w");
	fclose(fp);
		
	GetCurrentDirectory(256, dirname);
	logtofile(dirname);
	
	// do some loads. first, the book.



#ifdef USEDB
	sprintf(str, "initializing database");
	maxNdb = db_init(dbmegabytes,str);
#endif // USEDB

#ifdef BOOK
	// next, load the book 
	sprintf(str,"loading book...");
	book = loadbook(&bookentries, &bookmovenum);
#endif

	// allocate hashtable
	sprintf(str,"allocating hashtable...");
	hashtable = inithashtable(hashsize);
	
	// initialize xors 
	initxors((int*)hashxors);
	
	// initialize bit-lookup-table 
	initbitoperations(bitsinword, LSBarray);
	
	initeval();


#ifdef SPA
	// allocate memory for SPA
	initspa();
#endif

#ifdef BOOKHT
	initbookht();
#endif

#ifdef THREADSAFEHT
	InitializeCriticalSection(&hash_access);
#endif
#ifdef THREADSAFEDB
	InitializeCriticalSection(&db_access);
#endif 

	cakeisinit=1;
	return 1;
	}

void resetsearchinfo(SEARCHINFO *s)
	{
	// resets the search info structure nearly completely, with the exception of
	// the pointer si.repcheck, to which we allocated memory during initialization - 
	// we don't want to create a memory leak here.
	s->aborttime = 0;
	s->allscores = 0;
	s->bk = 0;
	s->bm = 0;
	s->cutoffs = 0;
	s->cutoffsatfirst = 0;
	s->dblookup = 0;
	s->dblookupfail = 0;
	s->dblookupsuccess = 0;
	s->Gbestindex = 0;
	s->hash.key = 0;
	s->hash.lock = 0;
	s->hashlookup = 0;
	s->hashlookupsuccess = 0;
	s->hashstores = 0;
	s->iidnegamax = 0;
	s->leaf = 0;
	s->leafdepth = 0;
	s->matcount.bk = 0;
	s->matcount.bm = 0;
	s->matcount.wk = 0;
	s->matcount.wm = 0;
	s->maxdepth = 0;
	s->maxtime = 0;
	s->negamax = 0;
	s->out = NULL;
	s->play = NULL;
	s->qsearch = 0;
	s->qsearchfail = 0;
	s->qsearchsuccess = 0;
	s->realdepth = 0;
	s->searchmode = 0;
	s->spalookups = 0;
	s->spasuccess = 0;
	s->start = 0;
	s->wk = 0;
	s->wm = 0;
	}


int hashreallocate(int x)
	{
	// TODO: move to little-used functions file
	// reallocates the hashtable to x MB
	
	static int currenthashsize;

	HASHENTRY *pointer;
	int newsize;

	hashmegabytes = x;
	newsize = (hashmegabytes)*1024*1024/sizeof(HASHENTRY);	

	// TODO: this must be coerced to a power of 2. luckily this is true
	// with sizeof(HASHENTRY) == 8 bytes, but if i ever change that it will
	// fail

#ifdef WINMEM
	VirtualFree(hashtable, 0, MEM_RELEASE);
	pointer = VirtualAlloc(0, (newsize+HASHITER)*sizeof(HASHENTRY), MEM_COMMIT, PAGE_READWRITE);
#else
	pointer = realloc(hashtable,(newsize+HASHITER)*sizeof(HASHENTRY));
#endif
	if(pointer != NULL)
		{
		hashsize = newsize;
		hashtable = pointer;
		}
	// TODO: do something if pointer == NULLbut what do we do when pointer == 0 here??
	return 1;
	}

int exitcake()
	{
	// deallocate memory 
	//free(hashtable);
	db_exit();
	//free(book);
	return 1;
	}


#ifdef BOOKGEN
int bookgen(OLDPOSITION *q, int color, int *numberofmoves, int values[MAXMOVES], int how, int depth, double searchtime)
	{
	// this function analyzes a position, and returns the number of possible moves (*numberofmoves) and
	// an array with the evaluation of each move. to speed up things, it only looks at "sensible" values,
	// i.e. if a move is 120 points worse than the best move, it stops there and doesn't determine exactly
	// how bad it is. 
	// bookgen is only used by the book generator, not in testcake or checkerboard.
	int playnow=0;
	int d;
	int value=0,lastvalue=0,n,i,j, guess;
	MOVE best,last, movelist[MAXMOVES];
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
	int index;
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
	memset(history,0,32*32*sizeof(int));
#endif

	printboard(&p);
	printf("\nthis is the new Cake bookgen function!");

	// clear the hashtable 
	memset(hashtable,0,(hashsize+HASHITER)*sizeof(HASHENTRY));
	
	norefresh=0;

	n = makecapturelist(&p, movelist, values, 0);
	if(!n)
		n = makemovelist(&si, &p, movelist, values, 0, 0);
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
		printf("\n",d);

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
		logtofile(si.out);
#endif		
		}
	logtofile(si->out);
	
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
	memset(history,0,32*32*sizeof(int));
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

#ifdef PLAYNOW
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
#endif

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
	/*		if (log&1) cake will write information into "log.txt"
	/*		if(log&2) cake will also print the information to stdout.
	/*		if reset!=0 cake++ will reset hashtables and repetition checklist
	/*		reset==0 generally means that the normal course of the game was disturbed
	// info currently has uses for it's first 4 bits:
	// info&1 means reset
	// info&2 means exact time level
	// info&4 means increment time level
	// info&8 means allscore search
	/*
	/*----------------------------------------------------------------------------*/

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
	int reset=(info&1);
	int forcedmove = 0; // indicates if a move is forced, then cake++ will play at depth 1.
	int zeroes = 0;		// number of zero evals returned
	int winslosses = 0;
	double t;
	char pvstring[1024];
	MOVE movelist[MAXMOVES];	// we make a movelist here which we pass on, so that it can be ordered
								// and remain so during iterative deeping.
	
	
//	int dolearn;

	// initialize module if necessary
	if(!cakeisinit) 
		initcake(str);

	resetsearchinfo(si);

	for(i=0;i<MAXDEPTH;i++)
		iscapture[i] = 0;


	*playnow = 0;
	si->play = playnow;
	si->out = str;
	si->searchmode = how;

	// set time limits
	si->maxtime = maximaltime;
	si->aborttime = 4*maximaltime;

	// if exact:
	if(info&2)
		si->aborttime = maximaltime;

	// allscores:
	if(info&8)
		si->allscores = 1;

	printboardtofile(p,NULL);
	
	// print current directory to see whether CB is getting confused at some point.
	GetCurrentDirectory(256, pvstring);
	
	// initialize material 
	countmaterial(p, &(si->matcount));

#ifdef MOHISTORY
	// reset history table 
	memset(history,0,32*32*sizeof(int));
#endif

	// clear the hashtable 
	memset(hashtable,0,(hashsize+HASHITER)*sizeof(HASHENTRY));

	// initialize hash key 
	absolutehashkey(p, &(si->hash));
	
	// if LEARNUSE is defined, we stuff learned positions in the hashtable
#ifdef LEARNUSE
	stufflearnpositions();
#endif

	// what is this doing at all?
	norefresh=0;

	// what is this doing here?
	n = makecapturelist(p, movelist, values, 0);

#ifdef REPCHECK
	// initialize history list: holds the last few positions of the current game 
	
	si->repcheck[HISTORYOFFSET].hash = si->hash.key;
	si->repcheck[HISTORYOFFSET].irreversible = 1;
	
	dummy.hash = 0;
	dummy.irreversible = 1;

	if(reset==0)
		{
		for(i=0;i<HISTORYOFFSET-2;i++)
			si->repcheck[i] = si->repcheck[i+2];
		}
	else
		{
		for(i=0;i<HISTORYOFFSET;i++)
			si->repcheck[i] = dummy;
		}
	//repetitiondraw = 0;
#endif

#ifdef TESTPERF
	for(i=0;i<MAXMOVES;i++)
		cutoffsat[i]=0;
#endif
	
	
	// do a book lookup TODO: put this all in a small function
	if(usethebook)
		{
		bookfound=0;
		bookindex=0;
		
		if(booklookup(p,&booklookupvalue,0,&bookdepth,&bookindex,str))
			{
			// booklookup was successful, it sets bookindex to the index of the move in movelist that it wants to play
			bookfound = 1;
			n = makecapturelist(p, movelist, values, 0);
			if(!n)
				n = makemovelist(si, p,movelist,values,0,0);
			// set best value
			bookmove = movelist[bookindex];
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
		logtofile(str);
		}
	else
		bookfound=0;

	absolutehashkey(p, &(si->hash));
	countmaterial(p, &(si->matcount));
	
	// check if the move on the board is forced - if yes, we don't waste time on it.
	forcedmove = isforced(p);

#ifdef SOLVE
	solve(p, color);
	return 0;
#endif

	// get a movelist which we pass around to mtdf() and firstnegamax() - do this here
	// because we want to keep it ordered during iterative deepening.
	n = getorderedmovelist(p, movelist);


	si->start = clock();
	guess=0;
	
	
	if(!bookfound)
		{
		// check if this version is set to search to a fixed depth
#ifdef FIXEDDEPTH
		how = DEPTH_BASED;
		depthtosearch = FIXEDDEPTH;
#endif

		for(d=1; d<MAXDEPTH; d+=2)
			{
			si->leaf=0;
			si->leafdepth=0;

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
				guess = value;	
				}
			else
				value = allscoresearch(si, p, movelist, FRAC*d, &best);
#endif

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
			if(d>1)
				{
				if( si->searchmode == TIME_BASED && ((clock()- si->start)/CLK_TCK>(si->maxtime/2)) ) 
					break;

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
#ifdef PLAYNOW
			if(*(si->play))
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
#endif

			lastvalue=value;	// save the value for this iteration 
			last=best;			// save the best move on this iteration 
			norefresh=1;
			}
		}

	// TODO: we haven't done our move yet, and we store the repcheck thing, 
	//	but that already would seem to have been stored before!
#ifdef REPCHECK
		si->repcheck[HISTORYOFFSET-2].hash = si->hash.key;
		si->repcheck[HISTORYOFFSET-2].irreversible = 0;
#endif

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

	if(*(si->play))
		best = last;
	
	togglemove(p,best);
	
	absolutehashkey(p, &(si->hash));

#ifdef REPCHECK
	si->repcheck[HISTORYOFFSET-1].hash = si->hash.key;
	si->repcheck[HISTORYOFFSET-1].irreversible = (best.bm | best.wm);
#endif
	
	// return value: WIN / LOSS / DRAW / UNKNOWN

	if(value > WINLEVEL)
		return WIN;
	if(value < -WINLEVEL)
		return LOSS;

#ifdef USEDB
	if(isdbpos(p, &(si->matcount)))
		{
#ifdef THREADSAFEDB
		EnterCriticalSection(&db_access);
#endif
		value = dblookup(p, 0);
#ifdef THREADSAFEDB
		LeaveCriticalSection(&db_access);
#endif
		if(value == DB_DRAW)
			return DRAW;
		}
#endif

	// TODO: find repetition draws and return DRAW too.

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


#ifdef LEARNSAVE
void learn(POSITION *p, int value, int depth, int bestindex)
	{
	LEARNENTRY le;
	FILE *learnfp;
	
	// set values of LEARNENTRY structure
	le.black = p->bm|p->bk;
	le.white = p->wm|p->wk;
	le.kings = p->bk|p->wk;
	le.value = value;
	le.color = p->color>>1;
	le.depth = depth*FRAC;
	le.best = bestindex; 
	learnfp = fopen("cakeM.lrn","ab");
	fwrite(&le, sizeof(LEARNENTRY), 1, learnfp);
	fclose(learnfp);
	}
#endif

#ifdef LEARNUSE
void stufflearnpositions(void)
	{
	FILE *learnfp;
	POSITION dummy;
	LEARNENTRY le;
	int index;

		
	learnfp = fopen("cakeM.lrn", "rb");
	if(learnfp != NULL)
		{
		// read learned positions one by one 
		while(!feof(learnfp))
			{
			fread(&le, sizeof(LEARNENTRY), 1, learnfp);
			dummy.bm = le.black & (~le.kings);
			dummy.bk = le.black & le.kings;
			dummy.wm = le.white & (~le.kings);
			dummy.wk = le.white & le.kings;
			absolutehashkey(&dummy);

			if(abs(le.value)<400)
				{
				// and stuff into hashtable - only if the value is small; that is to defend
				// against old learn entries that were saved in earlier versions of cake ss.
				index = Gkey & (hashsize-1);
				hashtable[index].best = le.best;
				hashtable[index].color = le.color;
				hashtable[index].depth = le.depth;
				hashtable[index].ispvnode = 0;
				hashtable[index].lock = Glock;
				hashtable[index].value = le.value;
				hashtable[index].valuetype = EXACT;
				}
			}
		fclose(learnfp);
		}
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
	double time;
	char Lstr1[1024]="",Lstr2[1024]="";
	
	g = firstguess;
	/*g=0; */ /* strange - this seems to help?! */
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
			upperbound=g;
			//upperbound = beta-1;
			sprintf(Lstr1,"value<%i",beta);
			}
		else
			{
			lowerbound=g;
			//lowerbound = beta;
			sprintf(Lstr1,"value>%i",beta-1);
			}
		
		time = ( (clock() - si->start)/CLK_TCK);
		getpv(si, p, Lstr2);

		searchinfotostring(si->out, depth, time, Lstr1, Lstr2, si);
#ifdef FULLLOG
		sprintf(Lstr,"\n -> %s",si.out);
		logtofile(Lstr);
#endif	
		}
	// for testing:
	//g = lowerbound;
	sprintf(Lstr1,"value=%i",g);
	searchinfotostring(si->out, depth, time, Lstr1, Lstr2, si);

	logtofile(si->out);
	return g;
	}
	
int windowsearch(SEARCHINFO *si, POSITION *p, MOVE movelist[MAXMOVES], int depth, int guess, MOVE *best)
	{
	int value;
	double time;
	char Lstr1[1024],Lstr2[1024];

	//do a search with aspiration window
	
	value = firstnegamax(si, p, movelist, depth, guess-ASPIRATIONWINDOW, guess+ASPIRATIONWINDOW, best);
	
	if(value >= guess+ASPIRATIONWINDOW)
		{
		// print info in status bar and cakelog.txt
		sprintf(Lstr1,"value>%i",value-1);
		time = ( (clock()- si->start)/CLK_TCK);
		getpv(si, p, Lstr2);
		searchinfotostring(si->out, depth, time, Lstr1, Lstr2, si);
		logtofile(si->out);
	
		value = firstnegamax(si, p, movelist, depth, guess, MATE, best);
		
		if(value <= guess)
			{
			// print info in status bar and cakelog.txt
			sprintf(Lstr1,"value<%i",value+1);
			time = ( (clock()- si->start)/CLK_TCK);
			getpv(si, p, Lstr2);
			searchinfotostring(si->out, depth, time, Lstr1, Lstr2, si);
			logtofile(si->out);

			value = firstnegamax(si, p, movelist, depth, -MATE, MATE, best);
			}
		}

	if(value <= guess-ASPIRATIONWINDOW)
		{
		// print info in status bar and cakelog.txt
		sprintf(Lstr1,"value<%i",value+1);
		time = ( (clock()- si->start)/CLK_TCK);
		getpv(si, p, Lstr2);
		searchinfotostring(si->out, depth, time, Lstr1, Lstr2, si);
		logtofile(si->out);

		value = firstnegamax(si, p, movelist, depth, -MATE, guess, best);
		
		if(value >= guess)
			{
			// print info in status bar and cakelog.txt
			sprintf(Lstr1,"value>%i",value-1);
			time = ( (clock()- si->start)/CLK_TCK);
			getpv(si, p, Lstr2);
			searchinfotostring(si->out, depth, time, Lstr1, Lstr2, si);
			logtofile(si->out);

			value = firstnegamax(si, p, movelist, depth, -MATE, MATE, best);
			}
		}
	sprintf(Lstr1,"value=%i",value);
	time = ( (clock()- si->start)/CLK_TCK);
	getpv(si, p, Lstr2);
	searchinfotostring(si->out, depth, time, Lstr1, Lstr2, si);
	logtofile(si->out);
	
	return value;
	}


int allscoresearch(SEARCHINFO *si, POSITION *p, MOVE movelist[MAXMOVES], int d, MOVE *best)
	{
	int i, j, n;
	static int values[MAXMOVES];
	int bestindex = 0;
	char str[256], tmpstr[256], movestr[256];
	int Lkiller;
	int tmpvalue;
	MOVE tmpmove;
	MATERIALCOUNT Lmatcount;
	HASH localhash;
	

#ifdef PLAYNOW
	if(*(si->play)) return 0;
#endif
	
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

#ifdef REPCHECK
		si->repcheck[si->realdepth + HISTORYOFFSET].hash = si->hash.key;
		si->repcheck[si->realdepth + HISTORYOFFSET].irreversible = movelist[i].bm|movelist[i].wm;
#endif
		
		/********************recursion********************/

		// TODO: this will not work any more!
		values[i] = -negamax(si,p,d-FRAC,0, &Lkiller, &Lkiller, 0,0,0);

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

	logtofile(str);

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

//	last = *p;
	return 1;
	}



int perft(SEARCHINFO *si, POSITION *p, int depth)
	{
	int i;
	char str[256];
	double t; 
	t = clock();
	
	logtofile("");
	logtofile("perft");
	sprintf(str, "perft %i", depth);
	logtofile(str);

	for(i=1; i<=depth; i++)
		{
		resetsearchinfo(si);
		si->start = clock(); 
		perftrec(si, p, i);
		t = clock(); 
		t = (t-si->start)/CLK_TCK; 
		sprintf(str, "depth %i positions %i  time %f   %.1f kN/s", i, si->negamax, t, si->negamax/(t*1000));
		logtofile(str);
		//sprintf(si->out, "%s", str);
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

	//si->negamax++;

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
		// domove 
		domove(q,p,movelist[i]);
		q.color = p->color^CC;
		perftrec(si, &q, depth-1);
		}
	return 0;
	}



int firstnegamax(SEARCHINFO *si, POSITION *p, MOVE movelist[MAXMOVES], int d, int alpha, int beta, MOVE *best)
	{

	/*--------------------------------------------------------------------------
	|																			|
	|		firstnegamax: first instance of negamax which returns a move		|
	|																			|
	 --------------------------------------------------------------------------*/
	
	int i,value,swap=0,bestvalue=-MATE;
	static int n;
	static MOVE ml2[MAXMOVES];
	MATERIALCOUNT Lmatcount;
	int Lalpha=alpha;
	int32 forcefirst=0;
	int32 Lkiller=0;
	static POSITION last;
	MOVE tmpmove;
	static int values[MAXMOVES];	/* holds the values of the respective moves - use to order */
	int refnodes[MAXMOVES];			/* number of nodes searched to refute a move */
	int statvalues[MAXMOVES];
	int tmpnodes;
	int bestindex=0;
	HASH localhash;

#ifdef PLAYNOW
	if(*(si->play)) return 0;
#endif
	si->negamax++;

	// TODO: will static position last work here? and static movelist??
	// probably i need to make a movelist in mtdf and send it to firstnegamax!

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
		si->realdepth++;
		updatehashkey(&movelist[i], &(si->hash));
#ifdef REPCHECK
		si->repcheck[si->realdepth + HISTORYOFFSET].hash = si->hash.key;
		si->repcheck[si->realdepth + HISTORYOFFSET].irreversible = movelist[i].bm|movelist[i].wm;
#endif
		tmpnodes = si->negamax;
		/********************recursion********************/
		
		value = -negamax(si, p,d-FRAC,-beta, &Lkiller, &forcefirst, 0,0,0);

		/******************* end recursion ***************/
		refnodes[i] = si->negamax-tmpnodes;
		values[i] = value;
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
	last=*p;
	return bestvalue;
}



#ifdef QSEARCH
int qsearch(SEARCHINFO *si, POSITION *p, int alpha, int beta, int qsdepth)
	{
	// qsearch checks "violent" moves, i.e. all moves that lead to a possible
	// capture for side not to move. 
	// qsearch assumes that there is no capture at the moment for the side to move!

	// if we find any move with value >= beta we can return immediately.
	int i, n, value;
	int maxvalue = -MATE;
	int32 Lkiller = 0;
	HASH h; 
	MOVE movelist[MAXMOVES];
	POSITION q;	
	int forcefirst = MAXMOVES-1;
	MATERIALCOUNT mc;

	si->qsearch++;
	si->negamax++;

	// check material:
	if(p->bm + p->bk == 0)
		return (p->color == BLACK) ? (-MATE+si->realdepth):(MATE-si->realdepth);
	if(p->wm + p->wk == 0)
		return (p->color == WHITE) ? (-MATE+si->realdepth):(MATE-si->realdepth); 


	/* what does qs do?
	 * 1) generate moves that lead to a capture for the opponent
	 * 2) if no moves found, return
	 * 3) else recursion over loops to csearch()
	 * 4) return maximal value
	 */


	n = makeQSmovelist(p, movelist);
	// if we have no potential shots, return
	if(n==0)
		{
		si->qsearchfail++;
		return maxvalue;
		}
	

	// now do recursion over all moves. note that we either have a capture, then we certainly
	// have moves, or we already 

	// store old material balance and hash
	h = si->hash;
	mc = si->matcount;

	for(i=0;i<n;i++)
		{
		domove(q,p,movelist[i]);
		q.color = p->color ^CC;

		// if we arrive here, the opponent has a capture now - so
		// we search this move.
		si->realdepth++;

		// update the hash key
		updatehashkey(&(movelist[i]), &(si->hash));  

		// updatematerial 
		if(p->color == BLACK)
			{
			if(movelist[i].bk && movelist[i].bm) 
				{si->matcount.bk++; si->matcount.bm--;}
			}
		else
			{
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
		/* and set alpha and beta bounds */
		if(maxvalue > alpha)
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
	if(p->bm + p->bk == 0)
		return (p->color == BLACK) ? (-MATE+si->realdepth):(MATE-si->realdepth);
	if(p->wm + p->wk == 0)
		return (p->color == WHITE) ? (-MATE+si->realdepth):(MATE-si->realdepth); 


	/* what does cs do?
	 * 1) recursion over all capture moves
	 * 2) return the value
	 */

	// TODO: check what is faster: testcapture followed by makecapturelist or just makecapturelist

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
		q.color = p->color ^CC;

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
	// it is only called when there are no kings on the board, therfore there is
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


int negamax(SEARCHINFO *si, POSITION *p, int d, int alpha, int32 *protokiller, int *bestmoveindex, int truncationdepth, int truncationdepth2, int iid)
	/*----------------------------------------------------------------------------
	|																			  |
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
	int valuetype;
	int hashfound;
	int32 forcefirst = MAXMOVES-1;
	int n, v1,v2, localeval = MATE;
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
	int safemovenum = 0;
#endif

#ifdef STOREEXACTDEPTH
	int originaldepth = d;
#endif
	HASH localhash;
	POSITION q;				// board for copy/make 
	MATERIALCOUNT Lmatcount;
	MOVE movelist[MAXMOVES];
	int values[MAXMOVES];

	
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

	// time check: abort search if we exceed the time given by aborttime
#ifdef CHECKTIME
	if( (si->negamax & 0xFFFF)==0 && si->searchmode == TIME_BASED)
		if( (clock()- si->start)/CLK_TCK > (si->aborttime)) 
			*(si->play)=1;
#endif

	si->negamax++;

	// check material:
	if(p->bm + p->bk == 0)
		return (p->color == BLACK) ? (-MATE+si->realdepth):(MATE-si->realdepth);
	if(p->wm + p->wk == 0)
		return (p->color == WHITE) ? (-MATE+si->realdepth):(MATE-si->realdepth); 
	
	// return if calculation interrupt is requested
#ifdef PLAYNOW
	if(*(si->play) != 0) 
		return 0;
#endif
	// stop search if maximal search depth is reached - this should basically never happen
	if(si->realdepth > MAXDEPTH) 
		{
		si->leaf++;
		si->leafdepth += si->realdepth;
		return evaluation(p,&(si->matcount),alpha,&delta,0,maxNdb);
		}
	
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
		hashfound = hashlookup(si, &value, &valuetype, d, &forcefirst, p->color, &ispvnode);
#ifdef THREADSAFEHT
		LeaveCriticalSection(&hash_access);
#endif	
		if(hashfound)
			{
			si->hashlookupsuccess++;
			// return value 1: the position is in the hashtable and the depth
			// is sufficient. it's value and valuetype are used to shift bounds or even cutoff 
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
	
	
#ifdef IITERD
	// TODO: we are counting # of pieces here and also in dblookup - do it just once for more efficiency...
	// TODO: if we do the dblookup first, then we don't have to count the pieces here!
#ifdef USEDB
	if(forcefirst == MAXMOVES-1 && d>IIDDEPTH && si->matcount.bm + si->matcount.bk + si->matcount.wm + si->matcount.wk > maxNdb)
#else
	if(forcefirst == MAXMOVES-1 && d>IIDDEPTH)
#endif
		{
		// forcefirst == MAXMOVES-1 means we have no hashmove 
		// get a good first move with a shallower search, 4 ply less, fixed
		value = negamax(si, p, d - IIDREDUCE, alpha, protokiller, &forcefirst, truncationdepth, truncationdepth2,1);
		assert(forcefirst >=0 && forcefirst < MAXMOVES);
		}
#endif //IITERD


#ifdef REPCHECK
	// check for repetitions 
	// TODO: maybe add repcheck[i-1].irreversible to the break statement.
	if(p->bk && p->wk)
		{
		for(i = si->realdepth + HISTORYOFFSET-2; i >= 0; i-=2)
			{
			// stop repetition search if move with a man is detected 
			// this could be expanded with ....[i+-1]
			if(si->repcheck[i].irreversible)
				break;
			if(si->repcheck[i].hash == si->hash.key)			
				return 0;                      
			}
		}
#endif
	
	// get info on captures: can side to move or side not to move capture now?
	stmcapture = testcapture(p);
	
	// get info on capture by opponent (if there is a capture on the board, then the database
	// contains no valid data
	// TODO: do we ever use sntmcapture again? because if no, we could calculate this only
	// if we have a dblookup....
	if(!stmcapture)
		{
		p->color ^= CC;
		sntmcapture = testcapture(p);
		p->color ^= CC;
		}

	if(stmcapture)
		n = makecapturelist(p,movelist,values,forcefirst);
    else
		n = 0;

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

#ifdef THREADSAFEDB
			EnterCriticalSection(&db_access);
#endif
			dbresult = dblookup(p,cl);
#ifdef THREADSAFEDB
			LeaveCriticalSection(&db_access);
#endif

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
					// TODO: we only really need dbwineval  if the initial position
					// is a dbwin/dbloss. i.e. we could set value to + something big, and only if
					// value < localbeta we would get dbwineval.
					// TODO: try the following two lines once
					//if( lbeta < 400)
					//	return 400;
					value = dbwineval(p, &(si->matcount));
										
					//if(value >= localbeta)
					if(value > alpha)
						return value;
					// TODO check on this: does it ever happen?
					localeval = value;
					//printf("!");
					}
				if(dbresult == DB_LOSS)
					{
					//if (alpha > -400)
					//	return -400;
					
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

#ifdef SPA_CUT
	// TODO: move the SPA_CUT thing behind truncation?!
	// use spa table to cut interior nodes
	// make a SPA lookup attempt
	if((bk+wk == 0) && (p->color==BLACK))
		{
		if(bm==5 && wm==5)
			{
			if(spa_lookup(Gkey,Glock,&value,0))
				return value;
			}
		else if(bm==6 && wm==6)
			{
			if(spa_lookup(Gkey, Glock, &value, 1))
				return value;
			}
		}
#endif SPA_CUT 

#ifdef BOOKHT
	if(d>BOOKHTMINDEPTH && d<BOOKHTMAXDEPTH)
		{
		if(bookht_lookup(Gkey, Glock, &value, p->color))
			return value;
		}
#endif

#ifdef NEWTRUNCATION
// new truncation stuff: only if no capture is here, do truncation.
// also, if we are in a potential db position, we don't truncate any
// further: reason: either it is a win/loss, and root is not win/loss ->
// will alphabeta cutoff immediately
// or else it's dangerous to call evaluation if side not to move may have a capture


#ifndef MARKPV
	ispvnode=0;
#endif
#ifdef EXTENDPV
	if (ispvnode)
		d += EXTENDPVDEPTH;
#endif
	// we don't do any truncation if:
	//  -> the side to move is capturing - we may be way off the score
	//  -> we are in a db position - there the huge scores confuse the pruning
	//  -> we are in a previous pv - we want to look at that carefully!

	/*-----------------------------------------------------
	|	pruning decisions:									|
	|	get local eval here and reduce search depth if it   |
	|   is too far away from (alpha, beta).                 |
	|   on the other hand, if the eval is close to the      |
	|   window again, we undo previous pruning              |
	\------------------------------------------------------*/

	// TODO: don't do this is we already have a localeval!
	// TODO: check if we should noprune with sntmcapture
	//if(!stmcapture && !sntmcapture && !isdbpos && !ispvnode)
	if(!stmcapture && !isdbpos && !ispvnode)
		{
		// only get evaluation here if we don't have it already!
		assert(localeval == MATE);
		// get whole evaluation
		// TODO: warning: evaluation can be called here with sntmcapture!
		//localeval = evaluation(p,&(si->matcount),localalpha,&delta, stmcapture|sntmcapture,maxNdb);
		localeval = evaluation(p,&(si->matcount),alpha,&delta, stmcapture|sntmcapture,maxNdb);
		// eval will tell us if it thinks this search is not stable and should not be pruned as much!
		v1 = localeval;
		
		// get distance to window
		v1 = max(alpha-v1, v1-(alpha+1));
		// allow for some distance to the window, taking into account delta from the search.

		// some compile options to test pruning.
#ifndef NOPRUNE
		delta = 0;
#endif
#ifdef NEVERPRUNE
		v1=0;
		v2=0;
#endif
		
		if(selfstalemate(p))
			{
			v1 = 0;
			v2 = 0;
			d += FRAC/2;
			}

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

			truncationdepth += v1/TRUNCDIVHARD; 
			d-= v1/TRUNCDIVHARD;  
			}
		else //not outside window with strong truncation - reexpand and check if we also have to reexpand
			// the weak truncation.
			{
			// reexpand
			d += truncationdepth;
			truncationdepth = 0;
			
			//new double trunc code
			if(v2)
				{
				v2 += TRUNCLEVELSOFT;
				truncationdepth2 += v2/TRUNCDIVSOFT;
				d -= v2/TRUNCDIVSOFT; 
				}
			else
				{
				d += truncationdepth2;
				truncationdepth2 = 0;
				}
			// end new double trunc code
			}
		}
#endif // NEWTRUNCATION

#ifdef SAFE
	//if((!stmcapture) && (localeval > localalpha) && ((p->bk+p->wk)==0))
	if((!stmcapture) && (localeval > alpha) && ((p->bk+p->wk)==0))
		{
		// TODO: safemovenum computation has changed, need to revisit this
		safemovenum = safemoves(p);
		if(safemovenum <= 2) // number to the right designates SAFE x version
			// SAFE 2 was clearly best in trials (+28-20) vs 114n
			safemovenum = 1;
		else
			safemovenum = 0;
		// now, safemovenum is 0 if there are a lot of safe moves, a bit confusing :-(
		}
#endif // SAFE
	
	//---------------------------------------------------//
	// check if we can return the evaluation             //
	// depth must be <=0, and sidetomove has no capture  //
	//---------------------------------------------------//

	if(d<=0 && !stmcapture && !iid && (safemovenum == 0)) // SAFE
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
				value = localeval;
			else
				value = evaluation(p,&(si->matcount),alpha,&delta,0,maxNdb);
	
			return value;
#else
			// optimize a bit
			if(localeval != MATE && ((localeval > alpha) || (localeval < alpha-QSEARCHLEVEL)))
				return localeval;
			else
				{
				if(localeval == MATE)
					{
					localeval = evaluation(p,&(si->matcount),alpha,&delta,0,maxNdb);
					if((localeval > alpha) || (localeval < alpha-QSEARCHLEVEL))
						return localeval;
					}
				//return localeval;
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
					//value = evaluation(p,&(si->matcount),localalpha,&delta,1,maxNdb);
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
	if(n==0)  // this can happen if side to move is stalemated.
		return -MATE+si->realdepth;


	// check for single move and extend appropriately 
	// TODO: move this up to extension decisions? or not, because we have to wait
	// for makemovelist? but isn't this here rather about extending captures?
	// and do we really want to extend all captures? shouldn't we just be extending
	// "sensible" captures, for instance, find "recaptures" and extend them by a full ply?
	if(n==1)
		d += SINGLEEXTEND;
	//if(si->realdepth > 1 && iscapture[si->realdepth-2] && iscapture[si->realdepth-1] && v1==0)
	//	d++;
	
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
		q.color = p->color^CC;
		
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
				}
			if(movelist[index].bk && movelist[index].bm) 
				{si->matcount.bk++; si->matcount.bm--;}
			}
		else
			{
			if(stmcapture)
				{
				si->matcount.bm -= bitcount(movelist[index].bm);
				si->matcount.bk -= bitcount(movelist[index].bk);
				}
			if(movelist[index].wk && movelist[index].wm)
				{si->matcount.wk++; si->matcount.wm--;}
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
		if(bestmovevalue > 128 || stmcapture || i==0 || d<=LATEMOVEMINDEPTH)
			value = -negamax(si, &q,d-FRAC,-(alpha+1),&Lkiller, &forcefirst, truncationdepth,truncationdepth2,0);
		else
			// we have a candidate for reduction
			{
			value = -negamax(si, &q,d-FRAC-LATEMOVEDEPTH,-(alpha+1), &Lkiller, &forcefirst, truncationdepth,truncationdepth2,0);
			// research on fail high
			if(value > alpha)
				value = -negamax(si, &q,d-FRAC,-(alpha+1), &Lkiller, &forcefirst, truncationdepth,truncationdepth2,0);
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
#endif

	// return best value
	return maxvalue;
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

	if (p->color == BLACK)
		{
		black = p->bm|p->bk;
		white = p->wm|p->wk;
		free = ~(black|white);
		
		m =((((black&LFJ2)<<4)&white)<<3);
		m|=((((black&LFJ1)<<3)&white)<<4);
		m|=((((black&RFJ1)<<4)&white)<<5);
		m|=((((black&RFJ2)<<5)&white)<<4);
		if(p->bk)
			{
			m|=((((p->bk&LBJ1)>>5)&white)>>4);
			m|=((((p->bk&LBJ2)>>4)&white)>>5);
			m|=((((p->bk&RBJ1)>>4)&white)>>3);
			m|=((((p->bk&RBJ2)>>3)&white)>>4);
			}
		if(m & free)
			return 1;
		return 0;
		}
	else
		{
		black = p->bm|p->bk;
		white = p->wm|p->wk;
		free = ~(black|white);
		m=((((white&LBJ1)>>5)&black)>>4);
		m|=((((white&LBJ2)>>4)&black)>>5);
		m|=((((white&RBJ1)>>4)&black)>>3);
		m|=((((white&RBJ2)>>3)&black)>>4);
		if(p->wk)
			{
			m|=((((p->wk&LFJ2)<<4)&black)<<3);
			m|=((((p->wk&LFJ1)<<3)&black)<<4);
			m|=((((p->wk&RFJ1)<<4)&black)<<5);
			m|=((((p->wk&RFJ2)<<5)&black)<<4);
			}
		if(m & free)
			return 1;
		return 0;
		}
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




void hashstore(SEARCHINFO *si, POSITION *p, int value, int alpha, int depth, MOVE *best, int32 bestindex)
	{
	// store position in hashtable
	// based on the search window, alpha&beta, the value is assigned a valuetype
	// UPPER, LOWER or EXACT. the best move is stored in a reduced form 
	// well, i no longer use EXACT, since i'm using MTD(f).

	int32 index,minindex;
	int mindepth=1000,iter=0;
	int from,to;

	si->hashstores++;
	//assert(alpha == beta-1);

#ifdef MOHISTORY
	// update history table 
	if(p->color==BLACK)
		{
		from = (best->bm|best->bk)&(p->bm|p->bk);    /* bit set on square from */
		to   = (best->bm|best->bk)&(~(p->bm|p->bk));
		history[LSB(from)][LSB(to)]++;
		}
	else
		{
		from = (best->wm|best->wk)&(p->wm|p->wk);    /* bit set on square from */
		to   = (best->wm|best->wk)&(~(p->wm|p->wk));
		history[LSB(from)][LSB(to)]++;
		}
#endif

	index = si->hash.key & (hashsize-1);
	minindex = index;


	while(iter<HASHITER)
		{
		if(hashtable[index].lock == si->hash.lock || hashtable[index].lock==0) // vtune: use | instead of ||
			/* found an index where we can write the entry */
			{
			hashtable[index].lock = si->hash.lock;
			hashtable[index].depth =(int16) (depth);
			hashtable[index].best = bestindex;
			hashtable[index].color = (p->color>>1);
			hashtable[index].value =( sint16)value;
			/* determine valuetype */
			if(value > alpha) 
				hashtable[index].valuetype = LOWER;
			else
				hashtable[index].valuetype = UPPER;

			return;
			}
		else
			{
			/* have to overwrite */
			if((int) hashtable[index].depth < mindepth)
				{
				minindex=index;
				mindepth=hashtable[index].depth;
				}
			}
		iter++;
		index++;
		}
		/* if we arrive here it means we have gone through all hashiter
		entries and all were occupied. in this case, we write the entry
		to minindex */

	// if alwaysstore is not defined, and we have found no entry with equal or
	// lower importance, we don't overwrite it.
#ifndef ALWAYSSTORE
	if(mindepth>(depth)) 
		return;
#endif

	hashtable[minindex].lock = si->hash.lock;
	hashtable[minindex].depth=depth;
	hashtable[minindex].best=bestindex;
	hashtable[minindex].color=(p->color>>1);
	hashtable[minindex].value=value;
	/* determine valuetype */
	//if(value>=beta) 
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
	useful information.
	else
	forcefirst is set to the best move found previously, and 0 returned
	if the position is not found at all, 0 is returned and forcefirst is
	left unchanged (at MAXMOVES-1). */

	int32 index;
	int iter=0;

	index = si->hash.key & (hashsize-1); // expects that hashsize is a power of 2!

	// TODO: what is this "hashtable[index].lock" good for? 
	while(iter<HASHITER && hashtable[index].lock) 
		{
		if(hashtable[index].lock == si->hash.lock && ((int)hashtable[index].color==(color>>1)))
			{
			// we have found the position 
			*ispvnode=hashtable[index].ispvnode;
			
			// move ordering 
			*forcefirst=hashtable[index].best;
			// use value if depth in hashtable >= current depth
			if((int)hashtable[index].depth>=depth)
				{
				*value=hashtable[index].value;
				*valuetype=hashtable[index].valuetype;

				return 1;
				}
			}
		iter++;
		index++;
		}

	return 0;
	}

int pvhashlookup(SEARCHINFO *si, int *value, int *valuetype, int depth, int32 *forcefirst, int color, int *ispvnode)
	{
	// pvhashlookup is like hashlookup, only with the exception that it marks the entry
	// in the hashtable as being part of the PV
	int32 index;
	int iter=0;

	index = si->hash.key & (hashsize-1);

#ifdef THREADSAFEHT
	EnterCriticalSection(&hash_access);
#endif


	while(iter<HASHITER && hashtable[index].lock) // vtune: use & instead
		{
		if(hashtable[index].lock == si->hash.lock && ((int)hashtable[index].color==(color>>1)))
			{
			// here's the only difference to the normal hashlookup!
			hashtable[index].ispvnode=1;

			/* move ordering */
			*forcefirst=hashtable[index].best;
			/* use value if depth in hashtable >= current depth)*/
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
//	int staticeval;
	int capture;

	//return;

	Lp=*p;
	absolutehashkey(&Lp, &(si->hash));
	sprintf(str,"pv ");
	for(i=0;i<MAXDEPTH;i++)
		{
		forcefirst=100;
		// pvhashlookup also stores the fact that these nodes are pv nodes
		// in the hashtable
		pvhashlookup(si, &dummy,&dummy,0, &forcefirst,Lp.color,&dummy);
		if(forcefirst==100)
			{
			break;
			}
		n = makecapturelist(&Lp,movelist,values,forcefirst);
		if(n)
			capture = 1;
		else 
			capture = 0;
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
			if(capture)
				sprintf(Lstr,"[-]",staticeval);
			else
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


/* check for mobile black men, however, not all - only those
		on rows 3 or more */

/* how to use this stuff: like this! */
	/*tmp=p->bm & 0xFFFFF000;
	while(tmp) //while black men
		{
		m= (tmp & (tmp-1))^tmp; // m is the least significant bit of tmp
		tmp = tmp&(tmp-1);		// and we peel it away. 

		// determine the white attack board without this man on:
		free = ~(p->bk|(p->bm^m)|p->wk|p->wm);
		wattack=backwardjump((p->wk|p->wm), free) | forwardjump(p->wk, free);
	
		// move this black man forward until he's on the last row,
		//	look if there is a way for it to get there 
		mobile = 20;
		while (m)
			{
			m=forward(m) & (~(p->wk | p->wm)) & (~wattack);
			if(m&0xF0000000)
				{
				mobileblackmen+=mobile;
				break;
				}
			mobile-=4;
			}
		}
	// only check for rows 2-6 */


/* table-lookup bitcount */
int bitcount(int32 n) // vtune: make this inlined maybe as a macro
	// returns the number of bits set in the 32-bit integer n 
	{
	return (bitsinword[n&0x0000FFFF]+bitsinword[(n>>16)&0x0000FFFF]);
	}


int LSB(int32 x)
	{
	//-----------------------------------------------------------------------------------------------------
	// returns the position of the least significant bit in a 32-bit word x 
	// or -1 if not found, if x=0.
	// LSB uses "intrinsics" for an efficient implementation
	//-----------------------------------------------------------------------------------------------------

	int returnvalue;

	if(_BitScanForward(&returnvalue,x))
		return returnvalue;
	else
		return -1;
	
	
	/*
	old, non-intrinsic code
	if(x&0x000000FF)
		return(LSBarray[x&0x000000FF]);
	if(x&0x0000FF00)
		return(LSBarray[(x>>8)&0x000000FF]+8);
	if(x&0x00FF0000)
		return(LSBarray[(x>>16)&0x000000FF]+16);
	if(x&0xFF000000)
		return(LSBarray[(x>>24)&0x000000FF]+24);
	return -1;*/
	
	}



void updatehashkey(MOVE *m, HASH *h)
	{
	// given a move m, updatehashkey updates the HASH structure h
	int32 x,y;
	x=m->bm;
	while(x)
		{
		y=LSB(x);
		h->key ^=hashxors[0][0][y];
		h->lock^=hashxors[1][0][y];
		x&=(x-1);
		}
	x=m->bk;
	while(x)
		{
		y=LSB(x);
		h->key ^=hashxors[0][1][y];
		h->lock^=hashxors[1][1][y];
		x&=(x-1);
		}
	x=m->wm;
	while(x)
		{
		y=LSB(x);
		h->key ^=hashxors[0][2][y];
		h->lock^=hashxors[1][2][y];
		x&=(x-1);
		}
	x=m->wk;
	while(x)
		{
		y=LSB(x);
		h->key ^=hashxors[0][3][y];
		h->lock^=hashxors[1][3][y];
		x&=(x-1);
		}
	}

void absolutehashkey(POSITION *p, HASH *hash)
	{
	/* absolutehashkey calculates the hashkey completely. slower than
		using updatehashkey, but useful sometimes */

	int32 x;

	hash->lock=0;
	hash->key=0;
	x=p->bm;
	while(x)
		{
		hash->key ^=hashxors[0][0][LSB(x)];
		hash->lock^=hashxors[1][0][LSB(x)];
		x&=(x-1);
		}
	x=p->bk;
	while(x)
		{
		hash->key ^=hashxors[0][1][LSB(x)];
		hash->lock^=hashxors[1][1][LSB(x)];
		x&=(x-1);
		}
	x=p->wm;
	while(x)
		{
		hash->key ^=hashxors[0][2][LSB(x)];
		hash->lock^=hashxors[1][2][LSB(x)];
		x&=(x-1);
		}
	x=p->wk;
	while(x)
		{
		hash->key ^=hashxors[0][3][LSB(x)];
		hash->lock^=hashxors[1][3][LSB(x)];
		x&=(x-1);
		}
	}


void countmaterial(POSITION *p, MATERIALCOUNT *m)
	{
	// TODO: make a MATERIALCOUNT structure and pass that as parameter.
	/* countmaterial initializes the globals bm, bk, wm and wk, which hold
		the number of black men, kings, white men and white kings respectively.
		during the search these globals are updated incrementally */
	m->bm = bitcount(p->bm);
	m->bk = bitcount(p->bk);
	m->wm = bitcount(p->wm);
	m->wk = bitcount(p->wk);
	}

// put stuff like this into something like string.c


// TODO: put this into book.c

int booklookup(POSITION *p, int *value, int depth, int32 *remainingdepth, int *best, char str[256])
	{
	/* searches for a position in the book hashtable.
	*/
	int32 index;
	int iter=0;
	int bookmoves;
	// todo: change this to bookhashentry!
	HASHENTRY *pointer;
	int bucketsize;
	int size;
	int bookfound = 0;
	int i,j,n;
	int bookcolor;
	int tmpvalue;
	MOVE tmpmove,bestmove;
	int dummy[MAXMOVES], values[MAXMOVES],indices[MAXMOVES];
	int depths[MAXMOVES];
	MOVE ml[MAXMOVES];
	char Lstr[1024], Lstr2[1024];
	HASH hash;
	SEARCHINFO s;
	
	bucketsize = BUCKETSIZEBOOK;
	pointer = book;
	size = bookentries;

	if(pointer == NULL)
		return 0;
	
	// now, look up current position
	absolutehashkey(p,&hash);
	index = hash.key % size;
	
	// harmonize colors between cake and book
	bookcolor = p->color;
	if(p->color==2) 
		bookcolor=0;

	while(iter<bucketsize)
		{
		if(pointer[index].lock == hash.lock && ((int)pointer[index].color==bookcolor)) // use &
			{
			/* we have found the position */
			*remainingdepth = pointer[index].depth;
			*value = pointer[index].value;
			*best = pointer[index].best;
			bookfound = 1;
			}
		iter++;
		index++;
		index %= size;
		if(bookfound) 
			break;
		}

	if(bookfound)
		{
		// search all successors in book
		n = makecapturelist(p, ml, dummy, 0);
		if(!n)
			n = makemovelist(&s, p, ml, dummy, 0,0);

		for(i=0;i<MAXMOVES;i++)
			{
			values[i]=-MATE;
			indices[i]=i;
			}
		bestmove = ml[*best];
		
		// harmonize colors for book and cake
		bookcolor = (p->color^CC);
		if(bookcolor == 2)
			bookcolor=0;

		// for all moves look up successor position in book
		for(i=0;i<n;i++)
			{
			togglemove(p,ml[i]);
			absolutehashkey(p, &hash);
			index = hash.key % size;
			iter = 0;
			while(iter<bucketsize)
				{
				if(pointer[index].lock == hash.lock && ((int)pointer[index].color==bookcolor))
					{
					// found position in book
					depths[i] = pointer[index].depth+1;
					values[i] = -pointer[index].value;
					}
				iter++;
				index++;
				index%=size;
				}
			togglemove(p,ml[i]);
			}
		}

	// print book moves. check if we have successors:
	// order moves so we can print them in status line
	if(bookfound && values[0] != -MATE)
		{
		sprintf(str,"book  ");
		for(i=0;i<n;i++)
			{
			for(j=0;j<n-1;j++)
				{
				if(100*values[j]+depths[j]<100*values[j+1]+depths[j+1])
					{
					tmpvalue = values[j];
					tmpmove = ml[j];
					values[j] = values[j+1];
					values[j+1] = tmpvalue;
					ml[j] = ml[j+1];
					ml[j+1] = tmpmove;
					tmpvalue = depths[j];
					depths[j] = depths[j+1];
					depths[j+1] = tmpvalue;
					tmpvalue = indices[j];
					indices[j] = indices[j+1];
					indices[j+1] = tmpvalue;
					}
				}
			}

		// count number of available book moves and put in variabe bookmoves
		for(i=0;i<n;i++)
			{
			if(values[i] != -MATE)
				bookmoves = i;
			}
		bookmoves++;

		// create ouput string with all moves, values, depths ordered by value
		for(i=0;i<n;i++)
			{	
			if(values[i]==-MATE)
				continue;
			movetonotation(p, &ml[i], Lstr);
			sprintf(Lstr2, " v%i d%i   ", values[i], depths[i]);
			strcat(str,Lstr);
			strcat(str,Lstr2);
			}

		// now, select a move according to value of usethebook 
		// if we have more than one available bookmove
		if(usethebook < BOOKBEST && bookmoves >1)
			{
			bookmoves = 0;
			if(usethebook == BOOKGOOD)
				{
				// select any move equal to the best in value
				for(i=1;i<n;i++)
					{
					if(values[i]==values[0])
						bookmoves = i;
					}
				bookmoves++;
				}
			if(usethebook == BOOKALLKINDS)
				{
				// select any move that is > -30 in value, and within
				// 10 points of the best move
				for(i=1;i<n;i++)
					{
					if(values[i]>values[0]-10 && values[i]>-30)
						bookmoves = i;
					}
				bookmoves++;
				}
			// we have bookmoves equivalent book moves.
			// pick one at random
			if(bookmoves !=0)
				{
				srand( (unsigned)time( NULL ) );
				i = rand() % bookmoves;
				*remainingdepth = depths[i];
				*value = values[i];
				*best = indices[i];
				//sprintf(Lstr," #%i/%i %i",i,bookmoves,values[i]);
				//strcat(Lstr,str);
				//sprintf(str,"%s",Lstr);
				}
			}
		}
	else
		{
		// last book move
		movetonotation(p,&bestmove,Lstr);
		sprintf(str,"book move: %s v %i d %i", Lstr,*value, *remainingdepth);
		}

	return bookfound;
	}

#ifdef SHOTS
int initshots(void)
	{
	POSITION p,q;
	MOVE ml1[MAXMOVES], ml2[MAXMOVES], ml3[MAXMOVES];
	int values[MAXMOVES];
	int i;
	int n1,n2,n3;
	int j,k,l;
	int isshot;
	int bm1, wm1, bm2, wm2;
	
	for(i=0;i<32768;i++)
		{
		blackshottable[i] = 0;
		// set positions corresponding to index i in shot table
		p.bk=0;
		p.wk=0;

		p.bm = i&0x7;
		p.bm |= (((i>>3)&0x7)<<4);
		p.bm |= (((i>>6)&0x7)<<8);

		p.wm = (((i>>9)&0x7)<<16);
		p.wm |= (((i>>12)&0x7)<<20);
		q=p;
		// number of pieces before shot attempt
		bm1 = bitcount(p.bm);
		wm1 = bitcount(p.wm);
		// now, make all black moves:
		n1 = makemovelist(&p, ml1, values, BLACK, 0, 0);
		//if(!(i%64))
		//	{
		//	printboard(&p);
		//	printf("\n%i",i);
		//	}
		for(j=0;j<n1;j++)
			{
			togglemove((&p), ml1[j]);
			// now, if white has a capture, do it
			n2 = makecapturelist(&p, ml2, values, WHITE, 0);
			
			// if there is a capture, set shot table to 1. 
			// revoke this later if it doesnt work out.
			if(n2)
				blackshottable[i]=1;
		
			for(k=0;k<n2;k++)
				{
				// do the white move:
				togglemove((&p), ml2[k]);

				//now, see if black can capture 2
				n3 = makecapturelist(&p, ml3, values, BLACK, 0);
				
				isshot = 0;
				for(l=0;l<n3;l++)
					{
					togglemove((&p), ml3[l]);
					// number of pieces after shot attempt
					bm2 = bitcount(p.bm);
					wm2 = bitcount(p.wm);
					togglemove((&p), ml3[l]);
						
					if((bm2-wm2) > (bm1-wm1))
						{
						// yeah, shot attempt seems successful
						isshot = 1;
						
						}
					}

				if(isshot == 0)
					blackshottable[i]=0;
				// undo the white move
				togglemove((&p), ml2[k]);
				}
			togglemove((&p), ml1[j]);
			}
		/*
		if(blackshottable[i])
			{
			printf("\nblack may have a shot: bm1 %i bm2 %i wm1 %i wm2 %i", bm1, bm2, wm1, wm2);
			printboard(&q);
			getch();
			}*/
		}

	// white shot table	
	for(i=0;i<32768;i++)
		{
		whiteshottable[i] = 0;
		// set positions corresponding to index i in shot table

//       WHITE
//  	28  29  30  31
//	 24  25  26  27
//	   20  21  22  23
//	 16  17  18  19
//	   12  13  14  15
//	  8   9  10  11
//	    4   5   6   7
//	  0   1   2   3
//	      BLACK

		p.bk=0;
		p.wk=0;

		p.wm = (i&0x7)<<29;
		p.wm |= (((i>>3)&0x7)<<25);
		p.wm |= (((i>>6)&0x7)<<21);

		p.bm = (((i>>9)&0x7)<<13);
		p.bm |= (((i>>12)&0x7)<<9);
		q=p;
		// number of pieces before shot attempt
		bm1 = bitcount(p.bm);
		wm1 = bitcount(p.wm);
		// now, make all black moves:
		n1 = makemovelist(&p, ml1, values, WHITE, 0, 0);
	//	if(!(i%4))
	//		{
	//		printboard(&p);
	//		printf("\n%i",i);
	//		getch();
	//		}
		for(j=0;j<n1;j++)
			{
			togglemove((&p), ml1[j]);
			// now, if white has a capture, do it
			n2 = makecapturelist(&p, ml2, values, BLACK, 0);
			
			// if there is a capture, set shot table to 1. 
			// revoke this later if it doesnt work out.
			if(n2)
				whiteshottable[i]=1;
		
			for(k=0;k<n2;k++)
				{
				// do the white move:
				togglemove((&p), ml2[k]);

				//now, see if black can capture 2
				n3 = makecapturelist(&p, ml3, values, WHITE, 0);
				
				isshot = 0;
				for(l=0;l<n3;l++)
					{
					togglemove((&p), ml3[l]);
					// number of pieces after shot attempt
					bm2 = bitcount(p.bm);
					wm2 = bitcount(p.wm);
					togglemove((&p), ml3[l]);
						
					if((bm2-wm2) < (bm1-wm1))
						{
						// yeah, shot attempt seems successful
						isshot = 1;
						
						}
					}

				if(isshot == 0)
					whiteshottable[i]=0;
				// undo the white move
				togglemove((&p), ml2[k]);
				}
			togglemove((&p), ml1[j]);
			}
		
		/*if(whiteshottable[i])
			{
			printf("\nwhite may have a shot: bm1 %i bm2 %i wm1 %i wm2 %i", bm1, bm2, wm1, wm2);
			printboard(&q);
			getch();
			}*/
		}

	return 1; 
	
	}

int isshot(POSITION *p, int color)
	{
	int index;
	int32 occupied;
	
	
	occupied = (p->bm | p->bk | p->wm | p->wk);
	//shots++;
	
//       WHITE
//  	28  29  30  31
//	 24  25  26  27
//	   20  21  22  23
//	 16  17  18  19
//	   12  13  14  15
//	  8   9  10  11
//	    4   5   6   7
//	  0   1   2   3
//	      BLACK

	if(color == BLACK)
		{
		// compute index in shottable: -original
	
		if(((occupied>>12)&0x7)==0)
			{
			index = p->bm & 0x7;
			index += ((p->bm>>4)&0x7)<<3;
			index += ((p->bm>>8)&0x7)<<6;
			index += ((p->wm>>16)&0x7)<<9;
			index += ((p->wm>>20)&0x7)<<12;
			
			if(blackshottable[index])
				return 1;
			}
	//       WHITE
	//  	28  29  30  31
	//	 24  25  26  27
	//	   20  21  22  23
	//	 16  17  18  19
	//	   12  13  14  15
	//	  8   9  10  11
	//	    4   5   6   7
	//	  0   1   2   3
	//	      BLACK

		// compute index in shottable: on bits 1,2,3 etc.
		if(((occupied>>13)&0x7)==0)
			{
			index = (p->bm>>1) & 0x7;
			index += ((p->bm>>5)&0x7)<<3;
			index += ((p->bm>>9)&0x7)<<6;
			index += ((p->wm>>17)&0x7)<<9;
			index += ((p->wm>>21)&0x7)<<12;
			
			if(blackshottable[index])
				return 1;
			}

		// compute index in shottable on bits 4,5,6 etc:
	//       WHITE
	//  	28  29  30  31
	//	 24  25  26  27
	//	   20  21  22  23
	//	 16  17  18  19
	//	   12  13  14  15
	//	  8   9  10  11
	//	    4   5   6   7
	//	  0   1   2   3
	//	      BLACK
		
		if(((occupied>>17)&0x7)==0)
			{
			index = (p->bm>>4) & 0x7;
			index += ((p->bm>>9)&0x7)<<3;
			index += ((p->bm>>12)&0x7)<<6;
			index += ((p->wm>>20)&0x7)<<9;
			index += ((p->wm>>25)&0x7)<<12;
			
			if(blackshottable[index])
				return 1;
			}
			// compute index in shottable for the back rank:
	//       WHITE
	//  	28  29  30  31
	//	 24  25  26  27
	//	   20  21  22  23
	//	 16  17  18  19
	//	   12  13  14  15
	//	  8   9  10  11
	//	    4   5   6   7
	//	  0   1   2   3
	//	      BLACK

		if(((occupied>>9)&0x7)==0)
			{
			index = 7; // edge of the board pretends that everything is there
			index += ((p->bm>>1)&0x7)<<3;
			index += ((p->bm>>4)&0x7)<<6;
			index += ((p->wm>>12)&0x7)<<9;
			index += ((p->wm>>17)&0x7)<<12;
			
			if(blackshottable[index])
				return 1;
			}
//		noshots++;
		return 0;
		}

	else         // color is WHITE
		{
		// compute index in shottable: -original
/* reverse computation	
		p.wm = i&0x7<<29;
		p.wm |= (((i>>3)&0x7)<<25);
		p.wm |= (((i>>6)&0x7)<<21);

		p.bm = (((i>>9)&0x7)<<13);
		p.bm |= (((i>>12)&0x7)<<9);
		
*/
		if(((occupied>>17)&0x7)==0)
			{
			index = (p->wm>>29) & 0x7;
			index += ((p->wm>>25)&0x7)<<3;
			index += ((p->wm>>21)&0x7)<<6;
			index += ((p->bm>>13)&0x7)<<9;
			index += ((p->bm>>9)&0x7)<<12;
			//printf("%i  ",index);
			if(whiteshottable[index])
				return 1;
			}
	//       WHITE
	//  	28  29  30  31
	//	 24  25  26  27
	//	   20  21  22  23
	//	 16  17  18  19
	//	   12  13  14  15
	//	  8   9  10  11
	//	    4   5   6   7
	//	  0   1   2   3
	//	      BLACK

		// compute index in shottable: on bits 28,29,30 instead of 29 30 31 etc.
		if(((occupied>>16)&0x7)==0)
			{
			index = (p->wm>>28) & 0x7;
			index += ((p->wm>>24)&0x7)<<3;
			index += ((p->wm>>20)&0x7)<<6;
			index += ((p->bm>>12)&0x7)<<9;
			index += ((p->bm>>8)&0x7)<<12;
			//printf("%i  ",index);
			if(whiteshottable[index])
				return 1;
			}

		// compute index in shottable on bits 25,26,27 etc:
	//       WHITE
	//  	28  29  30  31
	//	 24  25  26  27
	//	   20  21  22  23
	//	 16  17  18  19
	//	   12  13  14  15
	//	  8   9  10  11
	//	    4   5   6   7
	//	  0   1   2   3
	//	      BLACK
	
	if(((occupied>>12)&0x7)==0)
			{
			index = (p->wm>>25) & 0x7;
			index += ((p->wm>>20)&0x7)<<3;
			index += ((p->wm>>17)&0x7)<<6;
			index += ((p->bm>>9)&0x7)<<9;
			index += ((p->bm>>4)&0x7)<<12;
			//printf("%i  ",index);
			if(whiteshottable[index])
				return 1;
			}
			// compute index in shottable for the back rank:
	//       WHITE
	//  	28  29  30  31
	//	 24  25  26  27
	//	   20  21  22  23
	//	 16  17  18  19
	//	   12  13  14  15
	//	  8   9  10  11
	//	    4   5   6   7
	//	  0   1   2   3
	//	      BLACK

	if(((occupied>>20)&0x7)==0)
			{
			index = 7;
			index += ((p->wm>>28)&0x7)<<3;
			index += ((p->wm>>25)&0x7)<<6;
			index += ((p->bm>>17)&0x7)<<9;
			index += ((p->bm>>12)&0x7)<<12;
			//printf("%i  ",index);
			if(whiteshottable[index])
				return 1;
			}
	
//		noshots++;
		return 0;
		}

	}


#endif // SHOTS

#ifdef QS_SQUEEZE
int issqueeze(POSITION *p, int color)
	{
	// tries to determine whether the side to move can squeeze a man of the other side
	// and win with that.
	int32 occupied = p->bm|p->bk|p->wm|p->wk;
	int32 free = ~occupied;
	int32 m;
		
//       WHITE
//  	28  29  30  31
//	 24  25  26  27
//	   20  21  22  23
//	 16  17  18  19
//	   12  13  14  15
//	  8   9  10  11
//	    4   5   6   7
//	  0   1   2   3
//	      BLACK

	if(color==BLACK)
		{
		// right forward squeeze
		m=(~EDGE & p->bm & (occupied>>1) & twobackward(p->wm) & rightbackward(occupied) & leftbackward(free) & leftbackward(leftbackward(~(p->wm))) & twobackward(rightbackward(free)));
		if(m)
			{
			// create the squeezable stone
			m=twoforward(m);

			// check if it can be backed up:
			m = leftforward(m);
			m |= forward(m);
			if(!(m&p->wm))
				return 1;
			}
		// left forward squeeze
		m=(~EDGE & p->bm & (occupied<<1) & twobackward(p->wm) & leftbackward(occupied) & rightbackward(free) & rightbackward(rightbackward(~(p->wm))) & twobackward(leftbackward(free)));
		if(m)
			{
			// create squeezable stone:
			m=twoforward(m);
			// check if it can be backed up:
			m= rightforward(m);
			m|=forward(m);
			if(!(m&p->wm))
				return 1;
			}
		return 0;
		}

	// color is white
	

	// left backward squeeze
	m= (~EDGE & p->wm & (occupied>>1) & twoforward(p->bm) & rightforward(occupied) & leftforward(free) & leftforward(leftforward(~(p->bm))) & twoforward(rightforward(free)));
	if(m)
		{
		// create squeezable stone
		m = twobackward(m);
		// check if it can be backed up:
		m = leftbackward(m);
		m |= backward(m);
		if(!(m&p->bm))
			return 1;
		}
	// right backward squeeze
	m=(~EDGE & p->wm & (occupied<<1) & twoforward(p->bm) & leftforward(occupied) & rightforward(free) & rightforward(rightforward(~(p->bm))) & twoforward(leftforward(free)));
	if(m)
		{
		// create squeezable stone
		m = twobackward(m);
		// check if it can be backed up:
		m = rightbackward(m);
		m |= backward(m);
		if(!(m&p->bm))
			return 1;
		}
	
	return 0;
	}
#endif // QS_SQUEEZE


/*
int domove(POSITION *p, MOVE *m)
	{
	p->bm ^= m->bm;
	p->bk ^= m->bk;
	p->wm ^= m->wm;
	p->wk ^= m->wk;
	p-> color^= CC;
	return 1;
	}
*/


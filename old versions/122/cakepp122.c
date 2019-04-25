/*		cake++ 1.22 - a checkers engine			*/
/*															*/
/*		Copyright (C) 2001 by Martin Fierz		*/
/*															*/
/*		contact: checkers@fierz.ch					*/



/*   version history
	1.22 (13th may 2001)
		-> cake++ can use a book of tom lincke now
		-> fixed a bug with MOSTATIC and MOTESTCAPT
		TODO: move from 12byte hashentry to 8 bytes!
		
	1.21 (17th february 2001)
		-> removed terrible bug in hashtable XOR values: only
			half the entries were being used
		-> tested ed gilbert's xref array method, with a bubblesort,
			but it was slower than berni's linear search method
		-> tested ETC depth: 2* frac was best, 3* frac close, 1*frac
			definitely worse, no ETC very much worse

		-> Cake++ 1.21 - KingsRow 1.09e by Ed Gilbert
			+:44 =:173 -:64 unknown:1
			best result so far against 1.09e which is a huge improvement over
			1.09.
	
	1.2 (22nd december 2000)
		lots of ideas.
		->	tested MOSTATIC - no good! taken it out => ~5-10% speed 
			Cake++ 1.193 - KingsRow 1.09 by Ed Gilbert
			+:54 =:167 -:61 unknown:0
		->	improve hashtable size
			to start with i replace old mask-stuff with bitfields, so the hashentry
			struct gets more transparent anyway now, with valuetype, depth and color.
	
	1.2 beta (18th november 2000)

		cakepp now doesnt need a db.ini file any more. it autodetects
		either db6 or db4 or no database.

		added effective depth: globals 'leafs' and 'leafdepth', where
		leafs is the number of leaf nodes searched and leafdepth = sum of depth over
		all leaf nodes -> average depth = leafdepth/leafs.

		TODO... NEVER DONE!
		  try new stuff in eval: 'under the influence of a king' tries to 
		estimate how dangerous a king is by defining a few squares around
		every king as being under the influence - if there is a man of the
		opposite color on these squares, this gives a bonus for the side with
		the king.
	
	1.193 (ed has doubled the speed of his program! i try:

    -->  change material eval once more to more sensible values:
    		Cake++ 1.192b - KingsRow 1.08j by Ed Gilbert
			+:72 =:163 -:41 unknown:6
    --> change re-extension only if n==0 and fineevalwindow 140 instead
    		of 150 -> +72-50 damn.
    -->  changed fineevalwindow to 140
         Cake++ 1.192b - KingsRow 1.08j by Ed Gilbert
			+:69 =:164 -:42 unknown:7     ->back to 150
    --> turned off coarsegraining the same result as with!:
    		Cake++ 1.192b - KingsRow 1.08j by Ed Gilbert
			+:75 =:154 -:44 unknown:9
    --> tried a higher value for tempo for 10,9,8,7 pieces (+1) to no avail
    		Cake++ 1.192b - KingsRow 1.08j by Ed Gilbert
			+:68 =:165 -:48 unknown:1
    --> tried a cramped single corner feature :-(
    		+:71 =:159 -:48 unknown:4
    --> cramped single corner only if no kings:
    		+:68 =:158 -:49 unknown:7
    --> tried real dyke = bonus for piece on 19:
    		+:72 =:163 -:42 unknown:5  => indifferent.
	--> compiled the thing with visualC++ 6.0 pro instead of borlandC 5.5
			+:73 =:170 -:33 unknown:6
        wow! quite a difference!
		it's only 10% speed difference...
	--> another 10% after finding some more optimizations gives
			+:74 =:168 -:32 unknown:8
		not such a big improvement :-(
		theory: it's just in cake's low winlevel and draw code - turn it off...
			put up winlevel to 1600 and remove draw code:
			+:77 =:165 -:33 unknown:7

    -->	inlining the updatehashkey function in negamax.
    -->	repetition check using the hash key instead of the whole position.
         nope - didnt do this...

	 1.192  (8th october 2000)
    --> 	table-lookup for material evaluation, new global static int
    		materialeval[bm][bk][wm][wk] holds the material evaluation
         for this material distribution. approx. 5% speed increase!

         switched to kingsrow 1.08j as sparring partner
         baseline:
         Cake++ 1.192 - KingsRow 1.08j by Ed Gilbert
			+:65 =:173 -:42 unknown:2 (checked, = draws)
         PVS on:
         +:76 =:156 -:45 unknown:5 (checked, = draws)  YES! this seems to work!
         SE on:

         changing DSE window to alpha,beta+QLEVEL:  50%
         						  to alpha-QLEVEL: +:74 =:156 -:47 unknown:5

         QLEVEL 70: +:75 =:165 -:37 unknown:5 = best result ever!
         this will also be very much needed - ed has a factor 2 in speed coming!
    1.191  (6th october 2000)
	 -->  complete fail-soft introduced - up to now, (first)negamax would return
    		value on fail-high, but alpha on fail-low. now it returns the best
         value anyway.
         new logic in these routines: alpha and beta are fix, what comes
         in from the caller, but can be changed by hashlookups and ETC. but
         in the main recursion loop alpha&beta are here to stay, Lalpha is
         the increasing value of the position. bestvalue starts at -5000 and
         goes up to whatever the best value is and is the value which is returned
         using standards: PVSoff SEoff ETCon: overall 92.8% geometric 95.9%
         using PVSon                          overall 78.9% geometric 80.95%  time2181s
         PVSon results in small differences in cakelog.txt in the testcake
         program - i believe it's ok, because more lines may be truncated
         with smaller windows -> it sometimes misses some lines which could
         cause the value of the position to change.

         baseline (PVS off, SE off) result:
         Cake++ 1.191 - KingsRow 1.08 by Ed Gilbert
			+:80 =:150 -:42 unknown:10

    1.19   (1st october 2000)

    -->  singular extensions with switch SINGULAREXTENSIONS, use another
    		switch, SEDEPTH to know how far to extend. at the moment SE works
         like this: if a node has a value inside the alpha-beta-window, and
         all other siblings are worse than value-SELEVEL, SE kicks in.
         SE *only* in non-capturing nodes. could be changed to SE in capturing
         nodes too, but of course n>1, so that would just be n>1 instead of
         capture==0.
         @the moment: 6.10^-5 nodes of all get an SE

         current settings: SEDEPTH=20 and SELEVEL=50 take very few more
         nodes than no SE (overall 2.1%, geometric 8%, but this is mostly because
         of one 436% node), but take more time - the sefailed-test
         needs to re-search many times -> less nodes/sec (390  compared to 420 or so)

    -->  new compile switch PVS turns PVS on or off.

         new PVS re-search code
         overall 86%, geometric 87.5% of nodes searched compared to without PVS
         
    		(there still seems to be a problem with PVS! returns odd values
         even though coarsegraining is on, and maybe plays worse?!
         @ the moment: PVS off ->11 % more nodes, but stable. probably it's
         a bug with the re-search code of PVS. )

    -->  changed from windowed alphabeta to windowed PVS-style alphabeta.
         result in test: 81.7% overall nodes, 85.0% logratio nodes compared
         with no PVS.

    -->  changed hashlookup call: instead of
    		hashlookup(&value, &alpha, &beta, depth, &best, color)
         it is
         hashlookup(&value, &valuetype, depth, &best, color)
         this means that the shifting of bounds has to be done (and is done) in
         negamax now, but also that the valuetype information is transparent
         for the ETC code.

    1.181 (17th september)
    --> 	changed moveordering code: only first move is ordered in
        	movelist. all other sorting will be done in negamax if
        	necessary. a few % speed increase (2%? 3%?)
    -->  new compile switch TRUNCATIONTEST: when a final position will be examined
    		because of d<=0 and truncationdepth is larger than a threshold (20),
         evaluation is called, and if this value is NOT outside truncation bounds,
         the truncated line will be reexpanded. this is more 'correct' than a
         mere material test and can recognize positional compensation. probably
         this would deserve an own, lower truncation window test because the result
         is more accurate.

    1.18 (16th september)

    --> after some testing with values for truncation depth
    	  and singleextend parameters i now use 15 for TD, 5 for SE.
    --> added some more knowledge to evaluation function:
    	  -> runaways in two are detected as runaways now if the have
        	  a clear run left or right ahead, instead of both sides.
        -> more trapped king code: kings trapped in double corner
        	  by two men are recognized, and kings trapped in single corner
           by two men are recognized too.
        -> and a new cramp formation is recognized.
           result:
        		Cake++ 1.18 - KingsRow 1.08 by Ed Gilbert
				+:71 =:171 -:36 unknown:4
    1.17 (5th september)
    --> saw crashes because MAXMOVES = 24 was too small - is 28 now
    --> fixed one more bug (man center value) in eval
    --> fixed a bug in truncation code which made much
        too much truncations and didn't reexpand correctly
 	 --> moved truncation decision before (d<0) test

    1.16 (31st august)
    --> fixed quite a number of bugs in evaluation function.
    --> added some knowledge to evaluation

    1.15 (12th august)
    --> window size reduced to 10 -> 10% less nodes
    --> implemented ETC -> 10% less nodes
        (there is a switch #define ETC to turn this on/off)
        
 	 1.14 (27th june)
    improvements over 1.13:
    --> cramp evaluation
    --> additional info for database wins/losses
    --> static evaluation implemented

    */


#include "switches.h"
#include <stdio.h>
#include <stdlib.h> /* malloc() */
#include <string.h> /* memset() */
#include <time.h>
#if SYSTEM == SYS_WINDOWS
#include <conio.h>
#include <windows.h>
#endif/*SYSTEM*/

/* structs.h defines the data structures for cake++ */
#include "structs.h"

/* consts.h defines constants used by cake++ */
#include "consts.h"

/* function prototypes */
#include "cakepp.h"
#include "movegen.h"
#include "db.h"


#if SYSTEM == SYS_MACOS
#define CLK_TCK CLOCKS_PER_SEC
#elif SYSTEM == SYS_UNIX
#define CLK_TCK CLOCKS_PER_SEC
#endif /*SYSTEM*/

/* globals */
struct pos p;
static int32 Gkey,Glock;
static int bm,bk,wm,wk;
static int realdepth, maxdepth;

/* p is a global which always contains the current position */
unsigned int cake_nodes;
static int dblookups;
static unsigned int leafs,leafdepth;

int *play; /*is nonzero if the engine is to play immediately */


/* create two hashtables: one where all positions with realdepth < DEEP are stored, and
	where the bucket size is large, and another where everything is overwritten all the time */
int hashstores;
static struct hashentry *deep, *shallow, *book;
//struct move Gmovelist[MAXDEPTH][MAXMOVES];

static char *out;
#ifdef USEDB
int maxNdb=0; /* the largest number of stones which is still in the database */
#endif
static int Gtruncationdepth=TRUNCATIONDEPTH;

double start,t,maxtime; /* time variables */

#ifdef REPCHECK
struct pos Ghistory[MAXDEPTH+HISTORYOFFSET+10]; /*holds the current variation for repetition check*/
#endif
/* a file pointer for logging */
static int cakeisinit=0;
int searchmode;
int logging;
static FILE *cake_fp;
/* history table */
int32 history[32][32];
static int32  hashxors[2][4][32];
/* bit tables for last bit in byte, and number of set bits in word */
static int lastone[256];
static int bitsinword[65536];
/* material value lookup table */
static int materialeval[13][13][13][13]; /*bmbkwmwk*/
static int blackbackrankeval[256],whitebackrankeval[256],black2backrankeval[256],white2backrankeval[256];
static int norefresh;

/*----------------------------------interface---------------------------------*/
/* consists of initcake() exitcake() and getmove() */

int initcake(int log)
   /* initcake must be called before any calls to cake_getmove can be made.
   	it initializes the database and some constants, like the arrays for lastone
      and bitsinword */
	{
   int i,j,k,l;
   int v1,v2;
   int32 u,index;
	FILE *Lfp;

   static int br[32]={0,0,1,1,1,4,4,6,
  				 1,4,18,18,4,6,20,20,
             0,0,1,1,1,8,8,8,
             1,4,18,18,4,8,20,20};
   const int devsinglecornerval=5;
   const int intactdoublecornerval=5;
   const int oreoval=5;

   logging=log;
	deep=malloc(HASHSIZEDEEP*sizeof(struct hashentry));
   shallow=malloc(HASHSIZESHALLOW*sizeof(struct hashentry));
   book=malloc(HASHSIZEBOOK*sizeof(struct hashentry));
	if(deep==NULL || shallow==NULL || book==NULL)
		exit(0);
	/* load book */
	Lfp=fopen("cakebook.bin","rb");
	if(Lfp!=NULL)
		{
		fread(book,sizeof(struct hashentry),HASHSIZEBOOK,Lfp);
		fclose(Lfp);
		}

	/* initialize xors */
   srand(1);
   for(i=0;i<4;i++)
   	{
      for(j=0;j<32;j++)
   		{
         hashxors[0][i][j]=myrand();
         hashxors[1][i][j]=myrand();
         }
      }
   #if SYSTEM==SYS_WINDOWS
   srand(1);
   for(i=0;i<4;i++)
   	{
      for(j=0;j<32;j++)
   		{
         hashxors[0][i][j]=myrand();
         hashxors[1][i][j]=myrand();
         }
      }
   #endif
	/* clear the last two bits of hashlock for color puposes */
	for(i=0;i<4;i++)
		{
		for(j=0;j<32;j++)
			hashxors[1][i][j]&=0xFFFFFFFC;
		}
   /* initialize array for "lastone" */
   for(i=0;i<256;i++)
   	{
      if(i&BIT0) {lastone[i]=0;continue;}
      if(i&BIT1) {lastone[i]=1;continue;}
      if(i&BIT2) {lastone[i]=2;continue;}
      if(i&BIT3) {lastone[i]=3;continue;}
      if(i&BIT4) {lastone[i]=4;continue;}
      if(i&BIT5) {lastone[i]=5;continue;}
      if(i&BIT6) {lastone[i]=6;continue;}
      if(i&BIT7) {lastone[i]=7;continue;}
      }
   /* initialize bitsinbyte */
	for(i=0;i<65536;i++)
   	bitsinword[i]=recbitcount((int32)i);
#ifdef USEDB
	maxNdb=DBInit();
#endif
   if(log & 1)
      /* delete old logfile */
   	{
      cake_fp=fopen("cakelog.txt","w");
      fclose(cake_fp);
      }
   for(i=0;i<13;i++)
   	{
      for(j=0;j<13;j++)
      	{
         for(k=0;k<13;k++)
         	{
            for(l=0;l<13;l++)
            	{
               /*bm bk wm wk */
               v1=100*i+130*j;
               v2=100*k+130*l;
               if(v1+v2==0) continue;
               v1=v1-v2+(EXBIAS*(v1-v2))/(v1+v2);

               /* take away the 10 points which a side is up over 100 with a one
               	man advantage in the 12-11 position */
               if(v1<=-100) v1+=10;
               if(v1>=100) v1-=10;
               materialeval[i][j][k][l]=v1;
               }
            }
         }
      }
   /* initialize backrankeval and back2rankeval */
   for(u=0;u<256;u++)
   	{
      p.bm=u;
      p.wm=u<<24;

      index=p.bm&0x0000000F;
   	if(p.bm & BIT7) index+=16;
      blackbackrankeval[u]=br[index];


      index=0;
   	if(p.wm & BIT31) index++;
   	if(p.wm & BIT30) index+=2;
   	if(p.wm & BIT29) index+=4;
   	if(p.wm & BIT28) index+=8;
   	if(p.wm & BIT24) index+=16;

   	whitebackrankeval[u]=br[index];

      white2backrankeval[u]=0;
      black2backrankeval[u]=0;
      /* oreo */
   	if( (p.bm&BIT1) && (p.bm&BIT2) && (p.bm&BIT5) ) black2backrankeval[u]+=oreoval;
   	if( (p.wm&BIT29) && (p.wm&BIT30) && (p.wm&BIT26) ) white2backrankeval[u]+=oreoval;


      /* developed single corner */
   	if( ((~p.bm)&BIT0) && ((~p.bm)&BIT4) ) black2backrankeval[u]+=devsinglecornerval;
   	if( ((~p.wm)&BIT31) && ((~p.wm)&BIT27) ) white2backrankeval[u]+=devsinglecornerval;

      /*  double corner evals: intact and developed */
      if( p.bm&BIT3 )  /* for black */
      	{
         if( (p.bm&BIT6) || (p.bm&BIT7) ) black2backrankeval[u]+=intactdoublecornerval;
         /*if( (p.bm&BIT6)  ) intactdoublecorner+=intactdoublecornerval;  */
         }
      if( p.wm&BIT28 ) /* for white */
      	{
         if( (p.wm&BIT24) || (p.wm&BIT25) ) white2backrankeval[u]+=intactdoublecornerval;
         /*if( (p.wm&BIT25) ) intactdoublecorner-=intactdoublecornerval; */
         }

      }
   cakeisinit=1;
   return 1;
   }


int exitcake(void)
	{
	/*   fclose(cake_fp);*/
	/* deallocate memory for the hashtables */
	free(deep);
	free(shallow);
	return 1;
	}

int cake_getmove(struct pos *position,int color, int how,double maximaltime,
					  int depthtosearch, int32 maxnodes,
					  char str[255], int *playnow, int log, int reset)
	{
	/* cake_getmove is the entry point to cake++
		give a pointer to a position and you get the new position in
		this structure after cake++ has calculated.
		color is BLACK or WHITE and is the side to move.
		how is 0 for time-based search and 1 for depth-based search and 2 for node-based search
		maxtime and depthtosearch and maxnodes are used for these three search modes.
		cake++ prints information in str
		if playnow is set to a value != 0 cake++ aborts the search.
		if (logging&1) cake++ will write information into "log.txt"
		if(logging&2) cake++ will also print the information to stdout.
		if reset!=0 cake++ will reset hashtables and repetition checklist
	*/

	int d;
	int value,lastvalue=0,n,i,j, guess;
	struct move best,last, movelist[MAXMOVES];
	struct pos dummy;
	char Lstr[256];
	char tempstr[255];
	int bookvalue, bookvaluetype, bookff,bookfound=1,bookbestvalue;
	struct move bookmove;
	
	if(!cakeisinit) initcake(logging);

	play=playnow;
	out=str;
	logging=log;
	maxtime=maximaltime;
	searchmode=how;

	p=(*position);
	if(logging & 1)
		{
		cake_fp=fopen("cakelog.txt","a");
		printboardtofile(p);
		fprintf(cake_fp,"\nposition hex bm bk wm wk color:\n0x%x 0x%x 0x%x 0x%x %i",p.bm,p.bk,p.wm,p.wk,color);
		}

	/* initialize material */
	countmaterial();

	/* reset history table */
	for(i=0;i<32;i++)
		{
		for(j=0;j<32;j++)
			{
			history[i][j]=0;
			}
		}

	/* set truncation depth setting:
		in endgames do not truncate any more */
	if(bm+bk+wm+wk<=6) Gtruncationdepth=0;
	else Gtruncationdepth=TRUNCATIONDEPTH;

	//Gtruncationdepth=TRUNCATIONDEPTH;
	/* clear the hashtable */
	memset(deep,0,HASHSIZEDEEP*sizeof(struct hashentry));
	/*if(reset!=0)*/
	memset(shallow,0,HASHSIZESHALLOW*sizeof(struct hashentry));
	/*for(i=0;i<HASHSIZEDEEP;i++)
		deep[i].lock=0;
	for(i=0;i<HASHSIZESHALLOW;i++)
		shallow[i].lock=0;*/
	cake_nodes=0;
	dblookups=0;
	realdepth=0;
	maxdepth=0;
	hashstores=0;
	/*singular=0;*/
	norefresh=0;
	n=makecapturelist(movelist, color, 0);

#ifdef REPCHECK
	/*initialize history list */
	Ghistory[HISTORYOFFSET]=p;
	dummy.bm=BIT0;
	dummy.wm=BIT0;
	dummy.bk=BIT0;
	dummy.wk=BIT0;
	if(reset==0)
		{
		for(i=0;i<HISTORYOFFSET-2;i++)
			Ghistory[i]=Ghistory[i+2];
		}
	else
		{
		for(i=0;i<HISTORYOFFSET+8;i++)
			Ghistory[i]=dummy;
		}
#endif

	absolutehashkey();

		/* do a book lookup */
#ifdef BOOK	
	if(logging&1)
		{
		bookfound=0;	
		if(booklookup(&bookvalue,&bookvaluetype,0,&bookff,color))
			{
			bookfound=1;
			/* this position is there, now, do all moves and look up their values */
			/* possibly, this position does not have all followups, so it is not sure
				that we really have it!*/
			n=makecapturelist(movelist,color,0);
			if(!n)
				n=makemovelist(movelist,color,0,0);
			/* set best value*/
			bookbestvalue=-MATE;

			for(i=0;i<n;i++)
				{
				/* domove */
				togglemove(movelist[i]);
				countmaterial();
				absolutehashkey();

				if(!booklookup(&bookvalue,&bookvaluetype,0,&bookff,color^CC))
					bookfound=0;
				
				/* undo move */
				togglemove(movelist[i]);
				countmaterial();
				absolutehashkey();
				
				bookvalue=-bookvalue;
				if(bookvalue>bookbestvalue) 
					{
					bookbestvalue=bookvalue;
					bookmove=movelist[i];
					}
				movetonotation(p,movelist[i],Lstr,color);
				sprintf(str,"%s: %i\n",Lstr,bookvalue);
				if(i==0)
					fprintf(cake_fp,"\n\n");
				fprintf(cake_fp,"%s",str);
				}
		
			}
		if(bookfound)
			{
			movetonotation(p,bookmove,Lstr,color);
			sprintf(str,"found position in book, value=%i, move is %s\n",bookbestvalue,Lstr);
			best=bookmove;
			value=0;
			}
		else
			{
			sprintf(str,"%X %X not found in book\n",Gkey,Glock);
			value=0;
			}
		fprintf(cake_fp,"\n%s",str);
		}
#endif
	absolutehashkey();
	countmaterial();
	n=makecapturelist(movelist, color, 0);

	start=clock();
	guess=0;
	if(!bookfound)
		{
		for(d=1;d<MAXDEPTH;d+=2)
			{
			leafs=0;
			leafdepth=0;
	
			/* choose which search algorithm to use: */
		
			/* non - windowed search */
			//value=firstnegamax(FRAC*d, color, -MATE, MATE, &best);
	
			/*******************/
			/* windowed search */
			/* standard!       */
			/*******************/
			//value=windowsearch(FRAC*d, color, guess, &best);
			/* set value for next search */
			//guess=value;	
	
			/* MTD(F) search */
			value=mtdf(guess,FRAC*d,color,&best);
			guess=value;	

			/* generate output */
			movetonotation(p,best,Lstr,color);
			t=clock();
#ifndef ANALYSISMODULE
			if((t-start)>0)
				sprintf(str,"best: %s depth %i/%i/%2.1f nodes %lu value %i time %3.2fs %4.0fkN/s",Lstr,d,maxdepth,(float)leafdepth/((float)leafs+0.01),cake_nodes,value,(t-start)/CLK_TCK,cake_nodes/1000/(t-start)*CLK_TCK);
			else
				sprintf(str,"best: %s depth %i/%i/%2.1f nodes %lu value %i time %3.2fs ?kN/s",Lstr,d,maxdepth,(float)leafdepth/((float)leafs+0.01),cake_nodes,value,(t-start)/CLK_TCK);
#endif
			if(logging&1)
				{
				fprintf(cake_fp,"\n%s",str);
				if(d>FRAC*10) fflush(cake_fp);
				}
			if(logging&2)
				{
				printf("\n%s",str);
				fflush(stdout);
				}
			/* iterative deepening loop break conditions: depend on search mode, 'how':
				how=0: time mode;*/
			if(d>1)
				{
				if( how==0 && ((clock()-start)/CLK_TCK>(maxtime/2)) ) 
					break;
				if(how==1 && d>=depthtosearch)
					break;
				if(how==2 && cake_nodes>maxnodes)
					break;
#ifdef IMMEDIATERETURNONFORCED
				/* do not search if only one move is possible */
				if(n==1) /* this move was forced! */
					{
				/* set return value to 0 so no win/loss claims hamper engine matches*/
					value=0;
					break;
					}
#endif
				if(abs(value)>MATE-100) break;
				}
			if(*play)
				{
				/* stop the search. don't use the best move & value because they are rubbish */
				best=last;
				movetonotation(p,best,Lstr,color);
				value=lastvalue;
#ifndef ANALYSISMODULE
				sprintf(str,"interrupt: best %s value %i",Lstr,value);
#endif
				break;
				}
			lastvalue=value; /* save the value for this iteration */
			last=best; /* save the best move on this iteration */
			norefresh=1;
		}
	}
#ifdef REPCHECK
	Ghistory[HISTORYOFFSET-2]=p;
#endif
	if(!bookfound)
		{
		getpv(Lstr,color);
#ifndef ANALYSISMODULE
		strcat(str,"\n");
		strcat(str,Lstr);
		sprintf(tempstr,"");
		strcat(tempstr,str);
		sprintf(str,tempstr);
#endif
		}
	if(!(*play))
		{togglemove(best);}
	else
		{togglemove(last);}
	if(logging&1)
		{
		fprintf(cake_fp,"%s\n",Lstr);
		fflush(cake_fp);
		}
	if(logging&2)
		{
		printf("%s\n",Lstr);
		fflush(stdout);
		}
#ifdef REPCHECK
	Ghistory[HISTORYOFFSET-1]=p;
#endif
	*position=p;
	if(logging&1) fclose(cake_fp);
	return value;
}

/*-----------------------------------------------------------------------------*/


int mtdf(int firstguess,int depth, int color, struct move *best)
	{
	int g,lowerbound, upperbound,beta;

	g=firstguess;
	//g=0; /* strange - this seems to help?! */
	upperbound=MATE;
	lowerbound=-MATE;
	while(lowerbound<upperbound)
		{
		if(g==lowerbound)
			beta=g+1;
		else beta=g;
		g=firstnegamax(depth,color,beta-1,beta,best);
		if(g<beta)
			upperbound=g;
		else
			lowerbound=g;
		}
	return g;
	}


int windowsearch(int depth, int color, int guess, struct move *best)
	{
	int value;
	/*do a search with aspiration window*/
	value=firstnegamax(depth,color,guess-ASPIRATIONWINDOW,guess+ASPIRATIONWINDOW,best);

	if(value>=guess+ASPIRATIONWINDOW)
		{
		//value=firstnegamax(depth,color,guess+ASPIRATIONWINDOW-1,MATE,best);
		value=firstnegamax(depth,color,guess,MATE,best);
		//if(value<=guess+ASPIRATIONWINDOW-1)
		if(value<=guess)
			value=firstnegamax(depth,color,-MATE,MATE,best);
		}
	if(value<=guess-ASPIRATIONWINDOW)
		{
		//value=firstnegamax(depth,color,-MATE,guess-ASPIRATIONWINDOW+1,best);
		value=firstnegamax(depth,color,-MATE,guess,best);
		//if(value>=guess-ASPIRATIONWINDOW+1)
		if(value>=guess)
			value=firstnegamax(depth,color,-MATE,MATE,best);
		}
	return value;
	}


#ifndef ANALYSISMODULE
/* normal version of cake++ */
int firstnegamax(int d, int color, int alpha, int beta, struct move *best)
	{
	/* firstnegamax is the first instance of the negamax-type recursion for the
		tree search. at the root of the search tree a best move must be returned,
		this is the reason that firstnegamax exists. it calls negamax which calls
		itself again and does not return a move.

		firstnegamax is much simpler than negamax because it does not have to do
		some things:
		hashlookup(), ETC test, extension and truncation decisions, and it does
		not have to be fast since it is only called once for a fixed search depth.
		therefore it does not have to have an inlined material count.
		*/
	int i,j,value,swap=0,bestvalue=-MATE;
	static int n;
	static struct move movelist[MAXMOVES];
	int32 l_bm,l_bk,l_wm,l_wk;
	int32 Lkey,Llock;
	int Lalpha=alpha;
	int32 forcefirst=0;
	int32 Lkiller=0;
	static struct pos last;
	struct move tmpmove;
	static int values[MAXMOVES]; /* holds the values of the respective moves - use to order */
	int refnodes[MAXMOVES]; /* number of nodes searched to refute a move */
	int tmpnodes;
	
	if(*play) return 0;
	cake_nodes++;

	if(last.bm==p.bm && last.bk==p.bk && last.wm==p.wm && last.wk==p.wk &&norefresh)
	/* then we are still looking at the same position - no need to
		regenerate the movelist */    ;
		else
		{
		for(i=0;i<MAXMOVES;i++)
			{
			values[i]=color==BLACK?-MATE:MATE;
			refnodes[i]=0;
			}
		n=makecapturelist(movelist,color,forcefirst);
		if(n==0)
			{
			n=makemovelist(movelist,color,forcefirst,0);
			/* and order movelist completely */
			for(i=1;i<n;i++)
				{
				j=i;
				tmpmove=movelist[i];
				while ( (tmpmove.info)>(movelist[j-1].info) )
					{
					movelist[j]=movelist[j-1];
					j--;
					}
				movelist[j]=tmpmove;
				}
			}
		if(n==0)
			return -MATE+realdepth;
		}
	/* save old hashkey */
	Lkey=Gkey;
	Llock=Glock;

	/* save old material balance */
	l_bm=bm;l_bk=bk;l_wm=wm;l_wk=wk;
	*best=movelist[0];

	for(i=0;i<n;i++)
		{
		togglemove(movelist[i]);
		countmaterial();
		realdepth++;
		/*update the hash key*/
		updatehashkey(movelist[i]);
#ifdef REPCHECK
		Ghistory[realdepth+HISTORYOFFSET]=p;
#endif
		tmpnodes=cake_nodes;
		/********************recursion********************/
#ifndef PVS
		/* normal search: */
		value=-negamax(d-FRAC,color^CC,-beta,-Lalpha,&Lkiller,0);
#endif
#ifdef PVS
		/* my PVS search: */

		if(Lalpha==alpha)
			value=-negamax(d-FRAC,color^CC,-beta,-Lalpha, &Lkiller,0);
		else
			{
			/*not in PV - we expect this move to fail low in a minimal window search */
			value=-negamax(d-FRAC,color^CC,-(Lalpha+1),-Lalpha, &Lkiller,0);
			if(value>Lalpha)
				/*re-search because of fail-high*/
				value=-negamax(d-FRAC,color^CC,-beta,-Lalpha, &Lkiller,0);
			}
#endif
		/******************* end recursion ***************/
		refnodes[i]=cake_nodes-tmpnodes;
		values[i]=value;
		realdepth--;
		togglemove(movelist[i]);
		/* restore the old hash key*/
		Gkey=Lkey;
		Glock=Llock;
		/* restore the old material balance */
		bm=l_bm;bk=l_bk;wm=l_wm;wk=l_wk;
		bestvalue=max(bestvalue,value);

		if(value>=beta) {*best=movelist[i];/*Lalpha=value*/;swap=i;break;}
		if(value>Lalpha) {*best=movelist[i];Lalpha=value;swap=i;}
		}
	/* save the position in the hashtable */
	hashstore(bestvalue,alpha,beta,d,*best,color);
	/* and order the movelist */
	/* first: we order the best move to the front of the list */
	/* order movelist according to value[i]*/
	if(swap!=0)
		{
		tmpmove=movelist[swap];
		tmpnodes=refnodes[swap];
		for(i=swap;i>0;i--)
			{
			movelist[i]=movelist[i-1];
			refnodes[i]=refnodes[i-1];
			}
		movelist[0]=tmpmove;
		refnodes[0]=tmpnodes;
		}
	last=p;
	return bestvalue;
}


#else  /* this is the analysis version of cake++ */
int firstnegamax(int d, int color, int alpha, int beta, struct move *best)
	{
   int i,j,value,swap=0;
   static int n;
   static struct move movelist[MAXMOVES];
   int32 l_bm,l_bk,l_wm,l_wk;
   int32 Lkey,Llock;
   int Lalpha=alpha,Lbeta=beta;
/*   char Lstr[256];*/
   int32 forcefirst=0;
   int32 Lkiller=0;
   static struct pos last;
   struct move tmpmove;
   char Lstr[255];
   static int values[MAXMOVES]; /* holds the values of the respective moves - use to order */
   							 /* and since this is the analysis module, also used to diplay values of
                            all moves */

   if(*play) return 0;
   cake_nodes++;

	if(last.bm==p.bm && last.bk==p.bk && last.wm==p.wm && last.wk==p.wk)
   	/* then we are still looking at the same position - no need to
      	regenerate the movelist */  ;
   else
   	{
   	n=makecapturelist(movelist,color,forcefirst);
      if(n==0)
   		n=makemovelist(movelist,color,forcefirst,0);
   	if(n==0)
   		return -MATE+realdepth;
      }

   /* save old hashkey */
   Lkey=Gkey;
   Llock=Glock;

   /* save old material balance */
   l_bm=bm;l_bk=bk;l_wm=wm;l_wk=wk;
   *best=movelist[0];

   for(i=0;i<n;i++)
   	{
      togglemove(movelist[i]);
      countmaterial();
      realdepth++;
		/*update the hash key*/
      updatehashkey(movelist[i]);

#ifdef REPCHECK
      Ghistory[realdepth+HISTORYOFFSET]=p;
#endif

      /********************recursion********************/
      value=-negamax(d-FRAC,color^CC,-MATE,MATE,&Lkiller,0);
      /*************************************************/
      values[i]=value;
      /* output new list of moves with values in str */

      realdepth--;
      togglemove(movelist[i]);
      /* restore the old hash key*/
      Gkey=Lkey;
      Glock=Llock;
      /* restore the old material balance */
      bm=l_bm;bk=l_bk;wm=l_wm;wk=l_wk;

      sprintf(out,"d %i: ",(int)d/FRAC);
      for(j=0;j<n;j++)
      	{
			movetonotation(p,movelist[j],Lstr,color);
         strcat(out,Lstr);
         strcat(out,":");
         sprintf(Lstr,"%i   ",values[j]);
         strcat(out,Lstr);
         }
      if(value>=beta) {*best=movelist[i];swap=i;/*break;*/}
      if(value>alpha) {*best=movelist[i];swap=i;alpha=value;}
      }
	/* save the position in the hashtable */
   hashstore(alpha,Lalpha,Lbeta,d,*best,color);
   /* and order the movelist with a bubblesort*/
   for(i=0;i<n;i++)
   	{
      for(j=0;j<n-1;j++)
      	{
         if(values[j]<values[j+1])
         	{
            swap=values[j];
            values[j]=values[j+1];
            values[j+1]=swap;
            tmpmove=movelist[j];
            movelist[j]=movelist[j+1];
            movelist[j+1]=tmpmove;
            }
         }
      }
   last=p;
   return alpha;

   }
#endif

int negamax(int d, int color, int alpha, int beta, int32 *protokiller, int truncationdepth)
	/* negamax sets *protokiller to the best move compressed like in the hashtable */
	/* the calling negamax gets a best move in this way. by passing it on in further
	/* calls it can be used as a killer */
	{
	int32 forcefirst=0;
	int32 Lkey,Llock,x;
	int32 Lkiller=0;
#ifdef ETC
	int32 ETCdummy;
#endif

	int i,j,n,value,capture,v1;
	int index, bestmovevalue;
	int l_bm,l_bk,l_wm,l_wk;
	int localalpha=alpha,localbeta=beta,maxvalue=-MATE;
	int dbresult;
	int valuetype;
#ifdef ETC
	int ETCvalue=-MATE,bestETCvalue=-MATE,ETCvaluetype;
#endif
	struct move movelist[MAXMOVES];
	struct move best;//,tmpmove;
	//struct move *movelist;

#ifdef SINGULAREXTENSIONS
	int values[MAXMOVES];
	int sefailed=0;
	int failedlow[MAXMOVES];
#endif

	/* time check */
#ifndef OPLIB
	if((cake_nodes & 0xFFFF)==0)
		{
		if(searchmode==0)
			if( (clock()-start)/CLK_TCK>(4*maxtime)) (*play=1);
		}
#endif

	cake_nodes++;

	/* return if calculation interrupt */
	if(*play) return 0;
	/* stop search if maximal search depth is reached */
	if(realdepth>MAXDEPTH) 
		{
		leafs++;leafdepth+=realdepth;
		return evaluation(color,alpha,beta);
		}
	
	/* search the current position in the hashtable */
	/* only if there is still search depth left! */
	if(d>0)
		{
		if(hashlookup(&value,&valuetype,d,&forcefirst,color))
			{
			/* return value 1: the position is in the hashtable and the depth
			is sufficient. it's value and valuetype are used to shift bounds
			or even cutoff */
			if(valuetype==EXACT)
				return value;
			if(valuetype==LOWER)
				{
				if(value>=beta) return value;
				//if(value>alpha) alpha=value;
				if(value>localalpha) localalpha=value;
				}
			if(valuetype==UPPER)
				{
				if(value<=alpha) return value;
				//if(value<beta) beta=value;
				if(value<localbeta) localbeta=value;
				}
			}
		}
#ifdef REPCHECK
	/* check for repetitions */
	if(bk && wk)
		{
		for(i=realdepth+HISTORYOFFSET-2;i>=0;i-=2)
			{
			if((p.bm^Ghistory[i].bm)) break; /* stop repetition search if move with a man is detected */
			if((p.wm^Ghistory[i].wm)) break;
			if((p.bm==Ghistory[i].bm) && (p.bk==Ghistory[i].bk) && (p.wm==Ghistory[i].wm) && (p.wk==Ghistory[i].wk))
				return 0;                     /* same position detected! */
			}
		}
#endif
	//movelist=&Gmovelist[realdepth][0];
#ifdef CHECKCAPTURESINLINE
	if(testcapture(color)!=0)
		n=makecapturelist(movelist,color,forcefirst);
   else
		n=0;
#else
	/* uninlined capture test */
	n=makecapturelist(movelist,color,forcefirst);
#endif
	capture=n;

	/* check for database use */
#ifdef USEDB
	//allstones=bk+bm+wk+wm;
	if(bk+bm+wk+wm<=maxNdb)
		{
		if(capture==0)
			{
			if(testcapture(color^CC)==0)
				{
				if( (bk+bm>0) && (wk+wm>0))
					{
					/* found a position which should be in the database */
					/* no captures are possible */
					dblookups++;
					dbresult=DBLookup(p,(color)>>1);
					if(dbresult==DRAW)
						{
						leafs++;
						leafdepth+=realdepth;
						return 0;
						}
					if(dbresult==WIN)
						{
						value=dbwineval(color);
						//if (value>=beta)
						if(value>=localbeta) 
							{
							leafs++;
							leafdepth+=realdepth;
							return value;
							}
						}
					}
				}
			}
		}
#endif

	v1=materialeval[bm][bk][wm][wk];
	if (color==WHITE)
		v1=-v1;
	/* v1 now contains the material evaluation at this node */
	/* use this to decide about extensions/truncations */

#ifdef DOEXTENSIONS
	/* truncate or re-expand if the material count is outside / inside eval window */
	//if(v1<alpha...)
	if(v1<localalpha-TRUNCATEVALUE)
		{
		truncationdepth+=Gtruncationdepth;
		d-=Gtruncationdepth; /* forgot G in versions before 2nd september 1.16 and down */
		}
	else
		{
		//if(v1>beta...)
		if(v1>localbeta+TRUNCATEVALUE)
			{
			truncationdepth+=Gtruncationdepth;
			d-=Gtruncationdepth; /* forgot G in versions before 2nd september 1.16 and down */
			}
		else
			{
			/* always? or only if no capture is possible? */
			/*if(n==0)*/     /* always is standard */
				{
				d+=truncationdepth;
				truncationdepth=0;
				}
			}
		}
#endif
	/* now, if d<0 after truncation, the position will be evaluated and the value returned */
	/* however, if positional compensation is around we don't want this to happen. therefore :*/
#ifdef TRUNCATIONTEST
	if(n==0 && d<=0 && truncationdepth>20)
		{
		/* condition for return of eval.*/
		//2 lines below: no 'local'
		value=evaluation(color,localalpha,localbeta);
		if((value>localalpha-TRUNCATETESTVALUE) && (value<localbeta+TRUNCATETESTVALUE))  /* in enlarged window */
			{
			/* re-expand */
			d+=truncationdepth;
			truncationdepth=0;
			}
		}
#endif


	if(n==0)
		{
		if(d<=0)
			{
			if(realdepth>maxdepth) maxdepth=realdepth;
#ifdef EXTENDALL
			if(testcapture(color^CC)!=0)
				/* side not to move has a capture */
				/* now we test if the side to move is outside the window */
				{
				//8 lines below: no 'local'
				if(v1>localbeta+QLEVEL)
					return evaluation(color,localalpha,localbeta);
				if(v1<localalpha-QLEVEL)
					return evaluation(color,localalpha,localbeta);
				/* if the evaluation is in [alpha-QLEVEL,beta+QLEVEL] we extend */
				}
			else
				return evaluation(color,localalpha,localbeta);
#endif
			}
		n=makemovelist(movelist,color,forcefirst,*protokiller);
		}
	if(n==0)
		return -MATE+realdepth;


/* check for single move and extend appropriately */
	if(n==1)
		d+=SINGLEEXTEND;

	/* save old hashkey and old material balance*/
	Lkey=Gkey;
	Llock=Glock;
	l_bm=bm;l_bk=bk;l_wm=wm;l_wk=wk;

	/* for all moves: domove, update hashkey&material, recursion, restore
		material balance and hashkey, undomove, do alphabetatest */
	/* set best move in case of only fail-lows */
	//best=movelist[0];

#ifdef ETC
	/* enhanced transposition cutoffs: do every move and check if the resulting
		position is in the hashtable. if yes, check if the value there leads to a
		cutoff - if yes, we don't have to search */
	if(d>ETCDEPTH)
		{
		for(i=0;i<n;i++)
			{
			/* do move */
			togglemove(movelist[i]);
			/* update hashkey */ /*code below is just: updatehashkey(movelist[i]);*/
			x=movelist[i].bm;
			while(x)
				{
				Gkey ^=hashxors[0][0][lastbit(x)];
				Glock^=hashxors[1][0][lastbit(x)];
				x&=(x-1);
				}
			x=movelist[i].bk;
			while(x)
				{
				Gkey ^=hashxors[0][1][lastbit(x)];
				Glock^=hashxors[1][1][lastbit(x)];
				x&=(x-1);
				}
			x=movelist[i].wm;
			while(x)
				{
				Gkey ^=hashxors[0][2][lastbit(x)];
				Glock^=hashxors[1][2][lastbit(x)];
				x&=(x-1);
				}
			x=movelist[i].wk;
			while(x)
				{
				Gkey ^=hashxors[0][3][lastbit(x)];
				Glock^=hashxors[1][3][lastbit(x)];
				x&=(x-1);
				}
				/* do the ETC lookup:
			with reduced depth and changed color */
			if(hashlookup(&ETCvalue,&ETCvaluetype,d-FRAC,&ETCdummy,(color^CC)))
				{
				/* the position we searched had sufficient depth */
				/* if one of the values we find is > beta we quit! */
				/* code from schaeffer-paper - I find this strange to wrong!*/
				ETCvalue=-ETCvalue;  /* negamax-framework! */
				if(ETCvaluetype==EXACT || ETCvaluetype==UPPER)
					{
					/* if it was an upper bound one iteration on it is a lower bound
					on this iteration level. therefore, in these two cases ETCvalue
					is a lower bound on the value of our position here */
					if(ETCvalue>bestETCvalue) 
						{
						bestETCvalue=ETCvalue;
						best=movelist[i];
						}
					}
				}
			/* restore the hash key*/
			Gkey=Lkey;Glock=Llock;
			/* undo move */
			togglemove(movelist[i]);
			}
		}
	/* ETC lookups finished, test if we have something */
	if(bestETCvalue>=localbeta) 
		{
		/*save in hashtable and return*/
		//hashstore(bestETCvalue,alpha,beta,d,best,color);
		return bestETCvalue;
		}
	if(bestETCvalue>localalpha) 
		localalpha=bestETCvalue;
#endif

/* stuff for singular extensions */
/* set all values to alpha of this node */
#ifdef SINGULAREXTENSIONS
	for(i=0;i<MAXMOVES;i++) values[i]=alpha;
#endif

/*************** main "for all moves do loop starts here ! *******************/
/* up to here it was just lots of other stuff - now we are ready for the work*/

	
	for(i=0;i<n;i++)
		{
		/* for all moves: domove, count material, recursive call, undo move, check condition */
		
		/* movelist contains moves, movelist.info contains their values. 
			have to find the best move! 
			we do this by linear search through the movelist[] array */
		
		index=0;bestmovevalue=-1;
		
		for(j=0;j<n;j++)
			{
			if(movelist[j].info > bestmovevalue)
				{
				bestmovevalue=movelist[j].info;
				index=j;
				}
			}
		/* movelist[index] now holds the best move */
		/* set it's value to -1 so it won't be used any more */
		movelist[index].info=-1;
		togglemove(movelist[index]);
		if(i==0) best=movelist[index];
		/* inline material count */
		if(color==BLACK)
			{
			if(capture)
				{
				wm-=recbitcount(movelist[index].wm);
				wk-=recbitcount(movelist[index].wk);
				}
			if(movelist[index].bk && movelist[index].bm) 
				{bk++;bm--;}
			}
		else
			{
			if(capture)
				{
				bm-=recbitcount(movelist[index].bm);
				bk-=recbitcount(movelist[index].bk);
				}
			if(movelist[index].wk && movelist[index].wm)
				{wk++;wm--;}
			}
		/* end material count */
		/*the above is equivalent to: countmaterial();*/

		realdepth++;
#ifdef REPCHECK
		Ghistory[realdepth+HISTORYOFFSET]=p;
#endif
		/*update the hash key*/
		/*updatehashkey(movelist[i]);*/
		x=movelist[index].bm;
		while(x)
			{
			Gkey ^=hashxors[0][0][lastbit(x)];
			Glock^=hashxors[1][0][lastbit(x)];
			x&=(x-1);
			}
		x=movelist[index].bk;
		while(x)
			{
			Gkey ^=hashxors[0][1][lastbit(x)];
			Glock^=hashxors[1][1][lastbit(x)];
			x&=(x-1);
			}
		x=movelist[index].wm;
		while(x)
			{
			Gkey ^=hashxors[0][2][lastbit(x)];
			Glock^=hashxors[1][2][lastbit(x)];
			x&=(x-1);
			}
		x=movelist[index].wk;
		while(x)
			{
			Gkey ^=hashxors[0][3][lastbit(x)];
			Glock^=hashxors[1][3][lastbit(x)];
			x&=(x-1);
		}
		/********************recursion********************/
		value=-negamax(d-FRAC,color^CC,-beta,-localalpha, &Lkiller,truncationdepth);
		/*************************************************/

#ifdef SINGULAREXTENSIONS
		values[i]=value;
		if(value<=alpha) failedlow[i]=1;
		else failedlow[i]=0;
#endif
		/************** undo the move ***********/
		realdepth--;
		togglemove(movelist[index]);
		/* restore the hash key*/
		Gkey=Lkey;
		Glock=Llock;
		/* restore the old material balance */
		bm=l_bm;bk=l_bk;wm=l_wm;wk=l_wk;
		/************** end undo move ***********/
		
		/* update best value so far */
		maxvalue=max(value,maxvalue);
		/* and set alpha and beta bounds */
		if(maxvalue>=localbeta)
			{
			best=movelist[index];
			break;
			}
		if(maxvalue>localalpha) {localalpha=maxvalue;best=movelist[index];}
		} /* end main recursive loop of forallmoves */

	/* save the position in the hashtable */
	hashstore(maxvalue,alpha,beta,d,best,color);

#ifdef MOKILLER
	/* set the killer move */

	/* maybe change this: only if cutoff move or only if no capture?*/
/*   And I have one suggestion about killer move.  I think after "hashstore", killer move should be assigned new value only there is a cut-off and it is not a capture move. So new code may be

#ifdef MOKILLER
if (value >=beta)
{
    if (color == BLACK && !(best.wm|best.wk))
        *protokiller = ...
.....
}*/
	if(color==BLACK)
		*protokiller=best.bm|best.bk;
	else
		*protokiller=best.wm|best.wk;
#endif

	return maxvalue;
	}



void hashstore(int value, int alpha, int beta, int depth, struct move best,int color)
	{
	/* store position in hashtable
		if the realdepth is < deeplevel it is stored in the 'deep' hashtable,
		else in the shallow hashtable.
		based on the search window, alpha&beta, the value is assigned a valuetype
		UPPER, LOWER or EXACT. the best move is stored in a reduced form */

	int32 index,minindex;
	int mindepth=1000,iter=0;
	int from,to;

	if(depth<0) return;
	hashstores++;

	/* update history table */
	if(color==BLACK)
		{
		from=(best.bm|best.bk)&(p.bm|p.bk);    /* bit set on square from */
		to=  (best.bm|best.bk)&(~(p.bm|p.bk));
		from=lastbit(from);
		to=lastbit(to);
		from&=31;to&=31;
		//if(value>alpha && value<beta) {history[from][to]++;hashstores++;}
		history[from][to]++;
		}
	else
		{
		from = (best.wm|best.wk)&(p.wm|p.wk);    /* bit set on square from */
		to = (best.wm|best.wk)&(~(p.wm|p.wk));
		from=lastbit(from);
		to=lastbit(to);
		from&=31;to&=31;
		history[from][to]++;
		//if(value>alpha && value<beta) {history[from][to]++;hashstores++;}
		}

	if(realdepth < DEEPLEVEL)
		{
		/* its in the "deep" hashtable: take care not to overwrite other entries */
		index=Gkey&HASHMASKDEEP;
		minindex=index;
		while(iter<HASHITER)
			{
			if(deep[index].lock==Glock || deep[index].lock==0)
				/* found an index where we can write the entry */
				{
				deep[index].lock=Glock;
				deep[index].depth=(int16) (depth);
				if(color==BLACK)
					{
					deep[index].best=best.bm|best.bk;
					deep[index].color=BLACK;
					}
				else
					{
					deep[index].best=best.wm|best.wk;
					deep[index].color=WHITE;
					}
				deep[index].value=(sint16)value;
				/* determine valuetype */
				if(value>=beta) {deep[index].valuetype=LOWER;return;}
				if(value>alpha) {deep[index].valuetype=EXACT;return;}
				deep[index].valuetype=UPPER;
				return;
				}
			else
				{
				if(deep[index].depth < mindepth)
					{
					minindex=index;
					mindepth=deep[index].depth;
					}
				}
			iter++;
			index++;
			index=index&HASHMASKDEEP;
			}
			/* if we arrive here it means we have gone through all hashiter
			entries and all were occupied. in this case, we write the entry
			to minindex */
		if(mindepth>(depth)) return;
		deep[minindex].lock=Glock;
		deep[minindex].depth=depth;
		if(color==BLACK)
			deep[minindex].best=best.bm|best.bk;
		else
			deep[minindex].best=best.wm|best.wk;
		deep[minindex].color=color;
		deep[minindex].value=value;
		/* determine valuetype */
		if(value>=beta) {deep[minindex].valuetype=LOWER;return;}
		if(value>alpha) {deep[minindex].valuetype=EXACT;return;}
		deep[minindex].valuetype=UPPER;
		return;
		/* and return */
		}
	else
		{
		index=Gkey&HASHMASKSHALLOW;
		if(shallow[index].depth <= depth)
		/* replace the old entry if the new depth is larger */
			{
			shallow[index].lock=Glock;
			shallow[index].depth=(int16)depth;
			shallow[index].value=(sint16)value;
			if(color==BLACK)
				{
				shallow[index].best=best.bm|best.bk;
				shallow[index].color=BLACK;
				}
			else
				{
				shallow[index].best=best.wm|best.wk;
				shallow[index].color=WHITE;
				}
			/* determine valuetype */

			if(value>=beta) {shallow[index].valuetype=LOWER;return;}
			if(value>alpha) {shallow[index].valuetype=EXACT;return;}
			shallow[index].valuetype=UPPER;
			}
		}
	return;
	}

int hashlookup(int *value, int *valuetype, int depth, int32 *forcefirst, int color)
	{
	/* searches for a position in the hashtable.
	if the position is found and
	if (the stored depth is >= depth to search),
	hashlookup returns 1, indicating that the value and valuetype have
	useful information.
	else
	forcefirst is set to the best move found previously, and 0 returned
	if the position is not found at all, 0 is returned and forcefirst is
	left at zero.
	positions in the deep hashtable are looked for at the appropriate index,
	and also on the (HASHITER-1) next indices, in the shallow hashtable only
	the hashindex is probed.
	this scheme is supposed to store ALL positions close to the root of the
	search tree, as they are more important.*/

	int32 index;
	int iter=0;

	if(realdepth<DEEPLEVEL)
		/* a position in the "deep" hashtable - it's important to find it since */
		/* the effect is larger here! */
		{
		index=Gkey&HASHMASKDEEP;
		while(iter<HASHITER)
			{
			if(deep[index].lock==Glock && (deep[index].color==color))
				{
				/* we have found the position */
				/* move ordering */
				*forcefirst=deep[index].best;
				/* use value if depth in hashtable >= current depth)*/
				if(deep[index].depth>=depth)
					{
					*value=deep[index].value;
					*valuetype=deep[index].valuetype;
					return 1;
					}
				}
			iter++;
			index++;
			index&=HASHMASKDEEP;
			}
		return 0;
		}
	/* use shallow hashtable */
	else
		{
		index=Gkey&HASHMASKSHALLOW;
		if(shallow[index].lock==Glock && (shallow[index].color==color))
			{
			/*found the right entry!*/
			*forcefirst=shallow[index].best;
			/* use value if depth in hashtable >= current depth)*/
			if(shallow[index].depth>=depth)
				{
				*value=shallow[index].value;
				*valuetype=shallow[index].valuetype;
				return 1;
				}
			}
		}
	return 0;
	}


int booklookup(int *value, int *valuetype, int depth, int32 *forcefirst, int color)
	{
	/* searches for a position in the book hashtable.
	*/
	int32 index;
	int iter=0;
	
	struct hashentry *pointer;
	int bucketsize;
	int size;

	pointer=book;
	size=HASHSIZEBOOK;
	bucketsize=BUCKETSIZEBOOK;
	
	
	index=Gkey% size;
	
	while(iter<bucketsize)
		{
		if(pointer[index].lock==Glock && (pointer[index].color==color))
			{
			/* we have found the position */
			/* move ordering */
			*forcefirst=pointer[index].best;
			/* use value if depth in hashtable >= current depth)*/
			if(pointer[index].depth>=depth)
				{
				*value=pointer[index].value;
				*valuetype=pointer[index].valuetype;
				return 1;
				}
			}
		iter++;
		index++;
		index%=size;
		}
	return 0;
	}

void getpv(char *str,int color)
	{
	/* retrieves the principal variation from the 'deep' hashtable:
		looks up the position, finds the hashtable move, plays it, looks
		up the position again etc.
		color is the side to move, the whole PV gets written in *str */

	struct move movelist[MAXMOVES];
	int32 forcefirst;
	int dummy=0;
	int i,n;
	char Lstr[256];
	struct pos Lp;

	Lp=p;
	absolutehashkey();
	sprintf(str," pv: ");
	for(i=0;i<DEEPLEVEL;i++)
		{
		if(!hashlookup(&dummy,&dummy,0, &forcefirst,color)) break;
		n=makecapturelist(movelist,color,forcefirst);
		if(!n)
			n=makemovelist(movelist,color,forcefirst,0);
		if(!n) {p=Lp;absolutehashkey();return;}

		movetonotation(p,movelist[0],Lstr,color);

		strcat(str,Lstr);
		strcat(str," ");
		togglemove(movelist[0]);
		absolutehashkey();
		color=color^CC;
		}
	p=Lp;
	absolutehashkey();
	return;
	}


#ifdef USEDB
int dbwineval(int color)
	{
	/* dbwineval provides a heuristic value to a position which is a win for
		color according to the database

		the heuristic value is higher if...
		-> there are less pieces on the board (encourage exchanges)
		-> the winning side has more kings
		-> the winning side occupies the center with kings
		-> the winning side has a high tempo count for men (encourage crowning)
		*/

	int value;

	value = MATE-100*(bm+bk+wm+wk); 
	/* new in 1.14: modifiers to encourage the stronger side to
	promote and grab the center */

	if(color==BLACK) /* color is the side which is winning now */
		{
		value+=10*bitcount(p.bk);
		value+=2*bitcount(p.bk&CENTER);
		if(p.bm)
			{
			value += bitcount(p.bm&0x000000F0);
			value += 2*bitcount(p.bm&0x00000F00);
			value += 3*bitcount(p.bm&0x0000F000);
			value += 4*bitcount(p.bm&0x000F0000);
			value += 5*bitcount(p.bm&0x00F00000);
			value += 6*bitcount(p.bm&0x0F000000);
			}
		}
	else
		{
		value+=10*bitcount(p.wk);
		value+=2*bitcount(p.wk&CENTER);
		if(p.wm)
			{
			value += bitcount(p.wm&0x0F000000);
			value += 2*bitcount(p.wm&0x00F00000);
			value += 3*bitcount(p.wm&0x000F0000);
			value += 4*bitcount(p.wm&0x0000F000);
			value += 5*bitcount(p.wm&0x00000F00);
			value += 6*bitcount(p.wm&0x000000F0);
			}
		}
	return value;
	}

int dblosseval(int color)
	{
	/* the same purpose as dbwineval, but here, color has a LOSS on the board */
	/* for an extended description, see dbwineval above */

	int value;

	value = -MATE+100*(bm+bk+wm+wk);
	/* new in 1.14: modifiers to encourage the stronger side to
	promote and grab the center */

	if(color==BLACK) /* color is the side which is winning now */
		{
		value-=10*bitcount(p.wk);
		value-=2*bitcount(p.wk&CENTER);
		if(p.wm)
			{
			value-=  bitcount(p.wm&0x000000F0);
			value-=2*bitcount(p.wm&0x00000F00);
			value-=3*bitcount(p.wm&0x0000F000);
			value-=4*bitcount(p.wm&0x000F0000);
			value-=5*bitcount(p.wm&0x00F00000);
			value-=6*bitcount(p.wm&0x0F000000);
			}
		}
	else
		{
		value-=10*bitcount(p.bk);
		value-=2*bitcount(p.bk&CENTER);
		if(p.bm)
			{
			value-=6*  bitcount(p.bm&0x0F000000);
			value-=5*bitcount(p.bm&0x00F00000);
			value-=4*bitcount(p.bm&0x000F0000);
			value-=3*bitcount(p.bm&0x0000F000);
			value-=2*bitcount(p.bm&0x00000F00);
			value-=bitcount(p.bm&0x000000F0);
			}
		}
	return value;
	}
#endif

int evaluation(int color, int alpha, int beta)
	{
	/* evaluation returns the value of the position with color to move,
		and with current search bounds alpha and beta

		evaluation is in fact just a material/endgame database evaluation.
		if the evaluation is outside the alpha-beta search window, extended
		by the amount FINEEVALWINDOW, it returns the material value only.
		else, it calls the fine evaluation function to get a better evaluation
		*/

	int eval;
	int dbresult;
#ifdef EVALOFF
	return 0;
#endif
	leafs++;
	leafdepth+=realdepth;
	/************************* material **************************/
	if(bm+bk==0) return(color==BLACK?(-MATE+realdepth):MATE-realdepth);
	if(wm+wk==0) return(color==BLACK?(MATE-realdepth):-MATE+realdepth);

	#ifdef USEDB
	if(bm+bk+wm+wk<=maxNdb) /* this position is in the database!*/
		{
		dbresult=DBLookup(p,(color)>>1);
		if(dbresult==DRAW)
			return 0;
		if(dbresult==WIN)
			{
			eval=dbwineval(color);
			return eval;
			}
		if(dbresult==LOSS)
			{
			eval=dblosseval(color);
			return eval;
			}
		/*if(dbresult==UNKNOWN)
			{
			we do nothing - so we don't need this clause;
		}*/
		}
#endif
	eval=materialeval[bm][bk][wm][wk];
	if(color==WHITE) eval=-eval;
#ifdef EVALMATERIALONLY
	#ifdef COARSEGRAINING
		eval=(eval/GRAINSIZE)*GRAINSIZE;
	#endif
		return eval;
#endif

	if(( eval>beta+FINEEVALWINDOW) || (eval<alpha-FINEEVALWINDOW) )
		{
#ifdef COARSEGRAINING
		eval=(eval/GRAINSIZE)*GRAINSIZE;
#endif
		return eval;
		}
	eval+=fineevaluation(color);

	/* fudge factor */
	/*if(bm+bk+wm+wk==10) eval*=0.8;
	else
		{
		if(bm+bk+wm+wk==8) 
			eval/=2;
		}*/
#ifdef COARSEGRAINING
	eval=(eval/GRAINSIZE)*GRAINSIZE;
#endif

	
	return eval;
	}

int fineevaluation(int color)
	{
	/* fineevaluation is the positional evaluation of cake++
   	it knows a lot of features, most of the names are self-explanatory.
      it returns the positional evaluation of the position, positive
      is good for black, negative good for white, a man is worth 100.
   */

   /*static int br[32]={0,0,1,1,1,4,4,6,
  				 1,4,18,18,4,6,20,20,
             0,0,1,1,1,8,8,8,
             1,4,18,18,4,8,20,20};*/


   static int tmod[25]={0,2,2,2,2,1,1,1,1,0,0,0,0,-1,-1,-1,-1,-2,-2,-2,-2,-3,-3,-3,-3}; 
	/*static int tmod[25]={0,3,3,3,3,2,2,2,2,1,1,0,-1,-1,-1,-1,-2,-2,-3,-3,-4,-4,-5,-5,-5};*/
   /* about this DEO says in 'moveover': a tempo count of 4 is FATAL with 11 men each.
   	with 10 it is very bad, but not fatal...' */
   /* have to look at this closely again once: tmod is only 1 for 8 stones and 0
      for 10 -> i'd rather increase that to 2 and 1 at the least - hmm that
      sounds good but just doesnt work :-( */
   int stones=bm+wm;
   int kings=bk+wk;
   int allstones=kings+stones;
   int tempo=0;

   /* constant values for the positional features - end in ...val */
   const int dogholeval=5;
   /*const int devsinglecornerval=5;
   const int intactdoublecornerval=5;
   const int devdoublecornerval=4;
   const int oreoval=5;*/
   const int dykeval=2;
   const int kcval=4;
   const int mcval=1; /*1 is standard */
   const int keval=-4;
   const int turnval=3;
   const int crampval=4;
   const int realcrampval=20;
   const int kingtrappedinsinglecornerval=20;
   const int kingtrappedinsinglecornerbytwoval=10;
   const int kingtrappedindoublecornerval=10;

   const int onlykingval=20,roamingkingval=32;
   const int themoveval=2;
   const int promoteinone=27,promoteintwo=24,promoteinthree=18;
   const int unmobileval=8;
   /*const int crampedsinglecornerval=6;*/
   const int realdykeval=5;
   /* and these are the sum of the values of the features, eval=sum of all features */
   int eval=0;
   int n_attacked,n_free,n_all;
   int freebk=bk, freewk=wk;
   int potbk=0, potwk=0;
   int index;
   int brblack,brwhite;
   int doghole=0,dyke=0,balance=0;
   int kc=0,mc=0,ke=0,turn=0,move=0,cramp=0,runaway=0;
   int backrank=0,trappedking=0;
   int onlyking=0,mobility=0;
   /*int devsinglecorner=0,doublecorner=0,oreo=0;*/
   int32 white=p.wm|p.wk;
   int32 black=p.bk|p.bm;
   int32 tmp,free,attacked,squares;


   /************************ positional *************************/
   free=~(p.bm|p.bk|p.wm|p.wk);

   /* organize: things to do only if men, things only if kings, */
   /* things to do only if men are on the board */

   if(stones)
   	{
   	/* back rank */
      brblack=blackbackrankeval[(p.bm&0x000000FF)];
      brwhite=whitebackrankeval[p.wm>>24];
      backrank=brblack-brwhite;
      /*
   	index=p.bm&0x0000000F;
   	if(p.bm & BIT7) index+=16;

   	brblack=br[index];
   	backrank+=brblack;
   	index=0;
   	if(p.wm & BIT31) index++;
   	if(p.wm & BIT30) index+=2;
   	if(p.wm & BIT29) index+=4;
   	if(p.wm & BIT28) index+=8;
   	if(p.wm & BIT24) index+=16;

   	brwhite=br[index];
   	backrank-=brwhite;*/

    /* just this statement outcommented for cakebr */
	 eval+=backrank;
   	/* tempo */
  	 	tempo+=  bitcount(p.bm&0x000000F0);
   	tempo+=2*bitcount(p.bm&0x00000F00);
   	tempo+=3*bitcount(p.bm&0x0000F000);
   	tempo+=4*bitcount(p.bm&0x000F0000);
   	tempo+=5*bitcount(p.bm&0x00F00000);
   	tempo+=6*bitcount(p.bm&0x0F000000);
   	tempo-=  bitcount(p.wm&0x0F000000);
   	tempo-=2*bitcount(p.wm&0x00F00000);
   	tempo-=3*bitcount(p.wm&0x000F0000);
   	tempo-=4*bitcount(p.wm&0x0000F000);
   	tempo-=5*bitcount(p.wm&0x00000F00);
   	tempo-=6*bitcount(p.wm&0x000000F0);
      tempo*=tmod[stones];
      /* wait with tempo evaluation until doghole is done! */

      /* cramping squares */

      if( (p.bm&BIT16) && (p.wm&BIT20) ) cramp+=crampval;
      if( (p.bm&BIT11) && (p.wm&BIT15) ) cramp-=crampval;

      if( (p.bm&BIT15) && ((p.wm&0x00C80000)==0x00C80000) )
      	cramp+=crampval;
      if( (p.wm&BIT16) && ((p.bm&0x00001300)==0x00001300) )
      	cramp-=crampval;
/*       WHITE
   	28  29  30  31
	 24  25  26  27
	   20  21  22  23
	 16  17  18  19
	   12  13  14  15
	  8   9  10  11
	    4   5   6   7
	  0   1   2   3
	      BLACK */

      /* real cramp position - new in 1.14*/
      if( ((p.bm&0x00011100)==0x00011100) && ((p.wm&0x02320000)==0x02320000) && ((free&0x11000000)==0x11000000) )
      	{
         /* black on 8/12/16 */
         if( ((~(p.bm))&0x00000211) == 0x00000211) /* if a black man is on these squares, cramp loses effectivness */
      		cramp+=realcrampval;
         }
      if( ((p.bm&0x00008880)==0x00008880) && ((p.wm&0x004C4000)==0x004C4000) && ((free&0x08800000)==0x08800000) )
      	{
         /* black on 7/11/15 */
         if( ((~(p.bm))&0x00000048) == 0x00000048)
         	cramp+=realcrampval;
         }
      if( ((p.wm&0x01110000)==0x01110000) && ((p.bm&0x00023200)==0x00023200) && ((free&0x00000110)==0x00000110) )
      	{
         /* white on 16/20/24 */
         if( ((~(p.wm))&0x12000000) == 0x12000000)
         	cramp-=realcrampval;
         }
      if( ((p.wm&0x00888000)==0x00888000) && ((p.bm&0x00004C40)==0x00004C40) && ((free&0x00000088)==0x00000088) )
      	{
         /* white on 15/19/23 */
         if( ((~(p.wm))&0x88400000) == 0x88400000)
         	cramp-=realcrampval;
         }
      eval+=cramp;
      /*
       WHITE
   	28  29  30  31
	 24  25  26  27
	   20  21  22  23
	 16  17  18  19
	   12  13  14  15
	  8   9  10  11
	    4   5   6   7
	  0   1   2   3
	      BLACK
*/
		if(bm==wm)
      	{
   		/* balance */
   		index=-3*bitcount(black&0x01010101)-2*bitcount(black&0x10101010)-bitcount(black&0x02020202)+
   			 bitcount(black&0x40404040)+2*bitcount(black&0x08080808)+3*bitcount(black&0x80808080);
   		balance-=abs(index);
   		index=-3*bitcount(white&0x01010101)-2*bitcount(white&0x10101010)-bitcount(white&0x02020202)+
   			 bitcount(white&0x40404040)+2*bitcount(white&0x08080808)+3*bitcount(white&0x80808080);
   		balance+=abs(index);
         eval+=balance;
   		/* the move */
   		if(color==BLACK)
   			{
      		if(bitcount((~free)&0x0F0F0F0F)%2) /* the number of stones in blacks system is odd -> he has the move*/
         		move=themoveval*(24-allstones)/6;
      		else
         		move=-themoveval*(24-allstones)/6;
   			}
   		else
   			{
      		if(bitcount((~free)&0xF0F0F0F0)%2)
      			move=-themoveval*(24-allstones)/6;
      		else
      			move=themoveval*(24-allstones)/6;
      		}
         eval+=move;
         }
      /* center control */
   	mc+=mcval*bitcount(p.bm&CENTER);
   	mc-=mcval*bitcount(p.wm&CENTER);
      eval+=mc;

   	/* doghole */
   	if( (p.bm&BIT1) && (p.wm&BIT8))
      	{
         /* good for black */
         /* count away the tempo effect: */
         /* only if it's good for black! we don't want white to go in the doghole just
         to save tempo */
         if(tmod[stones]>0) tempo+=5*tmod[stones];
         /* and give a penalty - for sure here*/
         doghole+=dogholeval;
         }
   	if( (p.bm&BIT3) && (p.wm&BIT7) )
      	{
         /* count away tempo */
         if(tmod[stones]>0) tempo+=6*tmod[stones];
         /* give a penalty or not? - i don't know... */
         doghole+=dogholeval;
         }
   	if( (p.bm&BIT24) && (p.wm&BIT28) )
      	{
         if(tmod[stones]>0) tempo-=6*tmod[stones];
         doghole-=dogholeval;
         }
   	if( (p.bm&BIT23) && (p.wm&BIT30) )
      	{
         if(tmod[stones]>0) tempo-=5*tmod[stones];
         doghole-=dogholeval;
         }
      eval+=doghole;
      /* now that doghole has been evaluated, we can do the temp eval too */
      eval+=tempo;
/*
       WHITE
   	28  29  30  31
	 24  25  26  27
	   20  21  22  23
	 16  17  18  19
	   12  13  14  15
	  8   9  10  11
	    4   5   6   7
	  0   1   2   3
	      BLACK
*/
		/* real dyke val */
      if(p.bm&BIT17) eval+=realdykeval;
      if(p.wm&BIT14) eval-=realdykeval;
      /* cramped single corner */
      /*if((p.bm&BIT22) && (!kings))
      	{
         if(bitcount(p.wm&0xE0000000)==2)
         	eval+=crampedsinglecornerval;
         }
      if((p.wm&BIT9) && (!kings))
      	{
         if(bitcount(p.bm&0x00000007)==2)
         	eval-=crampedsinglecornerval;
         }  *
   	/* oreo */
   	/*if( (p.bm&BIT1) && (p.bm&BIT2) && (p.bm&BIT5) ) oreo+=oreoval;
   	if( (p.wm&BIT29) && (p.wm&BIT30) && (p.wm&BIT26) ) oreo-=oreoval;
      eval+=oreo; */

   	/* developed single corner */
   	/*if( ((~p.bm)&BIT0) && ((~p.bm)&BIT4) ) devsinglecorner+=devsinglecornerval;
   	if( ((~p.wm)&BIT31) && ((~p.wm)&BIT27) ) devsinglecorner-=devsinglecornerval;
      eval+=devsinglecorner;*/
      /*  double corner evals: intact and developed */
      /*if( p.bm&BIT3 )
        	{
         if( (p.bm&BIT6) || (p.bm&BIT7) ) doublecorner+=intactdoublecornerval;
         }
      if( p.wm&BIT28 )
        	{
         if( (p.wm&BIT24) || (p.wm&BIT25) ) doublecorner-=intactdoublecornerval;
         } */

      /* developed double corner */
      /*if( (!(p.bm&BIT7)) && (!(p.bm&BIT11)) )
      	doublecorner+=devdoublecornerval;
      if( (!(p.wm&BIT24)) && (!(p.wm&BIT20)) )
      	doublecorner-=devdoublecornerval; */
      /*eval+=doublecorner;*/
      eval+=(black2backrankeval[p.bm&0x000000FF]-white2backrankeval[p.wm>>24]);
   	/* dyke or DEOs a-line - encourage an a-line attack.*/
   	dyke+=dykeval*bitcount(p.bm&0x00022400);
   	dyke-=dykeval*bitcount(p.wm&0x00244000);
      eval+=dyke;



	/************* runaway checkers **************************/
   /** in one **/
   	if( (p.bm) & 0x0F000000)
   		{
   		if((p.bm)&(free>>4)&BIT24) {runaway+=promoteinone;potbk++;runaway-=6*tmod[stones];}
      	if((p.bm)&( (free>>3)|(free>>4) ) & BIT25) {runaway+=promoteinone;potbk++;runaway-=6*tmod[stones];}
      	if((p.bm)&( (free>>3)|(free>>4) ) & BIT26) {runaway+=promoteinone;potbk++;runaway-=6*tmod[stones];}
      	if((p.bm)&( (free>>3)|(free>>4) ) & BIT27) {runaway+=promoteinone;potbk++;runaway-=6*tmod[stones];}
      	}

   	if( (p.wm) & 0x000000F0)
   		{
   		if((p.wm)&(free<<4)&BIT7) 						{runaway-=promoteinone;potwk++;runaway+=6*tmod[stones];}
      	if((p.wm)&( (free<<3)|(free<<4) ) & BIT6) {runaway-=promoteinone;potwk++;runaway+=6*tmod[stones];}
      	if((p.wm)&( (free<<3)|(free<<4) ) & BIT5) {runaway-=promoteinone;potwk++;runaway+=6*tmod[stones];}
      	if((p.wm)&( (free<<3)|(free<<4) ) & BIT4) {runaway-=promoteinone;potwk++;runaway+=6*tmod[stones];}
      	}
   	/** in two **/
   	if( (p.bm) & 0x00F00000)
   		{
      	if( (p.bm&BIT20) && !(RA20&white) ) {runaway+=promoteintwo;potbk++;runaway-=5*tmod[stones];}
      	if( (p.bm&BIT21) && (!(RA21L&white) || !(RA21R&white)) ) {runaway+=promoteintwo;potbk++;runaway-=5*tmod[stones];}
      	if( (p.bm&BIT22) && (!(RA22L&white) || !(RA22R&white)) ) {runaway+=promoteintwo;potbk++;runaway-=5*tmod[stones];}
      	if( (p.bm&BIT23) && !(RA23&white) ) {runaway+=promoteintwo;potbk++;runaway-=5*tmod[stones];}
      	}
   	if( (p.wm) & 0x00000F00)
   		{
      	if( (p.wm&BIT8)  && !(RA8&black) ) {runaway-=promoteintwo;potwk++;runaway+=5*tmod[stones];}
      	if( (p.wm&BIT9)  && (!(RA9L&black) || !(RA9R&black))) {runaway-=promoteintwo;potwk++;runaway+=5*tmod[stones];}
      	if( (p.wm&BIT10) && (!(RA10L&black) || !(RA10R&black))) {runaway-=promoteintwo;potwk++;runaway+=5*tmod[stones];}
      	if( (p.wm&BIT11) && !(RA11&black) ) {runaway-=promoteintwo;potwk++;runaway+=5*tmod[stones];}
      	}
   	/** in 3 **/
   	if( (p.bm) & 0x000F0000)
   		{
      	if( (p.bm&BIT16) && !(RA16&white) ) {runaway+=promoteinthree;runaway-=4*tmod[stones];}
      	if( (p.bm&BIT17) && !(RA17&white) ) {runaway+=promoteinthree;runaway-=4*tmod[stones];}
      	if( (p.bm&BIT18) && !(RA18&white) ) {runaway+=promoteinthree;runaway-=4*tmod[stones];}
      	if( (p.bm&BIT19) && !(RA19&white) ) {runaway+=promoteinthree;runaway-=4*tmod[stones];}
      	}
   	if( (p.wm) & 0x0000F000)
   		{
      	if( (p.wm&BIT12) && !(RA12&black) ) {runaway-=promoteinthree;runaway+=4*tmod[stones];}
      	if( (p.wm&BIT13) && !(RA13&black) ) {runaway-=promoteinthree;runaway+=4*tmod[stones];}
      	if( (p.wm&BIT14) && !(RA14&black) ) {runaway-=promoteinthree;runaway+=4*tmod[stones];}
      	if( (p.wm&BIT15) && !(RA15&black) ) {runaway-=promoteinthree;runaway+=4*tmod[stones];}
      	}

      /* bridge situations */
      /* for black */
      if(p.bm&BIT21)
      	{
      	if( (p.wm&BIT28) && (p.wm&BIT30) && (free&BIT29) )
      		{
      		/* left side */
         	if( (free&BIT24) && (free&BIT25) && (p.bm&BIT20) )
         		{runaway+=promoteintwo;runaway-=5*tmod[stones];}
         	/* right side */
         	if( (free&BIT26) && (free&BIT27) && (p.bm&BIT22) )
         		{runaway+=promoteintwo;runaway-=5*tmod[stones];}
         	}
         }
      /* for white */
      if(p.wm&BIT10)
      	{
      	if( (p.bm&BIT1) && (p.bm&BIT3) && (free&BIT2) )
      		{
         	/* left side */
         	if( (free&BIT4) && (free&BIT5) && (p.wm&BIT9) )
         		{runaway-=promoteintwo;runaway+=5*tmod[stones];}
         	/* right side */
         	if( (free&BIT6) && (free&BIT7) && (p.wm&BIT11) )
         		{runaway-=promoteintwo;runaway+=5*tmod[stones];}
         	}
         }
      eval+=runaway;
      } /* end stuff only with men */


   /************** end runaway checkers *************/
 /*
       WHITE
   	28  29  30  31
	 24  25  26  27
	   20  21  22  23
	 16  17  18  19
	   12  13  14  15
	  8   9  10  11
	    4   5   6   7
	  0   1   2   3
	      BLACK
*/


/*************** everything about kings... ********************/
   if(kings)
   	{
   	/* king center control */
   	kc+=kcval*bitcount(p.bk&CENTER);
   	kc-=kcval*bitcount(p.wk&CENTER);
   	/* kings on edge */
   	ke+=keval*bitcount(p.bk&EDGE);
   	ke-=keval*bitcount(p.wk&EDGE);
      eval+=kc;
      eval+=ke;
   	/* free and trapped kings */
      /* king trapped by one man in single corner */
   	if( (p.wk&BIT0) && (p.bm&BIT1))
         {
         if (free&BIT8)
   			{
            freewk--;   /* old code takes care of a king trapped by a man */
            trappedking+=kingtrappedinsinglecornerval;
         	}
         }
   	if( (p.bk&BIT31) && (p.wm&BIT30) )
   		{
         if (free&BIT23)
         	{
            freebk--;
            trappedking-=kingtrappedinsinglecornerval;
            }
         }
         /*
       WHITE
   	28  29  30  31
	 24  25  26  27
	   20  21  22  23
	 16  17  18  19
	   12  13  14  15
	  8   9  10  11
	    4   5   6   7
	  0   1   2   3
	      BLACK */
      /* king assisted by man but trapped by two men */
      if( p.wk&(BIT0|BIT4) )
      	{
         if( p.wm&BIT8)
         	{
            if( (p.bm&(BIT1|BIT5)) == (BIT1|BIT5) )
            	{
               if( free & BIT12 )
               	{
                  trappedking+=kingtrappedinsinglecornerbytwoval;
               	freewk--;
                  }
               }
            }
         }
      if( p.bk&(BIT27|BIT31) )
      	{
         if( p.bm&BIT23)
         	{
            if( (p.wm&(BIT30|BIT26)) == (BIT30|BIT26) )
            	{
               if(free & BIT19) /*evtl ohne das */
               	{
                 	trappedking-=kingtrappedinsinglecornerbytwoval;
               	freebk--;
                  }
               }
            }
         }

    /* king trapped in double corner by two men */
		if(p.bk & BIT28)
      	{
         if((p.wm&BIT24) && (p.wm&BIT29))
         	{
            trappedking-=kingtrappedindoublecornerval;
            freebk--;
            }
         }
      if(p.wk & BIT3)
      	{
         if( (p.bm&BIT2) && (p.bm&BIT7) )
         	{
            trappedking+=kingtrappedindoublecornerval;
            freewk--;
            }
         }
      eval+=trappedking;
 /*
       WHITE
   	28  29  30  31
	 24  25  26  27
	   20  21  22  23
	 16  17  18  19
	   12  13  14  15
	  8   9  10  11
	    4   5   6   7
	  0   1   2   3
	      BLACK
*/


		/* 'absurd' self-trapping */
   	if( (p.bk&BIT31) && (p.wm&BIT30) && (p.bm&BIT27) && (p.bm&BIT23) )
   		{
      	freebk--;
      	eval-=230;
      	}
   	if( (p.wk&BIT0) && (p.bm&BIT1) && (p.wm&BIT4) && (p.wm&BIT8) )
   		{
      	freewk--;
      	eval+=230;
      	}
      } /* end things only if king ! */

   /* only king */
   /* the point of this is to make cake++ sacrifice a man if it gets a king,
      has a strong back rank and gets its king in the center */
      /* sacrificed man=-100
      	king -> +30 total -70
         only king +20 total -50
         strong br +18 total -22
         king in center +32 +10
      king in center has such a high value because cake sometimes sacrifices
      a man for a trapped king which isn't so obviously trapped */

   /* single black king */
   if( ((potbk+freebk)>0) && ((freewk+potwk)==0) )
   	{
      onlyking+=onlykingval;
      onlyking+=(brblack);  /* was -brwhite still */
      if(bk & CENTER)
      	onlyking+=roamingkingval;
      /* without bk&center: less aggressive saccing. tried and good.*/
      }
   /* single white king */
   if( ((potwk+freewk)>0) && ((freebk+potbk)==0) )
   	{
      onlyking-=onlykingval;
      onlyking-=brwhite; /* was +=brblack still */
      /* outcommenting this -> aggressive... */
      if(wk & CENTER)
      	onlyking-=roamingkingval;
      }
   eval+=onlyking;

   /* turn */
   if(color==BLACK) turn+=turnval;
   else turn-=turnval;
   eval+=turn;
 /*
       WHITE
   	28  29  30  31
	 24  25  26  27
	   20  21  22  23
	 16  17  18  19
	   12  13  14  15
	  8   9  10  11
	    4   5   6   7
	  0   1   2   3
	      BLACK
*/
  /* attacked squares - simple mobility check */
   tmp=p.wk|p.wm;
   attacked=lb1(tmp)|lb2(tmp)|rb1(tmp)|rb2(tmp);
   attacked|=lf1(p.wk)|lf2(p.wk)|rf1(p.wk)|rf2(p.wk);
   /* attack contains all squares where white pieces could move to */
   tmp=p.bm|p.bk;
   squares=lf1(tmp)|lf2(tmp)|rf1(tmp)|rf2(tmp);
   squares|=lb1(p.bk)|lb2(p.bk)|rb1(p.bk)|rb2(p.bk);
   /* squares contains all squares where black pieces can go to */
   /* if there are few squares which black pieces can occupy where
		they are not attacked, thats bad */
   n_free=bitcount(squares&(~attacked));
   n_attacked=bitcount(squares&attacked);
   n_all=bitcount(squares);
   mobility+=n_free-n_attacked;
   if(color==BLACK)
   	{
      if(n_free<2 && n_all>5) mobility-=unmobileval;
      if(n_free==0) mobility-=unmobileval;
      }
   tmp=p.bk|p.bm;
   attacked=lf1(tmp)|lf2(tmp)|rf1(tmp)|rf2(tmp);
   attacked|=lb1(p.bk)|lb2(p.bk)|rb1(p.bk)|rb2(p.bk);
   /* attack contains all squares where black pieces could move to */
   tmp=p.wm|p.wk;
   squares=lb1(tmp)|lb2(tmp)|rb1(tmp)|rb2(tmp);
   squares|=lf1(p.wk)|lf2(p.wk)|rf1(p.wk)|rf2(p.wk);
   /* squares contains all squares where white pieces can go to */
   /*if there are few squares which black pieces can occupy where
      they are not attacked, thats bad */
   n_free=bitcount(squares&(~attacked));
   n_attacked=bitcount(squares&attacked);
   n_all=bitcount(squares);
   mobility-=n_free-n_attacked;
   if(color==WHITE)
   	{
      if(n_free<2 &&n_all>5) mobility+=unmobileval;
      if(n_free==0) mobility+=unmobileval;
      }
   eval+=mobility;
   return color==BLACK ? eval:-eval;
   }



/* recursive bitcount */
int recbitcount(int32 n)
   /* counts & returns the number of bits which are set in a 32-bit integer
   	slower than a table-based bitcount if many bits are
      set. used to make the table for the table-based bitcount on initialization
   */
	{
   int r=0;
   while(n)
   	{
      n=n&(n-1);
      r++;
      }
   return r;
   }

/* table-lookup bitcount */
int bitcount(int32 n)
	/* returns the number of bits set in the 32-bit integer n */
  	{
	return (bitsinword[n&0x0000FFFF]+bitsinword[(n>>16)&0x0000FFFF]);
   }

int lastbit(int32 x)
	{
   /* returns the position of the last bit in x */

   if(x&0x000000FF)
   	return(lastone[x&0x000000FF]);
   if(x&0x0000FF00)
   	return(lastone[(x>>8)&0x000000FF]+8);
   if(x&0x00FF0000)
   	return(lastone[(x>>16)&0x000000FF]+16);
   if(x&0xFF000000)
   	return(lastone[(x>>24)&0x000000FF]+24);
   return 1000;
   }

void updatehashkey(struct move m)
	{
   /* given a move m, updatehashkey calculates the new hashkey (Glock, Gkey)
   	incrementally */
   int32 x;
   x=m.bm;
   while(x)
   	{
      Gkey ^=hashxors[0][0][lastbit(x)];
      Glock^=hashxors[1][0][lastbit(x)];
      x&=(x-1);
      }
   x=m.bk;
   while(x)
   	{
      Gkey ^=hashxors[0][1][lastbit(x)];
      Glock^=hashxors[1][1][lastbit(x)];
      x&=(x-1);
      }
   x=m.wm;
   while(x)
   	{
      Gkey ^=hashxors[0][2][lastbit(x)];
      Glock^=hashxors[1][2][lastbit(x)];
      x&=(x-1);
      }
   x=m.wk;
   while(x)
   	{
      Gkey ^=hashxors[0][3][lastbit(x)];
      Glock^=hashxors[1][3][lastbit(x)];
      x&=(x-1);
      }
   }

void absolutehashkey(void)
	{
   /* absolutehashkey calculates the hashkey completely. slower than
   	using updatehashkey, but useful sometimes */

   int32 x;

   Glock=0;
   Gkey=0;
   x=p.bm;
   while(x)
   	{
      Gkey ^=hashxors[0][0][lastbit(x)];
      Glock^=hashxors[1][0][lastbit(x)];
      x&=(x-1);
      }
   x=p.bk;
   while(x)
   	{
      Gkey ^=hashxors[0][1][lastbit(x)];
      Glock^=hashxors[1][1][lastbit(x)];
      x&=(x-1);
      }
   x=p.wm;
   while(x)
   	{
      Gkey ^=hashxors[0][2][lastbit(x)];
      Glock^=hashxors[1][2][lastbit(x)];
      x&=(x-1);
      }
   x=p.wk;
   while(x)
   	{
      Gkey ^=hashxors[0][3][lastbit(x)];
      Glock^=hashxors[1][3][lastbit(x)];
      x&=(x-1);
      }

   }

/* inline capture :*/
int testcapture(int color)
	{
	/* test for captures inline to speed things up:*/
   /* testcapture returns 1 if color has a capture on the global position p */
   int32 black,white,free,m;

   if (color==BLACK)
   	{
      black=p.bm|p.bk;
      white=p.wm|p.wk;
      free=~(black|white);
      m =((((black&LFJ2)<<4)&white)<<3);
      m|=((((black&LFJ1)<<3)&white)<<4);
      m|=((((black&RFJ1)<<4)&white)<<5);
      m|=((((black&RFJ2)<<5)&white)<<4);
      if(p.bk)
      	{
         m|=((((p.bk&LBJ1)>>5)&white)>>4);
      	m|=((((p.bk&LBJ2)>>4)&white)>>5);
   		m|=((((p.bk&RBJ1)>>4)&white)>>3);
   		m|=((((p.bk&RBJ2)>>3)&white)>>4);
         }
      if(m & free)
         return 1;
      return 0;
      }
   else
   	{
      black=p.bm|p.bk;
      white=p.wm|p.wk;
      free=~(black|white);
      m=((((white&LBJ1)>>5)&black)>>4);
      m|=((((white&LBJ2)>>4)&black)>>5);
   	m|=((((white&RBJ1)>>4)&black)>>3);
      m|=((((white&RBJ2)>>3)&black)>>4);
      if(p.wk)
      	{
         m|=((((p.wk&LFJ2)<<4)&black)<<3);
      	m|=((((p.wk&LFJ1)<<3)&black)<<4);
      	m|=((((p.wk&RFJ1)<<4)&black)<<5);
      	m|=((((p.wk&RFJ2)<<5)&black)<<4);
         }
      if(m & free)
         return 1;
   	return 0;
      }
  	}

void countmaterial(void)
	{
   /* countmaterial initializes the globals bm, bk, wm and wk, which hold
   	the number of black men, kings, white men and white kings respectively.
      during the search these globals are updated incrementally */
   bm=bitcount(p.bm);
   bk=bitcount(p.bk);
   wm=bitcount(p.wm);
   wk=bitcount(p.wk);
   }


void movetonotation(struct pos pos,struct move m, char *str, int color)

	{

   /* make a notation out of a move */
   /* m is the move, str the string, color the side to move */

   int from, to;

   char c;

 /*

       WHITE
   	28  29  30  31           32  31  30  29
	 24  25  26  27           28  27  26  25
	   20  21  22  23           24  23  22  21
	 16  17  18  19           20  19  18  17
	   12  13  14  15           16  15  14  13
	  8   9  10  11           12  11  10   9
	    4   5   6   7            8   7   6   5
	  0   1   2   3            4   3   2   1
	      BLACK
*/
	static int square[32]={4,3,2,1,8,7,6,5,
   					 12,11,10,9,16,15,14,13,
                   20,19,18,17,24,23,22,21,
                   28,27,26,25,32,31,30,29}; /* maps bits to checkers notation */

   if(color==BLACK)
   	{
      if(m.wk|m.wm) c='x';
        	else c='-';                      /* capture or normal ? */
      from=(m.bm|m.bk)&(pos.bm|pos.bk);    /* bit set on square from */
      to=  (m.bm|m.bk)&(~(pos.bm|pos.bk));
      from=lastbit(from);
      to=lastbit(to);
      from=square[from];
      to=square[to];
      sprintf(str,"%2i%c%2i",from,c,to);
      }
   else
   	{
      if(m.bk|m.bm) c='x';
      else c='-';                      /* capture or normal ? */
      from=(m.wm|m.wk)&(pos.wm|pos.wk);    /* bit set on square from */
      to=  (m.wm|m.wk)&(~(pos.wm|pos.wk));
      from=lastbit(from);
      to=lastbit(to);
      from=square[from];
      to=square[to];
      sprintf(str,"%2i%c%2i",from,c,to);
      }
   return;
   }

void printboardtofile(struct pos p)
	{
   int i;
   int free=~(p.bm|p.bk|p.wm|p.wk);
   int b[32];
   char c[15]="-wb      WB";

   for(i=0;i<32;i++)
   	{
      if((p.bm>>i)%2)
      	b[i]=BLACK;
      if((p.bk>>i)%2)
      	b[i]=BLACK|KING;
      if((p.wm>>i)%2)
      	b[i]=WHITE;
      if((p.wk>>i)%2)
      	b[i]=WHITE|KING;
      if((free>>i)%2)
      	b[i]=0;
      }
   /*        WHITE
 	   7  15  23  31
 	 3  11  19  27
	   6  14  22  30
  	 2  10  18  26
	   5  13  21  29
    1   9  17  25
 	   4  12  20  28
  	 0   8  16  24
        BLACK     */

	fprintf(cake_fp,"\n\n");
   fprintf(cake_fp,"\n %c %c %c %c",c[b[28]],c[b[29]],c[b[30]],c[b[31]]);
   fprintf(cake_fp,"\n%c %c %c %c ",c[b[24]],c[b[25]],c[b[26]],c[b[27]]);
   fprintf(cake_fp,"\n %c %c %c %c",c[b[20]],c[b[21]],c[b[22]],c[b[23]]);
   fprintf(cake_fp,"\n%c %c %c %c ",c[b[16]],c[b[17]],c[b[18]],c[b[19]]);
   fprintf(cake_fp,"\n %c %c %c %c",c[b[12]],c[b[13]],c[b[14]],c[b[15]]);
   fprintf(cake_fp,"\n%c %c %c %c ",c[b[8]],c[b[9]],c[b[10]],c[b[11]]);
   fprintf(cake_fp,"\n %c %c %c %c",c[b[4]],c[b[5]],c[b[6]],c[b[7]]);
   fprintf(cake_fp,"\n%c %c %c %c ",c[b[0]],c[b[1]],c[b[2]],c[b[3]]);
   fflush(cake_fp);
   }

void printboard(struct pos p)
	{
   int i;
   int free=~(p.bm|p.bk|p.wm|p.wk);
   int b[32];
   char c[15]="-wb      WB";
   for(i=0;i<32;i++)
   	{
      if((p.bm>>i)%2)
      	b[i]=BLACK;
      if((p.bk>>i)%2)
      	b[i]=BLACK|KING;
      if((p.wm>>i)%2)
      	b[i]=WHITE;
      if((p.wk>>i)%2)
      	b[i]=WHITE|KING;
      if((free>>i)%2)
      	b[i]=0;
      }
   /*        WHITE
 	   7  15  23  31
 	 3  11  19  27
	   6  14  22  30
  	 2  10  18  26
	   5  13  21  29
    1   9  17  25
 	   4  12  20  28
  	 0   8  16  24
        BLACK     */

    printf("\n\n");
    printf("\n %c %c %c %c",c[b[28]],c[b[29]],c[b[30]],c[b[31]]);
    printf("\n%c %c %c %c ",c[b[24]],c[b[25]],c[b[26]],c[b[27]]);
    printf("\n %c %c %c %c",c[b[20]],c[b[21]],c[b[22]],c[b[23]]);
    printf("\n%c %c %c %c ",c[b[16]],c[b[17]],c[b[18]],c[b[19]]);
    printf("\n %c %c %c %c",c[b[12]],c[b[13]],c[b[14]],c[b[15]]);
    printf("\n%c %c %c %c ",c[b[8]],c[b[9]],c[b[10]],c[b[11]]);
    printf("\n %c %c %c %c",c[b[4]],c[b[5]],c[b[6]],c[b[7]]);
    printf("\n%c %c %c %c ",c[b[0]],c[b[1]],c[b[2]],c[b[3]]);
   }
#ifdef OPLIB
void boardtobitboard(int b[8][8], struct pos *position)
	{
   /* initialize bitboard */
   int i,board[32];
   /*
       WHITE
   	28  29  30  31
	 24  25  26  27
	   20  21  22  23
	 16  17  18  19
	   12  13  14  15
	  8   9  10  11
	    4   5   6   7
	  0   1   2   3
	      BLACK
*/
   board[0]=b[0][0];board[1]=b[2][0];board[2]=b[4][0];board[3]=b[6][0];
	board[4]=b[1][1];board[5]=b[3][1];board[6]=b[5][1];board[7]=b[7][1];
	board[8]=b[0][2];board[9]=b[2][2];board[10]=b[4][2];board[11]=b[6][2];
	board[12]=b[1][3];board[13]=b[3][3];board[14]=b[5][3];board[15]=b[7][3];
	board[16]=b[0][4];board[17]=b[2][4];board[18]=b[4][4];board[19]=b[6][4];
	board[20]=b[1][5];board[21]=b[3][5];board[22]=b[5][5];board[23]=b[7][5];
	board[24]=b[0][6];board[25]=b[2][6];board[26]=b[4][6];board[27]=b[6][6];
	board[28]=b[1][7];board[29]=b[3][7];board[30]=b[5][7];board[31]=b[7][7];

   position->bm=0;
   position->bk=0;
   position->wm=0;
   position->wk=0;

   for(i=0;i<32;i++)
   	{
      switch (board[i])
      	{
         case BLACK|MAN:
            position->bm=position->bm|(1<<i);
            break;
         case BLACK|KING:
         	position->bk=position->bk|(1<<i);
         	break;
         case WHITE|MAN:
         	position->wm=position->wm|(1<<i);
            break;
         case WHITE|KING:
         	position->wk=position->wk|(1<<i);
            break;
         }
      }
	}
#endif

int32 myrand(void)
	{
	/* myrand produces a 32-bit random number */
	/* rand() returns a 15-bit random number, not 16,
		that's the reason i only take tha last 8 bits instead of 16*/
	int32 result=0;
	result+=(rand() & 0xFF);
	result=result<<8;
	result+=(rand() & 0xFF);
	result=result<<8;
	result+=(rand() & 0xFF);
	result=result<<8;
	result+=(rand() & 0xFF);
	return result;
	}

/* old singular extensions stuff - just to keep it... */
#ifdef SINGULAREXTENSIONS
	/* do the singularextension test */
	/* singular move: all nodes are below alpha except one, which is in window */
	/* condition: n>1 else there's not much point in this! */
	if(bestvalue>alpha && bestvalue <beta && capture==0) /* found a move above alpha */
		{
		v1=-MATE;
		k=0;
		for(i=0;i<n;i++)
			{
			if(values[i]>v1)
				{
				k=i;
				v1=values[i];
				}
			}

		/* do a quick test if bestvalue is in window */
		if(bestvalue < beta)
			{
			v1=-MATE;
			for(i=0;i<n;i++)
				{
				if(i==k) continue;
				if(failedlow[i]==0)
					v1=max(v1,values[i]);
				}
			/* v1 contains the value of the second-best move which did not fail low */
			if(bestvalue-SELEVEL<v1)
				sefailed=1;
			}
		if(sefailed==0)
			{
			for(i=0;i<n;i++)
				{
				/* index k is the best move - do not check again */
				if(i==k) continue;
				/* domove */
				togglemove(movelist[i]);
				countmaterial();
				updatehashkey(movelist[i]);
				realdepth++;
#ifdef REPCHECK
				Ghistory[realdepth+HISTORYOFFSET]=p;
#endif
				/* recursion */
				/* do an alphabeta-test with alpha=bestvalue-SELEVEL,beta=bestvalue-SELEVEL+1
					if this does not fail low - the move is not singular
					-negamax(-beta,-alpha) */
				value=-negamax(d-FRAC,color^CC,-(bestvalue-SELEVEL+1),-(bestvalue-SELEVEL), &Lkiller,truncationdepth);
				/* undomove */
				realdepth--;
				togglemove(movelist[i]);
				/* restore the hash key*/
				Gkey=Lkey;Glock=Llock;
				/* restore the old material balance */
				bm=l_bm;bk=l_bk;wm=l_wm;wk=l_wk;
				if(value>bestvalue-SELEVEL) /*se failed */
					{sefailed=1;break;}
				} /* end for */
			}
		/* now we do our singular extension */
		if(sefailed==0)
			{
			singular++;
			/* domove */
			togglemove(movelist[k]);
			countmaterial();
			updatehashkey(movelist[k]);
			realdepth++;
#ifdef REPCHECK
			Ghistory[realdepth+HISTORYOFFSET]=p;
#endif
			value=-negamax(d-FRAC+SEDEPTH,color^CC,-beta,-alpha, &Lkiller,truncationdepth);
			/* undomove */
			realdepth--;
			togglemove(movelist[k]);
			/* restore the hash key*/
			Gkey=Lkey;Glock=Llock;
			/* restore the old material balance */
			bm=l_bm;bk=l_bk;wm=l_wm;wk=l_wk;
			bestvalue=value;
			}
		}
#endif

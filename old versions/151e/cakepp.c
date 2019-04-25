/*
 *		cake - a checkers engine			
 *															
 *		Copyright (C) 2002 by Martin Fierz	
 *														
 *		contact: checkers@fierz.ch				
 */
// 1.51e: used ed gilberts parseindexfile code to improve speed with which cake loads dramatically.
//		also changed that cake will not prune any more in db positions. f
//		broke something somewhere which makes cake 151e see 16-19 in the white doctore 2 at d27
//		instead of d25 as 1.51d :-(

// 1.51d: fixed a problem in the 2-level-pruning code: 
//		i was doing truncationdepth += delta/truncdivsoft
//		but depth -= delta/truncdivhard. so i was pruning more than i would restore... this fixes
//		the problem with the gemini test position where 16-20 draws. damn! so many errors! this
//		one was because of TRUNCDIV and TRUNCDIV2 which were too similar. i renamed them to 
//		TRUNCDIVSOFT and TRUNCDIVHARD...													

/* 1.51c made noprunewindow 100: now it has no more problems with the chinook-lafferty position. not true!

/* 1.51b made qlevel window asymmetric 60/200 instead of 60/60. costs ~5% in op/mid, 2% in endgame

/* 1.51a fixed a bug in the eval which made a runaway running into being a trapped/dominated king
		seem like a great idea.


/* 1.51	found a couple of interesting things in las vegas. in the game cake lost to nemesis, 
		it played 11-15? with a big score and lost. this seemed to be a pruning problem. solution:
		get the evaluation to return a value 'delta' which the search uses for the pruning. normally,
		soft pruning starts at +-30 outside the window, hard pruning at +-100. however, now the eval
		returns a delta which is added to these limits. i have made delta=NOPRUNEWINDOW=50
		for all man-down positions,
		with or without kings.

		


/* 1.50 aka 'las vegas'
		is the version used at the tournament in las vegas.
		except: added an eval for a weird smother position shown in las vegas
		by vic habgood (whilter smother).

		Cake las Vegas 6-piece - Cake++ 1.41
		+:45 =:212 -:24 unknown:7 +B:13 -B:19
		athlon 1.4GHz, 5s/move, 16MB hash each.

/* 1.43 uses my own database now.
		improvements: newtruncation scheme streamlined, reusing the evaluation we got for
		the truncation to return, makes cake++ with newtruncation about same fast as old
		cake++ with old truncation and about 20-25% faster than old new truncation.

		have to test a few things: what is good Qlevel?

		fixed a bug in the eval of two 4-3 cramps.

		reversed preload list, so that now 2-pc db is at the head of the list when
		it is loaded, and 8-piece db is at the end. probably a todo is to throw out
		the 7-piece db from the preload.

		changed time management:
			1) maxtime is the nominal search time
			2) aborttime is the time where the search is aborted
			
/* 1.42 ("las vegas") todo: evaluation:	-> get positional compensation for man down to work
							-> better king eval generalized mobility
							-> better man eval in endgame generalized mobility
			  
				-> introduce new variables in fineevaluation: untrappedbk, untrappedwk.
					they are the kings xored with all special cases of trapped kings.
					on these untrapped kings, i do a mobility evaluation.

				-> more todo: change book file format!
				-> add conditional db lookup!
				-> change db read to 32K

		-> done: new truncation scheme which uses the evaluation at a node instead
				 of the material evaluation.
				 saw good results while truncating when off by only a value of 30. 
				 (great match results, particularly in deeper searches, i.e. 5s/move
				 on the duron900 instead of 2s search on the P3 450.) however, also
				 saw awful misevaluations in ladder match against tom canning. this
				 means that this version was not useful. turned up the value on which
				 truncation starts to 80, and made cutoff more aggressive - still get
				 good match results (but not quite as good), but the evaluation problems
				 are gone/smaller. 
				 -> trying: a double truncation scheme: if it's off by 30-80, use truncationdepth2
				    to cut search depth a bit, if it's off by more than 80, use truncationdepth.
					if eval comes back in +-80 window, reset truncationdepth, if it comes back in
					+-30 reset truncationdepth2. why? i think problems with evaluation errors 
					came from the fact that large truncations were not reset because they never
					returned in the +-30 range. this new scheme should combine the best of
					both worlds.
				-> settings for 1.423 - which is quite good: newtruncation on, TRUNCLEVELHARD 80,
					TRUNCDIVHARD 16. no second truncation.

/* 1.41 (26th january ff 2002 - 11th february 2002)
		"the knowledge revolution"

		Cake++ 1.41 - KingsRow 1.12l by Ed Gilbert
		+:49 =:190 -:38 unknown:2 +B:17 -B:34
	
		Cake++ 1.41 NT - Nemesis Beta 0.1 by Murray Cash
		+:49 =:183 -:56 unknown:0 +B:18 -B:39

		-> implemented:	-> 4th position 

						-> key formations P 168/169

						-> dynamically trapped white king on square 3 by men on 8,12

						-> new code for back rank when saccing a man requires that
						   all three squares 1,2,3 are occupied
 
						-> new code for saccing a man for positional compensation does
						   not work - yet, keep that for later.

/*   version history
	1.402 (19th december 2001)
			new king proximity evaluation - good if there is a king close to opponent's men which are
			not on the edge - they may be in danger. +42-42 -> +43-40 vs. Kingsrow 1.12f

	1.401 (12th december 2001)
			cake++ now saves its state in the registry (in cakedll.c)
			node counter is a double, so that large node numbers can be processed correctly.

	1.4 lots of changes in evaluation function, with versions 1.35 etc, new release on internet
		as version 1.4. this version of cake is much better than 1.3!


  1.35 (28th september ff)
		a match nemesis-cake shows me some glaring mistakes in eval - try to fix!
			ENDINGS
			-> self-trapping in double corner	(done)

			-> dominated king in single corner	(done)
				+63 -45 (laptop) instead of +58 -46 (cake1.3, a +6 here!)
				  -> make this true only if dominated king is alone (or maybe one other?).

			OPENING
			-> increase value of squares 6,7 (black)
			-> make back rank 2,3,6 equally strong as 2,3,5.
			
			-> work on cramping squares 13,20:
				-> for white, it is good to have a man on 20 if there is a black man on 16. 
					it is indifferent if there is a black man on 12. 
					it is bad if there is neither
				-> for white, it is good to have a man on 13 if there is a black man on 9
					it is indifferent if there is one on 5
					it is bad if there is none of the two
			-> cramped single corner = too many men in single corner 
				-> penalty for men on 4, 8
				-> penalty for too #men on 3,4,8,11,12,16>=5.
			
			MIDGAME
			-> make man sac weak if not king & CENTER.

		general: -> make a complete PV possible with triangle array
					-> analyze hashtable deep to see if it's really always open
					-> make 64 bit counters.

	1.3:			lots of evaluation tuning:
					-> the move is of no use, even if calculated only in 8piece endings
					-> the a-line attack was counterproductive!
					-> changed criterium for roaming king from &CENTER to 
						&ROAMINGBLACK/WHITEKING, which is a larger part of the
						board
					->	increased dogholeval
					-> added dogholemandownval
					-> and lots more which i dont remember!
					-> added flexible book size


	1.3beta2:	->	stop relying on global variable p (at least most of the time...)
					->	found serious bug in eval in the kingsacrifice code,
						roamingking was bk&CENTER instead of p->bk :-(
					->	add eval for bridge stone which can be a weakness if
						opponent has kings
					-> change move to if(wk+wm==bk+bm)

	1.3beta (22nd may ff 2001)
		-> changes struct hashentry to 8 bytes
			strange, for some reason this doesnt work quite as well as before :-(
		
		-> worked on eval: 
			->changed back rank eval, 
			->increased single king eval, so sacs more easily
			->added an king center monopoly to eval

			this was a HUGE improvement: engine match results:
				cake++1.3 cake++ 1.21b	+64=182-35u7 (2s/K7-600)
				cake++1.3 kr 1.10c		+48=187-51u2 (2s/PIV1.4)

		->  adds new stuff to evaluation function:
			-> men at sides of board running against good back rank on one side
			->maybe: ->king center monopoly (KCM) only if both sides have a king
						-> higher value for KCM?
						->sac less probable if less men on board
						->influence of king?
						->8piece eval?
						->one king holding two men in endgame?  OKOK
						->check for doghole stones before calculating the move? OKOK
						->keep men on 6 & 7 if there are men on 2&3 or 1&2 respectively OKOK
						->bridge stone on 10/23 may be weak!
		-> :
		-> add book really this time
		-> clean up extension management (keep track of original depth and save that?)
		-> check move generator for pointer use instead of pass by value

	1.22 (13th may 2001)
		-> cake++ can use a book of tom lincke now
		-> fixed a bug with MOSTATIC and MOTESTCAPT
		
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

		... NEVER DONE!
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
#include <assert.h>

#include <conio.h>
#include <windows.h>

/* structs.h defines the data structures for cake++ */
#include "structs.h"

/* consts.h defines constants used by cake++ */
#include "consts.h"

/* function prototypes */
#include "cakepp.h"
#include "move_gen.h"

#include "dblookup.h"


/* globals */
int32 cake_nodes;
int searchmode;
int realdepth;
unsigned int leafs,leafdepth;
int bm,bk,wm,wk;

#ifdef USEDB
int maxNdb=0; /* the largest number of stones which is still in the database */
#endif


int maxdepth;				/* maximal depth, in ply, reached by search */
int32 Gkey,Glock;			/* global hash key */
int dblookups, dbsuccess, dbfail;	/* number of dblookups in current search */
static int dbcachehits;
int *play;			/*is nonzero if the engine is to play immediately */

int firstmoveafterbook=1;

int hashstores;
static struct hashentry *hashtable;
static struct bookhashentry *book;

struct pos p;		/* holds the current position */

int hashmegabytes=8;
int usethebook=0;
static char *out;

double start,t,maxtime,aborttime; /* time variables */

#ifdef REPCHECK
struct pos Ghistory[MAXDEPTH+HISTORYOFFSET+10]; /*holds the current variation for repetition check*/
#endif

/* a file pointer for logging */
static int cakeisinit=0;
int logging;
static FILE *cake_fp;
/* history table */
#ifdef MOHISTORY
int32 history[32][32];
#endif
//static int32  hashxors[2][4][32];
int32  hashxors[2][4][32];
/* bit tables for last bit in byte, and number of set bits in word */
//static int lastone[256];
static unsigned char bitsinword[65536];
/* material value lookup table */
static short /*sint16*/ materialeval[13][13][13][13]; /*bmbkwmwk*/
static char /*sint8*/blackbackrankeval[256],whitebackrankeval[256];//,black2backrankeval[256],white2backrankeval[256];
static char blackbackrankpower[256], whitebackrankpower[256]; // used for man down situations
static int norefresh;
int rootalpha, rootbeta;

int bookentries; /* number of book positions - read from cakebook.bin first */

#ifdef TESTPERF
int TP_deepnotstored=0;
int cutoffsat[MAXMOVES];
#endif

#ifdef PV
int pv[MAXDEPTH][MAXDEPTH];
#endif

#ifdef ROOTCHECK
int rootnodes=0;
#endif

#ifdef GETLONGEST
struct move current[MAXDEPTH];
struct move longest[MAXDEPTH];
#endif

#ifdef DBCACHE
struct dbhashentry dbcache[DBCACHESIZE]; // store the last positions you looked up
#endif

#ifdef TESTPERF
int analyzeHT(void)
	{
	int i, empty=0, occupied=0;
	FILE *fp;
	char lStr[1024];

	for(i=0;i<hashsize;i++)
		{
		if(deep[i].lock==0) empty++;
		else occupied++;
		}
	fp=fopen("perfanalysis.txt","a");
	sprintf(lStr,"\ndeep hashtable: %i occupied, %i empty, %i fails", occupied, empty,TP_deepnotstored);
	printf("%s",lStr);
	fprintf(fp,"%s",lStr);
	fclose(fp);
	}
#endif


int hashsize = HASHSIZE;


/*----------------------------------------------------------------------------*/
/*																										*/
/*                                interface                                   */
/*																										*/
/*----------------------------------------------------------------------------*/

/* consists of initcake() exitcake() and getmove() */

int initcake(int log, char str[1024])
	/* initcake must be called before any calls to cake_getmove can be made.
		it initializes the database and some constants, like the arrays for lastone
		and bitsinword, plus some evaluation tables */
	{
	int i,j,k,l;
	int v1,v2;
	int32 u,index;
	FILE *Lfp;
	int32 *xor;


	/* constants for xoring to make hashcode */
	/* the hash 'lock' has it's last two bits cleared to store the color */
	/* like this in old code: hashxors[1][i][j]&=0xFFFFFFFC; */
	int32 xors[256]= {690208388, 1385187825, 3014056764, 610143516,
							3004321224, 520313469, 1574226940, 319654154,
							546984174, 306787062, 2907641074, 1808019639,
							4129460637, 2966067458, 746618565, 3001731532,
							3205813886, 2213500745, 1640878366, 1927683568,
							47704121, 303969140, 3567244452, 841151692,
							1221824373, 3854628651, 241127424, 2063930631,
							410030810, 2235459861, 2524122841, 3572976202,
							4195793681, 2046710491, 4098216920, 3311425853,
							1982714050, 1231714597, 2973461157, 3601402759,
							354261653, 2129242266, 1421843175, 2419599569,
							1582227760, 2682430946, 1698179654, 2054584514,
							3660148030, 884947366, 1829692268, 499892739,
							252970341, 3320261774, 4087909886, 4288729147,
							2272817997, 3686942273, 2138108071, 2858117139,
							3401481631, 4256714600, 2580402957, 2791723827,
							1582099459, 3023113898, 3391619032, 2032621890,
							3448538912, 2274708656, 2582068657, 2902999529,
							2682525855, 4266447137, 2406699840, 3396326863,
							1522427175, 2713609638, 842271020, 1455872925,
							2290169953, 4183085287, 227682589, 3681819898,
							2962577397, 2373245323, 2784393661, 3772866703,
							1533184650, 3850848827, 2227618737, 4000673539,
							273718216, 2331437470, 829214116, 2358776063,
							1472680025, 52985693, 877551676, 1118965035,
							3090117602, 3632583455, 383140641, 2552966190,
							3169896974, 2457120653, 1584021756, 392902321,
							3933347458, 3093223758, 2490637685, 1188754011,
							3407689449, 3534494219, 1079747379, 2917214933,
							3847937257, 1774533228, 3951532767, 3588662470,
							3280983594, 68223256, 476105248, 1163078103,
							3928419669, 737687480, 1548679839, 2240709040,
							3782006444, 4055624168, 2265726616, 112674780,
							1136364452, 154674460, 2532656440, 484159024,
							1081620220, 2659011036, 1145071312, 842768672,
							3783991256, 3111296328, 2580932992, 1405052884,
							769429092, 1644128652, 2644252272, 2219788916,
							743689160, 2651649236, 1501443532, 3486748716,
							3872988456, 2021754692, 3563163068, 993231388,
							1683279536, 4165364228, 372205416, 3495389300,
							2913870984, 625206588, 684592116, 2535950936,
							3385682132, 140591124, 287883660, 2542546928,
							3248218560, 2810626740, 3643003220, 3647775076,
							2633947504, 211530820, 2309587576, 1672554204,
							1658885648, 1065967344, 681647648, 1061226372,
							455631304, 1177946332, 3203591024, 1963363452,
							3502146236, 730659376, 1418080916, 199687412,
							2307539224, 4134697748, 4031003316, 197894416,
							1282663940, 2916079024, 2635353588, 2090648056,
							593803428, 1519210028, 1625207168, 1732355540,
							356646948, 76754864, 1133162364, 3550582220,
							720766892, 3033120872, 1291947796, 3366377480,
							1801982560, 1897351288, 2250873200, 1329051828,
							3749735532, 1130696968, 4159378872, 1849457652,
							2401721588, 849043776, 2804959760, 798911212,
							2168662312, 1201766728, 2665382304, 2588134860,
							3216841764, 2304751944, 3856532120, 3054824632,
							1367244060, 29249640, 2199459096, 2348355292,
							1030591536, 3047461076, 2155217480, 568032600,
							2373036172, 2952445872, 3856404568, 2174779228,
							1232339540, 4155057696, 3107293288, 2989645064,
							2002643892, 2476215636, 73970336, 3161939852,
							555002324, 2377038704, 3479033388, 2824457228,
							1317045464, 3990190752, 570178612, 2109730480};

	static int br[32]={0,0,2, 2, 4,6,10,10, /* last two were 10,10, but pdntool insists that this br is good!*/
								1,4,16,16,6,10,20,16,
								0,0,2, 2, 4,6,16,16,
								1,4,16,16,6,10,20,16};
	/* contains the back rank evaluation */
	/* strange: pdntool insists that the back rank 0 x 0 x (bridge) is worse than 0 x x 0 which
		leaves the double corner open....*/
	/* should try a change there! */
	const int devsinglecornerval=5;
	const int intactdoublecornerval=5;
	const int oreoval=5; //5 before 1.35
	/* new 1.3*/
	const int idealdoublecornerval = 4; //4 before 1.35 
	
	sprintf(str,"creating logfile");
	/* create a new logfile / delete old logfile */
	logging=log;
	if(log & 1)
		/* delete old logfile */
		{
		cake_fp=fopen("cakelog.txt","w");
		}

/* load book */
	
	sprintf(str,"loading book");
#ifdef BOOK
	Lfp=fopen("cakebook.bin","rb");
	if(Lfp!=NULL)
		{
		fscanf(Lfp,"%i",&bookentries);
#ifdef WINMEM
		book = VirtualAlloc(0, bookentries*sizeof(struct bookhashentry), MEM_COMMIT, PAGE_READWRITE);
#else
		book=malloc(bookentries*sizeof(struct bookhashentry));
#endif
		if(book!=0)
			{
			fread(book,sizeof(struct bookhashentry),bookentries,Lfp);
			if(log&1)
				fprintf(cake_fp,"book hashtable with %i entries allocated\n",bookentries);
			}
		else
			{
			if(log&1)
				fprintf(cake_fp,"malloc failure in initcake\n");
			}
		fclose(Lfp);
		}
	else
		{
		/* book file not found */
		book = NULL;
		if(log&1) fprintf(cake_fp,"no opening book detected\n");
		}
#endif // BOOK
	/* allocate memory for the hashtables			*/
	/* and check for success						*/


	sprintf(str,"allocating hashtable");
#ifdef WINMEM
	hashtable = VirtualAlloc(0, (hashsize+HASHITER)*sizeof(struct hashentry), MEM_COMMIT, PAGE_READWRITE);
#else
	hashtable=malloc((hashsize+HASHITER)*sizeof(struct hashentry));
#endif

	if(hashtable==NULL)
		{
		if(log&1) fprintf(cake_fp,"malloc failure in initcake\n");
		exit(0);
		}
	
	
	if(log & 1)
		fclose(cake_fp);
	
	/* initialize xors */
	/* fixed xors */
	xor=hashxors;
	for(i=0;i<256;i++)
		xor[i]=xors[i];

	
	/* initialize array for "lastone" */
/*	for(i=0;i<256;i++)
		{
		if(i&BIT0) {lastone[i]=0;continue;}
		if(i&BIT1) {lastone[i]=1;continue;}
		if(i&BIT2) {lastone[i]=2;continue;}
		if(i&BIT3) {lastone[i]=3;continue;}
		if(i&BIT4) {lastone[i]=4;continue;}
		if(i&BIT5) {lastone[i]=5;continue;}
		if(i&BIT6) {lastone[i]=6;continue;}
		if(i&BIT7) {lastone[i]=7;continue;}
		}*/
	/* initialize bitsinbyte */
	
	for(i=0;i<65536;i++)
		bitsinword[i]=recbitcount((int32)i);
	
#ifdef USEDB
	sprintf(str, "initializing database");

#ifdef SIX
	maxNdb=6;
#endif
#ifdef EIGHT
	maxNdb=8;
#endif

	initdblookup(str);
#endif // USEDB
	
	/* initialize material values */
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
					
					/* give a bonus for 2-1 kings */
					/* don't want to go to 3-2 kings if you have 2-1!*/
					/*if(j==2 && l==1)
						v1+=10;
					if(j==1 && l==2)
						v1-=10;*/

					materialeval[i][j][k][l]=v1;
					}
				}
			}
		}
	/* initialize backrankeval  */

	/* 		WHITE
		28  29  30	31
	  24  25  26  27
		20  21  22	23
	  16  17  18  19
		12  13  14	15
	   8  9	  10  11
		 4   5	6	 7
	   0   1   2   3
			BLACK */

  /* static int br[32]={0,0,2, 2, 4,6,10,10,
								1,4,16,16,6,10,20,16,
								0,0,2, 2, 4,6,16,16,
								1,4,16,16,6,10,20,16};*/

	/* static int br[32]={0,0,2, 2, 4,6,16,16,
								1,4,16,16,6,10,20,16,
								0,0,2, 2, 4,6,16,16,
								1,4,16,16,6,10,20,16};*/
	for(u=0;u<256;u++)
		{
		/* for all possible back ranks do: */
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
	 
	/* 		WHITE
		28  29  30	31
	  24  25  26  27
		20  21  22	23
	  16  17  18  19
		12  13  14	15
	   8  9	  10  11
		 4   5	6	 7
	   0   1   2   3
			BLACK */

		// backrankpower gives the power of the back rank to withstand onslaught
		// by enemy men - only used in the case of man down.
		if(match1(p.bm, 0xE))
			blackbackrankpower[u] = 30;
		if(match1(p.bm, 0x2E) || match1(p.bm, 0x4E))
			blackbackrankpower[u] = 35;
		if(match1(p.bm, 0x6E))
			blackbackrankpower[u] = 40;

		if(match1(p.wm, 0x70000000))
			whitebackrankpower[u] = 30;
		if(match1(p.wm, 0x72000000) || match1(p.wm, 0x74000000))
			whitebackrankpower[u] = 35;
		if(match1(p.wm, 0x76000000))
			whitebackrankpower[u] = 40;

		/* oreo */
		if( (p.bm&BIT1) && (p.bm&BIT2) && (p.bm&BIT5) ) blackbackrankeval[u]+=oreoval; //2!!
		if( (p.wm&BIT29) && (p.wm&BIT30) && (p.wm&BIT26) ) whitebackrankeval[u]+=oreoval; //2!!

	
		/* developed single corner */

/*		if(p.bm & BIT0) black2backrankeval[u]-=singlecornerpenaltyval;
		if(p.bm & BIT4) black2backrankeval[u]-=singlecornerpenaltyval;
		if(p.wm & BIT31) white2backrankeval[u]-=singlecornerpenaltyval;
		if(p.wm & BIT27) white2backrankeval[u]-=singlecornerpenaltyval;*/
		if( ((~p.bm)&BIT0) && ((~p.bm)&BIT4) ) blackbackrankeval[u]+=devsinglecornerval; //2!!
		if( ((~p.wm)&BIT31) && ((~p.wm)&BIT27) ) whitebackrankeval[u]+=devsinglecornerval;//2!!
		
		/*  double corner evals: intact and developed */
		if( p.bm&BIT3 )  /* for black */
			{
			if( (p.bm&BIT6) || (p.bm&BIT7) ) blackbackrankeval[u]+=intactdoublecornerval; //2!!
			/*if( (p.bm&BIT6)  ) intactdoublecorner+=intactdoublecornerval;  */
			}
		if( p.wm&BIT28 ) /* for white */
			{
			if( (p.wm&BIT24) || (p.wm&BIT25) ) whitebackrankeval[u]+=intactdoublecornerval;//2!!
			/*if( (p.wm&BIT25) ) intactdoublecorner-=intactdoublecornerval; */
			}
		/* ideal double corner */
/* 		WHITE
		28  29  30	31
	 24  25	26  27
		20  21  22	23
	 16  17	18  19
		12  13  14	15
	   8	9	10  11
    	 4   5	6	 7
	  0	1	 2   3
			BLACK */
		if( (p.bm&BIT3) && (p.bm&BIT2) && (p.bm&BIT6) && (!(p.bm&BIT7)))
			blackbackrankeval[u]+=idealdoublecornerval;//2!!
		// maybe also allow 2,6,7 &!3 to be ideal?
		if( (p.wm&BIT28) && (p.wm&BIT29) && (p.wm&BIT25) && (!(p.wm&BIT24)))
			whitebackrankeval[u]+=idealdoublecornerval;//2!!

		// new 1.41'
		if( (p.bm&BIT7) && (p.bm&BIT2) && (p.bm&BIT6) && (!(p.bm&BIT3)))
			blackbackrankeval[u]+=intactdoublecornerval;//2!!
		// maybe also allow 2,6,7 &!3 to be ideal?
		if( (p.wm&BIT24) && (p.wm&BIT29) && (p.wm&BIT25) && (!(p.wm&BIT28)))
			whitebackrankeval[u]+=intactdoublecornerval;//2!!

		
		}
	cakeisinit=1;
	return 1;
	}

int hashreallocate(int x)
	{
	// reallocates the hashtable to x MB
	//FILE *fp;

	static int currenthashsize;

	struct hashentry *pointer;
	int newsize;

	//fp = fopen("realloc.txt","a");
	hashmegabytes = x;
	if(hashmegabytes>=256) 
		hashmegabytes = 255;
	newsize = (hashmegabytes)*1024*1024/sizeof(struct hashentry);	
	//fprintf(fp,"\nreallocation: size %i MB %i bytes (old;%i)",hashmegabytes,newsizeshallow,hashsizeshallow);
	//fflush(fp);
#ifdef WINMEM
	VirtualFree(hashtable, 0, MEM_RELEASE);
	pointer = VirtualAlloc(0, (newsize+HASHITER)*sizeof(struct hashentry), MEM_COMMIT, PAGE_READWRITE);
#else
	pointer=realloc(hashtable,(newsize+HASHITER)*sizeof(struct hashentry));
#endif
	if(pointer != NULL)
		{
		hashsize = newsize;
		hashtable = pointer;
		}

	// but what do we do when pointer == 0 here??

	//todo!
	//fprintf(fp,"\npointer %i",pointer);fflush(fp);
	//fclose(fp);
	return 1;
	}

int exitcake(void)
	{
	/*   fclose(cake_fp);*/
	/* deallocate memory for the hashtables */
	free(hashtable);
	free(book);
	return 1;
	}


#ifdef BOOKGEN

int bookgen(struct pos *position, int color, int *numberofmoves, int values[MAXMOVES], int how, int depth, double searchtime)
	{
	int playnow=0;
	int d;
	int value=0,lastvalue=0,n,i,j, guess;
	struct move best,last, movelist[MAXMOVES];
	struct pos dummy;
	char Lstr[1024],str[1024];
	char tempstr[1024];
	int bookfound=1,bookbestvalue=-MATE;
	struct move bookmove;
	int bookequals=0;
	int bookdepth=0;
	int bookindex=-1,booklookupvalue;
	int reset=1;
	int forcedmove = 0; // indicates if a move is forced, then cake++ will play at depth 1.
	char pvstring[1024];
	char beststring[1024];
	int issearched[MAXMOVES];
	int bestvalue;
	int index;
	int bestnewvalue;

	play=&playnow;
	out=str;

	
	aborttime=40000*searchtime;
	maxtime = searchtime;
	searchmode=how;

	// initialize module if necessary
	if(!cakeisinit) initcake(logging,str);

	p=(*position);
	
	// initialize material 
	countmaterial(&p);

#ifdef MOHISTORY
	// reset history table 
	memset(history,0,32*32*sizeof(int));
#endif

	printboard(p);
	if(color==BLACK)
		printf("\nblack to move\n");
	else
		printf("\nwhite to move\n");


	// clear the hashtable 
	memset(hashtable,0,(hashsize+HASHITER)*sizeof(struct hashentry));
	
	// reset all counters,
	//	nodes, database lookups etc 
	cake_nodes=0;
	dblookups=0;
	dbsuccess=0;
	dbfail=0;
	realdepth=0;
	maxdepth=0;
	hashstores=0;
	norefresh=0;

	n=makecapturelist(&p,movelist,values, color, 0);
	if(!n)
		n=makemovelist(&p,movelist,values,color,0,0);
#ifdef REPCHECK
	// initialize history list: holds the last few positions of the current game 
	Ghistory[HISTORYOFFSET]=p;
	dummy.bm=BIT0;
	dummy.wm=BIT0;
	dummy.bk=BIT0;
	dummy.wk=BIT0;

	for(i=0;i<HISTORYOFFSET+8;i++)
		Ghistory[i]=dummy;
#endif
	
	/* initialize hash key */

	absolutehashkey(&p);
	countmaterial(&p);
	//n=makecapturelist(&p,movelist, values,color, 0);


	start=clock();
	guess=0;

	for(d=1;d<MAXDEPTH;d+=2)
		{

		leafs=0;
		leafdepth=0;
		value = -MATE;
		printf("\n");

		for(i=0;i<n;i++)
			issearched[i]=0;

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

			togglemove(p,movelist[index]);
			absolutehashkey(&p);
			countmaterial(&p);
			//printboard(p);
			//getch();
			/* choose which search algorithm to use: */
			/* non - windowed search */
			/*value=firstnegamax(FRAC*d, color, -MATE, MATE, &best);*/

	#ifndef MTD
			values[index]=-windowsearch(FRAC*(d-1), color^CC, 0 /*guess*/, &best);
			// set value for next search 
			//guess=value[i];	
	#endif
	#ifdef MTD
			/* MTD(F) search */
			values[index]=-bookmtdf(-values[index],FRAC*(d-1),color^CC,&best,-bestnewvalue);
			bestnewvalue = max(bestnewvalue,values[index]);
			//guess=value;	
	#endif
			value = max(value, values[index]);
			
			togglemove(p,movelist[index]);
			movetonotation(p,movelist[index],Lstr,color);
			printf("[%s %i] ",Lstr,values[index]);
			if(value == values[index])
				sprintf(beststring,"%s",Lstr);
			
			}


		/* generate output */
		sprintf(Lstr,"%s",beststring);
		getpv(pvstring,color);
		t=clock();
	
		if((t-start)>0)
			sprintf(str,"%s depth %i/%i/%2.1f nodes %lu value %i time %3.2fs %4.0fkN/s db %i (%.1f)",Lstr,d,maxdepth,(float)leafdepth/((float)leafs+0.01),cake_nodes,value,(t-start)/CLK_TCK,cake_nodes/1000/(t-start)*CLK_TCK,dblookups,100.0*(double)dbsuccess/((double)dbsuccess+(double)dbfail));
		else
			sprintf(str,"%s depth %i/%i/%2.1f nodes %lu value %i time %3.2fs ?kN/s db %i (%.1f)",Lstr,d,maxdepth,(float)leafdepth/((float)leafs+0.01),cake_nodes,value,(t-start)/CLK_TCK,dblookups,100*(double)dbsuccess/((double)dbsuccess+(double)dbfail));
		printf("\n%s",str);	
		/* iterative deepening loop break conditions: depend on search mode, 'how':
			how=0: time mode;*/
		if(d>1)
			{

			if( how==TIME_BASED && ((clock()-start)/CLK_TCK>(maxtime/2)) ) 
				break;
			if( how == DEPTH_BASED && d>=depth)
				break;
			if(abs(value)>MATE-100) break;
			}

		lastvalue=value;	// save the value for this iteration 
		last=best;			// save the best move on this iteration 
		norefresh=1;
		}

	*position=p;

	return value;
}


int bookmtdf(int firstguess,int depth, int color, struct move *best, int bestvalue)
	{
	int g,lowerbound, upperbound,beta;
	double time;
	char Lstr1[1024],Lstr2[1024];

	
	/*g=0; */ /* strange - this seems to help?! */
	upperbound=MATE;
	lowerbound=-MATE;

	
	g=firstguess;
	g=0;

	while(lowerbound<upperbound)
		{
		if(g==lowerbound)
			beta=g+1;
		else beta=g;
		rootalpha=beta-1;
		rootbeta=beta;
		g=firstnegamax(depth,color,beta-1,beta,best);
		
		if(g<beta)
			{
			upperbound=g;
			sprintf(Lstr1,"value<%i",beta);
			}
		else
			{
			lowerbound=g;
			sprintf(Lstr1,"value>%i",beta-1);
			}
		
		time = ( (clock()-start)/CLK_TCK);
		getpv(Lstr2,color);

		sprintf(out,"depth %i/%i/%.1f  time %.2fs  %s  nodes %i  %ikN/s  %s", 
			(depth/FRAC),maxdepth,(float)leafdepth/((float)leafs+0.01) , time, Lstr1, 
			cake_nodes, (int)((float)cake_nodes/time/1000),Lstr2); 
#ifdef FULLLOG
		fprintf(cake_fp,"\n%s",out);
		fflush(cake_fp);
#endif		
		}
#ifdef CONFIRMPV
	g=firstnegamax(depth,color,g-1,g+1,best);
	time = ( (clock()-start)/CLK_TCK);
	getpv(Lstr2,color);
	sprintf(Lstr1,"value=%i",g);
	sprintf(out,"depth %i/%i/%.1f  time %.2fs  %s  nodes %i  %ikN/s  %s", 
			(depth/FRAC),maxdepth, (float)leafdepth/((float)leafs+0.01) , time,Lstr1,
			cake_nodes, (int)((float)cake_nodes/time/1000),Lstr2); 
#else
	sprintf(Lstr1,"value=%i",g);
	sprintf(out,"depth %i/%i/%.1f  time %.2fs  %s  nodes %i  %ikN/s  %s", 
			(depth/FRAC),maxdepth,(float)leafdepth/((float)leafs+0.01) , time, Lstr1, 
			cake_nodes, (int)((float)cake_nodes/time/1000),Lstr2); 
#endif

	if(cake_fp != NULL)
		{
		fprintf(cake_fp,"\n%s\n",out);	
		fflush(cake_fp);
		}
	return g;
	}

#endif //bookgen

int cake_getmove(struct pos *position,int color, int how,double maximaltime,
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
	/*		maxtime and depthtosearch and maxnodes are used for these three search modes.
	/*		cake++ prints information in str
	/*		if playnow is set to a value != 0 cake++ aborts the search.
	/*		if (logging&1) cake++ will write information into "log.txt"
	/*		if(logging&2) cake++ will also print the information to stdout.
	/*		if reset!=0 cake++ will reset hashtables and repetition checklist
	/*		reset==0 generally means that the normal course of the game was disturbed
	// info currently has uses for it's first 3 bits:
	// info&1 means reset
	// info&2 means exact time level
	// info&4 means increment time level
	/*
	/*----------------------------------------------------------------------------*/

	int d;
	int value=0,lastvalue=0,n,i, guess;
	struct move best,last, movelist[MAXMOVES];
	struct pos dummy;
	char Lstr[1024];
	char tempstr[1024];
	int bookfound=1,bookbestvalue=-MATE;
	struct move bookmove;
	int bookequals=0;
	int bookdepth=0;
	int bookindex=-1,booklookupvalue;
	int values[MAXMOVES];
	int reset=(info&1);
	int forcedmove = 0; // indicates if a move is forced, then cake++ will play at depth 1.
	char pvstring[1024];

	play=playnow;
	out=str;
	logging=log;

	if(info&1)
		firstmoveafterbook=1;

	// set time variables
	maxtime=maximaltime;
	aborttime=4*maxtime;

	// if exact:
	if(info&2)
		aborttime = maxtime;

	// if increment level
	if(info&4)
		{
		if(firstmoveafterbook)
			{
			maxtime=0.25*maximaltime;
			aborttime=0.6*maximaltime;
			}
		else
			{
			maxtime=0.15*maximaltime;
			aborttime = 0.4*maximaltime;
			}
		}


	searchmode=how;

	// initialize module if necessary
	if(!cakeisinit) initcake(logging,str);

	p=(*position);
	if(logging & 1)
		{
		cake_fp=fopen("cakelog.txt","a");
		printboardtofile(p);
		fprintf(cake_fp,"\nposition hex bm bk wm wk color:\n{0x%8.8x, 0x%8.8x, 0x%8.8x, 0x%8.8x, %i}",p.bm,p.bk,p.wm,p.wk,color);
		}

	// initialize material 
	countmaterial(&p);

#ifdef MOHISTORY
	// reset history table 
	memset(history,0,32*32*sizeof(int));
#endif

#ifdef PV
	// clear PV array 
	for(i=0;i<MAXDEPTH;i++)
		{
		for(j=0;j<maxdepth;j++)
			pv[i][j]=-1;
		}
#endif


	// clear the hashtable 
	memset(hashtable,0,(hashsize+HASHITER)*sizeof(struct hashentry));
	
	// reset all counters,
	//	nodes, database lookups etc 
	cake_nodes=0;
	dblookups=0;
	dbsuccess=0;
	dbfail=0;
	realdepth=0;
	maxdepth=0;
	hashstores=0;
	norefresh=0;
#ifdef ROOTCHECK
	rootnodes=0;
#endif

	n=makecapturelist(&p,movelist,values, color, 0);

#ifdef REPCHECK
	// initialize history list: holds the last few positions of the current game 
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

#ifdef TESTPERF
	for(i=0;i<MAXMOVES;i++)
		cutoffsat[i]=0;
#endif
	
	/* initialize hash key */
	absolutehashkey(&p);

		/* do a book lookup */
//#ifdef BOOK	
	if(usethebook)
		{
		bookfound=0;
		bookindex=0;
		
		if(booklookup(&booklookupvalue,0,&bookdepth,color,&bookindex))
			{
			
			bookfound=1;
			
			/* this position is there, now, do all moves and look up their values */
			/* possibly, this position does not have all followups, so it is not sure
				that we really have it!*/
			n=makecapturelist(&p,movelist,values,color,0);
			if(!n)
				n=makemovelist(&p,movelist,values,color,0,0);
			/* set best value*/
						
			bookmove = movelist[bookindex];
			

			}
		/* if the remaining depth is too small, we dont use the book move */
		if(bookdepth<BOOKMINDEPTH) bookfound =0;
		if(bookfound)
			{
			movetonotation(p,bookmove,Lstr,color);
			sprintf(str,"found position in book, value=%i, move is %s (depth %i)\n",booklookupvalue,Lstr,bookdepth);
			best=bookmove;
			value=0;
			firstmoveafterbook=1;
			}
		else
			{
			sprintf(str,"%X %X not found in book\n",Gkey,Glock);
			value=0;
			}
		if(logging&1)
			{
			fprintf(cake_fp,"\n%s",str);
			}
		}
	else
		bookfound=0;


	absolutehashkey(&p);
	countmaterial(&p);
	//n=makecapturelist(&p,movelist, values,color, 0);

	forcedmove = isforced(&p,color);

	start=clock();
	guess=0;
	if(!bookfound)
		{
		if(firstmoveafterbook && info&4)
			{
			fprintf(cake_fp,"\nfirst non-book move\n");
			firstmoveafterbook=0;
			}

		for(d=1;d<MAXDEPTH;d+=2)
			{

			leafs=0;
			leafdepth=0;
	
			/* choose which search algorithm to use: */
		
			/* non - windowed search */
			/*value=firstnegamax(FRAC*d, color, -MATE, MATE, &best);*/
	
			/*******************/
			/* windowed search */
			/* standard!       */
			/*******************/
#ifndef MTD
			value=windowsearch(FRAC*d, color, guess, &best);
			// set value for next search 
			guess=value;	
#endif
#ifdef MTD
			/* MTD(F) search */
			value=mtdf(guess,FRAC*d,color,&best);
			guess=value;	
#endif

			/* generate output */
			movetonotation(p,best,Lstr,color);
			getpv(pvstring,color);
			t=clock();
			/*
			if((t-start)>0)
				sprintf(str,"%s depth %i/%i/%2.1f nodes %lu value %i time %3.2fs %4.0fkN/s db %i (%.1f)",Lstr,d,maxdepth,(float)leafdepth/((float)leafs+0.01),cake_nodes,value,(t-start)/CLK_TCK,cake_nodes/1000/(t-start)*CLK_TCK,dblookups,100.0*(double)dbsuccess/((double)dbsuccess+(double)dbfail));
			else
				sprintf(str,"%s depth %i/%i/%2.1f nodes %lu value %i time %3.2fs ?kN/s db %i (%.1f)",Lstr,d,maxdepth,(float)leafdepth/((float)leafs+0.01),cake_nodes,value,(t-start)/CLK_TCK,dblookups,100*(double)dbsuccess/((double)dbsuccess+(double)dbfail));
				*/
			/*getpv(Lstr,color);
			strcat(str,Lstr);*/
			if(logging&1)
				{
				//fprintf(cake_fp,"\n%s",str);
				//fprintf(cake_fp,"  %s",pvstring);
				if(d>10) fflush(cake_fp);
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

				if( how==TIME_BASED && ((clock()-start)/CLK_TCK>(maxtime/2)) ) 
					break;

				if(how==DEPTH_BASED && d>=depthtosearch)
					break;
				if(how==NODE_BASED && cake_nodes>maxnodes)
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
				if(abs(value)>MATE-100) break;
				}
			if(*play)
				{
				// the search has been aborted, either by the user or by cake++ because 
				// abort time limit was exceeded
				// stop the search. don't use the best move & value because they might be rubbish 
				best=last;
				movetonotation(p,best,Lstr,color);
				value=lastvalue;
#ifndef ANALYSISMODULE
				sprintf(str,"interrupt: best %s value %i",Lstr,value);
#endif
				break;
				}
			lastvalue=value;	// save the value for this iteration 
			last=best;			// save the best move on this iteration 
			norefresh=1;
			}
		}
#ifdef REPCHECK
	Ghistory[HISTORYOFFSET-2]=p;
#endif
	if(!bookfound)
		{
		getpv(Lstr,color);


		strcat(str,"\n");
		strcat(str,Lstr);
		sprintf(tempstr,"");
		strcat(tempstr,str);
		//sprintf(str,tempstr);

		}
	if(!(*play))
		{togglemove(p,best);}
	else
		{togglemove(p,last);}
	if(logging&1)
		{
		//fprintf(cake_fp,"%s\n",Lstr);
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

	/*--------------------------------------------------------------------------
	|																			|
	|		driver routines for search: MTD(f) and windowsearch					|
	|																			|
	 --------------------------------------------------------------------------*/


int mtdf(int firstguess,int depth, int color, struct move *best)
	{
	int g,lowerbound, upperbound,beta;
	double time;
	char Lstr1[1024],Lstr2[1024];

	g=firstguess;
	/*g=0; */ /* strange - this seems to help?! */
	upperbound=MATE;
	lowerbound=-MATE;
	while(lowerbound<upperbound)
		{
		if(g==lowerbound)
			beta=g+1;
		else beta=g;
		rootalpha=beta-1;
		rootbeta=beta;
		g=firstnegamax(depth,color,beta-1,beta,best);
		
		if(g<beta)
			{
			upperbound=g;
			sprintf(Lstr1,"value<%i",beta);
			}
		else
			{
			lowerbound=g;
			sprintf(Lstr1,"value>%i",beta-1);
			}
		
		time = ( (clock()-start)/CLK_TCK);
		getpv(Lstr2,color);

		sprintf(out,"depth %i/%i/%.1f  time %.2fs  %s  nodes %i  %ikN/s  %s", 
			(depth/FRAC),maxdepth,(float)leafdepth/((float)leafs+0.01) , time, Lstr1, 
			cake_nodes, (int)((float)cake_nodes/time/1000),Lstr2); 
#ifdef FULLLOG
		fprintf(cake_fp,"\n%s",out);
		fflush(cake_fp);
#endif		
		}
#ifdef CONFIRMPV
	g=firstnegamax(depth,color,g-1,g+1,best);
	time = ( (clock()-start)/CLK_TCK);
	getpv(Lstr2,color);
	sprintf(Lstr1,"value=%i",g);
	sprintf(out,"depth %i/%i/%.1f  time %.2fs  %s  nodes %i  %ikN/s  %s", 
			(depth/FRAC),maxdepth, (float)leafdepth/((float)leafs+0.01) , time,Lstr1,
			cake_nodes, (int)((float)cake_nodes/time/1000),Lstr2); 
#else
	sprintf(Lstr1,"value=%i",g);
	sprintf(out,"depth %i/%i/%.1f  time %.2fs  %s  nodes %i  %ikN/s  %s", 
			(depth/FRAC),maxdepth,(float)leafdepth/((float)leafs+0.01) , time, Lstr1, 
			cake_nodes, (int)((float)cake_nodes/time/1000),Lstr2); 
#endif

	if(cake_fp != NULL)
		{
		fprintf(cake_fp,"\n%s\n",out);	
		fflush(cake_fp);
		}
	return g;
	}


int windowsearch(int depth, int color, int guess, struct move *best)
	{
	int value;
	double time;
	char Lstr1[1024],Lstr2[1024];

	/*do a search with aspiration window*/
	rootalpha = guess-ASPIRATIONWINDOW;
	rootbeta = guess+ASPIRATIONWINDOW;

	value=firstnegamax(depth,color,guess-ASPIRATIONWINDOW,guess+ASPIRATIONWINDOW,best);
	
	if(value>=guess+ASPIRATIONWINDOW)
		{
		// print info in status bar and cakelog.txt
		sprintf(Lstr1,"value>%i",value-1);
		time = ( (clock()-start)/CLK_TCK);
		getpv(Lstr2,color);
		
		sprintf(out,"depth %i/%i/%.1f  time %.2fs  %s  nodes %i  %ikN/s  %s", 
			(depth/FRAC),maxdepth,(float)leafdepth/((float)leafs+0.01),time  , Lstr1, 
			cake_nodes, (int)((float)cake_nodes/time/1000),Lstr2); 
		if(cake_fp != NULL)
			{
			fprintf(cake_fp,"\n  %s",out);	
			fflush(cake_fp);
			}

		rootalpha=guess; 
		rootbeta=MATE;
		value=firstnegamax(depth,color,guess,MATE,best);
		if(value<=guess)
			{
			// print info in status bar and cakelog.txt
			sprintf(Lstr1,"value<%i",value+1);
			time = ( (clock()-start)/CLK_TCK);
			getpv(Lstr2,color);
		
			sprintf(out,"depth %i/%i/%.1f  time %.2fs  %s  nodes %i  %ikN/s  %s", 
			(depth/FRAC),maxdepth,(float)leafdepth/((float)leafs+0.01) , time, Lstr1, 
			cake_nodes, (int)((float)cake_nodes/time/1000),Lstr2); 
			if(cake_fp != NULL)
				{
				fprintf(cake_fp,"\n  %s",out);	
				fflush(cake_fp);
				}

			rootalpha=-MATE;
			value=firstnegamax(depth,color,-MATE,MATE,best);
			}
		}
	if(value<=guess-ASPIRATIONWINDOW)
		{
		// print info in status bar and cakelog.txt
		sprintf(Lstr1,"value<%i",value+1);
		time = ( (clock()-start)/CLK_TCK);
		getpv(Lstr2,color);
		
		sprintf(out,"depth %i/%i/%.1f  time %.2fs  %s  nodes %i  %ikN/s  %s", 
			(depth/FRAC),maxdepth,(float)leafdepth/((float)leafs+0.01) ,time,  Lstr1, 
			cake_nodes, (int)((float)cake_nodes/time/1000),Lstr2); 
		if(cake_fp != NULL)
			{
			fprintf(cake_fp,"\n  %s",out);
			fflush(cake_fp);
			}

		rootalpha=-MATE;
		rootbeta=guess;
		value=firstnegamax(depth,color,-MATE,guess,best);
		if(value>=guess)
			{
			// print info in status bar and cakelog.txt
			sprintf(Lstr1,"value>%i",value-1);
			time = ( (clock()-start)/CLK_TCK);
			getpv(Lstr2,color);
		
			sprintf(out,"depth %i/%i/%.1f  time %.2fs  %s  nodes %i  %ikN/s  %s", 
			(depth/FRAC),maxdepth,(float)leafdepth/((float)leafs+0.01) , time, Lstr1, 
			cake_nodes, (int)((float)cake_nodes/time/1000),Lstr2); 
			if(cake_fp != NULL)
				{
				fprintf(cake_fp,"\n  %s",out);	
				fflush(cake_fp);
				}

			rootbeta=MATE;
			value=firstnegamax(depth,color,-MATE,MATE,best);
			}
		}
	
	sprintf(Lstr1,"value=%i",value);
	time = ( (clock()-start)/CLK_TCK);
	getpv(Lstr2,color);
		
	sprintf(out,"depth %i/%i/%.1f  time %.2fs  %s  nodes %i  %ikN/s  %s", 
		(depth/FRAC),maxdepth,(float)leafdepth/((float)leafs+0.01) ,time,  Lstr1, 
		cake_nodes, (int)((float)cake_nodes/time/1000),Lstr2); 
	if(cake_fp!=NULL)
		{
		fprintf(cake_fp,"\n%s",out);
		fflush(cake_fp);
		}
	return value;
	}



int firstnegamax(int d, int color, int alpha, int beta, struct move *best)
	{

	/*--------------------------------------------------------------------------
	|																			|
	|		firstnegamax: first instance of negamax which returns a move		|
	|																			|
	 --------------------------------------------------------------------------*/
	
	int i,j,value,swap=0,bestvalue=-MATE;
	static int n;
	static struct move movelist[MAXMOVES],ml2[MAXMOVES];
	int32 l_bm,l_bk,l_wm,l_wk;
	int32 Lkey,Llock;
	int Lalpha=alpha;
	int32 forcefirst=0;
	int32 Lkiller=0;
	static struct pos last;
	struct move tmpmove;
	static int values[MAXMOVES];	/* holds the values of the respective moves - use to order */
	int refnodes[MAXMOVES];			/* number of nodes searched to refute a move */
	int statvalues[MAXMOVES];
	int tmpnodes;
	int bestindex=0;
	
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
		n=makecapturelist(&p,movelist,statvalues,color,forcefirst);
		if(n==0)
			{
			n=makemovelist(&p,movelist,statvalues,color,forcefirst,0);
			/* and order movelist completely */
			
			for(i=1;i<n;i++)
				{
				j=i;
				tmpmove=movelist[i];
				value=statvalues[i];
				while ( value>statvalues[j-1])
					{
					movelist[j]=movelist[j-1];
					statvalues[j]=statvalues[j-1];
					j--;
					}
				movelist[j]=tmpmove;
				statvalues[j]=value;
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
		togglemove(p,movelist[i]);
		countmaterial(&p);
		realdepth++;
		/*update the hash key*/
		updatehashkey(&movelist[i]);
#ifdef REPCHECK
		Ghistory[realdepth+HISTORYOFFSET]=p;
#endif
		tmpnodes=cake_nodes;
		/********************recursion********************/
#ifndef PVS
		/* normal search: */
#ifndef MPC
		value=-negamax(&p,d-FRAC,color^CC,-beta,-Lalpha,&Lkiller,0,0);
#else
		value=-mpc(&p,d-FRAC,color^CC,-beta,-Lalpha,&Lkiller);
#endif
#endif
#ifdef PVS
		/* my PVS search: */

		if(Lalpha==alpha)
			value=-negamax(&p,d-FRAC,color^CC,-beta,-Lalpha, &Lkiller,0,0);
		else
			{
			/*not in PV - we expect this move to fail low in a minimal window search */
			value=-negamax(&p,d-FRAC,color^CC,-(Lalpha+1),-Lalpha, &Lkiller,0,0);
			if(value>Lalpha)
				/*re-search because of fail-high*/
				value=-negamax(&p,d-FRAC,color^CC,-beta,-Lalpha, &Lkiller,0,0);
			}
#endif
		/******************* end recursion ***************/
		refnodes[i]=cake_nodes-tmpnodes;
		values[i]=value;
		realdepth--;
		togglemove(p,movelist[i]);
		/* restore the old hash key*/
		Gkey=Lkey;
		Glock=Llock;
		/* restore the old material balance */
		bm=l_bm;bk=l_bk;wm=l_wm;wk=l_wk;
		bestvalue=max(bestvalue,value);

		if(value>=beta) 
			{
			*best=movelist[i];/*Lalpha=value;*/
			swap=i;
#ifdef PV 
			update_pv(i,realdepth); 
#endif 
			break;
			}
		if(value>Lalpha) 
			{
			*best=movelist[i];
			Lalpha=value;
#ifdef PV 
			update_pv(i,realdepth); 
#endif 
			swap=i;
			}
		}
	/* save the position in the hashtable */
	/* a bit complicated because we need to save best move as index */
	n=makecapturelist(&p,ml2,statvalues,color,forcefirst);
	if(n==0)
		n=makemovelist(&p,ml2,statvalues,color,forcefirst,0);
	for(bestindex=0;bestindex<n;bestindex++)
		{
		if(ml2[bestindex].bm==best->bm && ml2[bestindex].bk==best->bk && ml2[bestindex].wm==best->wm && ml2[bestindex].wk==best->wk)
			break;
		}
	hashstore(&p,bestvalue,alpha,beta,d,best,color,bestindex);
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



int negamax(struct pos *p, int d, int color, int alpha, int beta, int32 *protokiller, int truncationdepth, int truncationdepth2)
	/*----------------------------------------------------------------------------
	|																			  |
	|		negamax: the basic recursion routine of cake++						  |
	|					returns the negamax value of the current position		  |
	|					sets protokiller to a compressed version of a move,		  |
	|					if it's black's turn, protokiller = best.bm|best.bk		  |
	|																			  |
	 ----------------------------------------------------------------------------*/
	{
	int value, valuetype;
	int32 forcefirst=MAXMOVES-1;
	int localalpha=alpha, localbeta=beta, n, v1,v2,localeval=MATE;
	int32 Lkey,Llock;
	int l_bm,l_bk,l_wm,l_wk, maxvalue=-MATE;
	int delta=0;
	
#ifdef ETC
	int bestETCvalue;
#endif

	int i,j;
	int index, bestmovevalue;
	int bestindex;
	int32 Lkiller=0;
	int isrootnode=0;
	struct move movelist[MAXMOVES];
	int values[MAXMOVES];
	struct pos q;
	int stmcapture = 0, sntmcapture = 0;
	int dbresult;
	int gotdbvalue=0;
	int isdbpos = 0;
	int cl=0; // conditional lookup - predefined as no. if CL && CLDEPTH>d then it's 1.
	int ispvnode=0;

#ifdef STOREEXACTDEPTH
	int originaldepth = d;
#endif
	// time check: abort search if we exceed the time given by aborttime
#ifdef CHECKTIME
	if((cake_nodes & 0xFFFF)==0)
		{
		if(searchmode==0)
			if( (clock()-start)/CLK_TCK>(aborttime)) (*play=1);
		}
#endif

	cake_nodes++;

	// check material:
	if(p->bm+p->bk==0)
		return color==BLACK?(-MATE+realdepth):(MATE-realdepth);
	if(p->wm+p->wk==0)
		return color==WHITE?(-MATE+realdepth):(MATE+realdepth);

	// return if calculation interrupt is requested

	if(*play) return 0;

	// stop search if maximal search depth is reached 
	if(realdepth>MAXDEPTH) 
		{
		leafs++;leafdepth+=realdepth;
		return evaluation(p,color,alpha,beta,&delta);
		}
	
	// search the current position in the hashtable 
	// only if there is still search depth left! 
	// should test this without d>0 !?
	if(d>0)
		{
		if(hashlookup(&value,&valuetype,d,&forcefirst,color,&ispvnode))
			{
			// return value 1: the position is in the hashtable and the depth
			// is sufficient. it's value and valuetype are used to shift bounds
			// or even cutoff 
			if(valuetype==EXACT)
				return value;
			if(valuetype==LOWER)
				{
				if(value>=beta) return value;
				if(value>localalpha) localalpha=value;
				}
			if(valuetype==UPPER)
				{
				if(value<=alpha) return value;
				if(value<localbeta) localbeta=value;
				}
			}
		}
#ifdef REPCHECK
	// check for repetitions 
	if(bk && wk)
		{
		for(i=realdepth+HISTORYOFFSET-2;i>=0;i-=2)
			{
			if((p->bm^Ghistory[i].bm)) break; // stop repetition search if move with a man is detected 
			if((p->wm^Ghistory[i].wm)) break; 
			if((p->bm==Ghistory[i].bm) && (p->bk==Ghistory[i].bk) && (p->wm==Ghistory[i].wm) && (p->wk==Ghistory[i].wk))
				return 0;                     // same position detected! 
			}
		}
#endif
	
	// get info on captures: can side to move or side not to move capture now?
	stmcapture = testcapture(p,color);
	// this should be ok: 
	if(!stmcapture)
		sntmcapture = testcapture(p,color^CC);

	if(stmcapture)
		n=makecapturelist(p,movelist,values,color,forcefirst);
    else
		n=0;


	//--------------------------------------------//
	// check for database use                     // 
	// conditions: -> #of men < maxNdb            //
	//			   -> no capture for either side  //
	//                else result is incorrect    //
	//--------------------------------------------//
#ifdef USEDB
	if(bm+bk+wm+wk<=MAXPIECES && max(bm+bk,wm+wk)<=MAXPIECE)
		{
		isdbpos = 1;
		if(!(stmcapture|sntmcapture))
			{
			// this position can be looked up!
			dblookups++;

#ifdef CL
			if(d<CLDEPTH*FRAC)
				cl=1;
#endif

			if(color==BLACK)
				dbresult = dblookup(p,DB_BLACK,cl);
			else
				dbresult = dblookup(p,DB_WHITE,cl);

			// statistics

			if(dbresult == DB_NOT_LOOKED_UP)
				dbfail++;
			else if(dbresult != DB_UNKNOWN)
				dbsuccess++;			

			if(dbresult == DB_DRAW)
				return 0;
			if(dbresult == DB_WIN)
				{
				value = dbwineval(p,color);
				if(value>=localbeta)
					return value;
				}
			if(dbresult == DB_LOSS)
				{
				value = dblosseval(p,color);
				if(value<=localalpha)
					return value;
				}
			}
		}
#endif // USEDB


#ifdef NEWTRUNCATION
// new truncation stuff: only if no capture is here, do truncation.
// also, if we are in a potential db position, we don't truncate any
// further: reason: either it is a win/loss, and root is not win/loss ->
// will alphabeta cutoff immediately
// or else it's dangerous to call evaluation if side not to move may have a capture
	//if(rootalpha==alpha && rootbeta==beta)
	//	ispvnode=1;
	//if(rootalpha==-beta && rootbeta==-alpha)
	//	ispvnode=1;

#ifndef MARKPV
	ispvnode=0;
#endif

	// we don't do any truncation if:
	//  -> the side to move is capturing - we may be way off the score
	//  -> we are in a db position - there the huge scores confuse the pruning
	//  -> we are in a previous pv - we want to look at that carefully!
	if((!stmcapture) && !(isdbpos /*&& sntmcapture*/) && !ispvnode)
		{
		// get whole evaluation
		localeval = evaluation(p,color,localalpha,localbeta, &delta);
		
		// eval will tell us if it thinks this search is not stable and should not be pruned as much!
		v1 = localeval;
		
		// get distance to window
		v1 = max(localalpha-v1, v1-localbeta);
		// allow for some distance to the window, taking into account delta from the search.

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
			v1+=TRUNCLEVELHARD; //1.42 which beats nemesis hands down does not have this
			// limit maximum truncation amount to MAXTRUNC:
			v1 = min(v1, MAXTRUNC*TRUNCDIVHARD); // and it does not have this

			truncationdepth += v1/TRUNCDIVHARD; // and it uses /10 here and on next line
			d-= v1/TRUNCDIVHARD;  // 1.422 used /16
			}
		else //not outside window with strong truncation - reexpand and check if we also have to reexpand
			// the weak truncation.
			{
			// reexpand
			d+=truncationdepth;
			truncationdepth=0;
			
			//new double trunc code
			if(v2)
				{
				v2+=TRUNCLEVELSOFT;
				truncationdepth2 += v2/TRUNCDIVSOFT;
				d-= v2/TRUNCDIVSOFT; 
				}
			else
				{
				d+=truncationdepth2;
				truncationdepth2=0;
				}
			// end new double trunc code
			}
		}

#endif // NEWTRUNCATION
	// return conditions: no capture and d<=0
	// also, if side not to move has capture, eval must be sufficiently outside search window.


// now, check return condition - never evaluate with a capture on board!
// when there is no capture it's easy:
	if(d<=0 && !stmcapture)
		{
		// normally, we'd be stopping here.
		// but if the side to move "has something", we shouldn't do this
		if(!sntmcapture)
			{
			if(realdepth>maxdepth) 
				maxdepth=realdepth;
			// no capture in this position - return
			if(localeval!=MATE)
				return localeval;
			return evaluation(p,color,localalpha,localbeta,&delta);
			}
		else
			{
			// returning a value with sntmcapture is risky, but probably good.
			// however, if this position is in the database, we don't want to 
			// do this.
			if(!isdbpos)
				{
				if(localeval!=MATE)
					value=localeval;
				else
					value = evaluation(p,color,localalpha,localbeta,&delta);
				// side not to move has a capture. what does this mean? it means that
				// possibly the value of this position will drop dramatically. therefore
				// this should be done asymmetrically: if value is already low, don't worry
				// too much. but if value is high, think about it again! value > localbeta+X*QLEVEL maybe?
				// QLEVEL:200, QLEVEL2:60. 
				// this takes care of this, i hope. at about 5% price opening & midgame, 2% endgame.
				// compared to 60/60
				if(value > localbeta+QLEVEL || value <localalpha-QLEVEL2)
					return value;
				}
			}
		}
		// if there is a capture for the side to move:
//	if(!stmcapture && sntmcapture && bm+bk+wm+wk>maxNdb)
//		return evaluation(p,color,localalpha,localbeta);	


// we could not return. prepare next iteration

	if(!n)
		n=makemovelist(p,movelist,values,color,forcefirst,*protokiller);
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
	
#ifdef ETC
	if(d>ETCDEPTH)
		{
		bestETCvalue=ETClookup(movelist,d,n,color);
		if(bestETCvalue>=localbeta) 
			return bestETCvalue;
		if(bestETCvalue>localalpha) 
			localalpha=bestETCvalue;
		}
#endif

/*************** main "for all moves do" loop starts here ! *******************/
/* up to here it was just lots of other stuff - now we are ready for the work*/

	for(i=0;i<n;i++)
		{
		/* for all moves: domove, count material, recursive call, undo move, check condition */
		
		/* movelist contains moves, movelist.info contains their values. 
			have to find the best move! 
			we do this by linear search through the movelist[] array */
		
		index=0;
		bestmovevalue=-1;
	
		for(j=0;j<n;j++)
			{
			if(values[j]>bestmovevalue)
				{
				bestmovevalue=values[j];
				index=j;
				}
			}
		/* movelist[index] now holds the best move */
		/* set values[index] to -1, that means that this move will no longer be considered */
		values[index]=-1;
		
		/* domove */
		q.bk=p->bk^movelist[index].bk;
		q.bm=p->bm^movelist[index].bm;
		q.wk=p->wk^movelist[index].wk;
		q.wm=p->wm^movelist[index].wm;


		if(i==0) 
			bestindex=index;
			
		/* inline material count */
		/*add=0;*/
		if(color==BLACK)
			{
			if(stmcapture)
				{
				wm-=recbitcount(movelist[index].wm);
				wk-=recbitcount(movelist[index].wk);
				}
			if(movelist[index].bk && movelist[index].bm) //vtune: use & instead of && but how??
				{bk++;bm--;/*if(d<=FRAC) add=2*FRAC;*/}
			}
		else
			{
			if(stmcapture)
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
		Ghistory[realdepth+HISTORYOFFSET]=q;
#endif
		/*update the hash key*/
		updatehashkey(&movelist[index]);  // vtune: pass pointer to the structure, not structure itself!
		
#ifndef PVS
		/********************recursion********************/
		value=-negamax(&q,d-FRAC,color^CC,-beta,-localalpha, &Lkiller,truncationdepth,truncationdepth2);
		/*************************************************/
#endif

#ifdef PVS
		/* my PVS search: */
		if(localalpha==alpha)
			value=-negamax(&q,d-FRAC,color^CC,-beta,-localalpha, &Lkiller,truncationdepth,truncationdepth2);
		else
			{
			/*not in PV - we expect this move to fail low in a minimal window search */
			value=-negamax(&q,d-FRAC,color^CC,-(localalpha+1),-localalpha, &Lkiller,truncationdepth,truncationdepth2);
			if(value>localalpha)
				/*re-search because of fail-high*/
				value=-negamax(&q,d-FRAC,color^CC,-beta,-localalpha, &Lkiller,truncationdepth,truncationdepth2);
			}
#endif

		/************** undo the move ***********/
		realdepth--;
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
			bestindex=index;
			break;
			}
		if(maxvalue>localalpha) 
			{
			localalpha=maxvalue;
			bestindex=index;
			}
		} /* end main recursive loop of forallmoves */

	/* save the position in the hashtable */
	/* we can/should restore the depth with which negamax was originally entered */
	/* since this is the depth with which it would have been compared */
#ifdef STOREEXACTDEPTH
	d = originaldepth;
#endif
	hashstore(p,maxvalue,alpha,beta,d,&movelist[bestindex],color,bestindex); // vtune: use pointer for "best"

#ifdef MOKILLER
	/* set the killer move */
	/* maybe change this: only if cutoff move or only if no capture?*/
	if(color==BLACK)
		*protokiller=movelist[bestindex].bm|movelist[bestindex].bk;
	else
		*protokiller=movelist[bestindex].wm|movelist[bestindex].wk;
#endif

	return maxvalue;
	}

#ifdef ETC
int ETClookup(struct move movelist[MAXMOVES], int d, int n,int color)
	{
	int i;
	int32 Lkey=Gkey, Llock=Glock;
	int ETCvalue, ETCvaluetype,ETCdummy,bestETCvalue=-MATE;
	int dummy;

	color^=CC;

	for(i=0;i<n;i++)
		{
		/* update hashkey - we don't need to actually execute the move!*/ 
		updatehashkey(&movelist[i]); // vtune: pass pointer
		/* do the ETC lookup:
		with reduced depth and changed color */
		if(hashlookup(&ETCvalue,&ETCvaluetype,d-FRAC,&ETCdummy,color,&dummy))
			{
			/* the position we searched had sufficient depth */
			/* if one of the values we find is > beta we quit! */
			/* code from schaeffer-paper - I find this strange to wrong!*/
			ETCvalue=-ETCvalue;  /* negamax-framework! */
			if(ETCvaluetype==EXACT || ETCvaluetype==UPPER) // vtune: use | instead of ||
				{
				/* if it was an upper bound one iteration on it is a lower bound
				on this iteration level. therefore, in these two cases ETCvalue
				is a lower bound on the value of our position here */
				if(ETCvalue>bestETCvalue) 
					{
					bestETCvalue=ETCvalue;
					}
				}
			}
		/* restore the hash key*/
		Gkey=Lkey;Glock=Llock;
		}
	return bestETCvalue;
	}
#endif


/* inline capture :*/
int testcapture(struct pos *p, int color)
	{
	/* test for captures inline to speed things up:*/
	/* testcapture returns 1 if color has a capture on the global position p */
	int32 black,white,free,m;

	if (color==BLACK)
		{
		black=p->bm|p->bk;
		white=p->wm|p->wk;
		free=~(black|white);
		
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
		black=p->bm|p->bk;
		white=p->wm|p->wk;
		free=~(black|white);
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


void hashstore(struct pos *p, int value, int alpha, int beta, int depth, struct move *best,int color, int32 bestindex)
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


#ifdef MOHISTORY
	/* update history table */
	if(color==BLACK)
		{
		from=(best->bm|best->bk)&(p->bm|p->bk);    /* bit set on square from */
		to=  (best->bm|best->bk)&(~(p->bm|p->bk));
		from=LSB(from);
		to=LSB(to);
		from&=31;to&=31;
		history[from][to]++;
		}
	else
		{
		from = (best->wm|best->wk)&(p->wm|p->wk);    /* bit set on square from */
		to = (best->wm|best->wk)&(~(p->wm|p->wk));
		from=LSB(from);
		to=LSB(to);
		from&=31;to&=31;
		history[from][to]++;
		}
#endif

	

	/* its in the "deep" hashtable: take care not to overwrite other entries */
	index=Gkey%hashsize;
	minindex=index;
	while(iter<HASHITER)
		{
		if(hashtable[index].lock==Glock || hashtable[index].lock==0) // vtune: use | instead of ||
			/* found an index where we can write the entry */
			{
			hashtable[index].lock=Glock;
			hashtable[index].depth=(int16) (depth);
			if(color==BLACK)
				{
				hashtable[index].best=bestindex;
				//hashtable[index].color=BLACK;
				hashtable[index].color=(BLACK>>1);
				}
			else
				{
				hashtable[index].best=bestindex;
				//hashtable[index].color=WHITE;
				hashtable[index].color=(WHITE>>1);
				}
			hashtable[index].value=(sint16)value;
			/* determine valuetype */
			if(value>=beta) {hashtable[index].valuetype=LOWER;return;}
			if(value>alpha) {hashtable[index].valuetype=EXACT;return;}
			hashtable[index].valuetype=UPPER;
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
	if(mindepth>(depth)) return;
#endif

	hashtable[minindex].lock=Glock;
	hashtable[minindex].depth=depth;
	hashtable[minindex].best=bestindex;
	//hashtable[minindex].color=color;
	hashtable[minindex].color=(color>>1);
	hashtable[minindex].value=value;
	/* determine valuetype */
	if(value>=beta) {hashtable[minindex].valuetype=LOWER;return;}
	if(value>alpha) {hashtable[minindex].valuetype=EXACT;return;}
	hashtable[minindex].valuetype=UPPER;

	return;
	/* and return */
	}

int hashlookup(int *value, int *valuetype, int depth, int32 *forcefirst, int color, int *ispvnode)
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


	index=Gkey%hashsize;
	//index = Gkey & (hashsize-1); // expects that hashsize is a power of 2!
	while(iter<HASHITER && hashtable[index].lock) // vtune: use & instead
		{
		//if(hashtable[index].lock==Glock && ((int)hashtable[index].color==color))
		if(hashtable[index].lock==Glock && ((int)hashtable[index].color==(color>>1)))
			{
			/* we have found the position */
			*ispvnode=hashtable[index].ispvnode;
			
			/* move ordering */
			*forcefirst=hashtable[index].best;
			/* use value if depth in hashtable >= current depth)*/
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

int pvhashlookup(int *value, int *valuetype, int depth, int32 *forcefirst, int color, int *ispvnode)
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


	index=Gkey%hashsize;
	while(iter<HASHITER && hashtable[index].lock) // vtune: use & instead
		{
		//if(hashtable[index].lock==Glock && ((int)hashtable[index].color==color))
		if(hashtable[index].lock==Glock && ((int)hashtable[index].color==(color>>1)))
			{
			/* we have found the position */
			hashtable[index].ispvnode=1;
			/* move ordering */
			*forcefirst=hashtable[index].best;
			/* use value if depth in hashtable >= current depth)*/
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



void getpv(char *str,int color)
	{
	/* retrieves the principal variation from the hashtable:
		looks up the position, finds the hashtable move, plays it, looks
		up the position again etc.
		color is the side to move, the whole PV gets written in *str */
	// getpv also marks pv nodes in the hashtable, so they won't be pruned!

	struct move movelist[MAXMOVES];
	int32 forcefirst;
	int dummy=0;
	int i,n;
	char Lstr[1024];
	struct pos Lp;
	int values[MAXMOVES];

	Lp=p;
	absolutehashkey(&p);
	sprintf(str,"pv ");
	for(i=0;i<MAXDEPTH;i++)
		{
		forcefirst=100;
		// pvhashlookup also stores the fact that these nodes are pv nodes
		// in the hashtable
		pvhashlookup(&dummy,&dummy,0, &forcefirst,color,&dummy);
		if(forcefirst==100)
			{
			break;
			}
		n=makecapturelist(&p,movelist,values,color,forcefirst);
		if(!n)
			n=makemovelist(&p,movelist,values,color,forcefirst,0);
		if(!n) 
			{
			p=Lp;
			absolutehashkey(&p);
			return;
			}
		
		movetonotation(p,movelist[forcefirst],Lstr,color);
		//sprintf(Lstr,"%i",forcefirst);
		if(i<MAXPV)
			{
			strcat(str,Lstr);
			strcat(str," ");
			}
		togglemove(p,movelist[forcefirst]);
		absolutehashkey(&p);
		color=color^CC;
		}
	p=Lp;
	absolutehashkey(&p);
	return;
	}



#ifdef USEDB
int dbwineval(struct pos *p,int color)
	{
	/* dbwineval provides a heuristic value to a position which is a win for
		color according to the database

		the heuristic value is higher if...
		-> there are less pieces on the board (encourage exchanges)
		-> the winning side has more kings
		-> the winning side occupies the center with kings
		-> the winning side has a high tempo count for men (encourage crowning)
		*/

	int value = 1000;
	
	/* new in 1.14: modifiers to encourage the stronger side to
	promote and grab the center */

	if(color==BLACK) /* color is the side which is winning now */
		{
		value += 100*(bm+bk-wm-wk);  /* go for largest piece difference first */
		value -= 25*(bm+bk+wm+wk);   /* go for fewer pieces next */
		value+=15*bitcount(p->bk);	 /* go for kings */
		value+=2*bitcount(p->bk&CENTER); /* go for center control of kings */
		value+=3*bitcount(p->wk&EDGE);	 /* push opponent kings to edge */
		if(p->bm)						 /* promote men */
			{
			value += bitcount(p->bm&0x000000F0);
			value += 2*bitcount(p->bm&0x00000F00);
			value += 3*bitcount(p->bm&0x0000F000);
			value += 4*bitcount(p->bm&0x000F0000);
			value += 5*bitcount(p->bm&0x00F00000);
			value += 6*bitcount(p->bm&0x0F000000);
			}
		}
	else
		{
		value += 100*(wm+wk-bm-bk);  /* go for largest piece difference first */
		value -= 25*(bm+bk+wm+wk);   /* go for fewer pieces next */
		value+=15*bitcount(p->wk);	 /* go for kings */
		value+=2*bitcount(p->wk&CENTER); /* go for center control of kings */
		value+=3*bitcount(p->bk&EDGE);	 /* push opponent kings to edge */
		if(p->wm)
			{
			value += bitcount(p->wm&0x0F000000);
			value += 2*bitcount(p->wm&0x00F00000);
			value += 3*bitcount(p->wm&0x000F0000);
			value += 4*bitcount(p->wm&0x0000F000);
			value += 5*bitcount(p->wm&0x00000F00);
			value += 6*bitcount(p->wm&0x000000F0);
			}
		}
	return value;
	}

int dblosseval(struct pos *p, int color)
	{
	/* the same purpose as dbwineval, but here, color has a LOSS on the board */
	/* for an extended description, see dbwineval above */

	int value=-1000;


	if(color==BLACK) /* color is the side which is losing now */
		{
		value -= 100*(wm+wk-bm-bk);  /* go for largest piece difference first */
		value += 25*(bm+bk+wm+wk);   /* go for fewer pieces next */
		value-=15*bitcount(p->wk);	 /* go for kings */
		value-=2*bitcount(p->wk&CENTER); /* go for center control of kings */
		value-=3*bitcount(p->bk&EDGE);	 /* push opponent kings to edge */
		if(p->wm)
			{
			value -= bitcount(p->wm&0x0F000000);
			value -= 2*bitcount(p->wm&0x00F00000);
			value -= 3*bitcount(p->wm&0x000F0000);
			value -= 4*bitcount(p->wm&0x0000F000);
			value -= 5*bitcount(p->wm&0x00000F00);
			value -= 6*bitcount(p->wm&0x000000F0);
			}
		}
	else /* white is losing */
		{
		value += 100*(wm+wk-bm-bk);  /* go for largest piece difference first */
		value += 25*(bm+bk+wm+wk);   /* go for fewer pieces next */
		value-=15*bitcount(p->bk);	 /* go for kings */
		value-=2*bitcount(p->bk&CENTER); /* go for center control of kings */
		value-=3*bitcount(p->wk&EDGE);	 /* push opponent kings to edge */
		if(p->wm)
			{
			value -= bitcount(p->wm&0x0F000000);
			value -= 2*bitcount(p->wm&0x00F00000);
			value -= 3*bitcount(p->wm&0x000F0000);
			value -= 4*bitcount(p->wm&0x0000F000);
			value -= 5*bitcount(p->wm&0x00000F00);
			value -= 6*bitcount(p->wm&0x000000F0);
			}
		}
	return value;
	}
#endif

int staticevaluation(struct pos *q, int color, int *total, int *material, int *positional)
	{
	int noprune;

	countmaterial(q);
	*total = evaluation(q,color,-MATE,MATE,&noprune);
	*material = materialeval[bm][bk][wm][wk];
	*positional = fineevaluation(q,color, &noprune);
	return 0;
	}

int evaluation(struct pos *p, int color, int alpha, int beta, int *delta)
	{
	/* evaluation returns the value of the position with color to move,
		and with current search bounds alpha and beta

		evaluation is in fact just a material/endgame database evaluation.
		if the evaluation is outside the alpha-beta search window, extended
		by the amount FINEEVALWINDOW, it returns the material value only.
		else, it calls the fine evaluation function to get a better evaluation
		*/
	// evaluation is NEVER to be called with a capture for either side on the board!
	// delta tells the search if this position is "quiet" in a positional sense
	// delta is the extent of how uncertain the eval is about the evaluation 

	int eval;
	int dbresult=0;
	int cl=0;
	int fe;

#ifdef EVALOFF
	return 0;
#endif
	leafs++;
	leafdepth+=realdepth;
	/************************* material **************************/
	if(bm+bk==0) return(color==BLACK?(-MATE+realdepth):MATE-realdepth);
	if(wm+wk==0) return(color==BLACK?(MATE-realdepth):-MATE+realdepth);

#ifdef USEDB

	if(bm+bk+wm+wk<=MAXPIECES)
		{
		
		dblookups++;
#ifdef CL
		cl=1;
#endif

		if(color==BLACK)
			dbresult = dblookup(p, DB_BLACK,cl);
		else
			dbresult = dblookup(p, DB_WHITE,cl);
		
		if(dbresult == DB_NOT_LOOKED_UP)
			dbfail++;
		else if(dbresult != DB_UNKNOWN)
			dbsuccess++;

		if(dbresult==DB_DRAW)
			return 0;
		if(dbresult==DB_WIN)
			return dbwineval(p,color);
		if(dbresult==DB_LOSS)
			return dblosseval(p,color);
		}

#endif //USEDB



	eval=materialeval[bm][bk][wm][wk];
	if(color==WHITE) eval=-eval;
#ifdef EVALMATERIALONLY
	#ifdef COARSEGRAINING
		eval=(eval/GRAINSIZE)*GRAINSIZE;
	#endif
		return eval;
#endif

	if(( eval>beta+FINEEVALWINDOW) || (eval<alpha-FINEEVALWINDOW) ) // vtune: use |
		{
#ifdef COARSEGRAINING
		eval=(eval/GRAINSIZE)*GRAINSIZE;
#endif
		return eval;
		}

	fe=fineevaluation(p,color,delta);
	eval+=fe;
#ifdef COARSEGRAINING
	eval=(eval/GRAINSIZE)*GRAINSIZE;
#endif

	// if the positional score is high, then the position should be searched a bit deeper!
	//*noprune = 0;
	//if(abs(fe)>40)
	//	*noprune=1;
	
	return eval;
	}


int fineevaluation(struct pos *p, int color, int *delta)
	{
	/* fineevaluation is the positional evaluation of cake++
   	it knows a lot of features, most of the names are self-explanatory.
      it returns the positional evaluation of the position, positive
      is good for black, negative good for white, a man is worth 100.
   */

   /*static int tmod[25]={0,3,3,3,3,2,2,2,2,1,1,0,-1,-1,-1,-1,-2,-2,-3,-3,-4,-4,-5,-5,-5};*/
   /* about this DEO says in 'moveover': a tempo count of 4 is FATAL with 11 men each.
   	with 10 it is very bad, but not fatal...' */
   /* have to look at this closely again once: tmod is only 1 for 8 stones and 0
      for 10 -> i'd rather increase that to 2 and 1 at the least - hmm that
      sounds good but just doesnt work :-( */

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
#define MCENTER 0x00066000 // region which men should occupy

	int32 free,m;
	int stones=bm+wm;
	//int brblack=0,brwhite=0,backrank=0;
	int eval=0;
	int ending=0;
	int equal=0;
	int tempo=0;
	int cramp=0, crampval=4, nocrampval=1; //20//crampval for small cramps, nocrampval for uselsess pieces on cramp sq.
	int badstructureval =2;
	int blackcramps,whitecramps,newcrampval=10;
	int index;
	int balance=0;
	int kings=bk+wk;
	int doghole=0, dogholeval=8, dogholemandownval=20; 
	int mc,mc_occupyval=1,mc_attackval=1; 
	int realdykeval=5;
	int greatdykeval=2;
	int runaway=0,promoteinone=24,promoteintwo=20,promoteinthree=12; 
	int potbk=0, potwk=0;
	int blackhasbridge=0,whitehasbridge=0;
	int kcval=4,keval=-4;  // was 4 and -4 before generalized king eval
	int kingcentermonopoly=12; // was 12 before generalized king eval
	int trappedking=0,kingtrappedinsinglecornerval=20;
	int kingtrappedinsinglecornerbytwoval=10;
	int kingtrappedindoublecornerval=10;
	int dominatedkingval=10;
	int dominatedkingindcval=10;
	int blacktailhooks=0,whitetailhooks=0,tailhookval=4;
	int kingproximityval=2, kingproximity=0;
	int32 tmp,m1,free2,attack;
	int /*immobilewhitemen=0,immobileblackmen=0,*/immobilemanval=2,kingholdstwomenval = 10;//allimmobileval=2;
	int onlyking=0,onlykingval=20,roamingkingval=40; 
	int endangeredbridgeval=6;
	//int weakmanval=4;
	int turn=0, turnval=3;
	int freebk=bk, freewk=wk;
	int32 white=p->wm|p->wk;
	int32 black=p->bk|p->bm;
	int32 untrappedbk=p->bk;
	int32 untrappedwk=p->wk;
	int realdoghole = 0;
   

	// white, black, free, stones, kings and tempo get a value assigned and keep it 
	// throughout this evaluation function and can always be used.
	// DEO says about this: once there is the same number of men on the board, as free squares, this goes
	// from positive to negative - that's where i go to -1, so maybe change that...
	// he also says that a tempo count of 5 should be FATAL in the opening, so maybe change that too.
	static int tmod[25]={0,2,2,2,2,1,1,1,1,0,0,0,0,-1,-1,-1,-1,-2,-2,-2,-2,-3,-3,-3,-3}; 
	

	
   /************************ positional *************************/
   /* initialize free board, and game status */
	free=~(p->bm|p->bk|p->wm|p->wk);
	if(bm+bk==wm+wk) equal=1;
	if(bm+bk+wm+wk<=10) ending=1;

   /* organize: things to do only if men, things only if kings, */
   /* things to do only if men are on the board */

	if(stones)
		{
		/* back rank */
		eval+=(blackbackrankeval[p->bm&0x000000FF]-whitebackrankeval[p->wm>>24]); //2!!

		/* tempo */
		tempo+=  bitcount(p->bm&0x000000F0);
		tempo+=2*bitcount(p->bm&0x00000F00);
		tempo+=3*bitcount(p->bm&0x0000F000);
		tempo+=4*bitcount(p->bm&0x000F0000);
		tempo+=5*bitcount(p->bm&0x00F00000);
		tempo+=6*bitcount(p->bm&0x0F000000);
		tempo-=  bitcount(p->wm&0x0F000000);
		tempo-=2*bitcount(p->wm&0x00F00000);
		tempo-=3*bitcount(p->wm&0x000F0000);
		tempo-=4*bitcount(p->wm&0x0000F000);
		tempo-=5*bitcount(p->wm&0x00000F00);
		tempo-=6*bitcount(p->wm&0x000000F0);
		
		// new 1.41
		// only evaluate tempo when black is to move, i.e. if white is to move, increase his tempo count.
		// tempo = tempoblack - tempowhite, => if color = white tempo--;
		
		if(color==WHITE) tempo--;
		
		//tempo*=tmod[stones];
		/* wait with tempo evaluation until doghole is done! */

	/* cramping squares */
	/* all the 'cramping nothing statements: found with PDNtool */
	if(p->bm&SQ20) 
		{
		if(p->wm&SQ24)  cramp+=crampval;
		if(free&SQ24) cramp-=nocrampval; //cramping nothing - discourage a little
		}

	if(p->wm&SQ13)
		{
		if(p->bm&SQ9)
			cramp-=crampval;
		if(free&SQ9)
			cramp+=nocrampval; // cramping nothing - discourage a little
		}

	if(p->bm&SQ13)
		{
		if( (p->wm&(SQ17|SQ21/*|SQ22*/))==(SQ17|SQ21/*|SQ22*/) )
	   		cramp+=crampval;
		if((free&(SQ17|SQ21))==(SQ17|SQ21))
			// this was wrong in all versions before 1.51e a&b==c is evaluated as a&(b==c)...
			cramp-=nocrampval; // cramping nothing - discourage a little
		}
	if(p->wm&SQ20) 
		{
		if( (p->bm&(/*SQ11|*/SQ12|SQ16))==(/*SQ11|*/SQ12|SQ16) ) 
			cramp-=crampval;
		if((free&(SQ12|SQ16))==(SQ12|SQ16))
			// this was wrong in all versions before 1.51e a&b==c is evaluated as a&(b==c)...
			cramp+=nocrampval; // cramping nothing - discourage a little
		}

	eval+=cramp;

		
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

	/* some bad structures */
	/* found with PDNtool */
	/*strange - the single corner side structure for white (0x80808000) is not bad?!! - pdntool */

	if( match1(p->bm,SQ4|SQ12|SQ20))
		eval-=badstructureval;
	if( match1(p->bm,SQ1|SQ5|SQ13))
		eval-=badstructureval;

	if( match1(p->wm,SQ32|SQ28|SQ20))
		eval+=badstructureval;
	if( match1(p->wm,SQ13|SQ21|SQ29)) // new 1.376
		eval+=badstructureval;


	
	// new 1.41 - from DEO with PDNtool
	// describes holds of 2-3 or 3-4 men. this increases in importance, the less men, the worse.
	// only do this if stones <= N.
	// without this: +40-41 vs kr 1.12l, with this +48-39!
	if(stones<=14) 
		{
		// increases in importance with less men.
		// todo: maybe make more sophisticated by including knowledge about kings here - 
		// if the side which has a hold on the other side also has a free king, it's even
		// worse - because that means he will never have to give up the hold
		// if the side has more free kings than the defending side, it's worse again,
		// because it means that the hold can be changed maybe.
		badstructureval = 20-stones;


		// strong double corner against men on side - terrible statistics!!
		// if more than two men run against a solid DC:
   		//   28  29  30  31
		// 24  25  26  27
		//   20  21  22   w
		// 16  17  18   w
		//   12  13   w   w
		//  8   9  10   w
		//    4   5   6   w
		//  0   1   b   b
		  
		if( match1(p->bm, SQ1|SQ2) && (bitcount(p->wm & (SQ5|SQ9|SQ13|SQ14|SQ17|SQ21)) > 2) ) 
			eval+=badstructureval;
		if( (bitcount(p->bm&(SQ12|SQ16|SQ19|SQ20|SQ24|SQ28))>2) && (match1(p->wm,SQ31|SQ32)) )
			eval-=badstructureval;

		// new 1.41b: not covered with DEO
		//   28  29  30  31
		// 24  25  26  27
		//   20  21  22  23
		// 16  17  18  19
		//   12  13  14   w
		//  8   9  10   w
		//    4   5   6   w
		//  0   1   b   b
		if( (match1(p->bm,(SQ1|SQ2)) ) && (bitcount(p->wm & (SQ5|SQ9|SQ13)) == 2) ) // why not >=2?
			eval+=badstructureval;
		if( (bitcount(p->bm&(SQ20|SQ24|SQ28))==2) && (match1(p->wm,(SQ31|SQ32)) )) // vtune: use & //vtuned &&->&!
			eval-=badstructureval;

		// cramp in advanced double corner (as in white doctor)
   		//   28  29  30  31
		// 24  25  w   27
		//   20  21  22  w
		// 16  17  18  b
		//   12  13  b   b
		//  8   9  10  11
		//    4   5   6   7
		//  0   1   2   3
		
		if(match2(p->bm, p->wm, (SQ13|SQ14|SQ17), (SQ21|SQ26)))
			eval-=badstructureval;
		if(match2(p->bm, p->wm, (SQ7|SQ12), (SQ16|SQ19|SQ20)))
			eval+=badstructureval;

		// cramped original double corner
   		//   28  29  30  31
		// 24  25  26  27
		//   20  21  22  23
		// 16  17   w  19
		//   12  13   F   w
		//  8   9  10   b
		//    4   5   b   b
		//  0   1   F   b
		if(match3(p->bm, p->wm, free, (SQ1|SQ5|SQ6|SQ9), (SQ13|SQ18), (SQ2|SQ14)))
			eval-=badstructureval;
		if(match3(p->bm, p->wm, free, (SQ15|SQ20), (SQ24|SQ27|SQ28|SQ32), (SQ19|SQ31)))
			eval+=badstructureval;
		
		// the 4 classic 4-3 cramps
		// not only the one below, but also bm on 8,12,16 vs wm on 17,20,21,25 (bits)
		// i'm only evaluating them if there are no stones on the diagonal below
		// the cramping stones. but it's not really clear whether this is a good
		// idea!
   		//   28  29  30  31
		// 24  25  26  27
		//   20  21   w  23
		// 16  17   w   w
		//   12  13   w   b
		//  8   9  10   b
		//    4   5   F   b
		//  0   1   2   F
		// these were all wrong until las vegas 1.04!!
		if(match3(p->bm, p->wm, free, (SQ5|SQ9|SQ13), (SQ14|SQ17|SQ18|SQ22), (SQ1|SQ6)))
			eval+=badstructureval;
		if(match3(p->bm, p->wm, free, (SQ12|SQ16|SQ20), (SQ19|SQ23|SQ24|SQ27), (SQ4|SQ8|SQ11))) 
			eval+=badstructureval;
		if(match3(p->bm, p->wm, free,(SQ11|SQ15|SQ16|SQ19), (SQ20|SQ24|SQ28), (SQ27|SQ32)))
			eval-=badstructureval;
		if(match3(p->bm, p->wm, free, (SQ6|SQ9|SQ10|SQ14), (SQ13|SQ17|SQ21), (SQ22|SQ25|SQ29))) 
			eval-=badstructureval;
		// add penalty anyway, whether it's free on that diagonal or not - allow one man there
		if(match2(p->bm, p->wm, (SQ5|SQ9|SQ13), (SQ14|SQ17|SQ18|SQ22)) && (bitcount(p->bm&(SQ1|SQ6))<2))
			eval+=badstructureval;
		if(match2(p->bm, p->wm, (SQ12|SQ16|SQ20), (SQ19|SQ23|SQ24|SQ27)) && (bitcount(p->bm&(SQ4|SQ8|SQ11))<2)) // this one was wrong!
			eval+=badstructureval;
		if(match2(p->bm, p->wm,(SQ11|SQ15|SQ16|SQ19), (SQ20|SQ24|SQ28)) && (bitcount(p->wm&(SQ27|SQ32))<2))
			eval-=badstructureval;
		if(match2(p->bm, p->wm, (SQ6|SQ9|SQ10|SQ14), (SQ13|SQ17|SQ21)) && (bitcount(p->wm&(SQ22|SQ25|SQ29))<2)) // this one too!
			eval-=badstructureval;

		// 3-2 cramps

		// DEO p. 169 (A)
   		//   28  29  30  31
		// 24  25  26  27
		//   20  21  22  23
		// 16  17  18   w
		//   12  13   w   w
		//  8   9  10   F
		//    4   5   b   b
		//  0   1   2   F
		if(match3(p->bm, p->wm, free, (SQ5|SQ6), (SQ13|SQ14|SQ17), (SQ1|SQ9)))
			eval+=badstructureval;
		if(match3(p->bm, p->wm, free, (SQ16|SQ19|SQ20), (SQ27|SQ28), (SQ24|SQ32)))
			eval-=badstructureval;

	  // DEO p. 169 (B) - needs "free" additionally
   		//   28  29  30  31
		// 24  25  26  27
		//   20  21  22   w
		// 16  17  18   w
		//   12  13  14   w
		//  8   9  10   b
		//    4   5   b   F
		//  0   1   2   F
		if(match3(p->bm, p->wm, free, (SQ6|SQ9), (SQ13|SQ17|SQ21), (SQ1|SQ5)))
			eval+=badstructureval;
		if(match3(p->bm, p->wm, free, (SQ12|SQ16|SQ20), (SQ24|SQ27), (SQ28|SQ32)))
			eval-=badstructureval;

		// DEO p. 169 (C)
   		//   28  29  30  31
		// 24  25  26   F
		//   20  21   w   w
		// 16  17  18   w
		//   12  13  14   b
		//  8   9  10   b
		//    4   5   6   F
		//  0   1   2   3
		if(match3(p->bm, p->wm, free, (SQ9|SQ13), (SQ17|SQ21|SQ22), (SQ5|SQ25)))
			eval+=badstructureval;
		if(match3(p->bm, p->wm, free, (SQ11|SQ12|SQ16), (SQ20|SQ24), (SQ8|SQ28)))
			eval-=badstructureval;

		// more holds for versions >= 1.426

		// DEO p. 168 (B)
   		//   28  29  30  31
		// 24  25  26  27
		//   20  21  22  23
		//  w  17  18  19
		//    w   F  14  15
		//  w   9  10  11
		//    b   5   6   7
		//  F   b   2   3
		if(match3(p->bm, p->wm, free, (SQ3|SQ8), (SQ12|SQ16|SQ20), (SQ4|SQ15)))
			eval+=badstructureval;
		if(match3(p->bm, p->wm, free, (SQ13|SQ17|SQ21), (SQ25|SQ30), (SQ18|SQ29)))
			eval-=badstructureval;

		// DEO p. 168 (D)
   		//   28  29  30  31
		// 24  25  26  27
		//    w  21  22  23
		//  w   w   F  19
		//    F  13  14  15
		//  b   b  10  11
		//    F   5   6   7
		//  F   1   2   3
		if(match3(p->bm, p->wm, free, (SQ11|SQ12), (SQ19|SQ20|SQ24),(SQ4|SQ8|SQ16|SQ18)))
			eval+=badstructureval;
		if(match3(p->bm, p->wm, free, (SQ9|SQ13|SQ14), (SQ21|SQ22), (SQ29|SQ25|SQ17|SQ15)))
			eval-=badstructureval;

		// DEO p. 169 (D)
   		//   28  29  30  31
		//  f  25  26   w
		//    f   F   w   w
		//  w   w  18   F
		//    f  13   b   b
		//  b   b   f   F
		//    b   5   6   F
		//  0   1   2   3
		if(match3(p->bm, p->wm, free, (SQ13|SQ14), (SQ21|SQ22|SQ25), (SQ5|SQ9|SQ17|SQ23)))
			eval+=badstructureval;
		if(match3(p->bm, p->wm, free, (SQ8|SQ11|SQ12), (SQ19|SQ20), (SQ28|SQ24|SQ16|SQ10)))
			eval-=badstructureval;
		} // end holds */

		
 

    /*   WHITE
   	   28  29  30  31
	 24  25  26  27
	   20  21  22  23
	 16  17  18  19
	   12  13  14  15
	  8   9  10  11
	    4   5   6   7
	  0   1   2   3
	      BLACK */

		/* balance */
	
		if(bm == wm)
			{
			index =	-3*bitcount(p->bm&ROW1)-2*bitcount(p->bm&ROW2)-bitcount(p->bm&ROW3)+
					bitcount(p->bm&ROW6)+2*bitcount(p->bm&ROW7)+3*bitcount(p->bm&ROW8);
			balance-=abs(index);
			index = -3*bitcount(p->wm&ROW1)-2*bitcount(p->wm&ROW2)-bitcount(p->wm&ROW3)+
					bitcount(p->wm&ROW6)+2*bitcount(p->wm&ROW7)+3*bitcount(p->wm&ROW8);
			balance+=abs(index);
			eval+=balance;
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
	      BLACK
*/

		
		/* doghole */
		if( (p->bm&SQ3) && (p->wm&SQ12)) // vtune use &
			{
			/* good for black */
			/* count away the tempo effect: */
			/* only if it's good for black! we don't want white to go in the doghole just
			to save tempo */
			if(tmod[stones]>0) tempo+=5;
			/* and give a penalty - for sure here*/
			doghole+=dogholeval;
			}
		if( (p->bm&SQ1) && (p->wm&SQ5) ) /*real doghole*/ 
			{
			// set white doghole bit so we know
			realdoghole &= WHITE;
			/* count away tempo */
			if(tmod[stones]>0) tempo+=6;
			/* give a penalty or not? - i don't know... */
			if(ending==0) 
				doghole+=dogholeval;
			/* give a real penalty if there is a white man on bit11 or bit 15,
				because this man cannot go through!*/
			if( (p->wm & (SQ9|SQ13) ) && (free&SQ10)) 
				doghole+=dogholemandownval;
			}
		if( (p->bm&SQ28) && (p->wm&SQ32) ) /* real doghole*/ 
			{
			realdoghole &= BLACK;
			if(tmod[stones]>0) tempo-=6;
			if(ending==0)
				doghole-=dogholeval;
			if( (p->bm & (SQ24|SQ20) ) && (free&SQ23)) 
				doghole-=dogholemandownval;
			}
		if( (p->bm&SQ21) && (p->wm&SQ30) ) // vtune: use &
			{
			if(tmod[stones]>0) tempo-=5;
			doghole-=dogholeval;
			}
	eval+=doghole;

	/* now that doghole has been evaluated, we can do the tempo eval too */
	
	eval+=tempo*tmod[stones];
	
	/* man center control */
	// both with both MCENTER is best result up to now. less good: only occupation with either MCENTER or CENTER
	
	/* by occupation */
	mc  = mc_occupyval*bitcount(p->bm&MCENTER);
	mc -= mc_occupyval*bitcount(p->wm&MCENTER);
	eval += mc;

	/* by attack */
	mc  = mc_attackval*bitcount(attack_forwardjump(p->bm,free)&MCENTER);
	mc -= mc_attackval*bitcount(attack_backwardjump(p->wm,free)&MCENTER);
	eval += mc; 

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
		/* real dyke val */ /* maybe change to only if p->bm&BIT17 AND p->wm&BIT24 - cramping...*/
		/* yes! this improved match result from +55-45 to +62-48 */
		/* add 'greatdykeval': no more men on bit2,3,6 -> the man on bit14 cannot be challenged
			any more! */
		if((p->bm&BIT17) && (p->wm&BIT24)) // vtune: use &
			{
			eval+=realdykeval;
			if(((~(p->wm))&(BIT28|BIT29|BIT25))==(BIT28|BIT29|BIT25)) 
				// new 1.375a // and in again 1.377
				// this was wrong in all versions before 1.51e a&b==c is evaluated as a&(b==c)...
				{
				eval+=greatdykeval;
				}
			}
		if((p->wm&BIT14) && (p->bm&BIT7 )) // vtune: use &
			{
			eval-=realdykeval;
			if(((~(p->bm))&(BIT3|BIT2|BIT6))==(BIT3|BIT2|BIT6))  // new 1.375a
				// this was wrong in all versions before 1.51e a&b==c is evaluated as a&(b==c)...
				{
				eval-=greatdykeval;
				}
			}
		
	
	/************* runaway checkers **************************/
	/** in one **/
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
			
		if( (p->bm) & 0x0F000000)
			{
			if((p->bm)&(free>>4)&BIT24) {runaway+=promoteinone;potbk++;runaway-=6*tmod[stones];}
			if((p->bm)&( (free>>3)|(free>>4) ) & BIT25) {runaway+=promoteinone;potbk++;runaway-=6*tmod[stones];}
			if((p->bm)&( (free>>3)|(free>>4) ) & BIT26) {runaway+=promoteinone;potbk++;runaway-=6*tmod[stones];}
			/* should modify this for men on 27 which will be trapped in single corner!*/
			if((p->bm)&( (free>>3)|(free>>4) ) & BIT27) 
				{
				// check if man will be trapped or dominated
				// trapped
				if( !(p->bm&BIT23 && p->wm&BIT30 && p->wm&BIT26 && free&BIT19))
					{
					// dominated
					if( !(p->wm&BIT30 && (p->wk&(BIT22|BIT18|BIT19|BIT14))))
						{
						runaway+=promoteinone;
						potbk++;
						runaway-=6*tmod[stones];
						}
					}
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
	      BLACK
*/

		if( (p->wm) & 0x000000F0)
			{
			if((p->wm)&(free<<4)&BIT7)						{runaway-=promoteinone;potwk++;runaway+=6*tmod[stones];}
			if((p->wm)&( (free<<3)|(free<<4) ) & BIT6) {runaway-=promoteinone;potwk++;runaway+=6*tmod[stones];}
			if((p->wm)&( (free<<3)|(free<<4) ) & BIT5) {runaway-=promoteinone;potwk++;runaway+=6*tmod[stones];}
			if((p->wm)&( (free<<3)|(free<<4) ) & BIT4) 
				{
				// check if man will be trapped or dominated
				// trapped
				if( !(p->wm&BIT8 && p->bm&BIT1 && p->bm&BIT5 && free&BIT12))
					{
					// dominated
					if( !(p->bm&BIT1 && (p->bk&(BIT9|BIT13|BIT12|BIT17))))
						{
						runaway-=promoteinone;
						potwk++;
						runaway+=6*tmod[stones];
						}
					}
				}
			}
		/** in two **/
		if( (p->bm) & 0x00F00000)
			{
			if( (p->bm&BIT20) && !(RA20&white) ) {runaway+=promoteintwo;potbk++;runaway-=5*tmod[stones];} // vtune: use &, in all others from here
			if( (p->bm&BIT21) && (!(RA21L&white) || !(RA21R&white)) ) {runaway+=promoteintwo;potbk++;runaway-=5*tmod[stones];}
			if( (p->bm&BIT22) && (!(RA22L&white) || !(RA22R&white)) ) {runaway+=promoteintwo;potbk++;runaway-=5*tmod[stones];}
			// add detection for support by man on 21! */
			if( (p->bm&BIT23) && !(RA23&white) && !(BIT22&p->wk) ) {runaway+=promoteintwo;potbk++;runaway-=5*tmod[stones];}
			}
		if( (p->wm) & 0x00000F00)
			{
			if( (p->wm&BIT8)  && !(RA8&black) && !(p->bk&BIT9)) {runaway-=promoteintwo;potwk++;runaway+=5*tmod[stones];}
			if( (p->wm&BIT9)  && (!(RA9L&black) || !(RA9R&black))) {runaway-=promoteintwo;potwk++;runaway+=5*tmod[stones];}
			// add detection for support by man on 12 */
			if( (p->wm&BIT10) && (!(RA10L&black) || !(RA10R&black))) {runaway-=promoteintwo;potwk++;runaway+=5*tmod[stones];}
			if( (p->wm&BIT11) && !(RA11&black) ) {runaway-=promoteintwo;potwk++;runaway+=5*tmod[stones];}
			}
		/** in 3 **/
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
		if( (p->bm) & 0x000F0000)
			{
			if( (p->bm&BIT16) && !(RA16&white) && !(BIT17 & p->wk)) {runaway+=promoteinthree;runaway-=4*tmod[stones];}
			if( (p->bm&BIT17) && !(RA17&white) ) {runaway+=promoteinthree;runaway-=4*tmod[stones];}
			if( (p->bm&BIT18) && !(RA18&white) ) {runaway+=promoteinthree;runaway-=4*tmod[stones];}
			if( (p->bm&BIT19) && !(RA19&white) ) {runaway+=promoteinthree;runaway-=4*tmod[stones];}
			}
		if( (p->wm) & 0x0000F000)
			{
			if( (p->wm&BIT12) && !(RA12&black) ) {runaway-=promoteinthree;runaway+=4*tmod[stones];}
			if( (p->wm&BIT13) && !(RA13&black) ) {runaway-=promoteinthree;runaway+=4*tmod[stones];}
			if( (p->wm&BIT14) && !(RA14&black) ) {runaway-=promoteinthree;runaway+=4*tmod[stones];}
			if( (p->wm&BIT15) && !(RA15&black) && !(BIT14 & p->bk)) {runaway-=promoteinthree;runaway+=4*tmod[stones];}
			}

	/* bridge situations */
	/* for black */
		/* todo: add bridge code: if kings get out from bridge, good. if not, bad! */
		/* maybe special bridge code for 8-piece ending? */
	if(p->bm&BIT21)
		{
		if( (p->wm&BIT28) && (p->wm&BIT30) && (free&BIT29) ) // vtune: use &
			{
				blackhasbridge=1;
				/* black has a bridgehead on BIT21 - maybe account for weakness?
				e.g. if p->wk -> penalty, or if wk>bk -> penalty?*/
			/* left side */
			if( (free&BIT24) && (free&BIT25) && (p->bm&BIT20) ) // vtune: use &
				{runaway+=promoteintwo;runaway-=5*tmod[stones];}
			/* right side */
			if( (free&BIT26) && (free&BIT27) && (p->bm&BIT22) ) // vtune: use &
				{runaway+=promoteintwo;runaway-=5*tmod[stones];}
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
	      BLACK
*/
	/* for white */
	if(p->wm&BIT10)
		{
		if( (p->bm&BIT1) && (p->bm&BIT3) && (free&BIT2) ) // vtune: use &
			{
				/*white has a bridgehead on BIT10 - maybe account for weakness?*/
				whitehasbridge=1;
			/* left side */
			if( (free&BIT4) && (free&BIT5) && (p->wm&BIT9) ) // vtune: use &
				{runaway-=promoteintwo;runaway+=5*tmod[stones];}
			/* right side */
			if( (free&BIT6) && (free&BIT7) && (p->wm&BIT11) ) // vtune: use &
				{runaway-=promoteintwo;runaway+=5*tmod[stones];}
			}
		}
	
		eval+=runaway;

		/* new cramp eval: a generalized cramp evaluation for 4-blocks */
		/* find black 'blocks' */
		m = p->bm & leftforward(p->bm) & rightforward(p->bm) & (p->bm<<8) & leftforward(free<<8) & rightforward(free<<8);
		blackcramps=bitcount(m);

		m = p->wm & leftforward(p->wm) & rightforward(p->wm) & (p->wm<<8) & leftbackward(free) & rightbackward(free);
		whitecramps=bitcount(m);

		eval+= newcrampval*(whitecramps-blackcramps);
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
/* how about a mobility eval?! */

/*************** everything about kings... ********************/
   if(kings)
		{
   		/* king center control */
   		eval+=kcval*bitcount(p->bk&CENTER);
   		eval-=kcval*bitcount(p->wk&CENTER);
   		/* kings on edge */
   		eval+=keval*bitcount(p->bk&EDGE);
   		eval-=keval*bitcount(p->wk&EDGE);
		
		
   		/* free and trapped kings */
		
		/* king trapped by one man in single corner */
		
		if( (p->wk&BIT0) && (p->bm&BIT1)) // vtune use &
			{
			if (free&BIT8)
				{
				freewk--;   /* old code takes care of a king trapped by a man */
				trappedking+=kingtrappedinsinglecornerval;
				untrappedwk^=BIT0;
				}
			}
		if( (p->bk&BIT31) && (p->wm&BIT30) ) 
			{
			if (free&BIT23)
				{
				freebk--;
				trappedking-=kingtrappedinsinglecornerval;
				untrappedbk^=BIT31;
				}
			}
		/* kings trapped in bridges on square 2,3 or 30,31*/
		/* new in 1.35 */
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
		/* with this, it solves the trapped king on 30 testpos much faster!*/
		/* together with weak man on 11 it hurts match result - test each separately later! */
		if(p->bk&SQ31)
			{
			if( (p->wm&SQ32) && (p->wm&SQ30) && (free&SQ23)) 
				{
				freebk--;
				untrappedbk^=SQ31;
				}
			}
		if(p->bk&SQ30)
			{ 
			if( ((p->wm&SQ31) && (p->wm&SQ29) && (free&SQ22)) || ((p->wm&SQ21) && (p->wm&SQ25) && (p->wm&SQ31) && (free&SQ22)) )
				{
				freebk--;// above new 1.376 - add penalties here?
				untrappedbk^=SQ30;
				}
			// new in 1.41b - dynamic trapping of a king
			else if(match1(p->wm, BIT27|BIT23))
				{
				freebk--;
				untrappedbk^=SQ30;
				}
			}
		if(p->wk&SQ2)
			{
			if( (p->bm&SQ1) && (p->bm&SQ3) && (free&SQ10))// vtune use &
				{
				freewk--;
				untrappedwk^=SQ2;
				}
			}
		if(p->wk&SQ3)
			{											// below new 1.376
			if( ((p->bm&SQ2) && (p->bm&SQ4) && (free&SQ11))  || ((p->bm&SQ2) && (p->bm&SQ8) && (p->bm&SQ12) && (free&SQ11)) )
				{
				freewk--;
				untrappedwk^=SQ3;
				}
			else if (match1(p->bm, BIT4|BIT8))
				{
				freewk--;
				untrappedwk^=SQ3;
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
		/* king assisted by man but trapped by two men in single corner*/
		/* should expand this to take care of situation where there is a man on 25 
			going to king -> gets runaway bonus and makes potbk */
		if( p->wk&(BIT0|BIT4) )
			{
			if( p->wm&BIT8)
				{
				if( (p->bm&(BIT1|BIT5)) == (BIT1|BIT5) )
					{
					if( free & BIT12 )
						{
						trappedking+=kingtrappedinsinglecornerbytwoval;
						freewk--;
						untrappedwk^=(p->wk&(BIT0|BIT4));
						}
					}
				}
			}
		if( p->bk&(BIT27|BIT31) )
			{
			if( p->bm&BIT23)
				{
				if( (p->wm&(BIT30|BIT26)) == (BIT30|BIT26) )
					{
					if(free & BIT19) // maybe without this?
					{
					trappedking-=kingtrappedinsinglecornerbytwoval;
					freebk--;
					untrappedbk^=(p->bk&(BIT27|BIT31));
					}
				}
			}
		}

	/* king trapped in double corner by two men */
		if(p->bk & BIT28)
		{
			if((p->wm&BIT24) && (p->wm&BIT29))
			{
			trappedking-=kingtrappedindoublecornerval;
			freebk--;
			untrappedbk^=BIT28;
			}
		}
		if(p->wk & BIT3)
		{
			if( (p->bm&BIT2) && (p->bm&BIT7) )
			{
			trappedking+=kingtrappedindoublecornerval;
			freewk--;
			untrappedwk^=BIT3;
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

	
	// absurd self trapping under a bridge:
	if(match3(p->bm, p->wm, p->wk, (BIT1|BIT3), (BIT5|BIT6|BIT10), BIT2))
		eval+=230;
	if(match3(p->wm, p->bm, p->bk, (BIT28|BIT30), (BIT21|BIT25|BIT26), BIT29))
		eval-=230;


	/* 'absurd' self-trapping in single corner */
	/* in all it's disguises: */
	if(p->wm&SQ30)
		{
		if(p->bm&SQ21)
			{
			if((p->bk&SQ29) && (p->bm&SQ25))
				{
				eval-=200;
				untrappedbk^=SQ29;
				}
			if((p->bk&SQ29) && (p->bm&SQ22))
				{
				eval-=100;
				untrappedbk^=SQ29;
				}
			if((p->bk&SQ25) && (p->bm&SQ22))
				{
				eval-=100;
				untrappedbk^=SQ25;
				}
			if((p->bm&SQ25) && (p->bm&SQ22))
				{
				eval-=100;
				}
			}
		if((p->bk&SQ29) && (p->bm&SQ22) && (free&SQ21))
			{
			eval-=50;
			untrappedbk^=SQ29;
			}
		/* self trapping with a white man on 21 - not so bad, but bad */
		/* new 1.377 */
		/*if((p->wm&SQ21) && (p->bm&SQ22) && (p->bk&(SQ25|SQ29)))
			{
			eval-=10;
			if(p->bm&SQ13)
				eval-=20;
			}*/
		}

	if(p->bm&SQ3)
		{
		if(p->wm&SQ12)
			{
			// todo: both clauses below could be active! not good!
			if(p->wk&SQ4 && p->wm&SQ8)
				{
				eval+=200;
				untrappedwk^=SQ4;
				}
			if(p->wk&SQ4 && p->wm&SQ11)
				{
				eval+=100;
				untrappedwk^=SQ4;
				}
			if(p->wk&SQ8 && p->wm&SQ11)
				{
				eval+=100;
				untrappedwk^=SQ8;
				}
			if(p->wm&SQ8 && p->wm&SQ11)
				{
				eval+=100;
				}
			}
		if(p->wk&SQ4 && p->wm&SQ11 && free&SQ12)// vtune use &
			{
			eval+=50;
			untrappedwk^=SQ4;
			}
		/* self trapping with a white man on 21 - not so bad, but bad */
		/* new 1.377 */
		/*if((p->bm&SQ12) && (p->wm&SQ1) && (p->wk&(SQ8|SQ4)))
			{
			eval+=10;
			if(p->wm&SQ20)
				eval+=20;
			}*/
		}


		/* self trapping in double corner - new in 1.35*/
		/* weights not tested! */
		if(p->bm&BIT2)
			{
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
		*/			/* awful case */
			if( (p->wk&BIT3) && (p->wm&BIT7))
				{
				if(p->bk&BIT10)
					{
					eval+=100;
					freewk--;
					untrappedwk^=BIT3;
					}
				else if((free&BIT11) && (!(p->wm&BIT15))) 
					{
					eval+=30;
					freewk--;
					untrappedwk^=BIT3;
					}
				}

			/* bad cases */
			// todo: probably scrap this one.
			if(p->wm&BIT11)
				{
				if(p->wk&(BIT3|BIT6|BIT7))
					{
					eval+=15;
					untrappedwk^=(p->wk&(BIT3|BIT6|BIT7));
					freewk--;
					}
				}
			}
		if(p->wm&BIT29)
			{
			/* awful case */
			if( (p->bk&BIT28) && (p->bm&BIT24) )
				{
				if(p->wk&BIT21)
					{
					eval-=100;
					freebk--;
					untrappedbk^=BIT28;
					}
				else if( (free&BIT20) && (!(p->bm&BIT16))) 
					{
					eval-=30;
					freebk--;
					untrappedbk^=BIT28;
					}
				}
			/* bad cases */
			// todo: is this case really bad?? maybe this should be handled by the
			// general code which sees if the king can escape.
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
			if(p->bm&BIT20) 
				{
				if(p->bk&(BIT28|BIT25|BIT24))
					{
					eval-=15;
					freebk--;
					untrappedbk^=(p->bk&(BIT28|BIT25|BIT24));
					}
				}
			}

		/* single dominated white king in single corner new 1.35*/
		/* weight not tested! */
	/* special bad case: */
		
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
		if(p->bm&BIT1)
			{
			if(p->wm&BIT8)
				{
				/* special bad case: black king trapping wk on 4, and stopping a man on 20*/
				/* if this is true, the others cannot be true, because it requires a BK on bit9*/
				if( p->wk&BIT0 && p->bk&BIT9 && p->wm&BIT16)// vtune use &
					{
					eval+=50;
					freewk--;
					untrappedwk^=BIT0;
					}

				if( (wk==1) && (p->wk&(BIT0|BIT4|BIT9)) && (p->bk&(BIT17|BIT18|BIT12|BIT13)))// vtune use &
					{
					eval+=dominatedkingval;
					freewk--;
					untrappedwk^=(p->wk&(BIT0|BIT4|BIT9));
					}
				if( (wk==0) && (p->wm&(BIT4)) && (p->bk&(BIT17|BIT18|BIT12|BIT13)))
					{
					eval+=dominatedkingval;
					}
				}
			}
		/* single dominated black king in single corner */
		if(p->wm&BIT30)
			{
			if(p->bm&BIT23)
				{
				/* special bad case: white king trapping bk on 29, and stopping a man on 13*/
				if( p->wk&SQ22 && p->bk&SQ29 && p->bm&SQ13)
					{
					eval-=50;
					freebk--;
					untrappedbk^=SQ29;
					}
				if( (bk==1) && (p->bk&(BIT31|BIT27|BIT22)) && (p->wk&(BIT13|BIT14|BIT18|BIT19)))
					{
					eval-=dominatedkingval;
					freebk--;
					untrappedbk^=(p->bk&(BIT31|BIT27|BIT22));
					}
				if( (bk==0) && (p->bm&(BIT27)) && (p->wk&(BIT13|BIT14|BIT18|BIT19)))
					{
					eval-=dominatedkingval;
					}
				}
			}


		/* single dominated white king in double corner - new 1.373 */
		if(p->bm&BIT2 && !(p->bm&BIT7))
			{
			if( (wk==1) && (p->wk&(BIT3|BIT7)) && (p->bk&BIT14) && !(p->wm&BIT15))
				{
				eval+=dominatedkingindcval;
				freewk--;
				untrappedwk^=(p->wk&(BIT3|BIT7));
				}
			if( (wk==0) && (p->wm&(BIT7)) && (p->bk&BIT14) && !(p->wm&BIT15))
				{
				eval+=dominatedkingindcval;
				}
			}
		/* single dominated black king in double corner - new 1.373 */
		if(p->wm&BIT29 && !(p->wm&BIT24))
			{
			if( (bk==1) && (p->bk&(BIT24|BIT28)) && (p->wk&BIT17) && !(p->bm&BIT16))
				{
				eval-=dominatedkingindcval;
				freebk--;
				untrappedbk^=(p->bk&(BIT24|BIT28));
				}
			if( (bk==0) && (p->bm&(BIT24)) && (p->wk&BIT17) && !(p->bm&BIT16))
				{
				eval-=dominatedkingindcval;
				}
			}
		/* king stopping men in endgame - new 1.3 III*/
		/* tailhooks */
		/* with 12 or with even less?*/
		if(bm+bk+wm+wk<=12)
			{
			m=( (0x37370000) & (p->bk) & (leftforward(p->wm)) & (leftforward(leftforward(p->wm))) );
			m|=( (0xECEC0000) & (p->bk) & (rightforward(p->wm)) & (rightforward(rightforward(p->wm))) );
			blacktailhooks = bitcount(m);
			m=( (0x0000ECEC) & (p->wk) & (rightbackward(p->bm)) & (rightbackward(rightbackward(p->bm))) );
			m|=( (0x00003737) & (p->wk) & (leftbackward(p->bm)) & (leftbackward(leftbackward(p->bm))) );
			whitetailhooks = bitcount(m);
			eval+=(blacktailhooks-whitetailhooks)*tailhookval;
			}

		/* king proximity value */
		/* if a king is close to a man of the opposite color, that should
			be dangerous, at least if that man is not on the edge. */
		if(bm+bk+wm+wk <= 12)
			{
			m = forward(p->bk) | backward(p->bk);
			m |= forward(m) | backward(m);
			m = m&(~EDGE)&p->wm;
			kingproximity += bitcount(m);

			m = forward(p->wk) | backward(p->wk);
			m |= forward(m) | backward(m);
			m = m&(~EDGE)&p->bm;
			kingproximity -= bitcount(m);

			eval += kingproximity * kingproximityval;
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
	      BLACK
		*/

		/* king center monopoly */
		/* if one side is the only side to have kings in center, add 12 points to eval*/
		/* tried: if the side with the monopoly has greaterequal kings - else it will lose its monopoly quickly!*/
		/* didnt work! */
		/* try again with freewk/freebk */
		if((p->bk & CENTER) && !(p->wk&CENTER) && (freebk>=freewk))
			eval+=kingcentermonopoly;
		if((p->wk & CENTER) && !(p->bk&CENTER) && (freewk>=freebk))
			eval-=kingcentermonopoly;
		
		/* endangered bridge heads */
		/* new 1.3 III */
		// todo should read: if white king cannot get out from under bridge, then big penalty!
		if(whitehasbridge)
			{
			if(freebk>freewk)
				eval+=2*endangeredbridgeval;
			if(freebk==freewk)   //else if freebk>0 do this not freebk==freewk
				eval+=endangeredbridgeval;
			}
		if(blackhasbridge)
			{
			if(freebk<freewk)
				eval-=2*endangeredbridgeval;
			if(freebk==freewk)
				eval-=endangeredbridgeval;
			}
		/* 'other' bridgeheads */
		/* new 1.35 */
		/* this is too little to stop it from believing in the testpos for this theme   */
		/* together with freeK-- for kings in bridge, this hurts the match result a bit */
		/* test both separately later! */
		/* not tested yet: "if bk" or "if bk>wk" or whatever!*/ 
		if(p->wm&SQ11)
			{
			if(bk && (p->bm&SQ4) && (p->bm&SQ2))
				eval+=endangeredbridgeval;
			}
		if(p->bm&SQ22)
			{
			if(wk && (p->wm&SQ29) && (p->wm&SQ31))
				eval-=endangeredbridgeval;
			}
		
		/* new endgame eval */
		/* if and ending is good for one side, that is, if it has more kings than the other side,
			the objective should be not to continue kinging for both sides. => the stronger side
			should try to stop the men of the opponent with its kings */

	
		if(ending) // number of total stones <=10
			{
			if(freebk>=freewk)
				{
				/* black may be strong */
				/* check for immobilized white men */
				/* only those which are on the edge */
				/* todo: this is stupid. should check for 2 men immobilized by 1 king or so. */
				/*
				   WHITE
   				   28  29  30  31
				 24  25  26  27
				   20  21  K  23
				 16  K  18  19
				   12  13  K  15
				  8  K  10  11
					4   5   6   7
				  0   1   2   3
					  BLACK
					*/				
				tmp = p->bk & 0x00424200; // other kings cannot immobilize two men
				free2 = ~(p->bk|p->bm|p->wk|(p->wm&(~0x81818181))); // a free board without white men on edge
				while(tmp)
					{
					// for all black kings do:
					m = (tmp&(tmp-1))^tmp;
					tmp = tmp^m; 
					// m now holds one black king.
					attack = attack_backwardjump(m, free2) | attack_forwardjump(m, free2);
					m1 = backward(p->wm&0x81818181); // squares white men can move to
					m1 &= attack; // men which can't go because of this king
					m1 = bitcount(m1);
					if(m1 == 2)
						// one king holds two men!
						eval += kingholdstwomenval;
					else if(m1 == 1)
						eval += immobilemanval;
					}
	
				}
			if(freewk>=freebk)
				{
				//white strong 

					/*   WHITE
   				   28  29  30  31
				 24  25  26  27
				   20  21  K  23
				 16  K  18  19
				   12  13  K  15
				  8  K  10  11
				    4  5   6   7
				  0  1   2   3
					  BLACK
					*/				
				tmp = p->wk & 0x00424200; // other kings cannot immobilize two men
				free2 = ~(p->bk|p->wm|p->wk|(p->bm&(~0x81818181))); // a free board without black men on edge
				while(tmp)
					{
					// for all black kings do:
					m = (tmp&(tmp-1))^tmp;
					tmp = tmp^m; 
					// m now holds one black king.
					attack = attack_backwardjump(m, free2) | attack_forwardjump(m, free2);
					m1 = forward(p->bm&0x81818181); // squares white men can move to
					m1 &= attack; // men which can't go because of this king
					m1 = bitcount(m1);
					if(m1 == 2)
						// one king holds two men!
						eval -= kingholdstwomenval;
					else if(m1 == 1)
						eval -= immobilemanval;
					}
			
				}
			}

		// general king mobility: for all untrapped kings, look how far they
		// can go in 3 steps on non-attacked squares. and if they can go in center.

		while(untrappedbk)
			{
			// peel off untrapped kings and analyze them
			tmp=(untrappedbk&(untrappedbk-1))^untrappedbk;
			untrappedbk^=tmp;
			free2=~(p->bm|p->bk|p->wm|p->wk);
			free2^=tmp;
			// free2 contains all free squares + the square of the potentially trapped king.

			attack = attack_backwardjump((p->wm|p->wk),free2);
			attack |= attack_forwardjump(p->wk,free2);
			// simulate 3 moves with king on tmp only over non-attacked squares
			tmp|=(forward(tmp)|backward(tmp));
			tmp&=(~attack);
			tmp&=free2;
			tmp|=(forward(tmp)|backward(tmp));
			tmp&=(~attack);
			tmp&=free2;
			tmp|=(forward(tmp)|backward(tmp));
			tmp&=(~attack);
			tmp&=free2;
			m=bitcount(tmp);
			if(!(tmp&CENTER))
				eval-=2;  //was 5 in 1.425
			if(m<=4)
				eval-=(2*(5-m)); // was 6 in 1.425
			if(m<=1)
				{
				freebk--; // 1.426''''
				eval-=10;
				}
			}
		
		
		while(untrappedwk)
			{
			// peel off untrapped kings and analyze them
			tmp=(untrappedwk&(untrappedwk-1))^untrappedwk;
			untrappedwk^=tmp;
			free2=~(p->bm|p->bk|p->wm|p->wk);
			free2^=tmp;
			attack = attack_forwardjump((p->bm|p->bk),free2);
			attack |= attack_backwardjump(p->bk,free2);
			// simulate 3 moves with king on tmp only over non-attacked squares
			tmp|=(forward(tmp)|backward(tmp));
			tmp&=free2;
			tmp&=(~attack);
			tmp|=(forward(tmp)|backward(tmp));
			tmp&=free2;
			tmp&=(~attack);
			tmp|=(forward(tmp)|backward(tmp));
			tmp&=free2;
			tmp&=(~attack);
			m=bitcount(tmp);
			if(!(tmp&CENTER))
				eval+=2;  // was 5 in 1.425'
			if(m<=4)
				eval+=(2*(5-m)); //was 6* in 1.425'
			if(m<=1)
				{
				freewk--; // 1.426''''
				eval+=10;
				}
			}

		} /* end things only if king ! */


	
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
	/* only king */
   /* the point of this is to make cake++ sacrifice a man if it gets a king,
      has a strong back rank and gets its king in the center */
      /* sacrificed man=-100
      	king -> +30 total -70
         only king +20 total -50
         strong br +30...40 total -20...-10
         king in center +32 +20
      king in center has such a high value because cake sometimes sacrifices
      a man for a trapped king which isn't so obviously trapped */

   /* single black king */
	if((freebk) && (freewk==0) && (potwk==0))			//1.35 beta A' +63-36! by far the best!
   		{
		// if black has sacced a man we want to extend this line:
		if(bm == wm-2)
			*delta = NOPRUNEWINDOW;

		onlyking=onlykingval;
		//onlyking+=(2*brblack);  /* was -brwhite still */
		// only if we have a really cool back rank do we go for this
		// todo: this is too crude: rather use if this king is considered free or not!
		onlyking+=blackbackrankpower[p->bm&0xFF];
		if(p->bk & ROAMINGBLACKKING)
      		onlyking+=roamingkingval;     
		}	
   /* single white king */
	if( (freewk) && (freebk==0) && (potbk==0) )
   		{
		// if white has sacced a man for this we want to extend:
		if(wm == bm-2)
			*delta = NOPRUNEWINDOW;

		onlyking=-onlykingval;
		//onlyking-=(2*brwhite); /* was +=brblack still */
		onlyking-=whitebackrankpower[p->wm>>24];
		if(p->wk & ROAMINGWHITEKING)
      		onlyking-=roamingkingval;
		}
   eval+=onlyking;

   // new in 1.41b: man down evaluation for positions with only men:
	// this looks like it's too optimistic. it works in the sense that
   // it produces man sacs, but it makes for a worse match result. 
	// should include all dogholes, because the "other" (as opposed to real)
   // doghole is also a factor in man-down-positions, i.e. 4th position depends on that.
   // the whole thing now looks rather far-fetched.

   // should include: man-down? => if many men are on board, check
   //		-> tempo (must be very good (5 tempo = 1 man, DEO))
   //		-> own back rank (must be good)
   //		-> opposing back rank (must be weak)
   //		-> men in dogholes (good if opposing side has that)
   //		-> holds - for every hold on opponent pos, good. 
	//		-> runaway>0 *in combination with* strong own back rank.

   // if there are not so many men, then we only look for all of the
   // above without tempo.

   if(!kings)
	   {
	   if(bm==wm+1)
		   {
		   // black is a man up
		   // look for compensation in form of intact back rank and runaway
		   // condition for this to be looked at: at least 13 men on the board,
		   // tempo very good for side without man, intact back rank for side with
		   // man down, and not quite intact back rank for the other side. else
		   // we don't even look....

		   //if(runaway>0) // would be 12...24 depending on where that man is 141b'
			//   {
			  // eval+=blackbackrankpower[p->bm&0xFF]; // can be up to 40
			  // if(realdoghole & WHITE) eval+=5;      // and another 5
			   // tempo is positive, if black is more advanced
			  // eval -= 2*tempo;						// assuming tempo to be down by 5, 25, for a total of nearly 100
			  // }
			if(stones>12 && runaway<=0) //many stones, black does not have a runaway checker
				{
				if(whitebackrankpower[p->wm>>24] && runaway <0)
					// white has a good back rank, then we want to look at this carefully
			   		*delta = NOPRUNEWINDOW;
			   	
				if(tempo>4 && whitebackrankpower[p->wm>>24] )
					{
					//*delta = NOPRUNEWINDOW;
					eval -= 2*tempo;
					// check back ranks
					eval-=whitebackrankpower[p->wm>>24];
					eval+=blackbackrankpower[p->bm&0xFF];
					// augment doghole values
					if(p->bm&BIT24)
						eval-=4;
					if(p->bm&BIT23)
						eval-=4;
				   }
			   }

		   }

//       WHITE
//   	28  29  30  31
//	 24  25  26  27
//	   20  21  22  23
//	 16  17  18  19
//	   12  13  14  15
//	  8   9  10  11
//	    4   5   6   7
//	  0   1   2   3
//	      BLACK


	   if(bm==wm-1)
		   {
		   // white is a man up
		   //if(runaway<0)
		//	   {
		//	   eval-=whitebackrankpower[p->wm>>24];
		//	   if(realdoghole & BLACK) eval-=5;
		//	   eval += 2*tempo;
		//	   }
		   if(stones>12 && runaway>=0) //1426
			   {
			   if(blackbackrankpower[p->bm&0xFF] && runaway>0)
				   *delta = NOPRUNEWINDOW;

			   if(tempo<-4 && blackbackrankpower[p->bm&0xFF] )
					{
					//*delta = NOPRUNEWINDOW;
					eval -=2*tempo; // black men less advanced than white -> good
					eval +=blackbackrankpower[p->bm&0xFF];
					eval -=whitebackrankpower[p->wm>>24];
					if(p->wm&BIT8)
						eval+=4;
					if(p->wm&BIT7)
						eval+=4;
				   }
			   }
		   }
	   }
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


#ifdef SHOWEVAL
	if(immobilewhitemen+immobileblackmen)
		{
		printboard(*p);
		if(color==BLACK)
			printf("\nblack to move");
		else
			printf("\nwhite to move");
		printf("\nimmobile men: black %i   white %i",immobileblackmen, immobilewhitemen);
		getch();
		}
#endif
   return color==BLACK ? eval:-eval;
   }


int32 attack_forwardjump(int32 x, int32 free)
	{
	// return all squares which are forward-attacked by pieces in x, with free
	return ( ((((x&LFJ1)<<3)&free)&(free>>4)) | ((((x&LFJ2)<<4)&free)&(free>>3)) | ((((x&RFJ1)<<4)&free)&(free>>5)) | ((((x&RFJ2)<<5)&free)&(free>>4)) );
	}

int32 attack_backwardjump(int32 x, int32 free)
	{
	return ( ((((x&LBJ1)>>5)&free)&(free<<4)) | ((((x&LBJ2)>>4)&free)&(free<<5)) | ((((x&RBJ1)>>4)&free)&(free<<3)) | ((((x&RBJ2)>>3)&free)&(free<<4)) );
	}

int32 forwardjump(int32 x, int32 free, int32 other)
	{
	return ( (((((x&LFJ1)<<3)&other)<<4)&free) | (((((x&LFJ2)<<4)&other)<<3)&free) | (((((x&RFJ1)<<4)&other)<<5)&free) | (((((x&RFJ2)<<5)&other)<<4)&free) );
	}

int32 backwardjump(int32 x, int32 free, int32 other)
	{
	return ( (((((x&LBJ1)>>5)&other)>>4)&free) | (((((x&LBJ2)>>4)&other)>>5)&free) | (((((x&RBJ1)>>4)&other)>>3)&free) | (((((x&RBJ2)>>3)&other)>>4)&free) );
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
	// only check for rows 2-6 


/* table-lookup bitcount */
int bitcount(int32 n) // vtune: make this inlined maybe as a macro
	/* returns the number of bits set in the 32-bit integer n */
	{
	return (bitsinword[n&0x0000FFFF]+bitsinword[(n>>16)&0x0000FFFF]);
	}

int LSB(int32 x)
	{
	//-----------------------------------------------------------------------------------------------------
	// returns the position of the least significant bit in a 32-bit word x 
	// or -1 if not found, if x=0.
	// LSB can maybe be implemented more efficiently on other CPU's which have a 
	// this operation.
	//-----------------------------------------------------------------------------------------------------
	__asm
		{
		mov	eax, -1		// change -1 to error value you want
		bsf	eax, dword ptr x 
		}
// code above very nonportable :-)
// use arrays to do it like this in a portable manner
/*	if(x&0x000000FF)
		return(LSBarray[x&0x000000FF]);
	if(x&0x0000FF00)
		return(LSBarray[(x>>8)&0x000000FF]+8);
	if(x&0x00FF0000)
		return(LSBarray[(x>>16)&0x000000FF]+16);
	if(x&0xFF000000)
		return(LSBarray[(x>>24)&0x000000FF]+24);
	return -1;
*/	
	}



void updatehashkey(struct move *m)
	{
	/* given a move m, updatehashkey calculates the new hashkey (Glock, Gkey)
		incrementally */
	int32 x,y;
	x=m->bm;
	while(x)
		{
		y=LSB(x);
		Gkey ^=hashxors[0][0][y];
		Glock^=hashxors[1][0][y];
		x&=(x-1);
		}
	x=m->bk;
	while(x)
		{
		y=LSB(x);
		Gkey ^=hashxors[0][1][y];
		Glock^=hashxors[1][1][y];
		x&=(x-1);
		}
	x=m->wm;
	while(x)
		{
		y=LSB(x);
		Gkey ^=hashxors[0][2][y];
		Glock^=hashxors[1][2][y];
		x&=(x-1);
		}
	x=m->wk;
	while(x)
		{
		y=LSB(x);
		Gkey ^=hashxors[0][3][y];
		Glock^=hashxors[1][3][y];
		x&=(x-1);
		}
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

void absolutehashkey(struct pos *p)
	{
	/* absolutehashkey calculates the hashkey completely. slower than
		using updatehashkey, but useful sometimes */

	int32 x;

	Glock=0;
	Gkey=0;
	x=p->bm;
	while(x)
		{
		Gkey ^=hashxors[0][0][LSB(x)];
		Glock^=hashxors[1][0][LSB(x)];
		x&=(x-1);
		}
	x=p->bk;
	while(x)
		{
		Gkey ^=hashxors[0][1][LSB(x)];
		Glock^=hashxors[1][1][LSB(x)];
		x&=(x-1);
		}
	x=p->wm;
	while(x)
		{
		Gkey ^=hashxors[0][2][LSB(x)];
		Glock^=hashxors[1][2][LSB(x)];
		x&=(x-1);
		}
	x=p->wk;
	while(x)
		{
		Gkey ^=hashxors[0][3][LSB(x)];
		Glock^=hashxors[1][3][LSB(x)];
		x&=(x-1);
		}

	}



#ifdef PV
int update_pv(int move, int realdepth)
	{
	int i;
	
	pv[realdepth][realdepth]=move;
	for(i=realdepth+1;i<MAXDEPTH;i++)
		pv[realdepth][i]=pv[realdepth+1][i];
	return 0;
	}
#endif

void countmaterial(struct pos *p)
	{
	/* countmaterial initializes the globals bm, bk, wm and wk, which hold
		the number of black men, kings, white men and white kings respectively.
		during the search these globals are updated incrementally */
	bm=bitcount(p->bm);
	bk=bitcount(p->bk);
	wm=bitcount(p->wm);
	wk=bitcount(p->wk);
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
      from=LSB(from);
      to=LSB(to);
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
      from=LSB(from);
      to=LSB(to);
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

#ifdef PV
void getrealpv(char *str,int color)
	{
	/* retrieves the principal variation from the 'deep' hashtable:
		looks up the position, finds the hashtable move, plays it, looks
		up the position again etc.
		color is the side to move, the whole PV gets written in *str */

	struct move movelist[MAXMOVES];
	int32 forcefirst;
	int dummy=0;
	int i,n;
	char Lstr[1024];
	struct pos Lp;
	int values[MAXMOVES];

	Lp=p;
	absolutehashkey(&p);
	sprintf(str," pv: ");
	for(i=0;i<DEEPLEVEL;i++)
		{
		printf("\n");
		for(n=0;n<DEEPLEVEL;n++)
			printf("%i ",pv[i][n]);
		
		
		//strcat(str,Lstr);
		/*forcefirst=pv[0][i];
		n=makecapturelist(&p,movelist,values,color,forcefirst);
		if(!n)
			n=makemovelist(&p,movelist,values,color,forcefirst,0);
		if(!n) {p=Lp;absolutehashkey(&p);return;}

		movetonotation(p,movelist[forcefirst],Lstr,color);
		strcat(str,Lstr);
		strcat(str," ");
		togglemove(movelist[forcefirst]);
		absolutehashkey(&p);		
		color=color^CC;*/
		}
	p=Lp;
	absolutehashkey(&p);
	return;
	}
#endif

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

int booklookup(int *value, int depth, int32 *remainingdepth, int color, int *best)
	{
	/* searches for a position in the book hashtable.
	*/
	int32 index;
	int iter=0;
	
	// todo: change this to bookhashentry!
	struct hashentry *pointer;
	int bucketsize;
	int size;

	pointer=book;
	size=bookentries;
	bucketsize=BUCKETSIZEBOOK;
	
	if(pointer==NULL)
		return 0;
	
	index=Gkey% size;
	
	while(iter<bucketsize)
		{
		if(pointer[index].lock==Glock && ((int)pointer[index].color==color)) // use &
		//if(pointer[index].lock==Glock && ((int)pointer[index].color==(color>>1))) // use &
			{
			/* we have found the position */
			*remainingdepth=pointer[index].depth;
			*value=pointer[index].value;
			*best = pointer[index].best;
			return 1;
			}
		iter++;
		index++;
		index%=size;
		}
	return 0;
	}

int isforced(struct pos *p,int color)
	// determines if color should move immediately
	{
	int i,n;
	struct move ml1[MAXMOVES];
	struct pos q1,q2;
	int values[MAXMOVES];
	int bestindex=0;

	// forced moves occur if a) only one capture move exist
	// and b) if 2 capture moves exist which transpose with 2 moves, then 4 single captures
	n = makecapturelist(p, ml1, values, color, bestindex);
	if(n==1)
		return 1;

	if(n!=2)
		return 0;

	// if we arrive here, there are two capture moves.
	// copy position so we dont wreck anything
	q1.bm = p->bm;
	q1.bk = p->bk;
	q1.wm = p->wm;
	q1.wk = p->wk;
	q2=q1;

	togglemove(q1,ml1[0]);
	togglemove(q2,ml1[1]);

	color^=CC;
	for(i=0;i<3;i++)
		{
		n=makecapturelist(&q1, ml1, values, color, bestindex);
		if(n!=1)
			return 0;
		togglemove(q1, ml1[0]);
		color^=CC;
		}

	color^=CC;

	for(i=0;i<3;i++)
		{
		n=makecapturelist(&q2, ml1, values, color, bestindex);
		if(n!=1)
			return 0;
		togglemove(q2, ml1[0]);
		color^=CC;
		}

	if(q1.bk==q2.bk && q1.bm==q2.bm && q1.wm==q2.wm && q1.wk==q2.wk)
		return 1;

	return 0;
	}0;
	}
#include <stdlib.h> 
#include <assert.h>
#include <windows.h>

// test what happens if I write something here 8.5.2019 07:42
// cake-specific includes - structs defines structures, consts defines constants,
// xxx.h defines function prototypes for xxx.c
#include "switches.h"
#include "structs.h"
#include "consts.h"
#include "cake_eval.h"
#include "dblookup.h"
#include "initcake.h"
#include "cakepp.h"

// tables for evaluation
static short materialeval[13][13][13][13];						// bm bk wm wk
char  blackbackrankeval[256],whitebackrankeval[256];			// not static because it's used too in movegen for move ordering
static char  blackbackrankpower[256], whitebackrankpower[256];	// used for man down situations


#define PARAMS 150
int v[PARAMS];


//static int ungroundedpenalty[13] = { -4,-4,-1,4,10,19,27,36,30,17,28,28,28 }; // optimized
//static int ungroundedpenalty[13] = { -3,-3,0,4,10,19,27,33,28,14,14,14,14 }; // optimized
//static int ungroundedpenalty[13] = { -5,-4,-1,4,10,20,30,39,38,25,7,7,7 }; // optimized
//static int ungroundedpenalty[13] = { -4,-4,-1,3,9,16,24,31,29,20,20,20,20 }; // optimized
static int ungroundedpenalty[13] = { -3,-3,0,4,9,16,22,28,23,21,21,21,21}; // optimized

//static int br[32] = { 0,0,2, 2, 4,6,10,10,1,4,16,16,6,10,24,16,			// old before optimization
//						   0,0,2, 2, 4,10,16,16,1,4,16,16,6,10,24,16 };
//static int br[32] = { -1,-10,-8, -15, 1,0,7,5,
//					1,-12,14,-2,13,11,19,13,			
//					-5,-22,-7, -17, 2,-3,8,2,
//					1,-17,15,-8,19,17,26,21 };

//static int br[32] = { -9,-18,-13, -20, -6,-6,3,0,
//					0,-17,10,-6,8,7,16,13,
//					-12,-28,-12, -22, -4,-9,3,0,
//					-6,-24,9,-14,13,11,22,19 };
static int br[32] = { -8,-17,-13, -20, -6,-6,2,-2,
					-5,-18,9,-8,8,5,14,9,
					-11,-28,-12, -23, -4,-9,3,-1,
					-6,-23,9,-15,14,11,21,17 };

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


int setparams(int* params, int n) {
	int i; 

	for (i = 0; i < n-13; i++)
		v[i] = params[i];

	for (i = 0; i < 13; i++) {
		ungroundedpenalty[i] = params[arraystart + i];
	}

	for (i = 0; i < 32; i++) {
		br[i] = params[arraystart + 13 + i];
	}

	return 0; 
}

int getparams(int* params, int* n) {
	int i; 
	*n = arraystart; 
	for (i = 0; i < (*n); i++)
		params[i] = v[i]; 

	*n = (arraystart + 13); 
	for (i = arraystart; i < (*n); i++)
		params[i] = v[i];

	*n = (arraystart + 13 + 32);
	for (i = arraystart + 13; i < (*n); i++)
		params[i] = v[i]; 
}

int optimalparams() {
	// meant to set Cake's parameters to the optimal values
	int i; 
	v[devsinglecorner] = 2; 
	v[intactdoublecorner] = 2; 
	v[oreoval] = 6; 
	v[idealdoublecornerval] = 9; 
	v[backrankpower1] = 37; 
	v[backrankpower2] = 47; 
	v[backrankpower3] = 53; 
	v[king_value] = 114;
	v[nocrampval] = 2; 
	v[dogholeval] = 19;
	v[dogholemandownval] = 2;
	v[mc_occupyval] = -2; 
	v[mc_attackval] = 2; 
	v[realdykeval] = 0;
	v[greatdykeval] = 1;
	v[promoteinone] = 12;
	v[promoteintwo] = 8; 
	v[promoteinthree] = 4; 
	v[tailhookval] = 12; 
	v[kcval] = 5; 
	v[keval] = -4; 
	v[turnval] = -1; 
	v[kingcentermonopoly] = 5; 
	v[kingtrappedinsinglecornerval] = 34; 
	v[kingtrappedinsinglecornerbytwoval] = 13;
	v[kingtrappedindoublecornerval] = 12;
	v[dominatedkingval] = 24;
	v[dominatedkingindcval] = 38;
	v[kingproximityval] = 4;
	v[immobilemanval] = 1;
	v[kingholdstwomenval] = 17;
	v[onlykingval] = 11;
	v[roamingkingval] = 6;
	v[man_value] = 96;
	v[balancemult] = 3;
	v[skewnessmult] = 8;
	v[cramp12] = 4;
	v[cramp13] = 23;
	v[cramp20] = 3;
	v[badstructure] = 6;
	v[dogholeval2] = 19;
	v[badstructure2] = 6;
	v[badstructuremax1] = 33;
	v[badstructuremax2] = 27;
	v[badstructuremin] = 7;
	v[badstructure3] = 5;
	v[badstructure4] = 5;
	v[badstructure2stones] = 12;
	v[kingmanstones] = 11;
	v[immobile_mult] = 3;
	v[runaway_destroys_backrank] = 11;
	v[king_blocks_king_and_man] = 80;
	v[king_denied_center] = 0;
	v[king_low_mobility_mult] = 3;
	v[king_no_mobility] = -10;
	v[experimental_king_cramp] = 33;
	v[compensation] = 74;
	v[compensation_mandown] = 35;


	// up to here was best version by far ever, then I added ungroundedcontact and endangeredbridge; 
	// remove those again if they are not better. 
	//v[ungroundedcontact] = 3;// 0;
	//v[endangeredbridge] = 7;// 0;
	for (i = 0; i < 13; i++)
		v[arraystart + i] = ungroundedpenalty[i];

	for (i = 0; i < 32; i++)
		v[arraystart + i + 13] = br[i]; 
	return 0; 
}


int startparams() {
	int i; 
	// sets parameters to 0 or to something halfway to 0 between optimized values so that 
	// optimizer has to find new values
	v[devsinglecorner] = 0;// 2;// 5;// 7;  // optimized? 5;
	v[intactdoublecorner] = 0;// 5;// 1;  // optimized? 5;
	v[oreoval] = 0; // 8; // 7; // optimized?  8;
	v[idealdoublecornerval] = 0; // 11;// 4; // 11;  // optimized? 4;
	v[backrankpower1] = 15;
	v[backrankpower2] = 15;
	v[backrankpower3] = 15;
	v[king_value] = 120;
	v[nocrampval] = 0; // 2;  // optimized 1;
	v[dogholeval] = 10;// 8;// 16;  // optimized 8;
	v[dogholemandownval] = 0;// 20;// 6;  // optimized 20;
	v[mc_occupyval] = 0;// 1;// 0; // optimized 1;
	v[mc_attackval] = 0; // 1; // optmized 1;
	v[realdykeval] = 0;// 5;// 1;  // optimized 5;
	v[greatdykeval] = 0;// 2;// 5;  //  optimized 2;
	v[promoteinone] = 10;// 24;// 16;  //optimized 24;
	v[promoteintwo] = 5; // 20;// 8;  // optimized 20;
	v[promoteinthree] = 0;// 12;// 4;  // optimized 12;
	v[tailhookval] = 8;// 10;// 15;  // optimized 10; // tailhookval is very small!
	v[kcval] = 0;// 4;// 9;  // optimized 4;
	v[keval] = 0;// -4;// -3;  // optimized -4;
	v[turnval] = 0; // 3;// -1;  // optimized?  3;
	v[kingcentermonopoly] = 0; // 12;// -2;  //  12; optimized?
	v[kingtrappedinsinglecornerval] = 15; // 20;// 40;  // 20; optimized?
	v[kingtrappedinsinglecornerbytwoval] = 7;// 20;// 7;  // 20; optimized?
	v[kingtrappedindoublecornerval] = 7;// 10;// 17;  // 10; optimized?
	v[dominatedkingval] = 10;// 10;// 26; // 10; optimized?
	v[dominatedkingindcval] = 20;// 10;// 58; // 10; optimized?
	v[kingproximityval] = 0;// 4;// 7; // 4; optimized?
	v[immobilemanval] = 0;// 2;// 3; // 2; optimized?
	v[kingholdstwomenval] = 8;// 10;// 28;  // 10; optimized?
	v[onlykingval] = 5;// 20;// 6; // 20; optimized?
	v[roamingkingval] = 5;// 40;// 16; // 40; optimized?
	v[man_value] = 100;
	v[balancemult] = 0;    // optimized? 1
	v[skewnessmult] = 4;		// optimized? 1
	v[cramp12] = 3;
	v[cramp13] = 15;
	v[cramp20] = 0;
	v[badstructure] = 5;
	v[dogholeval2] = 8;
	v[badstructure2] = 0;
	v[badstructuremax1] = 28;
	v[badstructuremax2] = 28;
	v[badstructuremin] = 10;
	v[badstructure3] = 3;
	v[badstructure4] = 5;
	v[badstructure2stones] = 10; 
	v[kingmanstones] = 8; 

	v[immobile_mult] = 0;
	v[runaway_destroys_backrank] = 0;
	v[king_blocks_king_and_man] = 60;
	v[king_denied_center] = 0;
	v[king_low_mobility_mult] = 0;
	v[king_no_mobility] = 0;
	v[experimental_king_cramp] = 40;
	v[compensation] = 40;
	v[compensation_mandown] = 0;


	//v[ungroundedcontact] = 0;
	//v[endangeredbridge] = 3;

	for (i = 0; i < 13; i++)
		v[arraystart + i] = ungroundedpenalty[i] / 2; 

	for (i = 0; i < 32; i++)
		v[arraystart + i + 13] = br[i] / 2; 

	return 0;
}



// tiny decider functions what no move, no material means (used so that cake can easily be 
// configured to play different versions (kingscourt, suicide))
int evaluation_nomaterial_black(POSITION *p, int depth) 
	{
	return (p->color == BLACK) ? (-MATE+depth):(MATE-depth);
	}
	
int evaluation_nomaterial_white(POSITION *p, int depth) 
	{
	return (p->color == WHITE) ? (-MATE+depth):(MATE-depth); 
	}

int evaluation_nomove(int depth)
	{
		return -MATE+depth;
	}
// some helper functions

int initeval(void) {
	// initializes the evaluation completely
	optimalparams();
	initializematerial(materialeval);
	initializebackrank(blackbackrankeval, whitebackrankeval, blackbackrankpower, whitebackrankpower);
	return 1;
	}

int updateeval(void) {
	// updates the eval once new parameters have been set
	initializematerial(materialeval);
	initializebackrank(blackbackrankeval, whitebackrankeval, blackbackrankpower, whitebackrankpower);
	return 1;
}

int staticevaluation(SEARCHINFO *si, EVALUATION *e, POSITION *q, int *total, int *material, int *positional, int *delta)
	{
	int noprune=0;
	int likelydraw=0;
	KINGINFO ki; 
	
	countmaterial(q, &(si->matcount));

	*total = evaluation(q,&(si->matcount),0,&noprune,0,0);
	e->material = materialevaluation(&(si->matcount));
	e->selftrap = selftrapeval(q, &(si->matcount), &ki, delta);
	e->backrank = (blackbackrankeval[q->bm & 0xFF] - whitebackrankeval[q->wm >> 24]); 
	
	*positional = fineevaluation(e, q, &(si->matcount), &ki, &noprune, &likelydraw);
	if(q->color == WHITE)
		{
		e->backrank = -e->backrank;
		e->compensation = -e->compensation;
		e->cramp = -e->cramp;
		e->hold = -e->hold;
		e->king = -e->king;
		e->king_man = -e->king_man;
		e->men = -e->men;
		e->runaway = -e->runaway;
		}
	*delta = noprune;
	return *total;
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

int materialevaluation(MATERIALCOUNT *mc)
	{
	return materialeval[mc->bm][mc->bk][mc->wm][mc->wk];
	}

int evaluation(POSITION *p, MATERIALCOUNT *mc, int alpha, int *delta, int capture, int maxNdb)
	{

	//	evaluation returns the value of the position with color to move,
	//	and with current search bounds alpha and beta
	//
	//	evaluation is in fact just a material/endgame database evaluation.
	//	if the evaluation is outside the alpha-beta search window, extended
	//	by the amount FINEEVALWINDOW, it returns the material value only.
	//	else, it calls the fine evaluation function to get a better evaluation
	//	
	//	-- evaluation is not to be called with a capture for either side on the board!
	//	delta tells the search if this position is "quiet" in a positional sense
	//	delta is the extent of how uncertain the eval is about the evaluation 
	//	-- evaluation is not to be called with one side having no material.

    int eval;
	int dbresult = -1;
	int cl = 0;
	int fe;
	int likelydraw = 0; // is set by fineeval if it thinks a draw will likely happen (ending & lots'o'kings)
#ifdef SPA
	int index;
#endif
	KINGINFO ki;	// is a structure to pass around info on kings between different parts of the eval.
	
#ifdef EVALOFF
	return 0;
#endif

	EVALUATION e;
	// check that there is material left 
	assert(p->bm|p->bk);
	assert(p->wm|p->wk);

	// the following assert is violated by getpv() but should hold otherwise
	//assert(alpha == beta-1);
	
#ifdef USEDB
	// TODO: check whether we can look up here without danger: perhaps we have stmcapture or
	// sntmcapture!
	// it seems to me that evaluation is being called with positions that cannot be looked up.
	// that appears to be the case most certainly....
	// TODO: find out why this code is ever called at all!
	if(capture == 0 && mc->bm + mc->bk + mc->wm + mc->wk <= maxNdb)
		{
		//printf(".");
		cl=1;
		dbresult = dblookup(p, cl, mc);

		// TODO: i am a bit mystified: when could eval ever be called with it making sense that
		// there is a dblookup? for pruning decisions?
		if(dbresult==DB_DRAW)
			return 0;
		else
			{
			if(dbresult == DB_WIN)
				{
				return dbwineval(p, mc);
				}
			else if(dbresult == DB_LOSS)
				{
				p->color ^= CC;
				eval = -dbwineval(p, mc);
				p->color ^= CC;
				return eval;	
				}
			}
		}
#endif //USEDB


#ifdef SPA
	// make a SPA lookup attempt
	if((bk+wk == 0) && (p->color==BLACK))
		{
		if(bm==5 && wm==5)
			{
			si.spalookups++;
			if(spa_lookup(Gkey,Glock,&eval,0))
				{
				si.spasuccess++;
				return eval;
				}
			}
		else if(bm==6 && wm==6)
			{
			si.spalookups++;
			if(spa_lookup(Gkey, Glock, &eval, 1))
				{
				si.spasuccess++;
				return eval;
				}
			}
		}
#endif

	eval = materialeval[mc->bm][mc->bk][mc->wm][mc->wk];
	if(p->color == WHITE) 
		eval = -eval;

	e.material = eval;

#ifdef EVALMATERIALONLY
	#ifdef COARSEGRAINING
		eval=(eval/GRAINSIZE)*GRAINSIZE;
	#endif
		return eval;
#endif

	// lazy eval: exit if materialeval is too far from window (400)
	if(( eval>alpha+1+LAZYEVALWINDOW) || (eval<alpha-LAZYEVALWINDOW) ) 
		{
#ifdef COARSEGRAINING
		eval=(eval/GRAINSIZE)*GRAINSIZE;
#endif
		return eval;
		}

	// get selftrapeval
	e.selftrap = selftrapeval(p, mc, &ki, delta);
	eval += e.selftrap;
	// TODO: selftrapeval does not evaluate "man-into-selftrap", i.e. black men on 22,25->29 vs wm 30. 
	
	// TODO: could add a second lazy exit here

	// get backrankeval
	e.backrank = (blackbackrankeval[p->bm & 0xFF] - whitebackrankeval[p->wm >> 24]); 
	if(p->color == WHITE)
		e.backrank = -e.backrank;
	eval += e.backrank;

	
	assert(ki.freebk >= 0);
	assert(ki.freewk >= 0);
	assert((ki.untrappedbk & (~p->bk)) == 0);
	assert((ki.untrappedwk & (~p->wk)) == 0);
	
	// SECOND LAZY EXIT (TODO: code lazy exit into function to not duplicate stuff)
	if(( eval>alpha+1+FINEEVALWINDOW) || (eval<alpha-FINEEVALWINDOW) ) // vtune: use |
		{
#ifdef COARSEGRAINING
		eval=(eval/GRAINSIZE)*GRAINSIZE;
#endif
		return eval;
		}

	// we could not make a lazy exit, so initialize the remainder of EVALUATION structure:

	// reset rest of eval structure:
	e.compensation = 0;
	e.cramp = 0;
	e.hold = 0;
	e.king = 0;
	e.king_man = 0;
	e.men = 0;
	e.runaway = 0;

	fe = fineevaluation(&e, p, mc, &ki, delta, &likelydraw);
	eval = eval + fe;

	// TODO: nonlinear evaluation with different eval parts
	assert(ki.freebk >= 0);
	assert(ki.freewk >= 0);
	
	// TODO: find better ways of computing likely draw!
	if(likelydraw)
		eval /= 2;
	
#ifdef COARSEGRAINING
	eval=(eval/GRAINSIZE)*GRAINSIZE;
#endif

	// TODO: think about making delta a function of our evaluation, e.g. *delta = abs(e.x + e.y +...) / Y
	//*delta += abs(e.backrank + e.compensation + e.cramp + e.hold + e.king + e.king_man + e.men + e.runaway) / 4;
	return eval;
	}

#ifdef USEDB
int dbwineval(POSITION *p, MATERIALCOUNT *mc)
	{
	/*------------------------------------------------------------------------------\ 
	|    dbwineval provides a heuristic value to a position which is a win for		|
	|	color according to the database												|
	|																				|
	|	the heuristic value is higher if...											|
	|	-> there are less pieces on the board (encourage exchanges)					|
	|	-> the winning side has more kings											|
	|	-> the winning side occupies the center with kings							|
	|	-> the winning side has a high tempo count for men (encourage crowning)		|
	\------------------------------------------------------------------------------*/

	int value = 1200;
	int32 m;
	// new in 1.14: modifiers to encourage the stronger side to promote and grab the center 

	// TODO: use powers of 2: 256 / 128 / 64 etc instead of 200/100 etc.
	if(p->color == BLACK) 
		{
		// black is to move and is winning
		value += 200*(mc->bm + mc->bk - mc->wm - mc->wk);  /* go for largest piece difference first */
		value -= 100*(mc->bm + mc->bk + mc->wm + mc->wk);   /* go for fewer pieces next */
		value += 40*mc->bk;	 /* go for kings */
		value += bitcount(p->bk&CENTER); /* go for center control of kings */
		value += bitcount(p->wk&EDGE);	 /* push opponent kings to edge */
		if(p->bm)						 /* promote men */
			{
			value += 2*bitcount(p->bm&0x00000FF0);
			value += 6*bitcount(p->bm&0x000FF000);
			value += 10*bitcount(p->bm&0x0FF00000);
			}

		if( (mc->bm + mc->bk == mc->wm + mc->wk) && p->wm)
			{
			// black is winning, make white men advance
			// make opponents men advance. this is si-intuitive, but it works - why?
			// e.g. in 1st position this is the winning plan. also, once the man is as far
			// as it can go, that will "narrow down" the winning line for the strong side.
			//value += 12*bitcount(p->wm&0x000000F0);
			value += 10*bitcount(p->wm&0x00000FF0);
			value += 6*bitcount(p->wm&0x000FF000);
			value += 2*bitcount(p->wm&0x0FF00000);
			}

		// now, a typical case in winning a won position: the strong side has one man left, and it's hard
		// to crown, because the opposite side has ganged up on that man and won't let it pass. so an important
		// intermediate goal here is to push the opponents kings away.
		if(mc->bm == 1 && (mc->bm + mc->bk)==(mc->wm + mc->wk+1))
			{
			m = p->bm | forward(p->bm) | forward(forward(p->bm));
			// reduce eval if there is a king in front of that man somehow
			m = m&p->wk;
			if(m)
				value -= 10*bitcount(m);
			// reduce eval further for supporting kings nearby
			// TODO: computations such as this might be faster with x = LSB(m); m = precomputed_vicinity_3[m].
			m = m | backward(m) | forward(m);
			m = m | backward(m) | forward(m);
			m = m | backward(m) | forward(m);
			value -= 3*bitcount(m&p->wk);
			}
		}
	else
		{
		// white is to move and is winning
		value += 200*(mc->wm + mc->wk - mc->bm - mc->bk);  /* go for largest piece difference first */
		value -= 100*(mc->bm + mc->bk + mc->wm + mc->wk);   /* go for fewer pieces next */
		value += 40*mc->wk;	 /* go for kings */
		value += bitcount(p->wk&CENTER); /* go for center control of kings */
		value += bitcount(p->bk&EDGE);	 /* push opponent kings to edge */
		if(p->wm)
			{
			value += 2*bitcount(p->wm&0x0FF00000);
			value += 6*bitcount(p->wm&0x000FF000);
			value += 10*bitcount(p->wm&0x00000FF0);
			}

		if(mc->bm + mc->bk == mc->wm + mc->wk && p->bm)
			{
			// white is winning, make black men advance.
			// make opponents men advance. this is si-intuitive, but it works - why?
			// e.g. in 1st position this is the winning plan. also, once the man is as far
			// as it can go, that will "narrow down" the winning line for the strong side.
			value += 2*bitcount(p->bm&0x00000FF0);
			value += 6*bitcount(p->bm&0x000FF000);
			value += 10*bitcount(p->bm&0x0FF00000);
			}

		// special case of one white man left over, harassed by black kings:
		if(mc->wm==1 && (mc->wm + mc->wk) == (mc->bm + mc->bk + 1))
			{
			m = p->wm | backward(p->wm) | backward(backward(p->wm));
			// reduce eval if there is a king in front of that man somehow
			m = m&p->bk;
			if(m)
				value -= 10*bitcount(m);
			// reduce eval further for supporting kings nearby
			m = m | backward(m) | forward(m);
			m = m | backward(m) | forward(m);
			m = m | backward(m) | forward(m);
			value -= 3*bitcount(m&p->bk);
			}
		}
	return value;
	}
#endif //USEDB


int selftrapeval(POSITION *p, MATERIALCOUNT *mc, KINGINFO *ki, int *delta)
	{
	/*------------------------------------------------------------------------------\ 
	| selftrapeval looks for a few common patterns of ridiculous selftrapping		|
	| if detected, this gives huge penalties. it is separated from the normal		|
	| evaluation to allow a lazy eval after material+selftrapeval, before the rest	|
	| of the positional evaluation.													|
	\------------------------------------------------------------------------------*/
	int eval = 0;
	int32 free; 

	// initialize kinginfo structure
	ki->freebk = mc->bk;
	ki->freewk = mc->wk;
	
	ki->untrappedbk = p->bk;
	ki->untrappedwk = p->wk;

	if(!( (p->bk & (WBR|SQ28|SQ25)) | (p->wk & (BBR|SQ5|SQ8))))
		return 0;

	free = ~(p->bm|p->bk|p->wm|p->wk);

	// selftrapeval looks for kings on the back rank as well as on square 5 for white, SQ28 for black

	// absurd self trapping of white king under bridge:
	if(match3(p->bm, p->wm, p->wk, (SQ1|SQ3), (SQ6|SQ7|SQ10), SQ2))
		{
		eval += 230;
		ki->freewk--;
		ki->untrappedwk ^= SQ2;
		}
	// absurd self trapping of white king under "other" bridge
	if(match3(p->bm, p->wm, p->wk, (SQ2|SQ4), (SQ7|SQ8|SQ11), SQ3))
		{
		eval += 230;
		ki->freewk--;
		ki->untrappedwk ^= SQ3;
		}

	// absurd self trapping of black king under bridge
	if(match3(p->wm, p->bm, p->bk, (SQ30|SQ32), (SQ23|SQ26|SQ27), SQ31))
		{
		eval -= 230;
		ki->freebk--;
		ki->untrappedbk ^= SQ31;
		}

	// absurd self trapping of black king under "other" bridge
	if(match3(p->wm, p->bm, p->bk, (SQ29|SQ31), (SQ22|SQ25|SQ26), SQ30))
		{
		eval -= 230;
		ki->freebk--;
		ki->untrappedbk ^= SQ30;
		}

	// 'absurd' self-trapping in single corner for black
	if(p->bk & SQ29)
		{
		if(p->wm & SQ30)
			{
			if(match1(p->bm, SQ21|SQ25))
				{
				eval -= 200;
				ki->untrappedbk ^= SQ29;
				ki->freebk--;
				}
			else if(p->bm & SQ22)
				{
				eval -= 100;
				ki->untrappedbk ^= SQ29;
				ki->freebk--;
				}
			}
		}
	if(p->bk & SQ25)
		{
		if(p->wm & SQ30)
			{
			if(p->bm & SQ22)
				{
				eval -= 100;
				ki->untrappedbk ^= SQ25;
				ki->freebk--;
				}
			}
		}

	// 'absurd' self-trapping in single corner for white
	if(p->wk & SQ4)
		{
		if(p->bm & SQ3)
			{
			if(match1(p->wm, SQ8|SQ12))
				{
				eval += 200;
				ki->untrappedwk ^= SQ4;
				ki->freewk--;
				}
			else if(p->wm & SQ11)
				{
				eval += 100;
				ki->untrappedwk ^= SQ4;
				ki->freewk--;
				}
			}
		}
	if(p->wk & SQ8)
		{
		if(p->bm & SQ3)
			{
			if(p->wm & SQ11)
				{
				eval += 100;
				ki->untrappedwk ^= SQ8;
				ki->freewk--;
				}
			}
		}

//       WHITE
//      28  29  30  31
//	  24  25  26  27
//	   20  21  22  23
//	 16  17  18  19
//	   12  13  14  15
//	  8   9  10  11
//	    4   5   6   7
//	  0   1   2   3
//	      BLACK

	// self trapping in double corner for black kings
	if(p->wm&SQ31)
		{
		// most awful case:
		if(match2((p->bm|p->bk), (p->bm),(SQ32|SQ28|SQ24|SQ27),(SQ24|SQ27)))
			{
			eval-=300;
			ki->freebk -= bitcount(p->bk & (SQ32|SQ28));
			ki->untrappedbk ^= (p->bk & (SQ32|SQ28));
			}
		// awful case 
		else if( (p->bk&SQ32) && (p->bm&SQ28) )
			{
			if(p->wk&SQ23)
				{
				eval -= 100;
				ki->freebk--;
				ki->untrappedbk^=SQ32;
				}
			else if( (free&SQ24) && (!(p->bm&SQ20))) 
				{
				eval -= 30;
				ki->freebk--;
				ki->untrappedbk^=SQ32;
				}
			}
		}

	// self trapping in double corner for white kings
	if(p->bm&SQ2)
		{
		// most awful case:
		if(match2((p->wm|p->wk), (p->wm),(SQ1|SQ5|SQ6|SQ9),(SQ6|SQ9)))
			{
			eval += 300;
			ki->freewk -= bitcount(p->wk & (SQ1|SQ5));
			ki->untrappedwk ^= (p->wk & (SQ1|SQ5));
			}
		// awful case 
		else if( (p->wk&SQ1) && (p->wm&SQ5) )
			{
			if(p->bk&SQ10)
				{
				eval += 100;
				ki->freewk--;
				ki->untrappedwk^=SQ1;
				}
			else if( (free&SQ9) && (!(p->wm&SQ13))) 
				{
				eval += 30;
				ki->freewk--;
				ki->untrappedwk^=SQ1;
				}
			}
		}
	return (p->color==BLACK) ? eval:-eval;
	}
	

int fineevaluation(EVALUATION *e, POSITION *p, MATERIALCOUNT *mc, KINGINFO *ki, int *delta, int *likelydraw)
	{
	/*------------------------------------------------------------------------------\ 
	|	fineevaluation is the positional evaluation of cake							|
   	|	it knows a lot of features, most of the names are self-explanatory.			|
    |	it returns the positional evaluation of the position, positive				|
	|	is good for black, negative good for white, a man is worth 100.				|
   \------------------------------------------------------------------------------*/

	// TODO: ideas for improving the evaluation:
	//	-> mobility evaluation
	//	-> use only grounded men to evaluate center control
	//	-> recognize runaways under "other" bridge
	//	-> simplify/improve runaway eval in general
	//  -> double doghole penalty
	//	-> general nonlinear defect penalty: count certain things as defects, and
	//		take difference of defects squared or so for eval. ideas for defects
	//		are: weaker back rank, higher tempo, doghole, missing oreo, not having only king
	//			 having many ungrounded men, bad structures/cramps, low mobility, trapped kings
	//			 tailhooks.

	//  -> general "men-running-into-strong-double-corner" eval
	//  -> improve developed single corner: right now, there is a penalty if there are men on either 4 or 8 or both
	//	   change to bigger penalty for both, smaller penalty for one.
	// give backrankpower for only oreo and for oreo+man-on-five
	// -> watching an engine match game, i see that bridge stones (1/3) holding
	//    2 dogholes and bridge man (5,10,12) is deadly (worth about a man).
	
	int32 free,m;
	int stones = mc->bm + mc->wm;
	int kings = mc->bk + mc->wk;
	int pieces = stones+ kings;
	int eval=0;
	int ending=0;
	int equal=0;
	int tempo=0;
	int index;
	int balance = 0;
	int mcenter;
	int potbk = 0, potwk = 0;
	int blacktailhooks = 0, whitetailhooks = 0; 
	int kingproximity = 0;
	int32 tmp,m1,free2,attack;
	//int endangeredbridgeval=6;
	int turn = 0; 
	int32 white=p->wm|p->wk;
	int32 black=p->bk|p->bm;
	int realdoghole = 0;
	int wr1,wr2,wr3,wr4,wr6,wr7,wr8;
	int br1,br2,br3,br4,br6,br7,br8;
	int32 ungrounded_black = 0, ungrounded_white = 0;
	int32 immobile_black = 0, immobile_white = 0;
	int badstructureval; // = 5;// 2; //  5;  // optimized 2;
	//int whitehasbridge = 0, blackhasbridge = 0; 
	//int badstructureval;

	/*int32 grounded_black_leftforward, grounded_black_rightforward;
	int32 grounded_white_leftbackward, grounded_white_rightbackward;
	int32 contacts;
	int contacteval = 0;*/
//	int32 ungrounded_immobile_black, ungrounded_immobile_white;
   

	// white, black, free, stones, kings and tempo get a value assigned and keep it 
	// throughout this evaluation function and can always be used.
	
	
	// DEO says about temp: once there is the same number of men on the board, as free squares, this goes
	// from positive to negative - that's where i go to -1, so maybe change that...
	// he also says that a tempo count of 5 should be FATAL in the opening, so maybe change that too.
	// with 21...24 stones big penalty for tempo
	// with 17...20 stones normal penalty for tempo
	// with 13...16 stones no penalty
	// with 9....12 stones bonus
	// here's an example with 14 stones where there should be a penalty:
	// W:W32,31,30,29,28,26,24,21:B16,14,13,12,4,3,2,1.
	// in general: you want to give a penalty for tempo count if the opponent has a strong
	// back rank, right? because only then it will be a problem that you run into him...
	
	static int tmod[25] = { 0,5 ,2, 2, 2, 1, 1, 1, 0, 0, 0, -1, -1,-2,-2,-3,-3,-4,-4, -5, -5, -6, -6, -7, -7 };  // my adapted version
	//static int tmod[25]={0,2,2,2,2,1,1,1,1,0,0,0,0,-1,-1,-1,-1,-2,-2,-2,-2,-3,-3,-3,-3};   // original  
	//static int tmod[25] = { 0,5 ,2, 2, 2, 1, 1, 1, 0, 0, 0, -1, -1,-2,-2,-3,-3,-4,-4, -5, -5, -6, -6, -7, -7 }; // - optimized?

	//static int ungroundedpenalty[13] = {0,   0,0,2, 5, 8,11,14,17,20,24,28,32};


	/* 
	 * fineevaluation does the following:
	 * 1) some initializations
	 * 2) evaluation for men: grounded/ungrounded, back rank, tempo
	 * 3) evaluation for men: small cramps and bad structures
	 * 4) evaluation for men: balance, skewness, doghole
	 * 5) evaluation for men: man center control by occupation and attack, dyke
	 * 6) runaway evaluation, which sets potwk and potbk, the numbers of potential kings coming up
	 * 7) generalized cramp evaluation
	 * 8) king eval: in center or on edge
	 * 9) king eval: trapped kings
	 * 10)king eval: dominated kings
	 * 11)king-man-interaction: tailhooks and kings-close-to-ungrounded men
	 * 12)king eval: center monopoly
	 * 13)king-man-interaction: endangered bridgeheads
	 * 14)king-man-interaction: king immobilizing men on edge
	 * 15)king eval: generalized king mobility
	 * 16)king eval: single king bonus
	 * 17)man-sac-compensation eval: bonus for good back rank and potential king
	 * 18)general eval: turn
	 * 19)likely draw detection
	 */

	/* big TODO: clean up the above stuff, group eval in lumps, namely:
	 * 1a) man evaluation
	 * 1b) back rank evaluation
	 * 2) cramp evaluation
	 * 3) runaway evaluation
	 * 4) king evaluation
	 * 5) king-man-interaction eval
	 * 6) compensation evaluation
	 * 
	 * the idea is to use this for a nonlinear eval: find out how many items favor side with higher
	 * eval, and give it a bonus which is, say, proportional to the square of that number. 
	 */

	
   /************************ positional *************************/
   /* initialize free board, and game status */
	free=~(black|white);  // = ~(white|black) should be a tiny bit faster
	if(mc->bm + mc->bk == mc->wm + mc->wk) 
		equal=1;
	if(pieces<=10) 
		ending=1;

	
   /* organize: things to do only if men, things only if kings, */
   /* things to do only if men are on the board */

	if(stones)
		{
		//
		// 2. man evaluation
		//
		
		// backward men: count all men which have no support from the edge, and are not yet 
		// well-advanced (first 5 rows)
		// this improved 2s laptop match result vs kr 113i from +39-31 to +50-31
		m = p->bm&EDGE;
		m |= (forward(m)&p->bm);
		m |= (forward(m)&p->bm);
		m |= (forward(m)&p->bm);
		m |= (forward(m)&p->bm);
		// m now contains all "grounded men"
		// we don't want to penalize ungrounded men that are runaways:
		ungrounded_black = (~m)&p->bm & 0x00FFFFFF;
		
		// TODO: maybe add penalty for two ungrounded men - in particular if
		// they have "contact"
		// TODO: set number of ungrounded men in relation to total men?
		//static int ungroundedpenalty[13] = {0,0,0,2,5,8,11,14,17,20,24,28,32};
		// TODO: center control can only be executed by grounded men?
		//ungrounded_black = bitcount(ungrounded_black);
		
		
		/* x12 had this disabled*/
		if(p->wk & ROAMINGWHITEKING)
			e->men -= 2*ungroundedpenalty[bitcount(ungrounded_black)];
		else
			e->men -= ungroundedpenalty[bitcount(ungrounded_black)];
		
		// 	static int ungroundedpenalty[13] = {0,0,0,2,5,8,11,14,17,20,24,28,32};
		
		// get ungrounded white men
		m = p->wm&EDGE;
		m |= (backward(m)&p->wm);
		m |= (backward(m)&p->wm); 
		m |= (backward(m)&p->wm);
		m |= (backward(m)&p->wm);
		ungrounded_white = (~m)&p->wm&0xFFFFFF00;

		
		if(p->bk & ROAMINGBLACKKING)
			e->men += 2*ungroundedpenalty[bitcount(ungrounded_white)];
		else
			e->men += ungroundedpenalty[bitcount(ungrounded_white)];
		

		// generalized immobile ungrounded  men eval
		/*if(ungrounded_black & immobile_black)
		{
			printboard(p);
			printint32(ungrounded_black & immobile_black);
			getch();
		}*/

		// include in x13
		// immobile man evaluation
		
		//immobile_black = p->bm & leftbackward(p->bm) & rightbackward(p->bm);
		//immobile_white = p->wm & leftforward(p->wm) & rightforward(p->wm);

		/*
		e->men += 2*bitcount(ungrounded_white & immobile_white);
		e->men -= 2*bitcount(ungrounded_black & immobile_black);
*/
		
		//immobile_black = p->bm & leftbackward(p->bm) & rightbackward(p->bm);
		//immobile_white = p->wm & leftforward(p->wm) & rightforward(p->wm);

		//immobile_black = p->bm & ~(backward(free | white));
		//immobile_white = p->wm & ~(forward(free | black));
		
		// TODOTODO: I think below would be a more sensible 
		// definition of immobiles!
		immobile_black = ungrounded_black & ~(backward(free | white));
		immobile_white = ungrounded_white & ~(forward(free | black));

		// new immobile definition 22.4.2019 1.85g without free was a bit worse:
		//immobile_black = p->bm & ~(backward(free));
		//immobile_white = p->wm & ~(forward(free));

		/*if (immobile_black) {
			printboard(p);
			printint32(immobile_black);
			getch();
		}*/

		// TODO: more often than not these immobile men are on the back rank guarding the back rank!
		e->men += v[immobile_mult] * bitcount(immobile_white);
		e->men -= v[immobile_mult] * bitcount(immobile_black);

		// try same with ungrounded  (// new in Cake 1.85g2) didn't work
		//e->men += bitcount(immobile_white & ungrounded_white);
		//e->men -= bitcount(immobile_black & ungrounded_black); 
		
		/*tmp = (immobile_black & ungrounded_black); 
		if (bitcount(tmp) > 3) {
			printboard(p);
			printint32(tmp);
			/*printf("immobile\n"); 
			printint32(immobile_black); 
			printf("ungrounded\n"); 
			printint32(ungrounded_black); 
			getch(); 
		}*/


		// find immobile ungrounded men: that means that they have no safe
		// squares to go to. safe squares are free but not under attack by
		// the opposing forces.


		// generalized cramp eval - find contact points and
		// check whether the contacting men are grounded or not.
		
		// find all ungrounded black men that are in contact with white men
		/*m = ungrounded_black & (rightbackward(p->wm) | leftbackward(p->wm));
		m1 = ungrounded_white & (leftforward(p->bm) | rightforward(p->bm));
		e->men += v[ungroundedcontact] * (bitcount(m1) - bitcount(m)); */
		
		
		/*if (m1 && !m) {
			printint32(m1); 
			printboard(p);
			getch();
		}*/
		
		/*m = p->bm & rightbackward(p->wm);
		m |= leftforward(m);

		e->men += 2*(bitcount(ungrounded_white & m) - bitcount(ungrounded_black & m));
		*/
		/*if(ungrounded_black & m)
		{
			printboard(p);
			printint32(ungrounded_black & m);
			getch();
		}*/
/*
		m = p->bm & leftbackward(p->wm);
		m |= rightforward(m);

		e->men += 2*(bitcount(ungrounded_white & m) - bitcount(ungrounded_black & m));
*/
		/*if(contacts)
			{
			
			grounded_black_leftforward = p->bm & (RANK1|ROW8);
			grounded_black_leftforward |= (leftforward(grounded_black_leftforward)&p->bm);
			grounded_black_leftforward |= (leftforward(grounded_black_leftforward)&p->bm);
			grounded_black_leftforward |= (leftforward(grounded_black_leftforward)&p->bm);
			grounded_black_leftforward |= (leftforward(grounded_black_leftforward)&p->bm);
			grounded_black_leftforward |= (leftforward(grounded_black_leftforward)&p->bm);

			grounded_white_rightbackward = p->wm & (RANK8|ROW1);
			grounded_white_rightbackward |= (rightbackward(grounded_white_rightbackward)&p->wm);
			grounded_white_rightbackward |= (rightbackward(grounded_white_rightbackward)&p->wm);
			grounded_white_rightbackward |= (rightbackward(grounded_white_rightbackward)&p->wm);
			grounded_white_rightbackward |= (rightbackward(grounded_white_rightbackward)&p->wm);
			grounded_white_rightbackward |= (rightbackward(grounded_white_rightbackward)&p->wm);
			
			
			contacteval += bitcount(contacts&grounded_black_leftforward);
			contacteval -= bitcount(leftforward(contacts)&grounded_white_rightbackward);
			}

		contacts = p->bm & leftbackward(p->wm);
		if(contacts)
			{
			grounded_black_rightforward = p->bm & RANK1|ROW1;
			grounded_black_rightforward |= (rightforward(grounded_black_rightforward)&p->bm);
			grounded_black_rightforward |= (rightforward(grounded_black_rightforward)&p->bm);
			grounded_black_rightforward |= (rightforward(grounded_black_rightforward)&p->bm);
			grounded_black_rightforward |= (rightforward(grounded_black_rightforward)&p->bm);
			grounded_black_rightforward |= (rightforward(grounded_black_rightforward)&p->bm);

			grounded_white_leftbackward = p->wm & (RANK8|ROW8);
			grounded_white_leftbackward |= (leftbackward(grounded_white_leftbackward)&p->wm);
			grounded_white_leftbackward |= (leftbackward(grounded_white_leftbackward)&p->wm);
			grounded_white_leftbackward |= (leftbackward(grounded_white_leftbackward)&p->wm);
			grounded_white_leftbackward |= (leftbackward(grounded_white_leftbackward)&p->wm);
			grounded_white_leftbackward |= (leftbackward(grounded_white_leftbackward)&p->wm);
			
			contacteval += bitcount(contacts&grounded_black_rightforward);
			contacteval -= bitcount(rightforward(contacts)&grounded_white_leftbackward);
			}

		
		eval += contacteval;*/

		
		/* tempo */
		// tempo is positive if black is more advanced than white
		tempo+=  bitcount(p->bm&0x000000F0);
		tempo+=2*bitcount(p->bm&0x00000F00);
		tempo+=3*bitcount(p->bm&0x0000F000);
		tempo+=4*bitcount(p->bm&0x000F0000);
		tempo+=5*bitcount(p->bm&0x00F00000);
		tempo+=6*bitcount(p->bm&0x0F000000);
		
		tempo-=6*bitcount(p->wm&0x000000F0);
		tempo-=5*bitcount(p->wm&0x00000F00);
		tempo-=4*bitcount(p->wm&0x0000F000);
		tempo-=3*bitcount(p->wm&0x000F0000);
		tempo-=2*bitcount(p->wm&0x00F00000);
		tempo-=  bitcount(p->wm&0x0F000000);
		
		
		//tempo*=tmod[stones];
		/* wait with tempo evaluation until doghole is done! */

		// TODO: expensive part of eval - don't do it if equal == 0 (one side is up a king or more)).
		if(mc->bm == mc->wm)
		//if(equal == 1) was worse in engine match
			{
			br1 = bitcount(p->bm&ROW1);
			br2 = bitcount(p->bm&ROW2);
			br3 = bitcount(p->bm&ROW3);
			br4 = bitcount(p->bm&(ROW4|ROW5));
			br6 = bitcount(p->bm&ROW6);
			br7 = bitcount(p->bm&ROW7);
			br8 = bitcount(p->bm&ROW8);

			wr1 = bitcount(p->wm&ROW1);
			wr2 = bitcount(p->wm&ROW2);
			wr3 = bitcount(p->wm&ROW3);
			wr4 = bitcount(p->wm&(ROW4|ROW5));
			wr6 = bitcount(p->wm&ROW6);
			wr7 = bitcount(p->wm&ROW7);
			wr8 = bitcount(p->wm&ROW8);
			
			index = -3*br1-2*br2-br3+br6+2*br7+3*br8;
			balance = -abs(index);
			index = -3*wr1-2*wr2-wr3+wr6+2*wr7+3*wr8;
			balance += abs(index);
			
			// standard: balance and index
			// try increasing

			e->men += (v[balancemult] * balance)/4;


			// skewness: is a large number for skewed positions i.e. for lots of men toward edge.
			
			index = -((br1+br2-br3-br4-br6+br7+br8));
			index += ((wr1+wr2-wr3-wr4-wr6+wr7+wr8));
			e->men += (v[skewnessmult]*index)/4;
			
			// adjust skewness for number of men - with few men on the board,
			// it doesn't matter any more
			//index = index * (mc->bm+mc->wm);
			//index /= 20;
			}
		
		// doghole 
		// TODO: test whether writing something like p->bm & SQ3 & backleft(p->wm) is faster
		// TODO: double doghole code
		// TODO: dogholemandownval 20 points is a bit big!

		
		if( (p->wm&SQ12) && (p->bm&SQ3)) // vtune use &
			{
			// good for black 
			// count away the tempo effect:
			// only if it's good for black! we don't want white to go in the doghole just
			//to save tempo
			
			if(tmod[stones]>0) 
				tempo+=5;
			
			// and give a penalty - for sure here
			e->men += v[dogholeval2];
			}
		if( (p->wm&SQ5) && (p->bm&SQ1) ) //real doghole
			{
			// set white doghole bit so we know
			realdoghole &= WHITE;
			// count away tempo 
			
			if(tmod[stones]>0) 
				tempo+=6;
			
			// give a penalty or not? - i don't know... 
			if(ending==0) 
				e->men += v[dogholeval];
			// give a real penalty if there is a white man on bit11 or bit 15,
			//	because this man cannot go through!
			if( (p->wm & (SQ9|SQ13) ) && (free&SQ10)) 
				e->men += v[dogholemandownval];
			}
		if( (p->bm&SQ28) && (p->wm&SQ32) ) // real doghole
			{
			realdoghole &= BLACK;
			
			if(tmod[stones]>0) 
				tempo-=6;
			
			if(ending == 0)
				e->men -= v[dogholeval];
			if( (p->bm & (SQ24|SQ20) ) && (free&SQ23)) 
				e->men -= v[dogholemandownval];
			}
		if( (p->bm&SQ21) && (p->wm&SQ30) ) // vtune: use &
			{
			
			if(tmod[stones]>0) 
				tempo-=5;
			
			e->men -= v[dogholeval2];
			}
		
		
		/* now that doghole has been evaluated, we can do the tempo eval too */
		//static int tmod[25]={0,2,2,2,2,1,1,1,1,0,0,0,0,-1,-1,-1,-1,-2,-2,-2,-2,-3,-3,-3,-3}; 
		
		e->men += tempo*tmod[stones];
		
		/* man center control */
		// both with both MCENTER is best result up to now. less good: only occupation with either MCENTER or CENTER
		// TODO: try using occupation only by grounded men.
		
		/* by occupation */
		mcenter  = v[mc_occupyval]*bitcount(p->bm&MCENTER&~ungrounded_black);  
		mcenter -= v[mc_occupyval]*bitcount(p->wm&MCENTER&~ungrounded_white);
		e->men += mcenter;

		/* by attack */
		mcenter  = v[mc_attackval]*bitcount(attack_forwardjump(p->bm,free)&MCENTER);
		mcenter -= v[mc_attackval]*bitcount(attack_backwardjump(p->wm,free)&MCENTER);
		e->men += mcenter; 

		
		// TODO: with kings on the board, it's probably not so good to have men in the center!

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

		
		if((p->bm & SQ19) && (p->wm & SQ28)) // vtune: use &
			{
			e->men += v[realdykeval];
			if(match1(~p->wm, SQ27|SQ31|SQ32))
				e->men += v[greatdykeval];
			}
		if((p->wm&SQ14) && (p->bm&SQ5 )) // vtune: use &
			{
			e->men -= v[realdykeval];
			if(match1(~p->bm, SQ1|SQ2|SQ6))
				e->men -= v[greatdykeval];
			}
		
		//
		// done with man evaluation
		//

		eval += e->men;
		//
			// now for cramps and bad structures:
			// 

			/* cramping squares */
			/* all the 'cramping nothing statements: found with PDNtool */

			//params[38] = 0; // cramp12
			//params[39] = 0; // cramp13
			//params[40] = 0; // cramp20
			// cramp20 = params[40]
		if (p->bm & SQ20)
		{
			if (p->wm & SQ24)
				e->cramp += v[cramp20];
			if (free & SQ24)
				e->cramp -= v[nocrampval]; //cramping nothing - discourage a little
		}

		if (p->wm & SQ13)
		{
			if (p->bm & SQ9)
				e->cramp -= v[cramp20];
			if (free & SQ9)
				e->cramp += v[nocrampval]; // cramping nothing - discourage a little
		}

		// cramp13 = params[39]
		if (p->bm & SQ13)
		{
			if (p->wm & SQ17)
				e->cramp += v[cramp13];
			//if ((p->wm&(SQ17 | SQ21/*|SQ22*/)) == (SQ17 | SQ21/*|SQ22*/))
			//	e->cramp += crampval;
			if ((free & (SQ17 | SQ21)) == (SQ17 | SQ21))
				e->cramp -= v[nocrampval]; // cramping nothing - discourage a little
		}
		if (p->wm & SQ20)
		{
			//if ((p->bm&(/*SQ11|*/SQ12 | SQ16)) == (/*SQ11|*/SQ12 | SQ16))
			//	e->cramp -= crampval;
			if (p->bm & SQ16)
				e->cramp -= v[cramp13];
			if ((free & (SQ12 | SQ16)) == (SQ12 | SQ16))
				e->cramp += v[nocrampval]; // cramping nothing - discourage a little
		}

		//params[38] = 0; // cramp12
		if (p->bm & SQ12) {
			if (p->wm & SQ16)
				e->cramp += v[cramp12];
		}
		if (p->wm & SQ21) {
			if (p->bm & SQ17)
				e->cramp -= v[cramp12];
		}

		//
		//  end of cramps
		//

		eval += e->cramp;

		// 
		// holds
		//

		// bad structure that often shows up:
 
		if(match3(p->bm, p->wm, free, (SQ19|SQ23), (SQ28|SQ31), (SQ24|SQ27|SQ32)))
			{
			e->hold -= v[badstructure3];  // was 6
			if(p->wk)
				e->hold -= v[badstructure4];  // was 10
			}
		if(match3(p->bm, p->wm, free, (SQ2|SQ5), (SQ10|SQ14), (SQ1|SQ6|SQ9)))
			{
			e->hold += v[badstructure3];  // was 6
			if(p->bk)
				e->hold += v[badstructure4];  // was 10
			}

		/* some bad structures */
		/* found with PDNtool */
		/*strange - the single corner side structure for white (0x80808000) is not bad?!! - pdntool */

		if (match1(p->bm, SQ4 | SQ12 | SQ20))
			e->hold -= v[badstructure]; // (20 - stones); // v[badstructure];
		if (match1(p->bm, SQ1 | SQ5 | SQ13))
			e->hold -= v[badstructure2]; // (20 - stones); // v[badstructure];

		if (match1(p->wm, SQ13 | SQ21 | SQ29)) // new 1.376
			e->hold += v[badstructure]; // (20 - stones); // v[badstructure];
		if (match1(p->wm, SQ32 | SQ28 | SQ20))
			e->hold += v[badstructure2]; // (20 - stones); //v[badstructure];
		

		
		// new 1.41 - from DEO with PDNtool
		// describes holds of 2-3 or 3-4 men. this increases in importance, the less men, the worse.
		// only do this if stones <= N.
		// without this: +40-41 vs kr 1.12l, with this +48-39!
		//if(stones<=14) // 14: +:37 =:241 -:10
		//if(stones<=16) // 16: +:33 =:238 -:17
		//if(stones<=12) // 12: +:45 =:228 -:15
		badstructureval = v[badstructuremax1]-stones;
		if(badstructureval < v[badstructuremin]) 
			badstructureval = v[badstructuremin];
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
			e->hold += badstructureval;
		if(match3(p->bm, p->wm, free, (SQ12|SQ16|SQ20), (SQ19|SQ23|SQ24|SQ27), (SQ4|SQ8|SQ11))) 
			e->hold += badstructureval;
		if(match3(p->bm, p->wm, free,(SQ11|SQ15|SQ16|SQ19), (SQ20|SQ24|SQ28), (SQ27|SQ32)))
			e->hold -= badstructureval;
		if(match3(p->bm, p->wm, free, (SQ6|SQ9|SQ10|SQ14), (SQ13|SQ17|SQ21), (SQ22|SQ25|SQ29))) 
			e->hold -= badstructureval;
		// add penalty anyway, whether it's free on that diagonal or not - allow one man there
		if(match2(p->bm, p->wm, (SQ5|SQ9|SQ13), (SQ14|SQ17|SQ18|SQ22)) && (bitcount(p->bm&(SQ1|SQ6))<2))
			e->hold += badstructureval;
		if(match2(p->bm, p->wm, (SQ12|SQ16|SQ20), (SQ19|SQ23|SQ24|SQ27)) && (bitcount(p->bm&(SQ4|SQ8|SQ11))<2)) // this one was wrong!
			e->hold += badstructureval;
		if(match2(p->bm, p->wm,(SQ11|SQ15|SQ16|SQ19), (SQ20|SQ24|SQ28)) && (bitcount(p->wm&(SQ27|SQ32))<2))
			e->hold -= badstructureval;
		if(match2(p->bm, p->wm, (SQ6|SQ9|SQ10|SQ14), (SQ13|SQ17|SQ21)) && (bitcount(p->wm&(SQ22|SQ25|SQ29))<2)) // this one too!
			e->hold -= badstructureval;
		// new cake manchester 1.10d: nascent classic 3-4-cramp - removed again, was no good in match
		/*if(match3(p->bm, p->wm, free, (SQ5|SQ6|SQ10|SQ14), (SQ13|SQ17|SQ21|SQ23), (SQ1|SQ2|SQ9|SQ18)))
			eval -= badstructureval;
		if(match3(p->bm, p->wm, free, (SQ10|SQ12|SQ16|SQ20), (SQ19|SQ23|SQ27|SQ28), (SQ15|SQ24|SQ31|SQ32)))
			eval += badstructureval;*/
		//if(stones<=14) 
		//if (stones <= 12)
		if (stones <= (2*v[badstructure2stones]))
			{
			// increases in importance with less men.
			// todo: maybe make more sophisticated by including knowledge about kings here - 
			// if the side which has a hold on the other side also has a free king, it's even
			// worse - because that means he will never have to give up the hold
			// if the side has more free kings than the defending side, it's worse again,
			// because it means that the hold can be changed maybe.
			badstructureval = v[badstructuremax2]-stones;

			// strong double corner against men on side - terrible statistics!!
			// TODO: we don't want to limit this to situations with less than 12 men. we could
			// use something like if match1(p->bm, SQ1|SQ2) eval += bitcount(p->wm & region where men shouldn't be)
			// and not only use bitcount but rather make the eval nonlinear.
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
				e->hold += badstructureval;
			if( (bitcount(p->bm&(SQ12|SQ16|SQ19|SQ20|SQ24|SQ28))>2) && (match1(p->wm,SQ31|SQ32)) )
				e->hold -= badstructureval;

			
			
			
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
				e->hold += badstructureval;
			if( (bitcount(p->bm&(SQ20|SQ24|SQ28))==2) && (match1(p->wm,(SQ31|SQ32)) )) // vtune: use & //vtuned &&->&!
				e->hold -= badstructureval;

			
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
				e->hold -= badstructureval;
			if(match2(p->bm, p->wm, (SQ7|SQ12), (SQ16|SQ19|SQ20)))
				e->hold += badstructureval;
			} 

		// 
		// end of holds
		//

		eval += e->hold;
 	
		// 
		// runaway evaluation
		//
		// TODO: i notice that man-on-22 supported by man on 21 vs lone white guard on
		//       30 is not classified as runaway. why?
		// TODO: stuff runaway eval in a separate function
	
		// runaways in one
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
			
		// TODO: simplify runaway-in-one:
		// tmp = p->bm & backward(free&WBR); (gives me all men that are potential runaways in one)
		// for bits 24...26 just give the bonus. 
		// for bit 27 do some checks.
		if( (p->bm) & RANK7)
			{
			if((p->bm)&(free>>4) & SQ28) 
				{e->runaway+=v[promoteinone];potbk++;e->runaway-=6*tmod[stones];}
			if((p->bm)&( (free>>3)|(free>>4) ) & SQ27) 
				{e->runaway+=v[promoteinone];potbk++;e->runaway-=6*tmod[stones];}
			if((p->bm)&( (free>>3)|(free>>4) ) & SQ26) 
				{e->runaway+=v[promoteinone];potbk++;e->runaway-=6*tmod[stones];}
			/* should modify this for men on 27 which will be trapped in single corner!*/
			if(p->bm & SQ25)
				{
				// man running into single corner - there's a danger of being trapped
				if(!(p->wm & SQ30))
					// no white man on 30 - no trapping
					{e->runaway+=v[promoteinone];potbk++;e->runaway-=6*tmod[stones];}
				else
					{
					
					// white man on 30, real danger of trapping. 
					if(free & SQ29)
						
						// else not a runaway in any case
						{
						//printboard(p);
						//printf("\nabove position: potential runaway");
						//getch();
						// if man is assisted and not disturbed, it's a runaway
						if(p->bm & SQ21)
							{
							// check whether white will stop man from moving out 
							// or dominate it with a king
							//if( (!(p->wm & (SQ26|SQ31))) && (!(p->wk&(0xECECE000))) )
							if( (!(p->wm & (SQ26))) && (!(p->wk&(0xECECE000))) )
								// nobody there to stop the man
								{
								if(!(p->wm & SQ31))
									{
									e->runaway+=v[promoteinone];
									potbk++;
									e->runaway-=6*tmod[stones];
									}
								else
									{
									// we don't have a runaway because white can move his man from
									// 31 to 26, but that will cost him his back rank
									//eval += 8;
									eval += v[runaway_destroys_backrank];
									}
								}
							}
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

		if( (p->wm) & RANK2)
			{
			if((p->wm)&(free<<4)& SQ5)						
				{e->runaway-=v[promoteinone];potwk++;e->runaway+=6*tmod[stones];}
			if((p->wm)&( (free<<3)|(free<<4) ) & SQ6) 
				{e->runaway-=v[promoteinone];potwk++;e->runaway+=6*tmod[stones];}
			if((p->wm)&( (free<<3)|(free<<4) ) & SQ7) 
				{e->runaway-=v[promoteinone];potwk++;e->runaway+=6*tmod[stones];}
			
			if(p->wm & SQ8)
				{
				// man running into single corner - there's a danger of being trapped
				if(!(p->bm & SQ3))
					// no black man on 3 - no trapping
					{e->runaway-=v[promoteinone];potwk++;e->runaway+=6*tmod[stones];}
				else
					{
					// white man on 30, real danger of trapping. 
					if(free & SQ4)
						// else not a runaway in any case
						{
						// if man is assisted and not disturbed, it's a runaway
						if(p->wm & SQ12)
							{
							// check whether white will stop man from moving out 
							// or dominate it with a king
							//if( (!(p->bm & (SQ2|SQ7))) && (!(p->bk&(0x00073737))) )
							if( (!(p->bm & (SQ7))) && (!(p->bk&(0x00073737))) )
								// nobody there to stop the man
								{
								if(!(p->bm&SQ2))
									{
									e->runaway-=v[promoteinone];
									potwk++;
									e->runaway+=6*tmod[stones];
									}
								else
									{
									// black has a man on two which can stop the runaway,
									// but it will cost him his back rank.
									//eval -= 8;
									eval -= v[runaway_destroys_backrank];
									}
								//printboard(p);
								//getch();
								}
							}
						}
					}
				}
			}

		//--------------
		//  in two     /
		//--------------

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
		

		if( (p->bm) & RANK6)
			{
			if( (p->bm & BIT20) && !(RA20 & white) ) 
				{
				e->runaway+=v[promoteintwo];
				potbk++;
				e->runaway-=5*tmod[stones];
				} // vtune: use &, in all others from here
			if( (p->bm&BIT21) && (!(RA21L&white) || !(RA21R&white)) ) 
				{
				e->runaway+=v[promoteintwo];
				potbk++;
				e->runaway-=5*tmod[stones];
				}
			if(p->color==BLACK &&  match3(p->bm, p->wm, free, (SQ20|SQ23),SQ31, (SQ32|SQ27|SQ24)))
				{
				if(!(match2(p->bm, free, SQ19, SQ15)))
					{
					e->runaway += v[promoteintwo]; 
					potbk++; 
					e->runaway -= 5*tmod[stones];
					}
				}
			if(p->bm&BIT22)
				{
				if(!(RA22L&white) || !(RA22R&white)) 
					{
					e->runaway+=v[promoteintwo];
					potbk++;
					e->runaway-=5*tmod[stones];
					}
				else if(match2(p->bm, free, SQ21, SQ25|SQ26|SQ29|SQ31))
					{
					//printboard(p);
					e->runaway+=v[promoteintwo];
					potbk++;
					e->runaway-=5*tmod[stones];
					}
				}
			// add detection for support by man on 21! 
			if( (p->bm&BIT23) && !(RA23&white) && !(BIT22&p->wk) ) 
				{
				e->runaway+=v[promoteintwo];
				potbk++;
				e->runaway-=5*tmod[stones];
				}
			}

		if( (p->wm) & RANK3)
			{
			if( (p->wm&BIT8)  && !(RA8&black) && !(p->bk&BIT9)) 
				{e->runaway-=v[promoteintwo];potwk++;e->runaway+=5*tmod[stones];}
			
			if	(p->wm&BIT9)  
				{
				if (!(RA9L&black) || !(RA9R&black))
					{e->runaway-=v[promoteintwo];potwk++;e->runaway+=5*tmod[stones];}
				else if(match2(p->wm, free, SQ12, SQ4|SQ2|SQ8|SQ7))
					{
					//printboard(p);
					e->runaway-=v[promoteintwo];potwk++;e->runaway+=5*tmod[stones];
					}
				}
			// add detection for support by man on 12 
			
			if( (p->wm&BIT10) && (!(RA10L&black) || !(RA10R&black))) 
				{e->runaway-=v[promoteintwo];potwk++;e->runaway+=5*tmod[stones];}
			
			if (p->color==WHITE && match3(p->wm, p->bm, free, (SQ10|SQ13), SQ2, (SQ1|SQ6|SQ9)))
				{
				if(!(match2(p->wm, free, SQ14, SQ18)))
					{
					e->runaway -= v[promoteintwo]; potwk++; e->runaway += 5*tmod[stones];
					}
				}
			
			if( (p->wm&BIT11) && !(RA11&black) ) 
				{e->runaway-=v[promoteintwo];potwk++;e->runaway+=5*tmod[stones];}
			}



		/** in 3 **/


		if( (p->bm) & RANK5)
			{
			if( (p->bm&BIT16) && !(RA16&white) && !(BIT17 & p->wk)) {e->runaway+=v[promoteinthree];e->runaway-=4*tmod[stones];}
			if( (p->bm&BIT17) && !(RA17&white) ) {e->runaway+=v[promoteinthree];e->runaway-=4*tmod[stones];}
			if( (p->bm&BIT18) && !(RA18&white) ) {e->runaway+=v[promoteinthree];e->runaway-=4*tmod[stones];}
			if( (p->bm&BIT19) && !(RA19&white) ) {e->runaway+=v[promoteinthree];e->runaway-=4*tmod[stones];}
			}
		if( (p->wm) & RANK4)
			{
			if( (p->wm&BIT12) && !(RA12&black) ) {e->runaway-=v[promoteinthree];e->runaway+=4*tmod[stones];}
			if( (p->wm&BIT13) && !(RA13&black) ) {e->runaway-=v[promoteinthree];e->runaway+=4*tmod[stones];}
			if( (p->wm&BIT14) && !(RA14&black) ) {e->runaway-=v[promoteinthree];e->runaway+=4*tmod[stones];}
			if( (p->wm&BIT15) && !(RA15&black) && !(BIT14 & p->bk)) {e->runaway-=v[promoteinthree];e->runaway+=4*tmod[stones];}
			}

	/* bridge situations */
	// TODO this is only for the regular bridge. add it for bm 2,4 vs wm 11
	/* for black */
		/* todo: add bridge code: if kings get out from bridge, good. if not, bad! */
		
	if(p->bm & SQ23) 
		{
		if( (p->wm & BIT28) && (p->wm&BIT30) && (free&BIT29) ) // vtune: use &
			{
			ungrounded_black |= SQ23;
			//blackhasbridge = 1;
			/* black has a bridgehead on BIT21 - maybe account for weakness?
			e.g. if p->wk -> penalty, or if wk>bk -> penalty?*/
			/* left side */
			if( (free&BIT24) && (free&BIT25) && (p->bm&BIT20) ) // vtune: use &
				{
				e->runaway+=v[promoteintwo];
				e->runaway-=5*tmod[stones];
				}
			/* right side */
			if( (free&BIT26) && (free&BIT27) && (p->bm&BIT22) ) // vtune: use &
				{
				e->runaway+=v[promoteintwo];
				e->runaway-=5*tmod[stones];
				}
			}
		}

	// "other" bridge
	
	if(p->bm & SQ22)
		{
		if(match2(p->wm, free, SQ29|SQ31, SQ30))
			{
			ungrounded_black |= SQ22;
			//blackhasbridge = 1;
			if(match2(p->bm, free, SQ23, SQ26|SQ27))
				{
				e->runaway+=v[promoteintwo];
				e->runaway-=5*tmod[stones];
				}
			if(match2(p->bm, free, SQ21, SQ25))
				{
				e->runaway+=v[promoteintwo];
				e->runaway-=5*tmod[stones];
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
	/* for white */
	if(p->wm & SQ10) // square 10
		{
		if( (p->bm&BIT1) && (p->bm&BIT3) && (free&BIT2) ) // vtune: use &
			{
			ungrounded_white |= SQ10;
			/*white has a bridgehead on BIT10 - maybe account for weakness?*/
			//whitehasbridge = 1;
			/* left side */
			if( (free&BIT4) && (free&BIT5) && (p->wm&BIT9) ) // vtune: use &
				{
				e->runaway-=v[promoteintwo];
				e->runaway+=5*tmod[stones];
				}
			/* right side */
			if( (free&BIT6) && (free&BIT7) && (p->wm&BIT11) ) // vtune: use &
				{
				e->runaway-=v[promoteintwo];
				e->runaway+=5*tmod[stones];
				}
			}
		}

	// "other" bridge
	
	if(p->wm&SQ11)
		{
		if(match2(p->bm, free, SQ2|SQ4, SQ3))
			{
			ungrounded_white |= SQ11;
			//whitehasbridge = 1;
			if(match2(p->wm, free, SQ12, SQ8))
				{
				e->runaway-=v[promoteintwo];
				e->runaway+=5*tmod[stones];
				}
			if(match2(p->wm, free, SQ10, SQ6|SQ7))
				{
				e->runaway-=v[promoteintwo];
				e->runaway+=5*tmod[stones];
				}
			}
		}

	//
	// end runaway checkers
	//

	eval += e->runaway;

	} /* end stuff only with men */
 
	// TODO:how about mobility eval?
 
   if(kings)
		{
   		// 
		// king-only evaluation
		//
		
		
		// king center control 
		e->king += v[kcval]*bitcount(p->bk&CENTER);
   		e->king -= v[kcval]*bitcount(p->wk&CENTER);
   		
		// kings on edge 
   		e->king += v[keval]*bitcount(p->bk&EDGE);
   		e->king -= v[keval]*bitcount(p->wk&EDGE);
		
		// free and trapped kings 
		// king trapped by one man in single corner 


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

		if( (ki->untrappedwk & SQ4) && (p->bm&SQ3)) // vtune use &
			{
			if (free&SQ12)
				{
				ki->freewk--;   
				e->king += v[kingtrappedinsinglecornerval];
				ki->untrappedwk ^= SQ4;
				}
			}

		if( (ki->untrappedbk & SQ29) && (p->wm&SQ30)) // vtune use &
			{
			if (free&SQ21)
				{
				ki->freebk--;   
				e->king-=v[kingtrappedinsinglecornerval];
				ki->untrappedbk^=SQ29;
				}
			}
		
		/* king assisted by man but trapped by two men in single corner*/
		/* should expand this to take care of situation where there is a man on 25 
			going to king -> gets runaway bonus and makes potbk */
		if(ki->untrappedwk & (BIT0|BIT4) )
			{
			if( p->wm&BIT8)
				{
				if( (p->bm&(BIT1|BIT5)) == (BIT1|BIT5) )
					{
					if( free & BIT12 )
						{
						e->king += v[kingtrappedinsinglecornerbytwoval];
						ki->freewk--;
						ki->untrappedwk ^= (ki->untrappedwk&(BIT0|BIT4));
						}
					}
				}
			}

		if(ki->untrappedbk & (BIT27|BIT31) )
			{
			if( p->bm&BIT23)
				{
				if( (p->wm&(BIT30|BIT26)) == (BIT30|BIT26) )
					{
					if(free & BIT19) // maybe without this?
						{
						e->king -= v[kingtrappedinsinglecornerbytwoval];
						ki->freebk--;
						ki->untrappedbk ^= (ki->untrappedbk & (BIT27|BIT31));
						}
					}
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
		if(ki->untrappedbk & SQ31)
			{
			if( (p->wm&SQ32) && (p->wm&SQ30) && (free&SQ23)) 
				{
				ki->freebk--;
				ki->untrappedbk^=SQ31;
				}
			}

		if(ki->untrappedbk & SQ30)
			{ 
			if( ((p->wm&SQ31) && (p->wm&SQ29) && (free&SQ22)) || ((p->wm&SQ21) && (p->wm&SQ25) && (p->wm&SQ31) && (free&SQ22)) )
				{
				ki->freebk--;// above new 1.376 - add penalties here?
				ki->untrappedbk^=SQ30;
				}

			// new in 1.41b - dynamic trapping of a king - if it moves, 25-22 will win it.
			else if(match1(p->wm, BIT27|BIT23))
				{
				ki->freebk--;
				ki->untrappedbk^=SQ30;
				}
			}

		if(ki->untrappedwk & SQ2)
			{
			if( (p->bm&SQ1) && (p->bm&SQ3) && (free&SQ10))// vtune use &
				{
				ki->freewk--;
				ki->untrappedwk ^= SQ2;
				}
			}

		if(ki->untrappedwk & SQ3)
			{											// below new 1.376
			if( ((p->bm&SQ2) && (p->bm&SQ4) && (free&SQ11))  || ((p->bm&SQ2) && (p->bm&SQ8) && (p->bm&SQ12) && (free&SQ11)) )
				{
				ki->freewk--;
				ki->untrappedwk ^= SQ3;
				}
			else if (match1(p->bm, BIT4|BIT8))
				{
				ki->freewk--;
				ki->untrappedwk ^= SQ3;
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
		
		

	/* king trapped in double corner by two men */
		if(ki->untrappedbk & BIT28)
		{
			if((p->wm&BIT24) && (p->wm&BIT29))
			{
			e->king-=v[kingtrappedindoublecornerval];
			ki->freebk--;
			ki->untrappedbk^=BIT28;
			}
		}
		if(ki->untrappedwk & BIT3)
		{
			if( (p->bm&BIT2) && (p->bm&BIT7) )
			{
			e->king+=v[kingtrappedindoublecornerval];
			ki->freewk--;
			ki->untrappedwk^=BIT3;
			}
		}

 
	/* single dominated white king in single corner new 1.35*/
	// eval weights not tested properly!
	/* special bad case: */

		if(p->wm&SQ12)
			{
			if(p->bm&SQ3)
				{
				/* special bad case: black king trapping wk on 4, and stopping a man on 20*/
				/* if this is true, the others cannot be true, because it requires a BK on bit9*/
				if( ki->untrappedwk&BIT0 && p->bk&BIT9 && p->wm&BIT16)// vtune use &
					{
					//e->king += 50;
					e->king += v[king_blocks_king_and_man];
					ki->freewk--;
					ki->untrappedwk^=BIT0;
					}

				if( (mc->wk==1) && (ki->untrappedwk&(BIT0|BIT4|BIT9)) && (p->bk&(BIT17|BIT18|BIT12|BIT13)))// vtune use &
					{
					e->king += v[dominatedkingval];
					ki->freewk--;
					ki->untrappedwk^=(ki->untrappedwk&(BIT0|BIT4|BIT9));
					}
				if( (mc->wk==0) && (p->wm&(BIT4)) && (p->bk&(BIT17|BIT18|BIT12|BIT13)))
					{
					e->king += v[dominatedkingval];
					}
				}
			}
		/* single dominated black king in single corner */
		if(p->bm&SQ21)
			{
			if(p->wm&SQ30)
				{
				/* special bad case: white king trapping bk on 29, and stopping a man on 13*/
				if( p->wk&SQ22 && ki->untrappedbk&SQ29 && p->bm&SQ13)
					{
					//e->king -= 50;
					e->king -= v[king_blocks_king_and_man];
					ki->freebk--;
					ki->untrappedbk^=SQ29;
					}
				if( (mc->bk==1) && (ki->untrappedbk & (BIT31|BIT27|BIT22)) && (p->wk&(BIT13|BIT14|BIT18|BIT19)))
					{
					e->king -= v[dominatedkingval];
					ki->freebk--;
					ki->untrappedbk^=(p->bk&(BIT31|BIT27|BIT22));
					}
				if( (mc->bk==0) && (p->bm&(BIT27)) && (p->wk&(BIT13|BIT14|BIT18|BIT19)))
					{
					e->king -= v[dominatedkingval];
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

		/* single dominated white king in double corner - new 1.373 */
		if(p->bm&BIT2 && !(p->bm&BIT7))
			{
			if( (mc->wk==1) && (ki->untrappedwk&(BIT3|BIT7)) && (p->bk&BIT14) && !(p->wm&BIT15))
				{
				e->king += v[dominatedkingindcval];
				ki->freewk--;
				ki->untrappedwk^=(ki->untrappedwk&(BIT3|BIT7));
				}
			if( (mc->wk==0) && (p->wm&(BIT7)) && (p->bk&BIT14) && !(p->wm&BIT15))
				{
				e->king += v[dominatedkingindcval];
				}
			}
		/* single dominated black king in double corner - new 1.373 */
		if(p->wm&BIT29 && !(p->wm&BIT24))
			{
			if( (mc->bk==1) && (ki->untrappedbk &(BIT24|BIT28)) && (p->wk&BIT17) && !(p->bm&BIT16))
				{
				e->king -= v[dominatedkingindcval];
				ki->freebk--;
				ki->untrappedbk^=(ki->untrappedbk&(BIT24|BIT28));
				}
			if( (mc->bk==0) && (p->bm&(BIT24)) && (p->wk&BIT17) && !(p->bm&BIT16))
				{
				e->king -= v[dominatedkingindcval];
				}
			}

		// general king mobility: for all untrapped kings, look how far they
		// can go in 3 steps on non-attacked squares. and if they can go in center.
		// WATCH OUT: this destroys the info in ki->untrappedbk, it cannot be used any more
		// after this...

		// TODOTODO: I checked king_no_mobility positions and it seems that this is all 
		// quite ridiculous: if a king can jump over an enemy man it is not seen, so often 
		// a completely normal position is given a penalty where the king is actually threatening to 
		// eat a man!

		while(ki->untrappedbk)
			{
			// peel off untrapped kings and analyze them
			tmp=(ki->untrappedbk&(ki->untrappedbk-1))^ki->untrappedbk;
			ki->untrappedbk^=tmp;
			
			free2 = free | tmp;
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
				e->king -= v[king_denied_center]; 
				//e->king -= 2;
			if(m<=4)
				e->king -= (v[king_low_mobility_mult] * (5 - m));
				//e->king -= (2*(5-m)); 
			// TODO printboard this and also <=4 - maybe only do this for kings not in center?
			if(m<=1)
				{
				ki->freebk--; 
				//e->king -= 10;
				e->king -= v[king_no_mobility]; 
				//printboard(p); 
				//printint32(tmp); 
				//getch(); 
				}
			}
		
		
		while(ki->untrappedwk)
			{
			// peel off untrapped kings and analyze them
			tmp=(ki->untrappedwk&(ki->untrappedwk-1))^ki->untrappedwk;
			ki->untrappedwk^=tmp;
			free2 = free|tmp;
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
			if (!(tmp & CENTER))
				e->king += v[king_denied_center]; // 2;
			if(m<=4)
				e->king += (v[king_low_mobility_mult]*(5-m)); 
			if(m<=1)
				{
				ki->freewk--; 
				e->king += v[king_no_mobility]; //10;
				}
			}

		// king center monopoly : only one side can occupy center
		if((p->bk & CENTER) && !(p->wk&CENTER) && (ki->freebk >= ki->freewk))
			e->king += v[kingcentermonopoly];
		if((p->wk & CENTER) && !(p->bk&CENTER) && (ki->freewk >= ki->freebk))
			e->king -= v[kingcentermonopoly];


		// only king: designed to make cake sac a man when it sees that it
		// gets a king for it, and retains a strong back rank
		/* single black king */
		if((ki->freebk) && (ki->freewk == 0) && (potwk==0))			
			{
			// if black has sacced a man we want to extend this line:
			if(mc->bm == mc->wm-2)
				*delta += NOPRUNEWINDOW;

			e->king += v[onlykingval];
			e->king += blackbackrankpower[p->bm&0xFF];
			if(p->bk & ROAMINGBLACKKING)
				e->king += v[roamingkingval];     
			}	
		/* single white king */
		if( (ki->freewk) && (ki->freebk==0) && (potbk==0) )
			{
			// if white has sacced a man for this we want to extend:
			if(mc->wm == mc->bm-2)
				*delta += NOPRUNEWINDOW;

			e->king += -v[onlykingval];
			e->king -= whitebackrankpower[p->wm>>24];
			if(p->wk & ROAMINGWHITEKING)
				e->king -= v[roamingkingval];
			}

		//
		// end king evaluation
		//

		eval += e->king;


		// 
		// king-man-interaction
		//
		
		if(pieces <= 2*v[kingmanstones]) //12)
			{
			// TODO: why only pieces < 12 for tailhooks and ungrounded men? this looks really bad...
			// optimization gave v[kingmanstones = 10], so pieces <= 20!
			// experimental kingcramper
			// but the match result with pieces <= 14 was worse....

			if (match3(p->bm, p->wm, p->bk, (SQ13), (SQ17 | SQ21 | SQ22 | SQ25), (SQ26)))
				e->king_man += v[experimental_king_cramp]; // 60;
			if (match3(p->wm, p->bm, p->wk, (SQ20), (SQ16 | SQ12 | SQ11 | SQ8), (SQ7)))
				e->king_man -= v[experimental_king_cramp]; // 60;
	
			
			// tailhooks 
			/* with 12 or with even less?*/
		
			m=( (0x37370000) & (p->bk) & (leftforward(p->wm)) & (twoleftforward(p->wm)) );
			m|=( (0xECEC0000) & (p->bk) & (rightforward(p->wm)) & (tworightforward(p->wm)) );
			blacktailhooks = bitcount(m);
			// new: don't penalize kings on edge if they tailhook
			e->king_man -= v[keval]*bitcount(m&EDGE);
			
			m=( (0x0000ECEC) & (p->wk) & (rightbackward(p->bm)) & (tworightbackward(p->bm)) );
			m|=( (0x00003737) & (p->wk) & (leftbackward(p->bm)) & (twoleftbackward(p->bm)) );
			whitetailhooks = bitcount(m);
			e->king_man += v[keval]*bitcount(m&EDGE);
			
			e->king_man += (blacktailhooks-whitetailhooks)*v[tailhookval];
			

			/* king proximity value */
			/* if a king is close to a man of the opposite color, that should
				be dangerous, at least if it is an "endangered man". */
			// 2-stage eval: ungrounded men next to kings get twice the penalty, 
			// ungrounded men one square further away get once the penalty.
			// TODO: why is this only evaluated with pieces <=12? - no longer!
			// and why are bridgeheads evaluated separately?
			// TODO: why not combine this with king mobility evaluation?
		
			m = forward(p->bk) | backward(p->bk);
			kingproximity += bitcount(m&ungrounded_white);
			m |= (forward(m) | backward(m));
			kingproximity += bitcount(m&ungrounded_white);
			
			m = forward(p->wk) | backward(p->wk);
			kingproximity -= bitcount(m&ungrounded_black);
			m |= (forward(m) | backward(m));
			kingproximity -= bitcount(m&ungrounded_black);
			
			e->king_man += kingproximity * v[kingproximityval];
			}

		
		/* endangered bridge heads */
		/* new 1.3 III */
		// TODO should read: if white king cannot get out from under bridge, then big penalty!
		/*if(whitehasbridge)
			{
			if (ki->freebk > ki->freewk)
				e->king_man += 2 * v[endangeredbridge]; 
			if(ki->freebk == ki->freewk)   //else if freebk>0 do this not freebk==freewk
				e->king_man += v[endangeredbridge];
			}
		if(blackhasbridge)
			{
			if(ki->freebk < ki->freewk)
				e->king_man -= 2* v[endangeredbridge];
			if(ki->freebk == ki->freewk)
				e->king_man -= v[endangeredbridge];
			}*/
		/* 'other' bridgeheads */
		/* new 1.35 */
		/* this is too little to stop it from believing in the testpos for this theme   */
		/* together with freeK-- for kings in bridge, this hurts the match result a bit */
		/* test both separately later! */
		/* not tested yet: "if bk" or "if bk>wk" or whatever!*/ 
		// TODO: why not similar to above - if blackhasbridge etc etc - just take code 
		// below to find out whether black has a bridge and set blackhasbridge.
		/*if(p->wm&SQ11)
			{
			if(p->bk && (p->bm&SQ4) && (p->bm&SQ2))
				e->king_man += endangeredbridgeval;
			}
		if(p->bm&SQ22)
			{
			if(p->wk && (p->wm&SQ29) && (p->wm&SQ31))
				e->king_man -= endangeredbridgeval;
			}
		*/
		/* new endgame eval */
		/* if and ending is good for one side, that is, if it has more kings than the other side,
			the objective should be not to continue kinging for both sides. => the stronger side
			should try to stop the men of the opponent with its kings */

	
		if(ending) // number of total stones <=10
			{
			if(ki->freebk >= ki->freewk)
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
						e->king_man += v[kingholdstwomenval];
					else if(m1 == 1)
						e->king_man += v[immobilemanval];
					}
				}
	
			if(ki->freewk >= ki->freebk)
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
						e->king_man -= v[kingholdstwomenval];
					else if(m1 == 1)
						e->king_man -= v[immobilemanval];
					}
			
				}
			}
		//
		// end of king-man-interaction
		//
	
		eval += e->king_man;
		
		
		} // 

	else
		{
		// no kings: give a serious bonus if one side has a runaway AND a strong back rank.
		// new 1.04a
		// taken out again 1.04b
		// TODO: why not increase this bonus?
		if(mc->bm == mc->wm && mc->bm <= 7)
			{
			if(potbk && !potwk && blackbackrankpower[p->bm&0xFF])
				{
				e->compensation += v[compensation]; // 20;   //10
				}
			if(potwk && !potbk && whitebackrankpower[p->wm>>24])
				{
				e->compensation -= v[compensation]; // 20;
				}
			}

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

		/*
		if(mc->bm == mc->wm+1)
			{
			//printf("\nbm == wm+1 tempo is %i", tempo);
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
			if (stones > 12 && e->runaway <= 0)
			{
				if (whitebackrankpower[p->wm >> 24] && e->runaway < 0)
					// white has a good back rank and a runaway, then we want to look at this carefully
					* delta += NOPRUNEWINDOW;

				// TODO: this is crap: tempo changes on every move by +-1, so this term can 
				// oscillate between being on/off on every move. improve this by either making
				// tempo independent of sidetomove; or even better by making this term a function
				// of tempo.
				if (tempo > 4 && whitebackrankpower[p->wm >> 24])
				{
					//*delta = NOPRUNEWINDOW;
					// tempo is >0, so black is advanced. this means we have
					// compensation. 
					e->compensation -= 2 * tempo;
					// check back ranks
					e->compensation -= whitebackrankpower[p->wm >> 24];
					e->compensation += blackbackrankpower[p->bm & 0xFF];
					// augment doghole values
					if (p->bm & BIT24)
						e->compensation -= v[augmentdogholeval];
					if (p->bm & BIT23)
						e->compensation -= v[augmentdogholeval];
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
		if (mc->bm == mc->wm - 1)
		{
			//printf("\nbm == wm-1 tempo is %i", tempo);
			// white is a man up; look for compensation for black
			//if(runaway<0)
			//	   {
			//	   eval-=whitebackrankpower[p->wm>>24];
			//	   if(realdoghole & BLACK) eval-=5;
			//	   eval += 2*tempo;
			//	   }
			if (stones > 12 && e->runaway >= 0) //1426
			{
				if (blackbackrankpower[p->bm & 0xFF] && e->runaway > 0)
					* delta += NOPRUNEWINDOW;

				if (tempo < -4 && blackbackrankpower[p->bm & 0xFF])
				{
					//*delta = NOPRUNEWINDOW;
					// tempo is <0 because white is further advanced that black.
					e->compensation -= 2 * tempo; // black men less advanced than white -> good
					// if black has a powerful back rank -> good
					e->compensation += blackbackrankpower[p->bm & 0xFF];
					// if white has a powerful backrank -> bad
					e->compensation -= whitebackrankpower[p->wm >> 24];
					if (p->wm & BIT8)
						e->compensation += v[augmentdogholeval];
					if (p->wm & BIT7)
						e->compensation += v[augmentdogholeval];
				}
			}
		}
		*/

			// TODOTODO new improved version below ??
		if (mc->wm == mc->bm - 1) {     // white is a man down
			if(stones>12 && e->runaway<0) //many stones, black does not have a runaway checker but white does
				{
				if (whitebackrankpower[p->wm >> 24]) {
					// white has a good back rank and a runaway, then we want to look at this carefully
					*delta += NOPRUNEWINDOW;
					e->compensation -= v[compensation_mandown];
					}
				}
			}

		if(mc->bm == mc->wm-1)
			{
			if (stones > 12 && e->runaway > 0) //many stones, white does not have a runaway checker
			{
				if (blackbackrankpower[p->bm & 0xFF]) {
					// white has a good back rank and a runaway, then we want to look at this carefully
					*delta += NOPRUNEWINDOW;
					e->compensation += v[compensation_mandown];
				}
			}

		//	if(stones>12 && e->runaway >=0 ) //1426
			//	{
				//if(blackbackrankpower[p->bm&0xFF] && e->runaway>0)
				//	*delta += NOPRUNEWINDOW;

				//if(tempo<-4 && blackbackrankpower[p->bm&0xFF] )
				//	{
					//*delta = NOPRUNEWINDOW;
					// tempo is <0 because white is further advanced that black.
					//e->compensation -=2*tempo; // black men less advanced than white -> good
					// if black has a powerful back rank -> good
					//e->compensation += blackbackrankpower[p->bm&0xFF];
					// if white has a powerful backrank -> bad
					//e->compensation -= whitebackrankpower[p->wm>>24];
					//if (p->wm & BIT8)
					//	e->compensation += v[augmentdogholeval]; // 4;
					//if (p->wm & BIT7)
					//	e->compensation += v[augmentdogholeval]; // 4;
					//}
				//}
			}
			// end of compensation eval
			eval += e->compensation;
		}

	// turn 
	if(p->color == BLACK) 
	   turn += v[turnval];
	else 
		turn -= v[turnval];
	eval += turn;
 
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

	// in endings: if weak side has N stones and N-1 kings, it is generally safe:
	// new in cake ss 102b; match result is +50-31 with and without, but eval seems better -> keep.
	if(ending && equal) // auf deutsch: 5-5 or 4-4.
		{
		if( (mc->wm - potwk <=1) && (mc->bm-potbk <=1))
			*likelydraw = 1;
		}
	
	return (p->color == BLACK) ? eval:-eval;
	}


	int initializematerial(short materialeval[13][13][13][13])
	{
		// initialize material value array for value[bm][bk][wm][wk]
		int i, j, k, l;
		int v1, v2;

		for (i = 0; i < 13; i++)
		{
			for (j = 0; j < 13; j++)
			{
				for (k = 0; k < 13; k++)
				{
					for (l = 0; l < 13; l++)
					{
						/*bm bk wm wk */
						//v1 = 100 * i + 130 * j;  // optimized? 130
						//v2 = 100 * k + 130 * l;  // optimized? 130
						v1 = v[man_value] * i + v[king_value] * j;  // optimized? 130
						v2 = v[man_value] * k + v[king_value] * l;  // optimized? 130
						if (v1 + v2 == 0)
							continue;
						v1 = v1 - v2 + (EXBIAS * (v1 - v2)) / (v1 + v2);

						/* take away the 10 points which a side is up over 100 with a one
							man advantage in the 12-11 position */
						if (v1 <= -100)
							v1 += 10;
						if (v1 >= 100)
							v1 -= 10;

						materialeval[i][j][k][l] = v1;
					}
				}
			}
		}
		return 1;
	}


	int initializebackrank(char blackbackrankeval[256], char whitebackrankeval[256], char blackbackrankpower[256], char whitebackrankpower[256])
	{
		// initializes the arrays   blackbackrankeval
		//							whitebackrankeval
		//							blackbackrankpower
		//							whitebackrankpower

		//static int br[32] = { 0,0,2, 2, 4,6,10,10,1,4,16,16,6,10,24,16,			// old before optimization
		//				   0,0,2, 2, 4,10,16,16,1,4,16,16,6,10,24,16 };

		//static int br[32] = { -5,-13,-5,-11, 5,6,14,14,0,-11,18,2,20,19,22,18,		// optimized
		//				 -6,-24,-3,-13, 7,2,13,6,5,-13,22,-2,31,26,31,26 };

		//static int br[32] = { -1,-12,-5,-12, 4,4,11,11,2,-11,16,1,20,19,21,18,		// optimized
		//				 -3,-23,-4,-14, 6,0,10,5,6,-11,20,-2,32,26,31,26 };


		// contains the back rank evaluation 
		// strange: pdntool insists that the back rank 0 x 0 x (bridge) is worse than 0 x x 0 which
		//leaves the double corner open
		// should try a change there! 
		POSITION p;
		int32 u;
		int32 index;
		

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

		for (u = 0; u < 256; u++)
		{
			// for all possible back ranks do: 
			p.bm = u;
			p.wm = u << 24;

			index = p.bm & 0x0000000F;
			if (p.bm & BIT7)
				index += 16;
			blackbackrankeval[u] = br[index];

			index = 0;
			if (p.wm & BIT31)
				index++;
			if (p.wm & BIT30)
				index += 2;
			if (p.wm & BIT29)
				index += 4;
			if (p.wm & BIT28)
				index += 8;
			if (p.wm & BIT24)
				index += 16;

			whitebackrankeval[u] = br[index];

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

					/* oreo */
			if (match1(p.bm, SQ2 | SQ3 | SQ7))
			{
				blackbackrankeval[u] += v[oreoval];
			}
			if (match1(p.wm, SQ31 | SQ30 | SQ26))
			{
				whitebackrankeval[u] += v[oreoval];
			}




			// developed single corner
			// TODO: make this more fine-grained: if men on 4 and 8, subtract something. if
			//		 only man on 4, but not on 8, subtract less
			//       if only man on 8 but not on 4 subract the least.

			if (((~p.bm) & SQ4) && ((~p.bm) & SQ8))
				blackbackrankeval[u] += v[devsinglecorner]; //devsinglecorner 5!!
			if (((~p.wm) & SQ29) && ((~p.wm) & SQ25))
				whitebackrankeval[u] += v[devsinglecorner];//2!!


			//  double corner evals: intact and developed 

			if (match1(p.bm, (SQ2 | SQ5 | SQ6)) && !(p.bm & SQ1))
				blackbackrankeval[u] += v[intactdoublecorner];
			if (match1(p.wm, (SQ27 | SQ28 | SQ31)) && !(p.wm & SQ32))
				whitebackrankeval[u] += v[intactdoublecorner];


			// ideal double corner
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
			if ((p.bm & BIT3) && (p.bm & BIT2) && (p.bm & BIT6) && (!(p.bm & BIT7)))
				blackbackrankeval[u] += v[idealdoublecornerval];//2!!
			// maybe also allow 2,6,7 &!3 to be ideal?
			if ((p.wm & BIT28) && (p.wm & BIT29) && (p.wm & BIT25) && (!(p.wm & BIT24)))
				whitebackrankeval[u] += v[idealdoublecornerval];//2!!

			// new 1.41'
			// maybe also allow 2,6,7 &!3 to be ideal?
			if ((p.bm & BIT7) && (p.bm & BIT2) && (p.bm & BIT6) && (!(p.bm & BIT3)))
				blackbackrankeval[u] += v[intactdoublecorner];//2!!
			if ((p.wm & BIT24) && (p.wm & BIT29) && (p.wm & BIT25) && (!(p.wm & BIT28)))
				whitebackrankeval[u] += v[intactdoublecorner];//2!!	



			// backrankpower gives the power of the back rank to withstand onslaught
			// by enemy men - only used in the case of man down.
						// ideal double corner
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
			// TODO: power is non-zero if black has a bridge for instance
			// TODO: write this with squares to make sure it's readable.
			if (match1(p.bm, 0xE))
				blackbackrankpower[u] = v[backrankpower1]; // 21;// 30;// 14; //30; optimized
			if (match1(p.bm, 0x2E) || match1(p.bm, 0x4E))
				blackbackrankpower[u] = v[backrankpower2]; // 11;// 35;// 3; //  35; optimized
			if (match1(p.bm, 0x6E))
				blackbackrankpower[u] = v[backrankpower3];// 40;// 0; //  40; optimized

			if (match1(p.wm, 0x70000000))
				whitebackrankpower[u] = v[backrankpower1]; // 21;// 30;// 14;// 30; optimized
			if (match1(p.wm, 0x72000000) || match1(p.wm, 0x74000000))
				whitebackrankpower[u] = v[backrankpower2]; // 11;// 35;// 3; // 35; optimized
			if (match1(p.wm, 0x76000000))
				whitebackrankpower[u] = v[backrankpower3]; // 40;// 0;// 40; optimized
		}

		return 1;
	}


	// reverse array
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
	/*
	int reverse(int x) {
		// return a reverted 8-bit pattern, i.e. if you have pattern
		// e.g. 00001111 return 11110000, or if you have 11000000 return 00000011
		int index = 0;
		int i; 
		for (i = 0; i < 8; i++) {
			if (x & (1 << i))
				index += (1 << (8 - i)); 
		}
		return index; 
	}  */
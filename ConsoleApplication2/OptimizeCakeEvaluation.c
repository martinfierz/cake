// TODO
// position selection should be on basis of file on disk, not of eval itself - otherwise, 
// when you run the optimizer again, it gives a different result because the eval has changed.

#include <stdlib.h> 
#include <assert.h>
#include <windows.h>
#include <math.h>
#include "stdafx.h"

// cake-specific includes - structs defines structures, consts defines constants,
// xxx.h defines function prototypes for xxx.c
#include "switches.h"
#include "structs.h"
#include "consts.h"
#include "cake_eval.h"

#define WHITE 1
#define BLACK 2
#define MAN 4
#define KING 8
#define FREE 16

#define PARAMS 135


/* bitboard masks for moves in various directions */
/* here "1" means the squares in the columns 1357 and "2" in 2468.*/
#define RF1  0x0F0F0F0F
#define RF2  0x00707070
#define LF1  0x0E0E0E0E
#define LF2  0x00F0F0F0
#define RB1  0x0F0F0F00
#define RB2  0x70707070
#define LB1  0x0E0E0E00
#define LB2  0xF0F0F0F0
/* bitboard masks for jumps in various directions */
#define RFJ1  0x00070707
#define RFJ2  0x00707070
#define LFJ1  0x000E0E0E
#define LFJ2  0x00E0E0E0
#define RBJ1  0x07070700
#define RBJ2  0x70707000
#define LBJ1  0x0E0E0E00
#define LBJ2  0xE0E0E000

/* back rank masks */
#define WBR  0xF0000000
#define BBR  0x0000000F
#define NWBR 0x0FFFFFFF
#define NBBR 0xFFFFFFF0

// ranks
#define RANK1 0xF
#define RANK2 0xF0
#define RANK3 0xF00
#define RANK4 0xF000
#define RANK5 0xF0000
#define RANK6 0xF00000
#define RANK7 0xF000000
#define RANK8 0xF0000000

float sigmoid(int v, float c); 

int result_translated[4] = { 0, 1, -1, 0 };
static int params[PARAMS]; // parameters to optimize



char strs[PARAMS][128] =  {
	"devsinglecorner", "intactdoublecorner", "oreoval", "idealdoublecornerval", "backrankpower1",
	"backrankpower2", "backrankpower3", "king_value", "nocrampval13", "nocrampval20", "dogholeval", "dogholemandownval",
	"mc_occupyval", "mc_attackval", "realdykeval", "greatdykeval",
	"promoteinone", "promoteintwo", "promoteinthree", "tailhookval", "kcval", "keval",
	"turnval", "kingcentermonopoly", "kingtrappedinsinglecornerval",
	"kingtrappedinsinglecornerbytwoval", "kingtrappedindoublecornerval", "dominatedkingval", "dominatedkingindcval",
	"kingproximityval", "immobilemanval", "kingholdstwomenval", "onlykingval", "roamingkingval",
	"man_value", "balancemult", "skewnessmult", "cramp12", "cramp13", "cramp20", "badstructure", 
	"dogholeval2", "badstructure2", /*"badstructuremax1", "badstructuremax2", "badstructuremin", */
	"badstructure3", "badstructure4",
	"badstructure5", "badstructure6", "badstructure7", "badstructure8",
	"badstructure9", "badstructure10", "badstructure11",
	/*"badstructure2stones",*/ "kingmanstones","immobile_mult", 
	"runaway_destroys_backrank", "king_blocks_king_and_man", "king_denied_center", "king_low_mobility_mult", "king_no_mobility",
	"experimental_king_cramp", "compensation", "compensation_mandown",	
	/*"compensation_mandown_norunaway",*/
	"ungroundedcontact", "endangeredbridge", "endangeredbridge_kingdown",
	"ungrounded0", "ungrounded1", "ungrounded2", "ungrounded3", "ungrounded4", "ungrounded5", "ungrounded6", 
	"ungrounded7", "ungrounded8", "ungrounded9", "ungrounded10", "ungrounded11", "ungrounded12",
	"br0", "br1", "br2", "br3", "br4", "br5", "br6", "br7",
	"br8", "br9", "br10", "br11", "br12", "br13", "br14", "br15",
	"br16", "br17", "br18", "br19", "br20", "br21", "br22", "br23",
	"br24", "br25", "br26", "br27", "br28", "br29", "br30", "br31",
	"tmod0", "tmod1", "tmod2", "tmod3", "tmod4", "tmod5", "tmod6", "tmod7",
	"tmod8", "tmod9", "tmod10", "tmod11", "tmod12", "tmod13", "tmod14", "tmod15",
	"tmod16", "tmod17", "tmod18", "tmod19", "tmod20", "tmod21", "tmod22", "tmod23", "tmod24"
};




float calc_error(int n, EVALUATEDPOSITION* ep, float c); 

void codeoutput() {
	// write parameters as C code to file
	FILE* fp;
	int i; 
	int paramnum = 0; 
	fp = fopen("C:\\code\\checkersdata\\codeoutput.txt", "w");

	getparams(params, &paramnum);
	// print parameters to show it's working
	//for (i = 0; i < paramnum; i++)
	//	fprintf(fp, "\nparameter[%i] is %i (%s)", i, params[i], strs[i]);
	//fprintf(fp, "\nfound %i parameters to optimize", paramnum);

	for (i = 0; i < paramnum - 13 - 25 - 32; i++) {
		fprintf(fp, "\nv[%s] = %i;", strs[i], params[i]);
	}

	//static int ungroundedpenalty[13] = { -1,-1,1,5,10,16,21,27,24,24,21,21,21 }; // optimized

	fprintf(fp, "\n\nstatic int ungroundedpenalty[13] = {");
	for (i = paramnum - 13 - 25 - 32; i < paramnum - 25 - 32; i++) {
		fprintf(fp, " %i,", params[i]);
	}
	fprintf(fp, "};");

	fprintf(fp, "\n\nstatic int br[32] = {");
	for (i = paramnum - 25 - 32; i < paramnum - 25; i++) {
		fprintf(fp, " %i,", params[i]);
	}
	fprintf(fp, "};");

	fprintf(fp, "\n\nstatic int tmod[25] = {");
	for (i = paramnum - 25; i < paramnum; i++) {
		fprintf(fp, " %i,", params[i]);
	}
	fprintf(fp, "};");
	fclose(fp); 
}


int main()
{
	int i = 0, n;
	FILE *fp; 
	unsigned int bm, bk, wm, wk, color; 
	POSITION p; 
	MATERIALCOUNT mc; 
	int delta = 100; 
	EVALUATEDPOSITION *ep, *cur; 
	float error; 
	int j; 
	int oldparam; 
	int iterations = 0; 
	float minerror; 
	int position_number = 0; 
	int changed = 0; 
	int res_from_file; 
	float c; 
	int rejected = 0; 
	int pos_num = 0; 
	int quiet_pos_num = 0; 
	int v1, v3; 
	int iter[3] = { 1,0,2 };
	int sameadjust; 
	int adjust; 
	int paramnum = 0; 
	float influence[PARAMS];
	float influence0[PARAMS]; 
	

	// if I want to check search times, run analyze_matchprogress()
	//analyze_matchprogress(); 


	ep = malloc(sizeof(EVALUATEDPOSITION) * 10000000);

	// initialize eval
	initeval();
	//startparams();
	optimalparams(); 
	updateeval();

	// recall parameters from cake's evaluation
	//getparams(params, &paramnum);

	// print parameters to show it's working
	//for (i = 0; i < paramnum; i++)
	//	printf("\nparameter[%i] is %i (%s)", i, params[i], strs[i]); 
	//getch(); 

	//startparams(); 
	optimalparams(); 
	updateeval();

	getparams(params, &paramnum);
	// print parameters to show it's working
	for (i = 0; i < paramnum; i++)
		printf("\nparameter[%i] is %i (%s)", i, params[i], strs[i]);
	printf("\nfound %i parameters to optimize", paramnum); 


	getch(); 

	// load either file with or without duplicates
//	fp = fopen("c:\\code\\checkersdata\\taggedevaluatedpositions.txt", "r");
	fp = fopen("c:\\code\\checkersdata\\taggedevaluatedpositions+duplicates.txt", "r");


	printf("\nloading...");

	cur = ep; 
	i = 0; 
	j = 0; 
	while (!feof(fp) /* && pos_num < 100000*/) {
		
		// bm, bk, wm, wk, color, evaluation, result
		// evaluation is from side to move; BLACK to move is 2, WHITE to move is 1
		// result is WIN (1) if black won, LOSS (2) if white won, DRAW (0) or UNKNOWN (3)
		// fprintf(fpout, "%u %u %u %u %i %i %i %i\n", bm, bk, wm, wk, color, eval, v1, v3);
			
		fscanf(fp, "%u %u %u %u %i %i %i %i\n", &bm, &bk, &wm, &wk, &color, &res_from_file, &v1, &v3);
		//fscanf(fp, "%u %u %u %u %i %i\n", &bm, &bk, &wm, &wk, &color, &res_from_file);
		pos_num++;
		//printf("\n%i %u %u %u %u %i %i %i\n", j++, bm, bk, wm, wk, color, eval, res_from_file); 
		//getch(); 

		p.bm = bm;
		p.bk = bk; 
		p.wm = wm; 
		p.wk = wk; 
		p.color = color; 
		if (!testcapture(&p)) {
			p.color = p.color^CC; 
			if (!testcapture(&p)) {
				quiet_pos_num++; 
				p.color = p.color^CC;
				//printboard(&p); 
				//getch(); 
				cur->bm = bm;
				cur->bk = bk;
				cur->wm = wm;
				cur->wk = wk;
				cur->color = color;
				cur->value = v3;
				cur->gameresult = res_from_file;
				
				mc.bm = bitcount(p.bm);
				mc.bk = bitcount(p.bk);
				mc.wm = bitcount(p.wm);
				mc.wk = bitcount(p.wk);
				cur->staticeval = evaluation(&p, &mc, 0, &delta, 0, 0);
				if (abs(cur->staticeval - cur->value) > 50) {
					rejected++;
					quiet_pos_num--;
					//printboard(p);
					//printf("\nthis position has evaluation %i but 1ply value %i", cur->staticeval, cur->value);
					//getch();
				}
				else
					cur++; 
				
				
				//getch();
				if (i % 10000 == 0)
					printf("\n%i", i);
				i++;
			}
		}
	}
	fclose(fp); 
	n = i - 1 - rejected;
	printf("\ntotal %i, quiet %i, final %i, %i were rejected", pos_num, quiet_pos_num, n, rejected);
	getch(); 
	printf("\nevaluating...");
	
	// step 1: find optimal constant for sigmoid
	/*float minc = 0; 
    minerror = 1000; 
	for(c = 0.02; c < 0.025; c+= 0.0002) {
		//printf("\n%.3f", c);
		//errorsum = 0; 
		//cur = ep; 

		errorsum = calc_error(n, ep, c);

		if ((errorsum) < minerror) {
			minerror = errorsum;
			minc = c;
		}
		printf("\n%.4f err=%.5f", c, errorsum);
	}
	printf("\nbest c is %.4f with %.5f error - hit key to continue", minc, minerror); 
	c = minc; 
	getch(); */

	float minc = 0.024; 
    c = 0.024; 
	
	

	//for (i = 0; i < PARAMSOPT; i++)
	//	params[i] = 0;
	//params[7] = 130; // keep only king value
	//params[35] = 100; // and man value
	//params[59] = 10; // int badstructuremax1 = 20;
	
	//for (i = 0; i < PARAMSOPT; i++)
	//	notadjust[i] = 0; 

	// initialize 
	//initeval(); 
	setparams(params, paramnum);
	updateeval();
	minerror = calc_error(n, ep, c);
	printf("\ninitial error is %.6f", minerror);
	printf("\nfound %i parameters to optimize", paramnum); 
	while (iterations < 1000) {
		changed = 0;
		sameadjust = 0; 
		for (j = 0; j < paramnum; j++) {  // for all params do
			// skip optimizing this parameter once if it wasn't adjusted on last iteration
			/*if (notadjust[j]) {
				notadjust[j] = 0;
				continue;
			}*/
	
			// save old value	
			oldparam = params[j];
			influence[j] = 0; 

			printf("\nparameter %i:%i", j, oldparam);
			// 1. try going in positive direction
			adjust = 0; 
			while (adjust < 3) {
				params[j] = params[j]+1; 
				setparams(params, paramnum); 
				updateeval(); 
				error = calc_error(n, ep, c);
				influence[j] += fabs(error - minerror);
				printf(" +(%.6f)", error); 
				if (error > (minerror - 1e-7)) {
					// revert
					params[j] = params[j] - 1; 
					break; 
				}
				adjust++; 
				minerror = error; 
				printf(" (%i)", params[j]);
			}

			// 2. if param was not adjusted in positive direction, try negative direction
			if (!adjust) {
				while (adjust < 3) {
					params[j] --;
					setparams(params, paramnum);
					updateeval();
					error = calc_error(n, ep, c);
					printf(" -(%.6f)", error);
					influence[j] += fabs(error - minerror);
					if (error > (minerror-1e-7)) {
						// revert
						params[j]++;
						break;
					}
					adjust++;

					minerror = error; 
					printf(" (%i)", params[j]);
				}
			}
			if (adjust) {
				changed++;
				printf("\nadjusted %s from %i to %i (%.6f)", strs[j], oldparam, params[j], minerror);
			}
			else {
				printf("\n%s unchanged (%i)", strs[j], params[j]); 
			}

		}
		printf("\niteration %i with %i changes-----------------", iterations, changed);
		iterations++; 

		if (changed == 0)
			break; 
	}

	// find out what the influence of this parameter is overall:
	for (i = 0; i < paramnum; i++) {
		oldparam = params[i];
		params[i] = 0;
		setparams(params, paramnum);
		updateeval();
		error = calc_error(n, ep, c);
		influence0[i] = error - minerror; 
		params[i] = oldparam;
	}


	fp = fopen("C:\\code\\checkersdata\\param_influence.txt", "w");
	for (i = 0; i < paramnum; i++) {
		printf("\n%i:\t%i\t(%s)  (+-%.3f, +-%.3f)", i, params[i], strs[i], 1000.0 * influence[i], 1000.0 * influence0[i]);
		fprintf(fp, "%i\t%s\t%i\t%.3f\t%.3f\n", i, strs[i], params[i], 1000.0* influence[i], 1000.0* influence0[i]);
	}
	fclose(fp); 
	printf("\n%i iterations made, no more improvement possible", iterations);
	printf("\nerror after optimization is %.7f", minerror);
	codeoutput();
	printf("\nparameters written to c:\code\checkersdata\codeoutput.txt");

	getch(); 
    return 0;
}

float calc_error(int n, EVALUATEDPOSITION* ep, float c) {
	int i;
	POSITION p;
	MATERIALCOUNT mc;
	int staticeval;
	int delta;
	float res, error, errorsum = 0;
	

	for (i = 0; i < n; i++) {
		p.bm = ep[i].bm;
		p.bk = ep[i].bk;
		p.wm = ep[i].wm;
		p.wk = ep[i].wk;
		p.color = ep[i].color;

		mc.bm = bitcount(p.bm);
		mc.bk = bitcount(p.bk);
		mc.wm = bitcount(p.wm);
		mc.wk = bitcount(p.wk);
		staticeval = evaluation(&p, &mc, 0, &delta, 0, 0);
		//errorsum += abs(staticeval - ep[i].value);

		//printboard(&p); 
		//printf("\nstatic eval is %i", staticeval); 
		//getch(); 

		// logistic optimization
		// staticeval was computed as side to move. normalize to black to move
		if (ep[i].color == WHITE)
			staticeval = -staticeval; // now staticeval is as seen from black's point of view
		res = result_translated[ep[i].gameresult];
		// res is game result as seen from black's point of view (1 = win, 0 = draw, -1 = loss)
		error = (res - sigmoid(staticeval, c));
		errorsum += error * error;
		//if (error < 0)
		//	error = -error; 
		//errorsum += (error); // error* error;
		//if (i % 100000 == 0) {
		//	printf("\n%i %.10f", i, errorsum);
		//}

		//printboard(&p);
		//printf("\nstatic eval, search eval: %i %i", staticeval, eval);
		//getch();

	}
	errorsum = errorsum / (float)n; 
	//printf("\n***** %.6f",errorsum);
	
	return errorsum;
}


float sigmoid(int v, float c) {
	float res; 
	//float c = 0.02; 
	//res = exp(0.001);

	res = 2 / (1 + exp((double) (-c * ((float)v)))) - 1;
	return res; 

	// example: if value = 100 i get
}


/*
int printboard(POSITION *p)
{
	int i;
	int free = ~(p->bm | p->bk | p->wm | p->wk);
	int b[32];
	char c[15] = "-wb      WB";
	for (i = 0; i<32; i++)
	{
		if ((p->bm >> i) % 2)
			b[i] = BLACK;
		if ((p->bk >> i) % 2)
			b[i] = BLACK | KING;
		if ((p->wm >> i) % 2)
			b[i] = WHITE;
		if ((p->wk >> i) % 2)
			b[i] = WHITE | KING;
		if ((free >> i) % 2)
			b[i] = 0;
	}

	printf("\n\n");
	printf("\n %c %c %c %c", c[b[28]], c[b[29]], c[b[30]], c[b[31]]);
	printf("\n%c %c %c %c ", c[b[24]], c[b[25]], c[b[26]], c[b[27]]);
	printf("\n %c %c %c %c", c[b[20]], c[b[21]], c[b[22]], c[b[23]]);
	printf("\n%c %c %c %c ", c[b[16]], c[b[17]], c[b[18]], c[b[19]]);
	printf("\n %c %c %c %c", c[b[12]], c[b[13]], c[b[14]], c[b[15]]);
	printf("\n%c %c %c %c ", c[b[8]], c[b[9]], c[b[10]], c[b[11]]);
	printf("\n %c %c %c %c", c[b[4]], c[b[5]], c[b[6]], c[b[7]]);
	printf("\n%c %c %c %c ", c[b[0]], c[b[1]], c[b[2]], c[b[3]]);

	if (p->color == BLACK)
		printf("\nblack to move");
	else
		printf("\nwhite to move");
}*/

//int db_exit() {
//	return 0; 
//}



int testcapture(POSITION *p)
{
	// testcapture returns 1 if the side to move has a capture.
	unsigned int black, white, free, m;

	black = p->bm | p->bk;
	white = p->wm | p->wk;
	free = ~(black | white);
	if (p->color == BLACK)
	{
		m = ((((black&LFJ2) << 4)&white) << 3);
		m |= ((((black&LFJ1) << 3)&white) << 4);
		m |= ((((black&RFJ1) << 4)&white) << 5);
		m |= ((((black&RFJ2) << 5)&white) << 4);
		if (p->bk)
		{
			m |= ((((p->bk&LBJ1) >> 5)&white) >> 4);
			m |= ((((p->bk&LBJ2) >> 4)&white) >> 5);
			m |= ((((p->bk&RBJ1) >> 4)&white) >> 3);
			m |= ((((p->bk&RBJ2) >> 3)&white) >> 4);
		}
	}
	else
	{
		m = ((((white&LBJ1) >> 5)&black) >> 4);
		m |= ((((white&LBJ2) >> 4)&black) >> 5);
		m |= ((((white&RBJ1) >> 4)&black) >> 3);
		m |= ((((white&RBJ2) >> 3)&black) >> 4);
		if (p->wk)
		{
			m |= ((((p->wk&LFJ2) << 4)&black) << 3);
			m |= ((((p->wk&LFJ1) << 3)&black) << 4);
			m |= ((((p->wk&RFJ1) << 4)&black) << 5);
			m |= ((((p->wk&RFJ2) << 5)&black) << 4);
		}
	}
	if (m & free)
		return 1;
	return 0;
}




//#include "dblookup.h"
//#include "initcake.h"
//#include "cakepp.h"


// some helper functions

int32 attack_forwardjump(int32 x, int32 free)
{
	// return all squares which are forward-attacked by pieces in x, with free
	return (((((x&LFJ1) << 3)&free)&(free >> 4)) | ((((x&LFJ2) << 4)&free)&(free >> 3)) | ((((x&RFJ1) << 4)&free)&(free >> 5)) | ((((x&RFJ2) << 5)&free)&(free >> 4)));
}

int32 attack_backwardjump(int32 x, int32 free)
{
	return (((((x&LBJ1) >> 5)&free)&(free << 4)) | ((((x&LBJ2) >> 4)&free)&(free << 5)) | ((((x&RBJ1) >> 4)&free)&(free << 3)) | ((((x&RBJ2) >> 3)&free)&(free << 4)));
}

int32 forwardjump(int32 x, int32 free, int32 other)
{
	return ((((((x&LFJ1) << 3)&other) << 4)&free) | (((((x&LFJ2) << 4)&other) << 3)&free) | (((((x&RFJ1) << 4)&other) << 5)&free) | (((((x&RFJ2) << 5)&other) << 4)&free));
}

int32 backwardjump(int32 x, int32 free, int32 other)
{
	return ((((((x&LBJ1) >> 5)&other) >> 4)&free) | (((((x&LBJ2) >> 4)&other) >> 5)&free) | (((((x&RBJ1) >> 4)&other) >> 3)&free) | (((((x&RBJ2) >> 3)&other) >> 4)&free));
}



int countmaterial(POSITION *p, MATERIALCOUNT *m)
{
	m->bm = bitcount(p->bm);
	m->bk = bitcount(p->bk);
	m->wm = bitcount(p->wm);
	m->wk = bitcount(p->wk);
	return 0;
}



// table-lookup bitcount - newer CPUs are going to have a popcount instruction soon, would be
// much more efficient.
int bitcount(int32 n)
// returns the number of bits set in the 32-bit integer n 
{
	return __popcnt(n);

	//return (bitsinword[n&0x0000FFFF]+bitsinword[(n>>16)&0x0000FFFF]);
}



/*Kingsrow(x64) 1.17d played 25 - 21
analysis: value = 4, depth 22 / 21.5 / 41, 2.7s, 5497 kN / s, pv 25 - 21 9 - 14 29 - 25 11 - 15 23 - 18 14x23 27x11 8x15 17 - 14 10x17 21x14
Cake 1.85_2017d(x64) played 9 - 14
analysis : depth 19 / 40 / 20.6  time 1.38s  value = 10  nodes 5452751  3922kN / s  db 98 % cut 95.3% pv  9 - 14 22 - 18 13x22 26x17 11 - 15 18x11  8x15 29 - 25
Kingsrow(x64) 1.17d played 21x14
analysis : value = 2, depth 5 / 6.7 / 12, 0.0s, 13 kN / s, pv 21x14 4 - 8 24 - 19 15x24 28x19 8 - 11
Cake 1.85_2017d(x64) played 12 - 16
analysis : depth 19 / 34 / 19.3  time 2.70s  value = 2  nodes 9382738  3460kN / s  db 100 % cut 95.6% pv 12 - 16 32 - 27 16 - 20 24 - 19 15x24 28x19  4 - 8 25 - 21
*/

int analyze_matchprogress(void) {
	// read match progress file
	FILE* fp; 
	char line[256]; 
	int value, d1, d2;
	float time, d3; 
	float timesum_kr = 0, timesum_cake = 0; 
	int n_kr = 0, n_cake = 0; 

	fp = fopen("C:\\Users\\Martin Fierz\\Documents\\Martin Fierz\\CheckerBoard\\games\\matches\\matchlog36'.txt", "r");
	while (!feof(fp)) {
		fgets(line, 255, fp);
		if (line[0] == 'a' && (line[10] == 'v' || line[33] == 'v')) {  // kingsrow
			//printf("\n%s", line);
			sscanf(line, "analysis: value=%i, depth  %i/%f/%i, %fs", &value, &d1, &d3, &d2, &time);
			//sscanf(line, "analysis: time remaining:%fs   value=%i, depth  %i/%f/%i, %fs", &dummy, &value, &d1, &d3, &d2, &time);
			if (d1 > 10 && d1 < 50
				) {
				timesum_kr += time;
				n_kr++;
			}
			//printf("\n%i   %i/%.1f/%i   %.1f", value, d1, d3, d2, time);
		}
		if (line[0] == 'a' && (line[10] == 'd' || line[33] == 'd')) {  // cake
			//printf("\n%s", line);
			sscanf(line, "analysis: depth %i/%i/%f  time %f", &d1, &d2, &d3, &time);
			//sscanf(line, "analysis: time remaining:%fs  depth %i/%i/%f  time %f", &dummy, &d1, &d2, &d3, &time);
			//printf("\n%i/%i/%.1f %.2f", d1,d2,d3,time);
			if (d1 > 10 && d1 < 50) {
				timesum_cake += time;
				n_cake++;
			}
			//getch(); 
		}
	}
	printf("\n\naverage time kingsrow: %.3f", timesum_kr / n_kr);
	printf("\naverage time cake: %.3f", timesum_cake / n_cake);
	getch();
	exit(0);
	return 0; 
}
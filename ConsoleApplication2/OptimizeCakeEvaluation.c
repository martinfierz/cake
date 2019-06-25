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
#include "..\\cake_eval.h"

#define WHITE 1
#define BLACK 2
#define MAN 4
#define KING 8
#define FREE 16

#define PARAMS 632


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
void codeoutput(int recall);

int result_translated[4] = { 0, 1, -1, 0 };
static int params[PARAMS]; // parameters to optimize

int active_set[PARAMS]; // specify which parameters are used in optimization.

char strs[PARAMS][128] =  {
	"man_value", "king_value", "piecedown_9", "piecedown_11", "twokingbonus_10", "twokingbonus_12",
	"exchangebias",
	/*"devsinglecorner", "intactdoublecorner", "oreoval", "idealdoublecornerval", */"backrankpower1",
	"backrankpower2", "backrankpower3", "backrankpower4", "nocrampval13", "nocrampval20", 
	"dogholeval", "dogholemandownval",
	"mc_occupyval", "mc_attackval", "realdykeval", "greatdykeval",
	"promoteinone", "promoteintwo", "promoteinthree", "tailhookval", "kcval", "keval",
	"turnval", "turnval_eg", "kingcentermonopoly", "kingtrappedinsinglecornerval",
	"kingtrappedinsinglecornerbytwoval", "kingtrappedindoublecornerval", "dominatedkingval", "dominatedkingindcval",
	"kingproximityval1", "kingproximityval2", "immobilemanval", "kingholdstwomenval", "onlykingval", "roamingkingval",
    "balancemult", "skewnessmult", "skewnessmult_eg", "cramp12", "cramp13", "cramp13_eg",
	"cramp20", "badstructure", 
	"dogholeval2", "badstructure2", 
	"badstructure3", "badstructure4",
	"badstructure5", "badstructure6", "badstructure7", "badstructure8",
	"badstructure9", "badstructure10", "badstructure11",
	"kingmanstones","immobile_mult", "immobile_mult_kings",
	"runaway_destroys_backrank", "king_blocks_king_and_man", "king_denied_center", "king_low_mobility_mult", 
	"king_no_mobility",
	"experimental_king_cramp", "compensation", "compensation_mandown",	
	"ungroundedcontact", "endangeredbridge", "endangeredbridge_kingdown",
	"ungrounded0", "ungrounded1", "ungrounded2", "ungrounded3", "ungrounded4", "ungrounded5", "ungrounded6", 
	"ungrounded7", "ungrounded8", "ungrounded9", "ungrounded10", "ungrounded11", "ungrounded12",
	
	"br0", "br1", "br2", "br3", "br4", "br5", "br6", "br7",
	"br8", "br9", "br10", "br11", "br12", "br13", "br14", "br15",
	"br16", "br17", "br18", "br19", "br20", "br21", "br22", "br23",
	"br24", "br25", "br26", "br27", "br28", "br29", "br30", "br31",

	"br32", "br33", "br34", "br35", "br36", "br37", "br38", "br39",
	"br40", "br41", "br42", "br43", "br44", "br45", "br46", "br47",
	"br48", "br49", "br50", "br51", "br52", "br53", "br54", "br55",
	"br56", "br57", "br58", "br59", "br60", "br61", "br62", "br63",

	"br64", "br65", "br66", "br67", "br68", "br69", "br70", "br71",
	"br72", "br73", "br74", "br75", "br76", "br77", "br78", "br79",
	"br80", "br81", "br82", "br83", "br84", "br85", "br86", "br87",
	"br88", "br89", "br90", "br91", "br92", "br93", "br94", "br95",

		"br96", "br97", "br98", "br99", "br100", "br101", "br102", "br103",
	"br104", "br105", "br106", "br107", "br108", "br109", "br110", "br111",
	"br112", "br113", "br114", "br115", "br116", "br117", "br118", "br119",
	"br120", "br121", "br122", "br123", "br124", "br125", "br126", "br127",

		"br128", "br129", "br130", "br131", "br132", "br133", "br134", "br135",
	"br136", "br137", "br138", "br139", "br140", "br141", "br142", "br143",
	"br144", "br145", "br146", "br147", "br148", "br149", "br150", "br151",
	"br152", "br153", "br154", "br155", "br156", "br157", "br158", "br159",

		"br160", "br161", "br162", "br163", "br164", "br165", "br166", "br167",
	"br168", "br169", "br170", "br171", "br172", "br173", "br174", "br175",
	"br176", "br177", "br178", "br179", "br180", "br181", "br182", "br183",
	"br184", "br185", "br186", "br187", "br188", "br189", "br190", "br191",

		"br192", "br193", "br194", "br195", "br196", "br197", "br198", "br199",
	"br200", "br201", "br202", "br203", "br204", "br205", "br206", "br207",
	"br208", "br209", "br210", "br211", "br212", "br213", "br214", "br215",
	"br216", "br217", "br218", "br219", "br220", "br221", "br222", "br223",

		"br224", "br225", "br226", "br227", "br228", "br229", "br230", "br231",
	"br232", "br233", "br234", "br235", "br236", "br237", "br238", "br239",
	"br240", "br241", "br242", "br243", "br244", "br245", "br246", "br247",
	"br248", "br249", "br250", "br251", "br252", "br253", "br254", "br255",

	"br_eg0", "br_eg1", "br_eg2", "br_eg3", "br_eg4", "br_eg5", "br_eg6", "br_eg7",
	"br_eg8", "br_eg9", "br_eg10", "br_eg11", "br_eg12", "br_eg13", "br_eg14", "br_eg15",
	"br_eg16", "br_eg17", "br_eg18", "br_eg19", "br_eg20", "br_eg21", "br_eg22", "br_eg23",
	"br_eg24", "br_eg25", "br_eg26", "br_eg27", "br_eg28", "br_eg29", "br_eg30", "br_eg31",

	"br_eg32", "br_eg33", "br_eg34", "br_eg35", "br_eg36", "br_eg37", "br_eg38", "br_eg39",
	"br_eg40", "br_eg41", "br_eg42", "br_eg43", "br_eg44", "br_eg45", "br_eg46", "br_eg47",
	"br_eg48", "br_eg49", "br_eg50", "br_eg51", "br_eg52", "br_eg53", "br_eg54", "br_eg55",
	"br_eg56", "br_eg57", "br_eg58", "br_eg59", "br_eg60", "br_eg61", "br_eg62", "br_eg63",

	"br_eg64", "br_eg65", "br_eg66", "br_eg67", "br_eg68", "br_eg69", "br_eg70", "br_eg71",
	"br_eg72", "br_eg73", "br_eg74", "br_eg75", "br_eg76", "br_eg77", "br_eg78", "br_eg79",
	"br_eg80", "br_eg81", "br_eg82", "br_eg83", "br_eg84", "br_eg85", "br_eg86", "br_eg87",
	"br_eg88", "br_eg89", "br_eg90", "br_eg91", "br_eg92", "br_eg93", "br_eg94", "br_eg95",

	"br_eg96", "br_eg97", "br_eg98", "br_eg99", "br_eg100", "br_eg101", "br_eg102", "br_eg103",
	"br_eg104", "br_eg105", "br_eg106", "br_eg107", "br_eg108", "br_eg109", "br_eg110", "br_eg111",
	"br_eg112", "br_eg113", "br_eg114", "br_eg115", "br_eg116", "br_eg117", "br_eg118", "br_eg119",
	"br_eg120", "br_eg121", "br_eg122", "br_eg123", "br_eg124", "br_eg125", "br_eg126", "br_eg127",

	"br_eg128", "br_eg129", "br_eg130", "br_eg131", "br_eg132", "br_eg133", "br_eg134", "br_eg135",
	"br_eg136", "br_eg137", "br_eg138", "br_eg139", "br_eg140", "br_eg141", "br_eg142", "br_eg143",
	"br_eg144", "br_eg145", "br_eg146", "br_eg147", "br_eg148", "br_eg149", "br_eg150", "br_eg151",
	"br_eg152", "br_eg153", "br_eg154", "br_eg155", "br_eg156", "br_eg157", "br_eg158", "br_eg159",

	"br_eg160", "br_eg161", "br_eg162", "br_eg163", "br_eg164", "br_eg165", "br_eg166", "br_eg167",
	"br_eg168", "br_eg169", "br_eg170", "br_eg171", "br_eg172", "br_eg173", "br_eg174", "br_eg175",
	"br_eg176", "br_eg177", "br_eg178", "br_eg179", "br_eg180", "br_eg181", "br_eg182", "br_eg183",
	"br_eg184", "br_eg185", "br_eg186", "br_eg187", "br_eg188", "br_eg189", "br_eg190", "br_eg191",

	"br_eg192", "br_eg193", "br_eg194", "br_eg195", "br_eg196", "br_eg197", "br_eg198", "br_eg199",
	"br_eg200", "br_eg201", "br_eg202", "br_eg203", "br_eg204", "br_eg205", "br_eg206", "br_eg207",
	"br_eg208", "br_eg209", "br_eg210", "br_eg211", "br_eg212", "br_eg213", "br_eg214", "br_eg215",
	"br_eg216", "br_eg217", "br_eg218", "br_eg219", "br_eg220", "br_eg221", "br_eg222", "br_eg223",

	"br_eg224", "br_eg225", "br_eg226", "br_eg227", "br_eg228", "br_eg229", "br_eg230", "br_eg231",
	"br_eg232", "br_eg233", "br_eg234", "br_eg235", "br_eg236", "br_eg237", "br_eg238", "br_eg239",
	"br_eg240", "br_eg241", "br_eg242", "br_eg243", "br_eg244", "br_eg245", "br_eg246", "br_eg247",
	"br_eg248", "br_eg249", "br_eg250", "br_eg251", "br_eg252", "br_eg253", "br_eg254", "br_eg255",


	"tmod0", "tmod1", "tmod2", "tmod3", "tmod4", "tmod5", "tmod6", "tmod7",
	"tmod8", "tmod9", "tmod10", "tmod11", "tmod12", "tmod13", "tmod14", "tmod15",
	"tmod16", "tmod17", "tmod18", "tmod19", "tmod20", "tmod21", "tmod22", "tmod23", "tmod24",
	"kmob0", "kmob1", "kmob2", "kmob3", "kmob4", "kmob5", "kmob6", "kmob7", "kmob8", "kmob9"
};




float calc_error(int n, EVALUATEDPOSITION* ep, float c); 



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
	int /*v0, */v1, v3, staticeval; 
	int iter[3] = { 1,0,2 };
	int sameadjust; 
	int adjust; 
	int paramnum = 0; 
	int allactive = 1; 
	int initialparams[PARAMS]; 
	float influence[PARAMS];
	float influence0[PARAMS]; 
	//char FEN[256]; 
	FILE* log; 

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
	//optimalparams(); 
	//updateeval();

	getparams(params, &paramnum);

	printf("\nPARAMS is %i, paramnum is %i", PARAMS, paramnum);
	// print parameters to show it's working
	for (i = 0; i < paramnum; i++) {
		printf("\nparameter[%i] is %i (%s)", i, params[i], strs[i]);
		initialparams[i] = params[i]; 
	}
	printf("\nfound %i parameters to optimize", paramnum); 

	// open optimizer logfile, save params
	log = fopen("c:\\code\\checkersdata\\optimizerlog.txt", "w");

	getch(); 

	// here, test specific positions: rattlesnake problem because of freewk < freebk, check again later
	/*
	sprintf(FEN, "W:W28,K15,13,K11,7:BK25,K22,12,5,4."); 
	FENtoPosition(FEN, &p);
	mc.bm = bitcount(p.bm);
	mc.bk = bitcount(p.bk);
	mc.wm = bitcount(p.wm);
	mc.wk = bitcount(p.wk);
	staticeval = evaluation(&p, &mc, 0, &delta, 0, 0);
	getch(); */

	// load either file with or without duplicates
    fp = fopen("c:\\code\\checkersdata\\taggedevaluatedpositions.txt", "r");
	//fp = fopen("c:\\code\\checkersdata\\taggedevaluatedpositions+duplicates.txt", "r");


	printf("\nloading...");

	cur = ep; 
	i = 0; 
	j = 0; 
	while (!feof(fp) /* && pos_num < 100000*/) {
		
		// bm, bk, wm, wk, color, evaluation, result
		// evaluation is from side to move; BLACK to move is 2, WHITE to move is 1
		// result is WIN (1) if black won, LOSS (2) if white won, DRAW (0) or UNKNOWN (3)
		//fprintf(fpout, "%u %u %u %u %i %i %i %i\n", bm, bk, wm, wk, color, eval, v1, v3);
		fscanf(fp, "%u %u %u %u %i %i %i %i %i\n", &bm, &bk, &wm, &wk, &color, &res_from_file, &staticeval, &v1, &v3);
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
				//cur->staticeval = evaluation(&p, &mc, 0, &delta, 0, 0);
				//if (abs(cur->staticeval - cur->value) > 50) {
				if(abs(staticeval - v3) > 50) {
					rejected++;
					quiet_pos_num--;
					//printboard(p);
					//printf("\nthis position (%i) has evaluation %i but 3ply value %i", i, staticeval, v3);
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
	minerror = calc_error(n, ep, c);
	printf("\ninitial error is %.7f", minerror);
	fprintf(log, "\ninitial error is %.7f", minerror);


	// reset all parameters to 0
	/*for (i = 0; i < PARAMS; i++)
		params[i] = 0;
	params[man_value] = 100; 
	params[king_value] = 130; 
	params[exchangebias] = 25; */
	//startparams(); 
	optimalparams(); 
	getparams(params, &paramnum); 

	for (i = 0; i < 256; i++) {
		params[arraystart + i + 13 + 256] = 0;
		params[arraystart + i + 13] = 0;
	}

	setparams(params, paramnum);
	updateeval();
	

	//setparams(params, paramnum);
	//updateeval();
	minerror = calc_error(n, ep, c);
	printf("\nerror after setting all parameters is %.7f", minerror);
	printf("\nfound %i parameters to optimize - hit key to continue", paramnum); 
	getch(); 


	/*deactivate_all(); 
	active_set[man_value] = 1;
	active_set[king_value] = 1;
	active_set[piecedown_9] = 1;
	active_set[piecedown_11] = 1;
	active_set[twokingbonus_10] = 1;
	active_set[twokingbonus_12] = 1;
	active_set[exchangebias] = 1; */
	activate_all(); 

	while (iterations < 1000) {
		changed = 0;
		sameadjust = 0; 
		for (j = 0; j < paramnum; j++) {  // for all params do
			// skip if parameter not activated
			if (active_set[j] == 0) {
				printf("\nskipping parameter %i", j);
				allactive = 0; 
				continue;
			}
	
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
				printf("\n---------->>>> adjusted %s from %i to %i (%.6f)", strs[j], oldparam, params[j], minerror);
				fprintf(log, "\n---------->>>> adjusted %s from %i to %i (%.6f)", strs[j], oldparam, params[j], minerror);
			}
			else {
				printf("\n%s unchanged (%i)", strs[j], params[j]);
				fprintf(log, "\n%s unchanged (%i) - setting inactive!", strs[j], params[j]);
				active_set[j] = 0;
			}
		}
		printf("\niteration %i with %i changes (%.7f)-----------------", iterations, changed, minerror);
		fprintf(log, "\niteration %i with %i changes (%.7f)-----------------", iterations, changed, minerror);
		iterations++; 

		if (changed == 0 && allactive)
			break; 
		if (changed == 0) {
			printf("\nno changes on this iteration, but not all active - reactivating all");
			fprintf(log, "\nno changes on this iteration, but not all active - reactivating all");
			allactive = 1;
			activate_all();
		}
			
	}

	// first write params directly
	codeoutput(0);

	// then write them after recalling from the eval
	// TODO: test what happens if I outcomment this, is influence then OK?
	// if yes, what does this mean for my code correctness???
	//codeoutput(1);
	activate_all();
	// find out what the influence of the parameters is overall:
	printf("\nmeasuring absolute influence of all parameters..."); 
	for (i = 0; i < paramnum; i++) {
		// skip if parameter not activated
		if (active_set[i] == 0)
			continue;
		
		printf(".");
		oldparam = params[i];
		params[i] = 0;
		setparams(params, paramnum);
		updateeval();
		error = calc_error(n, ep, c);
		influence0[i] = error - minerror; 
		params[i] = oldparam;
	}

	fprintf(log, "\ninitial\toptimized");
	fp = fopen("C:\\code\\checkersdata\\param_influence.txt", "w");
	for (i = 0; i < paramnum; i++) {
		printf("\n%i:\t%i\t(%s)  (+-%.3f, +-%.3f)", i, params[i], strs[i], 1000.0 * influence[i], 1000.0 * influence0[i]);
		fprintf(fp, "%i\t%s\t%i\t%.3f\t%.3f\n", i, strs[i], params[i], 1000.0* influence[i], 1000.0* influence0[i]);
		fprintf(log, "\n%i\t%i\t%s", initialparams[i], params[i], strs[i]); 
	}
	fclose(fp); 
	printf("\n%i iterations made, no more improvement possible", iterations);
	printf("\nerror after optimization is %.7f", minerror);
	fprintf(log, "\nerror after optimization is %.7f", minerror);
	fclose(log); 
	
	printf("\nparameters written to c:\\code\\checkersdata\\codeoutput.txt");

	getch(); 
    return 0;
}

int activate_all() {
	for (int i = 0; i < PARAMS; i++)
		active_set[i] = 1;
}

int deactivate_all() {
	for (int i = 0; i < PARAMS; i++)
		active_set[i] = 0;
}

void codeoutput(int recall) {
	// write parameters as C code to file
	FILE* fp;
	int i; // , j;
	int paramnum = 0;

	int br[256]; 
	

	if (recall != 0) {
		getparams(params, &paramnum);
		fp = fopen("C:\\code\\checkersdata\\codeoutputrecalled.txt", "w");
	}
	else {
		fp = fopen("C:\\code\\checkersdata\\codeoutput.txt", "w");
		paramnum = PARAMS; 
	}
	
	
	// print parameters to show it's working
	//for (i = 0; i < paramnum; i++)
	//	fprintf(fp, "\nparameter[%i] is %i (%s)", i, params[i], strs[i]);
	//fprintf(fp, "\nfound %i parameters to optimize", paramnum);

	for (i = 0; i < paramnum - 13 - 25 - 256 -256 -10; i++) {
		fprintf(fp, "\nv[%s] = %i;", strs[i], params[i]);
	}

	//static int ungroundedpenalty[13] = { -1,-1,1,5,10,16,21,27,24,24,21,21,21 }; // optimized

	fprintf(fp, "\n\nstatic int ungroundedpenalty[13] = {");
	for (i = paramnum - 13 - 25 - 256 -256 - 10; i < paramnum - 25 - -256 - 256 - 10; i++) {
		fprintf(fp, " %i,", params[i]);
	}
	fprintf(fp, "};");

	fprintf(fp, "\nstatic int backrank[256] = {");
	for (i = paramnum - 25 - 256 -256 -10; i < paramnum - 25 - 256 - 10; i++) {
		fprintf(fp, " %i,", params[i]);
		if ((i - paramnum + 25 + 256 + 10) % 16 == 0)
			fprintf(fp, "\n");
	}
	fprintf(fp, "};");

	fprintf(fp, "\nstatic int backrank_eg[256] = {");
	for (i = paramnum - 25 - 256 - 10; i < paramnum - 25 - 10; i++) {
		fprintf(fp, " %i,", params[i]);
		if ((i - paramnum + 25 + 256 + 10) % 16 == 0)
			fprintf(fp, "\n");
	}
	fprintf(fp, "};");



	fprintf(fp, "\nstatic int tmod[25] = {");
	for (i = paramnum - 25-10; i < paramnum-10; i++) {
		fprintf(fp, " %i,", params[i]);
	}
	fprintf(fp, "};");

	fprintf(fp, "\nstatic int kingmobility[10] = {");
	for (i = paramnum - 10; i < paramnum; i++) {
		fprintf(fp, " %i,", params[i]);
	}
	fprintf(fp, "};");


	/*
	getblackbackrank(br); 
	fprintf(fp, "\nstatic int blackbackrank[256] = {");
	for (int j = 0; j < 16; j++) {
		for (i = 0; i < 16; i++) {
			fprintf(fp, "%i,", br[16 * j + i]);
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "};");

	getwhitebackrank(br);
	fprintf(fp, "\nstatic int whitebackrank[256] = {");
	for (int j = 0; j < 16; j++) {
		for (i = 0; i < 16; i++) {
			fprintf(fp, "%i,", br[16 * j + i]);
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "};");*/

	fclose(fp);
}


float calc_error(int n, EVALUATEDPOSITION* ep, float c) {
	int i;
	POSITION p;
	MATERIALCOUNT mc;
	int staticeval;
	int delta;
	float res, error, errorsum = 0;
	int num = 0; 
	

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
		num++; 
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
	errorsum = errorsum / (float)num; 
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

int analyze_matchlog(void) {
	// read match log file
	FILE* fp; 
	char line[256]; 
	int value, d1, d2;
	float time, d3; 
	float timesum_kr = 0, timesum_cake = 0; 
	int n_kr = 0, n_cake = 0; 
	float time_cake[100000];
	float time_kingsrow[100000];

	fp = fopen("C:\\Users\\Martin Fierz\\Documents\\Martin Fierz\\CheckerBoard\\games\\matches\\matchlog84.txt", "r");
	while (!feof(fp)) {
		fgets(line, 255, fp);
		if (line[0] == 'a' && (line[10] == 'v' || line[33] == 'v')) {  // kingsrow
			//printf("\n%s", line);
			sscanf(line, "analysis: value=%i, depth  %i/%f/%i, %fs", &value, &d1, &d3, &d2, &time);
			//sscanf(line, "analysis: time remaining:%fs   value=%i, depth  %i/%f/%i, %fs", &dummy, &value, &d1, &d3, &d2, &time);
			if (d1 > 10 && d1 < 50
				) {
				timesum_kr += time;
				time_kingsrow[n_kr] = time; 
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
				time_cake[n_cake] = time; 
				n_cake++;
			}
			//getch(); 
		}
	}
	fclose(fp); 
	printf("\nfound %i moves of kr, %i moves of cake", n_kr, n_cake);
	printf("\n\naverage time kingsrow: %.3f", timesum_kr / n_kr);
	printf("\naverage time cake: %.3f", timesum_cake / n_cake);

	// write data to file
	fp = fopen("C:\\code\\checkersdata\\time_cake.txt", "w");
	for (int i = 0; i < n_cake; i++)
		fprintf(fp, "%.3f\n", time_cake[i]);
	fclose(fp); 
	fp = fopen("C:\\code\\checkersdata\\time_kingsrow.txt", "w");
	for (int i = 0; i < n_cake; i++)
		fprintf(fp, "%.3f\n", time_kingsrow[i]);
	fclose(fp);

	getch();
	exit(0);
	return 0; 
}

int analyze_matchprogress(void) {
	// read match progress file
	FILE* fp;
	
	int i; 
	int ballot; 
	char res1, res2; 
	int n[2401]; 
	int wins[2401]; 
	char filename[128]; 

#define NUMFILES 37

	char files[NUMFILES][128] = { "match_progress61.txt",
							"match_progress60.txt",
							"match_progress14.txt",
							"match_progress13.txt",
							"match_progress17.txt",
							"match_progress26.txt",
							"match_progress22.txt",
							"match_progress12.txt",
							"match_progress35.txt",
							"match_progress59.txt",
"match_progress6.txt",
"match_progress53.txt",
"match_progress21.txt",
"match_progress34.txt",
"match_progress25.txt",
"match_progress37.txt",
"match_progress46.txt",
"match_progress50.txt",
"match_progress58.txt",
"match_progress41.txt",
"match_progress49.txt",
"match_progress18.txt",
"match_progress27.txt",
"match_progress54.txt",
"match_progress52.txt",
"match_progress56.txt",
"match_progress43.txt",
"match_progress66.txt",
"match_progress67.txt",
"match_progress70.txt",
"match_progress75.txt",
"match_progress83.txt",
"match_progress73.txt",
"match_progress69.txt",
"match_progress84.txt",
"match_progress82.txt",
"match_progress79.txt"
	};
	char directory[128] = "C:\\Users\\Martin Fierz\\Documents\\Martin Fierz\\CheckerBoard\\games\\matches\\";


	for (i = 0; i < 2400; i++) {
		n[i] = 0; 
		wins[i] = 0; 
	}

	for (i = 0; i < NUMFILES; i++) {
		sprintf(filename, "%s%s", directory, files[i]);
		printf("\nanalyzing %s", filename);
		fp = fopen(filename, "r");
		if (fp == NULL) {
			printf("\nfile not found %s", filename);
		}
		while (!feof(fp)) {
			fscanf(fp, "%i:%c%c", &ballot, &res1, &res2);
			//printf("\nballot %i result %c%c", ballot, res1, res2);
			n[ballot] += 2;
			if (res1 == '+' || res1 == '-')
				wins[ballot]++;
			if (res2 == '+' || res2 == '-')
				wins[ballot]++;
		}
	}
	fclose(fp);
	//printf("\nfound %i moves of kr, %i moves of cake", n_kr, n_cake);
	//printf("\n\naverage time kingsrow: %.3f", timesum_kr / n_kr);
	//printf("\naverage time cake: %.3f", timesum_cake / n_cake);

	// write data to file
	fp = fopen("C:\\code\\checkersdata\\ballot_difficulty.txt", "w");
	for (int i = 1; i <= 2400; i++)
		fprintf(fp, "%i\t%.3f\n", i, (float)wins[i]/(float)(n[i]));
	fclose(fp);
	

	getch();
	exit(0);
	return 0;
}

int FENtoPosition(char* FEN, POSITION* p)
{
	/* parses the FEN string in *FEN and places the result in p and color */
	// example FEN string:
	// W:W32,31,30,29,28,27,26,25,24,22,21:B23,12,11,10,8,7,6,5,4,3,2,1.
	// returns 1 on success, 0 on failure.
	char* token;
	char* col, * white, * black;
	char FENstring[256];
	int i;
	int number;
	int32 one = 1;
	int piece;
	int length;
	char colorchar = 'x';


	// find the full stop in the FEN string which terminates it and 
	// replace it with a 0 for termination
	length = (int)strlen(FEN);
	token = FEN;
	i = 0;
	while (token[i] != '.' && i < length)
		i++;
	token[i] = 0;

	sprintf(FENstring, "%s", FEN);

	// detect empty FEN string
	if (strcmp(FENstring, "") == 0)
		return 0;

	/* parse color ,whitestring, blackstring*/
	col = strtok(FENstring, ":");

	if (col == NULL)
		return 0;

	if (strcmp(col, "W") == 0)
		p->color = WHITE;
	else
		p->color = BLACK;

	/* parse position: get white and black strings */

	white = strtok(NULL, ":");
	if (white == NULL)
		return 0;

	// check whether this was a normal fen string (white first, then black) or vice versa.
	colorchar = white[0];
	if (colorchar == 'B' || colorchar == 'b')
	{
		black = white;
		white = strtok(NULL, ":");
		if (white == NULL)
			return 0;
		// reversed fen string
	}
	else
	{
		black = strtok(NULL, ":");
		if (black == NULL)
			return 0;
	}
	// example FEN string:
	// W:W32,31,30,29,28,27,26,25,24,22,21:B23,12,11,10,8,7,6,5,4,3,2,1.
	// skip the W and B characters.
	white++;
	black++;


	/* reset board */
	p->bm = 0;
	p->wm = 0;
	p->bk = 0;
	p->wk = 0;

	/* parse white string */
	token = strtok(white, ",");

	while (token != NULL)
	{
		/* While there are tokens in "string" */
		/* a token might be 18, or 18K */
		piece = MAN;
		if (token[0] == 'K')
		{
			token++;
			piece = KING;
		}
		number = atoi(token);
		/* ok, piece and number found, transform number to coors */
		number = SquareToBit(number);

		if (piece == MAN)
			p->wm |= one << number;
		else
			p->wk |= one << number;
		/* Get next token: */
		token = strtok(NULL, ",");
	}
	/* parse black string */
	token = strtok(black, ",");
	while (token != NULL)
	{
		/* While there are tokens in "string" */
		/* a token might be 18, or 18K */
		piece = MAN;
		if (token[0] == 'K')
		{
			piece = KING;
			token++;
		}
		number = atoi(token);
		/* ok, piece and number found, transform number to coors */
		number = SquareToBit(number);
		if (piece == MAN)
			p->bm |= one << number;
		else
			p->bk |= one << number;
		/* Get next token: */
		token = strtok(NULL, ",");
	}
	return 1;
}


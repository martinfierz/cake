	// TODO

// todo 7.3.2021
// -> integrate br531k calculation with king calculation to see what difference that makes
// -> set weights after pattern optimizer in cake so optimizer can continue seamlessly
// -> start throwing out old weights that may be useless / timeconsuming
// -> get exact static eval from Cake


// position selection should be on basis of file on disk, not of eval itself - otherwise, 
// when you run the optimizer again, it gives a different result because the eval has changed.

#include <stdlib.h> 
#include <assert.h>
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <process.h>
#include <time.h>
#include "stdafx.h"

// cake-specific includes - structs defines structures, consts defines constants,
// xxx.h defines function prototypes for xxx.c
#include "../optimizeCake.h"  // important to load first!
#include "..\\switches.h"
#include "structs.h"
#include "consts.h"
#include "..\\cake_eval.h"
#include "..\\egdb.h"


#ifdef USE_KR_DB
EGDB_DRIVER* handle;
char* out = 0;
#endif

#define SKIP 0 // 4000000

#define WHITE 1
#define BLACK 2
#define MAN 4
#define KING 8
#define FREE 16






//#undef WEIGHT_BY_NUMBER
//#undef WEIGHT_BY_EQUAL
//#define AVERAGE  // use the average version of the data


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

double sigmoid(double v, double c);
void codeoutput(int recall);

int result_translated[4] = { 0, 1, -1, 0 };
static int params[PARAMS]; // parameters to optimize
int index[4096];
int indexnum[MAXINDEX_BR531K];
int indexknum[MAXKINGINDEX]; 
int indexknum2[MAXKINGINDEX]; 
int indexknumside[MAXKINGINDEX]; 
int indexknumcenter[MAXKINGINDEX];
char DBpath[256] = "C:\\Programme\\CheckerBoard64\\db";

int active_set[PARAMS]; // specify which parameters are used in optimization.

char strs[PARAMS][128] =  {
	"man_value", "king_value", "piecedown_9", "piecedown_11", "twokingbonus_10", "twokingbonus_12",
	"exchangebias",	"backrankpower1","backrankpower2", "backrankpower3", "backrankpower4", 
	"immobile_mult", "immobile_mult_kings", "ungroundedcontact", "balancemult", "skewnessmult", "skewnessmult_eg",
	"dogholeval2", "dogholeval", "dogholemandownval",
	"mc_occupyval", "mc_attackval", "realdykeval", "greatdykeval",
	"cramp20", "nocrampval20", "cramp13", "cramp13_eg", "nocrampval13", "cramp12",
	"badstructure3", "badstructure4", "badstructure",
	"badstructure2", "badstructure5", "badstructure6", "badstructure7", "badstructure8",
	"badstructure9", "badstructure10", "badstructure11",
	"promoteinone", "runaway_destroys_backrank", "promoteintwo", "promoteinthree",
	"kingtrappedinsinglecornerval", "kingtrappedinsinglecornerbytwoval", "kingtrappedindoublecornerval",
	"king_blocks_king_and_man", "dominatedkingval", "dominatedkingval2", "dominatedkingindcval", "dominatedkingindcval2",
	"king_denied_center", "kingcentermonopoly", "onlykingval", "roamingkingval",
	"experimental_king_cramp", "keval",
	"tailhookval", "kingproximityval1", "kingproximityval2", "endangeredbridge_kingdown", "endangeredbridge",
	"kingholdstwomenval", "immobilemanval", "compensation", "compensation_mandown",
	"turnval", "turnval_eg",
	"edgepressure1", "edgepressure2",


	"ungrounded0", "ungrounded1", "ungrounded2", "ungrounded3", "ungrounded4", "ungrounded5", "ungrounded6", 
	"ungrounded7", "ungrounded8", "ungrounded9", "ungrounded10", "ungrounded11", "ungrounded12",
	
	"tmod0", "tmod1", "tmod2", "tmod3", "tmod4", "tmod5", "tmod6", "tmod7",
	"tmod8", "tmod9", "tmod10", "tmod11", "tmod12", "tmod13", "tmod14", "tmod15",
	"tmod16", "tmod17", "tmod18", "tmod19", "tmod20", "tmod21", "tmod22", "tmod23", "tmod24",
	"kmob0", "kmob1", "kmob2", "kmob3", "kmob4", "kmob5", "kmob6", "kmob7", "kmob8", "kmob9",
		"kpst0", "kpst1", "kpst2", "kpst3", "kpst4", "kpst5", "kpst6", "kpst7",
		"kpst8", "kpst9", "kpst10", "kpst11", "kpst12", "kpst13", "kpst14", "kpst15",
		"kpst16", "kpst17", "kpst18", "kpst19", "kpst20", "kpst21", "kpst22", "kpst23",
		"kpst24", "kpst25", "kpst26", "kpst27", "kpst28", "kpst29", "kpst30", "kpst31",

};




double calc_error(int n, EVALUATEDPOSITION* ep, double c);
double calc_error_MT(int n, EVALUATEDPOSITION* ep, double c);

double error_ST[30000000];
double error_MT[30000000];
int totalquietpositions = 0;
#ifdef STOCHASTIC
short int* isinactive; 
#endif

typedef struct
{
	EVALUATEDPOSITION* ep; 
	int start; 
	int end; 
	double errorsum;
	int threadID; 
	double c;
	int gamenum; 
} THREADINFO;




int main()
{
	int i = 0, n;
	FILE* fp;
	unsigned int bm, bk, wm, wk, color;
	POSITION p;
	MATERIALCOUNT mc;
	int delta = 100;
	EVALUATEDPOSITION* ep, * cur;
	double error;
	int j;
	int oldparam;
	int iterations = 0;
	double minerror;
	int position_number = 0;
	int changed = 0;
	int res_from_file;
	double c;
	int rejected = 0;
	int rejectednum = 0;
	int pos_num = 0;
	int quiet_pos_num = 0;
	int /*v0, */v1, v3, staticeval;
	int iter[3] = { 1,0,2 };
	int sameadjust;
	int adjust;
	int paramnum = 0;
	int allactive = 1;
	int initialparams[PARAMS];
	double influence[PARAMS];
	double influence0[PARAMS];
	int wins, draws, losses;
	int totalgames = 0;
	int totalpositions = 0;
	
	char FEN[256];
	FILE* log;
	double tolerance = 3e-12;		// minimal change in error that a parameter is still changed
	int usedgames = 0;
	int unusedgames = 0;

	// if I want to check search times, run analyze_matchprogress()
	//analyze_matchprogress(); 


	//create_indices(); 

	ep = malloc(sizeof(EVALUATEDPOSITION) * 30000000);


	// initialize eval
	initeval();
	//startparams();
	optimalparams();
	updateeval();

	// recall parameters from cake's evaluation
	//startparams(); 
	//optimalparams(); 
	//updateeval
#ifdef ZERO
	zeroparams();
	updateeval();
#endif
	getparams(params, &paramnum);

	printf("\nPARAMS is %i, paramnum is %i", PARAMS, paramnum);
	// print parameters to show it's working
	for (i = 0; i < paramnum; i++) {
		//printf("\nparameter[%i] is %i (%s)", i, params[i], strs[i]);
		initialparams[i] = params[i];
	}
	printf("\nfound %i parameters to optimize", paramnum);

	// open optimizer logfile, save params
	log = fopen("c:\\code\\checkersdata\\optimizerlog.txt", "w");

	//getch(); 

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
#ifdef AVERAGE
	fp = fopen("c:\\code\\checkersdata\\taggedevaluatedpositions_av.txt", "r");
#else
#ifdef NODUPLICATES
	fp = fopen("c:\\code\\checkersdata\\taggedevaluatedpositions.txt", "r");
#else
	fp = fopen("c:\\code\\checkersdata\\taggedevaluatedpositions+duplicates.txt", "r");
#endif
#endif

	// set index numbers to 0 (telling how many time a position with index appears)
	for (i = 0; i < MAXINDEX_BR531K; i++)
		indexnum[i] = 0;
	for (i = 0; i < MAXKINGINDEX; i++) {
		indexknum[i] = 0;
		indexknum2[i] = 0; 
		indexknumside[i] = 0; 
#ifdef PAT5
		indexknumcenter[i] = 0; 
#endif
		}
	
	printf("\nloading...");

	cur = ep; 
	i = 0; 
	j = 0;
	int maxindex = 0; 
	while (!feof(fp)) {
		
		// bm, bk, wm, wk, color, evaluation, result
		// evaluation is from side to move; BLACK to move is 2, WHITE to move is 1
		// result is WIN (1) if black won, LOSS (2) if white won, DRAW (0) or UNKNOWN (3)
		//fprintf(fpout, "%u %u %u %u %i %i %i %i\n", bm, bk, wm, wk, color, eval, v1, v3);
#ifdef AVERAGE
		// fprintf(fpout, "%u %u %u %u %i %i %i %i %i %i\n", bm, bk, wm, wk, color, games, points, v0, v1, v3);
		fscanf(fp, "%u %u %u %u %i %i %i %i %i %i %i\n", &bm, &bk, &wm, &wk, &color, &wins, &draws, &losses, &staticeval, &v1, &v3);


#else
		fscanf(fp, "%u %u %u %u %i %i %i %i %i\n", &bm, &bk, &wm, &wk, &color, &res_from_file, &staticeval, &v1, &v3);
#endif
		
		pos_num++;

		p.bm = bm;
		p.bk = bk; 
		p.wm = wm; 
		p.wk = wk; 
		p.color = color; 
		totalpositions += (wins + draws + losses); 

		/*if (wins + draws + losses > 100) {
			printboard(p);
			printf("\n%i wins %i draws %i losses %i static eval", wins, draws, losses, color == BLACK? staticeval:-staticeval);
			getch(); 
		}*/

		if (!testcapture(&p)) {
			p.color = p.color^CC; 
			if (!testcapture(&p)) {
				quiet_pos_num++; 
				totalquietpositions += (wins + draws + losses); 
				p.color = p.color^CC;
				cur->bm = bm;
				cur->bk = bk;
				cur->wm = wm;
				cur->wk = wk;
				cur->color = color;
				cur->value = v3;
				cur->searchvalue = v1; // value from a deeper search, currently 9 ply
#ifdef AVERAGE
				//cur->gameresult = (float)points / (float)(2 * games); 
				//cur->gamenum = games; 
				cur->wins = wins; 
				cur->draws = draws; 
				cur->losses = losses; 
#else
				cur->gameresult = res_from_file;
#endif
				
				mc.bm = bitcount(p.bm);
				mc.bk = bitcount(p.bk);
				mc.wm = bitcount(p.wm);
				mc.wk = bitcount(p.wk);
				//cur->staticeval = evaluation(&p, &mc, 0, &delta, 0, 0);
				//if (abs(cur->staticeval - cur->value) > 50) {
#ifdef REJECTNONQUIET
				if(abs(staticeval - v3) > 50) {
#ifdef AVERAGE
					rejectednum += (wins + draws + losses); 
					totalquietpositions -= (wins + draws + losses); 
					rejected++; 
#else
					rejected++;
#endif
					quiet_pos_num--;
					//printboard(p);
					//printf("\nthis position (%i) has evaluation %i but 3ply value %i", i, staticeval, v3);
					//getch();
				}
				else {
#endif
					cur++;
#ifdef AVERAGE
					totalgames += (wins + draws + losses);
#else
					totalgames++;
#endif

					int index = getindex_br531k(&p);
					if (index >= 0) {
						indexnum[index] += (wins + draws + losses);
					}

					int revindex = getreverseindex_br531k(&p);
					if (revindex >= 0) {
						indexnum[revindex] += (wins + draws + losses);
					}
					if (index <= 0 && revindex <= 0)
						unusedgames += (wins + draws + losses);
					else
						usedgames += (wins + draws + losses); 
					index = getkingindex(&p); 
					indexknum[index] += (wins + draws + losses);
					revindex = getreversekingindex(&p); 
					indexknum[revindex] += (wins + draws + losses);
					index = getkingindex2(&p); 
					indexknum2[index] += (wins + draws + losses);
					revindex = getreversekingindex2(&p);
					indexknum2[revindex] += (wins + draws + losses);
					index = getkingindexside(&p); 
					indexknumside[index] += (wins + draws + losses);
					revindex = getreversekingindexside(&p); 
					indexknumside[revindex] += (wins + draws + losses);
#ifdef PAT5
					index = getkingindexcenter(&p);
					indexknumcenter[index] += (wins + draws + losses);
					revindex = getreversekingindexcenter(&p);
					indexknumcenter[revindex] += (wins + draws + losses);
#endif
				}
				
				if (i % 10000 == 0)
					printf("\n%i", i);
				i++;
			}
		}
	}

	
	int realized = 0; 
	for (int j = 0; j < MAXINDEX_BR531K; j++) {
		if (indexnum[j] > 0)
			realized++;
	}
	printf("\nof 531441 possible 3-backrank indices, %i are actually realized (n>0)", realized);

	realized = 0;
	for (int j = 0; j < MAXINDEX_BR531K; j++) {
		if (indexnum[j] > 10)
			realized++;
	}
	printf("\nof 531441 possible 3-backrank indices, %i are actually realized (n>10)", realized);

	realized = 0;
	for (int j = 0; j < MAXINDEX_BR531K; j++) {
		if (indexnum[j] > 100)
			realized++;
	}
	printf("\nof 531441 possible 3-backrank indices, %i are actually realized (n>100)", realized);

	realized = 0;
	for (int j = 0; j < MAXINDEX_BR531K; j++) {
		if (indexnum[j] > 1000)
			realized++;
	}
	printf("\nof 531441 possible 3-backrank indices, %i are actually realized (n>1000)", realized);
	printf("\n%i used games, %i unused games for pattern optimization", usedgames, unusedgames);

	fclose(fp); 
	n = i - 1 - rejected;
	printf("\ntotal unique positions %i, total positions %i, total quiet games %i, unique quiet %i, final %i, %i were rejected (%i games)", 
		pos_num, totalpositions, totalquietpositions, quiet_pos_num, n, rejected, rejectednum);
	printf("\nevaluating...");

	
#ifdef STOCHASTIC
	isinactive = malloc(n * sizeof(short int));
	for (i = 0; i < n; i++) {
		isinactive[i] = rand() % STOCHASTICNUM;
	}
#endif



	// intermezzo: save 1000 positions as fen to a text file for move ordering
	if (0) {
		printf("\ns saving 1000 positions...");
		fp = fopen("c:\\code\\checkersdata\\1000positions.txt", "w");
		for (i = 0; i < 1000; i++) {
			j = rand() + (rand() << 15);
			j = j % n;
			printf("\npos %i", j);
			p.bm = ep[j].bm;
			p.bk = ep[j].bk;
			p.wm = ep[j].wm;
			p.wk = ep[j].wk;
			p.color = ep[i].color;
			printboard(&p);
			PositiontoFEN(&p, FEN);
			fprintf(fp, "%s", FEN);
			fflush(fp);
			if (i % 100 == 0)
				getch();
		}
		fclose(fp);
		printf("\ndone saving"); 
		getch();
		
	}

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

	double minc = 0.024;
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
	//minerror = calc_error(n, ep, c);
	

	// reset all parameters to 0
	/*for (i = 0; i < PARAMS; i++)
		params[i] = 0;
	params[man_value] = 100; 
	params[king_value] = 130; 
	params[exchangebias] = 25; */
	//startparams(); 
	optimalparams(); 
#ifdef ZERO
	zeroparams(); 
	//for (i = 0; i < PARAMS; i++)
	//	params[i] = 0; 
	setparams(params, paramnum);
	updateeval();
#endif
	getparams(params, &paramnum); 


	// to "stretch" eval
	//for (i = 0; i < PARAMS; i++)
	//	params[i] = 2 * params[i]; 

	setparams(params, paramnum);
	updateeval();
	
	getparams(params, &paramnum);
	/*for (i = 0; i < PARAMS; i++) {
		printf("\nparam %i: %i", i, params[i]); 
	}
	getch(); */
	for (i = 0; i < n; i++) 
		error_MT[i] = 0;
	minerror = calc_error_MT(n, ep, c);
	for (i = 0; i < n; i++)
		error_MT[i] = 0;
	printf("\ninitial error MT is %.10f", minerror);
	fprintf(log, "\ninitial error MT is %.10f", minerror);

	//setparams(params, paramnum);
	//updateeval();
	if (1) {
		minerror = calc_error(n, ep, c);
		printf("\nerror after setting all parameters is %.10f (ST)", minerror);
	}
	printf("\nfound %i parameters to optimize - hit key to continue", paramnum); 
	getch(); 


	// then optimize 4x4 single and double corner with kings
	patterns_setbr_to_zero();
	patterns_setpat1_to_zero(); 
	patterns_setpat2_to_zero();
	patterns_setpat3_to_zero(); 
	patterns_setpat4_to_zero(); 
#ifdef PAT5
	patterns_setpat5_to_zero();
#endif
	pattern_kings_optimizer2(n, ep, c); 
	printf("\n...*** pattern optimizer done!! ");


	if (1) {
		minerror = calc_error(n, ep, c);
		printf("\nerror after patterns is %.10f (ST)", minerror);
	}

	/*deactivate_all(); 
	active_set[man_value] = 1;
	active_set[king_value] = 1;
*/


	

	int step = 1;
	int maxadjust = 100; 

	activate_all(); 

	//minerror = 1; 
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
			while (adjust < maxadjust) {
				params[j] = params[j]+step; 
				setparams(params, paramnum); 
				updateeval(); 
				error = calc_error_MT(n, ep, c);
				influence[j] += fabs(error - minerror);
				printf(" +(%.12f)", error); 
				if (error > (minerror - tolerance)) {
					// revert
					params[j] = params[j] - step; 
					break; 
				}
				adjust++; 
				minerror = error; 
				printf(" (%i)", params[j]);
			}

			// 2. if param was not adjusted in positive direction, try negative direction
			if (!adjust) {
				while (adjust < maxadjust) {
					params[j] -= step;
					setparams(params, paramnum);
					updateeval();
					error = calc_error_MT(n, ep, c);
					printf(" -(%.12f)", error);
					influence[j] += fabs(error - minerror);
					if (error > (minerror - tolerance)) {
						// revert
						params[j]+= step;
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
		codeoutput(0);
		iterations++; 

		if (changed == 0 && allactive)
			break; 
		if (changed == 0) {
			printf("\nno changes on this iteration, but not all active - reactivating all");
			fprintf(log, "\nno changes on this iteration, but not all active - reactivating all");
			allactive = 1;
			activate_all();
#ifdef STOCHASTIC
			for (i = 0; i < n; i++) {
				isinactive[i] = rand() % STOCHASTICNUM;
				error_MT[i] = 0;
				}
			minerror = calc_error_MT(n, ep, c);
			printf("\nnew initial error MT is %.10f", minerror);
			fprintf(log, "\nnew initial error MT is %.10f", minerror);
#endif
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
		error = calc_error_MT(n, ep, c);
		influence0[i] = error - minerror; 
		params[i] = oldparam;
	}

	fprintf(log, "\ninitial\toptimized");
	fp = fopen("C:\\code\\checkersdata\\param_influence.txt", "w");
	for (i = 0; i < paramnum; i++) {
		printf("\n%i:\t%i\t(%s)  (+-%.5f, +-%.5f)", i, params[i], strs[i], 1000.0 * influence[i], 1000.0 * influence0[i]);
		fprintf(fp, "%i\t%s\t%i\t%.5f\t%.5f\n", i, strs[i], params[i], 1000.0* influence[i], 1000.0* influence0[i]);
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

	/*
	deactivate_all(); 
	
	
	//active_set[edgepressure1] = 1; 
	return 0; 
	for (int i = 0; i < 7; i++)
		active_set[i] = 1; 

	for (int i = 0; i < BRNUM; i++) {
		active_set[arraystart + 13 + i] = 1;
	}
		//return 0; */

	for (int i = 0; i < PARAMS; i++)
		active_set[i] = 1;
	return 0; 
}

int deactivate_all() {
	for (int i = 0; i < PARAMS; i++)
		active_set[i] = 0;
	return 0; 
}

void codeoutput(int recall) {
	// write parameters as C code to file
	FILE* fp;
	int i; 
	int paramnum = 0;

	if (recall != 0) {
		getparams(params, &paramnum);
		fp = fopen("C:\\code\\checkersdata\\codeoutputrecalled.txt", "w");
	}
	else {
		fp = fopen("C:\\code\\checkersdata\\codeoutput.txt", "w");
		paramnum = PARAMS; 
	}
	
	for (i = 0; i < paramnum - 13 - 25 - 10 - 32; i++) {
		fprintf(fp, "\nv[%s] = %i;", strs[i], params[i]);
	}

	fprintf(fp, "\n\nstatic int ungroundedpenalty[13] = {");
	for (i = paramnum - 13 - 25  - 10 - 32 ; i < paramnum - 25  - 10 - 32 ; i++) {
		fprintf(fp, " %i,", params[i]);
	}
	fprintf(fp, "};");

	fprintf(fp, "\nstatic int tmod[25] = {");
	for (i = paramnum - 25-10-32; i < paramnum-10-32; i++) {
		fprintf(fp, " %i,", params[i]);
	}
	fprintf(fp, "};");

	fprintf(fp, "\nstatic int kingmobility[10] = {");
	for (i = paramnum - 10-32; i < paramnum-32; i++) {
		fprintf(fp, " %i,", params[i]);
	}
	fprintf(fp, "};");

	fprintf(fp, "\nstatic int king_pst[32] = {");
	for (i = paramnum - 32; i < paramnum; i++) {
		fprintf(fp, " %i,", params[i]);
	}
	fprintf(fp, "};");

	fclose(fp);
}

unsigned __stdcall subThread(void* pArguments)
{
	THREADINFO* threadinfo; 
	int i = 0; 
	
	threadinfo = (THREADINFO*)pArguments; 
	double c = threadinfo->c;
	POSITION p;
	MATERIALCOUNT mc;
	int staticeval;
	int delta = 0;
	double res = 0, error = 0, errorsum = 0;
	int num = 0;
	double sum;
	double s;
	int games; 

	//printf("In sub thread[%i]: %i...%i\n", threadinfo->threadID, threadinfo->start, threadinfo->end);

	threadinfo->errorsum = 0; 
	for (i = threadinfo->start; i < threadinfo->end; i++) {
#ifdef STOCHASTIC
		if (isinactive[i])
			continue; 
#endif
		p.bm = threadinfo->ep[i].bm;
		p.bk = threadinfo->ep[i].bk;
		p.wm = threadinfo->ep[i].wm;
		p.wk = threadinfo->ep[i].wk;
		p.color = threadinfo->ep[i].color;

		mc.bm = bitcount(p.bm);
		mc.bk = bitcount(p.bk);
		mc.wm = bitcount(p.wm);
		mc.wk = bitcount(p.wk);
		staticeval = evaluation(&p, &mc, 0, &delta, 0, 0);
#ifdef AVERAGE
		//num += threadinfo->ep[i].gamenum;

		games = threadinfo->ep[i].wins + threadinfo->ep[i].draws + threadinfo->ep[i].losses; 
		num += games;

		//printf("\n%i wins, %i draws, %i losses", threadinfo->ep[i].wins, threadinfo->ep[i].draws, threadinfo->ep[i].losses);
#else
		num++;
#endif
		//errorsum += abs(staticeval - ep[i].value);

		//printboard(&p); 
		//printf("\nstatic eval is %i", staticeval); 
		//getch(); 

		// logistic optimization
		// staticeval was computed as side to move. normalize to black to move
		if (threadinfo->ep[i].color == WHITE)
			staticeval = -staticeval; // now staticeval is as seen from black's point of view
#ifdef AVERAGE
#ifdef OPTIMIZE_SEARCHEVAL
		res = threadinfo->ep[i].searchvalue;
		if (threadinfo->ep[i].color == WHITE)
			res = -res; // now staticeval is as seen from black's point of view
		res = sigmoid(res, c);
		error = (res - sigmoid(staticeval, c));
		error = error * error;
		error *= (threadinfo->ep[i].wins + threadinfo->ep[i].draws + threadinfo->ep[i].losses); 
		threadinfo->errorsum += error; 
		error_MT[i] = error; 
#else
		s = sigmoid(staticeval, c); 
		error = (1 - s);
		error = error * error; 
		sum = error * threadinfo->ep[i].wins; 
		
		error = (0 - s);
		error = error * error;
		sum += error * threadinfo->ep[i].draws;
		
		error = (-1 - s);
		error = error * error;
		sum += error * threadinfo->ep[i].losses;
		threadinfo->errorsum  += sum;
		error_MT[i] = sum;
#endif

#else
		res = result_translated[threadinfo->ep[i].gameresult];

#ifdef OPTIMIZE_SEARCHEVAL
		res = threadinfo->ep[i].searchvalue;
		if (threadinfo->ep[i].color == WHITE)
			res = -res; // now staticeval is as seen from black's point of view
		res = sigmoid(res, c);
#endif
		// res is game result as seen from black's point of view (1 = win, 0 = draw, -1 = loss)
		error = (res - sigmoid(staticeval, c));

		error = error * error;
		printf("\nerror is %f", error);
		errorsum += error;
		error_MT[i] = error;
#endif

		//printboard(&p);
		//printf("\nstatic eval, search eval: %i %i", staticeval, eval);
		//getch();
	}
	//errorsum = errorsum / (float)num;
	//printf("\n***** %.6f",errorsum);

	//printf("\nDone sub thread[%i]: errorsum is %.7f", threadinfo->threadID, errorsum/(float)num);
	//printf("\ncalc MT[%i]: errorsum is %.2f, number is %i", threadinfo->threadID,errorsum, num);

	//threadinfo->errorsum = errorsum; 
	threadinfo->gamenum = num; 
	_endthreadex(0);
	return 0;
}

double calc_error_MT(int n, EVALUATEDPOSITION* ep, double c) {
	//int calc_error_MT() {
	// main function to call to run the calculation of the error in a multithreaded mode
	HANDLE hThread[4];
	unsigned threadID[4];
	int counter[4]; 
	THREADINFO threadinfo[4]; 
	double error;

	//printf("Creating subthreads...\n");

	threadinfo[0].ep = ep; 
	threadinfo[0].errorsum = 0; 
	threadinfo[0].threadID = 0; 
	threadinfo[0].start = 0; 
	threadinfo[0].end = n/4 ; 
	threadinfo[0].c = c; 
	threadinfo[0].gamenum = 0; 

	threadinfo[1].ep = ep;
	threadinfo[1].errorsum = 0;
	threadinfo[1].threadID = 1;
	threadinfo[1].start = n/4;
	threadinfo[1].end = n/2;
	threadinfo[1].c = c;
	threadinfo[1].gamenum = 0; 

	threadinfo[2].ep = ep;
	threadinfo[2].errorsum = 0;
	threadinfo[2].threadID = 2;
	threadinfo[2].start = n / 2;
	threadinfo[2].end = (3*n)/4;
	threadinfo[2].c = c;
	threadinfo[2].gamenum = 0;

	threadinfo[3].ep = ep;
	threadinfo[3].errorsum = 0;
	threadinfo[3].threadID = 3;
	threadinfo[3].start = (3 * n) / 4;
	threadinfo[3].end = n;
	threadinfo[3].c = c;
	threadinfo[3].gamenum = 0;

	// Create the second thread.
	// parameter 4 would be a pointer to an argument list = I could pass a structure there which would 
	// contain information to communicate with the thread
	hThread[0] = (HANDLE)_beginthreadex(NULL, 0, &subThread, &(threadinfo[0]), 0, &(threadID[0]));
	hThread[1] = (HANDLE)_beginthreadex(NULL, 0, &subThread, &(threadinfo[1]), 0, &(threadID[1]));
	hThread[2] = (HANDLE)_beginthreadex(NULL, 0, &subThread, &(threadinfo[2]), 0, &(threadID[2]));
	hThread[3] = (HANDLE)_beginthreadex(NULL, 0, &subThread, &(threadinfo[3]), 0, &(threadID[3]));
	// Wait until second thread terminates. If you comment out the line
	// below, Counter will not be correct because the thread has not
	// terminated, and Counter most likely has not been incremented to
	// 1000000 yet.
	//WaitForSingleObject(hThread[0], INFINITE);
	//WaitForSingleObject(hThread[1], INFINITE);
	WaitForMultipleObjects(4, hThread, TRUE, INFINITE);

	//     WaitForMultipleObjects(MAX_THREADS, hThreadArray, TRUE, INFINITE);

	//printf("Counters are-> %d %d\n", counter[0], counter[1]);
	// Destroy the thread object.
	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	CloseHandle(hThread[2]);
	CloseHandle(hThread[3]);

	//printf("\nerrorsum 1 is %.7f, errorsum2 is %.7f", threadinfo[0].errorsum, threadinfo[1].errorsum);

	//error = threadinfo[0].errorsum + threadinfo[1].errorsum; 
	//error = error / (threadinfo[0].gamenum + threadinfo[1].gamenum);

	// now use the stored error info
	double errorsum = 0;
	for (int i = 0; i < n; i++) {
		errorsum += error_MT[i]; 
	}
	error = errorsum / (threadinfo[0].gamenum + threadinfo[1].gamenum + threadinfo[2].gamenum + threadinfo[3].gamenum);
	//error = errorsum / n;
	//printf("\nusing error array: ");
	//printf("\ntotal error %.2f", errorsum);
	//printf("\nnumber of games %i", n);
	//printf("\ngiving av. error of %.10f", error);

	/*printf("\n\nusing threads:");
	error = threadinfo[0].errorsum + threadinfo[1].errorsum + threadinfo[2].errorsum + threadinfo[3].errorsum; 
	printf("\ntotal error from 4 threads is %.2f", error);
	printf("\nnumber of games %i", threadinfo[0].gamenum + threadinfo[1].gamenum + threadinfo[2].gamenum + threadinfo[3].gamenum);
	error = error / (threadinfo[0].gamenum + threadinfo[1].gamenum + threadinfo[2].gamenum + threadinfo[3].gamenum); 
	printf("\naverage error of %.10f", error);*/
	return error; 
}


int pattern_kings_optimizer2(int n, EVALUATEDPOSITION* ep, double c) {
	// here's what the pattern optimizer has to do:
	int i;
	POSITION p;
	MATERIALCOUNT mc;
	int delta;
	int* staticeval;
	double* changeplus;
	double* changeminus;
	double* weights;
	double* changeplus2;
	double* changeminus2;
	double* weights2;
	// next 3 lines new for br
	double* weightsbr531k;
	double* changeplusbr531k; 
	double* changeminusbr531k; 
	// next 3 lines for 4k br
	double* weightsbr4k;
	double* changeplusbr4k; 
	double* changeminusbr4k; 
	// next 3 lines for side king pattern
	double* changeplusside;
	double* changeminusside;
	double* weightsside;
	// next 3 lines for king center pattern
#ifdef PAT5
	double* changepluscenter;
	double* changeminuscenter;
	double* weightscenter;
#endif
	

	//if(p.color == BLACK)

	double error;
	//double errormodified;
	double sum;
	double starttime; 
	int adjustplus = 1, adjustminus = 1;
	int adjustplus2 = 1, adjustminus2 = 1; 
	int adjustplusbr531k = 1, adjustminusbr531k = 1; 
	int adjustplusbr4k = 1, adjustminusbr4k = 1; 
	int adjustplusside = 1, adjustminusside = 1; 
#ifdef PAT5
	int adjustpluscenter = 1, adjustminuscenter = 1;
#endif
	int pass = 1;

	int num=0; 
	double errorsum; 
	//int res;

	staticeval = malloc(n * sizeof(int));

	changeplus = malloc(MAXKINGINDEX * sizeof(double));
	changeminus = malloc(MAXKINGINDEX * sizeof(double));
	weights = malloc(MAXKINGINDEX * sizeof(double));

	changeplus2 = malloc(MAXKINGINDEX * sizeof(double));
	changeminus2 = malloc(MAXKINGINDEX * sizeof(double));
	weights2 = malloc(MAXKINGINDEX * sizeof(double));

	changeplusside = malloc(MAXKINGINDEX * sizeof(double));
	changeminusside = malloc(MAXKINGINDEX * sizeof(double));
	weightsside = malloc(MAXKINGINDEX * sizeof(double));

#ifdef PAT5
	changepluscenter = malloc(MAXKINGINDEX * sizeof(double));
	changeminuscenter = malloc(MAXKINGINDEX * sizeof(double));
	weightscenter = malloc(MAXKINGINDEX * sizeof(double));
#endif

	changeplusbr531k = malloc(MAXINDEX_BR531K * sizeof(double));
	changeminusbr531k = malloc(MAXINDEX_BR531K * sizeof(double));
	weightsbr531k = malloc(MAXINDEX_BR531K * sizeof(double));

	changeplusbr4k = malloc(MAXINDEX_BR4K * sizeof(double));
	changeminusbr4k = malloc(MAXINDEX_BR4K * sizeof(double));
	weightsbr4k = malloc(MAXINDEX_BR4K * sizeof(double));


	if (staticeval == 0 || changeplus == 0 || changeminus == 0 || weights == 0
		|| changeplus2 == 0 || changeminus2 == 0 || weights2 == 0 ||
		changeplusbr531k == 0 || changeminusbr531k == 0 || weightsbr531k == 0 ||
		changeplusbr4k == 0 || changeminusbr4k == 0 || weightsbr4k == 0 ||
		changeplusside == 0 || changeminusside == 0 || weightsside == 0 
#ifdef PAT5
		||
		changepluscenter == 0 || changeminuscenter == 0 || weightscenter == 0
#endif
		)
		{
		printf("\nnull pointer in pattern optimizer!");
		return 0;
	}

	for (i = 0; i < MAXKINGINDEX; i++) {
		weights[i] = 0;
		weights2[i] = 0; 
		weightsside[i] = 0; 
#ifdef PAT5
		weightscenter[i] = 0; 
#endif
	}
	for (i = 0; i < MAXINDEX_BR531K; i++)
		weightsbr531k[i] = 0; 
	for (i = 0; i < MAXINDEX_BR4K; i++)
		weightsbr4k[i] = 0;


	// 1. calculate current eval of each position without changes and save in an array of size N
	printf("\npattern king optimizer: getting initial eval for %i positions....", n);
	for (i = 0; i < n; i++) {
		p.bm = ep[i].bm;
		p.bk = ep[i].bk;
		p.wm = ep[i].wm;
		p.wk = ep[i].wk;
		p.color = ep[i].color;
		// calculate number of positions once in init, then use it later
		num += (ep[i].wins + ep[i].draws + ep[i].losses);

		mc.bm = bitcount(p.bm);
		mc.bk = bitcount(p.bk);
		mc.wm = bitcount(p.wm);
		mc.wk = bitcount(p.wk);
		staticeval[i] = evaluation(&p, &mc, 0, &delta, 0, 0);
		if (ep[i].color == WHITE)
			staticeval[i] = -staticeval[i]; // now staticeval is as seen from black's point of view
	}
	printf(" done!");

	starttime = clock(); 
	
	
	while (adjustplus + adjustminus && (pass </*2002*/ 2002)) {
		// 2. create two arrays "changeplus[531441]" and "changeminus[531441]", and set them all to 0

		// todo: use memset instead?
		for (i = 0; i < MAXKINGINDEX; i++) {
			changeplus[i] = 0;
			changeminus[i] = 0;
			changeplus2[i] = 0; 
			changeminus2[i] = 0; 
			changeplusside[i] = 0; 
			changeminusside[i] = 0; 
#ifdef PAT5
			changepluscenter[i] = 0;
			changeminuscenter[i] = 0;
#endif
		}
		for (i = 0; i < MAXINDEX_BR531K; i++) {
			changeplusbr531k[i] = 0;
			changeminusbr531k[i] = 0;
		}
		for (i = 0; i < MAXINDEX_BR4K; i++) {
			changeplusbr4k[i] = 0;
			changeminusbr4k[i] = 0;
		}

		errorsum = 0; 
		// 3. loop over all N positions
		for (i = 0; i < n; i++) {
			p.bm = ep[i].bm;
			p.bk = ep[i].bk;
			p.wm = ep[i].wm;
			p.wk = ep[i].wk;
			p.color = ep[i].color;

			//    4.calculate index of this position
			int index = indexk_get_magic(&p);
			int reverseindex = indexk_reverse_get_magic(&p);  
			int index2 = indexk2_get_magic(&p); 
			int reverseindex2 = indexk2_reverse_get_magic(&p); 
			int indexside = indexk_side_get_magic(&p); 
			int reverseindexside = indexk_side_reverse_get_magic(&p); 
#ifdef PAT5
			int indexcenter = indexk_center_get_magic(&p);
			int reverseindexcenter = indexk_center_reverse_get_magic(&p);
#endif
			int indexbr = index_get(&p); 
			int reverseindexbr = index_reverse_get(&p); 

#ifdef BR4
			int indexbr4k = (p.bm & 0xFFFF);
			int reverseindexbr4k = fastrev16((p.wm >> 16));
#else
			int indexbr4k = (p.bm & 0xFFF);
			int reverseindexbr4k = fastrev12((p.wm >> 20));
#endif



			double w = 0; 
			double res = 0; 

			//    5.adjust weight[index] by +1, evaluate position (actually, just add weight[index]+1 to the eval! no need to really evaluate!)
			//    6.calculate difference of sigmoid function for old and new eval  (sigmoid(old) - sigmoid(new))
			//    7.add this difference to changeplus[index]
			//    8.adjust weight[index]  by -1, evaluate position (actually, just add weight[index]-1 to the eval! no need to really evaluate!)
			//	  9.calculate difference of sigmoid function for old and new eval
			//    10.add this difference to changeminus[index]
			
				/*if (abs(weights[index]) > 30 && (indexknum[index] > 100) && ((pass % 100) == 0)) {
					printboard(&p);
					printf("\nindex is %i, weight is %.1f, static eval %i, games %i, %i (W%i D%i L%i)", index, weights[index], staticeval[i], indexknum[index], ep[i].wins + ep[i].draws + ep[i].losses, ep[i].wins, ep[i].draws, ep[i].losses);
				}
				if (abs(weights2[index2]) > 30 && (indexknum2[index2] > 100) && ((pass % 100) == 0)) {
					printboard(&p);
					printf("\nDC index is %i, weight is %.1f, static eval %i, games %i, %i (W%i D%i L%i)", index, weights2[index2], staticeval[i], indexknum2[index2], ep[i].wins + ep[i].draws + ep[i].losses, ep[i].wins, ep[i].draws, ep[i].losses);
				}*/

				w = weights[index] - weights[reverseindex] +weights2[index2] - weights2[reverseindex2];
				w += weightsside[indexside] - weightsside[reverseindexside]; 
#ifdef PAT5
				w += weightscenter[indexcenter] - weightscenter[reverseindexcenter];
#endif
				w += weightsbr531k[indexbr]; 
				w -= weightsbr531k[reverseindexbr]; 
				w += weightsbr4k[indexbr4k]; 
				w -= weightsbr4k[reverseindexbr4k]; 

#ifdef OPTIMIZE_SEARCHEVAL

				res = ep[i].searchvalue;
				if (ep[i].color == WHITE)
					res = -res; // now staticeval is as seen from black's point of view
				res = sigmoid(res, c);
				error = (res - sigmoid((double)staticeval[i] + w, c));
				error = error * error;
				error *= (ep[i].wins + ep[i].draws + ep[i].losses);
				errorsum += error;
				sum = error; 
				//error_ST[i] = error;
				//num += (ep[i].wins + ep[i].draws + ep[i].losses);

#else
				error = (1.0 - sigmoid((double)staticeval[i] + w, c));
				error = error * error;
				sum = error * ep[i].wins;

				error = (0.0 - sigmoid((double)staticeval[i] + w, c));
				error = error * error;
				sum += error * ep[i].draws;

				error = (-1.0 - sigmoid((double)staticeval[i] + w, c));
				error = error * error;
				sum += error * ep[i].losses;
				errorsum += sum;
#endif

				//num += (ep[i].wins + ep[i].draws + ep[i].losses);

				changeplus[index] += sum;
				changeminus[index] += sum;
				changeplus[reverseindex] += sum;
				changeminus[reverseindex] += sum;
				changeplus2[index2] += sum; 
				changeminus2[index2] += sum; 
				changeplus2[reverseindex2] += sum; 
				changeminus2[reverseindex2] += sum; 
				changeplusbr531k[indexbr] += sum; 
				changeminusbr531k[indexbr] += sum; 
				changeplusbr531k[reverseindexbr] += sum; 
				changeminusbr531k[reverseindexbr] += sum; 
				changeplusbr4k[indexbr4k] += sum;
				changeminusbr4k[indexbr4k] += sum;
				changeplusbr4k[reverseindexbr4k] += sum;
				changeminusbr4k[reverseindexbr4k] += sum;
				changeplusside[indexside] += sum; 
				changeminusside[indexside] += sum; 
				changeplusside[reverseindexside] += sum; 
				changeminusside[reverseindexside] += sum; 

#ifdef PAT5
				changepluscenter[indexcenter] += sum;
				changeminuscenter[indexcenter] += sum;
				changepluscenter[reverseindexcenter] += sum;
				changeminuscenter[reverseindexcenter] += sum;
#endif


#ifdef OPTIMIZE_SEARCHEVAL

				error = (res - sigmoid((double)staticeval[i] + w + 0.1, c));
				error = error * error;
				error *= (ep[i].wins + ep[i].draws + ep[i].losses);
				sum = error;
				//error_ST[i] = error;
				//num += (ep[i].wins + ep[i].draws + ep[i].losses);

#else
				error = (1.0 - sigmoid((double)staticeval[i] + w + 0.1, c));
				error = error * error;
				sum = error * ep[i].wins;

				error = (0.0 - sigmoid((double)staticeval[i] + w + 0.1, c));
				error = error * error;
				sum += error * ep[i].draws;

				error = (-1.0 - sigmoid((double)staticeval[i] + w + 0.1, c));
				error = error * error;
				sum += error * ep[i].losses;
#endif

				changeplus[index] -= sum;
				changeminus[reverseindex] -= sum;
				changeplus2[index2] -= sum; 
				changeminus2[reverseindex2] -= sum; 
				changeplusside[indexside] -= sum; 
				changeminusside[reverseindexside] -= sum; 

#ifdef PAT5
				changepluscenter[indexcenter] -= sum;
				changeminuscenter[reverseindexcenter] -= sum;
#endif
				changeplusbr531k[indexbr] -= sum; 
				changeminusbr531k[reverseindexbr] -= sum; 
				changeplusbr4k[indexbr4k] -= sum;
				changeminusbr4k[reverseindexbr4k] -= sum;

#ifdef OPTIMIZE_SEARCHEVAL


				error = (res - sigmoid((double)staticeval[i] + w -0.1, c));
				error = error * error;
				error *= (ep[i].wins + ep[i].draws + ep[i].losses);
				sum = error;
				//error_ST[i] = error;
				//num += (ep[i].wins + ep[i].draws + ep[i].losses);

#else

				error = (1.0 - sigmoid((double)staticeval[i] + w - 0.1, c));
				error = error * error;
				sum = error * ep[i].wins;

				error = (0.0 - sigmoid((double)staticeval[i] + w - 0.1, c));
				error = error * error;
				sum += error * ep[i].draws;

				error = (-1.0 - sigmoid((double)staticeval[i] + w - 0.1, c));
				error = error * error;
				sum += error * ep[i].losses;

#endif
				changeminus[index] -= sum;
				changeplus[reverseindex] -= sum;
				changeminus2[index2] -= sum; 
				changeplus2[reverseindex2] -= sum; 
				changeminusside[indexside] -= sum; 
				changeplusside[reverseindexside] -= sum; 
#ifdef PAT5
				changeminuscenter[indexcenter] -= sum;
				changepluscenter[reverseindexcenter] -= sum;
#endif
				changeminusbr531k[indexbr] -= sum; 
				changeplusbr531k[reverseindexbr] -= sum; 
				changeminusbr4k[indexbr4k] -= sum;
				changeplusbr4k[reverseindexbr4k] -= sum;
		}
		errorsum = errorsum / (double)num;
		printf("%.9f", errorsum);

		adjustplus = 0;
		adjustminus = 0;
		adjustplus2 = 0; 
		adjustminus2 = 0; 
		adjustplusbr531k = 0; 
		adjustminusbr531k = 0; 
		adjustplusbr4k = 0;
		adjustminusbr4k = 0;
		adjustplusside = 0; 
		adjustminusside = 0; 
#ifdef PAT5
		adjustpluscenter = 0;
		adjustminuscenter = 0;
#endif
		// find index with greatest change
		double maxchange = 0;
		for (i = 0; i < MAXKINGINDEX; i++) {
			if (changeplus[i] > maxchange)
				maxchange = changeplus[i];
			if (changeminus[i] > maxchange)
				maxchange = changeminus[i];
		}
		printf("  KSC %.3f", maxchange);
		
		maxchange = 0; 
		for (i = 0; i < MAXKINGINDEX; i++) {
			if (changeplus2[i] > maxchange)
				maxchange = changeplus2[i]; 
			if (changeminus2[i] > maxchange)
				maxchange = changeminus2[i]; 
		}
		printf("  KDC %.3f", maxchange);

		maxchange = 0;
		for (i = 0; i < MAXKINGINDEX; i++) {
			if (changeplusside[i] > maxchange)
				maxchange = changeplusside[i];
			if (changeminusside[i] > maxchange)
				maxchange = changeminusside[i];
		}
		printf("  KS %.3f", maxchange);
#ifdef PAT5
		maxchange = 0;
		for (i = 0; i < MAXKINGINDEX; i++) {
			if (changepluscenter[i] > maxchange)
				maxchange = changepluscenter[i];
			if (changeminuscenter[i] > maxchange)
				maxchange = changeminuscenter[i];
		}
#endif
		//printf("   king center %.3f", maxchange);
		
		maxchange = 0;
		for (i = 0; i < MAXINDEX_BR531K; i++) {
			if (changeplusbr531k[i] > maxchange)
				maxchange = changeplusbr531k[i]; 
			if (changeminusbr531k[i] > maxchange)
				maxchange = changeminusbr531k[i]; 
		}
		printf("  531kbr %.3f", maxchange);
		
		double maxchange2 = 0;
		for (i = 0; i < MAXINDEX_BR4K; i++) {
			if (changeplusbr4k[i] > maxchange2)
				maxchange2 = changeplusbr4k[i];
			if (changeminusbr4k[i] > maxchange2)
				maxchange2 = changeminusbr4k[i];
		}
		printf("  4kbr %.3f", maxchange2);



		for (i = 0; i < MAXKINGINDEX; i++) {
			
			// king single corner
			double mult = 0.06; // 0.08;  //0.1
			mult *= 2e7;
			mult /= totalquietpositions; 
#ifdef DUALCOLOR
			mult *= 1.6;
#endif
#ifdef DUALCOLOR2
			mult *= 0.9;
#endif
#ifdef BOOST
			if (pass > 100)
				mult *= 2; 
#endif
#ifdef BOOST2
			if (pass > 200)
				mult *= 1.5;
#endif
			if (changeplus[i] > 0 && (abs(weights[i]) < 5 * sqrt(indexknum[i]))) {
				adjustplus++;
				// Ed's suggestion for regularization:
				//weights[i] *= 0.9999;// 0.998;
				weights[i] += mult * changeplus[i];
			}
			if (changeminus[i] > 0 && (abs(weights[i]) < 5 * sqrt(indexknum[i]))) {
				adjustminus++;
				weights[i] -= mult * changeminus[i];
			}


			// king double corner and side
			mult =  0.055;  // 0.09
			mult *= 2e7;
			mult /= totalquietpositions;
#ifdef DUALCOLOR
			mult *= 1.6;
#endif
#ifdef DUALCOLOR2
			mult *= 0.9;
#endif

#ifdef BOOST
			if (pass > 100)
				mult *= 2;
#endif
#ifdef BOOST2
			if (pass > 200)
				mult *= 1.5;
#endif
			if (changeplus2[i] > 0 && (abs(weights2[i]) < 5 * sqrt(indexknum2[i]))) {
				adjustplus2++;
				weights2[i] += mult * changeplus2[i];
			}
			if (changeminus2[i] > 0 && (abs(weights2[i]) < 5 * sqrt(indexknum2[i]))) {
				adjustminus2++;
				weights2[i] -= mult * changeminus2[i];
			}

			if (changeplusside[i] > 0 && (abs(weightsside[i]) < 5 * sqrt(indexknumside[i]))) {
				adjustplusside++;
				// Ed's suggestion for regularization:
				//weights[i] *= 0.9999;// 0.998;
				weightsside[i] += mult * changeplusside[i];
			}
			if (changeminusside[i] > 0 && (abs(weightsside[i]) < 5 * sqrt(indexknumside[i]))) {
				adjustminusside++;
				weightsside[i] -= mult * changeminusside[i];
			}
#ifdef PAT5
			if (changepluscenter[i] > 0 && (abs(weightscenter[i]) < 5 * sqrt(indexknumcenter[i]))) {
				adjustpluscenter++;
				// Ed's suggestion for regularization:
				//weights[i] *= 0.9999;// 0.998;
				weightscenter[i] += mult * changepluscenter[i];
			}
			if (changeminuscenter[i] > 0 && (abs(weightscenter[i]) < 5 * sqrt(indexknumcenter[i]))) {
				adjustminuscenter++;
				weightscenter[i] -= mult * changeminuscenter[i];
			}
#endif
		}

		for (i = 0; i < MAXINDEX_BR531K; i++) {
			// example: if a pattern shows up 25 times, it is limited to value 25
			// for 100 times, limited to 50
			// for 400 times, to 100
			// if a pattern is very rare, it only has a tiny change, and hardly gets anywhere
			double mult = 0.21; // 0.2;
			mult *= 2e7;
			mult /= totalquietpositions;
#ifdef DUALCOLOR2
			//mult *= 1.2;
#endif

#ifdef BOOST
			if (pass > 100)
				mult *= 2;
#endif
#ifdef BOOST2
			if (pass > 200)
				mult *= 1.5;
#endif
			if (changeplusbr531k[i] > 0 && (abs(weightsbr531k[i]) < 5 * sqrt(indexnum[i]))) {
				adjustplusbr531k++;
				weightsbr531k[i] += mult * changeplusbr531k[i];
			}
			if (changeminusbr531k[i] > 0 && (abs(weightsbr531k[i]) < 5 * sqrt(indexnum[i]))) {
				adjustminusbr531k++;
				weightsbr531k[i] -= mult * changeminusbr531k[i];
			}
		}

		for (i = 0; i < MAXINDEX_BR4K; i++) {
			double mult = 0.15;// 0.15;
			mult *= 2e7;
			mult /= totalquietpositions;
//#ifdef DUALCOLOR
//			mult *= 1.6;
//#endif

#ifdef BOOST
			if (pass > 100)
				mult *= 2;
#endif
#ifdef BOOST2
			if (pass > 200)
				mult *= 1.5;
#endif
			if (changeplusbr4k[i] > 0) {
				adjustplusbr4k++;
				weightsbr4k[i] += mult * changeplusbr4k[i];
			}
			if (changeminusbr4k[i] > 0 ) {
				adjustminusbr4k++;
				weightsbr4k[i] -= mult * changeminusbr4k[i];
			}
		}

		//printf("\npattern king optimizer: pass %i done %.2fs,  %i/%i/%i/%i/%i/%i adjust +, %i/%i/%i/%i/%i/%i  -!", pass, ((clock() - starttime) / CLK_TCK),
		//	adjustplus, adjustplus2, adjustplusside, adjustpluscenter, adjustplusbr531k, adjustplusbr4k,
		//	adjustminus, adjustminus2, adjustminusside, adjustminuscenter, adjustminusbr531k, adjustminusbr4k);
		printf("\n%i, %.2f, ", pass, ((clock() - starttime) / CLK_TCK));
		pass++;

	}
	// 12. after loop is done, loop over all weights (531441 of them)
	//    13. if changeplus[index] is positive, add one to weight[index]
	//    14. else if changeminus[index] is positive, subract one from weight[index]
	// 15. if any changes were made, go back to step 3.

	// write a file with pattern weights but better:
	// write parameters as C code to file
	FILE* fp;
	fp = fopen("C:\\code\\checkersdata\\patternoutput_rounded.txt", "w");

	int nonzero = 0;
	int w;
	for (i = 0; i < MAXINDEX_BR531K; i++) {
		w = round(2 * weightsbr531k[i]);
		if (w != 0) {
			fprintf(fp, "pat1[%i] = %i;\n", i, w);  // cake eval divides weights by two, so write 2*weights
			nonzero++;
		}
	}
	printf("\npat1: %i weights of %i are nonzero", nonzero, MAXINDEX_BR531K);

	nonzero = 0; 
	for (i = 0; i < MAXKINGINDEX; i++) {
		w = round(2 * weights[i]);
		if (w != 0) {
			fprintf(fp, "pat2[%i] = %i;\n", i, w);  // cake eval divides weights by two, so write 2*weights
			nonzero++;
		}
	}
	printf("\npat2: %i weights of %i are nonzero", nonzero, MAXKINGINDEX);

	nonzero = 0;
	for (i = 0; i < MAXKINGINDEX; i++) {
		w = round(2 * weights2[i]);
		if (w != 0) {
			fprintf(fp, "pat3[%i] = %i;\n", i, w);  // cake eval divides weights by two, so write 2*weights
			nonzero++;
		}
	}

	printf("\npat3: %i weights of %i are nonzero - hit key to continue", nonzero, MAXKINGINDEX);
	
	

	nonzero = 0;
	for (i = 0; i < MAXKINGINDEX; i++) {
		w = round(2 * weightsside[i]);
		if (w != 0) {
			fprintf(fp, "pat4[%i] = %i;\n", i, w);  // cake eval divides weights by two, so write 2*weights
			nonzero++;
		}
	}
	printf("\npat4: %i weights of %i are nonzero", nonzero, MAXKINGINDEX);
#ifdef PAT5
	nonzero = 0;
	for (i = 0; i < MAXKINGINDEX; i++) {
		w = round(2 * weightscenter[i]);
		if (w != 0) {
			fprintf(fp, "pat5[%i] = %i;\n", i, w);  // cake eval divides weights by two, so write 2*weights
			nonzero++;
		}
	}
	printf("\npat5: %i weights of %i are nonzero", nonzero, MAXKINGINDEX);
#endif
	/*fprintf(fp, "\n\n\nstatic int backrank[BRNUM] = {");
	nonzero = 0;
	for (i = 0; i < MAXINDEX_BR4K; i++) {
		if (((int)weightsbr4k[i]) != 0) {
			nonzero++;
		}
		w = round(2 * weightsbr4k[i]);
		fprintf(fp, " %i,", w);
		if (i % 32 == 0)
			fprintf(fp, "\n");
	}

	fprintf(fp, "};");*/

	fclose(fp);
	getch();



	// write a file with pattern weights for br4k
	// write parameters as C code to file
	//FILE* fp;
	fp = fopen("C:\\code\\checkersdata\\patternoutput_br.txt", "w");

	fprintf(fp, "\nstatic int backrank[BRNUM] = {");
	nonzero = 0;
	for (i = 0; i < MAXINDEX_BR4K; i++) {
		if (((int)weightsbr4k[i]) != 0) {
			nonzero++;
		}
		w = round(2 * weightsbr4k[i]);
		fprintf(fp, " %i,", w);
		if (i % 32 == 0)
			fprintf(fp, "\n");
	}

	fprintf(fp, "};");
	fclose(fp);
	printf("\n%i weights of %i are nonzero", nonzero, MAXINDEX_BR4K);
	//getch();

	// finally finally set weights in Cake: 
	// set the weights in Cake's eval
	// next, set the br patterns found here in the eval so we can continue: 
	int* weights_int = malloc(sizeof(int) * max(MAXKINGINDEX, MAXINDEX_BR531K));


	for (i = 0; i < MAXINDEX_BR531K; i++)
		weights_int[i] = round(2 * weightsbr531k[i]);
	pattern_1_set(weights_int);

	for (i = 0; i < MAXKINGINDEX; i++)
		weights_int[i] = round(2 * weights[i]);
	pattern_2_set(weights_int);
	for (i = 0; i < MAXKINGINDEX; i++)
		weights_int[i] = round(2 * weights2[i]);
	pattern_3_set(weights_int);
	for (i = 0; i < MAXKINGINDEX; i++)
		weights_int[i] = round(2 * weightsside[i]);
	pattern_4_set(weights_int);
#ifdef PAT5
	for (i = 0; i < MAXKINGINDEX; i++)
		weights_int[i] = round(2 * weightscenter[i]);
	pattern_5_set(weights_int);
#endif
	for (i = 0; i < MAXINDEX_BR4K; i++)
		weights_int[i] = round(2 * weightsbr4k[i]);
	pattern_br_set(weights_int);


	// finally, calculate what will happen with the weights rounded to nearest integers:
	for (i = 0; i < MAXKINGINDEX; i++) {
		weights[i] = round(2 * weights[i]);
		weights[i] /= 2;
		weights2[i] = round(2 * weights2[i]);
		weights2[i] /= 2; 
		weightsside[i] = round(2 * weightsside[i]);
		weightsside[i] /= 2; 
#ifdef PAT5
		weightscenter[i] = round(2 * weightscenter[i]);
		weightscenter[i] /= 2;
#endif
	}

	for (i = 0; i < MAXINDEX_BR4K; i++) {
		weightsbr4k[i] = round(2 * weightsbr4k[i]); 
		weightsbr4k[i] /= 2; 
	}

	for (i = 0; i < MAXINDEX_BR531K; i++) {
		weightsbr531k[i] = round(2 * weightsbr531k[i]);
		weightsbr531k[i] /= 2;
	}

	errorsum = 0;
	num = 0;
	for (i = 0; i < n; i++) {
		p.bm = ep[i].bm;
		p.bk = ep[i].bk;
		p.wm = ep[i].wm;
		p.wk = ep[i].wk;
		p.color = ep[i].color;
		// seems I am missing br4k here?
		int index = getkingindex(&p);   // maybe just replace with &ep[i]?
		int reverseindex = getreversekingindex(&p);
		int index2 = getkingindex2(&p); 
		int reverseindex2 = getreversekingindex2(&p); 
		int indexbr = getindex_br531k(&p);
		int reverseindexbr = getreverseindex_br531k(&p); 
		int indexside = indexk_side_get_magic(&p);
		int reverseindexside = indexk_side_reverse_get_magic(&p);
#ifdef PAT5
		int indexcenter = indexk_center_get_magic(&p);
		int reverseindexcenter = indexk_center_reverse_get_magic(&p);
#endif

#ifdef BR4
		int indexbr4k = (p.bm & 0xFFFF);
		int reverseindexbr4k = fastrev16((p.wm >> 16));
#else
		int indexbr4k = (p.bm & 0xFFF);
		int reverseindexbr4k = fastrev12((p.wm >> 20));
#endif
		double w = 0;

		w = weights[index];
		w -= weights[reverseindex];
		w += weights2[index2]; 
		w -= weights2[reverseindex2]; 
		w += weightsbr531k[indexbr]; 
		w -= weightsbr531k[reverseindexbr]; 

		w += weightsbr4k[indexbr4k];
		w -= weightsbr4k[reverseindexbr4k];

		w += weightsside[indexside];
		w -= weightsside[reverseindexside];
#ifdef PAT5
		w += weightscenter[indexcenter];
		w -= weightscenter[reverseindexcenter];
#endif

#ifdef AVERAGE
		num += (ep[i].wins + ep[i].draws + ep[i].losses);
		error = (1.0 - sigmoid((double)staticeval[i] + w, c));
		error = error * error;
		sum = error * ep[i].wins;
		error = (0.0 - sigmoid((double)staticeval[i] + w, c));
		error = error * error;
		sum += error * ep[i].draws;
		error = (-1.0 - sigmoid((double)staticeval[i] + w, c));
		error = error * error;
		sum += error * ep[i].losses;
		if (sum != sum) {
			printf("\n%i! %i %.2f %.2f", i, staticeval[i], weights[index], weights[reverseindex]);
			getch();
		}
		errorsum += sum;
#endif
	}
	errorsum = errorsum / (double)num;
	printf("\nerror sum after rounding is is %.12f", errorsum);
	return 0; 
	//getch();
}


double calc_error(int n, EVALUATEDPOSITION* ep, double c) {
	int i;
	POSITION p;
	MATERIALCOUNT mc;
	int staticeval;
	int delta;
	double res, error, errorsum = 0;
	double sum;
	int num = 0; 
		 
	for (i = 0; i < n; i++) {
#ifdef STOCHASTIC
		if (isinactive[i])
			continue;
#endif
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
#ifdef AVERAGE
#ifdef OPTIMIZE_SEARCHEVAL

		res = ep[i].searchvalue;
		if (ep[i].color == WHITE)
			res = -res; // now staticeval is as seen from black's point of view
		res = sigmoid(res, c);
		error = (res - sigmoid(staticeval, c));
		error = error * error;
		error *= (ep[i].wins + ep[i].draws + ep[i].losses);
		errorsum += error; 
		error_ST[i] = error;
		num += (ep[i].wins + ep[i].draws + ep[i].losses);

#else
		num += (ep[i].wins + ep[i].draws + ep[i].losses);
		error = (1 - sigmoid((double)staticeval, c));
		error = error * error;
		sum = error * ep[i].wins;

		error = (0 - sigmoid((double)staticeval, c));
		error = error * error;
		sum += error * ep[i].draws;

		error = (-1 - sigmoid((double)staticeval, c));
		error = error * error;
		sum += error * ep[i].losses;

		errorsum += sum;
		error_ST[i] = sum;
#endif
#else
		// res is game result as seen from black's point of view (1 = win, 0 = draw, -1 = loss
		res = result_translated[ep[i].gameresult];

#ifdef OPTIMIZE_SEARCHEVAL
		res = ep[i].searchvalue; 
		if (ep[i].color == WHITE)
			res = -res; // now staticeval is as seen from black's point of view
		res = sigmoid(res, c); 
#endif
		error = (res - sigmoid(staticeval, c));
		error = error * error;
		errorsum += error;
		error_ST[i] = error;
		num++;
#endif

		//if (i == n / 2)
		//	printf("\ncalc standard @n/2: %.2f (%i games)", errorsum, num);
		//printboard(&p);
		//printf("\nstatic eval, search eval: %i %i", staticeval, eval);
		//getch();
	}
	//printf("\ncalc standard: errorsum is %.2f, number is %i", errorsum, num);

	errorsum = errorsum / (double)num;
	
	return errorsum;
}

double sigmoid(double v, double c) {
	double res;
	res = 2 / (1 + exp((double) (-c * ((double)v)))) - 1;
	return res; 
}

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
}



int PositiontoFEN(POSITION* p, char* FEN) {
	// W:WK7,11,14,18,22:B1,12,13,19,K30

	static int bit_to_square[32] = { 4,3,2,1,8,7,6,5,
		12,11,10,9,16,15,14,13,
		20,19,18,17,24,23,22,21,
		28,27,26,25,32,31,30,29 }; /* maps bits to checkers notation */


	//int i; 
	int32 tmp; 
	int square; 
	char s[16]; 
	int haskings = 0; 

	if (p->color == BLACK)
		sprintf(FEN, "B:");
	else
		sprintf(FEN, "W:"); 

	sprintf(s, "W");
	strcat(FEN, s); 
	tmp = p->wk;
	while (tmp) {
		square = LSB(tmp); 
		tmp ^= (1 << square);
		square = bit_to_square[square];
		sprintf(s, "K%i", square);
		strcat(FEN, s);
		if (tmp)
			strcat(FEN, ","); 
	}
	tmp = p->wm;
	if (p->wk)
		haskings = 1;
	else
		haskings = 0; 
	while (tmp) {
		square = LSB(tmp);
		tmp ^= (1 << square);
		square = bit_to_square[square];
		if (haskings)
			sprintf(s, ",%i", square); 
		else
			sprintf(s, "%i", square);
		strcat(FEN, s);
		if (tmp)
			strcat(FEN, ",");
		haskings = 0; 
	}

	sprintf(s, ":B");
	strcat(FEN, s);
	tmp = p->bk;
	if (p->bk)
		haskings = 1;
	else
		haskings = 0; 
	while (tmp) {
		square = LSB(tmp);
		tmp ^= (1 << square);
		square = bit_to_square[square];
		sprintf(s, "K%i", square);
		strcat(FEN, s);
		if (tmp)
			strcat(FEN, ",");
	}
	tmp = p->bm;
	while (tmp) {
		square = LSB(tmp);
		tmp ^= (1 << square);
		square = bit_to_square[square];
		if(haskings)
			sprintf(s, ",%i", square);
		else
			sprintf(s, "%i", square);
		strcat(FEN, s);
		if (tmp)
			strcat(FEN, ",");
		haskings = 0; 
	}
	strcat(FEN, "\n"); 
	return 0; 
}

int getindex_br531k(POSITION* p) {
	return index_get(p);
}

int getreverseindex_br531k(POSITION* p) {
	return index_reverse_get(p);
}

int getkingindex(POSITION* p) {
	//printf("\n %i %i ", indexk_get(p), indexk_get_magic(p)); 
	//return indexk_get(p);
	return indexk_get_magic(p);
}

int getreversekingindex(POSITION* p) {
	//printf("\n %i %i ", indexk_reverse_get(p), indexk_reverse_get_magic(p));
	//return indexk_reverse_get(p);
	return indexk_reverse_get_magic(p);
}

int getkingindex2(POSITION* p) {
	//printf("\n %i %i ", indexk2_get(p), indexk2_get_magic(p)); 
	//return indexk2_get(p);
	return indexk2_get_magic(p); 
}

int getreversekingindex2(POSITION* p) {
	//printf("\n %i %i ", indexk2_reverse_get(p), indexk2_reverse_get_magic(p));
	//return indexk2_reverse_get(p);
	return indexk2_reverse_get_magic(p);
}

int getkingindexside(POSITION* p) {
	//printf("\n %i %i ", indexk2_get(p), indexk2_get_magic(p)); 
	//return indexk2_get(p);
	return indexk_side_get_magic(p);
}

int getreversekingindexside(POSITION* p) {
	//printf("\n %i %i ", indexk2_reverse_get(p), indexk2_reverse_get_magic(p));
	//return indexk2_reverse_get(p);
	return indexk_side_reverse_get_magic(p);
}

#ifdef PAT5
int getkingindexcenter(POSITION* p) {
	return indexk_center_get_magic(p);
}

int getreversekingindexcenter(POSITION* p) {
	return indexk_center_reverse_get_magic(p);
}
#endif





/*Kingsrow(x64) 1.17d played 25 - 21
analysis: value = 4, depth 22 / 21.5 / 41, 2.7s, 5497 kN / s, pv 25 - 21 9 - 14 29 - 25 11 - 15 23 - 18 14x23 27x11 8x15 17 - 14 10x17 21x14
Cake 1.85_2017d(x64) played 9 - 14
analysis : depth 19 / 40 / 20.6  time 1.38s  value = 10  nodes 5452751  3922kN / s  db 98 % cut 95.3% pv  9 - 14 22 - 18 13x22 26x17 11 - 15 18x11  8x15 29 - 25
Kingsrow(x64) 1.17d played 21x14
analysis : value = 2, depth 5 / 6.7 / 12, 0.0s, 13 kN / s, pv 21x14 4 - 8 24 - 19 15x24 28x19 8 - 11
Cake 1.85_2017d(x64) played 12 - 16
analysis : depth 19 / 34 / 19.3  time 2.70s  value = 2  nodes 9382738  3460kN / s  db 100 % cut 95.6% pv 12 - 16 32 - 27 16 - 20 24 - 19 15x24 28x19  4 - 8 25 - 21
*/
/*
int analyze_matchlog(void) {
	// read match log file
	FILE* fp;
	char line[256];
	int value, d1, d2;
	double time, d3;
	double timesum_kr = 0, timesum_cake = 0;
	int n_kr = 0, n_cake = 0;
	double time_cake[100000];
	double time_kingsrow[100000];

	fp = fopen("C:\\Users\\Martin Fierz\\Documents\\Martin Fierz\\CheckerBoard\\games\\matches\\matchlog84.txt", "r");
	while (!feof(fp)) {
		fgets(line, 255, fp);
		if (line[0] == 'a' && (line[10] == 'v' || line[33] == 'v')) {  // kingsrow
			//printf("\n%s", line);
			sscanf(line, "analysis: value=%i, depth  %i/%lf/%i, %lfs", &value, &d1, &d3, &d2, &time);
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
			sscanf(line, "analysis: depth %i/%i/%lf  time %lf", &d1, &d2, &d3, &time);
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
*/


/*
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
		fprintf(fp, "%i\t%.3f\n", i, (double)wins[i]/(double)(n[i]));
	fclose(fp);


	getch();
	exit(0);
	return 0;
} */



/*
int FENtoPosition(char* FEN, POSITION* p)
{
	// parses the FEN string in *FEN and places the result in p and color
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

	// parse color ,whitestring, blackstring
	col = strtok(FENstring, ":");

	if (col == NULL)
		return 0;

	if (strcmp(col, "W") == 0)
		p->color = WHITE;
	else
		p->color = BLACK;

	// parse position: get white and black strings

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


	// reset board
	p->bm = 0;
	p->wm = 0;
	p->bk = 0;
	p->wk = 0;

	// parse white string
	token = strtok(white, ",");

	while (token != NULL)
	{
		// While there are tokens in "string"
		// a token might be 18, or 18K
		piece = MAN;
		if (token[0] == 'K')
		{
			token++;
			piece = KING;
		}
		number = atoi(token);
		// ok, piece and number found, transform number to coors
		number = SquareToBit(number);

		if (piece == MAN)
			p->wm |= one << number;
		else
			p->wk |= one << number;
		// Get next token:
		token = strtok(NULL, ",");
	}
	// parse black string
	token = strtok(black, ",");
	while (token != NULL)
	{
		// While there are tokens in "string"
		// a token might be 18, or 18K
		piece = MAN;
		if (token[0] == 'K')
		{
			piece = KING;
			token++;
		}
		number = atoi(token);
		// ok, piece and number found, transform number to coors
		number = SquareToBit(number);
		if (piece == MAN)
			p->bm |= one << number;
		else
			p->bk |= one << number;
		// Get next token:
		token = strtok(NULL, ",");
	}
	return 1;
}*/


/*
int loadpatterns(double *weights) {
	FILE *fp = fopen("C:\\code\\checkersdata\\patternoutput.txt", "r");
	int i, weight;
	while (!feof(fp)) {
		fscanf(fp, "w[%i] = %i;\n", &i, &weight);
		printf("\n%i %i", i, weight);
		weights[i] = weight;
	}
	printf("\n\nfile read, hit key to continue");
	getch();
}*/

/*int create_indices() {
	int mult[12];

	int totalindex;
	// create multipliers for bits (12 squares, with base 3)
	mult[0] = 1;
	for (int i = 1; i < 12; i++)
		mult[i] = mult[i - 1] * 3;


	// create index for a 12-bit pattern
	for (int i = 0; i < 4096; i++) {
		index[i] = 0;
		for (int j = 0; j < 12; j++) {
			if (i & (1 << j))
				index[i] += mult[j];
		}
	}


	for (int i = 0; i < MAXINDEX; i++) {
		// i is a number between 0 and 3^12, describing the pattern of the last 3 ranks 
		// for the case that there is no opponent king on the 2nd or 3rd rank
		indexnum[i] = 0;
	}
	return 1;
}*/



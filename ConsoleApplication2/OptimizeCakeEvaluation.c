// ConsoleApplication2.cpp : Defines the entry point for the console application.
//


// 26.11.2017
// a first test with an "optimized" evaluation gives catastrophic result
// +22-57 instead of ~equal!

#include <stdlib.h> 
#include <assert.h>
#include <windows.h>
#include <math.h>
#include "stdafx.h"
//#include "cake_misc.h"


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

#define PARAMS 24

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

// tables for evaluation
static short materialeval[13][13][13][13];						// bm bk wm wk
char  blackbackrankeval[256], whitebackrankeval[256];			// not static because it's used too in movegen for move ordering
static char  blackbackrankpower[256], whitebackrankpower[256];	// used for man down situations

int params[PARAMS]; // parameters to optimize


int main()
{
	int i = 0, n;
	FILE *fp; 
	unsigned int bm, bk, wm, wk, color; 
	int eval; 
	int staticeval; 
	POSITION p; 
	MATERIALCOUNT mc; 
	int delta = 100; 
	EVALUATEDPOSITION *ep, *cur; 
	float errorsum; 
	float error; 
	int j, k; 
	int oldparam; 
	float result[PARAMS][3]; // save optimization result here
	int iterations = 0; 
	float minerror; 
	int minj, mink; 
	int position_number = 0; 
	int changed = 0; 
	int result_translated[4] = {0, 1, -1, 0};
	int res_from_file; 
	float res; 
	float c; 
	int rejected = 0; 
	int pos_num = 0; 
	int quiet_pos_num = 0; 

	// if I want to check search times, run analyze_machtprogress()
	//	analyze_matchprogress(); 

	ep = malloc(sizeof(EVALUATEDPOSITION) * 2000000);

	// if I want to read a PDN file, and tag the positions by result, run load_PDN(); 
	position_number = load_PDN(ep); 
	fp = fopen("C:\\code\\checkersdata\\taggedpositions.txt", "w");
	for (i = 0; i < position_number; i++)
		fprintf(fp, "%u %u %u %u %i %i\n", ep[i].bm, ep[i].bk, ep[i].wm, ep[i].wk, ep[i].color, ep[i].gameresult);
	fclose(fp); 
	printf("\nall games in checked, %i positions stored", position_number);
	getch(); 
	exit(0); 
	//printf("\nfound %i positions in PDN file", position_number); 
	//getch(); 




	

	params[0] = 5; // devsinglecornerval
	params[1] = 5; // intactdoublecornerval; 
	params[2] = 8;  // oreoval; 
	params[3] = 4; // idealdoublecornerval; 
	params[4] = 30; // backrankpower 1
	params[5] = 35; // backrankpower 2
	params[6] = 40; // backrankpower 3
	params[7] = 130; // king value
	params[8] = 4; // crampval;
	params[9] = 1; // nocrampval; 
	params[10] = 2; // badstructureval; 
	params[11] = 8; // dogholeval; 
	params[12] = 20; // dogholemandownval; 
	params[13] = 1; // mc_occupyval
	params[14] = 1; // mc_attackval; 
	params[15] = 5; // realdykeval; 
	params[16] = 2; // greatdykeval; 
	params[17] = 24; // promoteinone;
	params[18] = 20; // promoteintwo;
	params[19] = 12; // promoteinthree; 
	params[20] = 10; // tailhookval; 
	params[21] = 4; // kcval
	params[22] = -4; // keval; 
	params[23] = 3; // turnval; 

	
	initeval();
					// optimized parameters are
	//-2, 0, -6, -3, 30,50, 40, 126,3,-4

	//for (i = 0; i <= 23; i++)
	//	params[i] = 0; 
	//params[7] = 130; // keep only king value


	//fp = fopen("c:\\code\\checkersdata\\evaluations2018.txt", "r");
	fp = fopen("c:\\code\\checkersdata\\taggedevaluatedpositions.txt", "r");
	printf("\nloading...");

	cur = ep; 
	i = 0; 
	j = 0; 
	while (!feof(fp)) {
		
		// bm, bk, wm, wk, color, evaluation, result
		// evaluation is from side to move; BLACK to move is 2, WHITE to move is 1
		// result is WIN (1) if black won, LOSS (2) if white won, DRAW (0) or UNKNOWN (3)
		
		fscanf(fp, "%u %u %u %u %i %i %i\n", &bm, &bk, &wm, &wk, &color, &eval, &res_from_file);
		pos_num++; 
		//printf("\n%i %u %u %u %u %i %i %i\n", j++, bm, bk, wm, wk, color, eval, res_from_file); 
		//getch(); 

		//fscanf(fp, "%u %u %u %u %i %i\n", &bm, &bk, &wm, &wk, &color, &eval);
		p.bm = bm;
		p.bk = bk; 
		p.wm = wm; 
		p.wk = wk; 
		p.color = color; 
		if (!testcapture(&p) && abs(eval)<=100) {
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
				cur->value = eval;
				cur->gameresult = res_from_file;
				
				mc.bm = bitcount(p.bm);
				mc.bk = bitcount(p.bk);
				mc.wm = bitcount(p.wm);
				mc.wk = bitcount(p.wk);
				cur->staticeval = evaluation(&p, &mc, 0, &delta, 0, 0);
				if (abs(cur->staticeval - cur->value) > 50)
					rejected++; 
				else
					cur++; 
				//printf("\nthis position has evaluation %i", evaluation);

				
				//getch();
				if (i % 10000 == 0)
					printf("\n%i", i);
				i++;
			}
		}
	}
	n = i - 1 - rejected;
	printf("\ntotal %i, quiet %i, final %i, %i were rejected", pos_num, quiet_pos_num, n, rejected);
	getch(); 
	printf("\nevaluating...");

	// step 1: find optimal constant for sigmoid
	for(c = 0.001; c < 0.04; c+= 0.001) {
		//printf("\n%.3f", c);
		errorsum = 0; 
		cur = ep; 
		for (i = 0; i < n; i++) {
			p.bm = cur->bm;
			p.bk = cur->bk;
			p.wm = cur->wm;
			p.wk = cur->wk;
			p.color = cur->color;

			/*if (abs(cur->staticeval - cur->value) > 50) {
				printboard(&p);
				printf("\npos %i, static %i shallow search %i", i, cur->staticeval, cur->value);
				getch();
			}*/

			mc.bm = bitcount(p.bm);
			mc.bk = bitcount(p.bk);
			mc.wm = bitcount(p.wm);
			mc.wk = bitcount(p.wk);
			staticeval = evaluation(&p, &mc, 0, &delta, 0, 0);
			//errorsum += abs(staticeval - ep[i].value);

			// logistic optimization
			// staticeval was computed as side to move. normalize to black to move
			if (p.color == WHITE)
				staticeval = -staticeval; // now staticeval is as seen from black's point of view

			res = result_translated[cur->gameresult];
			// res is game result as seen from black's point of view (1 = win, 0 = draw, -1 = loss)

			error = (res - sigmoid(staticeval, c));

			//printboard(&p); 
			//printf("\nstatic eval (black) %i result %i res-trans %.1f sigmoid %.3f", staticeval, cur->gameresult, res, sigmoid(staticeval, c));
			//getch(); 
			//printf("\npos %i res %.1f staticeval %i, err %.3f", i, res, staticeval, error);
			//printf("\n%i res %.1f", i, res);
			errorsum += error * error;
			cur++;
		}
		printf("\n%.3f err=%.5f", c, errorsum / n);
	}
	getch();
	c = 0.012; 

	while (iterations < 1000) {
		changed = 0; 
		for (j = 0; j < PARAMS; j++) {  // for all params do
			errorsum = 0;
			oldparam = params[j];
			//printf("\nparameter %i\n", j);
			for (k = -1; k <= 1; k++) {
				/*if (iterations > 0 && k == 0) {
					printf("%i:", oldparam);
					printf("%.5f\t", minerror);
					continue;
				}*/
				errorsum = 0;
				// adjust param j by k
				params[j] = oldparam + k;
				initeval();
				//printf("%i:", params[j]);
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

					// logistic optimization
					// staticeval was computed as side to move. normalize to black to move
					if (ep[i].color == WHITE)
						staticeval = -staticeval; // now staticeval is as seen from black's point of view
					
					res = result_translated[ep[i].gameresult];
					// res is game result as seen from black's point of view (1 = win, 0 = draw, -1 = loss)

					error = (res - sigmoid(staticeval,c));
					errorsum += abs(error); // error* error;


					//printboard(&p);
					//printf("\nstatic eval, search eval: %i %i", staticeval, eval);
					//getch();
				}
				//printf("%.5f\t", (float)errorsum / (float)n);
				result[j][k + 1] = (float)errorsum / (float)n;
			}
			// reset parameter
			params[j] = oldparam;
			// now, instead choose the parameter value that worked best: 
			minerror = 1000; 
			for (k = 0; k < 3; k++) {
				if (result[j][k] < minerror) {
					minerror = result[j][k];
					mink = k;
				}
			}
			params[j] += (mink - 1); 
			if (mink != 1)
				changed++; 
			printf("\n***adjusted parameter %i by %i to %i (%.5f)", j, (mink - 1), params[j], minerror);
		}
		printf("\niteration %i with %i changes-----------------", iterations, changed);
		iterations++; 
		// now we have tested everything, find minimal value
		/*if(iterations == 0) 
			minerror = 1000.0; 
		minj = 0; 
		mink = 0; 
		for (j = 0; j < PARAMS; j++) {
			for (k = 0; k < 3; k++) {
				if (result[j][k] < minerror) {
					minerror = result[j][k];
					minj = j;
					mink = k;
				}
			}
		}
		// now we have minj and mink!
		iterations++; 
		if (mink == 1)
			break; 
		// mink is not 1 = there was a change that helped
		params[minj] += (mink - 1);
		printf("\n***adjusted parameter %i by %i (%.5f)", minj, (mink - 1), minerror);
		//printf("\nparameters set to: ");
		//for (j = 0; j < PARAMS; j++)
		//	printf("\n%i: %i", j, params[j]); */
		if (changed == 0)
			break; 
	}
	
	printf("\n%i iterations made, no more improvement possible", iterations);
	printf("\nerror after optimization is %.5f", minerror);

	getch(); 
    return 0;
}

float sigmoid(int v, float c) {
	float res; 
	//float c = 0.02; 
	//res = exp(0.001);

	res = 2 / (1 + exp((double) (-c * ((float)v)))) - 1;
	return res; 

	// example: if value = 100 i get
}




int load_PDN(EVALUATEDPOSITION *ep) {
	FILE* fp;
	char* buffer;
	char* start;
	char* startheader; 
	char* tag; 
	char game[10000];
	char header[256];
	char headername[32]; 
	char headervalue[256]; 
	char token[256]; 
	char movestring[32]; 
	int bytesread;
	int from, to; 
	POSITION p; 
	int result; 
	MOVE movelist[MAXMOVES];
	POSITION historypos[4]; 

	int i, j, n; 
	int values[MAXMOVES];
	SEARCHINFO si; // dummy searchinfo, not used for anything
	int forcefirst = MAXMOVES - 1;
	int checkfrom, checkto; 
	char c; 
	int found; 
	int gamenumber = 0; 
	int positions = 0; 
	int rep; 
	int capture; 

	buffer = malloc(1000000);
	if (buffer == NULL)
		return 0;
	fp = fopen("C:\\code\\checkersdata\\match.pdn", "r");

	if (!fp)
		return 0;
	/* load a PDN database into buffer */
	bytesread = fread(buffer, 1, 1000000, fp);
	printf("read %i bytes in PDN database - hit a key", bytesread);
	getch();
	fclose(fp);
	buffer[bytesread] = 0;

	//printf("found %i games\n", PDNparseGetnumberofgames("C:\\code\\checkersdata\\match.pdn"));
	//getch(); 

	start = buffer;

	// get games
	while (PDNparseGetnextgame(&start, game)) {
		//printf("%s", game);
		gamenumber++; 
		printf("\ngame number %i", gamenumber);
		// now we have a game in the string game
		startheader = game; 

		// get headers
		while (PDNparseGetnextheader(&startheader, header)) {
			//get a header
			//printf("\nheader %s", header);
			//getch(); 

			tag = header;
			PDNparseGetnexttoken(&tag, headername);
			PDNparseGetnexttag(&tag, headervalue);
			_strlwr(headername);

			if (strcmp(headername, "result") == 0) {
				//result = string_to_pdn_result(headervalue, gametype);
				printf("result is %s", headervalue);
				if (strcmp(headervalue, "1-0") == 0) {
					result = WIN;
					printf("\nWIN\n");
				}
				else if (strcmp(headervalue, "0-1") == 0) {
					result = LOSS;
					printf("\nLOSS\n");
				}
				else if (strcmp(headervalue, "1/2-1/2") == 0) {
					result = DRAW;
					printf("\nDRAW\n");
				}
				else {
					result = UNKNOWN;
					printf("\nUNKNOWN");
				}
			}

			if (strcmp(headername, "fen") == 0) {
				//printf("fen is <%s>", headervalue);
				FENtoPosition(headervalue, &p); 
				//printboard(&p); 
			}
		}

		// store position
		ep[positions].bm = p.bm;
		ep[positions].bk = p.bk;
		ep[positions].wm = p.wm;
		ep[positions].wk = p.wk;
		ep[positions].color = p.color;
		ep[positions].value = 0;
		ep[positions].gameresult = result;
		positions++;

		// get moves
		while (PDNparseGetnexttoken(&startheader, token)) {
			//printf("\ntoken %s", token);
			/* skip move numbers, that is, skip if the last character
				of the token is a full stop */
			if (token[strlen(token) - 1] == '.') continue;
			if (token[0] == '{') continue; // skip comments
			//printf("%s", token);
			PDNparseTokentonumbers(token, &from, &to);
			//printf("  %i  %i", from, to);
		
			resetsearchinfo(&si);

			found = 0; 
			n = makecapturelist(&p, movelist, values, forcefirst);
			if (n == 0)
				n = makemovelist(&si, &p, movelist, values, forcefirst, 0);
			for (i = 0; i < n; i++) {
				movetonotation(&p, &(movelist[i]), movestring);
				sscanf(movestring, "%i%c%i", &checkfrom, &c, &checkto);
				//printf("\nmove generated %i %i", checkfrom, checkto);
				if (checkfrom == from && checkto == to) {
					//printf("found");
					togglemove((&p), movelist[i]);
					found = 1; 
					break;
				}
			}
			if (found == 0) {
				printf("move not found!");
				getch();
			}
			// check for capture
			capture = 0; 
			if (testcapture(&p))
				capture = 1; 
			p.color ^= CC; 
			if (testcapture(&p))
				capture = 1; 
			p.color ^= CC; 

			// check for repetition
			rep = 0; 
			if (positions > 4 && p.bk && p.wk) {
				for (j = 0; j < 4; j++) {
					if (historypos[j].bm == p.bm &&
						historypos[j].bk == p.bk &&
						historypos[j].wm == p.wm &&
						historypos[j].wk == p.wk)
						rep = 1; 
				}
			}
			
			if (bitcount(p.bm | p.bk | p.wm | p.wk) <= 8)
				rep = 1; 

			// store position
 
			historypos[positions % 4].bm = p.bm; 
			historypos[positions % 4].bk = p.bk; 
			historypos[positions % 4].wm = p.wm;
			historypos[positions % 4].wk = p.wk; 
			if (capture == 0 && rep == 0) {
				ep[positions].bm = p.bm;
				ep[positions].bk = p.bk;
				ep[positions].wm = p.wm;
				ep[positions].wk = p.wk;
				ep[positions].color = p.color;
				ep[positions].value = 0;
				ep[positions].gameresult = result;
				positions++;
			}
		}
	}
	return positions; 
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

int db_exit() {
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

/*
int SquareToBit(int square)
{
	int x, y;
	int bit;

	square--;

	x = square % 4;
	y = square / 4;

	bit = 4 * y + (3 - x);

	return bit;
}*/



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


// tiny decider functions what no move, no material means (used so that cake can easily be 
// configured to play different versions (kingscourt, suicide))
int evaluation_nomaterial_black(POSITION *p, int depth)
{
	return (p->color == BLACK) ? (-MATE + depth) : (MATE - depth);
}

int evaluation_nomaterial_white(POSITION *p, int depth)
{
	return (p->color == WHITE) ? (-MATE + depth) : (MATE - depth);
}

int evaluation_nomove(int depth)
{
	return -MATE + depth;
}
// some helper functions

int initeval(void)
{
	initializematerial(materialeval);
	initializebackrank(blackbackrankeval, whitebackrankeval, blackbackrankpower, whitebackrankpower);
	return 1;
}

int staticevaluation(SEARCHINFO *si, EVALUATION *e, POSITION *q, int *total, int *material, int *positional, int *delta)
{
	int noprune = 0;
	int likelydraw = 0;
	KINGINFO ki;

	countmaterial(q, &(si->matcount));

	*total = evaluation(q, &(si->matcount), 0, &noprune, 0, 0);
	e->material = materialevaluation(&(si->matcount));
	e->selftrap = selftrapeval(q, &(si->matcount), &ki, delta);
	e->backrank = (blackbackrankeval[q->bm & 0xFF] - whitebackrankeval[q->wm >> 24]);

	*positional = fineevaluation(e, q, &(si->matcount), &ki, &noprune, &likelydraw);
	if (q->color == WHITE)
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
	assert(p->bm | p->bk);
	assert(p->wm | p->wk);

	// the following assert is violated by getpv() but should hold otherwise
	//assert(alpha == beta-1);





	eval = materialeval[mc->bm][mc->bk][mc->wm][mc->wk];
	if (p->color == WHITE)
		eval = -eval;

	e.material = eval;
	//printf("\nmaterial: %i", eval); 

#ifdef EVALMATERIALONLY
#ifdef COARSEGRAINING
	eval = (eval / GRAINSIZE)*GRAINSIZE;
#endif
	return eval;
#endif

	// lazy eval: exit if materialeval is too far from window (400)
	if ((eval>alpha + 1 + LAZYEVALWINDOW) || (eval<alpha - LAZYEVALWINDOW))
	{
#ifdef COARSEGRAINING
		eval = (eval / GRAINSIZE)*GRAINSIZE;
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
	if (p->color == WHITE)
		e.backrank = -e.backrank;
	eval += e.backrank;


	assert(ki.freebk >= 0);
	assert(ki.freewk >= 0);
	assert((ki.untrappedbk & (~p->bk)) == 0);
	assert((ki.untrappedwk & (~p->wk)) == 0);

	// SECOND LAZY EXIT (TODO: code lazy exit into function to not duplicate stuff)
	if ((eval>alpha + 1 + FINEEVALWINDOW) || (eval<alpha - FINEEVALWINDOW)) // vtune: use |
	{
#ifdef COARSEGRAINING
		eval = (eval / GRAINSIZE)*GRAINSIZE;
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
	if (likelydraw)
		eval /= 2;

#ifdef COARSEGRAINING
	eval = (eval / GRAINSIZE)*GRAINSIZE;
#endif

	// TODO: think about making delta a function of our evaluation, e.g. *delta = abs(e.x + e.y +...) / Y
	//*delta += abs(e.backrank + e.compensation + e.cramp + e.hold + e.king + e.king_man + e.men + e.runaway) / 4;
	return eval;
}


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

	if (!((p->bk & (WBR | SQ28 | SQ25)) | (p->wk & (BBR | SQ5 | SQ8))))
		return 0;

	free = ~(p->bm | p->bk | p->wm | p->wk);

	// selftrapeval looks for kings on the back rank as well as on square 5 for white, SQ28 for black

	// absurd self trapping of white king under bridge:
	if (match3(p->bm, p->wm, p->wk, (SQ1 | SQ3), (SQ6 | SQ7 | SQ10), SQ2))
	{
		eval += 230;
		ki->freewk--;
		ki->untrappedwk ^= SQ2;
	}
	// absurd self trapping of white king under "other" bridge
	if (match3(p->bm, p->wm, p->wk, (SQ2 | SQ4), (SQ7 | SQ8 | SQ11), SQ3))
	{
		eval += 230;
		ki->freewk--;
		ki->untrappedwk ^= SQ3;
	}

	// absurd self trapping of black king under bridge
	if (match3(p->wm, p->bm, p->bk, (SQ30 | SQ32), (SQ23 | SQ26 | SQ27), SQ31))
	{
		eval -= 230;
		ki->freebk--;
		ki->untrappedbk ^= SQ31;
	}

	// absurd self trapping of black king under "other" bridge
	if (match3(p->wm, p->bm, p->bk, (SQ29 | SQ31), (SQ22 | SQ25 | SQ26), SQ30))
	{
		eval -= 230;
		ki->freebk--;
		ki->untrappedbk ^= SQ30;
	}

	// 'absurd' self-trapping in single corner for black
	if (p->bk & SQ29)
	{
		if (p->wm & SQ30)
		{
			if (match1(p->bm, SQ21 | SQ25))
			{
				eval -= 200;
				ki->untrappedbk ^= SQ29;
				ki->freebk--;
			}
			else if (p->bm & SQ22)
			{
				eval -= 100;
				ki->untrappedbk ^= SQ29;
				ki->freebk--;
			}
		}
	}
	if (p->bk & SQ25)
	{
		if (p->wm & SQ30)
		{
			if (p->bm & SQ22)
			{
				eval -= 100;
				ki->untrappedbk ^= SQ25;
				ki->freebk--;
			}
		}
	}

	// 'absurd' self-trapping in single corner for white
	if (p->wk & SQ4)
	{
		if (p->bm & SQ3)
		{
			if (match1(p->wm, SQ8 | SQ12))
			{
				eval += 200;
				ki->untrappedwk ^= SQ4;
				ki->freewk--;
			}
			else if (p->wm & SQ11)
			{
				eval += 100;
				ki->untrappedwk ^= SQ4;
				ki->freewk--;
			}
		}
	}
	if (p->wk & SQ8)
	{
		if (p->bm & SQ3)
		{
			if (p->wm & SQ11)
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
	if (p->wm&SQ31)
	{
		// most awful case:
		if (match2((p->bm | p->bk), (p->bm), (SQ32 | SQ28 | SQ24 | SQ27), (SQ24 | SQ27)))
		{
			eval -= 300;
			ki->freebk -= bitcount(p->bk & (SQ32 | SQ28));
			ki->untrappedbk ^= (p->bk & (SQ32 | SQ28));
		}
		// awful case 
		else if ((p->bk&SQ32) && (p->bm&SQ28))
		{
			if (p->wk&SQ23)
			{
				eval -= 100;
				ki->freebk--;
				ki->untrappedbk ^= SQ32;
			}
			else if ((free&SQ24) && (!(p->bm&SQ20)))
			{
				eval -= 30;
				ki->freebk--;
				ki->untrappedbk ^= SQ32;
			}
		}
	}

	// self trapping in double corner for white kings
	if (p->bm&SQ2)
	{
		// most awful case:
		if (match2((p->wm | p->wk), (p->wm), (SQ1 | SQ5 | SQ6 | SQ9), (SQ6 | SQ9)))
		{
			eval += 300;
			ki->freewk -= bitcount(p->wk & (SQ1 | SQ5));
			ki->untrappedwk ^= (p->wk & (SQ1 | SQ5));
		}
		// awful case 
		else if ((p->wk&SQ1) && (p->wm&SQ5))
		{
			if (p->bk&SQ10)
			{
				eval += 100;
				ki->freewk--;
				ki->untrappedwk ^= SQ1;
			}
			else if ((free&SQ9) && (!(p->wm&SQ13)))
			{
				eval += 30;
				ki->freewk--;
				ki->untrappedwk ^= SQ1;
			}
		}
	}
	return (p->color == BLACK) ? eval : -eval;
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

	int32 free, m;
	int stones = mc->bm + mc->wm;
	int kings = mc->bk + mc->wk;
	int pieces = stones + kings;
	int eval = 0;
	int ending = 0;
	int equal = 0;
	int tempo = 0;




	int crampval = params[8];  //; 4; 
	int nocrampval = params[9]; // 1;
	int badstructureval = params[10];  //2;
	//int blackcramps,whitecramps,newcrampval=10;
	int index;
	int balance = 0;
	int dogholeval = params[11]; // 8;
	int dogholemandownval = params[12]; // 20;
	int mcenter;
	int mc_occupyval = params[13]; // 1;
	int mc_attackval = params[14]; // 1;
	int realdykeval = params[15]; // 5;
	int greatdykeval = params[16]; // 2;
	int promoteinone = params[17]; // 24;
	int promoteintwo = params[18]; // 20;
	int promoteinthree = params[19]; // 12;
	int potbk = 0, potwk = 0;
	//int blackhasbridge=0,whitehasbridge=0;
	int kcval = params[21]; // 4,
	int keval = params[22]; // -4;
	 int kingcentermonopoly = 12;
	 int kingtrappedinsinglecornerval = 20;
	 int kingtrappedinsinglecornerbytwoval = 20;
	 int kingtrappedindoublecornerval = 10;
	 int dominatedkingval = 10;
	 int dominatedkingindcval = 10;
	 int blacktailhooks = 0; 
	 int whitetailhooks = 0;  
	 int tailhookval = params[20]; // 10; // tailhookval is very small!
	int kingproximityval = 4, kingproximity = 0;
	int32 tmp, m1, free2, attack;
	int /*immobilewhitemen=0,immobileblackmen=0,*/immobilemanval = 2, kingholdstwomenval = 10;//allimmobileval=2;
	int onlykingval = 20, roamingkingval = 40;
	int endangeredbridgeval = 6;
	int turn = 0; 
	int turnval = params[23]; // 3;
	int32 white = p->wm | p->wk;
	int32 black = p->bk | p->bm;
	int realdoghole = 0;
	int wr1, wr2, wr3, wr4, wr6, wr7, wr8;
	int br1, br2, br3, br4, br6, br7, br8;
	int32 ungrounded_black = 0, ungrounded_white = 0;
	int32 immobile_black = 0, immobile_white = 0;
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
	static int tmod[25] = { 0,2,2,2,2,1,1,1,1,0,0,0,0,-1,-1,-1,-1,-2,-2,-2,-2,-3,-3,-3,-3 };

	static int ungroundedpenalty[13] = { 0,0,0,2,5,8,11,14,17,20,24,28,32 };


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

	/* big TODO: clean up the above stuff, group eval in lumps, namels:
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
	free = ~(black | white);  // = ~(white|black) should be a tiny bit faster
	if (mc->bm + mc->bk == mc->wm + mc->wk)
		equal = 1;
	if (pieces <= 10)
		ending = 1;


	/* organize: things to do only if men, things only if kings, */
	/* things to do only if men are on the board */

	if (stones)
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
		if (p->wk & ROAMINGWHITEKING)
			e->men -= 2 * ungroundedpenalty[bitcount(ungrounded_black)];
		else
			e->men -= ungroundedpenalty[bitcount(ungrounded_black)];


		// get ungrounded white men
		m = p->wm&EDGE;
		m |= (backward(m)&p->wm);
		m |= (backward(m)&p->wm);
		m |= (backward(m)&p->wm);
		m |= (backward(m)&p->wm);
		ungrounded_white = (~m)&p->wm & 0xFFFFFF00;


		if (p->bk & ROAMINGBLACKKING)
			e->men += 2 * ungroundedpenalty[bitcount(ungrounded_white)];
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

		immobile_black = p->bm & ~(backward(free | white));
		immobile_white = p->wm & ~(forward(free | black));

		e->men += bitcount(immobile_white);
		e->men -= bitcount(immobile_black);

		// try same with ungrounded

		// find immobile ungrounded men: that means that they have no safe
		// squares to go to. safe squares are free but not under attack by
		// the opposing forces.


		// generalized cramp eval - find contact points and
		// check whether the contacting men are grounded or not.
		// leftforward as seen from black men 
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
		tempo += bitcount(p->bm & 0x000000F0);
		tempo += 2 * bitcount(p->bm & 0x00000F00);
		tempo += 3 * bitcount(p->bm & 0x0000F000);
		tempo += 4 * bitcount(p->bm & 0x000F0000);
		tempo += 5 * bitcount(p->bm & 0x00F00000);
		tempo += 6 * bitcount(p->bm & 0x0F000000);

		tempo -= 6 * bitcount(p->wm & 0x000000F0);
		tempo -= 5 * bitcount(p->wm & 0x00000F00);
		tempo -= 4 * bitcount(p->wm & 0x0000F000);
		tempo -= 3 * bitcount(p->wm & 0x000F0000);
		tempo -= 2 * bitcount(p->wm & 0x00F00000);
		tempo -= bitcount(p->wm & 0x0F000000);


		//tempo*=tmod[stones];
		/* wait with tempo evaluation until doghole is done! */

		// TODO: expensive part of eval - don't do it if equal == 0 (one side is up a king or more)).
		if (mc->bm == mc->wm)
			//if(equal == 1) was worse in engine match
		{
			br1 = bitcount(p->bm&ROW1);
			br2 = bitcount(p->bm&ROW2);
			br3 = bitcount(p->bm&ROW3);
			br4 = bitcount(p->bm&(ROW4 | ROW5));
			br6 = bitcount(p->bm&ROW6);
			br7 = bitcount(p->bm&ROW7);
			br8 = bitcount(p->bm&ROW8);

			wr1 = bitcount(p->wm&ROW1);
			wr2 = bitcount(p->wm&ROW2);
			wr3 = bitcount(p->wm&ROW3);
			wr4 = bitcount(p->wm&(ROW4 | ROW5));
			wr6 = bitcount(p->wm&ROW6);
			wr7 = bitcount(p->wm&ROW7);
			wr8 = bitcount(p->wm&ROW8);

			index = -3 * br1 - 2 * br2 - br3 + br6 + 2 * br7 + 3 * br8;
			balance = -abs(index);
			index = -3 * wr1 - 2 * wr2 - wr3 + wr6 + 2 * wr7 + 3 * wr8;
			balance += abs(index);

			// standard: balance and index
			// try increasing

			e->men += balance;
			// skewness: is a large number for skewed positions i.e. for lots of men toward edge.

			index = -((br1 + br2 - br3 - br4 - br6 + br7 + br8));
			index += ((wr1 + wr2 - wr3 - wr4 - wr6 + wr7 + wr8));
			e->men += index;

			// adjust skewness for number of men - with few men on the board,
			// it doesn't matter any more
			//index = index * (mc->bm+mc->wm);
			//index /= 20;
		}

		// doghole 
		// TODO: test whether writing something like p->bm & SQ3 & backleft(p->wm) is faster
		// TODO: double doghole code
		// TODO: dogholemandownval 20 points is a bit big!


		if ((p->wm&SQ12) && (p->bm&SQ3)) // vtune use &
		{
			// good for black 
			// count away the tempo effect:
			// only if it's good for black! we don't want white to go in the doghole just
			//to save tempo

			if (tmod[stones]>0)
				tempo += 5;

			// and give a penalty - for sure here
			e->men += dogholeval;
		}
		if ((p->wm&SQ5) && (p->bm&SQ1)) //real doghole
		{
			// set white doghole bit so we know
			realdoghole &= WHITE;
			// count away tempo 

			if (tmod[stones]>0)
				tempo += 6;

			// give a penalty or not? - i don't know... 
			if (ending == 0)
				e->men += dogholeval;
			// give a real penalty if there is a white man on bit11 or bit 15,
			//	because this man cannot go through!
			if ((p->wm & (SQ9 | SQ13)) && (free&SQ10))
				e->men += dogholemandownval;
		}
		if ((p->bm&SQ28) && (p->wm&SQ32)) // real doghole
		{
			realdoghole &= BLACK;

			if (tmod[stones]>0)
				tempo -= 6;

			if (ending == 0)
				e->men -= dogholeval;
			if ((p->bm & (SQ24 | SQ20)) && (free&SQ23))
				e->men -= dogholemandownval;
		}
		if ((p->bm&SQ21) && (p->wm&SQ30)) // vtune: use &
		{

			if (tmod[stones]>0)
				tempo -= 5;

			e->men -= dogholeval;
		}


		/* now that doghole has been evaluated, we can do the tempo eval too */
		//static int tmod[25]={0,2,2,2,2,1,1,1,1,0,0,0,0,-1,-1,-1,-1,-2,-2,-2,-2,-3,-3,-3,-3}; 

		e->men += tempo*tmod[stones];

		/* man center control */
		// both with both MCENTER is best result up to now. less good: only occupation with either MCENTER or CENTER
		// TODO: try using occupation only by grounded men.

		/* by occupation */
		mcenter = mc_occupyval*bitcount(p->bm&MCENTER&~ungrounded_black);
		mcenter -= mc_occupyval*bitcount(p->wm&MCENTER&~ungrounded_white);
		e->men += mcenter;

		/* by attack */
		mcenter = mc_attackval*bitcount(attack_forwardjump(p->bm, free)&MCENTER);
		mcenter -= mc_attackval*bitcount(attack_backwardjump(p->wm, free)&MCENTER);
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


		if ((p->bm & SQ19) && (p->wm & SQ28)) // vtune: use &
		{
			e->men += realdykeval;
			if (match1(~p->wm, SQ27 | SQ31 | SQ32))
				e->men += greatdykeval;
		}
		if ((p->wm&SQ14) && (p->bm&SQ5)) // vtune: use &
		{
			e->men -= realdykeval;
			if (match1(~p->bm, SQ1 | SQ2 | SQ6))
				e->men -= greatdykeval;
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
		if (p->bm&SQ20)
		{
			if (p->wm&SQ24)
				e->cramp += crampval;
			if (free&SQ24)
				e->cramp -= nocrampval; //cramping nothing - discourage a little
		}

		if (p->wm&SQ13)
		{
			if (p->bm&SQ9)
				e->cramp -= crampval;
			if (free&SQ9)
				e->cramp += nocrampval; // cramping nothing - discourage a little
		}

		if (p->bm&SQ13)
		{
			if ((p->wm&(SQ17 | SQ21/*|SQ22*/)) == (SQ17 | SQ21/*|SQ22*/))
				e->cramp += crampval;
			if ((free&(SQ17 | SQ21)) == (SQ17 | SQ21))
				e->cramp -= nocrampval; // cramping nothing - discourage a little
		}
		if (p->wm&SQ20)
		{
			if ((p->bm&(/*SQ11|*/SQ12 | SQ16)) == (/*SQ11|*/SQ12 | SQ16))
				e->cramp -= crampval;
			if ((free&(SQ12 | SQ16)) == (SQ12 | SQ16))
				e->cramp += nocrampval; // cramping nothing - discourage a little
		}

		//
		//  end of cramps
		//

		eval += e->cramp;


		// 
		// holds
		//

		// bad structure that often shows up:

		if (match3(p->bm, p->wm, free, (SQ19 | SQ23), (SQ28 | SQ31), (SQ24 | SQ27 | SQ32)))
		{
			e->hold -= 6;
			if (p->wk)
				e->hold -= 10;
		}
		if (match3(p->bm, p->wm, free, (SQ2 | SQ5), (SQ10 | SQ14), (SQ1 | SQ6 | SQ9)))
		{
			e->hold += 6;
			if (p->bk)
				e->hold += 10;
		}

		/* some bad structures */
		/* found with PDNtool */
		/*strange - the single corner side structure for white (0x80808000) is not bad?!! - pdntool */

		if (match1(p->bm, SQ4 | SQ12 | SQ20))
			e->hold -= badstructureval;
		if (match1(p->bm, SQ1 | SQ5 | SQ13))
			e->hold -= badstructureval;

		if (match1(p->wm, SQ32 | SQ28 | SQ20))
			e->hold += badstructureval;
		if (match1(p->wm, SQ13 | SQ21 | SQ29)) // new 1.376
			e->hold += badstructureval;



		// new 1.41 - from DEO with PDNtool
		// describes holds of 2-3 or 3-4 men. this increases in importance, the less men, the worse.
		// only do this if stones <= N.
		// without this: +40-41 vs kr 1.12l, with this +48-39!
		//if(stones<=14) // 14: +:37 =:241 -:10
		//if(stones<=16) // 16: +:33 =:238 -:17
		//if(stones<=12) // 12: +:45 =:228 -:15
		badstructureval = 20 - stones;
		if (badstructureval < 4)
			badstructureval = 4;
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
		if (match3(p->bm, p->wm, free, (SQ5 | SQ9 | SQ13), (SQ14 | SQ17 | SQ18 | SQ22), (SQ1 | SQ6)))
			e->hold += badstructureval;
		if (match3(p->bm, p->wm, free, (SQ12 | SQ16 | SQ20), (SQ19 | SQ23 | SQ24 | SQ27), (SQ4 | SQ8 | SQ11)))
			e->hold += badstructureval;
		if (match3(p->bm, p->wm, free, (SQ11 | SQ15 | SQ16 | SQ19), (SQ20 | SQ24 | SQ28), (SQ27 | SQ32)))
			e->hold -= badstructureval;
		if (match3(p->bm, p->wm, free, (SQ6 | SQ9 | SQ10 | SQ14), (SQ13 | SQ17 | SQ21), (SQ22 | SQ25 | SQ29)))
			e->hold -= badstructureval;
		// add penalty anyway, whether it's free on that diagonal or not - allow one man there
		if (match2(p->bm, p->wm, (SQ5 | SQ9 | SQ13), (SQ14 | SQ17 | SQ18 | SQ22)) && (bitcount(p->bm&(SQ1 | SQ6))<2))
			e->hold += badstructureval;
		if (match2(p->bm, p->wm, (SQ12 | SQ16 | SQ20), (SQ19 | SQ23 | SQ24 | SQ27)) && (bitcount(p->bm&(SQ4 | SQ8 | SQ11))<2)) // this one was wrong!
			e->hold += badstructureval;
		if (match2(p->bm, p->wm, (SQ11 | SQ15 | SQ16 | SQ19), (SQ20 | SQ24 | SQ28)) && (bitcount(p->wm&(SQ27 | SQ32))<2))
			e->hold -= badstructureval;
		if (match2(p->bm, p->wm, (SQ6 | SQ9 | SQ10 | SQ14), (SQ13 | SQ17 | SQ21)) && (bitcount(p->wm&(SQ22 | SQ25 | SQ29))<2)) // this one too!
			e->hold -= badstructureval;
		// new cake manchester 1.10d: nascent classic 3-4-cramp - removed again, was no good in match
		/*if(match3(p->bm, p->wm, free, (SQ5|SQ6|SQ10|SQ14), (SQ13|SQ17|SQ21|SQ23), (SQ1|SQ2|SQ9|SQ18)))
		eval -= badstructureval;
		if(match3(p->bm, p->wm, free, (SQ10|SQ12|SQ16|SQ20), (SQ19|SQ23|SQ27|SQ28), (SQ15|SQ24|SQ31|SQ32)))
		eval += badstructureval;*/
		//if(stones<=14) 
		if (stones <= 12)
		{
			// increases in importance with less men.
			// todo: maybe make more sophisticated by including knowledge about kings here - 
			// if the side which has a hold on the other side also has a free king, it's even
			// worse - because that means he will never have to give up the hold
			// if the side has more free kings than the defending side, it's worse again,
			// because it means that the hold can be changed maybe.
			badstructureval = 20 - stones;

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

			if (match1(p->bm, SQ1 | SQ2) && (bitcount(p->wm & (SQ5 | SQ9 | SQ13 | SQ14 | SQ17 | SQ21)) > 2))
				e->hold += badstructureval;
			if ((bitcount(p->bm&(SQ12 | SQ16 | SQ19 | SQ20 | SQ24 | SQ28))>2) && (match1(p->wm, SQ31 | SQ32)))
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
			if ((match1(p->bm, (SQ1 | SQ2))) && (bitcount(p->wm & (SQ5 | SQ9 | SQ13)) == 2)) // why not >=2?
				e->hold += badstructureval;
			if ((bitcount(p->bm&(SQ20 | SQ24 | SQ28)) == 2) && (match1(p->wm, (SQ31 | SQ32)))) // vtune: use & //vtuned &&->&!
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

			if (match2(p->bm, p->wm, (SQ13 | SQ14 | SQ17), (SQ21 | SQ26)))
				e->hold -= badstructureval;
			if (match2(p->bm, p->wm, (SQ7 | SQ12), (SQ16 | SQ19 | SQ20)))
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
		if ((p->bm) & RANK7)
		{
			if ((p->bm)&(free >> 4) & SQ28)
			{
				e->runaway += promoteinone; potbk++; e->runaway -= 6 * tmod[stones];
			}
			if ((p->bm)&((free >> 3) | (free >> 4)) & SQ27)
			{
				e->runaway += promoteinone; potbk++; e->runaway -= 6 * tmod[stones];
			}
			if ((p->bm)&((free >> 3) | (free >> 4)) & SQ26)
			{
				e->runaway += promoteinone; potbk++; e->runaway -= 6 * tmod[stones];
			}
			/* should modify this for men on 27 which will be trapped in single corner!*/
			if (p->bm & SQ25)
			{
				// man running into single corner - there's a danger of being trapped
				if (!(p->wm & SQ30))
					// no white man on 30 - no trapping
				{
					e->runaway += promoteinone; potbk++; e->runaway -= 6 * tmod[stones];
				}
				else
				{

					// white man on 30, real danger of trapping. 
					if (free & SQ29)

						// else not a runaway in any case
					{
						//printboard(p);
						//printf("\nabove position: potential runaway");
						//getch();
						// if man is assisted and not disturbed, it's a runaway
						if (p->bm & SQ21)
						{
							// check whether white will stop man from moving out 
							// or dominate it with a king
							//if( (!(p->wm & (SQ26|SQ31))) && (!(p->wk&(0xECECE000))) )
							if ((!(p->wm & (SQ26))) && (!(p->wk&(0xECECE000))))
								// nobody there to stop the man
							{
								if (!(p->wm & SQ31))
								{
									e->runaway += promoteinone;
									potbk++;
									e->runaway -= 6 * tmod[stones];
								}
								else
								{
									// we don't have a runaway because white can move his man from
									// 31 to 26, but that will cost him his back rank
									eval += 8;
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

		if ((p->wm) & RANK2)
		{
			if ((p->wm)&(free << 4)& SQ5)
			{
				e->runaway -= promoteinone; potwk++; e->runaway += 6 * tmod[stones];
			}
			if ((p->wm)&((free << 3) | (free << 4)) & SQ6)
			{
				e->runaway -= promoteinone; potwk++; e->runaway += 6 * tmod[stones];
			}
			if ((p->wm)&((free << 3) | (free << 4)) & SQ7)
			{
				e->runaway -= promoteinone; potwk++; e->runaway += 6 * tmod[stones];
			}

			if (p->wm & SQ8)
			{
				// man running into single corner - there's a danger of being trapped
				if (!(p->bm & SQ3))
					// no black man on 3 - no trapping
				{
					e->runaway -= promoteinone; potwk++; e->runaway += 6 * tmod[stones];
				}
				else
				{
					// white man on 30, real danger of trapping. 
					if (free & SQ4)
						// else not a runaway in any case
					{
						// if man is assisted and not disturbed, it's a runaway
						if (p->wm & SQ12)
						{
							// check whether white will stop man from moving out 
							// or dominate it with a king
							//if( (!(p->bm & (SQ2|SQ7))) && (!(p->bk&(0x00073737))) )
							if ((!(p->bm & (SQ7))) && (!(p->bk&(0x00073737))))
								// nobody there to stop the man
							{
								if (!(p->bm&SQ2))
								{
									e->runaway -= promoteinone;
									potwk++;
									e->runaway += 6 * tmod[stones];
								}
								else
								{
									// black has a man on two which can stop the runaway,
									// but it will cost him his back rank.
									eval -= 8;
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


		if ((p->bm) & RANK6)
		{
			if ((p->bm & BIT20) && !(RA20 & white))
			{
				e->runaway += promoteintwo;
				potbk++;
				e->runaway -= 5 * tmod[stones];
			} // vtune: use &, in all others from here
			if ((p->bm&BIT21) && (!(RA21L&white) || !(RA21R&white)))
			{
				e->runaway += promoteintwo;
				potbk++;
				e->runaway -= 5 * tmod[stones];
			}
			if (p->color == BLACK &&  match3(p->bm, p->wm, free, (SQ20 | SQ23), SQ31, (SQ32 | SQ27 | SQ24)))
			{
				if (!(match2(p->bm, free, SQ19, SQ15)))
				{
					e->runaway += promoteintwo;
					potbk++;
					e->runaway -= 5 * tmod[stones];
				}
			}
			if (p->bm&BIT22)
			{
				if (!(RA22L&white) || !(RA22R&white))
				{
					e->runaway += promoteintwo;
					potbk++;
					e->runaway -= 5 * tmod[stones];
				}
				else if (match2(p->bm, free, SQ21, SQ25 | SQ26 | SQ29 | SQ31))
				{
					//printboard(p);
					e->runaway += promoteintwo;
					potbk++;
					e->runaway -= 5 * tmod[stones];
				}
			}
			// add detection for support by man on 21! 
			if ((p->bm&BIT23) && !(RA23&white) && !(BIT22&p->wk))
			{
				e->runaway += promoteintwo;
				potbk++;
				e->runaway -= 5 * tmod[stones];
			}
		}

		if ((p->wm) & RANK3)
		{
			if ((p->wm&BIT8) && !(RA8&black) && !(p->bk&BIT9))
			{
				e->runaway -= promoteintwo; potwk++; e->runaway += 5 * tmod[stones];
			}

			if (p->wm&BIT9)
			{
				if (!(RA9L&black) || !(RA9R&black))
				{
					e->runaway -= promoteintwo; potwk++; e->runaway += 5 * tmod[stones];
				}
				else if (match2(p->wm, free, SQ12, SQ4 | SQ2 | SQ8 | SQ7))
				{
					//printboard(p);
					e->runaway -= promoteintwo; potwk++; e->runaway += 5 * tmod[stones];
				}
			}
			// add detection for support by man on 12 

			if ((p->wm&BIT10) && (!(RA10L&black) || !(RA10R&black)))
			{
				e->runaway -= promoteintwo; potwk++; e->runaway += 5 * tmod[stones];
			}

			if (p->color == WHITE && match3(p->wm, p->bm, free, (SQ10 | SQ13), SQ2, (SQ1 | SQ6 | SQ9)))
			{
				if (!(match2(p->wm, free, SQ14, SQ18)))
				{
					e->runaway -= promoteintwo; potwk++; e->runaway += 5 * tmod[stones];
				}
			}

			if ((p->wm&BIT11) && !(RA11&black))
			{
				e->runaway -= promoteintwo; potwk++; e->runaway += 5 * tmod[stones];
			}
		}



		/** in 3 **/


		if ((p->bm) & RANK5)
		{
			if ((p->bm&BIT16) && !(RA16&white) && !(BIT17 & p->wk)) { e->runaway += promoteinthree; e->runaway -= 4 * tmod[stones]; }
			if ((p->bm&BIT17) && !(RA17&white)) { e->runaway += promoteinthree; e->runaway -= 4 * tmod[stones]; }
			if ((p->bm&BIT18) && !(RA18&white)) { e->runaway += promoteinthree; e->runaway -= 4 * tmod[stones]; }
			if ((p->bm&BIT19) && !(RA19&white)) { e->runaway += promoteinthree; e->runaway -= 4 * tmod[stones]; }
		}
		if ((p->wm) & RANK4)
		{
			if ((p->wm&BIT12) && !(RA12&black)) { e->runaway -= promoteinthree; e->runaway += 4 * tmod[stones]; }
			if ((p->wm&BIT13) && !(RA13&black)) { e->runaway -= promoteinthree; e->runaway += 4 * tmod[stones]; }
			if ((p->wm&BIT14) && !(RA14&black)) { e->runaway -= promoteinthree; e->runaway += 4 * tmod[stones]; }
			if ((p->wm&BIT15) && !(RA15&black) && !(BIT14 & p->bk)) { e->runaway -= promoteinthree; e->runaway += 4 * tmod[stones]; }
		}

		/* bridge situations */
		// TODO this is only for the regular bridge. add it for bm 2,4 vs wm 11
		/* for black */
		/* todo: add bridge code: if kings get out from bridge, good. if not, bad! */

		if (p->bm & SQ23)
		{
			if ((p->wm & BIT28) && (p->wm&BIT30) && (free&BIT29)) // vtune: use &
			{
				ungrounded_black |= SQ23;
				//blackhasbridge = 1;
				/* black has a bridgehead on BIT21 - maybe account for weakness?
				e.g. if p->wk -> penalty, or if wk>bk -> penalty?*/
				/* left side */
				if ((free&BIT24) && (free&BIT25) && (p->bm&BIT20)) // vtune: use &
				{
					e->runaway += promoteintwo;
					e->runaway -= 5 * tmod[stones];
				}
				/* right side */
				if ((free&BIT26) && (free&BIT27) && (p->bm&BIT22)) // vtune: use &
				{
					e->runaway += promoteintwo;
					e->runaway -= 5 * tmod[stones];
				}
			}
		}

		// "other" bridge

		if (p->bm & SQ22)
		{
			if (match2(p->wm, free, SQ29 | SQ31, SQ30))
			{
				ungrounded_black |= SQ22;
				//blackhasbridge = 1;
				if (match2(p->bm, free, SQ23, SQ26 | SQ27))
				{
					e->runaway += promoteintwo;
					e->runaway -= 5 * tmod[stones];
				}
				if (match2(p->bm, free, SQ21, SQ25))
				{
					e->runaway += promoteintwo;
					e->runaway -= 5 * tmod[stones];
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
		if (p->wm & SQ10) // square 10
		{
			if ((p->bm&BIT1) && (p->bm&BIT3) && (free&BIT2)) // vtune: use &
			{
				ungrounded_white |= SQ10;
				/*white has a bridgehead on BIT10 - maybe account for weakness?*/
				//whitehasbridge = 1;
				/* left side */
				if ((free&BIT4) && (free&BIT5) && (p->wm&BIT9)) // vtune: use &
				{
					e->runaway -= promoteintwo;
					e->runaway += 5 * tmod[stones];
				}
				/* right side */
				if ((free&BIT6) && (free&BIT7) && (p->wm&BIT11)) // vtune: use &
				{
					e->runaway -= promoteintwo;
					e->runaway += 5 * tmod[stones];
				}
			}
		}

		// "other" bridge

		if (p->wm&SQ11)
		{
			if (match2(p->bm, free, SQ2 | SQ4, SQ3))
			{
				ungrounded_white |= SQ11;
				//whitehasbridge = 1;
				if (match2(p->wm, free, SQ12, SQ8))
				{
					e->runaway -= promoteintwo;
					e->runaway += 5 * tmod[stones];
				}
				if (match2(p->wm, free, SQ10, SQ6 | SQ7))
				{
					e->runaway -= promoteintwo;
					e->runaway += 5 * tmod[stones];
				}
			}
		}

		//
		// end runaway checkers
		//

		eval += e->runaway;

	} /* end stuff only with men */

	  // TODO:how about mobility eval?

	if (kings)
	{
		// 
		// king-only evaluation
		//


		// king center control 
		e->king += kcval*bitcount(p->bk&CENTER);
		e->king -= kcval*bitcount(p->wk&CENTER);

		// kings on edge 
		e->king += keval*bitcount(p->bk&EDGE);
		e->king -= keval*bitcount(p->wk&EDGE);

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

		if ((ki->untrappedwk & SQ4) && (p->bm&SQ3)) // vtune use &
		{
			if (free&SQ12)
			{
				ki->freewk--;
				e->king += kingtrappedinsinglecornerval;
				ki->untrappedwk ^= SQ4;
			}
		}

		if ((ki->untrappedbk & SQ29) && (p->wm&SQ30)) // vtune use &
		{
			if (free&SQ21)
			{
				ki->freebk--;
				e->king -= kingtrappedinsinglecornerval;
				ki->untrappedbk ^= SQ29;
			}
		}

		/* king assisted by man but trapped by two men in single corner*/
		/* should expand this to take care of situation where there is a man on 25
		going to king -> gets runaway bonus and makes potbk */
		if (ki->untrappedwk & (BIT0 | BIT4))
		{
			if (p->wm&BIT8)
			{
				if ((p->bm&(BIT1 | BIT5)) == (BIT1 | BIT5))
				{
					if (free & BIT12)
					{
						e->king += kingtrappedinsinglecornerbytwoval;
						ki->freewk--;
						ki->untrappedwk ^= (ki->untrappedwk&(BIT0 | BIT4));
					}
				}
			}
		}

		if (ki->untrappedbk & (BIT27 | BIT31))
		{
			if (p->bm&BIT23)
			{
				if ((p->wm&(BIT30 | BIT26)) == (BIT30 | BIT26))
				{
					if (free & BIT19) // maybe without this?
					{
						e->king -= kingtrappedinsinglecornerbytwoval;
						ki->freebk--;
						ki->untrappedbk ^= (ki->untrappedbk & (BIT27 | BIT31));
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
		if (ki->untrappedbk & SQ31)
		{
			if ((p->wm&SQ32) && (p->wm&SQ30) && (free&SQ23))
			{
				ki->freebk--;
				ki->untrappedbk ^= SQ31;
			}
		}

		if (ki->untrappedbk & SQ30)
		{
			if (((p->wm&SQ31) && (p->wm&SQ29) && (free&SQ22)) || ((p->wm&SQ21) && (p->wm&SQ25) && (p->wm&SQ31) && (free&SQ22)))
			{
				ki->freebk--;// above new 1.376 - add penalties here?
				ki->untrappedbk ^= SQ30;
			}

			// new in 1.41b - dynamic trapping of a king - if it moves, 25-22 will win it.
			else if (match1(p->wm, BIT27 | BIT23))
			{
				ki->freebk--;
				ki->untrappedbk ^= SQ30;
			}
		}

		if (ki->untrappedwk & SQ2)
		{
			if ((p->bm&SQ1) && (p->bm&SQ3) && (free&SQ10))// vtune use &
			{
				ki->freewk--;
				ki->untrappedwk ^= SQ2;
			}
		}

		if (ki->untrappedwk & SQ3)
		{											// below new 1.376
			if (((p->bm&SQ2) && (p->bm&SQ4) && (free&SQ11)) || ((p->bm&SQ2) && (p->bm&SQ8) && (p->bm&SQ12) && (free&SQ11)))
			{
				ki->freewk--;
				ki->untrappedwk ^= SQ3;
			}
			else if (match1(p->bm, BIT4 | BIT8))
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
		if (ki->untrappedbk & BIT28)
		{
			if ((p->wm&BIT24) && (p->wm&BIT29))
			{
				e->king -= kingtrappedindoublecornerval;
				ki->freebk--;
				ki->untrappedbk ^= BIT28;
			}
		}
		if (ki->untrappedwk & BIT3)
		{
			if ((p->bm&BIT2) && (p->bm&BIT7))
			{
				e->king += kingtrappedindoublecornerval;
				ki->freewk--;
				ki->untrappedwk ^= BIT3;
			}
		}


		/* single dominated white king in single corner new 1.35*/
		// eval weights not tested properly!
		/* special bad case: */

		if (p->wm&SQ12)
		{
			if (p->bm&SQ3)
			{
				/* special bad case: black king trapping wk on 4, and stopping a man on 20*/
				/* if this is true, the others cannot be true, because it requires a BK on bit9*/
				if (ki->untrappedwk&BIT0 && p->bk&BIT9 && p->wm&BIT16)// vtune use &
				{
					e->king += 50;
					ki->freewk--;
					ki->untrappedwk ^= BIT0;
				}

				if ((mc->wk == 1) && (ki->untrappedwk&(BIT0 | BIT4 | BIT9)) && (p->bk&(BIT17 | BIT18 | BIT12 | BIT13)))// vtune use &
				{
					e->king += dominatedkingval;
					ki->freewk--;
					ki->untrappedwk ^= (ki->untrappedwk&(BIT0 | BIT4 | BIT9));
				}
				if ((mc->wk == 0) && (p->wm&(BIT4)) && (p->bk&(BIT17 | BIT18 | BIT12 | BIT13)))
				{
					e->king += dominatedkingval;
				}
			}
		}
		/* single dominated black king in single corner */
		if (p->bm&SQ21)
		{
			if (p->wm&SQ30)
			{
				/* special bad case: white king trapping bk on 29, and stopping a man on 13*/
				if (p->wk&SQ22 && ki->untrappedbk&SQ29 && p->bm&SQ13)
				{
					e->king -= 50;
					ki->freebk--;
					ki->untrappedbk ^= SQ29;
				}
				if ((mc->bk == 1) && (ki->untrappedbk & (BIT31 | BIT27 | BIT22)) && (p->wk&(BIT13 | BIT14 | BIT18 | BIT19)))
				{
					e->king -= dominatedkingval;
					ki->freebk--;
					ki->untrappedbk ^= (p->bk&(BIT31 | BIT27 | BIT22));
				}
				if ((mc->bk == 0) && (p->bm&(BIT27)) && (p->wk&(BIT13 | BIT14 | BIT18 | BIT19)))
				{
					e->king -= dominatedkingval;
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
		if (p->bm&BIT2 && !(p->bm&BIT7))
		{
			if ((mc->wk == 1) && (ki->untrappedwk&(BIT3 | BIT7)) && (p->bk&BIT14) && !(p->wm&BIT15))
			{
				e->king += dominatedkingindcval;
				ki->freewk--;
				ki->untrappedwk ^= (ki->untrappedwk&(BIT3 | BIT7));
			}
			if ((mc->wk == 0) && (p->wm&(BIT7)) && (p->bk&BIT14) && !(p->wm&BIT15))
			{
				e->king += dominatedkingindcval;
			}
		}
		/* single dominated black king in double corner - new 1.373 */
		if (p->wm&BIT29 && !(p->wm&BIT24))
		{
			if ((mc->bk == 1) && (ki->untrappedbk &(BIT24 | BIT28)) && (p->wk&BIT17) && !(p->bm&BIT16))
			{
				e->king -= dominatedkingindcval;
				ki->freebk--;
				ki->untrappedbk ^= (ki->untrappedbk&(BIT24 | BIT28));
			}
			if ((mc->bk == 0) && (p->bm&(BIT24)) && (p->wk&BIT17) && !(p->bm&BIT16))
			{
				e->king -= dominatedkingindcval;
			}
		}

		// general king mobility: for all untrapped kings, look how far they
		// can go in 3 steps on non-attacked squares. and if they can go in center.
		// WATCH OUT: this destroys the info in ki->untrappedbk, it cannot be used any more
		// after this...

		while (ki->untrappedbk)
		{
			// peel off untrapped kings and analyze them
			tmp = (ki->untrappedbk&(ki->untrappedbk - 1)) ^ ki->untrappedbk;
			ki->untrappedbk ^= tmp;

			free2 = free | tmp;
			// free2 contains all free squares + the square of the potentially trapped king.

			attack = attack_backwardjump((p->wm | p->wk), free2);
			attack |= attack_forwardjump(p->wk, free2);
			// simulate 3 moves with king on tmp only over non-attacked squares
			tmp |= (forward(tmp) | backward(tmp));
			tmp &= (~attack);
			tmp &= free2;
			tmp |= (forward(tmp) | backward(tmp));
			tmp &= (~attack);
			tmp &= free2;
			tmp |= (forward(tmp) | backward(tmp));
			tmp &= (~attack);
			tmp &= free2;
			m = bitcount(tmp);
			if (!(tmp&CENTER))
				e->king -= 2;
			if (m <= 4)
				e->king -= (2 * (5 - m));
			if (m <= 1)
			{
				ki->freebk--;
				e->king -= 10;
			}
		}


		while (ki->untrappedwk)
		{
			// peel off untrapped kings and analyze them
			tmp = (ki->untrappedwk&(ki->untrappedwk - 1)) ^ ki->untrappedwk;
			ki->untrappedwk ^= tmp;
			free2 = free | tmp;
			attack = attack_forwardjump((p->bm | p->bk), free2);
			attack |= attack_backwardjump(p->bk, free2);
			// simulate 3 moves with king on tmp only over non-attacked squares
			tmp |= (forward(tmp) | backward(tmp));
			tmp &= free2;
			tmp &= (~attack);
			tmp |= (forward(tmp) | backward(tmp));
			tmp &= free2;
			tmp &= (~attack);
			tmp |= (forward(tmp) | backward(tmp));
			tmp &= free2;
			tmp &= (~attack);
			m = bitcount(tmp);
			if (!(tmp&CENTER))
				e->king += 2;
			if (m <= 4)
				e->king += (2 * (5 - m));
			if (m <= 1)
			{
				ki->freewk--;
				e->king += 10;
			}
		}

		// king center monopoly : only one side can occupy center
		if ((p->bk & CENTER) && !(p->wk&CENTER) && (ki->freebk >= ki->freewk))
			e->king += kingcentermonopoly;
		if ((p->wk & CENTER) && !(p->bk&CENTER) && (ki->freewk >= ki->freebk))
			e->king -= kingcentermonopoly;


		// only king: designed to make cake sac a man when it sees that it
		// gets a king for it, and retains a strong back rank
		/* single black king */
		if ((ki->freebk) && (ki->freewk == 0) && (potwk == 0))
		{
			// if black has sacced a man we want to extend this line:
			if (mc->bm == mc->wm - 2)
				*delta += NOPRUNEWINDOW;

			e->king += onlykingval;
			e->king += blackbackrankpower[p->bm & 0xFF];
			if (p->bk & ROAMINGBLACKKING)
				e->king += roamingkingval;
		}
		/* single white king */
		if ((ki->freewk) && (ki->freebk == 0) && (potbk == 0))
		{
			// if white has sacced a man for this we want to extend:
			if (mc->wm == mc->bm - 2)
				*delta += NOPRUNEWINDOW;

			e->king += -onlykingval;
			e->king -= whitebackrankpower[p->wm >> 24];
			if (p->wk & ROAMINGWHITEKING)
				e->king -= roamingkingval;
		}

		//
		// end king evaluation
		//

		eval += e->king;


		// 
		// king-man-interaction
		//

		if (pieces <= 12)
		{
			// TODO: why only pieces < 12 for tailhooks and ungrounded men? this looks really bad...
			// experimental kingcramper
			// but the match result with pieces <= 14 was worse....

			if (match3(p->bm, p->wm, p->bk, (SQ13), (SQ17 | SQ21 | SQ22 | SQ25), (SQ26)))
				e->king_man += 60;
			if (match3(p->wm, p->bm, p->wk, (SQ20), (SQ16 | SQ12 | SQ11 | SQ8), (SQ7)))
				e->king_man -= 60;


			// tailhooks 
			/* with 12 or with even less?*/

			m = ((0x37370000) & (p->bk) & (leftforward(p->wm)) & (twoleftforward(p->wm)));
			m |= ((0xECEC0000) & (p->bk) & (rightforward(p->wm)) & (tworightforward(p->wm)));
			blacktailhooks = bitcount(m);
			// new: don't penalize kings on edge if they tailhook
			e->king_man -= keval*bitcount(m&EDGE);

			m = ((0x0000ECEC) & (p->wk) & (rightbackward(p->bm)) & (tworightbackward(p->bm)));
			m |= ((0x00003737) & (p->wk) & (leftbackward(p->bm)) & (twoleftbackward(p->bm)));
			whitetailhooks = bitcount(m);
			e->king_man += keval*bitcount(m&EDGE);

			e->king_man += (blacktailhooks - whitetailhooks)*tailhookval;


			/* king proximity value */
			/* if a king is close to a man of the opposite color, that should
			be dangerous, at least if it is an "endangered man". */
			// 2-stage eval: ungrounded men next to kings get twice the penalty, 
			// ungrounded men one square further away get once the penalty.
			// TODO: why is this only evaluated with pieces <=12?
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

			e->king_man += kingproximity * kingproximityval;
		}


		/* endangered bridge heads */
		/* new 1.3 III */
		// TODO should read: if white king cannot get out from under bridge, then big penalty!
		/*if(whitehasbridge)
		{
		if(ki->freebk>ki->freewk)
		e->king_man += 2*endangeredbridgeval;
		if(ki->freebk == ki->freewk)   //else if freebk>0 do this not freebk==freewk
		e->king_man += endangeredbridgeval;
		}
		if(blackhasbridge)
		{
		if(ki->freebk < ki->freewk)
		e->king_man -= 2*endangeredbridgeval;
		if(ki->freebk == ki->freewk)
		e->king_man -= endangeredbridgeval;
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


		if (ending) // number of total stones <=10
		{
			if (ki->freebk >= ki->freewk)
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
				free2 = ~(p->bk | p->bm | p->wk | (p->wm&(~0x81818181))); // a free board without white men on edge
				while (tmp)
				{
					// for all black kings do:
					m = (tmp&(tmp - 1)) ^ tmp;
					tmp = tmp^m;
					// m now holds one black king.
					attack = attack_backwardjump(m, free2) | attack_forwardjump(m, free2);
					m1 = backward(p->wm & 0x81818181); // squares white men can move to
					m1 &= attack; // men which can't go because of this king
					m1 = bitcount(m1);
					if (m1 == 2)
						// one king holds two men!
						e->king_man += kingholdstwomenval;
					else if (m1 == 1)
						e->king_man += immobilemanval;
				}
			}

			if (ki->freewk >= ki->freebk)
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
				free2 = ~(p->bk | p->wm | p->wk | (p->bm&(~0x81818181))); // a free board without black men on edge
				while (tmp)
				{
					// for all black kings do:
					m = (tmp&(tmp - 1)) ^ tmp;
					tmp = tmp^m;
					// m now holds one black king.
					attack = attack_backwardjump(m, free2) | attack_forwardjump(m, free2);
					m1 = forward(p->bm & 0x81818181); // squares white men can move to
					m1 &= attack; // men which can't go because of this king
					m1 = bitcount(m1);
					if (m1 == 2)
						// one king holds two men!
						e->king_man -= kingholdstwomenval;
					else if (m1 == 1)
						e->king_man -= immobilemanval;
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
		if (mc->bm == mc->wm && mc->bm <= 7)
		{
			if (potbk && !potwk && blackbackrankpower[p->bm & 0xFF])
			{
				e->compensation += 20;   //10
			}
			if (potwk && !potbk && whitebackrankpower[p->wm >> 24])
			{
				e->compensation -= 20;
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

		if (mc->bm == mc->wm + 1)
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
			if (stones>12 && e->runaway <= 0) //many stones, black does not have a runaway checker
			{
				if (whitebackrankpower[p->wm >> 24] && e->runaway <0)
					// white has a good back rank and a runaway, then we want to look at this carefully
					*delta += NOPRUNEWINDOW;

				// TODO: this is crap: tempo changes on every move by +-1, so this term can 
				// oscillate between being on/off on every move. improve this by either making
				// tempo independent of sidetomove; or even better by making this term a function
				// of tempo.
				if (tempo>4 && whitebackrankpower[p->wm >> 24])
				{
					//*delta = NOPRUNEWINDOW;
					// tempo is >0, so black is advanced. this means we have
					// compensation. 
					e->compensation -= 2 * tempo;
					// check back ranks
					e->compensation -= whitebackrankpower[p->wm >> 24];
					e->compensation += blackbackrankpower[p->bm & 0xFF];
					// augment doghole values
					if (p->bm&BIT24)
						e->compensation -= 4;
					if (p->bm&BIT23)
						e->compensation -= 4;
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
			if (stones>12 && e->runaway >= 0) //1426
			{
				if (blackbackrankpower[p->bm & 0xFF] && e->runaway>0)
					*delta += NOPRUNEWINDOW;

				if (tempo<-4 && blackbackrankpower[p->bm & 0xFF])
				{
					//*delta = NOPRUNEWINDOW;
					// tempo is <0 because white is further advanced that black.
					e->compensation -= 2 * tempo; // black men less advanced than white -> good
												  // if black has a powerful back rank -> good
					e->compensation += blackbackrankpower[p->bm & 0xFF];
					// if white has a powerful backrank -> bad
					e->compensation -= whitebackrankpower[p->wm >> 24];
					if (p->wm&BIT8)
						e->compensation += 4;
					if (p->wm&BIT7)
						e->compensation += 4;
				}
			}
		}
		// end of compensation eval
		eval += e->compensation;
	}

	// turn 
	if (p->color == BLACK)
		turn += turnval;
	else
		turn -= turnval;
	eval += turn;

#ifdef SHOWEVAL
	if (immobilewhitemen + immobileblackmen)
	{
		printboard(*p);
		if (color == BLACK)
			printf("\nblack to move");
		else
			printf("\nwhite to move");
		printf("\nimmobile men: black %i   white %i", immobileblackmen, immobilewhitemen);
		getch();
	}
#endif

	// in endings: if weak side has N stones and N-1 kings, it is generally safe:
	// new in cake ss 102b; match result is +50-31 with and without, but eval seems better -> keep.
	if (ending && equal) // auf deutsch: 5-5 or 4-4.
	{
		if ((mc->wm - potwk <= 1) && (mc->bm - potbk <= 1))
			*likelydraw = 1;
	}

	return (p->color == BLACK) ? eval : -eval;
}



int initializematerial(short materialeval[13][13][13][13])
{
	// initialize material value array for value[bm][bk][wm][wk]
	int i, j, k, l;
	int v1, v2;

	for (i = 0; i<13; i++)
	{
		for (j = 0; j<13; j++)
		{
			for (k = 0; k<13; k++)
			{
				for (l = 0; l<13; l++)
				{
					/*bm bk wm wk */
					v1 = 100 * i + params[7] * j;
					v2 = 100 * k + params[7] * l;
					if (v1 + v2 == 0)
						continue;
					v1 = v1 - v2 + (EXBIAS*(v1 - v2)) / (v1 + v2);

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

	static int br[32] = { 0,0,2, 2, 4,6,10,10,1,4,16,16,6,10,24,16,
		0,0,2, 2, 4,10,16,16,1,4,16,16,6,10,24,16 };

	// contains the back rank evaluation 
	// strange: pdntool insists that the back rank 0 x 0 x (bridge) is worse than 0 x x 0 which
	//	leaves the double corner open
	// should try a change there! 
	POSITION p;
	int32 u;
	int32 index;



	const int devsinglecornerval = params[0];// 5;
	const int intactdoublecornerval = params[1]; // 5;

	const int oreoval = params[2]; // 8;

	const int idealdoublecornerval = params[3]; // 4;


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

	for (u = 0; u<256; u++)
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
			blackbackrankeval[u] += oreoval;
		}
		if (match1(p.wm, SQ31 | SQ30 | SQ26))
		{
			whitebackrankeval[u] += oreoval;
		}




		// developed single corner
		// TODO: make this more fine-grained: if men on 4 and 8, subtract something. if
		//		 only man on 4, but not on 8, subtract less
		//       if only man on 8 but not on 4 subract the least.

		if (((~p.bm)&SQ4) && ((~p.bm)&SQ8))
			blackbackrankeval[u] += devsinglecornerval; //devsinglecorner 5!!
		if (((~p.wm)&SQ29) && ((~p.wm)&SQ25))
			whitebackrankeval[u] += devsinglecornerval;//2!!


													   //  double corner evals: intact and developed 

		if (match1(p.bm, (SQ2 | SQ5 | SQ6)) && !(p.bm&SQ1))
			blackbackrankeval[u] += intactdoublecornerval;
		if (match1(p.wm, (SQ27 | SQ28 | SQ31)) && !(p.wm&SQ32))
			whitebackrankeval[u] += intactdoublecornerval;


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
		if ((p.bm&BIT3) && (p.bm&BIT2) && (p.bm&BIT6) && (!(p.bm&BIT7)))
			blackbackrankeval[u] += idealdoublecornerval;//2!!
														 // maybe also allow 2,6,7 &!3 to be ideal?
		if ((p.wm&BIT28) && (p.wm&BIT29) && (p.wm&BIT25) && (!(p.wm&BIT24)))
			whitebackrankeval[u] += idealdoublecornerval;//2!!

														 // new 1.41'
														 // maybe also allow 2,6,7 &!3 to be ideal?
		if ((p.bm&BIT7) && (p.bm&BIT2) && (p.bm&BIT6) && (!(p.bm&BIT3)))
			blackbackrankeval[u] += intactdoublecornerval;//2!!
		if ((p.wm&BIT24) && (p.wm&BIT29) && (p.wm&BIT25) && (!(p.wm&BIT28)))
			whitebackrankeval[u] += intactdoublecornerval;//2!!	



														  // backrankpower gives the power of the back rank to withstand onslaught
														  // by enemy men - only used in the case of man down.

														  // TODO: power is non-zero if black has a bridge for instance
														  // TODO: write this with squares to make sure it's readable.
		if (match1(p.bm, 0xE))
			blackbackrankpower[u] = params[4]; // 30;
		if (match1(p.bm, 0x2E) || match1(p.bm, 0x4E))
			blackbackrankpower[u] = params[5]; // 35;
		if (match1(p.bm, 0x6E))
			blackbackrankpower[u] = params[6]; // 40;

		if (match1(p.wm, 0x70000000))
			whitebackrankpower[u] = params[4]; // 30;
		if (match1(p.wm, 0x72000000) || match1(p.wm, 0x74000000))
			whitebackrankpower[u] = params[5]; // 35;
		if (match1(p.wm, 0x76000000))
			whitebackrankpower[u] = params[6];  40;
	}

	return 1;
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

	fp = fopen("C:\\Users\\Martin Fierz\\Documents\\Martin Fierz\\CheckerBoard\\games\\matches\\matchlog185f'_QS.txt", "r");
	while (!feof(fp)) {
		fgets(line, 255, fp);
		if (line[0] == 'a' && (line[10] == 'v' || line[33] == 'v')) {  // kingsrow
			printf("\n%s", line);
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
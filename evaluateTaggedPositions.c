#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <conio.h>
#include <Windows.h>
#include "structs.h"
#include "cakepp.h"
#include "consts.h"
#include "cake_misc.h"
#include "boolean.h"
#define BLACK 2
#define WHITE 1
#define MAN   4
#define KING  8
#define FREE 16
#define CC 3
//#define DEPTH1 17	//17
//#define DEPTH2 21	//21

#undef TESTBOOKGEN


int FENtoPosition( char *FEN, POSITION *p);

//#define MAXTESTPOS 100

FILE *cake_fp0;
FILE *fp2;
FILE *fp3;

main()
	{
	int play=0;
	POSITION p;
	int color=BLACK;
	int totalnodes=0;
	double totaltime=0;
	
	double start;
	double time1=0;
	double time2=0;
	extern int usethebook; // from cakepp.c
	extern char DBpath[256]; // from dblookup.c
	//char *c;

	SEARCHINFO si;

	char dirname[512];

	GetCurrentDirectory(512, dirname);
	sprintf(DBpath, "%s\\db", dirname);
	sprintf(DBpath, "C:\\Program Files\\CheckerBoard64\\db");


	// allocate memory for repcheck array
	si.repcheck = malloc((MAXDEPTH+HISTORYOFFSET) * sizeof(REPETITION));
	
	// reset all counters, nodes, database lookups etc 
	resetsearchinfo(&si);

	// turn off book
	usethebook = 0;

	printf("\nevaluateTaggedPositions 1.00");
	printf("\n27th April 2019");
	fflush(stdout);

	// if you want to use testcake to evaluate tagged positions then use this
	start = clock();
	evaluate_tagged_positions(); 
	time1 = clock() - start; 
	printf("\n\nTime: %.1fs - hit key to end", time1 / CLK_TCK);
	getch(); 
	return 1;
	}

	int evaluate_tagged_positions(void) {
		FILE* fp, *fpout; 
		int32 bm, bk, wm, wk; 
		int color, eval;
		POSITION p; 
		int play = 0; 
		char str[512];
		SEARCHINFO si;
		MATERIALCOUNT mc; 
		int n = 0; 
		int v0, v1, v3; 
		int delta; 
		int qsvalue; 
		
	
		si.repcheck = malloc((MAXDEPTH + HISTORYOFFSET) * sizeof(REPETITION));

		fp = fopen("c:\\code\\checkersdata\\taggedpositions.txt", "r");
		fpout = fopen("c:\\code\\checkersdata\\taggedevaluatedpositions.txt", "w");
		printf("\nloading...");
		while (!feof(fp)) {
			// load a position from file
			fscanf(fp, "%u %u %u %u %i %i\n", &bm, &bk, &wm, &wk, &color, &eval);

			// check that it is quiet = no captures
			p.bm = bm; p.bk = bk; p.wm = wm; p.wk = wk; p.color = color;
			if (testcapture(&p))
				continue; 
			p.color ^= CC; 
			if (testcapture(&p))
				continue; 
			p.color ^= CC; 

			// arriving here, we have a quiet position, evaluate!
			resetsearchinfo(&si);

			mc.bm = bitcount(p.bm);
			mc.bk = bitcount(p.bk);
			mc.wm = bitcount(p.wm);
			mc.wk = bitcount(p.wk);
			v0 = evaluation(&p, &mc, 0, &delta, 0, 0);

			//qsvalue = qsearch(&si, &p, v0, v0 + 50, 0);

			cake_getmove(&si, &p, 1, 1, 0, 10000, str, &play, 0, 0);
			v1 = si.spasuccess; 
			//printf("\nd1 %s", si.out);

			p.bm = bm; p.bk = bk; p.wm = wm; p.wk = wk; p.color = color;
			resetsearchinfo(&si);
			cake_getmove(&si, &p, 1, 1, 3, 10000, str, &play, 0, 0);
			v3 = si.spasuccess;
			//printf("\nd3 %s", si.out);


			//printf("\n%s", str);
			printf("\n(%i) eval is %i %i %i", n, v0,v1,v3);
			/*if (eval == WIN)
				printf(" (black won)");
			if (eval == DRAW)
				printf(" (draw)");
			if (eval == LOSS)
				printf(" (white won)");*/
			fprintf(fpout, "%u %u %u %u %i %i %i %i %i\n", bm, bk, wm, wk, color, eval, v0, v1, v3);
			/*if (abs(v0 - v1) > 50) {
				p.bm = bm; p.bk = bk; p.wm = wm; p.wk = wk; p.color = color;
				printboard(&p);
				getch(); 
			}*/
			n++; 

		}
		fclose(fp); 
		fclose(fpout); 
		return 0; 
	}





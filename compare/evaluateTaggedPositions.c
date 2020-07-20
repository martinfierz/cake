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

#include "optimizeCake.h"
//#define AVERAGE	// use files with average data for all positions


int FENtoPosition( char *FEN, POSITION *p);

//#define MAXTESTPOS 100

FILE *cake_fp0;
FILE *fp2;
FILE *fp3;

typedef struct
// struct hashentry needs 28 bytes 
{
	int32  lock;
	unsigned int best : 6;		// could be 5 bits
	int value : 12;				// value is 12 bits because +-MATE is 4000
	unsigned int color : 1;
	unsigned int ispvnode : 1;
	unsigned int depth : 10;		// could be 9 bits
	unsigned int valuetype : 2;
	unsigned int bm; 
	unsigned int bk; 
	unsigned int wm; 
	unsigned int wk; 
	unsigned int fom; 
	unsigned int wins; 
	unsigned int draws; 
	unsigned int losses; 
} EXTHASHENTRY;

main()
	{
	char str[1024]; 
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

	printf("\nevaluateTaggedPositions 1.01");
	printf("\n21st January 2020");
	fflush(stdout);

	// if you want to use testcake to evaluate tagged positions then use this
	start = clock();

#ifdef AVERAGE
	finduniquepositions(); 
#endif

	evaluate_tagged_positions(); 
	//initcake(str); 
	time1 = clock() - start; 
	printf("\n\nTime: %.1fs - hit key to end", time1 / CLK_TCK);
	getch(); 
	return 1;
	}

	int evaluate_tagged_positions(void) {
		FILE* fp, *fpout; 
		int32 bm, bk, wm, wk; 
		int color, eval;
		//int games, points; 
		int wins, draws, losses;
		POSITION p; 
		int play = 0; 
		char str[512];
		SEARCHINFO si;
		MATERIALCOUNT mc; 
		int n = 0; 
		int v0, v1, v3; 
		int delta; 
		int qsvalue; 
		double start, time; 
		
	
		si.repcheck = malloc((MAXDEPTH + HISTORYOFFSET) * sizeof(REPETITION));
#ifdef AVERAGE
		fp = fopen("c:\\code\\checkersdata\\uniquepositions.txt", "r");
		fpout = fopen("c:\\code\\checkersdata\\taggedevaluatedpositions_av.txt", "w");
#else
#ifdef NODUPLICATES
		fp = fopen("c:\\code\\checkersdata\\taggedpositions.txt", "r");
		fpout = fopen("c:\\code\\checkersdata\\taggedevaluatedpositions.txt", "w");
#else
		fp = fopen("c:\\code\\checkersdata\\taggedpositions+duplicates.txt", "r");
		fpout = fopen("c:\\code\\checkersdata\\taggedevaluatedpositions+duplicates.txt", "w");
#endif
#endif
		
		printf("\nloading...");

		start = clock(); 

		
		while (!feof(fp)) {
			// load a position from file
#ifdef AVERAGE
			fscanf(fp, "%u %u %u %u %i %i %i %i\n", &bm, &bk, &wm, &wk, &color, &wins, &draws, &losses);
#else
			fscanf(fp, "%u %u %u %u %i %i\n", &bm, &bk, &wm, &wk, &color, &eval);
#endif

			// check that it is quiet = no captures
			// TODO check if this is necessary, I think readmatchfile already throws these away!
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
			if(n % 1000 == 0)
				printf("\n(%i) eval is %i %i %i", n, v0,v1,v3);
			/*if (eval == WIN)
				printf(" (black won)");
			if (eval == DRAW)
				printf(" (draw)");
			if (eval == LOSS)
				printf(" (white won)");*/
#ifdef AVERAGE
			fprintf(fpout, "%u %u %u %u %i %i %i %i %i %i %i\n", bm, bk, wm, wk, color, wins, draws, losses, v0, v1, v3);
#else
			fprintf(fpout, "%u %u %u %u %i %i %i %i %i\n", bm, bk, wm, wk, color, eval, v0, v1, v3);
#endif		
			// show positions where eval is very wrong
			/*if (abs(v0 - v1) > 50) {
				p.bm = bm; p.bk = bk; p.wm = wm; p.wk = wk; p.color = color;
				printboard(&p);
				getch(); 
			}*/
			n++; 
		}
		time = ((clock() - start) / CLK_TCK);
		printf("\n%i positions analyzed in %.2f seconds", n, time);

		fclose(fp); 
		fclose(fpout); 
		return 0; 
	}

	int finduniquepositions(void) {
		FILE* fp, * fpout;
		int32 bm, bk, wm, wk;
		int color;
		POSITION p;
		int play = 0;
		char str[1024];
		SEARCHINFO si;
		MATERIALCOUNT mc;
		int i, n = 0;
		int v0, v1, v3;
		int delta;
		int qsvalue;
		double start, time;
		int stones[25];
		EXTHASHENTRY* hashtable; 
		int hashsize = 20000000; // 20 mio positions, x8 bytes = 160MB 
		int index; 
		int longest = 0; 
		int aborted = 0; 
		int j; 
		int numstones = 0, numkings = 0; 
		int result; 

		int allpositions = 0; 
		int uniquepositions = 0; 

		initcake(str); 

		for (i = 0; i < 25; i++)
			stones[i] = 0; 

		si.repcheck = malloc((MAXDEPTH + HISTORYOFFSET) * sizeof(REPETITION));
		hashtable = malloc((hashsize + 32) * sizeof(EXTHASHENTRY));
		for (i = 0; i < hashsize + 32; i++) {
			hashtable[i].lock = 0;
			hashtable[i].wins = 0;
			hashtable[i].draws = 0; 
			hashtable[i].losses = 0;
		}

#ifdef NODUPLICATES
		fp = fopen("c:\\code\\checkersdata\\taggedpositions.txt", "r");
#else
		fp = fopen("c:\\code\\checkersdata\\taggedpositions+duplicates.txt", "r");
#endif
		fpout = fopen("c:\\code\\checkersdata\\uniquepositions.txt", "w");
		printf("\nloading...");

		start = clock();


		while (!feof(fp)) {
			// load a position from file
			fscanf(fp, "%u %u %u %u %i %i\n", &bm, &bk, &wm, &wk, &color, &result);
						
			// check that it is quiet = no captures
			p.bm = bm; p.bk = bk; p.wm = wm; p.wk = wk; p.color = color;

			/*if (p.bk == 0x70000000 && ((p.bm &0x06000000) == 0x06000000)) {
				printboard(&p); 
				printf("\nfound in input file");
				getch();
			}*/

			
			if (testcapture(&p))
				continue;
			p.color ^= CC;
			if (testcapture(&p))
				continue;
			p.color ^= CC;

			
			// arriving here, we have a quiet position, store in hashtable!
			resetsearchinfo(&si);

			//mc.bm = bitcount(p.bm);
			//mc.bk = bitcount(p.bk);
			//mc.wm = bitcount(p.wm);
			//mc.wk = bitcount(p.wk);
			//numstones = mc.bm + mc.bk + mc.wm + mc.wk; 
			//numkings = mc.bk + mc.wk; 

			absolutehashkey(&p, &(si.hash));
			
			

			// store in hashtable
			index = si.hash.key % hashsize; 
			j = 0; 
			while (hashtable[index].lock != 0 ) {
				if (hashtable[index].lock == si.hash.lock)
					break; 
				index++; 
				j++; 
			}
			// we have found an index to write to
			if (hashtable[index].lock == 0 || hashtable[index].lock == si.hash.lock) {
				hashtable[index].lock = si.hash.lock;
				hashtable[index].bm = p.bm;
				hashtable[index].wm = p.wm;
				hashtable[index].bk = p.bk;
				hashtable[index].wk = p.wk;
				hashtable[index].color = (p.color >> 1);
				//hashtable[index].games++;
				if (result == DRAW)
					hashtable[index].draws++;
				if (result == WIN)
					hashtable[index].wins++;
				if (result == LOSS)
					hashtable[index].losses++;
				if (j > longest)
					longest = j;
				/*if (hashtable[index].draws > 100) {
					printboard(&p); 
					getch(); 

				}*/
			}
			else
				printf("\nhash error check code!!"); 

			
			if (n % 10000 == 0)
				printf("\n%i", n); 

			n++;
		}
		
		
		time = ((clock() - start) / CLK_TCK);
		printf("\n%i positions analyzed in %.2f seconds, longest = %i ",  n, time, longest);

		// now dump hashtable to disk

		for (i = 0; i < hashsize; i++) {
			if (hashtable[i].lock != 0) {
				p.bm = hashtable[i].bm; 
				p.wm = hashtable[i].wm; 
				p.wk = hashtable[i].wk; 
				p.bk = hashtable[i].bk; 
				p.color = 1 << hashtable[i].color;
				fprintf(fpout, "%u %u %u %u %i %i %i %i\n", p.bm, p.bk, p.wm, p.wk, p.color, hashtable[i].wins, hashtable[i].draws, hashtable[i].losses);
				uniquepositions++; 
				/*if (hashtable[i].games > 100) {
					printboard(&p); 
					printf("\n%i games, %i points", hashtable[i].games, hashtable[i].points); 
					getch(); 
				}*/
			}
		}

		printf("\n%i unique positions found in %i positions", uniquepositions, n);

		/*
		for (i = 0; i < 25; i++) {
			printf("\n%i: %i", i, stones[i]); 
		}

		allpositions = 0; 
		//for (n = 129; n >= 0; n--) {
		for (n = 0; n <129; n++) {
				j = 0;
			for (i = 0; i < hashsize; i++) {
				if (hashtable[i].lock != 0 && hashtable[i].fom == n) {
					j++;
					allpositions++; 
					//if (allpositions < 400000)
					//	continue; 
					p.bm = hashtable[i].bm; 
					p.wm = hashtable[i].wm; 
					p.bk = hashtable[i].bk; 
					p.wk = hashtable[i].wk; 
					p.color = 1<<hashtable[i].color; 
					//if(p.color == BLACK)
					//	printboard(&p); 

					// arriving here, we have a quiet position, evaluate!
					resetsearchinfo(&si);

					mc.bm = bitcount(p.bm);
					mc.bk = bitcount(p.bk);
					mc.wm = bitcount(p.wm);
					mc.wk = bitcount(p.wk);
					p.color = 1 << hashtable[i].color;
					v0 = evaluation(&p, &mc, 0, &delta, 0, 0);

					
					
					// TODO: make sure there is no abort time limit (done with info 16)
					p.bm = hashtable[i].bm; p.bk = hashtable[i].bk; p.wm = hashtable[i].wm; 
					p.wk = hashtable[i].wk; 
					p.color = 1 << hashtable[i].color;
					resetsearchinfo(&si);
					cake_getmove(&si, &p, TIME_BASED, 0.02, 3, 10000, str, &play, 0, 16);
					v3 = si.spasuccess;

					if (si.aborted) {
						aborted++;
						continue;
					}
					
					
					p.bm = hashtable[i].bm; p.bk = hashtable[i].bk; p.wm = hashtable[i].wm;
					p.wk = hashtable[i].wk; 
					p.color = 1 << hashtable[i].color;
					printf(" %i", allpositions);
					if (abs(v0 - v3) > 100) {
						printboard(&p);
						printf("\n(%i) static eval = %i, search = %i (d%i)", allpositions, v0, v3, si.depth);
						printf("\nd3 %s\n", si.out);
					}
					fprintf(fpout, "%u %u %u %u %i %i %i %i\n",p.bm, p.bk,p.wm, p.wk, p.color, v0, v3, si.depth);


				}
			}
			printf("\n%i positions with fom %i found", j, n); 
		}
		*/

		fclose(fp);
		fclose(fpout);
		return 0;
	}





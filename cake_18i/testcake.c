#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <conio.h>
#include "structs.h"
#include "cakepp.h"
#include "consts.h"
#include "cake_misc.h"
#define BLACK 2
#define WHITE 1
#define MAN   4
#define KING  8
#define FREE 16
#define CC 3

#undef TESTBOOKGEN


int FENtoPosition( char *FEN, POSITION *p);

#define MAXTESTPOS 100

FILE *cake_fp0;
FILE *fp2;
FILE *fp3;

main()
	{
	int i,n;
	int play=0;
	char str[1024];
	POSITION p;
	int color=BLACK;
	int totalnodes=0;
	double totaltime=0;
	FILE *testfile;
	char FEN[256];
	
	int newnodes[80][2];
	int oldnodes[80][2];
	int nodes17=0,allnodes17=0;
	int nodes23=0,allnodes23=0;
	double r17 = 1;
	double r21 = 1;
	double start;
	double time1=0;
	double time2=0;
	extern int usethebook; // from cakepp.c
	//char *c;

	SEARCHINFO si;


	/*
	[11 Man Ballot #1]
[FEN "B:W19,21,22,23,25,26,27,29,30,31,32:B1,2,3,4,5,6,7,9,10,11,12"]
{Remove 11 and 28; 8-11, 24-19}
{Remove 12 and 28; 8-12, 24-19}

[11 Man Ballot #2]
[FEN "B:W19,21,22,23,25,26,27,29,30,31,32:B1,2,3,4,5,6,8,9,10,11,12"]
{Remove 10 and 28; 7-10, 24-19}
{Remove 11 and 28; 7-11, 24-19}
*/
	// just using testcake for something else
	/*fp2 = fopen("11man.txt","r");
	fp3 = fopen("11man_reduced.txt","w");
	i = 0;
	while(!feof(fp2))
		{
		fscanf(fp2, "%s %s\n", FEN,str);
		if(FEN[1] == 'F')
			{
			i++;
			c = str+1;
			for(n=1; n<1000; n++)
				{
				if(c[n] == '"')
					{
					c[n] = 0;
					break;
					}
				}
			fprintf(fp3,"%s\t\t{11-man-ballot #%i}\n",c,i);
			}
		}
	fclose(fp2);
	fclose(fp3);
	_getch();*/

	// allocate memory for repcheck array
	si.repcheck = malloc((MAXDEPTH+HISTORYOFFSET) * sizeof(REPETITION));
	
	// reset all counters, nodes, database lookups etc 
	resetsearchinfo(&si);

	// turn off book
	usethebook = 0;

	printf("\ntestcake 2.1");
	printf("\n31st july 2007");
	fflush(stdout);

	testfile = fopen("testcake.txt","r");
	if(testfile == NULL)
	{
		printf("\nCould not open testcake.txt");
		_getch();
		exit(0);
	}

	printf("\ninitializing cake");
	fflush(stdout);
	initcake(str);

#ifdef TESTBOOKGEN
	fp2=fopen("booknodes.txt","r");
#else
	fp2=fopen("nodes.txt","r");
#endif

	if(fp2!=NULL)
		{
		for(i=0; i<MAXTESTPOS; i++)
			fscanf(fp2,"%i,%i\n",&oldnodes[i][0],&oldnodes[i][1]);
		fclose(fp2);
		}
	else
		{
		for(i=0;i<MAXTESTPOS;i++)
			{
			oldnodes[i][0]=100;
			newnodes[i][0]=100;
			}
		}
#ifdef TESTBOOKGEN
	fp2=fopen("newbooktest.txt","w");
#else
	fp2=fopen("newtest.txt","w");
#endif
	
	fprintf(fp2,"\npos\tratio17\tratio23");

	
	fp3=fopen("output.txt","w");
	
	////////////////////////////////////////////////////////////////////////
	//
	// main loop over all test positions
	//
	////////////////////////////////////////////////////////////////////////

	for(n=0; n<MAXTESTPOS; n++)
		{
		printf("\nposition #%i @ depth 17",n);
		/* load position */

		fscanf(testfile, "%s\n", FEN);
		if(feof(testfile))
			break;
		printf("\nPosition %i: %s",n,FEN);
		FENtoPosition(FEN, &p);


		printboard(&p);
		start = clock();

		// for profile
		//cake_getmove(&p,color,1,1,11,10000,str,&play,3,1);
		//exit(0);
		//play=0;
		//continue;	

#ifdef TESTBOOKGEN	
		bookgen(&p,color,&n,values,DEPTH_BASED,33,0);
		//cake_getmove(&p,color,1,1,13,10000,str,&play,3,1);
		//getch();
#else
		cake_getmove(&si, &p,1,1,17,10000,str,&play,3,1);
		//getch();
#endif
		time1 += (clock()-start);
		fprintf(fp3,"position %i\n%s\n",n,str);
		
		newnodes[n][0] = si.negamax;
		allnodes17 += si.negamax;
		totalnodes += si.negamax;
		printf("\nnew: %i  old %i nodes",newnodes[n][0],oldnodes[n][0]);
		printf("  %f\n",(float)si.negamax/(float)oldnodes[n][0]*100);


		
#ifndef TESTBOOKGEN
		printf("\nposition #%i @ depth 21",n);
		/*p.bm=testpos[i][0];
		p.bk=testpos[i][1];
		p.wm=testpos[i][2];
		p.wk=testpos[i][3];
		color=testpos[i][4];*/

		FENtoPosition(FEN, &p);
		
		printboard(&p);
		start = clock();
		cake_getmove(&si, &p, 1, 1, 21, 10000, str, &play,3,1);
		time2 += (clock()-start);
		fprintf(fp3,"%s\n\n",str);
		newnodes[n][1] = si.negamax;
		allnodes23 += si.negamax;
		totalnodes += si.negamax;
		printf("\nnew: %i  old: %i nodes",newnodes[n][1],oldnodes[n][1]);
		printf("  %f\n",(float)si.negamax/(float)oldnodes[n][1]*100);
#endif

		fprintf(fp2,"\n%2i \t%3.3f \t%3.3f",i,(float)newnodes[n][0]/(float)oldnodes[n][0]*100,(float)newnodes[n][1]/(float)oldnodes[n][1]*100);
		fflush(fp2);
		
		r17*=((double)newnodes[n][0]/(double)oldnodes[n][0]);
		r21*=((double)newnodes[n][1]/(double)oldnodes[n][1]);
		}

	printf("\n n is %i",n);

	fclose(testfile);

	r17 = pow(r17,((double)1.0/(double)n));
	r21 = pow(r21,((double)1.0/(double)n));
	
	sprintf(str,"\n\nRatio for depth 17: %2.4f",r17);
	fprintf(fp2,"%s",str);
	printf("%s",str);
	sprintf(str,"\n\nRatios for depth 21: %2.4f",r21);
	fprintf(fp2,"%s",str);
	printf("%s",str);
	
	sprintf(str,"\n\nTime: d17: %.1fs  d21: %.1fs",time1/CLK_TCK,time2/CLK_TCK);
	fprintf(fp2,"%s",str);
	printf("%s",str);
	
	sprintf(str,"\n\nSpeed: %3.1f kN/s\n\n",totalnodes/1000.0/((time1+time2)/CLK_TCK));
	fprintf(fp2,"%s",str);
	printf("%s",str);
	
	fclose(fp2);
	
	fp2 = fopen("newnodes.txt","w");
	
	for(i=0; i<n; i++)
		fprintf(fp2,"\n%i,%i",newnodes[i][0],newnodes[i][1]);

	fclose(fp2);   
	fclose(fp3);
	exitcake();
	return 1;
	}

	int FENtoPosition( char *FEN, POSITION *p)
	{
	/* parses the FEN string in *FEN and places the result in p and color */
	// example FEN string:
	// W:W32,31,30,29,28,27,26,25,24,22,21:B23,12,11,10,8,7,6,5,4,3,2,1.
	// returns 1 on success, 0 on failure.
	char *token;
	char *col,*white,*black;
	char FENstring[256];
	int i;
	int number;
	int32 one = 1;
	int piece;
	int length;
	char colorchar='x';
	
	
	// find the full stop in the FEN string which terminates it and 
	// replace it with a 0 for termination
	length = (int) strlen(FEN);
	token = FEN;
	i = 0;
	while(token[i] != '.' && i<length)
		i++;
	token[i] = 0;

	sprintf(FENstring,"%s",FEN);

	// detect empty FEN string
	if( strcmp(FENstring,"") == 0)
		return 0;

	/* parse color ,whitestring, blackstring*/
	col = strtok(FENstring,":");

	if(col == NULL)
		return 0;

	if(strcmp(col,"W")==0) 
		p->color = WHITE;
	else 
		p->color = BLACK;
	
	/* parse position: get white and black strings */
	
	white = strtok(NULL,":");
	if(white == NULL)
		return 0;

	// check whether this was a normal fen string (white first, then black) or vice versa.
	colorchar = white[0];
	if(colorchar == 'B' || colorchar == 'b')
		{
		black = white;
		white = strtok(NULL,":");
		if(white == NULL)
			return 0;
		// reversed fen string
		}
	else
		{
		black=strtok(NULL,":");
		if(black == NULL)
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
	token = strtok(white,",");

	while( token != NULL )
		{
		/* While there are tokens in "string" */
		/* a token might be 18, or 18K */
		piece = MAN;
		if(token[0] == 'K')
			{
			token++;
			piece = KING;
			}
		number = atoi(token);
		/* ok, piece and number found, transform number to coors */
		number = SquareToBit(number);
		
		if(piece == MAN)
			p->wm |= one<<number;
		else 
			p->wk |= one<<number;
		/* Get next token: */
		token = strtok( NULL, "," );
		}
	/* parse black string */
	token = strtok(black,",");
	while( token != NULL )
		{
		/* While there are tokens in "string" */
		/* a token might be 18, or 18K */
		piece = MAN;
		if(token[0] == 'K')
			{
			piece = KING;
			token++;
			}
		number = atoi(token);
		/* ok, piece and number found, transform number to coors */
		number = SquareToBit(number);
		if(piece == MAN)
			p->bm |= one<<number;
		else 
			p->bk |= one<<number;
		/* Get next token: */
		token = strtok( NULL, "," );
		}
	return 1;
	}



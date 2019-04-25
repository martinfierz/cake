/*		cake++ 1.22 - a checkers engine			*/
/*															*/
/*		Copyright (C) 2001 by Martin Fierz		*/
/*															*/
/*		contact: checkers@fierz.ch					*/

#include <stdio.h>
#include <time.h>
#include <math.h>
//#include <windows.h>
//#include <winbase.h>
#include "structs.h"
#include "cakepp.h"
#include "consts.h"
#define BLACK 2
#define WHITE 1
#define MAN   4
#define KING  8
#define FREE 16
#define CC 3
#define TESTPOS 79

//#define TESTBOOKGEN

extern int islegal(struct pos *position,int color, int from, int to);
extern unsigned int hashxors[2][4][32];

int InitBoard(int b[8][8]);
void boardtobitboard(int b[8][8], struct pos *position);
void bitboardtoboard(struct pos position,int b[8][8]);

#define OP 32
#define MID 34
#define END 13

int32 testpos[TESTPOS][5]=
	{  /* 32 opening positions */
	// test pos for rep detection
	//{0x00200aff, 0x00000000, 0xff811000, 0x00000000, 1},
	{0x00008cc0, 0x01000000, 0x206c0000, 0x00000001, 2},
	//{0xfff ,0x0 ,0xfff00000 ,0x0, 2},
	{0x4bff, 0x0 ,0xfff00000 ,0x0 ,1},
	{0x4bff, 0x0 ,0xffb80000 ,0x0 ,2},
	{0x4fdf, 0x0 ,0xffb80000, 0x0 ,1},
	{0x4fdf, 0x0 ,0xffb08000, 0x0 ,2},
	{0x6ddf, 0x0, 0xffb08000, 0x0,1},
	{0x6ddf, 0x0, 0xf7f08000, 0x0, 2},
	{0x6fcf, 0x0, 0xf7f08000, 0x0, 1},
	{0x6fcf, 0x0, 0xf7e18000, 0x0 ,2},
	{0x6fed, 0x0 ,0xf7e18000, 0x0 ,1},

	{0x6fed, 0x0, 0xf7a98000, 0x0 ,2},
	{0x4bff, 0x0, 0xffb40000, 0x0, 2},
	{0x5aff, 0x0, 0xffb40000, 0x0, 1},
	{0x5aff, 0x0, 0xffa50000, 0x0, 2},
	{0x5bef, 0x0, 0xffa50000, 0x0, 1},
	{0x5bef, 0x0, 0xfeb50000, 0x0, 2},
	{0x5fcf, 0x0, 0xfea70000, 0x0, 2},
	{0xd74f, 0x0, 0xfea30000, 0x0, 1},	
	{0xd74f, 0x0, 0xf6e30000, 0x0, 2},
	{0x2bff, 0x0, 0xfff00000 ,0x0, 1},

	{0x2bff, 0x0, 0xffb80000 ,0x0, 2},
	{0x20bff, 0x0, 0xffb80000, 0x0, 1},
	{0x401ff, 0x0, 0xff880000, 0x0, 1},
	{0x401ff, 0x0, 0xfba80000, 0x0, 2},
	{0x00002dff, 0x00000000, 0xfff00000, 0x00000000, 1},
	{0x00400dff, 0x00000000, 0xffb00000, 0x00000000, 1},
	{0x00000dff, 0x00000000, 0xf7b40000, 0x00000000, 2},
	{0x00000fef, 0x00000000, 0xf7b40000, 0x00000000, 1},
	{0x00000fef, 0x00000000, 0x7fb40000, 0x00000000, 2},
	{0x00000ffe, 0x00000000, 0x7fb40000, 0x00000000, 1},

	{0x00000ffe, 0x00000000, 0x7fb04000, 0x00000000, 2},
	{0x000003fe, 0x00000000, 0x7f104000, 0x00000000, 2},

	/* 23 midgame positions */
	{0x430f, 0x0, 0x53e00000, 0x0, 1},
	{0x430f,0x0, 0x52f00000, 0x0 ,2},
	{0x432d, 0x0, 0x52f00000, 0x0, 1},
	{0x325, 0x0, 0x50940000, 0x0 ,2},
	{0x420080, 0x40000000, 0x3004000, 0x4, 1},
	{0x420080, 0x40000000, 0x3004000, 0x20, 2},
	{0x4002a2, 0x50000000, 0xa1014000, 0x44, 1},
	{0x4002a2, 0x50000000, 0xa1014000, 0x804, 2},
	{0x4002a2, 0x18000000, 0xa1014000, 0x8004, 2},
	{0x4002a2, 0x10800000, 0xa1014000, 0x8004, 1},

	{0x4002a2, 0x10800000, 0xa1014000, 0x80004, 2},
	{0x400a22, 0x10800000, 0xa1014000, 0x80004, 1},
	{0x400222, 0x10004000, 0xa1010080, 0x4, 1},
	{0x400222, 0x10004000, 0xa1010000, 0xc, 2},
	{0x400222, 0x10040000, 0xa1010000, 0xc, 1}, 
	{0x0002621e, 0x00000000, 0x75980000, 0x00000000, 2},
	{0x0002621a, 0x00000000, 0x71d00000, 0x00000000, 2},
	{0x0006221a, 0x00000000, 0x71d00000, 0x00000000, 1},
	{0x0042221a, 0x00000000, 0x71980000, 0x00000000, 1},
	{0x0042221a, 0x00000000, 0x71904000, 0x00000000, 2},

	{0x0060221a, 0x00000000, 0x71904000, 0x00000000, 1},
	{0x0060221a, 0x00000000, 0x71184000, 0x00000000, 2},
	{0x0064001a, 0x00000000, 0x71084000, 0x00000000, 1},

	/* 11 man sac positions */
	{0x00014176, 0x00000000, 0x7580a000, 0x00000000, 1},
	{0x00016136, 0x00000000, 0x75808000, 0x00000000, 1},
	{0x00016136, 0x00000000, 0x75800800, 0x00000000, 2},
	{0x00016326, 0x00000000, 0x75800800, 0x00000000, 1},
	{0x00016326, 0x00000000, 0x75800080, 0x00000000, 2},
	{0x00016706, 0x00000000, 0x75800080, 0x00000000, 1},
	{0x0006272e, 0x00000000, 0x77890800, 0x00000000, 2},
	{0x000627a6, 0x00000000, 0x77890800, 0x00000000, 1},
	{0x00062fa2, 0x00000000, 0x77890000, 0x00000000, 1},
	{0x00062fa2, 0x00000000, 0x77818000, 0x00000000, 2},
	{0x000667a2, 0x00000000, 0x77818000, 0x00000000, 1},

	/* 13 endings */	
	{0x00070402, 0x00000000, 0x07800000, 0x00000001, 1},
	{0x00070402, 0x00000000, 0x07080000, 0x00000001, 2},
	{0x00430402, 0x00000000, 0x07080000, 0x00000001, 1},
	{0x00010402, 0x20000000, 0x01081000, 0x00000001, 2},
	{0x00010402, 0x04000000, 0x01081000, 0x00000001, 1},
	{0x00010402, 0x00040000, 0x01000900, 0x00000001, 2},
	{0x00010402, 0x00002000, 0x01000900, 0x00000001, 1},
	{0x00010402, 0x00002000, 0x01000900, 0x00000010, 2},
	{0x00014002, 0x00002000, 0x01000900, 0x00000010, 1},
	{0x00644008, 0x00000000, 0x41801000, 0x00000004, 2},
	
	//{0x04444008, 0x00000000, 0x41801000, 0x00000004, 1},
	//{0x00444008, 0x00000000, 0x01a01000, 0x00000004, 2},
	{0x02404008, 0x00000000, 0x01801000, 0x00000004, 1},
	{0x02404008, 0x00000000, 0x01801000, 0x00000020, 2},
	{0x00404008, 0x10000000, 0x01801000, 0x00000020, 1}
	};



FILE *cake_fp0;
FILE *fp2;
FILE *fp3;
main()
	{
	int board[8][8];
	int i;
	int n;
	int play=0;
	char str[1024];
	struct pos p;
	int color=BLACK;
	extern int cake_nodes;
	int totalnodes=0;
	double totaltime=0;
	int *xors;
	int values[MAXMOVES];
	
	int newnodes[80][2];
	int oldnodes[80][2];
	int nodes13=0,allnodes13=0;
	int nodes19=0,allnodes19=0;
	double r13_op=1, r13_mid=1, r13_end=1;
	double r19_op=1, r19_mid=1, r19_end=1;
	double start;
	double time13=0;
	double time19=0;

//	unsigned int sectorspercluster,bytespersector,numberoffreeclusters,totalnumberofclusters;	
	printf("\ntestcake 2.0");
	printf("\n23rd may 2001");
	fflush(stdout);


//	GetDiskFreeSpace(NULL, &sectorspercluster, &bytespersector, &numberoffreeclusters, &totalnumberofclusters);
//	printf("\nspc %i bps %i nfc %i tnc %i",sectorspercluster,bytespersector,numberoffreeclusters,totalnumberofclusters);
//	getch();
	/* initialize board */
	InitBoard(board);
	boardtobitboard(board,&p);
	printf("\ninitializing cake");
	fflush(stdout);
	initcake(2,str);

#ifdef TESTBOOKGEN
	fp2=fopen("booknodes.txt","r");
#else
	fp2=fopen("nodes.txt","r");
#endif

	if(fp2!=NULL)
		{
		for(i=0;i<TESTPOS;i++)
			fscanf(fp2,"%i,%i\n",&oldnodes[i][0],&oldnodes[i][1]);
		fclose(fp2);
		}
	else
		{
		for(i=0;i<TESTPOS;i++)
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
	
	fprintf(fp2,"\npos\tratio13\tratio19");

	
	fp3=fopen("output.txt","w");
	xors=hashxors;
	
	////////////////////////////////////////////////////////////////////////
	//
	// main loop over all test positions
	//
	////////////////////////////////////////////////////////////////////////

	for(i=0;i<TESTPOS;i++)
		{
		printf("\nposition #%i @ depth 13",i);
		/* load position */
		p.bm=testpos[i][0];
		p.bk=testpos[i][1];
		p.wm=testpos[i][2];
		p.wk=testpos[i][3];
		color=testpos[i][4];
		printboard(p);
		start=clock();

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
		cake_getmove(&p,color,1,1,13,10000,str,&play,3,1);
		//getch();
#endif
		time13+=(clock()-start);
		fprintf(fp3,"position %i\n%s\n",i,str);
		
		newnodes[i][0]=cake_nodes;
		allnodes13+=cake_nodes;
		totalnodes+=cake_nodes;
		printf("\nnew: %i  old %i nodes",newnodes[i][0],oldnodes[i][0]);
		printf("  %f\n",(float)cake_nodes/(float)oldnodes[i][0]*100);


		
#ifndef TESTBOOKGEN
		printf("\nposition #%i @ depth 19",i);
		p.bm=testpos[i][0];
		p.bk=testpos[i][1];
		p.wm=testpos[i][2];
		p.wk=testpos[i][3];
		color=testpos[i][4];
		printboard(p);
		start=clock();
		cake_getmove(&p,color,1,1,19,10000,str,&play,3,1);
		time19+=(clock()-start);
		fprintf(fp3,"%s\n\n",str);
		newnodes[i][1]=cake_nodes;
		allnodes19+=cake_nodes;
		totalnodes+=cake_nodes;
		printf("\nnew: %i  old: %i nodes",newnodes[i][1],oldnodes[i][1]);
		printf("  %f\n",(float)cake_nodes/(float)oldnodes[i][1]*100);
#endif

		fprintf(fp2,"\n%2i \t%3.1f \t%3.1f",i,(float)newnodes[i][0]/(float)oldnodes[i][0]*100,(float)newnodes[i][1]/(float)oldnodes[i][1]*100);
		fflush(fp2);
		
		
		if(i<OP)
			{
			r13_op*=((double)newnodes[i][0]/(double)oldnodes[i][0]);
			r19_op*=((double)newnodes[i][1]/(double)oldnodes[i][1]);
			}
		if(i>=OP && i<OP+MID)
			{
			r13_mid*=((double)newnodes[i][0]/(double)oldnodes[i][0]);
			r19_mid*=((double)newnodes[i][1]/(double)oldnodes[i][1]);
			}
		if(i>=OP+MID)
			{
			r13_end*=((double)newnodes[i][0]/(double)oldnodes[i][0]);
			r19_end*=((double)newnodes[i][1]/(double)oldnodes[i][1]);
			}
		}
	r13_op=pow(r13_op,((double)1.0/(double)OP));
	r13_mid=pow(r13_mid,((double)1.0/(double)MID));
	r13_end=pow(r13_end,((double)1.0/(double)END));
	
	r19_op=pow(r19_op,((double)1.0/(double)OP));
	r19_mid=pow(r19_mid,((double)1.0/(double)MID));
	r19_end=pow(r19_end,((double)1.0/(double)END));
	
	
	sprintf(str,"\n\nRatios for depth 13: op: %2.2f mid %2.2f end %2.2f",r13_op,r13_mid,r13_end);
	fprintf(fp2,"%s",str);
	printf("%s",str);
	sprintf(str,"\n\nRatios for depth 19: op: %2.2f mid %2.2f end %2.2f",r19_op,r19_mid,r19_end);
	fprintf(fp2,"%s",str);
	printf("%s",str);
	
	sprintf(str,"\n\nTime: d13: %.1fs  d19: %.1fs",time13/CLK_TCK,time19/CLK_TCK);
	fprintf(fp2,"%s",str);
	printf("%s",str);
	
	sprintf(str,"\n\nSpeed: %3.1f kN/s\n\n",totalnodes/1000.0/((time13+time19)/CLK_TCK));
	fprintf(fp2,"%s",str);
	printf("%s",str);
	
	fclose(fp2);
	
	fp2=fopen("newnodes.txt","w");
	for(i=0;i<TESTPOS;i++)
		fprintf(fp2,"\n%i,%i",newnodes[i][0],newnodes[i][1]);
	fclose(fp2);   
	fclose(fp3);
	exitcake();
	return 1;
	}


int InitBoard(int b[8][8])
	{
   /* initialize board */
   int i,j;
   for(i=0;i<=7;i++)
   	{
      for(j=0;j<=7;j++)
      	{
         b[i][j]=FREE;
         }
      }
   b[0][0]=BLACK|MAN;
   b[2][0]=BLACK|MAN;
   b[4][0]=BLACK|MAN;
   b[6][0]=BLACK|MAN;
   b[1][1]=BLACK|MAN;
   b[3][1]=BLACK|MAN;
   b[5][1]=BLACK|MAN;
   b[7][1]=BLACK|MAN;
   b[0][2]=BLACK|MAN;
   b[2][2]=BLACK|MAN;
   b[4][2]=BLACK|MAN;
   b[6][2]=BLACK|MAN;

   b[1][7]=WHITE|MAN;
   b[3][7]=WHITE|MAN;
   b[5][7]=WHITE|MAN;
   b[7][7]=WHITE|MAN;
   b[0][6]=WHITE|MAN;
   b[2][6]=WHITE|MAN;
   b[4][6]=WHITE|MAN;
   b[6][6]=WHITE|MAN;
   b[1][5]=WHITE|MAN;
   b[3][5]=WHITE|MAN;
   b[5][5]=WHITE|MAN;
   b[7][5]=WHITE|MAN;

   return 1;
	}

void bitboardtoboard(struct pos position,int b[8][8])
	{
	/* return a board from a bitboard */
   int i,board[32];

   for(i=0;i<32;i++)
   	{
      if (position.bm & (1<<i))
      	board[i]=(BLACK|MAN);
      if (position.bk & (1<<i))
      	board[i]=(BLACK|KING);
      if (position.wm & (1<<i))
      	board[i]=(WHITE|MAN);
      if (position.wk & (1<<i))
      	board[i]=(WHITE|KING);
      if ( (~(position.bm|position.bk|position.wm|position.wk)) & (1<<i))
      	board[i]=0;
      }
   /* return the board */
   b[0][0]=board[0];b[2][0]=board[1];b[4][0]=board[2];b[6][0]=board[3];
	b[1][1]=board[4];b[3][1]=board[5];b[5][1]=board[6];b[7][1]=board[7];
	b[0][2]=board[8];b[2][2]=board[9];b[4][2]=board[10];b[6][2]=board[11];
	b[1][3]=board[12];b[3][3]=board[13];b[5][3]=board[14];b[7][3]=board[15];
	b[0][4]=board[16];b[2][4]=board[17];b[4][4]=board[18];b[6][4]=board[19];
	b[1][5]=board[20];b[3][5]=board[21];b[5][5]=board[22];b[7][5]=board[23];
	b[0][6]=board[24];b[2][6]=board[25];b[4][6]=board[26];b[6][6]=board[27];
	b[1][7]=board[28];b[3][7]=board[29];b[5][7]=board[30];b[7][7]=board[31];
   }

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

   (*position).bm=0;
   (*position).bk=0;
   (*position).wm=0;
   (*position).wk=0;

   for(i=0;i<32;i++)
   	{
      switch (board[i])
      	{
         case BLACK|MAN:
            (*position).bm=(*position).bm|(1<<i);
            break;
         case BLACK|KING:
         	(*position).bk=(*position).bk|(1<<i);
         	break;
         case WHITE|MAN:
         	(*position).wm=(*position).wm|(1<<i);
            break;
         case WHITE|KING:
         	(*position).wk=(*position).wk|(1<<i);
            break;
         }
      }
	}



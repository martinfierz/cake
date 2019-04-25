/* ansicake */

/* demonstrates how to use the cake++ routines */

#include <stdio.h>
#include <time.h>
#include <math.h>
#include "structs.h"
#include "cakepp.h"
#include "consts.h"
#define BLACK 2
#define WHITE 1
#define MAN   4
#define KING  8
#define FREE 16
#define CC 3

extern int islegal(struct pos *position,int color, int from, int to);
int InitBoard(int b[8][8]);
void boardtobitboard(int b[8][8], struct pos *position);
void bitboardtoboard(struct pos position,int b[8][8]);

int32 testpos[80][4]={  {0XFFF,0,0XFFF00000,0},
						{0X2DFF,0,0XFFF00000,0},
                        {0X2DFF,0,0XFF780000,0},
                        {0X65FF,0,0XFF780000,0},
                        {0X65FF,0,0XF7F80000,0},
                        {0X67EF,0,0XF7F80000,0},
						{0X67EF,0,0XF7F08000,0},
                        {0X75EF,0,0XF7F08000,0},
                        {0X75EF,0,0XF7E18000,0},
                        {0X255CF,0,0XF7E08000,0},
                        {0X255CF,0,0XF5F08000,0},
                        {0X251DE,0,0XF5E08000,0},
						{0X251DE,0,0XD7E08000,0},
                        {0X253CE,0,0XD7E08000,0},
                        {0X253CE,0,0X5FE08000,0},
                        {0X25B4E,0,0X5FE08000,0},
                        {0X25B4E,0,0X5FA88000,0},
                        {0X25F0A,0,0X5FA80000,0},
                        {0X25F0A,0,0X5FA08000,0},
                        {0X27702,0,0X5FA00000,0},
                        {0X27702,0,0X5DB00000,0},
                        {0X27720,0,0X5DB00000,0},
                        {0X27720,0,0X4FB00000,0},
                        {0X36620,0,0X4F900000,0},
                        {0X36620,0,0X47D00000,0},
                        {0X37420,0,0X47D00000,0},
                        {0X37420,0,0X47980000,0},
						{0X73420,0,0X47980000,0},
                        {0X73420,0,0X47908000,0},
                        {0X77020,0,0X47908000,0},
                        {0X77020,0,0X47900800,0},
                        {0X77200,0,0X47900800,0},
						{0X77200,0,0X47900040,0},
                        {0X73200,0,0X47104040,0},
                        {0X33200,0,0X45144040,0},
						{0X2023200,0,0X45044040,0},
                        {0X2023200,0,0X41444040,0},
                        {0X2203200,0,0X41444040,0},
                        {0X2203200,0,0X41440840,0},
                        {0X203200,0X20000000,0X41440840,0},
						{0X203200,0X20000000,0X41404840,0},
                        {0X203200,0X4000000,0X41404840,0},
                        {0X203200,0X4000000,0X41084840,0},
                        {0X221200,0X4000000,0X41084840,0},
                        {0X221200,0X4000000,0X41080C40,0},
						{0X221200,0X400000,0X41080C40,0},
                        {0X221200,0X400000,0X41004C40,0},
                        {0X221200,0X80000,0X41004C40,0},
                        {0X221200,0X80000,0X41004C00,0X4},
                        {0X230200,0X80000,0X41004C00,0X4},
						{0X230200,0X80000,0X41004440,0X4},
                        {0X232000,0X80000,0X41004440,0X4},
                        {0X232000,0X80000,0X41004440,0X20},
						{0X310000,0X80000,0X40004440,0X20},
                        {0X310000,0X80000,0X40004440,0X200},
                        {0X2210000,0X80000,0X40004440,0X200},
                        {0X2210000,0X80000,0X40004400,0X204},
                        {0X210000,0X20080000,0X40004400,0X204},
						{0X210000,0X20080000,0X40004400,0X1004},
                        {0X300000,0X20080000,0X40004400,0X1004},
                        {0X300000,0X20080000,0X40004400,0X1040},
                        {0X1200000,0X20080000,0X40004400,0X1040},
                        {0X1200000,0X20080000,0X40004400,0X1800},
						{0X3000000,0X20080000,0X40004400,0X1800},
                        {0X3000000,0X20080000,0X40004400,0X9000},
                        {0X3000000,0X20800000,0X40004400,0X9000},
                        };

int org_n[80]={77864851,109607679,62742300,64272387,72601674,
			  77942272,84342810,57969445,24383553,25299556,21247913,
           31031879,27045368,11700711,11442786,1591013,
           1108211,2816483,2390537,
           704276,1088026,296817,
           671366,2557237,345386,
           102833,120719,196402,681126,328919,
           1684814,927738,1207808,993442,
           5323809,5473119,18390284,26439700,

           18803256,20598184,15165472,10067191,17372349,
           26750824,11076862,28270068,44952554,42185821,
           18226853,26205798,9146728,10801032,15838490,
           88219747,52545569,108640073,
           66009501,157325726,110551051,173438785,82941888,
           159628145,102351024,43436380,30699232,35149766};
           /* old data cake++ just so */




FILE *cake_fp0;
FILE *fp2;
main()
	{
   int board[8][8];
   int i;
   int play=0;
   char str[2550];
   struct pos p;
   int color=BLACK;
   extern int cake_nodes;
   int newnodes[80][2];
   int oldnodes[80][2];
   int nodes13=0,allnodes13=0;
   int nodes19=0,allnodes19=0;
   double ratio13=1;
   double logratio13=0;
   double ratio19=1;
   double logratio19=0;
   double start;
   
   printf("\ntestcake 1.00");
   printf("\n18th december 2000");
   fflush(stdout);

   
/* for a profile:		  p.bm=testpos[0][0];
      p.bk=testpos[0][1];
      p.wm=testpos[0][2];
      p.wk=testpos[0][3];
	  cake_getmove(&p,color,1,1,11,10000,str,&play,3,1);
	  exit(0);*/
   /* initialize board */
   InitBoard(board);
   boardtobitboard(board,&p);
   printf("\ninitializing cake");
   fflush(stdout);
   initcake(2);
	fp2=fopen("nodes.txt","r");
   for(i=0;i<64;i++)
	fscanf(fp2,"%i,%i\n",&oldnodes[i][0],&oldnodes[i][1]);
   fclose(fp2);
   /* create a logfile */
   /*cake_fp0=fopen("testpos.txt","w");
   for(i=0;i<100;i++)
   	{
      fprintf(cake_fp0,"%x %x %x %x\n",p.bm,p.bk,p.wm,p.wk);
      printf("\n%i",i);
      printboard(p);
      cake_getmove(&p,color,0,30,19,str,&play,2,1);
      color=color^CC;
      newnodes[i]=nodes;
      }
   fclose(cake_fp0);
   cake_fp0=fopen("nodes.txt","w");
   for(i=0;i<100;i++)
   	fprintf(cake_fp0,"%i ",newnodes[i]);
   fclose(cake_fp0); */
   /* test with crashing cake */
   /*p.bk=33685504;
   p.bm=806354950;
   p.wk=2147483648;
   p.wm=8388864;
   printboard(p);
   //getch();
   p.bm=0x4000446;
   p.bk=0x8024000;
   p.wm=0x889000;
   p.wk=0x8;
   printboard(p);
   cake_getmove(&p,WHITE,1,1,27,10000,str,&play,2,1);
   getch();*/
   /* end test */
   /* test singular extensions */
      /*
       WHITE                WHITE
   	28  29  30  31      32  31  30  29
	 24  25  26  27      28  27  26  25
	   20  21  22  23      24  23  22  21
	 16  17  18  19      20  19  18  17
	   12  13  14  15      16  15  14  13
	  8   9  10  11      12  11  10   9
	    4   5   6   7       8   7   6   5
	  0   1   2   3       4   3   2   1
	      BLACK
*/

  // for(i=1;i<11;i++)
  // 	cake_gettree(p,WHITE,i);
   //printboard(p);
   //getch();
   p.bm=BIT3+BIT0+BIT7+BIT5+BIT20;
   p.bk=0;
   p.wm=BIT6+BIT14+BIT16+BIT22+BIT31;
   p.wk=0;

   //cake_getmove(&p,WHITE,1,1,11,10000,str,&play,3,1);
   //getch();
   /* end test */

   /* test islegal
   if(islegal(&p,BLACK, 11, 15))
   	printboard(p);
   else
   	printf("11-15 is not legal");
   getchar();

      p.bm=0x4000446;
   p.bk=0x8024000;
   p.wm=0x889000;
   p.wk=0x8;
   printboard(p);
    test islegal
   if(islegal(&p,BLACK, 11, 15))
   	printboard(p);
   else
   	printf("11-15 is not legal");
   getchar(); */

   fp2=fopen("test.txt","w");
   fprintf(fp2,"\npos\tratio13\tratio19");
   start=clock();
   for(i=0;i<64;i++)
   	{
      printf("\nposition #%i",i);
	  p.bm=testpos[i][0];
      p.bk=testpos[i][1];
      p.wm=testpos[i][2];
      p.wk=testpos[i][3];

		cake_getmove(&p,color,1,1,13,10000,str,&play,3,1);
	  newnodes[i][0]=cake_nodes;
	  allnodes13+=cake_nodes;
      printf("\nnew: %i  old %i nodes",newnodes[i][0],oldnodes[i][0]);
	  printf("  %f\n",(float)cake_nodes/(float)oldnodes[i][0]*100);
      	
	  p.bm=testpos[i][0];
      p.bk=testpos[i][1];
      p.wm=testpos[i][2];
      p.wk=testpos[i][3];
      cake_getmove(&p,color,1,1,19,10000,str,&play,3,1);
	  newnodes[i][1]=cake_nodes;
	  allnodes19+=cake_nodes;
	  color=color^CC;
      printf("\nnew: %i  old: %i nodes",newnodes[i][1],oldnodes[i][1]);
      printf("  %f\n",(float)cake_nodes/(float)oldnodes[i][1]*100);
      //printf("\n%2i \t%3.1f \t%3.1f",i,(float)newnodes[i][0]/(float)oldnodes[i][0]*100,(float)newnodes[i][1]/(float)oldnodes[i][1]*100);
      fprintf(fp2,"\n%2i \t%3.1f \t%3.1f",i,(float)newnodes[i][0]/(float)oldnodes[i][0]*100,(float)newnodes[i][1]/(float)oldnodes[i][1]*100);
      fflush(fp2);
	  //exit(0);
      
      
      ratio13*=((double)newnodes[i][0]/(double)oldnodes[i][0]);
	  ratio19*=((double)newnodes[i][1]/(double)oldnodes[i][1]);
      logratio13+=log((double)newnodes[i][0]/(double)oldnodes[i][0]);
	  logratio19+=log((double)newnodes[i][1]/(double)oldnodes[i][1]);
      }
   //printf("\noverall: new:%i ref:%i ratio:%f",allnodes,refallnodes,(float)allnodes/(float)refallnodes);
   //fprintf(fp2,"\noverall: new:%i ref:%i ratio:%f",allnodes,refallnodes,(float)allnodes/(float)refallnodes);
   //printf("\nratio:%f logratio:%f",allratio,logratio);
   fprintf(fp2,"\nratio:13:%2.2f 19:%2.2f\tlogratio:13:%2.2f 19:%2.2f",ratio13,ratio19,logratio13,logratio19);

   ratio13=pow(ratio13,((double)1.0/(double)64.0));
   logratio13=logratio13/64;
   logratio13=exp(logratio13);
   ratio19=pow(ratio19,((double)1.0/(double)64.0));
   logratio19=logratio19/64;
   logratio19=exp(logratio19);
   printf("\ngeometric mean:%f or %f (13), %f or %f (19)",ratio13,logratio13,ratio19, logratio19);
   fprintf(fp2,"\ngeometric mean:%f or %f (13), %f or %f (19)",ratio13,logratio13,ratio19, logratio19);
   fprintf(fp2,"\ntime:%f",(clock()-start)/CLK_TCK);
   fclose(fp2);
   fp2=fopen("newnodes.txt","w");
   for(i=0;i<64;i++)
   	fprintf(fp2,"\n%i,%i",newnodes[i][0],newnodes[i][1]);
   fclose(fp2);   
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



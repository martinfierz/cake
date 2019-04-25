/* cakedll - is the dll interface to cake++.c */

#include <windows.h>
#include <stdio.h>

#include "switches.h"  /* hashtable size is in here */
#include "structs.h" /*cake data structures*/
#include "cakepp.h" /* cake function prototypes */
#include "consts.h"
/* return values */
#define DRAW 0
#define WIN 1
#define LOSS 2
#define UNKNOWN 3
#define WINLEVEL 160 /* 160*/
#define D 21
#undef D
void boardtobitboard(int b[8][8], struct pos *position);
void bitboardtoboard(struct pos position,int b[8][8]);


int  	WINAPI getmove(int b[8][8],int color, double maxtime, char str[255], int *playnow, int info, int unused, struct CBmove *move);
int 	WINAPI engineoptions(HWND hwnd);
int 	WINAPI enginename(char str[255]);
int 	WINAPI enginehelp(char str[255]);
int 	WINAPI engineabout(HWND hwnd);

HINSTANCE hInst; /*instance of the dll */
/*-------------- PART 1: dll stuff -------------------------------------------*/


#define LOG 1

BOOL WINAPI DllEntryPoint (HANDLE hDLL, DWORD dwReason, LPVOID lpReserved)
	 {

    switch (dwReason)
        {
        case DLL_PROCESS_ATTACH:
            initcake(LOG);
            hInst=hDLL; /* initialize handle */
            break;
        case DLL_PROCESS_DETACH:
            exitcake();
            break;
        case DLL_THREAD_ATTACH:
        		break;
        case DLL_THREAD_DETACH:
        		break;
        default:
            break;
        }
    return TRUE;
    }

int WINAPI staticevaluation(int b[8][8],int color,char str[1024])
	{
   struct pos position;

   boardtobitboard(b,&position);
   sprintf(str,"cake++ does not support static evaluation");
   //staticeval(position,color,str);
   return 1;
   }

int WINAPI enginename(char str[255])
	{
	sprintf(str,"Cake++ 1.2b");
	return 1;
   }

int WINAPI enginehelp(char str[255])
	{
   /* here you are supposed to return a string with the name of a html file */
   /* checkerboard will attempt to open this file with the default .htm browser */
   /* use this file to give the user extensive information, links to your homepage etc */
  	sprintf(str,"cakepp.htm");
   return 1;
   }

int WINAPI engineabout(HWND hwnd)
	{
   /* here you should display a messagebox with an "about" to your engine */
   extern int maxNdb;
   double ram;
   char msg[1000];
   char ETCstring[100]="",PVSstring[100]="",SEstring[100]="";

   initcake(LOG);
   ram=(float)(HASHSIZEDEEP+HASHSIZESHALLOW)*sizeof(struct hashentry)/(float)(1<<20);
   #ifdef ETC
   	sprintf(ETCstring,"\nusing ETC");
   #endif
   #ifdef PVS
   	sprintf(PVSstring,"\nusing PVS");
   #endif
   #ifdef SINGULAREXTENSIONS
   	sprintf(SEstring,"\nusing singular extensions");
   #endif
   sprintf(msg,"Cake++ 1.2 \nxmas 2000 by Martin Fierz\n\nAcknowledgements:\n\nAI programming support:\nFabian Mäser, Bernhard Seybold.\n\nBitboard code inspiration:\nBob Hyatt's 'Crafty' chess program\nJonathan Schaeffers 'Chinook' checkers program\nArthur Samuel's paper on his checkers program\n\nThis program uses the Chinook endgame databases.\nThx to the Chinook people for making them available!\nUsing %i-piece databases\nUsing %3.1fMB for the hashtable%s%s%s\n\nThanks to Ed Gilbert for programming\nKingsRow. Without a strong sparring\npartner Cake++ would be much weaker\n\nCake++ uses an MTD(f) alphabeta algorithm\nwith the following additional enhancements:\n-> windowing\n-> quiescense extensions\n-> transposition table\n-> static move ordering\n-> killer moves\n-> history heuristics\n-> forward pruning\n-> enhanced transposition cutoffs\n-> evaluation coarsegraining",maxNdb,ram,ETCstring,PVSstring,SEstring);
   MessageBox(hwnd,msg,"About Cake++",MB_OK);
   return 1;
   }

int 	WINAPI engineoptions(HWND hwnd)
	{
   /* here you can include a dialog box with search options of your engine. for
   	dialog boxes you need an instance handle, so you have to get this in libmain. */
   MessageBox(hwnd,"There are no options yet","engine options",MB_OK);
   return 1;
   }


int WINAPI getmove(int b[8][8],int color, double maxtime, char str[255], int *playnow, int info, int unused, struct CBmove *CBMove)
	{

   /* getmove is what checkerboard calls. you get 6 parameters:
   b[8][8] 	is the current position. the values in the array are determined by
   			the #defined values of BLACK, WHITE, KING, MAN. a black king for
            instance is represented by BLACK|KING.
   color		is the side to make a move. BLACK or WHITE.
   maxtime	is the time your program should use to make a move. this is about
   			40% of what you put in as level in checkerboard. so if you exceed
            this time it's not too bad - just don't exceed it too much...
   str		is a pointer to the output string of the checkerboard status bar.
   			you can use sprintf(str,"information"); to print any information you
            want into the status bar.
   *playnow	is a pointer to the playnow variable of checkerboard. if the user
   			would like your engine to play immediately, this value is nonzero,
            else zero. you should respond to a nonzero value of *playnow by
            interrupting your search IMMEDIATELY. */
   struct pos p;
	int value;
   int dbresult;
   int i;
   int how=0;
   int maxdepth=0;
   extern int maxNdb;
#ifdef REPCHECK
	extern struct pos Ghistory[MAXDEPTH+HISTORYOFFSET+10]; /*holds the current variation for repetition check*/
   int rep=0;
#endif
#ifdef D
   how=1;
   maxdepth=D;
#endif
   boardtobitboard(b,&p);
   value=cake_getmove(&p,color,how,maxtime,maxdepth,0,str,playnow,LOG,(info&1));
   /* p is the bitboard position, color the side to move, 0 is 'how to search',
   	0 means time-based, maxtime is the average time to use, 0 is the depthtosearch
      parameter which would be used if how was 1, cake++ prints info to str,
      returns if playnow is !=0 and does logging according to LOG. and does a reset
      if this is requested (info bit 1 set)*/
   bitboardtoboard(p,b);

   if(value>WINLEVEL) return WIN;
   if(value<-WINLEVEL) return LOSS;

   /* new in 1.14 */
   if(bitcount(p.bm)+bitcount(p.bk)+bitcount(p.wm)+bitcount(p.wk)<=maxNdb)
   		{
		if((bitcount(p.bm|p.bk)>0) && (bitcount(p.wm|p.wk)>0))
			{
			if(!testcapture(color) && !testcapture(color^CC))
   				{
				dbresult=DBLookup(p,(color)>>1);
   				if(dbresult==DRAW)
   					return DRAW;
				}
			}
		}
   /* end new in 1.14 */
   	
#ifdef REPCHECK
	/* return a draw in case of repetition */
   /* new from 1.192 on */
   for(i=HISTORYOFFSET-2;i>=0;i--)
   	{
      if(Ghistory[i].bm==p.bm && Ghistory[i].wm==p.wm && Ghistory[i].bk==p.bk && Ghistory[i].wk==p.wk)
      	rep++;
      }
	if(rep>1) return DRAW;
#endif
   return UNKNOWN;
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





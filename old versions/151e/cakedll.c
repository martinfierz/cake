/* cakedll - is the dll interface to cake.c */

#include <windows.h>
#include <stdio.h>
#include <time.h>

#include "switches.h"  /* hashtable size is in here */
#include "structs.h" /*cake data structures*/
#include "cakepp.h" /* cake function prototypes */
#include "consts.h"
//#include "jenkinshash.h"
#include "dblookup.h"

/* return values */
#define DRAW 0
#define WIN 1
#define LOSS 2
#define UNKNOWN 3
#define WINLEVEL 160 /* 160*/
#define D 21
#undef D

/* ---------------------> exported functions ----------------------------------------------------*/
int  	WINAPI getmove(int b[8][8],int color, double maxtime, char str[255], int *playnow, int info, int unused, struct CBmove *move);
int	WINAPI enginecommand(char command[256], char reply[256]);

/* -------------------->end exported functions --------------------------------------------------*/

void boardtobitboard(int b[8][8], struct pos *position);
void bitboardtoboard(struct pos position,int b[8][8]);
void FENtoboard8(int board[8][8], char *p, int *color);
void numbertocoors(int number,int *x, int *y);
int staticevaluation(struct pos *p,int color,int *total, int *material, int *positional);



/* user-definable engine options */
extern int hashmegabytes;
extern int usethebook;
static int cake_is_init=0;
static int dll_is_init=0;

HINSTANCE hInst; /*instance of the dll */
/*-------------- PART 1: dll stuff -------------------------------------------*/


#define LOG 1

BOOL WINAPI DllEntryPoint (HANDLE hDLL, DWORD dwReason, LPVOID lpReserved)
	 {
	

    switch (dwReason)
        {
        case DLL_PROCESS_ATTACH:
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



int WINAPI enginecommand(char str[256], char reply[256])
	{
	char command[256],param1[256],param2[256];
	extern int maxNdb;
	extern int hashsize;
	double ram;
	char ETCstring[100]="",PVSstring[100]="",SEstring[100]="";
	HKEY hKey;
	unsigned long result;
	DWORD datasize,datatype;
	char version[20]="";
	int board8[8][8];
	int color;
	struct pos p;
	int staticeval=0;
	int total, material, positional;

	if(!dll_is_init)
		{
		sprintf(version,VERSION);
		RegCreateKeyEx(HKEY_CURRENT_USER,"Software\\Martin Fierz\\Cake",0,"CB_Key",0,KEY_ALL_ACCESS,NULL,&hKey,&result);
		if(result == REG_OPENED_EXISTING_KEY)
			{
			/* load settings from last session */
			datasize=sizeof(int);
			result=RegQueryValueEx(hKey,"Hashsize",NULL,&datatype,(LPBYTE)&hashmegabytes,&datasize);
			result=RegQueryValueEx(hKey,"Use Book",NULL,&datatype,(LPBYTE)&usethebook,&datasize);
			RegSetValueEx(hKey,"Version",0,REG_SZ,(LPBYTE)version,strlen(version)+1);
			}
		if(result == REG_CREATED_NEW_KEY)
			{
			/* set default settings */
			RegSetValueEx(hKey,"Hashsize",0,REG_DWORD,(LPBYTE)&hashmegabytes,sizeof(int));
			RegSetValueEx(hKey,"Use Book",0,REG_DWORD,(LPBYTE)&usethebook,sizeof(int));
			RegSetValueEx(hKey,"Version",0,REG_SZ,(LPBYTE)version,strlen(version)+1);
			}
		RegCloseKey(hKey);

		dll_is_init=1;
		}

		
	sscanf(str,"%s %s %s",command,param1,param2);

	// check for command keywords 

	if(strcmp(command,"name")==0)
		{
		sprintf(reply,"Cake %s", VERSION);
		
#ifdef EIGHT
		strcat(reply," 8-piece");
#endif		
#ifdef SIX
		strcat(reply," 6-piece");
#endif		

#ifdef MPC
		strcat(reply," MPC");
#endif
#ifdef MTD
		strcat(reply," MTD");
#else
		strcat(reply," windowed");
#endif
#ifdef PVS
		strcat(reply," PVS");
#endif
		return 1;
		}

	if(strcmp(command,"about")==0)
		{
		if(!cake_is_init)
			{
			initcake(LOG, reply);
			cake_is_init = 1;
			}
		ram=(float)(hashsize)*sizeof(struct hashentry)/(float)(1<<20);
		sprintf(reply,"Cake 'las Vegas' (1.50)\nAugust 2002 by Martin Fierz\n\nUsing %i-piece databases\nUsing %3.1fMB for the hashtable\n\n",maxNdb,ram);
		return 1;
		}

	if(strcmp(command,"help")==0)
		{
		sprintf(reply,"cakelv.htm");
		return 1;
		}
	if(strcmp(command,"staticevaluation")==0)
		{
		FENtoboard8(board8,param1,&color);	
		boardtobitboard(board8,&p);
		staticeval = staticevaluation(&p,color,&total,&material,&positional);
		sprintf(reply,"static evaluation is: \nTotal: %i\nMaterial: %i\nPositional: %i",total, material, positional);
		return 1;
		}

	if(strcmp(command,"set")==0)
		{
		if(strcmp(param1,"hashsize")==0)
			{
			hashmegabytes = atoi(param2);
			hashreallocate(hashmegabytes);
			sprintf(reply,"hash size set to %i",hashmegabytes);
			RegCreateKeyEx(HKEY_CURRENT_USER,"Software\\Martin Fierz\\Cake",0,"CB_Key",0,KEY_ALL_ACCESS,NULL,&hKey,&result);
			RegSetValueEx(hKey,"Hashsize",0,REG_DWORD,(LPBYTE)&hashmegabytes,sizeof(int));
			RegCloseKey(hKey);
			return 1;
			}
		if(strcmp(param1,"book")==0)
			{
			usethebook = atoi(param2);
			RegCreateKeyEx(HKEY_CURRENT_USER,"Software\\Martin Fierz\\Cake",0,"CB_Key",0,KEY_ALL_ACCESS,NULL,&hKey,&result);
			RegSetValueEx(hKey,"Use Book",0,REG_DWORD,(LPBYTE)&usethebook,sizeof(int));
			RegCloseKey(hKey);
			return 1;
			}
		if(strcmp(param1,"allscores")==0)
			{
			return 0;
			}
		}
	
	if(strcmp(command,"get")==0)
		{
		if(strcmp(param1,"hashsize")==0)
			{
			sprintf(reply,"%i",hashmegabytes);
			return 1;
			}
		if(strcmp(param1,"book")==0)
			{
			sprintf(reply,"%i",usethebook);
			return 1;
			}
		if(strcmp(param1,"protocolversion")==0)
			{
			sprintf(reply,"2");
			return 1;
			}
		if(strcmp(param1,"gametype")==0)
			{
			sprintf(reply,"21");
			return 1;
			}
		if(strcmp(param1,"allscores")==0)
			{
			sprintf(reply,"0");
			return 0;
			}
		}


	return 0;
	}



int WINAPI getmove(int b[8][8],int color, double maxtime, char str[1024], int *playnow, int info, int unused, struct CBmove *CBMove)
	{

   /* getmove is what checkerboard calls. you get 6 parameters:
   b[8][8] 	is the current position. the values in the array are determined by
   			the #defined values of BLACK, WHITE, KING, MAN. a black king for
            instance is represented by BLACK|KING.

   color		is the side to make a move. BLACK or WHITE.

   maxtime	is the time your program should use to make a move. this is about
   			
   str		is a pointer to the output string of the checkerboard status bar.
   			you can use sprintf(str,"information"); to print any information you
            want into the status bar.

   *playnow	is a pointer to the playnow variable of checkerboard. if the user
   			would like your engine to play immediately, this value is nonzero,
            else zero. you should respond to a nonzero value of *playnow by
            interrupting your search IMMEDIATELY. */

	// info currently has uses for it's first 3 bits:
	// info&1 means reset
	// info&2 means exact time level
	// info&4 means increment time level
	struct pos p;
	int value;
   int dbresult;
   int i;
   int how=0;
   int maxdepth=0;
#ifdef USEDB
   extern int maxNdb;
#endif

#ifdef REPCHECK
	extern struct pos Ghistory[MAXDEPTH+HISTORYOFFSET+10]; /*holds the current variation for repetition check*/
   int rep=0;
#endif

#ifdef D
   how=1;
   maxdepth=D;
#endif

	if(!cake_is_init)
		{
		initcake(LOG,str);
		cake_is_init = 1;
		}

   boardtobitboard(b,&p);


   value=cake_getmove(&p,color,how,maxtime,maxdepth,0,str,playnow,LOG,info);
   /* p is the bitboard position, color the side to move, 0 is 'how to search',
   	0 means time-based, maxtime is the average time to use, 0 is the depthtosearch
      parameter which would be used if how was 1, cake prints info to str,
      returns if playnow is !=0 and does logging according to LOG. and does a reset
      if this is requested (info bit 1 set)*/
   bitboardtoboard(p,b);

   if(value>WINLEVEL) return WIN;
   if(value<-WINLEVEL) return LOSS;

#ifdef USEDB
   /* new in 1.14 */
   if(recbitcount(p.bm)+recbitcount(p.bk)+recbitcount(p.wm)+recbitcount(p.wk)<=maxNdb)
   		{
		if((recbitcount(p.bm|p.bk)>0) && (recbitcount(p.wm|p.wk)>0))
			{
			if(!testcapture(&p,color) && !testcapture(&p,color^CC))
   				{

				if(color == BLACK)
					dbresult = dblookup(&p,DB_BLACK,0);
				else
					dbresult = dblookup(&p, DB_WHITE,0);
				if(dbresult == DB_DRAW)
					return DRAW;
				}
			}
		}
   /* end new in 1.14 */
#endif
	
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




void FENtoboard8(int board[8][8], char *p, int *color)
	{
	/* parses the FEN string in *p and places the result in board8 and color */
	char *token;
	char *col,*white,*black;
	char FENstring[256];
	int i,j;
	int number;
	int piece;


	/* reset board */
	for(i=0;i<8;i++)
		{
		for(j=0;j<8;j++)
			board[i][j]=0;
		}
	
	sprintf(FENstring,"%s",p);
	/* parse color ,whitestring, blackstring*/
	col=strtok(FENstring,":");
	if(strcmp(col,"W")==0) *color=WHITE;
	else *color=BLACK;
	
	/* parse position: get white and black strings */
	
	white=strtok(NULL,":");
	black=strtok(NULL,":");
	white++;
	black++;
	
	/* parse white string */
	token=strtok(white,",");

	while( token != NULL )
		{
		/* While there are tokens in "string" */
		/* a token might be 18, or 18K */
		piece=WHITE|MAN;
		if(token[0]=='K')
			{
			piece=WHITE|KING;
			token++;
			}
		number=atoi(token);
		/* ok, piece and number found, transform number to coors */
		numbertocoors(number,&i,&j);
		board[i][j]=piece;
		/* Get next token: */
      token = strtok( NULL, "," );
		}
	/* parse black string */
	token=strtok(black,",");
   while( token != NULL )
		{
      /* While there are tokens in "string" */
		/* a token might be 18, or 18K */
		piece=BLACK|MAN;
		if(token[0]=='K')
			{
			piece=BLACK|KING;
			token++;
			}
		number=atoi(token);
		/* ok, piece and number found, transform number to coors */
		numbertocoors(number,&i,&j);
		board[i][j]=piece;
		/* Get next token: */
      token = strtok( NULL, "," );
		}
	}


void numbertocoors(int number,int *x, int *y)
	{
   /* given a board number this function returns the coordinates */
	number--;
	*y=number/4;
	*x=2*(3-number%4);
	if((*y) % 2)
		(*x)++;
	return;
   }
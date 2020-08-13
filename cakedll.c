/* cakedll - is the dll interface to cake.c */

#include <windows.h>
#include <stdio.h>
#include <time.h>

#include "switches.h"  /* hashtable size is in here */
#include "structs.h" /*cake data structures*/
#include "cakepp.h" /* cake function prototypes */
#include "consts.h"
#include "dblookup.h"
#include "initcake.h"

/* ---------------------> exported functions ----------------------------------------------------*/
int	WINAPI getmove(int b[8][8],int color, double maxtime, char str[1024], int *playnow, int info, int unused, struct CBmove *move);
int	WINAPI enginecommand(char command[256], char reply[256]);

/* -------------------->end exported functions --------------------------------------------------*/

void boardtobitboard(int b[8][8], POSITION *p);
void bitboardtoboard(POSITION *p,int b[8][8]);
void FENtoboard8(int board[8][8], char *p, int *color);
void numbertocoors(int number,int *x, int *y);
int staticevaluation(SEARCHINFO *si, EVALUATION *e, POSITION *p,int *total, int *material, int *positional, int*delta);

/* user-definable engine options */
extern int hashmegabytes;
extern int dbmegabytes;
extern int usethebook;
extern char DBpath[256];  // from dblookup

static int cake_is_init=0;
static int dll_is_init=0;
static int allscores = 0;

// for detection of repetition draws

static REPETITION repdetect[HISTORYOFFSET+2];

HINSTANCE hInst; /*instance of the dll */
/*-------------- PART 1: dll stuff -------------------------------------------*/

#define LOG 1

BOOL __stdcall WINAPI DllMain (HANDLE hDLL, DWORD dwReason, LPVOID lpReserved)
	{
	switch (dwReason)
        {
        case DLL_PROCESS_ATTACH:
            // initialize dll handle		
			hInst=hDLL; 
			break;
        case DLL_PROCESS_DETACH:
			// this is called by freelibrary?!
            exitcake();
			break;
        case DLL_THREAD_ATTACH:
			// this would be called if a new thread of the same dll starts
			// the right place to initialize a new searchinfo structure...
        	break;
        case DLL_THREAD_DETACH:
			//exitcake();
        	break;
        default:
            break;
        }
    return TRUE;
    }


int __stdcall WINAPI enginecommand(char str[256], char reply[256])
	{
	char command[256],param1[256],param2[256];
	char Lstr[1024];
	extern int maxNdb;
	extern int hashsize;
	extern int bookmovenum;
	char ETCstring[100]="",PVSstring[100]="",SEstring[100]="";
	static char version[100];
	HKEY hKey;
	unsigned long result;
	DWORD datasize,datatype;
	int board8[8][8];
	POSITION p;
	int staticeval=0;
	int total, material, positional;
	int delta = 0;
	int registry_error;
	SEARCHINFO si;
	EVALUATION e;
	char *s;
	int dbresult;
	//FILE* fp; 

	if(!dll_is_init)
		{
		sprintf(version,VERSION);
		// regcreatekeyex returns ERROR_SUCCESS if successful
		registry_error = RegCreateKeyEx(HKEY_CURRENT_USER,"Software\\Martin Fierz\\Cake",0,"CB_Key",0,KEY_ALL_ACCESS,NULL,&hKey,&result);
		if(registry_error == ERROR_SUCCESS)
			{
			if(result == REG_OPENED_EXISTING_KEY)
				{
				/* load settings from last session */
				datasize=sizeof(int);
				result=RegQueryValueEx(hKey,"Hashsize",NULL,&datatype,(LPBYTE)&hashmegabytes,&datasize);
				result=RegQueryValueEx(hKey,"DBcachesize",NULL,&datatype,(LPBYTE)&dbmegabytes,&datasize);
				result=RegQueryValueEx(hKey,"Use Book",NULL,&datatype,(LPBYTE)&usethebook,&datasize);
				result=RegQueryValueEx(hKey,"All Scores",NULL,&datatype,(LPBYTE)&allscores,&datasize);
				datasize = 256;
				result=RegQueryValueEx(hKey,"DBpath",NULL,&datatype,(LPBYTE)DBpath,&datasize);
				RegSetValueEx(hKey,"Version",0,REG_SZ,(LPBYTE)version,strlen(version)+1);
				}
			if(result == REG_CREATED_NEW_KEY)
				{
				/* set default settings */
				RegSetValueEx(hKey,"Hashsize",0,REG_DWORD,(LPBYTE)&hashmegabytes,sizeof(int));
				RegSetValueEx(hKey,"DBcachesize",0,REG_DWORD,(LPBYTE)&dbmegabytes,sizeof(int));
				RegSetValueEx(hKey,"Use Book",0,REG_DWORD,(LPBYTE)&usethebook,sizeof(int));
				RegSetValueEx(hKey,"All Scores",0,REG_DWORD,(LPBYTE)&allscores,sizeof(int));
				RegSetValueEx(hKey,"Version",0,REG_SZ,(LPBYTE)version,strlen(version)+1);
				RegSetValueEx(hKey,"DBpath",0,REG_SZ,(LPBYTE)DBpath, strlen(DBpath)+1);
				}
			RegCloseKey(hKey);
			}
		dll_is_init=1;
		}
	
	// check for command keywords 
	sscanf(str,"%s %s %s",command,param1,param2);
	if(strcmp(command,"name")==0)
		{
#ifdef FIXEDDEPTH
		sprintf(reply,"Cake %s%i", VERSION,FIXEDDEPTH);
#else
		sprintf(reply,"Cake %s", VERSION);
#endif
		return 1;
		}

	if(strcmp(command,"exit")==0)
		{
		exitcake();
		sprintf(reply,"exit!");
		return 1;
		}

	if(strcmp(command,"initcake")==0)
		{
		initcake(Lstr);
		sprintf(reply,"cake is initialized");
		return 1;
		}

	if(strcmp(command,"about")==0)
		{

		if(!cake_is_init)
			{
			sprintf(reply,"Cake %s\nJanuary 2020 by Martin Fierz\n\nEngine not initialized yet!",version);
			return 1;
			}
		sprintf(reply,"Cake %s\nJanuary 2020 by Martin Fierz\n\nUsing %i MB for database cache\nUsing %i MB for the hashtable\n\nCompile options:",version,db_getcachesize()/1024,hashmegabytes);
#ifdef ETC
		strcat(reply,"\n    - ETC");
#endif
#ifdef MTD
		strcat(reply,"\n    - MTD");
#endif
#ifdef IITERD
		strcat(reply,"\n    - IID");
#endif
#ifdef EXTENDPV
		strcat(reply,"\n    - XPV");
#endif
#ifdef QSEARCH
		strcat(reply, "\n    - QS");
#endif
#ifdef LATEMOVEREDUCTION
		strcat(reply,"\n    - LMR");
#endif

#ifdef MPC
		strcat(reply,"\n    - MPC");
#endif
#ifdef PVS
		strcat(reply,"\n    - PVS");
#endif
#ifdef LEARNUSE
		strcat(reply,"\n    - LRN");
#endif
#ifdef SPA
		strcat(reply,"\n    - SPA");
#endif

		strcat(reply, "\n\noptimized with 8279k 135p"); 
		// get book info
		sprintf(Lstr,"\n\n%i moves in opening book",bookmovenum);
		strcat(reply,Lstr);

		// get endgame database info
		sprintf(Lstr,"\n\nUsing %i-piece databases",maxNdb);
		strcat(reply,Lstr);
		db_infostring(Lstr);
		strcat(reply, Lstr);

		return 1;
		}

	if(strcmp(command,"help")==0)
		{
		sprintf(reply,"cake188.htm");
		return 1;
		}

	if(strcmp(command,"staticevaluation")==0)
		{
		FENtoboard8(board8,param1,&p.color);	
		boardtobitboard(board8,&p);
		memset(&e, 0, sizeof(EVALUATION));
		staticeval = staticevaluation(&si, &e, &p,&total,&material,&positional,&delta);
		dbresult = dblookup(&p, 0, &(si.matcount));
		sprintf(reply, "evaluation from point of view of side to move is:\nTotal:\t%i\nMaterial:\t%i\nMen:\t%i\nBack Rank:\t%i\nRunaway:\t%i\nCramp:\t%i\nHold:\t%i\nKing:\t%i\nSelftrap:\t%i\nKing-Man:\t%i\nCompensation:\t%i\nDatabase:\t%i",staticeval, e.material, e.men, e.backrank, e.runaway, e.cramp, e.hold, e.king, e.selftrap, e.king_man, e.compensation, dbresult);
		return 1;
		}

	// new in cake 1.801: perft: usage perft FEN depth
	if(strcmp(command, "perft") == 0)
		{
		FENtoboard8(board8,param1,&p.color);	
		boardtobitboard(board8,&p);
		delta = perft(&si, &p, atoi(param2));
		sprintf(reply, "Perft returned %i", delta); 
		return 1;
		}

	if(strcmp(command,"set")==0)
		{
		if(strcmp(param1,"dbpath") == 0)
			{
			// ugly method to skip to the DB path after set dbpath <path> - because <path> can have whitespace the
			// usual method with param2 doesn't work - just skip 11 characters.
			s = str+11;
			sprintf(DBpath,"%s",s);
			registry_error = RegCreateKeyEx(HKEY_CURRENT_USER,"Software\\Martin Fierz\\Cake",0,"CB_Key",0,KEY_ALL_ACCESS,NULL,&hKey,&result);
			if(registry_error != ERROR_SUCCESS)
				{
				sprintf(reply,"Message Cannot access registry!");
				return 0;
				}
			RegSetValueEx(hKey,"DBpath",0,REG_SZ,(LPBYTE)DBpath,strlen(DBpath)+1);
			RegCloseKey(hKey);
			return 1;
			}

		if(strcmp(param1,"hashsize")==0)
			{
			hashmegabytes = atoi(param2);
			hashreallocate(hashmegabytes);
			sprintf(reply,"hash size set to %i",hashmegabytes);
			registry_error = RegCreateKeyEx(HKEY_CURRENT_USER,"Software\\Martin Fierz\\Cake",0,"CB_Key",0,KEY_ALL_ACCESS,NULL,&hKey,&result);
			if(registry_error != ERROR_SUCCESS)
				{
				sprintf(reply,"Message Cannot access registry!");
				return 0;
				}
			RegSetValueEx(hKey,"Hashsize",0,REG_DWORD,(LPBYTE)&hashmegabytes,sizeof(int));
			RegCloseKey(hKey);
			return 1;
			}

		if(strcmp(param1,"dbmbytes")==0)
			{
			if(dbmegabytes != atoi(param2))
				// only do this if a new setting for dbmegabytes was made
				{
				dbmegabytes = atoi(param2);
				//dbreallocate(dbmegabytes);
				registry_error = RegCreateKeyEx(HKEY_CURRENT_USER,"Software\\Martin Fierz\\Cake",0,"CB_Key",0,KEY_ALL_ACCESS,NULL,&hKey,&result);
				if(registry_error != ERROR_SUCCESS)
					{
					sprintf(reply,"Message Cannot access registry!");
					return 0;
					}
				RegSetValueEx(hKey,"DBcachesize",0,REG_DWORD,(LPBYTE)&dbmegabytes,sizeof(int));
				RegCloseKey(hKey);
				sprintf(reply,"Message DB cache size set to %i\nYou must restart Cake for the\nchange to take effect",dbmegabytes);
				}
			return 1;
			}

		if(strcmp(param1,"book")==0)
			{
			usethebook = atoi(param2);
			sprintf(reply,"Book set to %i",usethebook);
			registry_error = RegCreateKeyEx(HKEY_CURRENT_USER,"Software\\Martin Fierz\\Cake",0,"CB_Key",0,KEY_ALL_ACCESS,NULL,&hKey,&result);
			if(registry_error != ERROR_SUCCESS)
					{
					sprintf(reply,"Message Cannot access registry!");
					return 0;
					}
			RegSetValueEx(hKey,"Use Book",0,REG_DWORD,(LPBYTE)&usethebook,sizeof(int));
			RegCloseKey(hKey);
			return 1;
			}

		if(strcmp(param1,"allscores")==0)
			{
			allscores = atoi(param2);
			sprintf(reply,"Allscores set to %i",allscores);
			registry_error = RegCreateKeyEx(HKEY_CURRENT_USER,"Software\\Martin Fierz\\Cake",0,"CB_Key",0,KEY_ALL_ACCESS,NULL,&hKey,&result);
			if(registry_error != ERROR_SUCCESS)
					{
					sprintf(reply,"Message Cannot access registry!");
					return 0;
					}
			RegSetValueEx(hKey,"All Scores",0,REG_DWORD,(LPBYTE)&allscores,sizeof(int));
			RegCloseKey(hKey);
			return 0;
			}
		}
	
	if(strcmp(command,"get")==0)
		{
		if(strcmp(param1,"dbpath")==0)
			{
			sprintf(reply,"%s",DBpath);
			return 1;
			}
		
		if(strcmp(param1,"hashsize")==0)
			{
			sprintf(reply,"%i",hashmegabytes);
			return 1;
			}

		if(strcmp(param1,"dbmbytes")==0)
			{
			sprintf(reply,"%i",db_getcachesize()/1024);
			return 1;
			}

		if(strcmp(param1,"book")==0)
			{
			sprintf(reply,"%i",usethebook);
			return 1;
			}

		if(strcmp(param1,"allscores")==0)
			{
			sprintf(reply,"%i",allscores);
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
		}

	return 0;
	}



int __stdcall WINAPI getmove(int b[8][8],int color, double maxtime, char str[1024], int *playnow, int info, int unused, struct CBmove *CBMove)
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

		// info currently has uses for its first 3 bits:
		// info & 1 means reset
		// info & 2 means exact time level
		// info & 4 means increment time level
		// info & 8 means all score search
		POSITION p;
		int value;
		int how=0;
		int maxdepth=0;
		int i;
		static SEARCHINFO si;
		HASH h; 
		//FILE* fp; 
		
		if(!cake_is_init)
			{
			initcake(str);
			cake_is_init = 1;
			si.repcheck = malloc((MAXDEPTH + HISTORYOFFSET) * sizeof(REPETITION));
			}

		resetsearchinfo(&si);
		boardtobitboard(b,&p);
		p.color = color;
		
		// allocate memory for repcheck array
		//si.repcheck = malloc((MAXDEPTH+HISTORYOFFSET) * sizeof(REPETITION));
		absolutehashkey(&p, &h);

		// initialize repdetect array
		/*repdetect[HISTORYOFFSET].hash = h.key;
		repdetect[HISTORYOFFSET].irreversible = !(p.bk && p.wk); // only if both sides have kings it can be reversible 
		for(i = 0; i<=HISTORYOFFSET;i++) 
			si.repcheck[i] = repdetect[i];*/
		// if reset, then we clear repdetect array
		if(info & 1) {
			for(i = 0 ;i<HISTORYOFFSET; i++) {
				repdetect[i].hash = 0;
				repdetect[i].irreversible = 1; 
			}
		}

		// tell cake whether allscores is true or not.
		if(allscores == 1)
			info |= 8;

		value = cake_getmove(&si, &p, how, maxtime, maxdepth, 0, str, playnow, LOG, info);
		/* p is the bitboard position, color the side to move, 0 is 'how to search',
		0 means time-based, maxtime is the average time to use, 0 is the depthtosearch
		parameter which would be used if how was 1, cake prints info to str,
		returns if playnow is !=0 and does logging according to LOG. and does a reset
		if this is requested (info bit 1 set)*/
		bitboardtoboard(&p,b);
		/*absolutehashkey(&p, &h);
		repdetect[HISTORYOFFSET+1].hash = h.key;
		repdetect[HISTORYOFFSET+1].irreversible = 0;
		for(i = 0; i<HISTORYOFFSET;i++) 
			repdetect[i] = repdetect[i+2];
		free(si.repcheck);*/

		return value;
		}


void bitboardtoboard(POSITION *p,int b[8][8])
	{
	/* return a board from a bitboard */
   int i,board[32];

   for(i=0;i<32;i++)
   	{
      if (p->bm & (1<<i))
      	board[i]=(BLACK|MAN);
      if (p->bk & (1<<i))
      	board[i]=(BLACK|KING);
      if (p->wm & (1<<i))
      	board[i]=(WHITE|MAN);
      if (p->wk & (1<<i))
      	board[i]=(WHITE|KING);
      if ( (~(p->bm|p->bk|p->wm|p->wk)) & (1<<i))
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

void boardtobitboard(int b[8][8], POSITION *p)
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

   p->bm=0;
   p->bk=0;
   p->wm=0;
   p->wk=0;

   for(i=0;i<32;i++)
   	{
      switch (board[i])
      	{
         case BLACK|MAN:
            p->bm=p->bm|(1<<i);
            break;
         case BLACK|KING:
         	p->bk=p->bk|(1<<i);
         	break;
         case WHITE|MAN:
         	p->wm=p->wm|(1<<i);
            break;
         case WHITE|KING:
         	p->wk=p->wk|(1<<i);
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
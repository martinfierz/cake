#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winbase.h>
#include <assert.h>
#include <string.h>
#include <shlwapi.h>
#include <shlobj.h>

#include "structs.h"
#include "consts.h"
#include "switches.h"
#include "cakepp.h"
#include "move_gen.h"
#include "cake_misc.h"
#include "dblookup.h"

char suffix[128] = ""; 

void setsuffix(char* str) {
	sprintf(suffix, "%s", str); 
}

void searchinfotostring(char *out, int depth, double time, char *valuestring, char *pvstring, SEARCHINFO *si)
	{
	char percent='%';

	sprintf(out,"depth %i/%i/%.1f  time %.2fs  %s  nodes %u  %ikN/s  db %.0f%c cut %.1f%c %s", 
		(depth/FRAC), si->maxdepth, (float)si->leafdepth/((float)si->leaf+0.01) , time, valuestring, 
		si->negamax, (int)((float)si->negamax/(time+0.01)/1000),
		100*((float)si->dblookupsuccess/(float)(si->dblookupsuccess+si->dblookupfail+1)),
		percent, 100.0*(float)(si->cutoffsatfirst)/(float)(si->cutoffs), percent,	
		pvstring); 
	}


void movetonotation(POSITION *p,MOVE *m, char *str)
	{
	/* make a notation out of a move */
	/* m is the move, str the string, color the side to move */

	int from, to;
	char c;

	/*

	WHITE
	28  29  30  31           32  31  30  29
	24  25  26  27           28  27  26  25
	20  21  22  23           24  23  22  21
	16  17  18  19           20  19  18  17
	12  13  14  15           16  15  14  13
	8   9  10  11           12  11  10   9
	4   5   6   7            8   7   6   5
	0   1   2   3            4   3   2   1
	BLACK
	*/
	static int square[32]={4,3,2,1,8,7,6,5,
		12,11,10,9,16,15,14,13,
		20,19,18,17,24,23,22,21,
		28,27,26,25,32,31,30,29}; /* maps bits to checkers notation */
	// TODO: i just wrote a function that does this, i can get rid of this table.

	if(p->color==BLACK)
		{
		if(m->wk|m->wm) c='x';
		else c='-';                      /* capture or normal ? */
		from = (m->bm|m->bk)&(p->bm|p->bk);    /* bit set on square from */
		to = (m->bm|m->bk)&(~(p->bm|p->bk));
		from = LSB(from); 
		to = LSB(to);		
		from = square[from];
		to = square[to];
		sprintf(str,"%2i%c%2i",from,c,to);
		}
	else
		{
		if(m->bk|m->bm) c='x';
		else c='-';                      /* capture or normal ? */
		from=(m->wm|m->wk)&(p->wm|p->wk);    /* bit set on square from */
		to=  (m->wm|m->wk)&(~(p->wm|p->wk));
		from=LSB(from);
		to=LSB(to);
		from=square[from];
		to=square[to];
		sprintf(str,"%2i%c%2i",from,c,to);
		}
	return;
	}

int SquareToBit(int square)
{
	int x,y;
	int bit;

	square--;

	x = square % 4;
	y = square/4;

	bit = 4*y + (3-x);

	return bit;
}


void printint32(int32 x)
	{
	int i;
	int b[32];
	char c[15]="-x";
	for(i=0;i<32;i++)
		{
		if(((x>>i)%2) != 0)
			b[i]=1;
		else
			b[i] = 0;
		}

	printf("\n\n");
	printf("\n %c %c %c %c",c[b[28]],c[b[29]],c[b[30]],c[b[31]]);
	printf("\n%c %c %c %c ",c[b[24]],c[b[25]],c[b[26]],c[b[27]]);
	printf("\n %c %c %c %c",c[b[20]],c[b[21]],c[b[22]],c[b[23]]);
	printf("\n%c %c %c %c ",c[b[16]],c[b[17]],c[b[18]],c[b[19]]);
	printf("\n %c %c %c %c",c[b[12]],c[b[13]],c[b[14]],c[b[15]]);
	printf("\n%c %c %c %c ",c[b[8]],c[b[9]],c[b[10]],c[b[11]]);
	printf("\n %c %c %c %c",c[b[4]],c[b[5]],c[b[6]],c[b[7]]);
	printf("\n%c %c %c %c ",c[b[0]],c[b[1]],c[b[2]],c[b[3]]);
	}


void printboard(POSITION *p)
	{
	int i;
	int free=~(p->bm|p->bk|p->wm|p->wk);
	int b[32];
	char c[15]="-wb      WB";
	for(i=0;i<32;i++)
		{
		if((p->bm>>i)%2)
			b[i]=BLACK;
		if((p->bk>>i)%2)
			b[i]=BLACK|KING;
		if((p->wm>>i)%2)
			b[i]=WHITE;
		if((p->wk>>i)%2)
			b[i]=WHITE|KING;
		if((free>>i)%2)
			b[i]=0;
		}

	printf("\n\n");
	printf("\n %c %c %c %c",c[b[28]],c[b[29]],c[b[30]],c[b[31]]);
	printf("\n%c %c %c %c ",c[b[24]],c[b[25]],c[b[26]],c[b[27]]);
	printf("\n %c %c %c %c",c[b[20]],c[b[21]],c[b[22]],c[b[23]]);
	printf("\n%c %c %c %c ",c[b[16]],c[b[17]],c[b[18]],c[b[19]]);
	printf("\n %c %c %c %c",c[b[12]],c[b[13]],c[b[14]],c[b[15]]);
	printf("\n%c %c %c %c ",c[b[8]],c[b[9]],c[b[10]],c[b[11]]);
	printf("\n %c %c %c %c",c[b[4]],c[b[5]],c[b[6]],c[b[7]]);
	printf("\n%c %c %c %c ",c[b[0]],c[b[1]],c[b[2]],c[b[3]]);

	if(p->color == BLACK)
		printf("\nblack to move");
	else
		printf("\nwhite to move");
	}


void printboardtofile(POSITION *p, FILE *fp)
	{
	int i;
	int free=~(p->bm|p->bk|p->wm|p->wk);
	int b[32];
	char c[15]="-wb      WB";

	if(fp == NULL)
		return;

	for(i=0;i<32;i++)
		{
		if((p->bm>>i)%2)
			b[i]=BLACK;
		if((p->bk>>i)%2)
			b[i]=BLACK|KING;
		if((p->wm>>i)%2)
			b[i]=WHITE;
		if((p->wk>>i)%2)
			b[i]=WHITE|KING;
		if((free>>i)%2)
			b[i]=0;
		}

	fprintf(fp,"\n\n");
	fprintf(fp,"\n %c %c %c %c",c[b[28]],c[b[29]],c[b[30]],c[b[31]]);
	fprintf(fp,"\n%c %c %c %c ",c[b[24]],c[b[25]],c[b[26]],c[b[27]]);
	fprintf(fp,"\n %c %c %c %c",c[b[20]],c[b[21]],c[b[22]],c[b[23]]);
	fprintf(fp,"\n%c %c %c %c ",c[b[16]],c[b[17]],c[b[18]],c[b[19]]);
	fprintf(fp,"\n %c %c %c %c",c[b[12]],c[b[13]],c[b[14]],c[b[15]]);
	fprintf(fp,"\n%c %c %c %c ",c[b[8]],c[b[9]],c[b[10]],c[b[11]]);
	fprintf(fp,"\n %c %c %c %c",c[b[4]],c[b[5]],c[b[6]],c[b[7]]);
	fprintf(fp,"\n%c %c %c %c ",c[b[0]],c[b[1]],c[b[2]],c[b[3]]);
	//fclose(Lfp);
	}

/*
void createCakeFolder(void) {
	// function to be called from initCake which generates the \Martin Fierz\Cake subdirectory
	// in personal folder that Cake needs for writing its logfile to. 

	char dir[256]; 

	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, dir))) {
		// personal folder is now in dir, append my path
		strcat(dir, "\\Martin Fierz");
		CreateDirectoryA(dir, NULL);
		strcat(dir, "\\Cake");
		CreateDirectoryA(dir, NULL);
	}
}*/


FILE *getlogfile(int clear)
{
	// returns a file pointer to the cake log file under 
	// <personal documents>\martin fierz\Cake\cakelog.txt
	// if clear is != 0 it erases the file, if clear is 0 it appends to file
	// TODO: check if getcakedir, GetCurrentDirectory, SetCurrentDirectory 
	// is necessary at all.

	//char lstr[256];		// todo: comment out
	//char dirname[256];	// todo: comment out
	static char dir[256] = ""; 
	FILE* fp; 

	// create a logfile in the personal folders path
	if (dir[0] == 0) {
		if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, dir))) {
			// personal folder is now in dir, append my path
			strcat(dir, "\\Martin Fierz\\Cake\\cakelog");
			strcat(dir, suffix);
			strcat(dir, ".txt");
			//printf("\nlogfile created %s: ", dir); 
			//getch(); 
		}
			
	}

	if (clear) {
		fp = fopen(dir, "w");
		
		//printf("\n log file cleared"); 
		//getch(); 
		return fp; 
	}
	else {
		fp = fopen(dir, "a");
		//printf("\ntrying to append to log file"); 
		//getch(); 
		if (fp == NULL) {
			fp = fopen(dir, "w");
			//printf("\ncould not append, clearing log file");
			//getch(); 
		}
		
	return fp;
	}
	return NULL;
}


// ed's code below

/*
char* build_logfilename(char* enginename, char* filename)
{
	if (fullfilename[0] == 0) {

		// Get path for the My Documents directory.
		// On WinXP, this is \Documents and Settings\<user>\My Documents.
		// On Vista, this is \Users\<user>\Documents.
		 //
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, fullfilename))) {

			// Create the directories in case they do not exist. 
			PathAppend(fullfilename, "Ed Gilbert");
			CreateDirectory(fullfilename, NULL);

			PathAppend(fullfilename, enginename);
			CreateDirectory(fullfilename, NULL);

			PathAppend(fullfilename, filename);
		}
	}
	return(fullfilename);
}


void init_logfile(char* enginename, char* filename)
{
	FILE* fp;

	build_logfilename(enginename, filename);

	// Erase any previous logfiles. 
	fp = fopen(fullfilename, "w");
	fclose(fp);
}



 // Write a message string to the log file.

void log_msg(char* pvstr)
{
	FILE* fp;

	fp = fopen(logfilename(), "a");
	if (fp) {
		fprintf(fp, "%s", pvstr);
		fclose(fp);
	}
}


void log_msg(const char* fmt, ...)
{
	FILE* fp;
	va_list args;
	va_start(args, fmt);

	if (fmt == NULL)
		return;

	fp = fopen(logfilename(), "a");
	if (fp == NULL)
		return;

	vfprintf(fp, fmt, args);
	fclose(fp);
}
*/

// ed's code above



void clearlogfile(FILE* fp)
{
	
	if (fp != NULL) {
		fprintf(fp, "Cake log file\n\n");
		//fclose(fp);
	}
	return; 
}

/*
void getcakedir(char *lstr)
{
	sprintf(lstr, "C:\\code"); 
	return; 
	// Create the directories under My Documents. 
	//WCHAR *s = (WCHAR)* "Martin Fierz";
	//TCHAR *s = "Martin Fierz"; 
	
	//PathAppend(lstr, "Martin Fierz");
	CreateDirectory(lstr, NULL);

	//PathAppend(lstr, "Cake");
	CreateDirectory(lstr, NULL);
}*/

int logtofile(FILE *fp, char *str)
{
	// log a string to the engine logfile 
	
	if(fp != NULL)
		{
		fprintf(fp,"\n%s",str);
		//printf("\nlog: %s", str); 
		//getch(); 
		fflush(fp); 
		return 1;
		}
	else {
		; 
		//printf("\n could not log, fp is nULL!");
		//getch();
	}
	return 0;
}

int isforced(POSITION *p)
// determines if color should move immediately
	{
	int i,n;
	MOVE ml1[MAXMOVES];
	POSITION q1,q2;
	int values[MAXMOVES];
	int bestindex=0;

	// forced moves occur if a) only one capture move exists
	// and b) if 2 capture moves exist which transpose with 2 moves, then 4 single captures
	n = makecapturelist(p, ml1, values, bestindex);
	if(n==1)
		return 1;

	if(n!=2)
		return 0;

	// if we arrive here, there are two capture moves.
	// copy position so we dont wreck anything
	q1 = (*p);
	q2=q1;

	togglemove((&q1),ml1[0]);
	togglemove((&q2),ml1[1]);

	for(i=0;i<3;i++)
		{
		n = makecapturelist(&q1, ml1, values, bestindex);
		if(n!=1)
			return 0;
		togglemove((&q1), ml1[0]);
		}

	for(i=0;i<3;i++)
		{
		n = makecapturelist(&q2, ml1, values, bestindex);
		if(n!=1)
			return 0;
		togglemove((&q2), ml1[0]);
		}

	if(q1.bk==q2.bk && q1.bm==q2.bm && q1.wm==q2.wm && q1.wk==q2.wk)
		return 1;

	return 0;
	}

void resetsearchinfo(SEARCHINFO *s)
	{
	// resets the search info structure nearly completely, with the exception of
	// the pointer si.repcheck, to which we allocated memory during initialization - this is restored
	int *rep = (int*)s->repcheck; 
	memset(s, 0, sizeof(SEARCHINFO)); 
	s->repcheck = (REPETITION *) rep; //malloc((MAXDEPTH+HISTORYOFFSET) * sizeof(REPETITION));
	}

int exitcake()
	{
	// deallocate memory 
	db_exit();

	return 1;
	}
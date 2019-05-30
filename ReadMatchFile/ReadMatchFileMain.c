// ReadMatchFileMain


// 26.11.2017
// a first test with an "optimized" evaluation gives catastrophic result
// +22-57 instead of ~equal!

#include <stdio.h>
#include <stdlib.h> 
#include <assert.h>
#include <windows.h>
#include <math.h>

//#include "cake_misc.h"


// cake-specific includes - structs defines structures, consts defines constants,
// xxx.h defines function prototypes for xxx.c
#include "switches.h"
#include "structs.h"
#include "consts.h"
#include "PDNparser.h"
#include "move_gen.h"
//#include "cake_eval.h"


#define NODUPLICATES  // undef to allow duplicate games!


#define WHITE 1
#define BLACK 2
#define MAN 4
#define KING 8
#define FREE 16

//#define PARAMS 24

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

typedef struct
{
	__int64 startposition;	// 64 bit encoded start position (bm + wm<<32)
	__int64 moves[5];	// encode one move as (index%16) so 8 moves, with 
					// 5 integers that's 40 ply - if a game is identical this long
					// I count it as identical
	__int64 result;
	int32 start;	// start of positions belonging to this game
	int32 end;		// end of positions belonging to this game
	
} SHORTGAME;

#define MAXGAMES 700000
SHORTGAME* gamelist;


#define NUMFILES 67
#define NUMFILESREP 5

// repeat identical matches should be 46,53,54 and 50,52

char repeatfiles[NUMFILESREP][128] = { "match46.pdn", "match54.pdn", "match53.pdn", "match50.pdn", "match52.pdn" };

char files[NUMFILES][128] = { "match1.pdn",
							"match2.pdn",
							"match4.pdn",
							"match5.pdn",
							"match6.pdn",
							"match7.pdn",
							"match8.pdn",
							"match9.pdn",
							"match10.pdn",
							"match11.pdn",
"match12.pdn",
"match13.pdn",
"match14.pdn",
"match15.pdn",
"match16.pdn",
"match17.pdn",
"match18.pdn",
"match19.pdn",
"match20.pdn",
"match21.pdn",
"match22.pdn",
"match23.pdn",
"match0.5_1.pdn",
"match24.pdn",
"match25.pdn",
"match26.pdn",
"match27.pdn",
"match28.pdn",
"match29.pdn",
"match30.pdn",
"match31.pdn",
"match32.pdn",
"match33.pdn",
"match34.pdn",
"match35.pdn",
"match36'.pdn",
"match37.pdn",
"match38.pdn",
"match39.pdn",
"match40.pdn",
"match41.pdn",
"match0.5_2.pdn",
"match42.pdn",
"match43.pdn",
"match44.pdn",
"match45.pdn",
"match46.pdn",
"match47.pdn",
"match48.pdn",
"match49.pdn",
"match50.pdn",
"match51.pdn",
"match52.pdn",
"match53.pdn",
"match54.pdn",
"match55.pdn",
"match56.pdn",
"match57.pdn",
"match58.pdn",
"match59.pdn",
"match60.pdn",
"match61.pdn",
"match62.pdn",
"match63.pdn",
"match64.pdn",
"match65.pdn",
"match66.pdn"
};
char directory[64] = "C:\\code\\checkersdata\\";


// ugly globals
int gamenumber = 0;
int positions = 0;
int repetitions = 0;
int captures = 0;
int endgames = 0;



int main()
{
	int i = 0;
	FILE* fp;
	
	EVALUATEDPOSITION* ep;
	 
	
	int iterations = 0;
	int position_number = 0;
	int changed = 0;
	int result_translated[4] = { 0, 1, -1, 0 };
	int rejected = 0;
	int pos_num = 0;
	int quiet_pos_num = 0;
	char filename[128]; 

	
	ep = malloc(sizeof(EVALUATEDPOSITION) * MAXGAMES*50);  // allow for 2 million positions currently
	gamelist = malloc(sizeof(SHORTGAME) * MAXGAMES); // allow for 100'000 games currently
	if(gamelist != NULL)
		memset(gamelist, 0, sizeof(SHORTGAME) * MAXGAMES); // set to 0

	//for (i = 0; i < NUMFILESREP; i++) {
	//	sprintf(filename, "%s%s", directory, repeatfiles[i]);
	for (i = 0; i < NUMFILES; i++) {
		sprintf(filename, "%s%s", directory, files[i]);
			printf("\nfile to open is %s", filename);
		position_number = load_PDN(ep, filename);
		//getch();
	}

	printf("\nnow storing positions..."); 
#ifdef NODUPLICATES
	fp = fopen("C:\\code\\checkersdata\\taggedpositions.txt", "w");
#else
	fp = fopen("C:\\code\\checkersdata\\taggedpositions+duplicates.txt", "w");
#endif
	for (i = 0; i < position_number; i++)
		fprintf(fp, "%u %u %u %u %i %i\n", ep[i].bm, ep[i].bk, ep[i].wm, ep[i].wk, ep[i].color, ep[i].gameresult);
	fclose(fp);
	printf("\n%i games checked, %i positions stored", gamenumber, position_number);
	getch();
	exit(0);
	
}


int load_PDN(EVALUATEDPOSITION * ep, char *filename) {
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
	int duplicates = 0; 
	int isduplicate; 
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
	
	
	int gamestart_position; 
	int rep;
	int capture;
	int error; 

	int movenumber; 
	int moveindex; 


	buffer = malloc(10000000);
	if (buffer == NULL)
		return 0;
	fp = fopen(filename, "r");

	if (!fp)
		return 0;
	/* load a PDN database into buffer */
	bytesread = fread(buffer, 1, 10000000, fp);
	//printf("read %i bytes in PDN database - hit a key", bytesread);
	//getch();
	fclose(fp);
	buffer[bytesread] = 0;

	start = buffer;

	// get games
	while (PDNparseGetnextgame(&start, game)) {
		//printf("%s", game);
		//printf("\ngame number %i", gamenumber);
		// now we have a game in the string game
		startheader = game;
		result = UNKNOWN; 
		error = 0;
		isduplicate = 0; 

		// get headers
		p.bm = 0; p.wm = 0; 
		result = readheaders(&p, &startheader); 


		/*
		while (PDNparseGetnextheader(&startheader, header)) {
			tag = header;
			PDNparseGetnexttoken(&tag, headername);
			PDNparseGetnexttag(&tag, headervalue);
			_strlwr(headername);
			if (strcmp(headername, "result") == 0) {
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
				FENtoPosition(headervalue, &p);
			}
		}
		// headers parsed*/
		
		// initialize the gamelist entry:
		gamelist[gamenumber].result = result; 
		gamelist[gamenumber].startposition = ((__int64) p.bm) + (((__int64)p.wm) << 32);
		gamelist[gamenumber].moves[0] = 0;
		gamelist[gamenumber].moves[1] = 0;
		gamelist[gamenumber].moves[2] = 0;
		gamelist[gamenumber].moves[3] = 0;
		gamelist[gamenumber].moves[4] = 0;


		// remember the position index so that we could reset
		gamestart_position = positions; 

		// store initial position
		if (!testcaptureEither(&p)) {
			ep[positions].bm = p.bm;
			ep[positions].bk = p.bk;
			ep[positions].wm = p.wm;
			ep[positions].wk = p.wk;
			ep[positions].color = p.color;
			ep[positions].value = 0;
			ep[positions].gameresult = result;
			positions++;
		}


		// get moves
		movenumber = 0; 
		while (PDNparseGetnexttoken(&startheader, token)) {
			// stop parsing if an error occured
			if (error)
				break; 
//			printf("\ntoken %s", token);
			/* skip move numbers, that is, skip if the last character
				of the token is a full stop */
			if (token[strlen(token) - 1] == '.') continue;
			if (token[0] == '{') continue; // skip comments
			//printf("%s", token);
			PDNparseTokentonumbers(token, &from, &to);
			//printf("  %i  %i", from, to);

			resetsearchinfo(&si);
			found = 0;
			moveindex = 0; 
			n = makecapturelist(&p, movelist, values, forcefirst);
			if (n == 0)
				n = makemovelist(&si, &p, movelist, values, forcefirst, 0);
			for (i = 0; i < n; i++) {
				movetonotation(&p, &(movelist[i]), movestring);
				sscanf(movestring, "%i%c%i", &checkfrom, &c, &checkto);
				//printf("\nmove generated %i %i", checkfrom, checkto);
				if (checkfrom == from && checkto == to) {
					//printf("found");
					moveindex = i; 
					togglemove((&p), movelist[i]);
					found = 1;
					break;
				}
			}
			// store move in short game list
			
			if (movenumber < 40) {
				gamelist[gamenumber].moves[movenumber / 8] +=  ( ((__int64) (moveindex % 16)) << ((moveindex % 16) * 5) ); 
			}

			if (found == 0) {
				//printf("move #%i (%s) not found!\n", movenumber, movestring);
				//printboard(&p); 
				//printf("\n%s", game); 
				//getch();
				error = 1; 
				printf("!!error!!"); 
			}
			// check for capture
			capture = 0;
			if (testcaptureEither(&p)) {
				capture = 1;
				captures++;
			}

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
			if (rep)
				repetitions++;

			if (bitcount(p.bm | p.bk | p.wm | p.wk) <= 8) {
				rep = 1;
				endgames++;
			}

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
			movenumber++; 
		}
		
		if (!error) {

#ifdef NODUPLICATES
			// check if we have a repeat game
			for (i = 0; i < gamenumber; i++) {
				if (gamelist[i].startposition == gamelist[gamenumber].startposition &&
					gamelist[i].result == gamelist[gamenumber].result &&
					gamelist[i].moves[0] == gamelist[gamenumber].moves[0] &&
					gamelist[i].moves[1] == gamelist[gamenumber].moves[1] &&
					gamelist[i].moves[2] == gamelist[gamenumber].moves[2] &&
					gamelist[i].moves[3] == gamelist[gamenumber].moves[3] &&
					gamelist[i].moves[4] == gamelist[gamenumber].moves[4]) {
					duplicates++;
					isduplicate = 1; 
					//printf("\nduplicate game found! %i %i (%i)", i, gamenumber, positions); 
					//printf("\n%s", game); 
					//getch(); 
				}
			}
#endif
			gamenumber++;
		}
		if(error || isduplicate)		// go back to original entry of array thereby overwriting 
			positions = gamestart_position;
		if (isduplicate)
			gamenumber--;
		//else
		//	printf("\ngame %i is no duplicate", gamenumber); 
	}
	printf("\n%i positions stored, %i repetitions, %i endgames, %i captures, %i duplicates found", 
		positions, repetitions, endgames, captures, duplicates);
	free(buffer); 
	return positions;
}


int readheaders(POSITION *p, char** startheader) {

	int result; 
	char* tag;
	char header[256];
	char headername[32];
	char headervalue[256];

	while (PDNparseGetnextheader(startheader, header)) {
		tag = header;
		PDNparseGetnexttoken(&tag, headername);
		PDNparseGetnexttag(&tag, headervalue);
		_strlwr(headername);
		if (strcmp(headername, "result") == 0) {
			//printf("result is %s", headervalue);
			if (strcmp(headervalue, "1-0") == 0) {
				result = WIN;
				//printf("\nWIN\n");
			}
			else if (strcmp(headervalue, "0-1") == 0) {
				result = LOSS;
				//printf("\nLOSS\n");
			}
			else if (strcmp(headervalue, "1/2-1/2") == 0) {
				result = DRAW;
				//printf("\nDRAW\n");
			}
			else {
				result = UNKNOWN;
				//printf("\nUNKNOWN");
			}
		}
		if (strcmp(headername, "fen") == 0) {
			FENtoPosition(headervalue, p);
			//printf("FEN %s", headervalue); 
		}
	}
	return result; 
	// headers parsed
}

int FENtoPosition(char* FEN, POSITION * p)
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



int testcapture(POSITION * p)
{
	// testcapture returns 1 if the side to move has a capture.
	unsigned int black, white, free, m;

	black = p->bm | p->bk;
	white = p->wm | p->wk;
	free = ~(black | white);
	if (p->color == BLACK)
	{
		m = ((((black & LFJ2) << 4) & white) << 3);
		m |= ((((black & LFJ1) << 3) & white) << 4);
		m |= ((((black & RFJ1) << 4) & white) << 5);
		m |= ((((black & RFJ2) << 5) & white) << 4);
		if (p->bk)
		{
			m |= ((((p->bk & LBJ1) >> 5) & white) >> 4);
			m |= ((((p->bk & LBJ2) >> 4) & white) >> 5);
			m |= ((((p->bk & RBJ1) >> 4) & white) >> 3);
			m |= ((((p->bk & RBJ2) >> 3) & white) >> 4);
		}
	}
	else
	{
		m = ((((white & LBJ1) >> 5) & black) >> 4);
		m |= ((((white & LBJ2) >> 4) & black) >> 5);
		m |= ((((white & RBJ1) >> 4) & black) >> 3);
		m |= ((((white & RBJ2) >> 3) & black) >> 4);
		if (p->wk)
		{
			m |= ((((p->wk & LFJ2) << 4) & black) << 3);
			m |= ((((p->wk & LFJ1) << 3) & black) << 4);
			m |= ((((p->wk & RFJ1) << 4) & black) << 5);
			m |= ((((p->wk & RFJ2) << 5) & black) << 4);
		}
	}
	if (m & free)
		return 1;
	return 0;
}

int testcaptureEither(POSITION* p)
{
	// testcapture returns 1 if the side to move has a capture.
	unsigned int black, white, free, m;

	black = p->bm | p->bk;
	white = p->wm | p->wk;
	free = ~(black | white);
	
	m = ((((black & LFJ2) << 4) & white) << 3);
	m |= ((((black & LFJ1) << 3) & white) << 4);
	m |= ((((black & RFJ1) << 4) & white) << 5);
	m |= ((((black & RFJ2) << 5) & white) << 4);
	if (p->bk)
	{
		m |= ((((p->bk & LBJ1) >> 5) & white) >> 4);
		m |= ((((p->bk & LBJ2) >> 4) & white) >> 5);
		m |= ((((p->bk & RBJ1) >> 4) & white) >> 3);
		m |= ((((p->bk & RBJ2) >> 3) & white) >> 4);
	}
	m |= ((((white & LBJ1) >> 5) & black) >> 4);
	m |= ((((white & LBJ2) >> 4) & black) >> 5);
	m |= ((((white & RBJ1) >> 4) & black) >> 3);
	m |= ((((white & RBJ2) >> 3) & black) >> 4);
	if (p->wk)
	{
		m |= ((((p->wk & LFJ2) << 4) & black) << 3);
		m |= ((((p->wk & LFJ1) << 3) & black) << 4);
		m |= ((((p->wk & RFJ1) << 4) & black) << 5);
		m |= ((((p->wk & RFJ2) << 5) & black) << 4);
	}
	if (m & free)
		return 1;
	return 0;
}




int countmaterial(POSITION * p, MATERIALCOUNT * m)
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

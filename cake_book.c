#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "structs.h"
#include "switches.h"
#include "cakepp.h"
#include "move_gen.h"
#include "cake_misc.h"

// TODO: variables such as the book hashtable, number of bookentries and the usethebook setting should be owned by this file

int booklookup(POSITION *p, int *value, int depth, int32 *remainingdepth, int *best, char str[256], HASHENTRY *book, int bookentries, int usethebook)
	{
	// searches for a position in the book hashtable.
	int32 index;
	int iter=0;
	int bookmoves;
	// todo: change this to bookhashentry!
	HASHENTRY *pointer;
	int bucketsize;
	int size;
	int bookfound = 0;
	int i,j,n;
	int bookcolor;
	int tmpvalue;
	MOVE tmpmove,bestmove;
	int dummy[MAXMOVES], values[MAXMOVES],indices[MAXMOVES];
	int depths[MAXMOVES];
	MOVE ml[MAXMOVES];
	char Lstr[1024], Lstr2[1024];
	HASH hash;
	SEARCHINFO s;
	
	bucketsize = BUCKETSIZEBOOK;
	pointer = book;
	size = bookentries;

	if(pointer == NULL)
		return 0;

	printboardtofile(p); 
	
	// now, look up current position
	absolutehashkey(p,&hash);
	index = hash.key % size;

	sprintf(Lstr, "hash is %i (key) %i (lock), index is %i, searching through %i book entries", hash.key, hash.lock, index, bookentries);
	logtofile(Lstr); 
	printf(str); 

	sprintf(Lstr, "the size of an int32 is %zi", sizeof(int32)); 
	logtofile(Lstr); 
	
	// harmonize colors between cake and book
	bookcolor = p->color;
	if(p->color==2) 
		bookcolor=0;

	//index = 0; 
	while(iter<bucketsize)
	//while (iter<bookentries)
		{
		sprintf(Lstr, "iteration %i index %i: %i =? %i", iter, index, pointer[index].lock, hash.lock);
		logtofile(Lstr);
		if(pointer[index].lock == hash.lock)  {
			sprintf(Lstr, "\npotential match in book found"); 
			logtofile(Lstr); 
			if (((int)pointer[index].color == bookcolor)) {
				*remainingdepth = pointer[index].depth;
				*value = pointer[index].value;
				*best = pointer[index].best;
				bookfound = 1;
				}
			}

		iter++;
		index++;
		index %= size;
		
		if(bookfound) 
			break;
		}

	if (bookfound)
		sprintf(Lstr, "found position in book at index %i", index);
	else
		sprintf(Lstr, "position not found in book after %i iterations", iter);
	logtofile(Lstr);
	printf(Lstr);

	if(bookfound)
		{
		// search all successors in book
		n = makecapturelist(p, ml, dummy, 0);
		if(!n)
			n = makemovelist(&s, p, ml, dummy, 0,0);

		for(i=0;i<MAXMOVES;i++)
			{
			values[i]=-MATE;
			indices[i]=i;
			}
		bestmove = ml[*best];
		
		// harmonize colors for book and cake
		bookcolor = (p->color^CC);
		if(bookcolor == 2)
			bookcolor = 0;

		// for all moves look up successor position in book
		for(i=0;i<n;i++)
			{
			togglemove(p,ml[i]);
			absolutehashkey(p, &hash);
			index = hash.key % size;
			iter = 0;
			while(iter<bucketsize)
				{
				if(pointer[index].lock == hash.lock && ((int)pointer[index].color==bookcolor))
					{
					// found position in book
					depths[i] = pointer[index].depth+1;
					values[i] = -pointer[index].value;
					}
				iter++;
				index++;
				index%=size;
				}
			togglemove(p,ml[i]);
			}
		}

	// print book moves. check if we have successors:
	// order moves so we can print them in status line
	if(bookfound && values[0] != -MATE)
		{
		sprintf(str,"book  ");
		for(i=0;i<n;i++)
			{
			for(j=0;j<n-1;j++)
				{
				if(100*values[j]+depths[j]<100*values[j+1]+depths[j+1])
					{
					tmpvalue = values[j];
					tmpmove = ml[j];
					values[j] = values[j+1];
					values[j+1] = tmpvalue;
					ml[j] = ml[j+1];
					ml[j+1] = tmpmove;
					tmpvalue = depths[j];
					depths[j] = depths[j+1];
					depths[j+1] = tmpvalue;
					tmpvalue = indices[j];
					indices[j] = indices[j+1];
					indices[j+1] = tmpvalue;
					}
				}
			}

		// count number of available book moves and put in variabe bookmoves
		for(i=0;i<n;i++)
			{
			if(values[i] != -MATE)
				bookmoves = i;
			}
		bookmoves++;

		// create ouput string with all moves, values, depths ordered by value
		for(i=0;i<n;i++)
			{	
			if(values[i]==-MATE)
				continue;
			movetonotation(p, &ml[i], Lstr);
			sprintf(Lstr2, " v%i d%i   ", values[i], depths[i]);
			strcat(str,Lstr);
			strcat(str,Lstr2);
			}

		// now, select a move according to value of usethebook 
		// if we have more than one available bookmove
		if(usethebook < BOOKBEST && bookmoves >1)
			{
			bookmoves = 0;
			if(usethebook == BOOKGOOD)
				{
				// select any move equal to the best in value
				for(i=1;i<n;i++)
					{
					if(values[i]==values[0])
						bookmoves = i;
					}
				bookmoves++;
				}
			if(usethebook == BOOKALLKINDS)
				{
				// select any move that is > -30 in value, and within
				// 10 points of the best move
				for(i=1;i<n;i++)
					{
					if(values[i]>values[0]-10 && values[i]>-30)
						bookmoves = i;
					}
				bookmoves++;
				}
			// we have bookmoves equivalent book moves.
			// pick one at random
			if(bookmoves !=0)
				{
			    srand( (unsigned)time( NULL ) );
				i = rand() % bookmoves;
				*remainingdepth = depths[i];
				*value = values[i];
				*best = indices[i];
				}
			}
		}
	else
		{
		// last book move
		movetonotation(p,&bestmove,Lstr);
		sprintf(str,"book move: %s v %i d %i", Lstr,*value, *remainingdepth);
		}

	return bookfound;
	}
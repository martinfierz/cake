#include "switches.h"

#ifdef SPA

#include <stdio.h>
#include <stdlib.h> /* malloc() */
#include <string.h> /* memset() */
#include <windows.h>

/* structs.h defines the data structures for cake++ */
#include "structs.h"

/* consts.h defines constants used by cake++ */
#include "consts.h"

/* function prototypes */
#include "cakepp.h"

//int position_to_hashkey(POSITION *p, int32 *key, int32 *lock);


//SPA_ENTRY *spatable[4];
SPA_ENTRY *spa;

int initspa(void)
	{
	/*int i=0,j;
	int32 lock, key;
	FILE *fp;
	SPA_POSITION spapos;
	SPA_ENTRY spaentry;
	POSITION p;
	char *buffer;
	int items;
	int index;
	int k,l;
	char str[256];*/

	spa = malloc((SPASIZE + SPAITER) * sizeof(SPA_ENTRY));
	if(spa != 0)
		memset(spa, 0, (SPASIZE + SPAITER) * sizeof(SPA_ENTRY));

	/*for(l=0; l<2; l++)
		{
		spatable[l] = NULL;
#ifdef WINMEM
		spatable[l] = VirtualAlloc(0, (SPASIZE+SPAITER)*sizeof(SPA_ENTRY), MEM_COMMIT, PAGE_READWRITE);
#else
		spatable[l] = malloc((SPASIZE+SPAITER)*sizeof(SPA_ENTRY));
		memset(spatable[l],0,(SPASIZE+SPAITER)*sizeof(SPA_ENTRY));
#endif

		if(spatable[l] == NULL)
			{
			printf("\ncould not allocate spa memory!");
			//getch();
			}
	
		spa = spatable[l];
		switch(l)
			{
			case 0:
				sprintf(str,"analysed-10pc-men.dat");
				break;
			case 1: 
				sprintf(str,"analysed-12pc-men.dat");
				break;
			}
		fp = fopen(str, "rb");
		if(fp == NULL)
			{
			printf("\ncould not allocate spa memory!");
			//getch();
			}
		printf("\nreading %s",str);
		// now, load spa position for spa position, transform position into a hashkey,
		// and store in spa hashtable
		//printf("\nsizeof(SPA_ENTRY) is %i",sizeof(SPA_ENTRY));
		buffer = malloc(65536*sizeof(SPA_POSITION));
		i = 0;
		while(!feof(fp))
			{
			// read
			items = fread(buffer,sizeof(SPA_POSITION),65536,fp);
			for(j=0;j<items;j++)
				{
				memcpy(&spapos, buffer+j*sizeof(SPA_POSITION), sizeof(SPA_POSITION));
				// transform to position
				p.bm = spapos.black & (~spapos.kings);
				p.bk = spapos.black & spapos.kings;
				p.wm = spapos.white & (~spapos.kings);
				p.wk = spapos.white & spapos.kings;
				// get hashkey
				position_to_hashkey(&p, &key, &lock);
				//printf("\nkey,lock: %u %u",key,lock);
				//printboard(&p,BLACK);
				//printf("\nsearch eval, static eval %i %i",spapos.value, spapos.staticeval);
				//getch();
				
				// transform to spaentry
				spaentry.hashkey = lock;
				spaentry.value = spapos.value;
				// save in spatable
				index = key & (SPASIZE-1);
				
				for(k=0;k<SPAITER;k++)
					{
					if(spa[index+k].hashkey == 0)
						{
						spa[index+k] = spaentry;
						}
					}
				// we don't overwrite in SPA table!
				i++;
				if(!(i%(1024*1024)))
					printf("\nreading spa: %i positions read",i);
				}
			}
		}*/
	return 1;
	}

int loadspa(void) {
	FILE* fp; 
	int i, j; 
	int dummy, eval, depth; 
	int n = 0; 
	int index; 

	POSITION lp; 
	HASH h; 

	printf("\nloading spa..:"); 
	fp = fopen("c:\\code\\checkersdata\\uniquepositions1.txt", "r");

	// fprintf(fpout, "%u %u %u %u %i %i %i %i\n",p.bm, p.bk,p.wm, p.wk, p.color, v0, v3, si.depth);
	while (!feof(fp)) {
		fscanf(fp, "%u %u %u %u %u %i %i %i\n", &(lp.bm), &(lp.bk), &(lp.wm), &(lp.wk), &(lp.color), &dummy, &eval, &depth);
		n++; 
		absolutehashkey(&lp, &h);
		//if (n % 50000 == 0) {
		//	printboard(&lp); 
		//	printf("evaluation is %i", eval); 
		//	getch(); 
		//}

		spastore(h.key, h.lock, eval, depth, lp.color); 

		
	}
	printf("\n%i spa positions loaded", n); 

	fclose(fp); 
}

int spalookup(SEARCHINFO* si, int* value, int d, int color) {
	int index; 
	int i; 

	index = si->hash.key & (SPASIZE - 1); // only works for spasize as 2^x
	for (i = 0; i < SPAITER; i++) {
		if (spa[index + i].lock == si->hash.lock && 
			spa[index+i].color == color) {
			if (spa[index + i].depth* FRAC >= d) {
				*value = spa[index + i].value;
				//printf("spa color is %i, color is %i", spa[index + i].color, color); 
				return 1;
			}
			return 0; 
		}
		if(spa[index + i].lock == 0)
			return 0; 
	}
	return 0; 
}

int spastore(int key, int lock, int eval, int depth, int color) {
	int index; 
	int i; 

	index = key & (SPASIZE - 1); 
	for (i = 0; i < SPAITER; i++) {
		if (spa[index + i].lock == 0 || spa[index + i].lock == lock) {
			spa[index + i].lock = lock; 
			spa[index + i].depth = depth; 
			spa[index + i].value = eval; 
			spa[index + i].color = color; 
			return 1; 
		}
	}
	return 0; 
}

/*

int spa_lookup(int32 key, int32 lock, int *eval, int table)
	{
	int index;
	int i;
	index = key&(SPASIZE-1);

	//spa = spatable[table];

	for(i=0;i<SPAITER;i++)
		{
		if(spa[index+i].hashkey == lock)
			{
			*eval = spa[index+i].value;
			return 1;
			}
		}
	return 0;
	}

int position_to_hashkey(POSITION *p, int32 *key, int32 *lock)
	{
//	int32 x;
	int32 lkey = 0,llock = 0;
	extern int32 Gkey, Glock; // from cakepp.c
	HASH h; 

	absolutehashkey(p, &h);
	//printf("\nkey, lock: %i %i",Gkey, Glock);
	//*key = Gkey;
	//*lock = Glock;

	return 1;
	}
*/
#endif // SPA
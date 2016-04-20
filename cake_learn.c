#include <stdio.h>
#include "switches.h"
#include "structs.h"

// learning routines removed from cakepp.c august 2010 - probably they would be obsolete and not work anyway...

#ifdef LEARNSAVE
void learn(POSITION *p, int value, int depth, int bestindex)
	{
	LEARNENTRY le;
	FILE *learnfp;
	
	// set values of LEARNENTRY structure
	le.black = p->bm|p->bk;
	le.white = p->wm|p->wk;
	le.kings = p->bk|p->wk;
	le.value = value;
	le.color = p->color>>1;
	le.depth = depth*FRAC;
	le.best = bestindex; 
	learnfp = fopen("cakeM.lrn","ab");
	fwrite(&le, sizeof(LEARNENTRY), 1, learnfp);
	fclose(learnfp);
	}
#endif

#ifdef LEARNUSE
void stufflearnpositions(void)
	{
	FILE *learnfp;
	POSITION dummy;
	LEARNENTRY le;
	int index;
	HASH h; 

		
	learnfp = fopen("cakeM.lrn", "rb");
	if(learnfp != NULL)
		{
		// read learned positions one by one 
		while(!feof(learnfp))
			{
			fread(&le, sizeof(LEARNENTRY), 1, learnfp);
			dummy.bm = le.black & (~le.kings);
			dummy.bk = le.black & le.kings;
			dummy.wm = le.white & (~le.kings);
			dummy.wk = le.white & le.kings;
			absolutehashkey(&dummy, &h);

			if(abs(le.value)<400)
				{
				// and stuff into hashtable - only if the value is small; that is to defend
				// against old learn entries that were saved in earlier versions of cake ss.
				index = h.key & (hashsize-1);
				hashtable[index].best = le.best;
				hashtable[index].color = le.color;
				hashtable[index].depth = le.depth;
				hashtable[index].ispvnode = 0;
				hashtable[index].lock = h.lock;
				hashtable[index].value = le.value;
				hashtable[index].valuetype = EXACT;
				}
			}
		fclose(learnfp);
		}
	}

#endif


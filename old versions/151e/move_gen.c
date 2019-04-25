/*		cake++ 1.3 - a checkers engine			*/
/*															*/
/*		Copyright (C) 2001 by Martin Fierz		*/
/*															*/
/*		contact: checkers@fierz.ch					*/

/* movegen.c is the movegenerator for the bitboard checkers engine */
/* the move structure consists of 4 int32's to toggle the position with */
/* and one int32 info with additional information about the move */
/* info & 0x0000FFFF contains the ordering value of the move */
#define MCV 1
#define KCV 2
#define PVVAL 3
#define KINGCAPT 13
#define MANCAPT 10

#include "structs.h"
#include "consts.h"
#include "move_gen.h"
#include "cakepp.h"
#include "switches.h"

/* values used for dynamic move ordering */
#define HASHMOVE 1<<30
#define KILLER 1<<29
/* values used for static move ordering */
#define C4 0x00042000   /*innermost squares */
#define C3 0x00624600   /* center squares */
#define C1 0xF181818F   /* edge squares */
#define PROM 30     /*20*/    /* a promotion */
#define PROM1 8   /*18 */    /* far down the board */
#define PROM2 3   /*16 */
#define GIVEUPBACK 12 /*12*/
#define MANC3VAL 2
#define MANC4VAL 4
#define KINGC3VAL 5
#define KINGC4VAL 10
#define KINGC1VAL -10
#define CAPT 50
#define HISTORY 300
#define MINHASH 100


#pragma warning( disable : 4146 ) // disable warning that i compute -x for an unsigned int x

int makemovelist(struct pos *p, struct move movelist[MAXMOVES],int values[MAXMOVES],int color, int bestindex, int32 killer)
	{
   int32 i,n=0,free;
   int32 m,tmp;
   
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
	/* forcefirst is the move which should be ordered to front, it is
   m.bm|m.bk if color==BLACK */
   /* and m.wm|m.wk if color==WHITE*/
   free=~(p->bm|p->bk|p->wm|p->wk);
   if(color==BLACK)
   	{
      if(p->bk)
      	{
         /* moves left forwards */
         /* I: columns 1357 */
         m=((p->bk&LF1)<<3)&free;
         /* now m contains a bit for every free square where a black king can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            tmp=tmp|(tmp>>3);
            movelist[n].bm=0;
            movelist[n].bk=tmp;
            movelist[n].wm=0;
            movelist[n].wk=0;
            values[n]=0;
            n++;
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         /* II: columns 2468 */
         m=((p->bk&LF2)<<4)&free;
         while(m)
   			{
            tmp=(m&-m);
            tmp=tmp|(tmp>>4);
            movelist[n].bm=0;
            movelist[n].bk=tmp;
            movelist[n].wm=0;
            movelist[n].wk=0;
            values[n]=0;
            n++;
      		m=m&(m-1);
      		}
         /* moves right forwards */
         /* I: columns 1357 */
         m=((p->bk&RF1)<<4)&free;
         while(m)
   			{
            tmp=(m&-m);
            tmp=tmp|(tmp>>4);
            movelist[n].bm=0;
            movelist[n].bk=tmp;
            movelist[n].wm=0;
            movelist[n].wk=0;
            values[n]=0;
            n++;
      		m=m&(m-1);
      		}
         /* II: columns 2468 */
         m=((p->bk&RF2)<<5)&free;
         while(m)
   			{
            tmp=(m&-m);
            tmp=tmp|(tmp>>5);
            movelist[n].bm=0;
            movelist[n].bk=tmp;
            movelist[n].wm=0;
            movelist[n].wk=0;
            values[n]=0;
            n++;
      		m=m&(m-1);
      		}
         /* moves left backwards */
         /* I: columns 1357 */
         m=((p->bk&LB1)>>5)&free;
         /* now m contains a bit for every free square where a black man can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            tmp=tmp|(tmp<<5);
            movelist[n].bm=0;
            movelist[n].bk=tmp;
            movelist[n].wm=0;
            movelist[n].wk=0;
            values[n]=0;
            n++;
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         /* II: columns 2468 */
         m=((p->bk&LB2)>>4)&free;
         while(m)
   			{
            tmp=(m&-m);
            tmp=tmp|(tmp<<4);
            movelist[n].bm=0;
            movelist[n].bk=tmp;
            movelist[n].wm=0;
            movelist[n].wk=0;
            values[n]=0;
            n++;
      		m=m&(m-1);
      		}
         /* moves right backwards */
         /* I: columns 1357 */
         m=((p->bk&RB1)>>4)&free;
         while(m)
   			{
            tmp=(m&-m);
            tmp=tmp|(tmp<<4);
            movelist[n].bm=0;
            movelist[n].bk=tmp;
            movelist[n].wm=0;
            movelist[n].wk=0;
            values[n]=0;
            n++;
      		m=m&(m-1);
      		}
         /* II: columns 2468 */
         m=((p->bk&RB2)>>3)&free;
         while(m)
   			{
            tmp=(m&-m);
            tmp=tmp|(tmp<<3);
            movelist[n].bm=0;
            movelist[n].bk=tmp;
            movelist[n].wm=0;
            movelist[n].wk=0;
            values[n]=0;
            n++;
      		m=m&(m-1);
      		}
         }
         /* moves with black stones:*/
      if(p->bm)
      	{
         /* moves left forwards */
         /* I: columns 1357: just moves */
         m=((p->bm&LF1)<<3)&free;
         /* now m contains a bit for every free square where a black man can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            tmp=tmp|(tmp>>3); /* square where man came from */
            movelist[n].bm=tmp&NWBR; /* NWBR: not white back rank */
            movelist[n].bk=tmp&WBR; /*if stone moves to WBR (white back rank) it's a king*/
            movelist[n].wm=0;
            movelist[n].wk=0;
				values[n]=0;
            n++;
      		m=m&(m-1);   /* clears least significant bit of m */
      		}

         /* II: columns 2468 */
         m=((p->bm&LF2)<<4)&free;
         while(m)
   			{
            tmp=(m&-m);
            tmp=tmp|(tmp>>4);
            movelist[n].bm=tmp;
            movelist[n].bk=0;
            movelist[n].wm=0;
            movelist[n].wk=0;
            values[n]=0;
            n++;
      		m=m&(m-1);
      		}
         /* moves right forwards */
         /* I: columns 1357 :just moves*/
         m=((p->bm&RF1)<<4)&free;
         while(m)
   			{
            tmp=(m&-m);
            tmp=tmp|(tmp>>4);
				movelist[n].bm=tmp&NWBR;
            movelist[n].bk=tmp&WBR;
            movelist[n].wm=0;
            movelist[n].wk=0;
				values[n]=0;
            n++;
      		m=m&(m-1);
      		}

         /* II: columns 2468 */
         m=((p->bm&RF2)<<5)&free;
         while(m)
   			{
            tmp=(m&-m);
            tmp=tmp|(tmp>>5);
            movelist[n].bm=tmp;
            movelist[n].bk=0;
            movelist[n].wm=0;
            movelist[n].wk=0;
            values[n]=0;
            n++;
      		m=m&(m-1);
      		}
         }
#ifndef MOVEORDERING
		return n;
#endif
		
#ifdef MOHASH
      /* give the forcefirst-move a high eval */
		values[bestindex]+=HASHMOVE;
#endif
#ifdef MOKILLER
      /* sort moves: according to movelist[n].info */
      if(n>1)
      	{
         /* give the forcefirst-move a high eval */
         if(killer)
         	{
            for(i=0;i<n;i++)
         		{
            	if((movelist[i].bm|movelist[i].bk) == killer)
            		{
      				values[i]+=KILLER;
					break;
               		}
            	}
            }
         }
#endif

/* do a static evaluation of the moves */
     if(n>1)
		{
         blackorderevaluation(p,movelist,values,n);
         }
		return n;
      }
            /* ****************************************************************/
   else     /* color is WHITE */
            /******************************************************************/
   	{
      /* moves with white kings:*/
   	if(p->wk)
      	{
         /* moves left forwards */
         /* I: columns 1357 */
         m=((p->wk&LF1)<<3)&free;
         /* now m contains a bit for every free square where a black man can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
				tmp=tmp|(tmp>>3);
            movelist[n].bm=0;
            movelist[n].bk=0;
            movelist[n].wm=0;
            movelist[n].wk=tmp;
      		values[n]=0;
            n++;
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         /* II: columns 2468 */
         m=((p->wk&LF2)<<4)&free;
         while(m)
   			{
            tmp=(m&-m);
            tmp=tmp|(tmp>>4);
            movelist[n].bm=0;
            movelist[n].bk=0;
            movelist[n].wm=0;
            movelist[n].wk=tmp;
      		values[n]=0;
            n++;
      		m=m&(m-1);
      		}
         /* moves right forwards */
         /* I: columns 1357 */
         m=((p->wk&RF1)<<4)&free;
         while(m)
   			{
            tmp=(m&-m);
            tmp=tmp|(tmp>>4);
            movelist[n].bm=0;
            movelist[n].bk=0;
            movelist[n].wm=0;
            movelist[n].wk=tmp;
      		values[n]=0;
            n++;
      		m=m&(m-1);
      		}
         /* II: columns 2468 */
         m=((p->wk&RF2)<<5)&free;
         while(m)
   			{
            tmp=(m&-m);
            tmp=tmp|(tmp>>5);
            movelist[n].bm=0;
            movelist[n].bk=0;
            movelist[n].wm=0;
            movelist[n].wk=tmp;
      		values[n]=0;
            n++;
      		m=m&(m-1);
      		}

         /* moves left backwards */
         /* I: columns 1357 */
         m=((p->wk&LB1)>>5)&free;
         /* now m contains a bit for every free square where a black man can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            tmp=tmp|(tmp<<5);
            movelist[n].bm=0;
            movelist[n].bk=0;
            movelist[n].wm=0;
            movelist[n].wk=tmp;
      		values[n]=0;
            n++;
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         /* II: columns 2468 */
         m=((p->wk&LB2)>>4)&free;
         while(m)
   			{
            tmp=(m&-m);
            tmp=tmp|(tmp<<4);
            movelist[n].bm=0;
            movelist[n].bk=0;
            movelist[n].wm=0;
            movelist[n].wk=tmp;
      		values[n]=0;
            n++;
      		m=m&(m-1);
      		}

         /* moves right backwards */
         /* I: columns 1357 */
         m=((p->wk&RB1)>>4)&free;
         while(m)
   			{
            tmp=(m&-m);
            tmp=tmp|(tmp<<4);
            movelist[n].bm=0;
            movelist[n].bk=0;
            movelist[n].wm=0;
            movelist[n].wk=tmp;
      		values[n]=0;
            n++;
      		m=m&(m-1);
      		}
         /* II: columns 2468 */
         m=((p->wk&RB2)>>3)&free;
         while(m)
   			{
            tmp=(m&-m);
            tmp=tmp|(tmp<<3);
            movelist[n].bm=0;
            movelist[n].bk=0;
            movelist[n].wm=0;
            movelist[n].wk=tmp;
      		values[n]=0;
            n++;
      		m=m&(m-1);
      		}
         }

       /* moves with white stones:*/
      if(p->wm)
      	{
         /* moves left backwards */
         /* II: columns 2468 ;just moves*/
         m=((p->wm&LB2)>>4)&free;
         while(m)
   			{
            tmp=(m&-m);
            tmp=tmp|(tmp<<4);
            movelist[n].bm=0;
            movelist[n].bk=0;
            movelist[n].wm=tmp&NBBR;
            movelist[n].wk=tmp&BBR;
				values[n]=0;
				n++;
      		m=m&(m-1);
      		}
         /* I: columns 1357 */
         m=((p->wm&LB1)>>5)&free;
         /* now m contains a bit for every free square where a white man can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            tmp=tmp|(tmp<<5);
            movelist[n].bm=0;
            movelist[n].bk=0;
            movelist[n].wm=tmp;
            movelist[n].wk=0;
      		values[n]=0;

            n++;
      		m=m&(m-1);   /* clears least significant bit of m */
      		}

         /* moves right backwards */

         /* II: columns 2468 : just the moves*/
         m=((p->wm&RB2)>>3)&free;
         while(m)
   			{
            tmp=(m&-m);
            tmp=tmp|(tmp<<3);
            movelist[n].bm=0;
            movelist[n].bk=0;
            movelist[n].wm=tmp&NBBR;
            movelist[n].wk=tmp&BBR;
				values[n]=0;
				n++;
      		m=m&(m-1);
      		}
         /* I: columns 1357 */
         m=((p->wm&RB1)>>4)&free;
         while(m)
   			{
            tmp=(m&-m);
            tmp=tmp|(tmp<<4);
            movelist[n].bm=0;
            movelist[n].bk=0;
            movelist[n].wm=tmp;
            movelist[n].wk=0;
      		values[n]=0;
            n++;
      		m=m&(m-1);
      		}
         }
#ifndef MOVEORDERING
		return n;
#endif
#ifdef MOHASH
      /* sort moves first */
		values[bestindex]+=HASHMOVE;
		
#endif
#ifdef MOKILLER
      if(n>1)
      	{
         /* give the forcefirst-move a high eval */
         if(killer)
         	{
            for(i=0;i<n;i++)
         		{
            	if((movelist[i].wm|movelist[i].wk) == killer)
            		{
      				values[i]+=KILLER;
               		
					break;
               		}
				}
            }
         }
#endif

  		if(n>1) /* in this case we have to order the list */
      	{
         whiteorderevaluation(p,movelist,values,n);
	      }
		return n;
      }
   }


static void blackorderevaluation(struct pos *p,struct move ml[MAXMOVES],int values[MAXMOVES],int n)
	{
   int eval;
   int32 from,to;
   int32 black;
	int i;

#ifdef MOHISTORY
   extern int32 history[32][32]; /*has entries for how often a move was good */
   extern int hashstores;        /* is the number of entries in history list */
#endif
	
	black=p->bm|p->bk;

	for(i=0;i<n;i++)
		{
   	eval=128;
  	
      from=(ml[i].bm|ml[i].bk)&black;
      to=(ml[i].bm|ml[i].bk)&(~black);
#ifdef MOHISTORY
      /* history...*/
      if(hashstores>MINHASH) eval+=( (HISTORY*history[LSB(from)][LSB(to)]) / (hashstores));  // vtune: if is loopindependent - take out
#endif

#ifdef MOSTATIC
   
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
		if(ml[i].bm)
      	{
         /* man going down the board */
			if(to&0xF0000000)
				eval+=PROM;
      	if(to&0x0F000000)
      		eval+=PROM1;
         if(to&0x00F00000)
         	eval+=PROM2;
         /* man giving up back rank */
         if(from&0x0000000E)
         	eval-=GIVEUPBACK;
         /* centralization for men */
         if(to&C3)
      		eval+=MANC3VAL;
   		if(from&C3)
      		eval-=MANC3VAL;
      	if(to&C4)
      		eval+=MANC4VAL;
      	if(from&C4)
      		eval-=MANC4VAL;
         }
		if(ml[i].bk)
      	{
         /* it's a move with a king */
         if(to&C4)
         	eval+=KINGC4VAL;
         if(to&C3)
         	eval+=KINGC3VAL;
         if(to&C1)
         	eval+=KINGC1VAL;
         if(from&C4)
         	eval-=KINGC4VAL;
         if(from&C3)
         	eval-=KINGC3VAL;
         if(from&C1)
         	eval-=KINGC1VAL;
         }

   	/*    (white)
   				 37  38  39  40
              32  33  34  35
                28  29  30  31
              23  24  25  26
                19  20  21  22
              14  15  16  17
                10  11  12  13
               5   6   7   8
         (black)   */
#endif
#ifdef MOTESTCAPT
       /* toggle move */
       p->bm^=ml[i].bm;
       p->bk^=ml[i].bk;
       if(testcapture(p,WHITE)) eval-=CAPT;
       else
       	{if(testcapture(p,BLACK)) eval+=CAPT;}
       /* toggle move */
       p->bm^=ml[i].bm;
       p->bk^=ml[i].bk;
#endif
   	values[i]+=eval;
   	}
   return;
   }

static void whiteorderevaluation(struct pos *p,struct move ml[MAXMOVES],int values[MAXMOVES],int n)
	{
   int eval;
   int32 from,to;
   int32 white;
   int i;

   extern int32 history[32][32];
   extern int hashstores;

	white=p->wm|p->wk;

	for(i=0;i<n;i++)
		{
   	eval=128;
      
      from=( (ml[i].wm)|(ml[i].wk) )&white;
      to=((ml[i].wm)|(ml[i].wk))&(~white);
#ifdef MOHISTORY
      /* history...*/
      if(hashstores>MINHASH)
      eval+=( (HISTORY*history[LSB(from)][LSB(to)]) / (hashstores));
#endif

#ifdef MOSTATIC
   
		if(ml[i].wm)
      	{
         /* man going down the board */
			if(to&0x0000000F)
				eval+=PROM;
			if(to&0x000000F0)
      		eval+=PROM1;
         if(to&0x00000F00)
         	eval+=PROM2;
         /* man giving up back rank */
         if(from&0x70000000)
         	eval-=GIVEUPBACK;
         /* centralization for men */
         if(to&C3)
      		eval+=MANC3VAL;
   		if(from&C3)
      		eval-=MANC3VAL;
      	if(to&C4)
      		eval+=MANC4VAL;
      	if(from&C4)
      		eval-=MANC4VAL;
         }
		if(ml[i].wk)
      	{
         /* it's a move with a king */
         if(to&C4)
         	eval+=KINGC4VAL;
			if(to&C3)
         	eval+=KINGC3VAL;
			if(to&C1)
         	eval+=KINGC1VAL;
			if(from&C4)
         	eval-=KINGC4VAL;
         if(from&C3)
         	eval-=KINGC3VAL;
         if(from&C1)
         	eval-=KINGC1VAL;
         }      /* toggle move */
#endif
#ifdef MOTESTCAPT
       p->wm^=ml[i].wm;
       p->wk^=ml[i].wk;
       if(testcapture(p,BLACK)) eval-=CAPT;
       else
       	{if(testcapture(p,WHITE)) eval+=CAPT;}
       /* toggle move */
       p->wm^=ml[i].wm;
       p->wk^=ml[i].wk;
#endif
    	values[i]+=eval;
   	}
   return;
   }


/******************************************************************************/
/* capture list */
/******************************************************************************/

/* used to be captgen.c */
/* generates the capture moves */


int makecapturelist(struct pos *p,struct move movelist[MAXMOVES],int values[MAXMOVES],int color, int32 bestindex)
	{
   int32 free,free2,m,tmp,white,black,white2,black2;
   int n=0;
   struct move partial;
   struct pos q;

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

   free=~(p->bm|p->bk|p->wm|p->wk);
   if(color==BLACK)
   	{
      if(p->bm)
      	{
         /* captures with black men! */
         white=p->wm|p->wk;
      	/* jumps left forwards with men*/
      	m=((((p->bm&LFJ2)<<4)&white)<<3)&free;
      	/* now m contains a bit for every free square where a black man can move*/
         while(m)
   			{
            /* find a move */
            tmp=(m&-m); /* least significant bit of m */
            partial.bm=(tmp|(tmp>>7))&NWBR;  /* NWBR: not white back rank */
            partial.bk=(tmp|(tmp>>7))&WBR;  /*if stone moves to WBR (white back rank) it's a king*/
            partial.wm=(tmp>>3)&p->wm;
            partial.wk=(tmp>>3)&p->wk;
            /* toggle it */
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            /* recursion */
            /* only if black has another capture move! */
            white2=p->wm|p->wk;
            free2=~(p->wm|p->wk|p->bm|p->bk);
            if ( (((((tmp&LFJ2)<<4)&white2)<<3)&free2) | (((((tmp&RFJ2)<<5)&white2)<<4)&free2))
            	blackmancapture2(&q,movelist,values, &n, &partial,tmp);
            else
            	{
               /* save move */
					/*
               partial.info=MANCAPT*recbitcount(partial.wm);
               partial.info+=KINGCAPT*recbitcount(partial.wk);
               if(tmp&CENTER) partial.info+=MCV;
               if(tmp&WBR)
               	{
                  partial.info+=PVVAL;
                  }*/
      			movelist[n]=partial;
					values[n]=MANCAPT*recbitcount(partial.wm);
               values[n]+=KINGCAPT*recbitcount(partial.wk);
               if(tmp&CENTER) values[n]+=MCV;
               if(tmp&WBR)
               	{
                  values[n]+=PVVAL;
                  }
      			n++;
               }
            
            /* clears least significant bit of m, associated with that move. */
      		m=m&(m-1);
      		}
         m=((((p->bm&LFJ1)<<3)&white)<<4)&free;
      	/* now m contains a bit for every free square where a black man can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.bm=(tmp|(tmp>>7));
            partial.bk=0;
            partial.wm=(tmp>>4)&p->wm;
            partial.wk=(tmp>>4)&p->wk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            

            white2=p->wm|p->wk;
            free2=~(p->wm|p->wk|p->bm|p->bk);
            if ( ( ((((tmp&LFJ1)<<3)&white2)<<4)&free2 ) | ( ((((tmp&RFJ1)<<4)&white2)<<5)&free2 ))
            	blackmancapture1(&q,movelist, values,&n,&partial,tmp);
            else
            	{
               /* save move */
               values[n]=MANCAPT*recbitcount(partial.wm);
               values[n]+=KINGCAPT*recbitcount(partial.wk);
               if(tmp&CENTER) values[n]+=MCV;

      			movelist[n]=partial;
      			n++;
               }

            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         /* jumps right forwards with men*/
         m=((((p->bm&RFJ2)<<5)&white)<<4)&free;
      	/* now m contains a bit for every free square where a black man can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.bm=(tmp|(tmp>>9))&NWBR;
            partial.bk=(tmp|(tmp>>9))&WBR;
            partial.wm=(tmp>>4)&p->wm;
            partial.wk=(tmp>>4)&p->wk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            
            white2=p->wm|p->wk;
            free2=~(p->wm|p->wk|p->bm|p->bk);
            if ( ( ((((tmp&LFJ2)<<4)&white2)<<3)&free2 ) | ( ((((tmp&RFJ2)<<5)&white2)<<4)&free2 ))
            	blackmancapture2(&q,movelist, values,&n, &partial,tmp);
            else
            	{
               /* save move */
               values[n]=MANCAPT*recbitcount(partial.wm);
               values[n]+=KINGCAPT*recbitcount(partial.wk);
               if(tmp&CENTER) values[n]+=MCV;
               if(tmp&WBR)
               	{
                  values[n]+=PVVAL;
               	}
      			movelist[n]=partial;
      			n++;
               }
            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         m=((((p->bm&RFJ1)<<4)&white)<<5)&free;
      	/* now m contains a bit for every free square where a black man can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.bm=tmp|(tmp>>9);
            partial.bk=0;
            partial.wm=(tmp>>5)&p->wm;
            partial.wk=(tmp>>5)&p->wk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            

            white2=p->wm|p->wk;
            free2=~(p->wm|p->wk|p->bm|p->bk);
            if ( ( ((((tmp&LFJ1)<<3)&white2)<<4)&free2 ) | ( ((((tmp&RFJ1)<<4)&white2)<<5)&free2 ))
            	blackmancapture1(&q,movelist,values, &n, &partial,tmp);
            else
            	{
               /* save move */
               values[n]=MANCAPT*recbitcount(partial.wm);
               values[n]+=KINGCAPT*recbitcount(partial.wk);
               if(tmp&CENTER) values[n]+=MCV;
      			movelist[n]=partial;
      			n++;
               }

            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
      	}
      if(p->bk)
      	{
         white=p->wm|p->wk;
      	/* jumps left forwards with black kings*/
      	m=((((p->bk&LFJ1)<<3)&white)<<4)&free;
      	/* now m contains a bit for every free square where a black king can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.bm=0;
            partial.bk=(tmp|(tmp>>7));
            partial.wm=(tmp>>4)&p->wm;
            partial.wk=(tmp>>4)&p->wk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            
            blackkingcapture1(&q,movelist,values, &n, &partial,tmp);
            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         m=((((p->bk&LFJ2)<<4)&white)<<3)&free;
      	/* now m contains a bit for every free square where a black king can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.bm=0;
            partial.bk=(tmp|(tmp>>7));
            partial.wm=(tmp>>3)&p->wm;
            partial.wk=(tmp>>3)&p->wk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            
            blackkingcapture2(&q,movelist, values,&n, &partial,tmp);
            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         /* jumps right forwards with black kings*/
      	m=((((p->bk&RFJ1)<<4)&white)<<5)&free;
      	/* now m contains a bit for every free square where a black king can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.bm=0;
            partial.bk=tmp|(tmp>>9);
            partial.wm=(tmp>>5)&p->wm;
            partial.wk=(tmp>>5)&p->wk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            
            blackkingcapture1(&q,movelist, values,&n, &partial,tmp);
            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         m=((((p->bk&RFJ2)<<5)&white)<<4)&free;
      	/* now m contains a bit for every free square where a black king can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.bm=0;
            partial.bk=(tmp|(tmp>>9));
            partial.wm=(tmp>>4)&p->wm;
            partial.wk=(tmp>>4)&p->wk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            
            blackkingcapture2(&q,movelist,values, &n, &partial,tmp);
            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}

      	/* jumps left backwards with black kings*/
      	m=((((p->bk&LBJ1)>>5)&white)>>4)&free;
      	/* now m contains a bit for every free square where a black king can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.bm=0;
            partial.bk=(tmp|(tmp<<9));
            partial.wm=(tmp<<4)&p->wm;
            partial.wk=(tmp<<4)&p->wk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            
            blackkingcapture1(&q,movelist, values,&n, &partial,tmp);
            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         m=((((p->bk&LBJ2)>>4)&white)>>5)&free;
      	/* now m contains a bit for every free square where a black king can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.bm=0;
            partial.bk=(tmp|(tmp<<9));
            partial.wm=(tmp<<5)&p->wm;
            partial.wk=(tmp<<5)&p->wk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            
            blackkingcapture2(&q,movelist,values, &n, &partial,tmp);
            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         /* jumps right backwards with black kings*/
      	m=((((p->bk&RBJ1)>>4)&white)>>3)&free;
      	/* now m contains a bit for every free square where a black king can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.bm=0;
            partial.bk=tmp|(tmp<<7);
            partial.wm=(tmp<<3)&p->wm;
            partial.wk=(tmp<<3)&p->wk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            
            blackkingcapture1(&q,movelist, values,&n, &partial,tmp);
            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         m=((((p->bk&RBJ2)>>3)&white)>>4)&free;
      	/* now m contains a bit for every free square where a black king can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.bm=0;
            partial.bk=(tmp|(tmp<<7));
            partial.wm=(tmp<<4)&p->wm;
            partial.wk=(tmp<<4)&p->wk;
            
   			q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            blackkingcapture2(&q,movelist, values,&n, &partial,tmp);
            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         }
#ifndef MOVEORDERING
		return n;
#endif
#ifdef MOHASH
      values[bestindex]+=HASHMOVE;
#endif
		return n;
      }
   else /*******************COLOR IS WHITE *********************************/
      {
      if(p->wm)
      	{
         black=p->bm|p->bk;
      	/* jumps left backwards with men*/
      	m=((((p->wm&LBJ1)>>5)&black)>>4)&free;
      	/* now m contains a bit for every free square where a white man can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.wm=(tmp|(tmp<<9))&NBBR;
            partial.wk=(tmp|(tmp<<9))&BBR;
            partial.bm=(tmp<<4)&p->bm;
            partial.bk=(tmp<<4)&p->bk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            /* only if white has another capture move! */
            black2=p->bm|p->bk;
            free2=~(p->wm|p->wk|p->bm|p->bk);
            if ( ( ((((tmp&LBJ1)>>5)&black2)>>4)&free2) | ( ((((tmp&RBJ1)>>4)&black2)>>3)&free2 ))
            	whitemancapture1(&q,movelist,values, &n, &partial,tmp);
            else
            	{
               /* save move */
               values[n]=MANCAPT*recbitcount(partial.bm);
               values[n]+=KINGCAPT*recbitcount(partial.bk);
               if(tmp&CENTER) values[n]+=MCV;
               if(partial.wk)
               	{
                  values[n]+=PVVAL;
                  }
      			movelist[n]=partial;
      			n++;
               }

            
            m=m&(m-1);   /* clears least significant bit of m */
      		}
         m=((((p->wm&LBJ2)>>4)&black)>>5)&free;
      	/* now m contains a bit for every free square where a white man can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.wm=(tmp|(tmp<<9));
            partial.wk=0;
            partial.bm=(tmp<<5)&p->bm;
            partial.bk=(tmp<<5)&p->bk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
				black2=p->bm|p->bk;
            free2=~(p->wm|p->wk|p->bm|p->bk);
            if ( ( ((((tmp&LBJ2)>>4)&black2)>>5)&free2) | ( ((((tmp&RBJ2)>>3)&black2)>>4)&free2 ))
            	whitemancapture2(&q,movelist, values,&n, &partial,tmp);
            else
            	{
               /* save move */
               values[n]=MANCAPT*recbitcount(partial.bm);
               values[n]+=KINGCAPT*recbitcount(partial.bk);
               if(tmp&CENTER) values[n]+=MCV;
      			movelist[n]=partial;
      			n++;
               }

            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         /* jumps right backwards with men*/
      	m=((((p->wm&RBJ1)>>4)&black)>>3)&free;
      	/* now m contains a bit for every free square where a white man can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.wm=(tmp|(tmp<<7))&NBBR;
            partial.wk=(tmp|(tmp<<7))&BBR;
            partial.bm=(tmp<<3)&p->bm;
            partial.bk=(tmp<<3)&p->bk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            black2=p->bm|p->bk;
            free2=~(p->wm|p->wk|p->bm|p->bk);
            if ( ( ((((tmp&LBJ1)>>5)&black2)>>4)&free2) | ( ((((tmp&RBJ1)>>4)&black2)>>3)&free2 ))
            	whitemancapture1(&q,movelist,values, &n, &partial,tmp);
            else
            	{
               /* save move */
               values[n]=MANCAPT*recbitcount(partial.bm);
               values[n]+=KINGCAPT*recbitcount(partial.bk);
               if(tmp&CENTER) values[n]+=MCV;
               if(partial.wk)
               	{
                  values[n]+=PVVAL;
                  }
      			movelist[n]=partial;
      			n++;
               }
            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         m=((((p->wm&RBJ2)>>3)&black)>>4)&free;
      	/* now m contains a bit for every free square where a black man can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.wm=(tmp|(tmp<<7));
            partial.wk=0;
            partial.bm=(tmp<<4)&p->bm;
            partial.bk=(tmp<<4)&p->bk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            black2=p->bm|p->bk;
            free2=~(p->wm|p->wk|p->bm|p->bk);
            if ( ( ((((tmp&LBJ2)>>4)&black2)>>5)&free2) | ( ((((tmp&RBJ2)>>3)&black2)>>4)&free2 ))
            	whitemancapture2(&q,movelist,values, &n, &partial,tmp);
            else
            	{
               /* save move */
               values[n]=MANCAPT*recbitcount(partial.bm);
               values[n]+=KINGCAPT*recbitcount(partial.bk);
               if(tmp&CENTER) values[n]+=MCV;
      			movelist[n]=partial;
      			n++;
               }
            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
      	}
      if(p->wk)
      	{
         black=p->bm|p->bk;
      	/* jumps left forwards with white kings*/
      	m=((((p->wk&LFJ1)<<3)&black)<<4)&free;
      	/* now m contains a bit for every free square where a white king can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.wm=0;
            partial.wk=(tmp|(tmp>>7));
            partial.bm=(tmp>>4)&p->bm;
            partial.bk=(tmp>>4)&p->bk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            whitekingcapture1(&q,movelist,values, &n, &partial,tmp);
            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         m=((((p->wk&LFJ2)<<4)&black)<<3)&free;
      	/* now m contains a bit for every free square where a white king can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.wm=0;
            partial.wk=(tmp|(tmp>>7));
            partial.bm=(tmp>>3)&p->bm;
            partial.bk=(tmp>>3)&p->bk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            whitekingcapture2(&q,movelist,values, &n, &partial,tmp);
            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         /* jumps right forwards with white kings*/
      	m=((((p->wk&RFJ1)<<4)&black)<<5)&free;
      	/* now m contains a bit for every free square where a white king can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.wm=0;
            partial.wk=tmp|(tmp>>9);
            partial.bm=(tmp>>5)&p->bm;
            partial.bk=(tmp>>5)&p->bk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            whitekingcapture1(&q,movelist,values, &n, &partial,tmp);
            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         m=((((p->wk&RFJ2)<<5)&black)<<4)&free;
      	/* now m contains a bit for every free square where a white king can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.wm=0;
            partial.wk=(tmp|(tmp>>9));
            partial.bm=(tmp>>4)&p->bm;
            partial.bk=(tmp>>4)&p->bk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            whitekingcapture2(&q,movelist, values,&n, &partial,tmp);
            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}


      	/* jumps left backwards with white kings*/
      	m=((((p->wk&LBJ1)>>5)&black)>>4)&free;
      	/* now m contains a bit for every free square where a white king can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.wm=0;
            partial.wk=(tmp|(tmp<<9));
            partial.bm=(tmp<<4)&p->bm;
            partial.bk=(tmp<<4)&p->bk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            whitekingcapture1(&q,movelist,values, &n, &partial,tmp);
            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         m=((((p->wk&LBJ2)>>4)&black)>>5)&free;
      	/* now m contains a bit for every free square where a white king can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.wm=0;
            partial.wk=(tmp|(tmp<<9));
            partial.bm=(tmp<<5)&p->bm;
            partial.bk=(tmp<<5)&p->bk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            whitekingcapture2(&q,movelist, values,&n, &partial,tmp);
            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         /* jumps right backwards with white kings*/
      	m=((((p->wk&RBJ1)>>4)&black)>>3)&free;
      	/* now m contains a bit for every free square where a white king can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.wm=0;
            partial.wk=tmp|(tmp<<7);
            partial.bm=(tmp<<3)&p->bm;
            partial.bk=(tmp<<3)&p->bk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            whitekingcapture1(&q,movelist, values,&n, &partial,tmp);
            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         m=((((p->wk&RBJ2)>>3)&black)>>4)&free;
      	/* now m contains a bit for every free square where a white king can move*/
         while(m)
   			{
            tmp=(m&-m); /* least significant bit of m */
            partial.wm=0;
            partial.wk=(tmp|(tmp<<7));
            partial.bm=(tmp<<4)&p->bm;
            partial.bk=(tmp<<4)&p->bk;
            q.bm=p->bm^partial.bm;
   			q.bk=p->bk^partial.bk;
   			q.wm=p->wm^partial.wm;
   			q.wk=p->wk^partial.wk;
            whitekingcapture2(&q,movelist,values, &n, &partial,tmp);
            
      		m=m&(m-1);   /* clears least significant bit of m */
      		}
         }
#ifndef MOVEORDERING
		return n;
#endif
#ifdef MOHASH
      /* sort moves: according to movelist[n].info */
      values[bestindex]+=HASHMOVE;
#endif
		return n;
      }
   }

static void blackmancapture1(struct pos *p,struct move movelist[MAXMOVES],int values[MAXMOVES],int *n, struct move *partial, int32 square)
	{
   /* partial move has already been executed. seek LFJ1 and RFJ1 */
   int32 m,free,white;
   int found=0;
   struct move next_partial,whole_partial;
	struct pos q;

   free=~(p->bm|p->bk|p->wm|p->wk);
   white=p->wm|p->wk;
   /* left forward jump */
   m=((((square&LFJ1)<<3)&white)<<4)&free;
   if(m)
   	{
      next_partial.bm=(m|(m>>7));
      next_partial.bk=0;
      next_partial.wm=(m>>4)&p->wm;
      next_partial.wk=(m>>4)&p->wk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      blackmancapture1(&q,movelist,values,n,&whole_partial,m);
     
      found=1;
      }

   /* right forward jump */
   m=((((square&RFJ1)<<4)&white)<<5)&free;
   if(m)
   	{
      next_partial.bm=(m|(m>>9));
      next_partial.bk=0;
      next_partial.wm=(m>>5)&p->wm;
      next_partial.wk=(m>>5)&p->wk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      blackmancapture1(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   if(!found)
   	{
      /* no continuing jumps - save the move in the movelist */
      /* set info tag in move */
		values[*n]=MANCAPT*recbitcount(partial->wm);
      values[*n]+=KINGCAPT*recbitcount(partial->wk);
      if(square&CENTER) values[*n]+=MCV;
      movelist[*n]=*partial;
      (*n)++;
      }
   }

static void blackmancapture2(struct pos *p,struct move movelist[MAXMOVES],int values[MAXMOVES],int *n, struct move *partial, int32 square)
	{
   /* partial move has already been executed. seek LFJ2 and RFJ2 */
   /* additional complication: black stone might crown here */
   int32 m,free,white;
   struct move next_partial,whole_partial;
   int found=0;
	struct pos q;

   free=~(p->bm|p->bk|p->wm|p->wk);
   white=p->wm|p->wk;
   /* left forward jump */
   m=((((square&LFJ2)<<4)&white)<<3)&free;
   if(m)
   	{
      next_partial.bm=(m|(m>>7))&NWBR;
      next_partial.bk=(m|(m>>7))&WBR;
      next_partial.wm=(m>>3)&p->wm;
      next_partial.wk=(m>>3)&p->wk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      blackmancapture2(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   /* right forward jump */
   m=((((square&RFJ2)<<5)&white)<<4)&free;
   if(m)
   	{
      next_partial.bm=(m|(m>>9))&NWBR;
      next_partial.bk=(m|(m>>9))&WBR;
      next_partial.wm=(m>>4)&p->wm;
      next_partial.wk=(m>>4)&p->wk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      blackmancapture2(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   if(!found)
   	{
      /* no continuing jumps - save the move in the movelist */
      values[*n]=MANCAPT*recbitcount(partial->wm);
      values[*n]+=KINGCAPT*recbitcount(partial->wk);
      if(square&CENTER) values[*n]+=MCV;
      if(partial->bk)
        	{
         values[*n]+=PVVAL;
         }
      movelist[*n]=*partial;
      (*n)++;
      }
   }


static void blackkingcapture1(struct pos *p,struct move movelist[MAXMOVES],int values[MAXMOVES],int *n, struct move *partial, int32 square)
	{
   /* partial move has already been executed. seek LFJ1 RFJ1 LBJ1 RBJ1*/
   int32 m,free,white;
   struct move next_partial,whole_partial;
   int found=0;
	struct pos q;

   free=~(p->bm|p->bk|p->wm|p->wk);
   white=p->wm|p->wk;
   /* left forward jump */
   m=((((square&LFJ1)<<3)&white)<<4)&free;
   if(m)
   	{
      next_partial.bm=0;
      next_partial.bk=(m|(m>>7));
      next_partial.wm=(m>>4)&p->wm;
      next_partial.wk=(m>>4)&p->wk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      blackkingcapture1(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   /* right forward jump */
   m=((((square&RFJ1)<<4)&white)<<5)&free;
   if(m)
   	{
      next_partial.bm=0;
      next_partial.bk=(m|(m>>9));
      next_partial.wm=(m>>5)&p->wm;
      next_partial.wk=(m>>5)&p->wk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      blackkingcapture1(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   /* left backward jump */
   m=((((square&LBJ1)>>5)&white)>>4)&free;
   if(m)
   	{
      next_partial.bm=0;
      next_partial.bk=(m|(m<<9));
      next_partial.wm=(m<<4)&p->wm;
      next_partial.wk=(m<<4)&p->wk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      blackkingcapture1(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   /* right backward jump */
   m=((((square&RBJ1)>>4)&white)>>3)&free;
   if(m)
   	{
      next_partial.bm=0;
      next_partial.bk=(m|(m<<7));
      next_partial.wm=(m<<3)&p->wm;
      next_partial.wk=(m<<3)&p->wk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      blackkingcapture1(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   if(!found)
   	{
      /* no continuing jumps - save the move in the movelist */
      values[*n]=MANCAPT*recbitcount(partial->wm);
      values[*n]+=KINGCAPT*recbitcount(partial->wk);
      if(square&CENTER) values[*n]+=KCV;
      movelist[*n]=*partial;
      (*n)++;
      }
   }
static void blackkingcapture2(struct pos *p,struct move movelist[MAXMOVES],int values[MAXMOVES],int *n, struct move *partial, int32 square)
	{
   /* partial move has already been executed. seek LFJ1 RFJ1 LBJ1 RBJ1*/
   int32 m,free,white;
   struct move next_partial,whole_partial;
   int found=0;
	struct pos q;

   free=~(p->bm|p->bk|p->wm|p->wk);
   white=p->wm|p->wk;
   /* left forward jump */
   m=((((square&LFJ2)<<4)&white)<<3)&free;
   if(m)
   	{
      next_partial.bm=0;
      next_partial.bk=(m|(m>>7));
      next_partial.wm=(m>>3)&p->wm;
      next_partial.wk=(m>>3)&p->wk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      blackkingcapture2(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   /* right forward jump */
   m=((((square&RFJ2)<<5)&white)<<4)&free;
   if(m)
   	{
      next_partial.bm=0;
      next_partial.bk=(m|(m>>9));
      next_partial.wm=(m>>4)&p->wm;
      next_partial.wk=(m>>4)&p->wk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      blackkingcapture2(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   /* left backward jump */
   m=((((square&LBJ2)>>4)&white)>>5)&free;
   if(m)
   	{
      next_partial.bm=0;
      next_partial.bk=(m|(m<<9));
      next_partial.wm=(m<<5)&p->wm;
      next_partial.wk=(m<<5)&p->wk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      blackkingcapture2(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   /* right backward jump */

   m=((((square&RBJ2)>>3)&white)>>4)&free;
   if(m)
   	{
      next_partial.bm=0;
      next_partial.bk=(m|(m<<7));
      next_partial.wm=(m<<4)&p->wm;
      next_partial.wk=(m<<4)&p->wk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      blackkingcapture2(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   if(!found)
   	{
      /* no continuing jumps - save the move in the movelist */
      values[*n]=MANCAPT*recbitcount(partial->wm);
      values[*n]+=KINGCAPT*recbitcount(partial->wk);
      if(square&CENTER) values[*n]+=KCV;
      movelist[*n]=*partial;
      (*n)++;
      }
   }

static void whitemancapture1(struct pos *p,struct move movelist[MAXMOVES],int values[MAXMOVES],int *n, struct move *partial, int32 square)
	{
   /* partial move has already been executed. seek LBJ1 and RBJ1 */
   int32 m,free,black;
   struct move next_partial,whole_partial;
   int found=0;
	struct pos q;

   free=~(p->bm|p->bk|p->wm|p->wk);
   black=p->bm|p->bk;
   /* left backward jump */
   m=((((square&LBJ1)>>5)&black)>>4)&free;
   if(m)
   	{
      next_partial.wm=(m|(m<<9))&NBBR;
      next_partial.wk=(m|(m<<9))&BBR;
      next_partial.bm=(m<<4)&p->bm;
      next_partial.bk=(m<<4)&p->bk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      whitemancapture1(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   /* right backward jump */
   m=((((square&RBJ1)>>4)&black)>>3)&free;
   if(m)
   	{
      next_partial.wm=(m|(m<<7))&NBBR;
      next_partial.wk=(m|(m<<7))&BBR;
      next_partial.bm=(m<<3)&p->bm;
      next_partial.bk=(m<<3)&p->bk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      whitemancapture1(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   if(!found)
   	{
      /* no continuing jumps - save the move in the movelist */
      values[*n]=MANCAPT*recbitcount(partial->bm);
      values[*n]+=KINGCAPT*recbitcount(partial->bk);
      if(square&CENTER) values[*n]+=MCV;
      if(partial->wk)
      	{
         values[*n]+=PVVAL;
         }
      movelist[*n]=*partial;
      (*n)++;
      }
   }
static void whitemancapture2(struct pos *p,struct move movelist[MAXMOVES],int values[MAXMOVES],int *n, struct move *partial, int32 square)
	{
   /* partial move has already been executed. seek LBJ1 and RBJ1 */
   int32 m,free,black;
   struct move next_partial,whole_partial;
   int found=0;
	struct pos q;

   free=~(p->bm|p->bk|p->wm|p->wk);
   black=p->bm|p->bk;
   /* left backward jump */
   m=((((square&LBJ2)>>4)&black)>>5)&free;
   if(m)
   	{
      next_partial.wm=(m|(m<<9));
      next_partial.wk=0;
      next_partial.bm=(m<<5)&p->bm;
      next_partial.bk=(m<<5)&p->bk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      whitemancapture2(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   /* right backward jump */
   m=((((square&RBJ2)>>3)&black)>>4)&free;
   if(m)
   	{
      next_partial.wm=(m|(m<<7));
      next_partial.wk=0;
      next_partial.bm=(m<<4)&p->bm;
      next_partial.bk=(m<<4)&p->bk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      whitemancapture2(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   if(!found)
   	{
      /* no continuing jumps - save the move in the movelist */
      values[*n]=MANCAPT*recbitcount(partial->bm);
      values[*n]+=KINGCAPT*recbitcount(partial->bk);
      if(square&CENTER) values[*n]+=MCV;
      movelist[*n]=*partial;
      (*n)++;
      }
   }

static void whitekingcapture1(struct pos *p,struct move movelist[MAXMOVES],int values[MAXMOVES],int *n, struct move *partial, int32 square)
	{
   /* partial move has already been executed. seek LFJ1 RFJ1 LBJ1 RBJ1*/
   int32 m,free,black;
   struct move next_partial,whole_partial;
   int found=0;
	struct pos q;

   free=~(p->bm|p->bk|p->wm|p->wk);
   black=p->bm|p->bk;
   /* left forward jump */
   m=((((square&LFJ1)<<3)&black)<<4)&free;
   if(m)
   	{
      next_partial.wm=0;
      next_partial.wk=(m|(m>>7));
      next_partial.bm=(m>>4)&p->bm;
      next_partial.bk=(m>>4)&p->bk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      whitekingcapture1(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   /* right forward jump */
   m=((((square&RFJ1)<<4)&black)<<5)&free;
   if(m)
   	{
      next_partial.wm=0;
      next_partial.wk=(m|(m>>9));
      next_partial.bm=(m>>5)&p->bm;
      next_partial.bk=(m>>5)&p->bk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      whitekingcapture1(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   /* left backward jump */
   m=((((square&LBJ1)>>5)&black)>>4)&free;
   if(m)
   	{
      next_partial.wm=0;
      next_partial.wk=(m|(m<<9));
      next_partial.bm=(m<<4)&p->bm;
      next_partial.bk=(m<<4)&p->bk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      whitekingcapture1(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   /* right backward jump */
   m=((((square&RBJ1)>>4)&black)>>3)&free;
   if(m)
   	{
      next_partial.wm=0;
      next_partial.wk=(m|(m<<7));
      next_partial.bm=(m<<3)&p->bm;
      next_partial.bk=(m<<3)&p->bk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      whitekingcapture1(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   if(!found)
   	{
      /* no continuing jumps - save the move in the movelist */
      values[*n]=MANCAPT*recbitcount(partial->bm);
      values[*n]+=KINGCAPT*recbitcount(partial->bk);
      if(square&CENTER) values[*n]+=KCV;
      movelist[*n]=*partial;
      (*n)++;
      }
   }
static void whitekingcapture2(struct pos *p,struct move movelist[MAXMOVES],int values[MAXMOVES],int *n, struct move *partial, int32 square)
	{
   /* partial move has already been executed. seek LFJ1 RFJ1 LBJ1 RBJ1*/
   int32 m,free,black;
   struct move next_partial,whole_partial;
   int found=0;
	struct pos q;

   free=~(p->bm|p->bk|p->wm|p->wk);
   black=p->bm|p->bk;
   /* left forward jump */
   m=((((square&LFJ2)<<4)&black)<<3)&free;
   if(m)
   	{
      next_partial.wm=0;
      next_partial.wk=(m|(m>>7));
      next_partial.bm=(m>>3)&p->bm;
      next_partial.bk=(m>>3)&p->bk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      whitekingcapture2(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   /* right forward jump */
   m=((((square&RFJ2)<<5)&black)<<4)&free;
   if(m)
   	{
      next_partial.wm=0;
      next_partial.wk=(m|(m>>9));
      next_partial.bm=(m>>4)&p->bm;
      next_partial.bk=(m>>4)&p->bk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      whitekingcapture2(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   /* left backward jump */
   m=((((square&LBJ2)>>4)&black)>>5)&free;
   if(m)
   	{
      next_partial.wm=0;
      next_partial.wk=(m|(m<<9));
      next_partial.bm=(m<<5)&p->bm;
      next_partial.bk=(m<<5)&p->bk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      whitekingcapture2(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   /* right backward jump */
   m=((((square&RBJ2)>>3)&black)>>4)&free;
   if(m)
   	{
      next_partial.wm=0;
      next_partial.wk=(m|(m<<7));
      next_partial.bm=(m<<4)&p->bm;
      next_partial.bk=(m<<4)&p->bk;
      q.bm=p->bm^next_partial.bm;
      q.bk=p->bk^next_partial.bk;
   	q.wm=p->wm^next_partial.wm;
   	q.wk=p->wk^next_partial.wk;
      
      whole_partial.bm=partial->bm^next_partial.bm;
      whole_partial.bk=partial->bk^next_partial.bk;
      whole_partial.wm=partial->wm^next_partial.wm;
      whole_partial.wk=partial->wk^next_partial.wk;
      whitekingcapture2(&q,movelist,values,n,&whole_partial,m);
      
      found=1;
      }

   if(!found)
   	{
      /* no continuing jumps - save the move in the movelist */
      values[*n]=MANCAPT*recbitcount(partial->bm);
      values[*n]+=KINGCAPT*recbitcount(partial->bk);
      if(square&CENTER) values[*n]+=KCV;
      movelist[*n]=*partial;
      (*n)++;
      }
   }

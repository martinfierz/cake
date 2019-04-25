
//
// mpc.c
//
// implements buro's multi-probcut
//
#ifdef MPC

#include "consts.h"
#include "structs.h"
#include "switches.h"
#include "cakepp.h"
#include "dblookup.h"
#include <time.h>

extern int cake_nodes;
extern double start;
extern int *play;
extern double aborttime;
extern int realdepth;
extern int leafs;
extern int leafdepth;
extern int searchmode;
extern int bm,bk,wm,wk;
extern int dblookups,dbfail,dbsuccess,maxdepth;
extern int Gkey, Glock;

#ifdef REPCHECK
extern struct pos Ghistory[MAXDEPTH+HISTORYOFFSET+10]; /*holds the current variation for repetition check*/
#endif

#define MPCREPEAT 4
#define MPCMINDEPTH 4
#define MPCDIVIDE 2
#define MPCCUTOFFVAL 5

#define SIGMAMULT 3 // how many sigma must a value be off to be considered too far out?



int mpc(struct pos *p, int d, int color, int alpha, int beta, int32 *protokiller)
	{
	int value, valuetype;
	int32 forcefirst=MAXMOVES-1;
	int localalpha=alpha, localbeta=beta, n, localeval=MATE;
	int32 Lkey,Llock;
	int l_bm,l_bk,l_wm,l_wk, maxvalue=-MATE;
	int delta=0;
#ifdef ETC
	int bestETCvalue;
#endif
	int i,j;
	int index, bestmovevalue;
	int bestindex;
	int32 Lkiller=0;
	int isrootnode=0;
	struct move movelist[MAXMOVES];
	int values[MAXMOVES];
	struct pos q;
	int stmcapture = 0, sntmcapture = 0;
	int dbresult;
	int gotdbvalue=0;
	int isdbpos = 0;
	int cl=0; // conditional lookup - predefined as no. if CL && CLDEPTH>d then it's 1.
	int mpccutoffvalue;
	int dummy;

#ifdef STOREEXACTDEPTH
	int originaldepth = d;
#endif
	// time check: abort search if we exceed the time given by aborttime
#ifdef CHECKTIME
	if((cake_nodes & 0xFFFF)==0)
		{
		if(searchmode==0)
			if( (clock()-start)/CLK_TCK>(aborttime)) (*play=1);
		}
#endif

	cake_nodes++;

	// check material:
	if(p->bm+p->bk==0)
		return color==BLACK?(-MATE+realdepth):(MATE-realdepth);
	if(p->wm+p->wk==0)
		return color==WHITE?(-MATE+realdepth):(MATE+realdepth);

	// return if calculation interrupt is requested
	if(*play) return 0;

	// stop search if maximal search depth is reached 
	if(realdepth>MAXDEPTH) 
		{
		leafs++;leafdepth+=realdepth;
		return evaluation(p,color,alpha,beta,&delta);
		}
	
	// search the current position in the hashtable 
	// only if there is still search depth left! 
	// should test this without d>0 !?
	if(d>0)
		{
		if(hashlookup(&value,&valuetype,d,&forcefirst,color,&dummy))
			{
			// return value 1: the position is in the hashtable and the depth
			// is sufficient. it's value and valuetype are used to shift bounds
			// or even cutoff 
			if(valuetype==EXACT)
				return value;
			if(valuetype==LOWER)
				{
				if(value>=beta) return value;
				if(value>localalpha) localalpha=value;
				}
			if(valuetype==UPPER)
				{
				if(value<=alpha) return value;
				if(value<localbeta) localbeta=value;
				}
			}
		}
#ifdef REPCHECK
	// check for repetitions 
	if(bk && wk)
		{
		for(i=realdepth+HISTORYOFFSET-2;i>=0;i-=2)
			{
			if((p->bm^Ghistory[i].bm)) break; // stop repetition search if move with a man is detected 
			if((p->wm^Ghistory[i].wm)) break; 
			if((p->bm==Ghistory[i].bm) && (p->bk==Ghistory[i].bk) && (p->wm==Ghistory[i].wm) && (p->wk==Ghistory[i].wk))
				return 0;                     // same position detected! 
			}
		}
#endif
	
	// get info on captures: can side to move or side not to move capture now?
	stmcapture = testcapture(p,color);
	// this should be ok: 
	if(!stmcapture)
		sntmcapture = testcapture(p,color^CC);

	if(stmcapture)
		n=makecapturelist(p,movelist,values,color,forcefirst);
    else
		n=0;


	//--------------------------------------------//
	// check for database use                     // 
	// conditions: -> #of men < maxNdb            //
	//			   -> no capture for either side  //
	//                else result is incorrect    //
	//--------------------------------------------//
#ifdef USEDB
	if(bm+bk+wm+wk<=MAXPIECES && max(bm+bk,wm+wk)<=MAXPIECE)
		{
		isdbpos = 1;
		if(!(stmcapture|sntmcapture))
			{
			// this position can be looked up!
			dblookups++;

#ifdef CL
			if(d<CLDEPTH*FRAC)
				cl=1;
#endif

			if(color==BLACK)
				dbresult = dblookup(p,DB_BLACK,cl);
			else
				dbresult = dblookup(p,DB_WHITE,cl);

			// statistics

			if(dbresult == DB_NOT_LOOKED_UP)
				dbfail++;
			else if(dbresult != DB_UNKNOWN)
				dbsuccess++;			

			if(dbresult == DB_DRAW)
				return 0;
			if(dbresult == DB_WIN)
				{
				value = dbwineval(p,color);
				if(value>=localbeta)
					return value;
				}
			if(dbresult == DB_LOSS)
				{
				value = dblosseval(p,color);
				if(value<=localalpha)
					return value;
				}
			}
		}
#endif // USEDB

// now, check return condition - never evaluate with a capture on board!
// when there is no capture it's easy:
	if(d<=0 && !stmcapture)
		{
		// normally, we'd be stopping here.
		// but if the side to move "has something", we shouldn't do this
		if(!sntmcapture)
			{
			if(realdepth>maxdepth) 
				maxdepth=realdepth;
			// no capture in this position - return
			return evaluation(p,color,localalpha,localbeta,&delta);
			}
		else
			{
			// returning a value with sntmcapture is risky, but probably good.
			// however, if this position is in the database, we don't want to 
			// do this.
			if(!isdbpos)
				{
				value = evaluation(p,color,localalpha,localbeta,&delta);
				// side not to move has a capture. what does this mean? it means that
				// possibly the value of this position will drop dramatically. therefore
				// this should be done asymmetrically: if value is already low, don't worry
				// too much. but if value is high, think about it again! value > localbeta+X*QLEVEL maybe?
				// QLEVEL:200, QLEVEL2:60. 
				// this takes care of this, i hope. at about 5% price opening & midgame, 2% endgame.
				// compared to 60/60
				if(value > localbeta+QLEVEL || value <localalpha-QLEVEL2)
					return value;
				}
			}
		}
		
		// if there is a capture for the side to move:
//	if(!stmcapture && sntmcapture && bm+bk+wm+wk>maxNdb)
//		return evaluation(p,color,localalpha,localbeta);	


// we could not return. prepare next iteration

	if(!n)
		n=makemovelist(p,movelist,values,color,forcefirst,*protokiller);
	if(n==0)
		return -MATE+realdepth;


/* check for single move and extend appropriately */
	if(n==1)
		d+=SINGLEEXTEND;

	/* save old hashkey and old material balance*/
	Lkey=Gkey;
	Llock=Glock;
	l_bm=bm;l_bk=bk;l_wm=wm;l_wk=wk;

	/* for all moves: domove, update hashkey&material, recursion, restore
		material balance and hashkey, undomove, do alphabetatest */
	/* set best move in case of only fail-lows */
	
#ifdef ETC
	if(d>ETCDEPTH)
		{
		bestETCvalue=ETClookup(movelist,d,n,color);
		if(bestETCvalue>=localbeta) 
			return bestETCvalue;
		if(bestETCvalue>localalpha) 
			localalpha=bestETCvalue;
		}
#endif

	/* MPC! */
	if( !(realdepth%MPCREPEAT) && d>MPCMINDEPTH)
		{
		/* the larger the remaining depth, the larger we must make the mpccutoffvalue */
		/* remember, FRAC is 4 */
		/* example: 10 ply remaining, cutoffvalue = 50 
		/*				20 ply remaining, cutoffvalue = 100 */
		// old version
		// mpccutoffvalue = d/FRAC * MPCCUTOFFVAL; 

		// new version based on evaluating a big logfile:
		// i find that sigma( value(d)-value(d/2)) ~ 12+0.55*d
		// leaving out all data points where both are 0, i get
		// ~ 12+5/6*d
		mpccutoffvalue = SIGMAMULT*(12 + (54*(d/FRAC))/64);
		//mpccutoffvalue = 100;
		value=mpcnegamax(p,d/MPCDIVIDE,color,alpha-mpccutoffvalue,alpha-mpccutoffvalue+1, &Lkiller);
		if(value<=alpha-mpccutoffvalue) return value;
		value=mpcnegamax(p,d/MPCDIVIDE,color,beta+mpccutoffvalue-1,beta+mpccutoffvalue,&Lkiller);
		if(value>=beta+mpccutoffvalue) return value;
		}


/*************** main "for all moves do" loop starts here ! *******************/
/* up to here it was just lots of other stuff - now we are ready for the work*/

	for(i=0;i<n;i++)
		{
		/* for all moves: domove, count material, recursive call, undo move, check condition */
		
		/* movelist contains moves, movelist.info contains their values. 
			have to find the best move! 
			we do this by linear search through the movelist[] array */
		
		index=0;
		bestmovevalue=-1;
	
		for(j=0;j<n;j++)
			{
			if(values[j]>bestmovevalue)
				{
				bestmovevalue=values[j];
				index=j;
				}
			}
		/* movelist[index] now holds the best move */
		/* set values[index] to -1, that means that this move will no longer be considered */
		values[index]=-1;
		
		/* domove */
		q.bk=p->bk^movelist[index].bk;
		q.bm=p->bm^movelist[index].bm;
		q.wk=p->wk^movelist[index].wk;
		q.wm=p->wm^movelist[index].wm;


		if(i==0) 
			bestindex=index;
			
		/* inline material count */
		/*add=0;*/
		if(color==BLACK)
			{
			if(stmcapture)
				{
				wm-=recbitcount(movelist[index].wm);
				wk-=recbitcount(movelist[index].wk);
				}
			if(movelist[index].bk && movelist[index].bm) //vtune: use & instead of && but how??
				{bk++;bm--;/*if(d<=FRAC) add=2*FRAC;*/}
			}
		else
			{
			if(stmcapture)
				{
				bm-=recbitcount(movelist[index].bm);
				bk-=recbitcount(movelist[index].bk);
				}
			if(movelist[index].wk && movelist[index].wm)
				{wk++;wm--;}
			}
		/* end material count */
		/*the above is equivalent to: countmaterial();*/

		realdepth++;
	
#ifdef REPCHECK
		Ghistory[realdepth+HISTORYOFFSET]=q;
#endif
		/*update the hash key*/
		updatehashkey(&movelist[index]);  // vtune: pass pointer to the structure, not structure itself!
		/********************recursion********************/
		value=-mpc(&q,d-FRAC,color^CC,-beta,-localalpha, &Lkiller);
		/*************************************************/

		/************** undo the move ***********/
		realdepth--;
		/* restore the hash key*/
		Gkey=Lkey;
		Glock=Llock;
		/* restore the old material balance */
		bm=l_bm;bk=l_bk;wm=l_wm;wk=l_wk;
		/************** end undo move ***********/
		
		/* update best value so far */
		maxvalue=max(value,maxvalue);
		/* and set alpha and beta bounds */
		if(maxvalue>=localbeta)
			{
			bestindex=index;
			break;
			}
		if(maxvalue>localalpha) 
			{
			localalpha=maxvalue;
			bestindex=index;
			}
		} /* end main recursive loop of forallmoves */

	/* save the position in the hashtable */
	/* we can/should restore the depth with which negamax was originally entered */
	/* since this is the depth with which it would have been compared */
#ifdef STOREEXACTDEPTH
	d = originaldepth;
#endif
	hashstore(p,maxvalue,alpha,beta,d,&movelist[bestindex],color,bestindex); // vtune: use pointer for "best"

#ifdef MOKILLER
	/* set the killer move */
	/* maybe change this: only if cutoff move or only if no capture?*/
	if(color==BLACK)
		*protokiller=movelist[bestindex].bm|movelist[bestindex].bk;
	else
		*protokiller=movelist[bestindex].wm|movelist[bestindex].wk;
#endif

	return maxvalue;
	}


int mpcnegamax(struct pos *p, int d, int color, int alpha, int beta, int32 *protokiller)
	{
	int value, valuetype;
	int32 forcefirst=MAXMOVES-1;
	int localalpha=alpha, localbeta=beta, n, localeval=MATE;
	int32 Lkey,Llock;
	int l_bm,l_bk,l_wm,l_wk, maxvalue=-MATE;
	int delta=0;
#ifdef ETC
	int bestETCvalue;
#endif
	int i,j;
	int index, bestmovevalue;
	int bestindex;
	int32 Lkiller=0;
	int isrootnode=0;
	struct move movelist[MAXMOVES];
	int values[MAXMOVES];
	struct pos q;
	int stmcapture = 0, sntmcapture = 0;
	int dbresult;
	int gotdbvalue=0;
	int isdbpos = 0;
	int cl=0; // conditional lookup - predefined as no. if CL && CLDEPTH>d then it's 1.
	int dummy;

#ifdef STOREEXACTDEPTH
	int originaldepth = d;
#endif
	// time check: abort search if we exceed the time given by aborttime
#ifdef CHECKTIME
	if((cake_nodes & 0xFFFF)==0)
		{
		if(searchmode==0)
			if( (clock()-start)/CLK_TCK>(aborttime)) (*play=1);
		}
#endif

	cake_nodes++;

	// check material:
	if(p->bm+p->bk==0)
		return color==BLACK?(-MATE+realdepth):(MATE-realdepth);
	if(p->wm+p->wk==0)
		return color==WHITE?(-MATE+realdepth):(MATE+realdepth);

	// return if calculation interrupt is requested
	if(*play) return 0;

	// stop search if maximal search depth is reached 
	if(realdepth>MAXDEPTH) 
		{
		leafs++;leafdepth+=realdepth;
		return evaluation(p,color,alpha,beta,&delta);
		}
	
	// search the current position in the hashtable 
	// only if there is still search depth left! 
	// should test this without d>0 !?
	if(d>0)
		{
		if(hashlookup(&value,&valuetype,d,&forcefirst,color,&dummy))
			{
			// return value 1: the position is in the hashtable and the depth
			// is sufficient. it's value and valuetype are used to shift bounds
			// or even cutoff 
			if(valuetype==EXACT)
				return value;
			if(valuetype==LOWER)
				{
				if(value>=beta) return value;
				if(value>localalpha) localalpha=value;
				}
			if(valuetype==UPPER)
				{
				if(value<=alpha) return value;
				if(value<localbeta) localbeta=value;
				}
			}
		}
#ifdef REPCHECK
	// check for repetitions 
	if(bk && wk)
		{
		for(i=realdepth+HISTORYOFFSET-2;i>=0;i-=2)
			{
			if((p->bm^Ghistory[i].bm)) break; // stop repetition search if move with a man is detected 
			if((p->wm^Ghistory[i].wm)) break; 
			if((p->bm==Ghistory[i].bm) && (p->bk==Ghistory[i].bk) && (p->wm==Ghistory[i].wm) && (p->wk==Ghistory[i].wk))
				return 0;                     // same position detected! 
			}
		}
#endif
	
	// get info on captures: can side to move or side not to move capture now?
	stmcapture = testcapture(p,color);
	// this should be ok: 
	if(!stmcapture)
		sntmcapture = testcapture(p,color^CC);

	if(stmcapture)
		n=makecapturelist(p,movelist,values,color,forcefirst);
    else
		n=0;


	//--------------------------------------------//
	// check for database use                     // 
	// conditions: -> #of men < maxNdb            //
	//			   -> no capture for either side  //
	//                else result is incorrect    //
	//--------------------------------------------//
#ifdef USEDB
	if(bm+bk+wm+wk<=MAXPIECES && max(bm+bk,wm+wk)<=MAXPIECE)
		{
		isdbpos = 1;
		if(!(stmcapture|sntmcapture))
			{
			// this position can be looked up!
			dblookups++;

#ifdef CL
			if(d<CLDEPTH*FRAC)
				cl=1;
#endif

			if(color==BLACK)
				dbresult = dblookup(p,DB_BLACK,cl);
			else
				dbresult = dblookup(p,DB_WHITE,cl);

			// statistics

			if(dbresult == DB_NOT_LOOKED_UP)
				dbfail++;
			else if(dbresult != DB_UNKNOWN)
				dbsuccess++;			

			if(dbresult == DB_DRAW)
				return 0;
			if(dbresult == DB_WIN)
				{
				value = dbwineval(p,color);
				if(value>=localbeta)
					return value;
				}
			if(dbresult == DB_LOSS)
				{
				value = dblosseval(p,color);
				if(value<=localalpha)
					return value;
				}
			}
		}
#endif // USEDB

// now, check return condition - never evaluate with a capture on board!
// when there is no capture it's easy:
	if(d<=0 && !stmcapture)
		{
		// normally, we'd be stopping here.
		// but if the side to move "has something", we shouldn't do this
		if(!sntmcapture)
			{
			if(realdepth>maxdepth) 
				maxdepth=realdepth;
			// no capture in this position - return
			return evaluation(p,color,localalpha,localbeta,&delta);
			}
		else
			{
			// returning a value with sntmcapture is risky, but probably good.
			// however, if this position is in the database, we don't want to 
			// do this.
			if(!isdbpos)
				{
				value = evaluation(p,color,localalpha,localbeta,&delta);
				// side not to move has a capture. what does this mean? it means that
				// possibly the value of this position will drop dramatically. therefore
				// this should be done asymmetrically: if value is already low, don't worry
				// too much. but if value is high, think about it again! value > localbeta+X*QLEVEL maybe?
				// QLEVEL:200, QLEVEL2:60. 
				// this takes care of this, i hope. at about 5% price opening & midgame, 2% endgame.
				// compared to 60/60
				if(value > localbeta+QLEVEL || value <localalpha-QLEVEL2)
					return value;
				}
			}
		}
		
		// if there is a capture for the side to move:
//	if(!stmcapture && sntmcapture && bm+bk+wm+wk>maxNdb)
//		return evaluation(p,color,localalpha,localbeta);	


// we could not return. prepare next iteration

	if(!n)
		n=makemovelist(p,movelist,values,color,forcefirst,*protokiller);
	if(n==0)
		return -MATE+realdepth;


/* check for single move and extend appropriately */
	if(n==1)
		d+=SINGLEEXTEND;

	/* save old hashkey and old material balance*/
	Lkey=Gkey;
	Llock=Glock;
	l_bm=bm;l_bk=bk;l_wm=wm;l_wk=wk;

	/* for all moves: domove, update hashkey&material, recursion, restore
		material balance and hashkey, undomove, do alphabetatest */
	/* set best move in case of only fail-lows */
	
#ifdef ETC
	if(d>ETCDEPTH)
		{
		bestETCvalue=ETClookup(movelist,d,n,color);
		if(bestETCvalue>=localbeta) 
			return bestETCvalue;
		if(bestETCvalue>localalpha) 
			localalpha=bestETCvalue;
		}
#endif

/*************** main "for all moves do" loop starts here ! *******************/
/* up to here it was just lots of other stuff - now we are ready for the work*/

	for(i=0;i<n;i++)
		{
		/* for all moves: domove, count material, recursive call, undo move, check condition */
		
		/* movelist contains moves, movelist.info contains their values. 
			have to find the best move! 
			we do this by linear search through the movelist[] array */
		
		index=0;
		bestmovevalue=-1;
	
		for(j=0;j<n;j++)
			{
			if(values[j]>bestmovevalue)
				{
				bestmovevalue=values[j];
				index=j;
				}
			}
		/* movelist[index] now holds the best move */
		/* set values[index] to -1, that means that this move will no longer be considered */
		values[index]=-1;
		
		/* domove */
		q.bk=p->bk^movelist[index].bk;
		q.bm=p->bm^movelist[index].bm;
		q.wk=p->wk^movelist[index].wk;
		q.wm=p->wm^movelist[index].wm;


		if(i==0) 
			bestindex=index;
			
		/* inline material count */
		/*add=0;*/
		if(color==BLACK)
			{
			if(stmcapture)
				{
				wm-=recbitcount(movelist[index].wm);
				wk-=recbitcount(movelist[index].wk);
				}
			if(movelist[index].bk && movelist[index].bm) //vtune: use & instead of && but how??
				{bk++;bm--;/*if(d<=FRAC) add=2*FRAC;*/}
			}
		else
			{
			if(stmcapture)
				{
				bm-=recbitcount(movelist[index].bm);
				bk-=recbitcount(movelist[index].bk);
				}
			if(movelist[index].wk && movelist[index].wm)
				{wk++;wm--;}
			}
		/* end material count */
		/*the above is equivalent to: countmaterial();*/

		realdepth++;
	
#ifdef REPCHECK
		Ghistory[realdepth+HISTORYOFFSET]=q;
#endif
		/*update the hash key*/
		updatehashkey(&movelist[index]);  // vtune: pass pointer to the structure, not structure itself!
		/********************recursion********************/
		value=-mpcnegamax(&q,d-FRAC,color^CC,-beta,-localalpha, &Lkiller);
		/*************************************************/

		/************** undo the move ***********/
		realdepth--;
		/* restore the hash key*/
		Gkey=Lkey;
		Glock=Llock;
		/* restore the old material balance */
		bm=l_bm;bk=l_bk;wm=l_wm;wk=l_wk;
		/************** end undo move ***********/
		
		/* update best value so far */
		maxvalue=max(value,maxvalue);
		/* and set alpha and beta bounds */
		if(maxvalue>=localbeta)
			{
			bestindex=index;
			break;
			}
		if(maxvalue>localalpha) 
			{
			localalpha=maxvalue;
			bestindex=index;
			}
		} /* end main recursive loop of forallmoves */

	/* save the position in the hashtable */
	/* we can/should restore the depth with which negamax was originally entered */
	/* since this is the depth with which it would have been compared */
#ifdef STOREEXACTDEPTH
	d = originaldepth;
#endif
	hashstore(p,maxvalue,alpha,beta,d,&movelist[bestindex],color,bestindex); // vtune: use pointer for "best"

#ifdef MOKILLER
	/* set the killer move */
	/* maybe change this: only if cutoff move or only if no capture?*/
	if(color==BLACK)
		*protokiller=movelist[bestindex].bm|movelist[bestindex].bk;
	else
		*protokiller=movelist[bestindex].wm|movelist[bestindex].wk;
#endif

	return maxvalue;
	}

#endif
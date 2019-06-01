// switches.h: definitions which influence the way cake++ searches 

#pragma warning( once : 4996 )
#pragma warning( once : 4133 )  // once instead of disable is maybe better

#ifdef _WIN64 
//#define VERSION "1.86 RC0 (5500k-log)"
//#define VERSION "1.86 RC2 (8279k)"
#define VERSION "1.86 RC2 QS (8279k)"
//#define VERSION "1.85 original (x64)"
#else
#define VERSION "1.85"
#endif

// two new flags to test logging and rep detection later on
#define NEWREPDETECTION
//#define NEWLOG

//#define FIXEDDEPTH 3
#undef THREADSAFEHT				// locks access to hashlookup and hashstore if set.
#include "consts.h"

#define SAFE

#undef FASTUPDATE

#define QSEARCH
#define MAXQS 1
#define QSEARCHLEVEL 150
#undef QS_SQUEEZE

// compile options for cake
// add a bookgen function?
#define BOOKGEN

// enable position learning?
#undef LEARNSAVE	// save learned positions
#undef LEARNUSE	// use learned positions

//#define HEURISTICDBSAVE // save positions that were evaluated with 9-12 pieces
#undef HEURISTICDBSAVE

#define COARSEGRAINING      		// use evaluation coarse graining 
#define IMMEDIATERETURNONFORCED		// if defined, cake will not think about forced moves
#define BOOK						// use opening book
#define CHECKTIME					// check if time is > than breaktime and abort if yes
#define USEDB						// use the endgame database 

#define NOPRUNE						// new: if this is defined, the eval will be able 
									// to tell the search not to prune by expanding the pruning window
#define NOPRUNEWINDOW 50			// this is how much delta is set to in the noprune case

#define STOREEXACTDEPTH
#undef  EVALOFF						// return 0 in eval ca
#undef EVALMATERIALONLY				// turn off all positional evaluation 
#define REPCHECK					// check for repetitions 
#define MOVEORDERING 				// turns on all move ordering 
									// if not set: overrules the three switches below ! 
#define MOHASH						// use a move from hashtable or killer move 
#define MOKILLER 					// use killer if no move from hashtable 
#define MOHISTORY					// use history table 
#define MOSTATIC					// use static move ordering
#define MOTESTCAPT					// order moves to back of list which lead to a capture for opponent
#define MOTESTCAPT2					// order moves to front of list which lead to threat of capture for sidetomove
#define GRAINSIZE 2					// 2 with this grain size 

#define QLEVEL 200	// 200			// qlevel gives the value an eval must be out of the window to be used
#define QLEVEL2 60	// 60			// if the side not to move has a capture
						

#define NEWTRUNCATION				// use new, eval-based truncation?
#define MAXTRUNC 3*FRAC				//2*FRAC	// maximum truncation per ply 
#define TRUNCLEVELHARD 120			//100			// TRUNCLEVEL was 30 for 1.42
#define TRUNCLEVELSOFT 30			// new double truncation scheme!
#define TRUNCDIVHARD 16 //16				// divider to get ply: if outside window by 80, divide by 16 to get the number of ply to truncate
#define TRUNCDIVSOFT 32 //32				// positional cutoff: example: for 32 outside, 0.25 ply, for 64: 0.5ply. very gentle

#define ETC							// use enhanced transposition cutoffs 
#define ETCDEPTH FRAC*2				//maybe 3 is better.	if depth>etcdepth do ETC 

#define MAXDEPTH 99					// maximal number of plies cake++ can search 
#define LAZYEVALWINDOW 400			// if materialeval is more than this outside of alpha-beta window, 
#define FINEEVALWINDOW 200			// do only materialeval; LAZY for only material, FINE for material+selftrapeval

#define HISTORYOFFSET 10		
#define ASPIRATIONWINDOW 60			// aspiration window size for windowed search
#define SINGLEEXTEND FRAC/2			// extension for forced move 
#define CLDEPTH 5					// if depth < CLDEPTH // 5

#define HASHSIZE 0x00800000			// 64 MB default hashtable size  (was 64 if you something has changed somewhere...)

#define HASHITER 2					// 2 probes in the hashtable

#define ALWAYSSTORE					// overwrite hashentries even if the new one has less depth? 
									// if yes, define this
									// book settings, also for book builder 
#define TMPBOOKSIZE (1048576L<<1)	// size of the temporary book 
#define HASHSIZEBOOK 400000			// size of the final book 
#define BUCKETSIZEBOOK 64			// bucket size of book hashtable 
#define BOOKMINDEPTH 0				// minimal depth which a book move must have for cake++ to use it  

#define MTD
#define EXTENDPV					// extend nodes marked as ispv
#define EXTENDPVDEPTH 1				// by this amount

#undef SPA							// use SPA - if this is not defined, no SPA-specific code is
									// generated
#define SPASIZE 33554432			// 32 million entries in the SPA table - need 6 bytes per entry.
									// => 192 MB SPA table size
#undef SPA_CUT						// use SPA table to cutoff at interior nodes
#define SPA_CUT_DEPTH 10*FRAC		// depth below which to use SPA cuts
#define SPAITER 4

#undef BOOKHT						// use book hashtable - if this is not defined, no bookht code
									// is generated.
#define BOOKHTSIZE 2*1024*1024		// size of book hashtable
#define BOOKHTMINDEPTH 3*FRAC		// don't look up if remaining depth smaller than this
#define BOOKHTMAXDEPTH 100*FRAC		// don't look up if remaining depth larger than this - 19*FRAC didn't work out as expected

#undef FULLLOG						// produce a very explicit logfile
#ifdef FULLLOG
#define MAXPV 40
#else
#define MAXPV 8						// max length of PV printout in ply
#endif

#define MARKPV						// mark pv in hashtable and don't prune


#define IITERD						// use internal iterative deepening
#define IIDDEPTH 6*FRAC				// only use it when the remaining depth is > this
#define IIDREDUCE 4*FRAC			// and reduce by this amount for IID search 

#undef LATEMOVEREDUCTION			// use late move reduction
#undef LATEMOVEREDUCTIONROOT
#define LATEMOVEMINDEPTH 2*FRAC		// minimal remaining search depth to use LMR
#define LATEMOVEDEPTH 2				// reduce late moves by 1/4 a ply times this
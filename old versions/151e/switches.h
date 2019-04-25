//
//		Cake - a checkers engine			
//															
//		Copyright (C) 1999 - 2002 by Martin Fierz		
//															
//		contact: checkers@fierz.ch				
//

// switches.h: definitions which influence the way cake++ searches 

#define VERSION "1.51e''"
#include "consts.h"

// compile options for cake
// add a bookgen function?
#define BOOKGEN

// which database?
#define EIGHT
//#define SIX

#define COARSEGRAINING      		// use evaluation coarse graining 
#define IMMEDIATERETURNONFORCED		// if defined, cake will not think about forced moves
#define BOOK						// use opening book
#define CHECKTIME					// check if time is > than breaktime and abort if yes
#define USEDB						// use the endgame database 
#define MYDB						// use cake db instead of chinook db

#define NOPRUNE						// new: if this is defined, the eval will be able 
									// to tell the search not to prune by expanding the pruning window
#define NOPRUNEWINDOW 50			// this is how much delta is set to in the noprune case

//#define NEVERPRUNE				// if this is defined, all pruning is turned off. for testing
#define STOREEXACTDEPTH
#undef  EVALOFF						// return 0 in eval 
#undef EVALMATERIALONLY				// turn off all positional evaluation 
#define REPCHECK					// check for repetitions 
#define MOVEORDERING 				// turns on all move ordering 
									// if not set: overrules the three switches below ! 
#define MOHASH						// use a move from hashtable or killer move 
#define MOKILLER 					// use killer if no move from hashtable 
#define MOHISTORY					// use history table 
#define MOSTATIC					// use static move ordering
#define MOTESTCAPT					// a very expensive part of static move ordering 
#define GRAINSIZE 2					// 2 with this grain size 
#define EXTENDALL					// if extendall is defined cake++ will extend if a capture
									// for the side not to move is possible 
									// extension for capture for the side to move is always done! 
#define QLEVEL 200	// 200			// qlevel gives the value an eval must be out of the window to be used
#define QLEVEL2 60	// 60			// if the side not to move has a capture
						

#define NEWTRUNCATION				// use new, eval-based truncation?
#define MAXTRUNC 2*FRAC				// maximum truncation per ply 
#define TRUNCLEVELHARD 100			// TRUNCLEVEL was 30 for 1.42
#define TRUNCLEVELSOFT 30			// new double truncation scheme!
#define TRUNCDIVHARD 16				// divider to get ply: if outside window by 80, divide by 16 to get the number of ply to truncate
#define TRUNCDIVSOFT 32				// positional cutoff: example: for 32 outside, 0.25 ply, for 64: 0.5ply. very gentle
#define DOEXTENSIONS				// use the truncation 

#define ETC							// use enhanced transposition cutoffs 
#define ETCDEPTH FRAC*2				//maybe 3 is better.	if depth>etcdepth do ETC 
#define CHECKCAPTURESINLINE

#define MAXDEPTH 99					// maximal number of plies cake++ can search 
#define FINEEVALWINDOW 400			// if materialeval is more than this outside of alpha-beta window, 
									// do only materialeval 
#define HISTORYOFFSET 10		
#define ASPIRATIONWINDOW 10			// aspiration window size for windowed search
#define SINGLEEXTEND FRAC/2			// extension for forced move 
#define CL							// conditional db lookup is done
#define CLDEPTH 5					// if depth < CLDEPTH

#define HASHSIZE 0x00400000			// 32 MB default hashtable size

#define HASHITER 2					// 2 probes in the hashtable

#define ALWAYSSTORE					// overwrite hashentries even if the new one has less depth? 
									// if yes, define this

									// book settings, also for book builder 
#define TMPBOOKSIZE (1048576L<<1)	// size of the temporary book 
#define HASHSIZEBOOK 400000			// size of the final book 
#define BUCKETSIZEBOOK 64			// bucket size of book hashtable 
#define BOOKMINDEPTH 0				// minimal depth which a book move must have for cake++ to use it  

#define MTD

//#define MPC							// use multi-probcut instead of my own truncation scheme.

#undef PV

//#define FULLLOG						// produce a very explicit logfile

//#define CONFIRMPV					// do an additional unnecessary search to return a pv in MTD

//#define PVS
#define MAXPV 24
#define MARKPV						// mark pv in hashtable and don't prune
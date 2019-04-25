/* switches.h: definitions which influence the way cake++ searches */

#include "consts.h"

#undef OPLIB
/* compile options for cake++*/

#define SYS_WINDOWS 1
#define SYS_MACOS 2
#define SYS_UNIX 3

#define SYSTEM SYS_WINDOWS


/* undef the two #defines below for OPLIB */
#ifndef OPLIB
	#define COARSEGRAINING      		/* use evaluation coarse graining */
	#define IMMEDIATERETURNONFORCED  /* if defined, cake++ will not think about forced moves*/
#endif

#define USEDB					/* use the endgame database */

#undef  EVALOFF              /* return 0 in eval */
#undef  EVALMATERIALONLY     /* turn off all positional evaluation */
#define REPCHECK            /* check for repetitions */
/* repcheck reduces nodes/sec by 4% overall in the testcake test */
#define MOVEORDERING 		 /* turns on all move ordering */
									 /* if not set: overrules the three switches below ! */
//#define MOSTATIC  				/* use static move ordering */
#define MOTESTCAPT          /* a very expensive part of static move ordering */
#define MOHASH              /* use a move from hashtable or killer move */
#define MOKILLER 				/* use killer if no move from hashtable */
#define MOHISTORY           /* use history table */
#define DOCAPTORDERING
//#define DOORDERING	    /* do the bubble sort: just to test how slow this really is */
//#define  SORTALL              /* sort the entire movelist in the move generator or not? */
#define GRAINSIZE 2         /* 2 with this grain size */
#define EXTENDALL				 /* if extendall is defined cake++ will extend if a capture
                               for the side not to move is possible */
                            /* extension for capture for the side to move is fix! */
#define QLEVEL 60  /*70*/
#define DOEXTENSIONS        /* use the truncation */
#define TRUNCATIONDEPTH 3*FRAC/2  /* how much do i truncate... */
                            /* this will be AUTOMATICALLY overruled in endgames */
                            /* where truncationdepth is set to 0! */
#define TRUNCATEVALUE 100   /* 110...if value is more than this out of window */

#undef TRUNCATIONTEST      /* use full eval to test truncation condition */
#define TRUNCATETESTVALUE 110 /* use this value as window extension for full test */
#define MTDF
#define ETC						/* use enhanced transposition cutoffs */
#define ETCDEPTH FRAC*2			/* if depth>etcdepth do ETC */
//#define PVS /* use principal variation search */
#undef SINGULAREXTENSIONS /* check for singular extensions */
#define SEDEPTH 20 /* how much do we extend singular moves? */
#define SELEVEL 80 /* how much better must the singular move be?*/
#define CHECKCAPTURESINLINE
/* some stuff for search */
#define MAXDEPTH 99
#define FINEEVALWINDOW 150
#define HISTORYOFFSET 10
#define ASPIRATIONWINDOW 10
#define HASH
#define SINGLEEXTEND FRAC/2 /* 5 looks good here */

/* hashtable settings */
/*0x00010000=65535*/   /*1.5MB for the deep hashtable*/
/*=0x00100000 = 12MB*/   /*6MB for the shallow hashtable*/
#define HASHSIZEDEEP    0x00020000
#define HASHSIZESHALLOW 0x00080000
#define HASHMASKDEEP    0x0001FFFF
#define HASHMASKSHALLOW 0x0007FFFF
/* ciao tom, da kannst du eventuell eine groessere hashtable machen */
/* zB so - braucht dann halt 12 MB. ist aber besser mehr hash dafuer
weniger buffers zu haben in db.ini, wenn ich mich recht entsinne. hab
dir daher auch ein db.ini mitgeschickt */
/* #define HASHSIZESHALLOW 0x00100000
   #define HASHMASKSHALLOW 0x000FFFFF
*/

#define DEEPLEVEL 10
/* positions with realdepth < deeplevel
													  are stored in deep */
#define HASHITER 2

#undef ANALYSISMODULE /* compiles a version of cake++ which analyses all moves */

#undef VERBOSE /* print lots of output */

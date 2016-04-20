
/*
 * This library provides an API into several checkers endgame databases.
 * These include:
 *		- Cake 8 piece wld database built by Martin Fierz.
 *		- Chinook 8 piece wld database built by Jonathan Schaeffer.
 *		- KingsRow 10 piece wld database built by Ed Gilbert
 *		- KingsRow 10 piece mtc.
 *		- KingsRow Italian 7 piece wld.
 *		- KingsRow Italian 7 piece mtc.
 *		- Chinook Italian 6 piece wld (note: as of 4/9/2005 this db has errors).
 *
 * The purpose in creating this library is to make it easier for the various
 * checkers programs to use any of the publicly available databases, so that
 * users will not need to fill up their hard drives with multiple versions
 * of the same basic information.  Another purpose is to make it easier for
 * new checkers programmers to use these databases with their programs.  
 * The indexing and caching algorithms needed for an efficient 
 * database driver are complex, and creating this code is a lot of work.
 * Using this library hides the complexity of these databases from 
 * the checkers programs.
 *
 * The API has functions to
 *		- identify a database and return its attributes
 *		- open a database for lookups
 *		- lookup values in an open database
 *		- get statistics on database lookups, cache hits, misses, etc.
 *		- perform a crc check on each database file
 *		- close a database
 *
 * To use the library, include the header file egdb.h in your application source
 * code and link with the lib file egdb.lib.  The egdb.dll file must be somewhere 
 * in the search path for dynamic libraries when the application runs.
 *
 * To lookup values in a database, the database must first be opened using
 * egdb_open().  This returns a handle to an open database.  The handle is
 * a pointer to a structure of db access function pointers.  These
 * access functions are then used for all subsequent communication with 
 * the driver.  The handle also holds all state data that is used by each 
 * instance of an open driver, thus multiple handles can be opened from a
 * a single process without conflicting with one another.  These drivers
 * are safe for multi-threading.
 * 
 * The open function should be tested for a NULL return pointer, which means
 * that some kind of error occurred.  The likely reasons for errors are either
 * the wrong directory path was given for the database files, or the driver
 * was unable to allocate all the memory that it needed from the heap.  If a
 * NULL pointer is returned by the open function, an error message will be passed
 * back to the application through the callback message function (the fourth
 * argument passed to egdb_open).
 *
 * The first argument to the open function is an enum which tells the driver
 * which type of position bitmaps will be used for lookups.  The driver will
 * tailor the lookup function pointer for the particular bitmap type that is
 * requested.  The driver can accept positions in either Cake or KingsRow
 * format.  These structure and bitmap conventions are defined below.
 *
 * The second argument to the open function is the maximum number of pieces for
 * which the database is being opened for lookups.  Normally this will be 8 or
 * 10, but a smaller number of pieces can be given.  The driver will use
 * less ram and initialize more quickly when a smaller than maximum number of
 * pieces is given.  The egdb_open function will error if a pieces argument is
 * given that is greater than the capability of the database.  The attributes of
 * a database can be determined using the function egdb_find_database before
 * calling egdb_open.
 *
 * The third argument to the open function is the number of megabytes (2^20 bytes)
 * of heap memory that the driver will use.  The driver will use some minimum
 * amount of memory even if you give a value which is lower than this minimum.
 * For a database that is being opened to use 8 pieces this minimum is between
 * 35 and 40mb, which includes 10mb of cache buffers.  Any memory that you give
 * above this mimimum is used exclusively to create more cache buffers.  Cache
 * buffers are allocated in 4k blocks using VirtualAlloc, and are managed using
 * a 'least recently used' replacement strategy.
 * If the memory argument is large enough the driver will allocate permanent
 * buffers for some of the smaller subsets of the database.  In the statistics
 * counters these are called 'autoload' buffers.  If the maximum pieces requested
 * is 8 and the memory size argument is greater than 120mb then the 2 - 6 piece
 * slices are autoloaded, and if it is greater than 900mb then the 7 piece slices
 * are autoloaded.
 *
 * The last argument to the open function is a pointer to a function that will 
 * receive status and error messages from the driver.  The driver normally
 * produces around 30 lines of status messages when it starts up, with 
 * information about files opened and memory used.  If any errors occur during
 * the open call, the details will be reported through the message function.
 * A message function that writes messages to stdout can be provided
 * like this:
 *
 *        void msg_fn(char *msg)
 *        {
 *             printf(msg);
 *        }
 *
 * which can be passed to a driver like this:
 *
 *        driver = egdb_open(EGDB_NORMAL, 8, 512, "\\db\\cake", msg_fn);
 *
 * This callback function is also used to report error messages during
 * lookups.  In KingsRow I write these messages to a log file.
 *
 * The API provides a couple of functions for gathering statistics on db lookups.
 * (*driver->get_stats)() returns a pointer to a structure of various counts
 * that have accumulated since the counts were last reset.
 * (*driver->reset_stats)() resets all the counts to 0.
 *
 * The (*driver->lookup)() function returns the value of a position in an open
 * database.  It takes 4 arguments -- a driver handle, position, color, and a
 * conditional lookup boolean.  If the conditional lookup argument is true,
 * then the driver will only get the value of the
 * position if the position is already cached in ram somewhere, otherwise
 * EGDB_NOT_IN_CACHE will be returned.  If the conditional lookup
 * argument if false, the driver will always attempt to get a value for the
 * position even if it has to read a disk file to get it.
 *
 * For the WLD databases, the values returned by lookup are defined by the
 * macros EGDB_UNKNOWN, EGDB_WIN, EGDB_LOSS, EGDB_DRAW, and EGDB_NOT_IN_CACHE.
 * The lookup function will return the correct EGDB_WIN or EGDB_LOSS if it is
 * given a position where one side has no pieces.
 *
 * For the MTC databases, the values returned by lookup are either the number
 * of moves to a conversion move, or the value MTC_LESS_THAN_THRESHOLD for 
 * positions which are close to a conversion move.  The KingsRow mtc databases
 * do not store positions which are closer than MTC_THRESHOLD moves to a 
 * conversion move.  The KingsRow mtc db will only return even mtc values,
 * because the database represents the mtc value internally by distance/2.  The
 * true value is either the value returned, or value + 1.  An application
 * program can infer the true even or odd value of a position by looking up
 * the mtc value of the position's successors.  If the largest successor mtc
 * value is the same as the position's, then the position's mtc true value is
 * 1 greater than the returned value.
 *
 * The WLD databases will not return valid values for positions where either
 * the side to move has a capture or where the opposite side would have a capture
 * if it was that side's turn to move.  There is no way to tell this from the
 * return value, which will normally be one of the win, loss, or draw values
 * from a lookup on a capture position.  Your application must test for these
 * captures and avoid calling lookup for any positions where they are present.
 * The MTC databases do not have this restriction.
 *
 * The Cake database does not have data for positions where one side has more than
 * 4 pieces.  If you give the lookup function a position where one side has more 
 * pieces than it knows about it returns the value EGDB_UNKNOWN.  The Chinook
 * database has all positions for up to 6 pieces, but the 7 piece and 8 piece
 * slices do not have data for more than 4 pieces on a side.  The KingsRow databases
 * have data for positions with up to 5 pieces on a side.
 *
 * The driver has tables of good crc values for all db files.
 * The (*driver->verify)() function performs a crc check on each index and
 * data file in the open database.  It returns 0 if all crcs compared ok. 
 * If any file fails the check, the verify call returns a non-zero value, and error
 * messages are sent through the callback msg_fn that was registered when the
 * driver was opened.
 *
 * The (*driver->close)() function frees all heap memory used by the driver.
 * If the application needs to change anything about an egdb driver after it
 * has been opened, such as number of pieces or megabytes of ram to use,
 * it must be closed and then opened again with the new parameters.  Close
 * returns 0 on success, non-zero if there are any errors.
 *
 */

/* Prevent name mangling of exported dll publics. */
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

/* Dont define EGDB_EXPORTS when using this library.
 * Only define it if you are actually compiling the library source code.
 */
#ifdef EGDB_EXPORTS
#define EGDB_API EXTERNC __declspec(dllexport)
#else
#define EGDB_API EXTERNC __declspec(dllimport)
#endif


/* Color definitions. */
#define EGDB_BLACK 0
#define EGDB_WHITE 1

/* Values returned by handle->lookup(). */
#define EGDB_UNKNOWN 0			/* value not in the database. */
#define EGDB_WIN 1
#define EGDB_LOSS 2
#define EGDB_DRAW 3
#define EGDB_DRAW_OR_LOSS 4
#define EGDB_WIN_OR_DRAW 5

#define EGDB_NOT_IN_CACHE -1		/* conditional lookup and position not in cache. */

/* MTC macros. */
#define MTC_THRESHOLD 10
#define MTC_LESS_THAN_THRESHOLD 1
#define MTC_UNKNOWN 0


typedef enum {
	EGDB_KINGSROW_WLD = 0,
	EGDB_KINGSROW_MTC,
	EGDB_CAKE_WLD,
	EGDB_CHINOOK_WLD,
	EGDB_KINGSROW32_WLD,
	EGDB_KINGSROW32_MTC,
	EGDB_CHINOOK_ITALIAN_WLD,
	EGDB_KINGSROW32_ITALIAN_WLD,
	EGDB_KINGSROW32_ITALIAN_MTC,
} EGDB_TYPE;

typedef enum {
	EGDB_NORMAL = 0,
	EGDB_ROW_REVERSED
} EGDB_BITMAP_TYPE;

/* for database lookup stats. */
typedef struct {
	unsigned int lru_cache_hits;
	unsigned int lru_cache_loads;
	unsigned int autoload_hits;
	unsigned int db_requests;				/* total egdb requests. */
	unsigned int db_returns;				/* total egdb w/l/d returns. */
	unsigned int db_not_present_requests;	/* requests for positions not in the db */
} EGDB_STATS;

/* This is KingsRow's definition of a checkers position.
 * The bitmaps use bit0 for square 1, bit1 for square 2, ...,
 * For Italian checkers, the rows are reversed, so that bit 0 represents
 * Italian square 4, bit 1 represents square 3, bit 2 represents square 2, etc.
 */
typedef struct {
	unsigned int black;
	unsigned int white;
	unsigned int king;
} EGDB_NORMAL_BITMAP;

/* This is Cake's definition of a board position.
 * The bitmaps use bit3 for square 1, bit2, for square 2, ...
 * This is repeated on each row of squares, thus bit7 for square 5, 
 * bit6 for square 6, bit5 for square 7, ...
 * For Italian checkers, the rows are reversed, so that bit 0 represents
 * Italian square 1, bit 1 represents square 2, bit 2 represents square 3, etc.
 */
typedef struct {
	unsigned int black_man;
	unsigned int black_king;
	unsigned int white_man;
	unsigned int white_king;
} EGDB_ROW_REVERSED_BITMAP;

typedef union {
	EGDB_NORMAL_BITMAP normal;
	EGDB_ROW_REVERSED_BITMAP row_reversed;
} EGDB_BITMAP;

/* The driver handle type */
typedef struct egdb_driver {
	int (*lookup)(struct egdb_driver *handle, EGDB_BITMAP *position, int color, int cl);
	void (*reset_stats)(struct egdb_driver *handle);
	EGDB_STATS *(*get_stats)(struct egdb_driver *handle);
	int (*verify)(struct egdb_driver *handle);
	int (*close)(struct egdb_driver *handle);
	void *internal_data;
} EGDB_DRIVER;


/* Open an endgame database driver. */
EGDB_API EGDB_DRIVER *egdb_open(EGDB_BITMAP_TYPE bitmap_type,
								int pieces, 
								int cache_mb,
								char *directory,
								void (*msg_fn)(char *));

/*
 * Identify which type of database is present, and the maximum number of pieces
 * for which it has data.
 * The return value of the function is 0 if a database is found, and its
 * database type and piece info are written using the pointer arguments.
 * The return value is non-zero if no database is found.  In this case the values
 * for egdb_type and max_pieces are undefined on return.
 */
EGDB_API int egdb_identify(char *directory, EGDB_TYPE *egdb_type, int *max_pieces);

/*
 * The next 3 functions are not necessarily needed to use the databases.
 * They are useful for testing the drivers, for iterating through all
 * database positions to verify, collect statistics, etc.
 *
 * egdb_indextoposition returns the board position that corresponds to
 * an index number.
 * Each index number from 0 through (range - 1) corresponds to a unique
 * board position in a subset of game positions called a slice, where
 * range is the total number of positions in the slice.  A slice is a fixed
 * number of black men, black kings, white men, and white kings.
 */
EGDB_API void egdb_indextoposition(__int64 index, EGDB_NORMAL_BITMAP *pos,
								  int nbm, int nbk, int nwm, int nwk);

/*
 * Return the total number of possible board positions in a slice.
 */
EGDB_API __int64 egdb_index_range(int nbm, int nbk, int nwm, int nwk);

/*
 * Return true if the position has a capture move when it is color's
 * turn to move.  [Note: this function is for English checkers only, it
 * does not work with Italian checkers].
 */
EGDB_API int egdb_iscapture(EGDB_NORMAL_BITMAP *board, int color);

/*
 * These tables are initialized when the process is attached to the dll.
 * It is likely that a checkers program needs these tables, and having
 * a single copy for both the dll and the program saves memory and increases
 * the effectiveness of caching.
 *
 * egdb_reverse_nibble_table[n] returns a 16-bit word with every ith bit in
 * each nibble of n exchanged with bit (3-i).  This table is used to
 * convert between a normal bitmap and a row-reversed bitmap.
 *
 * egdb_reverse_int_table[n] returns a 16-bit word with every ith bit in
 * n exchanged with bit (15-i).  This table is used to create a color-reversed
 * image of a board position.
 *
 * egdb_bitcount_table[n] returns the number of bits that are set in n.
 */
EGDB_API extern unsigned short egdb_reverse_nibble_table[0x10000];
EGDB_API extern unsigned short egdb_reverse_int_table[0x10000];
EGDB_API extern char egdb_bitcount_table[0x10000];

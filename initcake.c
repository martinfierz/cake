#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winbase.h>
#include <assert.h>
#include "structs.h"
#include "consts.h"
#include "switches.h"
#include "cakepp.h"
#include "initcake.h"
#include "cake_misc.h"
#include "boolean.h"

#ifdef BOOK
HASHENTRY *loadbook(int *bookentries, int *bookmovenum, FILE *fp)
// loads the cake book file and returns a pointer to the book,
// sets bookentries to the number of entries in the opening book.
{
	FILE *Lfp;
	int32 i,j;
	char Lstr[256];
	static HASHENTRY *book;
	char dirname[256]; 
//	__int64 *x; 

	GetCurrentDirectory(256, dirname);

	Lfp = fopen("engines\\book.bin", "rb");
	//Lfp = fopen("D:\\book.bin", "rb");
	if(Lfp == NULL)
		{
		/* book file not found */
		book = NULL;
		sprintf(Lstr,"no opening book detected under %s\\engines\\book.bin\n", dirname);
		logtofile(fp, Lstr);
		return 0;	
		}

	printf("book found, loading");

	fscanf(Lfp,"%i",bookentries);
	printf("book claims to have %i book entries", *bookentries);
#ifdef WINMEM
	book = VirtualAlloc(0, bookentries*sizeof(struct bookhashentry), MEM_COMMIT, PAGE_READWRITE);
#else
	book = malloc((*bookentries)*sizeof(struct bookhashentry));
#endif

	if (book == NULL)
	{
		sprintf(Lstr,"malloc failure in initcake (book malloc failed)");
		logtofile(fp, Lstr);
		return NULL;
	}

	// hack on new laptop where file cannot be read otherwise!
	fseek(Lfp, -1, SEEK_CUR); 

	fread(book,sizeof(struct bookhashentry),(*bookentries),Lfp);
	fclose(Lfp); 

	sprintf(Lstr,"allocated %zi KB for book hashtable",sizeof(struct bookhashentry)*(*bookentries)/1024);
	logtofile(fp, Lstr);
	printf(Lstr); 
	sprintf(Lstr,"book hashtable with %i entries allocated\n",(*bookentries));
	logtofile(fp, Lstr);
	printf(Lstr); 

	//Lfp = fopen("d:\\booklog.txt", "w"); 
	

	j=0;
	for(i=0;i<(*bookentries);i++)
		{
		//x = &(book[i]); 
		//sprintf(Lstr, "index %i, entry %i, lock %i (%x), best %i, value %i, color %i, ispvnode %i, depth %i, valuetype %i [%llx]\n",
		//	i, j, book[i].lock, book[i].lock, book[i].best, book[i].value, book[i].color, book[i].ispvnode, book[i].depth, book[i].valuetype, *x); 
		//fprintf(Lfp, Lstr); 
		if(book[i].lock != 0)
			j++;
		}

	sprintf(Lstr,"%i moves in opening book\n",j);
	logtofile(fp, Lstr);
	//printf(Lstr); 

	*bookmovenum = j;

	fclose(Lfp);
	return book;
	}

#endif

HASHENTRY *inithashtable(int hashsize, FILE *logfile)
	{
	// allocate memory for the hashtable. 
	// align the hashtable on a 64-byte-boundary
	// terminate program if hashtable cannot be allocated.
	char Lstr[256];
	HASHENTRY *ptr;

#ifdef WINMEM
	hashtable = VirtualAlloc(0, (hashsize+HASHITER)*sizeof(HASHENTRY), MEM_COMMIT, PAGE_READWRITE);
#else
	ptr = malloc((hashsize+HASHITER)*sizeof(HASHENTRY)+64);
#endif

	if(ptr == NULL)
		{
		sprintf(Lstr,"malloc failure in initcake (hashtable memory allocation failed)");
		logtofile(logfile, Lstr);
		fclose(logfile); 
		exit(0);
		}
	else {
		sprintf(Lstr, "\nhashtable allocated with %zi bytes", (hashsize + HASHITER) * sizeof(HASHENTRY) + 64);
		logtofile(logfile, Lstr); 
	}

	return ptr;
}




int initxors(int *ptr)
{

	// constants for xoring to make hashcode 
	// the hash 'lock' has it's last two bits cleared to store the color 
	// like this in old code: hashxors[1][i][j]&=0xFFFFFFFC; 

	int32 xors[256]= {690208388, 1385187825, 3014056764, 610143516,
							3004321224, 520313469, 1574226940, 319654154,
							546984174, 306787062, 2907641074, 1808019639,
							4129460637, 2966067458, 746618565, 3001731532,
							3205813886, 2213500745, 1640878366, 1927683568,
							47704121, 303969140, 3567244452, 841151692,
							1221824373, 3854628651, 241127424, 2063930631,
							410030810, 2235459861, 2524122841, 3572976202,
							4195793681, 2046710491, 4098216920, 3311425853,
							1982714050, 1231714597, 2973461157, 3601402759,
							354261653, 2129242266, 1421843175, 2419599569,
							1582227760, 2682430946, 1698179654, 2054584514,
							3660148030, 884947366, 1829692268, 499892739,
							252970341, 3320261774, 4087909886, 4288729147,
							2272817997, 3686942273, 2138108071, 2858117139,
							3401481631, 4256714600, 2580402957, 2791723827,
							1582099459, 3023113898, 3391619032, 2032621890,
							3448538912, 2274708656, 2582068657, 2902999529,
							2682525855, 4266447137, 2406699840, 3396326863,
							1522427175, 2713609638, 842271020, 1455872925,
							2290169953, 4183085287, 227682589, 3681819898,
							2962577397, 2373245323, 2784393661, 3772866703,
							1533184650, 3850848827, 2227618737, 4000673539,
							273718216, 2331437470, 829214116, 2358776063,
							1472680025, 52985693, 877551676, 1118965035,
							3090117602, 3632583455, 383140641, 2552966190,
							3169896974, 2457120653, 1584021756, 392902321,
							3933347458, 3093223758, 2490637685, 1188754011,
							3407689449, 3534494219, 1079747379, 2917214933,
							3847937257, 1774533228, 3951532767, 3588662470,
							3280983594, 68223256, 476105248, 1163078103,
							3928419669, 737687480, 1548679839, 2240709040,
							3782006444, 4055624168, 2265726616, 112674780,
							1136364452, 154674460, 2532656440, 484159024,
							1081620220, 2659011036, 1145071312, 842768672,
							3783991256, 3111296328, 2580932992, 1405052884,
							769429092, 1644128652, 2644252272, 2219788916,
							743689160, 2651649236, 1501443532, 3486748716,
							3872988456, 2021754692, 3563163068, 993231388,
							1683279536, 4165364228, 372205416, 3495389300,
							2913870984, 625206588, 684592116, 2535950936,
							3385682132, 140591124, 287883660, 2542546928,
							3248218560, 2810626740, 3643003220, 3647775076,
							2633947504, 211530820, 2309587576, 1672554204,
							1658885648, 1065967344, 681647648, 1061226372,
							455631304, 1177946332, 3203591024, 1963363452,
							3502146236, 730659376, 1418080916, 199687412,
							2307539224, 4134697748, 4031003316, 197894416,
							1282663940, 2916079024, 2635353588, 2090648056,
							593803428, 1519210028, 1625207168, 1732355540,
							356646948, 76754864, 1133162364, 3550582220,
							720766892, 3033120872, 1291947796, 3366377480,
							1801982560, 1897351288, 2250873200, 1329051828,
							3749735532, 1130696968, 4159378872, 1849457652,
							2401721588, 849043776, 2804959760, 798911212,
							2168662312, 1201766728, 2665382304, 2588134860,
							3216841764, 2304751944, 3856532120, 3054824632,
							1367244060, 29249640, 2199459096, 2348355292,
							1030591536, 3047461076, 2155217480, 568032600,
							2373036172, 2952445872, 3856404568, 2174779228,
							1232339540, 4155057696, 3107293288, 2989645064,
							2002643892, 2476215636, 73970336, 3161939852,
							555002324, 2377038704, 3479033388, 2824457228,
							1317045464, 3990190752, 570178612, 2109730480};

	// copy xor values 
	memcpy(ptr, xors, 256*sizeof(int));

	return 1;
}



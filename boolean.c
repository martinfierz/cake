#include <Windows.h>
#include <winbase.h>
#include <intrin.h>
#include "structs.h"
#include "boolean.h"
#include <assert.h>


//static char bitsinword[65536];

/*
void initbool()
{
	int i; 
	// initialize bitsinword, the number of bits in a word
	for(i=0;i<65536;i++)
		bitsinword[i]=recbitcount((int32)i);
}*/



// table-lookup bitcount - newer CPUs are going to have a popcount instruction soon, would be
// much more efficient.



/*int bitcount(int32 n)
	// returns the number of bits set in the 32-bit integer n 
	{
	return __popcnt(n);

	//return (bitsinword[n&0x0000FFFF]+bitsinword[(n>>16)&0x0000FFFF]);
	}*/


/*
int recbitcount(int32 n)
	// counts & returns the number of bits which are set in a 32-bit integer
	//	slower than a table-based bitcount if many bits are
	//	set. used to make the table for the table-based bitcount on initialization
	{
	int r=0;
	while(n)
		{
		n=n&(n-1);
		r++;
		}
	return r;
	}
	*/

int LSB(int32 x)
	{
	//-----------------------------------------------------------------------------------------------------
	// returns the position of the least significant bit in a 32-bit word x 
	// or -1 if not found, if x=0.
	// LSB uses "intrinsics" for an efficient implementation
	//-----------------------------------------------------------------------------------------------------

	unsigned long returnvalue;

	//_BitScanForward(&returnvalue, x); 
	//return returnvalue; 
	
	// todo: that looks like a bug! if x is 1, it should give 0 but will return -1!?
	//assert(x != 1);
	//if (x == 1)
	//	printf("!");
	if (x != 0) {
		(_BitScanForward(&returnvalue, x));
		return returnvalue;
	}
	return -1;
	}


int MSB(int32 x)
	{
	//-----------------------------------------------------------------------------------------------------
	// returns the position of the most significant bit in a 32-bit word x 
	// or -1 if not found, if x=0.
	//-----------------------------------------------------------------------------------------------------
	unsigned long returnvalue;

	//assert(x != 1); 
	//if (x == 1)
	//	printf("!");
	if (x != 0) {
		(_BitScanReverse(&returnvalue, x));
		return returnvalue;
	}
	return -1;
	}

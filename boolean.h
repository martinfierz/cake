//void initbool(void);
//int recbitcount(int32 n);
int LSB(int32 x);
//unsigned int bitcount(int32 n);
//#define bitcount(a) __popcnt((a))

#define bitcount(x) __popcnt(x)


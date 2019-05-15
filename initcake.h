HASHENTRY *loadbook(int *bookentries, int *bookmovenum);
int initbitoperations(unsigned char bitsinword[65536], unsigned char LSBarray[256]);
HASHENTRY *inithashtable(int hashsize);
int initxors(int *ptr);
int recbitcount(int32 n);


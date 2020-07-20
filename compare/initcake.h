HASHENTRY *loadbook(int *bookentries, int *bookmovenum, FILE *fp);
int initbitoperations(unsigned char bitsinword[65536], unsigned char LSBarray[256]);
HASHENTRY *inithashtable(int hashsize, FILE *logfile);
int initxors(int *ptr);
int recbitcount(int32 n);


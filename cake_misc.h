void searchinfotostring(char *out, int depth, double time, char *valuestring, char *pvstring, SEARCHINFO *counter);
void movetonotation(POSITION *p, MOVE *m, char *str);
void printint32(int32 x);
void printboard(POSITION *p);
void printboardtofile(POSITION *p);
int isforced(POSITION *p);
int SquareToBit(int square);
int logtofile(char *str);
FILE * getlogfile(void);
void clearlogfile();
void getcakedir(char *lstr);



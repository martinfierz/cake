void clearlogfile();
FILE * getlogfile(int clear);
int exitcake();
void getcakedir(char *lstr);
int isforced(POSITION *p);
int logtofile(FILE *fp, char *str);
void movetonotation(POSITION *p, MOVE *m, char *str);
void printint32(int32 x);
void printboard(POSITION *p);
void printboardtofile(POSITION *p, FILE*fp);
void resetsearchinfo(SEARCHINFO *s);
void searchinfotostring(char *out, int depth, double time, char *valuestring, char *pvstring, SEARCHINFO *counter);
int SquareToBit(int square);



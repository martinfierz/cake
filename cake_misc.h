void clearlogfile();
FILE * getlogfile(void);
int exitcake();
void getcakedir(char *lstr);
int isforced(POSITION *p);
int logtofile(char *str);
void movetonotation(POSITION *p, MOVE *m, char *str);
void printint32(int32 x);
void printboard(POSITION *p);
void printboardtofile(POSITION *p);
void resetsearchinfo(SEARCHINFO *s);
void searchinfotostring(char *out, int depth, double time, char *valuestring, char *pvstring, SEARCHINFO *counter);
int SquareToBit(int square);



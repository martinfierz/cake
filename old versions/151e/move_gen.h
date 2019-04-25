/* movegen.h: function prototypes of movegen.c */

int makemovelist(struct pos *p,struct move movelist[MAXMOVES],int values[MAXMOVES],int color, int bestindex, int32 killer);
static void blackorderevaluation(struct pos *p,struct move ml[MAXMOVES],int values[MAXMOVES],int n);
static void whiteorderevaluation(struct pos *p,struct move ml[MAXMOVES],int values[MAXMOVES],int n);

/* captgen.h: function prototypes for captgen.c */

int makecapturelist(struct pos *p,struct move movelist[MAXMOVES],int values[MAXMOVES],int color, int32 bestindex);

static void blackmancapture1( struct pos *p,struct move movelist[MAXMOVES],int values[MAXMOVES], int *n, struct move *partial, int32 square);
static void blackkingcapture1(struct pos *p,struct move movelist[MAXMOVES],int values[MAXMOVES], int *n, struct move *partial, int32 square);
static void whitemancapture1( struct pos *p,struct move movelist[MAXMOVES],int values[MAXMOVES], int *n, struct move *partial, int32 square);
static void whitekingcapture1(struct pos *p,struct move movelist[MAXMOVES],int values[MAXMOVES], int *n, struct move *partial, int32 square);
static void blackmancapture2( struct pos *p,struct move movelist[MAXMOVES],int values[MAXMOVES], int *n, struct move *partial, int32 square);
static void blackkingcapture2(struct pos *p,struct move movelist[MAXMOVES],int values[MAXMOVES], int *n, struct move *partial, int32 square);
static void whitemancapture2( struct pos *p,struct move movelist[MAXMOVES],int values[MAXMOVES], int *n, struct move *partial, int32 square);
static void whitekingcapture2(struct pos *p,struct move movelist[MAXMOVES],int values[MAXMOVES], int *n, struct move *partial, int32 square);

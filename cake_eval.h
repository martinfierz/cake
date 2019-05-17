int evaluation_nomaterial_black(POSITION *p, int depth);
int evaluation_nomaterial_white(POSITION *p, int depth);
int evaluation_nomove(int depth);

int evaluation(POSITION *p, MATERIALCOUNT *mc, int alpha, int *delta, int capture, int maxNdb);
int fineevaluation(EVALUATION *e, POSITION *p, MATERIALCOUNT *mc, KINGINFO *ki, int *noprune, int*likelydraw);
int selftrapeval(POSITION *p, MATERIALCOUNT *mc, KINGINFO *ki, int *delta);
static int32 attack_backwardjump(int32 x, int32 free);
static int32 attack_forwardjump(int32 x, int32 free);
static int32 backwardjump(int32 x, int32 free,int32 other);
static int32 forwardjump(int32 x, int32 free,int32 other);
int initeval(void);
int materialevaluation(MATERIALCOUNT *m);

int initializematerial(short materialeval[13][13][13][13]);
int initializebackrank(char blackbackrankeval[256], char whitebackrankeval[256], char blackbackrankpower[256], char whitebackrankpower[256]);


#ifdef USEDB
int dbwineval(POSITION *p, MATERIALCOUNT *mc);
#endif

enum {
	devsinglecorner = 0, intactdoublecorner, oreoval, idealdoublecornerval, backrankpower1,
	backrankpower2, backrankpower3, king_value, nocrampval, dogholeval, dogholemandownval,
	mc_occupyval, mc_attackval, realdykeval, greatdykeval,
	promoteinone, promoteintwo, promoteinthree, tailhookval, kcval, keval,
	turnval, kingcentermonopoly, kingtrappedinsinglecornerval,
	kingtrappedinsinglecornerbytwoval, kingtrappedindoublecornerval, dominatedkingval, dominatedkingindcval,
	kingproximityval, immobilemanval, kingholdstwomenval, onlykingval, roamingkingval,
	man_value, balancemult, skewnessmult, cramp12, cramp13, cramp20, badstructure, dogholeval2, badstructure2, 
	badstructuremax1, badstructuremax2, badstructuremin, badstructure3, badstructure4, 
	badstructure2stones, kingmanstones, /*ungroundedcontact, endangeredbridge, */arraystart
};
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
int setparams(int* params, int n);
int optimalparams(void);
int startparams(void); 
int updateeval(void);

#ifdef USEDB
int dbwineval(POSITION *p, MATERIALCOUNT *mc);
#endif

enum {
	devsinglecorner = 0, intactdoublecorner, oreoval, idealdoublecornerval, backrankpower1,
	backrankpower2, backrankpower3, backrankpower4, backrankpower5, 
	king_value, nocrampval13, nocrampval20, 
	dogholeval,	dogholemandownval,
	mc_occupyval, mc_attackval, realdykeval, greatdykeval,
	promoteinone, promoteintwo, promoteinthree, tailhookval, kcval, keval,
	turnval, turnval_eg, kingcentermonopoly, kingtrappedinsinglecornerval,
	kingtrappedinsinglecornerbytwoval, kingtrappedindoublecornerval, dominatedkingval, dominatedkingindcval,
	kingproximityval1, kingproximityval2, immobilemanval, kingholdstwomenval, onlykingval, roamingkingval,
	man_value, balancemult, skewnessmult, skewnessmult_eg,
	cramp12, cramp13, cramp13_eg, cramp20, badstructure, dogholeval2,
	badstructure2, 
	/*badstructuremax1, badstructuremax2, badstructuremin, */badstructure3, badstructure4, 
	badstructure5, badstructure6, badstructure7, badstructure8,
	badstructure9, badstructure10, badstructure11,
	/*badstructure2stones,*/ kingmanstones, immobile_mult, immobile_mult_kings, runaway_destroys_backrank,
	king_blocks_king_and_man, king_denied_center, king_low_mobility_mult, king_no_mobility,
	experimental_king_cramp, compensation, compensation_mandown, /*compensation_mandown_norunaway,*/
	ungroundedcontact, endangeredbridge, endangeredbridge_kingdown, arraystart
};



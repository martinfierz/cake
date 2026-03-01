int evaluation_nomaterial_black(POSITION *p, int depth);
int evaluation_nomaterial_white(POSITION *p, int depth);
int evaluation_nomove(int depth);

int evaluation(POSITION *p, MATERIALCOUNT *mc, int alpha, int *delta, int capture, int maxNdb);
int fineevaluation(EVALUATION *e, POSITION *p, MATERIALCOUNT *mc, KINGINFO *ki, int *noprune, int*likelydraw);
//int selftrapeval(POSITION *p, MATERIALCOUNT *mc, KINGINFO *ki, int *delta);
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
int reverse(int x); 
int reverse12(int x);
int reverse16(int x); 
int indexk_get_magic(POSITION* p);
int indexk_reverse_get_magic(POSITION* p);
int indexk2_get_magic(POSITION* p);
int indexk2_reverse_get_magic(POSITION* p);
int indexk_side_get_magic(POSITION* p);
int indexk_side_reverse_get_magic(POSITION* p);
int indexk_center_get_magic(POSITION* p);
int indexk_center_reverse_get_magic(POSITION* p);

#if defined USEDB || defined USE_KR_DB
int dbwineval(POSITION *p, MATERIALCOUNT *mc);
#endif

#ifdef PATTERNS
int index_reverse_get(POSITION* p);
int index_get(POSITION* p);
//int indexk_get(POSITION* p);
//int indexk_reverse_get(POSITION* p);
//int indexk2_get(POSITION* p);
//int indexk2_reverse_get(POSITION* p);
int indices_create();
int patterns_set_to_zero();
#endif


/*
enum {
	man_value = 0, king_value, piecedown_9, piecedown_11, twokingbonus_10, twokingbonus_12, exchangebias,

	backrankpower1, backrankpower2, backrankpower3, backrankpower4,
	nocrampval13, nocrampval20,
	dogholeval, dogholemandownval,
	mc_occupyval, mc_attackval, realdykeval, greatdykeval,
	promoteinone, promoteintwo, promoteinthree, tailhookval, dominatedkingval2, keval,
	turnval, turnval_eg, kingcentermonopoly, kingtrappedinsinglecornerval,
	kingtrappedinsinglecornerbytwoval, kingtrappedindoublecornerval, dominatedkingval, dominatedkingindcval,
	kingproximityval1, kingproximityval2, immobilemanval, kingholdstwomenval, onlykingval, roamingkingval,
	balancemult, skewnessmult, skewnessmult_eg,
	cramp12, cramp13, cramp13_eg, cramp20, badstructure, dogholeval2,
	badstructure2,
	badstructure3, badstructure4,
	badstructure5, badstructure6, badstructure7, badstructure8,
	badstructure9, badstructure10, badstructure11,
	 dominatedkingindcval2, immobile_mult, immobile_mult_kings, runaway_destroys_backrank,
	king_blocks_king_and_man, king_denied_center, edgepressure1, edgepressure2,
	experimental_king_cramp, compensation, compensation_mandown, 
	ungroundedcontact, endangeredbridge, endangeredbridge_kingdown, arraystart
};*/


enum {
	man_value = 0, king_value, piecedown_9, piecedown_11, twokingbonus_10, twokingbonus_12, exchangebias, 
	backrankpower1, backrankpower2, backrankpower3, backrankpower4, 
	immobile_mult, immobile_mult_kings, ungroundedcontact, balancemult, skewnessmult, skewnessmult_eg,
	dogholeval2, dogholeval, dogholemandownval,
	mc_occupyval, mc_attackval, realdykeval, greatdykeval,
	cramp20, nocrampval20, cramp13, cramp13_eg, nocrampval13, cramp12,
	badstructure3, badstructure4, badstructure,
	badstructure2, badstructure5, badstructure6, badstructure7, badstructure8,
	badstructure9, badstructure10, badstructure11,
	promoteinone, runaway_destroys_backrank, promoteintwo, promoteinthree, 
	kingtrappedinsinglecornerval, kingtrappedinsinglecornerbytwoval, kingtrappedindoublecornerval,
	king_blocks_king_and_man, dominatedkingval, dominatedkingval2, dominatedkingindcval, dominatedkingindcval2,
	king_denied_center, kingcentermonopoly, onlykingval, roamingkingval,
	experimental_king_cramp, keval,
	tailhookval, kingproximityval1, kingproximityval2, endangeredbridge_kingdown, endangeredbridge,
	kingholdstwomenval, immobilemanval, compensation, compensation_mandown,
	turnval, turnval_eg, 
	edgepressure1, edgepressure2,
	arraystart
};



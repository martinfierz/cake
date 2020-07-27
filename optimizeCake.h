// this header file is shared among the 3 projects to optimize Cake's eval and 
// tells all subprojects how to optimize

// noduplicates define removes duplicate games from the position list in ReadMatchFile
// when generating the taggedpositions file
#undef NODUPLICATES

// average lumps together all results of a single position and can thus also do fractional
// scores (not just win/loss/draw) and will optimize towards the fractional score
#define AVERAGE

#define REJECTNONQUIET// throw away positions that look nonquiet

#define STOCHASTIC  // run minibatches
#define STOCHASTICNUM 10 // only look at one in 10 positions

#define ZERO // start all params at 0!
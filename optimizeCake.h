// this header file is shared among the 3 projects to optimize Cake's eval and 
// tells all subprojects how to optimize


// use new files or old intellectual incest?
#define NEWFILES

// noduplicates define removes duplicate games from the position list in ReadMatchFile
// when generating the taggedpositions file
#undef NODUPLICATES

// average lumps together all results of a single position and can thus also do fractional
// scores (not just win/loss/draw) and will optimize towards the fractional score
#define AVERAGE

#undef OPTIMIZE_SEARCHEVAL 

#define REJECTNONQUIET// throw away positions that look nonquiet

#undef STOCHASTIC  // run minibatches
#define STOCHASTICNUM 4 // only look at one in 4 positions

#undef ZERO // start all params at 0!
typedef unsigned long int ub4;		/* unsigned 4-byte quantities */
typedef unsigned char ub1;			/* unsigned 1-byte quantities */

ub4 jenkins_hash(ub1 *k, ub4 length, ub4 initval);
ub4 jenkins_hash2(ub1 *k, ub4 length, ub4 initval);



/* Mersenne Twister PRNG. */

/* < http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/ARTICLES/mt.pdf > */
/* < http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/MT2002/emt19937ar.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef PIZ_MT32_H
#define PIZ_MT32_H

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _PIZRandom {
    PIZUInt32   mt_[624];
    int         mti_;
    } MTState32;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

MTState32   *genrand32_new          (void);
MTState32   *genrand32_newByArray   (long argc, PIZUInt32 *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void        genrand32_free  (MTState32 *x);
PIZUInt32   genrand32_int32 (MTState32 *x);  // -- Random number on [0, 0xffffffff] interval. 
double      genrand32_real2 (MTState32 *x);  // -- Random number on [0, 1) interval.
double      genrand32_res53 (MTState32 *x);  // -- Random number on [0, 1) interval with 53-bit resolution.

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // PIZ_MT32_H


/* 
    Copyright (c) 2014, Nicolas Danet, < nicolas.danet@free.fr >. 
*/

/* < http://opensource.org/licenses/MIT > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef PIZ_RANDOM_H
#define PIZ_RANDOM_H

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_32BIT

    #include "pizMT32.h"
    typedef MTState32 PIZRandom;

#endif // PD_32BIT

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_64BIT

    #include "pizMT64.h"
    typedef MTState64 PIZRandom;

#endif // PD_64BIT

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

PIZRandom   *pizRandomNew   (void);

void        pizRandomFree   (PIZRandom *x);
double      pizRandomDouble (PIZRandom *x);             // -- Random float on [0, 1) interval.   
long        pizRandomLong   (PIZRandom *x, long v);     // -- Random integer on [0, v) interval.

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // PIZ_RANDOM_H


/* 
    Copyright (c) 2014, Nicolas Danet, < nicolas.danet@free.fr >. 
*/

/* < http://opensource.org/licenses/MIT > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "pizUtils.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://graphics.stanford.edu/~seander/bithacks.html > */
/* < http://aggregate.org/MAGIC/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Use intrinsics? */

/* < http://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers > */
/* < http://hackage.haskell.org/package/bits-0.3.3/src/cbits/debruijn.c > */
/* < http://hackage.haskell.org/package/bits-0.3.3/docs/src/Data-Bits-Extras.html > */
/* < http://llvm.org/docs/doxygen/html/MathExtras_8h_source.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static const int pizUtilsDeBruijn32[] =
    {   
        0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
        8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31   
    };

static const int pizUtilsDeBruijn64[] =
    {
        63, 0, 58, 1, 59, 47, 53, 2, 60, 39, 48, 27, 54, 33, 42, 3,
        61, 51, 37, 40, 49, 18, 28, 20, 55, 30, 34, 11, 43, 14, 22, 4,
        62, 57, 46, 52, 38, 26, 32, 41, 50, 36, 17, 19, 29, 10, 13, 21,
        56, 45, 25, 31, 35, 16, 9, 12, 44, 24, 15, 8, 23, 7, 6, 5
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int pizUInt32LogBase2Index (uint32_t v)
{
    if (!v) { return 0; }
    else {
    //
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    
    return (pizUtilsDeBruijn32[(uint32_t)(v * 0x07c4acddU) >> 27]);
    //
    }
}

int pizUInt32NextPower2Index (uint32_t v)
{
    if (PD_IS_POWER_2 (v)) {
        return pizUInt32LogBase2Index (v);
    } else {
        return pizUInt32LogBase2Index (v) + 1;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int pizUInt64LogBase2Index (uint64_t v)
{
    if (!v) { return 0; }
    else {
    //
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    
    return (pizUtilsDeBruijn64[((uint64_t)((v - (v >> 1)) * 0x07edd5e59a4e28c2ULL)) >> 58]);
    //
    }
}

int pizUInt64NextPower2Index (uint64_t v)
{
    if (PD_IS_POWER_2 (v)) {
        return pizUInt64LogBase2Index (v);
    } else {
        return pizUInt64LogBase2Index (v) + 1;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

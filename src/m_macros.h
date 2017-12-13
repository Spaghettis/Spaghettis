
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __m_macros_h_
#define __m_macros_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_SHORT_FILE       (strrchr (__FILE__, '/') ? strrchr (__FILE__, '/') + 1 : __FILE__)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_WITH_DEBUG

    #define PD_BUG          PD_ASSERT (0)
    #define PD_ASSERT(x)    if (!(x)) { post_log ("*** Assert / %s / line %d", PD_SHORT_FILE, __LINE__); }

#else
    
    #define PD_BUG
    #define PD_ASSERT(x)

#endif // PD_WITH_DEBUG

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_UNUSED(x)        (void)(x)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define PD_ABORT(x)         if (x) { abort(); }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if ( PD_GCC || PD_CLANG )

    #define PD_RESTRICTED   t_sample* __restrict__

#else

    #define PD_RESTRICTED   t_sample*

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_ATOMS_ALLOCA(x, n)  \
    (x) = (t_atom *)((n) < 64 ? alloca ((n) * sizeof (t_atom)) : PD_MEMORY_GET ((n) * sizeof (t_atom)))
        
#define PD_ATOMS_FREEA(x, n)   \
    if (n >= 64) { PD_MEMORY_FREE ((x)); }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Roughly every object that is not a scalar. */

#define cast_objectIfConnectable(x)         (class_isBox (pd_class (x)) ? (t_object *)(x) : NULL)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define ADDRESS_FLOAT(atom)                 &((atom)->a_w.w_float)
#define ADDRESS_SYMBOL(atom)                &((atom)->a_w.w_symbol)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define inlet_new2(x, type)                 inlet_new (cast_object ((x)), cast_pd ((x)), (type), sym__inlet2)
#define inlet_new3(x, type)                 inlet_new (cast_object ((x)), cast_pd ((x)), (type), sym__inlet3)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_INT_MAX                          0x7fffffff
#define PD_FLT_MAX                          FLT_MAX
#define PD_DBL_MAX                          DBL_MAX
#define PD_EPSILON                          1E-9

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_IS_POWER_2(v)                    (!((v) & ((v) - 1)))
#define PD_NEXT_POWER_2(v)                  sys_nextPowerOfTwo ((uint64_t)(v))
#define PD_TO_RADIANS(degrees)              ((PD_PI * (degrees)) / 180.0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Notice that it returns zero with an argument of zero. */

static inline uint64_t sys_nextPowerOfTwo (uint64_t v)
{
    v--;
    v |= (v >> 1);
    v |= (v >> 2);
    v |= (v >> 4);
    v |= (v >> 8);
    v |= (v >> 16);
    v |= (v >> 32);
    v++;
    
    return v;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define SECONDS_TO_MILLISECONDS(n)          ((double)(n) * 1000.0)
#define MILLISECONDS_TO_SECONDS(n)          ((double)(n) * 1e-3)
#define SECONDS_TO_MICROSECONDS(n)          ((double)(n) * 1000000.0)
#define MICROSECONDS_TO_SECONDS(n)          ((double)(n) * 1e-6)
#define MILLISECONDS_TO_MICROSECONDS(n)     ((double)(n) * 1000.0)
#define MICROSECONDS_TO_MILLISECONDS(n)     ((double)(n) * 1e-3)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Assumed IEEE 754 floating-point format. */

typedef union {
    t_float     z_f;
    uint32_t    z_i;
    } t_rawcast32;

typedef union {
    double      z_d;
    uint32_t    z_i[2];
    } t_rawcast64;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_LITTLE_ENDIAN

    #define PD_RAWCAST64_MSB        1                                                              
    #define PD_RAWCAST64_LSB        0

#else
                                                                      
    #define PD_RAWCAST64_MSB        0
    #define PD_RAWCAST64_LSB        1

#endif // PD_LITTLE_ENDIAN

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -
        
static inline int PD_IS_NAN (t_float f)                 /* True if NaN. */
{
    t_rawcast32 z;
    z.z_f = f;
    return ((z.z_i & 0x7fffffff) > 0x7f800000);
}

static inline int PD_IS_DENORMAL_OR_ZERO (t_float f)    /* True if zero, denormal, infinite, or NaN. */
{
    t_rawcast32 z;
    z.z_f = f;
    z.z_i &= 0x7f800000;
    return ((z.z_i == 0) || (z.z_i == 0x7f800000));
}

static inline int PD_IS_BIG_OR_SMALL (t_float f)        /* True if exponent falls out (-64, 64) range. */
{
    t_rawcast32 z;
    z.z_f = f;
    return ((z.z_i & 0x20000000) == ((z.z_i >> 1) & 0x20000000)); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Rand48. */

/* < http://en.wikipedia.org/wiki/Linear_congruential_generator > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_RAND48_INIT(s)           ((s) = (t_rand48)time_makeRandomSeed() & 0xffffffffffffULL)
#define PD_RAND48_NEXT(s)           ((s) = (((s) * 0x5deece66dULL + 0xbULL) & 0xffffffffffffULL))
#define PD_RAND48_UINT32(s)         (PD_RAND48_NEXT (s) >> 16)
#define PD_RAND48_DOUBLE(s)         (PD_RAND48_UINT32 (s) * (1.0 / 4294967296.0))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Infamously bad RANDU. */

/* < http://en.wikipedia.org/wiki/RANDU > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PIZ_RANDU_INIT(s)           ((s) = ((uint32_t)time_makeRandomSeed() | 1) & 0x7fffffff)
#define PIZ_RANDU_UINT32(s)         ((s) = (65539 * (s)) & 0x7fffffff)
#define PIZ_RANDU_DOUBLE(s)         (PIZ_RANDU_UINT32 (s) * (1.0 / 2147483648.0))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_macros_h_

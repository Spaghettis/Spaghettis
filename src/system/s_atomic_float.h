
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __s_atomic_float_h_
#define __s_atomic_float_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_ATOMIC_FLOAT64_READ(q)           atomic_float64Read ((q))
#define PD_ATOMIC_FLOAT64_WRITE(value, q)   atomic_float64Write ((value), (q))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_LOAD_STORE_64_IS_ATOMIC

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef volatile double __attribute__ ((__aligned__ (8)))   t_float64Atomic;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline double atomic_float64Read (t_float64Atomic *q)
{
    return (*q);
}

static inline void atomic_float64Write (double f, t_float64Atomic *q)
{
    (*q) = f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#else

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef t_uint64Atomic t_float64Atomic;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Just wrap integer versions with type punning. */

static inline double atomic_float64Read (t_float64Atomic *q)
{
    t_rawcast64 t; t.z_l = PD_ATOMIC_UINT64_READ (q); return t.z_d;
}

static inline void atomic_float64Write (double f, t_float64Atomic *q)
{
    t_rawcast64 t; t.z_d = f; PD_ATOMIC_UINT64_WRITE (t.z_l, q);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // PD_LOAD_STORE_64_IS_ATOMIC

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_atomic_float_h_

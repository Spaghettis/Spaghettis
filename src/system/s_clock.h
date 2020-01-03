
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __s_clock_h_
#define __s_clock_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct _clock {
    t_float64Atomic c_systime;
    t_float64Atomic c_unit;         /* A positive value in milliseconds (negative in samples). */
    t_int32Atomic   c_count;
    t_float         c_t;
    t_clockfn       c_fn;
    void            *c_owner;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

double  clock_getRealTimeInSeconds  (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     clock_setUnitParsed     (t_clock *x, t_float f, t_symbol *unitName);

int         clock_isSet             (t_clock *);
t_systime   clock_getLogicalTime    (t_clock *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_clock_h_

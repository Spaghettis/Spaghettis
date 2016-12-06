
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __d_math_h_
#define __d_math_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define RSQRT_MANTISSA_SIZE         1024
#define RSQRT_EXPONENTIAL_SIZE      256

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_float rsqrt_tableMantissa[];
extern t_float rsqrt_tableExponential[];

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void    rsqrt_tilde_initialize  (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_int   *sqrt_tilde_perform     (t_int *w);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_math_h_
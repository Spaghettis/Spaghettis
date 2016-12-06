
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "d_dsp.h"
#include "d_math.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Square root good to 8 mantissa bits.  */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *sqrt_tilde_class;           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct sqrt_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_sqrt_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_int *sqrt_tilde_perform (t_int *w)
{
    t_sample *in = *(t_sample **)(w+1), *out = *(t_sample **)(w+2);
    t_int n = *(t_int *)(w+3);
    while (n--)
    {   
        t_sample f = *in++;
        union {
          float f;
          long l;
        } u;
        u.f = f;
        if (f < 0) *out++ = 0;
        else
        {
            t_sample g = rsqrt_tableExponential[(u.l >> 23) & 0xff] *
                rsqrt_tableMantissa[(u.l >> 13) & 0x3ff];
            *out++ = f * (1.5 * g - 0.5 * g * g * g * f);
        }
    }
    return (w + 4);
}

static void sqrt_tilde_dsp (t_sqrt_tilde *x, t_signal **sp)
{
    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    
    dsp_add (sqrt_tilde_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *sqrt_tilde_new (void)
{
    t_sqrt_tilde *x = (t_sqrt_tilde *)pd_new (sqrt_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void sqrt_tilde_setup (void)
{
    t_class *c = NULL;
    
    rsqrt_tilde_initialize();
    
    c = class_new (sym_sqrt__tilde__,
            (t_newmethod)sqrt_tilde_new,
            NULL,
            sizeof (t_sqrt_tilde),
            CLASS_DEFAULT,
            A_NULL);
    
    CLASS_SIGNAL (c, t_sqrt_tilde, x_f);
    
    class_addDSP (c, sqrt_tilde_dsp);
    
    #if PD_WITH_LEGACY
    
    class_addCreator (sqrt_tilde_new, sym_q8_sqrt__tilde__, A_NULL);
    
    #endif
    
    sqrt_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

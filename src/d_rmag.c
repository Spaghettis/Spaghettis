
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "d_dsp.h"
#include "d_math.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *rmag_tilde_class;           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _rmag_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_rmag_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */
/* Notice that the two signals incoming could be theoretically just one. */
/* But as only loads are done, it is assumed safe to use restricted pointers. */

t_int *rmag_tilde_perform (t_int *w)
{
    PD_RESTRICTED in1 = (t_sample *)(w[1]);
    PD_RESTRICTED in2 = (t_sample *)(w[2]);
    PD_RESTRICTED out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n--) {
    //
    t_sample f = *in1++;
    t_sample g = *in2++;

    *out++ = (t_sample)rsqrt_fast ((t_float)(f * f + g * g));
    //
    }
    
    return (w + 5);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void rmag_tilde_dsp (t_rmag_tilde *x, t_signal **sp)
{
    dsp_add (rmag_tilde_perform, 4, sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void *rmag_tilde_new (t_symbol *s, int argc, t_atom *argv)
{
    t_rmag_tilde *x = (t_rmag_tilde *)pd_new (rmag_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_newSignal (cast_object (x));

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void rmag_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_rmag__tilde__,
            (t_newmethod)rmag_tilde_new,
            NULL,
            sizeof (t_rmag_tilde),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    
    CLASS_SIGNAL (c, t_rmag_tilde, x_f);
    
    class_addDSP (c, (t_method)rmag_tilde_dsp);
    
    rmag_tilde_class = c;
}

void rmag_tilde_destroy (void)
{
    class_free (rmag_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

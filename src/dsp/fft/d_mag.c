
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *mag_tilde_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _mag_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_mag_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Notice that the two signals incoming could be theoretically just one. */
/* But as only loads are done, it is assumed safe to use restricted pointers. */

static void mag_tilde_dsp (t_mag_tilde *x, t_signal **sp)
{
    dsp_addMagnitudePerform (sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void *mag_tilde_new (t_symbol *s, int argc, t_atom *argv)
{
    t_mag_tilde *x = (t_mag_tilde *)pd_new (mag_tilde_class);
    
    x->x_outlet = outlet_newSignal (cast_object (x));
    
    inlet_newSignal (cast_object (x));

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void mag_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_mag__tilde__,
            (t_newmethod)mag_tilde_new,
            NULL,
            sizeof (t_mag_tilde),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    
    CLASS_SIGNAL (c, t_mag_tilde, x_f);
    
    class_addDSP (c, (t_method)mag_tilde_dsp);
    
    mag_tilde_class = c;
}

void mag_tilde_destroy (void)
{
    class_free (mag_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------


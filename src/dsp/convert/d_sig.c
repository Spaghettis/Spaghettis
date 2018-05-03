
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *sig_tilde_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _sig_tilde {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_sig_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void sig_tilde_float (t_sig_tilde *x, t_float f)
{
    x->x_f = f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void sig_tilde_dsp (t_sig_tilde *x, t_signal **sp)
{
    dsp_addScalarPerform (&x->x_f, sp[0]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *sig_tilde_new (t_float f)
{
    t_sig_tilde *x = (t_sig_tilde *)pd_new (sig_tilde_class);
    
    x->x_f      = f;
    x->x_outlet = outlet_newSignal (cast_object (x));
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void sig_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_sig__tilde__,
            (t_newmethod)sig_tilde_new,
            NULL,
            sizeof (t_sig_tilde),
            CLASS_DEFAULT, 
            A_DEFFLOAT,
            A_NULL);
    
    class_addDSP (c, (t_method)sig_tilde_dsp);
    class_addFloat (c, (t_method)sig_tilde_float);
    
    sig_tilde_class = c;
}

void sig_tilde_destroy (void)
{
    class_free (sig_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
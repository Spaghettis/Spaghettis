
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

#include "d_global.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *receive_tilde_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _receive_tilde {
    t_object    x_obj;                          /* Must be the first. */
    t_sample    *x_vector;
    t_symbol    *x_name;
    t_outlet    *x_outlet;
    } t_receive_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void receive_tilde_set (t_receive_tilde *x, t_symbol *s)
{
    t_send_tilde *sender = (t_send_tilde *)symbol_getThingByClass ((x->x_name = s), send_tilde_class);
    
    x->x_vector = NULL;
    
    if (!sender) { if (x->x_name != &s_) { error_canNotFind (sym_receive__tilde__, x->x_name); } }
    else {
        x->x_vector = sender->x_vector;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *receive_tilde_perform (t_int *w)
{
    t_receive_tilde *x = (t_receive_tilde *)(w[1]);
    PD_RESTRICTED out  = (t_sample *)(w[2]);
    PD_RESTRICTED in   = x->x_vector;
    
    if (in) { memcpy (out, in, INTERNAL_BLOCKSIZE * sizeof (t_sample)); }
    else {
        memset (out, 0, INTERNAL_BLOCKSIZE * sizeof (t_sample));
    }
    
    return (w + 3);
}

static void receive_tilde_dsp (t_receive_tilde *x, t_signal **sp)
{
    if (sp[0]->s_vectorSize != INTERNAL_BLOCKSIZE) { error_mismatch (sym_receive__tilde__, sym_size); }
    else {
        receive_tilde_set (x, x->x_name);
        PD_ASSERT (x->x_vector != sp[0]->s_vector);
        dsp_add (receive_tilde_perform, 2, x, sp[0]->s_vector);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *receive_tilde_new (t_symbol *s)
{
    t_receive_tilde *x = (t_receive_tilde *)pd_new (receive_tilde_class);
    
    x->x_vector = NULL;
    x->x_name   = s;
    x->x_outlet = outlet_newSignal (cast_object (x));
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void receive_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_receive__tilde__,
            (t_newmethod)receive_tilde_new,
            NULL,
            sizeof (t_receive_tilde),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addCreator ((t_newmethod)receive_tilde_new, sym_r__tilde__, A_DEFSYMBOL, A_NULL);
    
    class_addDSP (c, (t_method)receive_tilde_dsp);
    
    class_addMethod (c, (t_method)receive_tilde_set, sym_set, A_SYMBOL, A_NULL);
    
    class_setHelpName (c, sym_send__tilde__);
    
    receive_tilde_class = c;
}

void receive_tilde_destroy (void)
{
    class_free (receive_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

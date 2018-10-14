
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

/* Biquad filter. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://ccrma.stanford.edu/~jos/filters/Direct_Form_II.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *biquad_tilde_class;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _biquad_tilde_control {
    t_sample                c_real1;
    t_sample                c_real2;
    t_sample                c_a1;
    t_sample                c_a2;
    t_sample                c_b0;
    t_sample                c_b1;
    t_sample                c_b2;
    } t_biquad_tilde_control;

typedef struct biquad_tilde {
    t_object                x_obj;              /* Must be the first. */
    t_float                 x_f;
    t_biquad_tilde_control  x_space;
    t_outlet                *x_outlet;
    } t_biquad_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void biquad_tilde_list (t_biquad_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float a1 = atom_getFloatAtIndex (0, argc, argv);
    t_float a2 = atom_getFloatAtIndex (1, argc, argv);
    t_float b0 = atom_getFloatAtIndex (2, argc, argv);
    t_float b1 = atom_getFloatAtIndex (3, argc, argv);
    t_float b2 = atom_getFloatAtIndex (4, argc, argv);
    
    x->x_space.c_a1 = (t_sample)a1;
    x->x_space.c_a2 = (t_sample)a2;
    x->x_space.c_b0 = (t_sample)b0;
    x->x_space.c_b1 = (t_sample)b1;
    x->x_space.c_b2 = (t_sample)b2;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *biquad_tilde_perform (t_int *w)
{
    t_biquad_tilde_control *c = (t_biquad_tilde_control *)(w[1]);
    PD_RESTRICTED in  = (t_sample *)(w[2]);
    PD_RESTRICTED out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    t_sample last1  = c->c_real1;
    t_sample last2  = c->c_real2;
    t_sample a1     = c->c_a1;
    t_sample a2     = c->c_a2;
    t_sample b0     = c->c_b0;
    t_sample b1     = c->c_b1;
    t_sample b2     = c->c_b2;
    
    while (n--) {
    //
    t_sample f = (*in++) + a1 * last1 + a2 * last2; 
        
    if (PD_FLOAT32_IS_BIG_OR_SMALL (f)) { f = 0.0; }
        
    *out++ = b0 * f + b1 * last1 + b2 * last2;
    last2  = last1;
    last1  = f;
    //
    }
    
    c->c_real1 = last1;
    c->c_real2 = last2;
    
    return (w + 5);
}

static void biquad_tilde_dsp (t_biquad_tilde *x, t_signal **sp)
{
    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    
    dsp_add (biquad_tilde_perform, 4, &x->x_space, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_buffer *biquad_tilde_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_biquad_tilde *x = (t_biquad_tilde *)z;
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, &s_list);
    buffer_appendFloat (b,  (t_float)x->x_space.c_a1);
    buffer_appendFloat (b,  (t_float)x->x_space.c_a2);
    buffer_appendFloat (b,  (t_float)x->x_space.c_b0);
    buffer_appendFloat (b,  (t_float)x->x_space.c_b1);
    buffer_appendFloat (b,  (t_float)x->x_space.c_b2);
    buffer_appendComma (b);
    buffer_appendSymbol (b, sym__restore);
    buffer_appendFloat (b,  (t_float)x->x_space.c_real1);
    buffer_appendFloat (b,  (t_float)x->x_space.c_real2);
    buffer_appendComma (b);
    buffer_appendSymbol (b, sym__signals);
    buffer_appendFloat (b,  x->x_f);
    
    return b;
    //
    }
    
    return NULL;
}

static void biquad_tilde_restore (t_biquad_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    x->x_space.c_real1 = (t_sample)atom_getFloatAtIndex (0, argc, argv);
    x->x_space.c_real2 = (t_sample)atom_getFloatAtIndex (1, argc, argv);
}

static void biquad_tilde_signals (t_biquad_tilde *x, t_float f)
{
    x->x_f = f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *biquad_tilde_new (t_symbol *s, int argc, t_atom *argv)
{
    t_biquad_tilde *x = (t_biquad_tilde *)pd_new (biquad_tilde_class);
    
    x->x_outlet = outlet_newSignal (cast_object (x));
    
    biquad_tilde_list (x, s, argc, argv);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void biquad_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_biquad__tilde__,
            (t_newmethod)biquad_tilde_new,
            NULL,
            sizeof (t_biquad_tilde),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    CLASS_SIGNAL (c, t_biquad_tilde, x_f);
    
    class_addDSP (c, (t_method)biquad_tilde_dsp);
    class_addList (c, (t_method)biquad_tilde_list);
    
    class_addMethod (c, (t_method)biquad_tilde_restore, sym_clear,      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)biquad_tilde_restore, sym__restore,   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)biquad_tilde_signals, sym__signals,   A_FLOAT, A_NULL);
    
    class_setDataFunction (c, biquad_tilde_functionData);
    
    biquad_tilde_class = c;
}

void biquad_tilde_destroy (void)
{
    class_free (biquad_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

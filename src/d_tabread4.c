
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "g_graphics.h"
#include "d_dsp.h"
#include "d_tab.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *tabread4_tilde_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _tabread4_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_float     x_onset;
    int         x_size;
    t_word      *x_vector;
    t_symbol    *x_name;
    t_outlet    *x_outlet;
    } t_tabread4_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tabread4_tilde_set (t_tabread4_tilde *x, t_symbol *s)
{
    tab_fetchArray ((x->x_name = s), &x->x_size, &x->x_vector, sym_tabread4__tilde__);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *tabread4_tilde_perform (t_int *w)
{
    t_tabread4_tilde *x = (t_tabread4_tilde *)(w[1]);
    PD_RESTRICTED in  = (t_sample *)(w[2]);
    PD_RESTRICTED out = (t_sample *)(w[3]);
    int n = (int)(w[4]);    
    
    int size = x->x_size;
    t_word *data = x->x_vector;
    
    if (data && size > 3) { 
    //
    while (n--) {
    //
    double position = (*in++) + (double)x->x_onset;
    int i = (int)position;
    double fractional = position - i;
    
    if (i < 1) { i = 1; fractional = 0.0; }
    else if (i > size - 3) { i = size - 3; fractional = 1.0; }

    *out++ = dsp_4PointsInterpolationWithWords ((t_float)fractional, data + i - 1);
    //
    }
    //
    } else { while (n--) { *out++ = (t_sample)0.0; } }
    
    return (w + 5);
}

static void tabread4_tilde_dsp (t_tabread4_tilde *x, t_signal **sp)
{
    tabread4_tilde_set (x, x->x_name);

    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    
    dsp_add (tabread4_tilde_perform, 4, x, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *tabread4_tilde_new (t_symbol *s)
{
    t_tabread4_tilde *x = (t_tabread4_tilde *)pd_new (tabread4_tilde_class);
    
    x->x_name   = s;
    x->x_outlet = outlet_newSignal (cast_object (x));
    
    inlet_newFloat (cast_object (x), &x->x_onset);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void tabread4_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_tabread4__tilde__,
            (t_newmethod)tabread4_tilde_new,
            NULL,
            sizeof (t_tabread4_tilde),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
            
    CLASS_SIGNAL (c, t_tabread4_tilde, x_f);
    
    class_addDSP (c, (t_method)tabread4_tilde_dsp);
    
    class_addMethod (c, (t_method)tabread4_tilde_set, sym_set, A_SYMBOL, A_NULL);
    
    tabread4_tilde_class = c;
}

void tabread4_tilde_destroy (void)
{
    class_free (tabread4_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

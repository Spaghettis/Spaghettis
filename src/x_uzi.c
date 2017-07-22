
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *uzi_class;              /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _uzi {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_count;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletMiddle;
    t_outlet    *x_outletRight;
    } t_uzi;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void uzi_proceed (t_uzi *x)
{
    int i, count = PD_MAX (1, x->x_count);
    
    for (i = 0; i < count; i++) {
    //
    outlet_float (x->x_outletRight, (t_float)i); outlet_bang (x->x_outletLeft);
    //
    }
    
    outlet_bang (x->x_outletMiddle);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void uzi_bang (t_uzi *x)
{
    uzi_proceed (x);
}

static void uzi_float (t_uzi *x, t_float f)
{
    x->x_count = f; uzi_proceed (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *uzi_new (t_symbol *s, int argc, t_atom *argv)
{
    t_uzi *x = (t_uzi *)pd_new (uzi_class);
    
    x->x_outletLeft   = outlet_new (cast_object (x), &s_bang);
    x->x_outletMiddle = outlet_new (cast_object (x), &s_bang);
    x->x_outletRight  = outlet_new (cast_object (x), &s_float);
    
    if (argc && IS_FLOAT (argv)) {
        x->x_count = GET_FLOAT (argv);
        argc-- ; argv++;
    }
    
    if (argc) {
        warning_unusedArguments (s, argc, argv);
    }

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void uzi_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_uzi,
            (t_newmethod)uzi_new,
            NULL,
            sizeof (t_uzi),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addBang (c, (t_method)uzi_bang);
    class_addFloat (c, (t_method)uzi_float);
    
    uzi_class = c;
}

void uzi_destroy (void)
{
    class_free (uzi_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
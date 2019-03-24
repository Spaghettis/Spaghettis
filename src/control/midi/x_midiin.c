
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *midiin_class;           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _midiin {
    t_object    x_obj;                  /* Must be the first. */
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_midiin;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void midiin_list (t_midiin *x, t_symbol *s, int argc, t_atom *argv)
{
    int byte = (int)atom_getFloatAtIndex (0, argc, argv);
    int port = (int)atom_getFloatAtIndex (1, argc, argv);

    outlet_float (x->x_outletRight, (t_float)port);
    outlet_float (x->x_outletLeft,  (t_float)byte);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *midiin_new (void)
{
    t_midiin *x = (t_midiin *)pd_new (midiin_class);
    
    x->x_outletLeft  = outlet_newFloat (cast_object (x));
    x->x_outletRight = outlet_newFloat (cast_object (x));
    
    pd_bind (cast_pd (x), sym__midiin);
    
    return x;
}

static void midiin_free (t_midiin *x)
{
    pd_unbind (cast_pd (x), sym__midiin);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void midiin_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_midiin,
            (t_newmethod)midiin_new,
            (t_method)midiin_free,
            sizeof (t_midiin),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_NULL);
            
    class_addList (c, (t_method)midiin_list);
    
    midiin_class = c;
}

void midiin_destroy (void)
{
    class_free (midiin_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

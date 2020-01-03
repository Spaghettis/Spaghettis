
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "x_array.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *arrayquantile_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _arrayquantile {
    t_arrayrange    x_arrayrange;           /* Must be the first. */
    t_outlet        *x_outlet;
    } t_arrayquantile;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void arrayquantile_float (t_arrayquantile *x, t_float f)
{
    if (!arrayrange_isValid (&x->x_arrayrange)) { error_invalid (sym_array__space__quantile, sym_field); }
    else {
        outlet_float (x->x_outlet, arrayrange_quantile (&x->x_arrayrange, f));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *arrayquantile_new (t_symbol *s, int argc, t_atom *argv)
{
    t_arrayquantile *x = (t_arrayquantile *)arrayrange_new (arrayquantile_class, argc, argv, 1, 1);
    
    if (ARRAYRANGE_GOOD (x)) { x->x_outlet = outlet_newFloat (cast_object (x)); }
    else {
        error_invalidArguments (sym_array__space__quantile, argc, argv);
        pd_free (cast_pd (x)); x = NULL; 
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void arrayquantile_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_array__space__quantile,
            (t_newmethod)arrayquantile_new,
            (t_method)arrayclient_free,
            sizeof (t_arrayquantile),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addFloat (c, (t_method)arrayquantile_float);
    
    class_addMethod (c, (t_method)arrayrange_restore, sym__restore, A_GIMME, A_NULL);

    class_setDataFunction (c, arrayrange_functionData);
    class_requirePending (c);
    
    class_setHelpName (c, sym_array);
    
    arrayquantile_class = c;
}

void arrayquantile_destroy (void)
{
    class_free (arrayquantile_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

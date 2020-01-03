
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "x_binop.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *atan2_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef t_binop t_atan2;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *atan2_new (void)
{
    t_atan2 *x = (t_atan2 *)pd_new (atan2_class);
    
    x->bo_outlet = outlet_newFloat (cast_object (x));
        
    inlet_newFloat (cast_object (x), &x->bo_f2);

    return x;
}

static void atan2_bang (t_atan2 *x)
{
    outlet_float (x->bo_outlet, (x->bo_f1 == 0.0 && x->bo_f2 == 0.0 ? 0.0 : atan2 (x->bo_f1, x->bo_f2)));
}

static void atan2_float (t_atan2 *x, t_float f)
{
    x->bo_f1 = f; atan2_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void atan2_setup (void)
{
    t_class *c = NULL;

    c = class_new (sym_atan2,
            (t_newmethod)atan2_new,
            NULL,
            sizeof (t_atan2),
            CLASS_DEFAULT,
            A_NULL);
    
    class_addBang (c, (t_method)atan2_bang);
    class_addFloat (c, (t_method)atan2_float);
    
    class_addMethod (c, (t_method)binop_restore, sym__restore, A_GIMME, A_NULL);

    class_setDataFunction (c, binop_functionData);
    class_requirePending (c);

    class_setHelpName (c, sym_math);
    
    atan2_class = c;
}

void atan2_destroy (void)
{
    class_free (atan2_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

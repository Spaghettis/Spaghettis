
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *arguments_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _arguments {
    t_object    x_obj;                      /* Must be the first. */
    t_glist     *x_owner;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_arguments;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void arguments_bang (t_arguments *x)
{
    t_environment *e = glist_getEnvironment (x->x_owner);
    
    PD_ASSERT (e);
    
    outlet_symbol (x->x_outletRight, symbol_removeExtension (environment_getFileName (e)));
    outlet_list (x->x_outletLeft, environment_getNumberOfArguments (e), environment_getArguments (e));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *arguments_new (void)
{
    t_arguments *x = (t_arguments *)pd_new (arguments_class);
    
    x->x_owner       = instance_contextGetCurrent();
    x->x_outletLeft  = outlet_newAnything (cast_object (x));
    x->x_outletRight = outlet_newSymbol (cast_object (x));
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void arguments_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_arguments,
            (t_newmethod)arguments_new,
            NULL,
            sizeof (t_arguments),
            CLASS_DEFAULT,
            A_NULL);
    
    class_addBang (c, (t_method)arguments_bang);
    
    arguments_class = c;
}

void arguments_destroy (void)
{
    class_free (arguments_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

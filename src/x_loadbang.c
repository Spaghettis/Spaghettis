
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *loadbang_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _loadbang {
    t_object    x_obj;                  /* Must be the first. */
    t_outlet    *x_outlet;
    } t_loadbang;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void loadbang_loadbang (t_loadbang *x)
{
    outlet_bang (x->x_outlet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *loadbang_new (void)
{
    t_loadbang *x = (t_loadbang *)pd_new (loadbang_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_bang);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void loadbang_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_loadbang,
            (t_newmethod)loadbang_new,
            NULL,
            sizeof (t_loadbang),
            CLASS_NOINLET,
            A_NULL);
            
    class_addMethod (c, (t_method)loadbang_loadbang, sym_loadbang, A_NULL);
    
    loadbang_class = c;
}

void loadbang_destroy (void)
{
    class_free (loadbang_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

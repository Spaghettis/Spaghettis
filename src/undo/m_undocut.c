
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *undocut_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _undocut {
    t_undoaction    x_undo;             /* Must be the first. */
    } t_undocut;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_undoaction *undocut_new (void)
{
    t_undoaction *x = (t_undoaction *)pd_new (undocut_class);
    
    x->ua_type  = UNDO_CUT;
    x->ua_label = sym_cut;
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undocut_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_undocut,
            NULL,
            NULL,
            sizeof (t_undocut),
            CLASS_INVISIBLE,
            A_NULL);
    
    undocut_class = c;
}

void undocut_destroy (void)
{
    class_free (undocut_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
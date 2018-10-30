
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"
#include "../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *undodisconnect_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _undodisconnect {
    t_undoaction    x_undo;                 /* Must be the first. */
    t_id            x_src;
    t_id            x_dest;
    int             x_m;
    int             x_n;
    } t_undodisconnect;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undodisconnect_undo (t_undodisconnect *z, t_symbol *s, int argc, t_atom *argv)
{
    glist_lineConnectByUnique (z->x_src, z->x_m, z->x_dest, z->x_n);
}

void undodisconnect_redo (t_undodisconnect *z, t_symbol *s, int argc, t_atom *argv)
{
    glist_lineDisconnectByUnique (z->x_src, z->x_m, z->x_dest, z->x_n);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_undoaction *undodisconnect_new (t_object *src, int m, t_object *dest, int n)
{
    t_undoaction *x = (t_undoaction *)pd_new (undodisconnect_class);
    t_undodisconnect *z = (t_undodisconnect *)x;
    
    x->ua_type  = UNDO_DISCONNECT;
    x->ua_label = sym_disconnect;
    
    z->x_src    = gobj_getUnique (cast_gobj (src));
    z->x_dest   = gobj_getUnique (cast_gobj (dest));
    z->x_m      = m;
    z->x_n      = n;
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undodisconnect_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_undodisconnect,
            NULL,
            NULL,
            sizeof (t_undodisconnect),
            CLASS_INVISIBLE,
            A_NULL);
    
    class_addMethod (c, (t_method)undodisconnect_undo, sym_undo, A_GIMME, A_NULL);
    class_addMethod (c, (t_method)undodisconnect_redo, sym_redo, A_GIMME, A_NULL);
    
    undodisconnect_class = c;
}

void undodisconnect_destroy (void)
{
    class_free (undodisconnect_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WITH_BELLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_core.h"
#include "x_mica.hpp"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *micamap_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _micamap {
    t_object    x_obj;                  /* Must be the first. */
    t_symbol    *x_tag;
    t_outlet    *x_outlet;
    } t_micamap;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void micamap_bang (t_micamap *x)
{
    outlet_symbol (x->x_outlet, x->x_tag);
}

static void micamap_list (t_micamap *x, t_symbol *s, int argc, t_atom *argv)
{
    mica::Concept t;
    
    if (argc > 1) {
    //
    mica::Concept a (concept_fetch (atom_getSymbolAtIndex (0, argc, argv)));
    mica::Concept b (concept_fetch (atom_getSymbolAtIndex (1, argc, argv)));
    mica::Concept c (concept_fetch (atom_getSymbolAtIndex (2, argc, argv)));
    
    if (argc > 2) { t = mica::map (a, b, c); }
    else {
        t = mica::map (a, b);
    }
    //
    }
    
    x->x_tag = concept_tag (t);
    
    micamap_bang (x);
}

static void micamap_anything (t_micamap *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)micamap_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *micamap_new (t_symbol *s, int argc, t_atom *argv)
{
    t_micamap *x = (t_micamap *)pd_new (micamap_class);
    
    x->x_tag    = concept_tag (mica::Undefined);
    x->x_outlet = outlet_newSymbol (cast_object (x));
    
    if (argc) { warning_unusedArguments (s, argc, argv); }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void micamap_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_mica__space__map,
            (t_newmethod)micamap_new,
            NULL,
            sizeof (t_micamap),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addBang (c, (t_method)micamap_bang);
    class_addList (c, (t_method)micamap_list);
    class_addAnything (c, (t_method)micamap_anything);
    
    class_setHelpName (c, sym_mica);
    
    micamap_class = c;
}

void micamap_destroy (void)
{
    class_free (micamap_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#else

void micamap_setup (void)
{
}

void micamap_destroy (void)
{
}

#endif // PD_WITH_BELLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------


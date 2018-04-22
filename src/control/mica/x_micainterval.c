
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WITH_BELLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "x_mica.hpp"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *micainterval_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _micainterval {
    t_object            x_obj;              /* Must be the first. */
    mica::MIR::Interval x_interval;
    t_outlet            *x_outlet;
    } t_micainterval;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void micainterval_bang (t_micainterval *x)
{
    outlet_symbol (x->x_outlet, concept_tag (x->x_interval.getName()));
}

static void micainterval_list (t_micainterval *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 2) {
    //
    mica::Concept a (concept_fetch (atom_getSymbolAtIndex (0, argc, argv)));
    mica::Concept b (concept_fetch (atom_getSymbolAtIndex (1, argc, argv)));
    
    if (!mica::index (mica::ChromaticPitches, a).isUndefined()) {
    if (!mica::index (mica::ChromaticPitches, b).isUndefined()) {
    //
    x->x_interval = mica::MIR::Interval::withNotes (a, b);
    //
    }
    }
    //
    }
    
    micainterval_bang (x);
}

static void micainterval_anything (t_micainterval *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)micainterval_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void micainterval_set (t_micainterval *x, t_symbol *s, int argc, t_atom *argv)
{
    mica::Concept a (concept_fetch (atom_getSymbolAtIndex (0, argc, argv)));
    mica::Concept b (concept_fetch (atom_getSymbolAtIndex (1, argc, argv)));
    mica::Concept c (concept_fetch (atom_getSymbolAtIndex (2, argc, argv)));
    
    mica::MIR::Interval t;
    
    if (argc == 1) { t = mica::MIR::Interval::withName (a); }
    if (argc == 2) { t = mica::MIR::Interval::withName (a, b); }
    if (argc == 3) { t = mica::MIR::Interval::withName (a, b, c.getNumerator()); }
    
    if (t.isValid()) { x->x_interval = t; }
}

static void micainterval_apply (t_micainterval *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    t_atom *a = NULL;
    int i;
    
    PD_ATOMS_ALLOCA (a, argc);
    
    for (i = 0; i < argc; i++) {
        mica::Concept c (concept_fetch (atom_getSymbolAtIndex (i, argc, argv)));
        mica::Concept t (x->x_interval.appliedTo (c));
        t_symbol *tag = concept_tag (t);
        SET_SYMBOL (a + i, tag);
    }
    
    outlet_list (x->x_outlet, argc, a);
    
    PD_ATOMS_FREEA (a, argc);
    //
    }
}

static void micainterval_octaves (t_micainterval *x)
{
    /* LLVM analyzer seems to report false positives. */

    #ifndef __clang_analyzer__
    
    outlet_symbol (x->x_outlet, concept_tag (x->x_interval.getOctaves()));
    
    #endif
}

static void micainterval_distance (t_micainterval *x)
{
    /* LLVM analyzer seems to report false positives. */

    #ifndef __clang_analyzer__
    
    outlet_symbol (x->x_outlet, concept_tag (x->x_interval.getDistance()));
    
    #endif
}

static void micainterval_quality (t_micainterval *x)
{
    /* LLVM analyzer seems to report false positives. */

    #ifndef __clang_analyzer__
    
    outlet_symbol (x->x_outlet, concept_tag (x->x_interval.getQuality()));
    
    #endif
}

static void micainterval_direction (t_micainterval *x)
{
    /* LLVM analyzer seems to report false positives. */

    #ifndef __clang_analyzer__

    outlet_symbol (x->x_outlet, concept_tag (x->x_interval.getDirection()));
    
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *micainterval_new (t_symbol *s, int argc, t_atom *argv)
{
    t_micainterval *x = (t_micainterval *)pd_new (micainterval_class);
    
    /* Calling the placement constructor should not be necessary (but in case). */
    
    try { new (x) t_micainterval; } catch (...) { PD_BUG; }
    
    x->x_outlet = outlet_newAnything (cast_object (x));
    
    if (argc) {
    //
    t_symbol *t3 = NULL;
    t_symbol *t2 = NULL;
    t_symbol *t1 = NULL;
    
    if (IS_FLOAT (argv + argc - 1)) { t3 = concept_tagParsed (1, argv + argc - 1); argc--; }
    
    if (argc) {
    //
    t_symbol *last = atom_getSymbolAtIndex (argc - 1, argc, argv);
    
    if (last == sym_Ascending || last == sym_Descending) {
        t2 = concept_tagParsed (1, argv + argc - 1); argc--;
    }
    
    if (argc) {
    //
    t1 = concept_tagParsed (argc, argv);
    
    t_atom a[3];
    
    SET_SYMBOL (a + 0, t1);
    SET_SYMBOL (a + 1, t2);
    SET_SYMBOL (a + 2, t3);
    
    if (t1 && t2 && t3) { micainterval_set (x, sym_set, 3, a); }
    else if (t1 && t2)  { micainterval_set (x, sym_set, 2, a); }
    else {
        micainterval_set (x, sym_set, 1, a);
    }
    //
    }
    //
    }
    //
    }
    
    return x;
}

static void micainterval_free (t_micainterval *x)
{
    /* Ditto for the destructor. */
    
    x->~t_micainterval();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void micainterval_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_mica__space__interval,
            (t_newmethod)micainterval_new,
            (t_method)micainterval_free,
            sizeof (t_micainterval),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addBang (c, (t_method)micainterval_bang);
    class_addList (c, (t_method)micainterval_list);
    class_addAnything (c, (t_method)micainterval_anything);
    
    class_addMethod (c, (t_method)micainterval_set,       sym_set,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)micainterval_apply,     sym_apply,     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)micainterval_octaves,   sym_octaves,   A_NULL);
    class_addMethod (c, (t_method)micainterval_distance,  sym_distance,  A_NULL);
    class_addMethod (c, (t_method)micainterval_quality,   sym_quality,   A_NULL);
    class_addMethod (c, (t_method)micainterval_direction, sym_direction, A_NULL);
    
    class_setHelpName (c, sym_mica);
    
    micainterval_class = c;
}

void micainterval_destroy (void)
{
    class_free (micainterval_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#else

void micainterval_setup (void)
{
}

void micainterval_destroy (void)
{
}

#endif // PD_WITH_BELLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------


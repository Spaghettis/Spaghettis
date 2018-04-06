
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "x_atomoutlet.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_atom *atomoutlet_getAtom (t_atomoutlet *x)
{
    return &x->ao_atom;
}

t_inlet *atomoutlet_getInlet (t_atomoutlet *x)
{
    return x->ao_inlet;
}

t_outlet *atomoutlet_getOutlet (t_atomoutlet *x)
{
    return x->ao_outlet;
}

t_gpointer *atomoutlet_getPointer (t_atomoutlet *x)
{
    return &x->ao_gpointer;
}

t_atomtype atomoutlet_getType (t_atomoutlet *x)
{
    return atom_getType (&x->ao_atom);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int atomoutlet_isPointer (t_atomoutlet *x)
{
    return (atomoutlet_getType (x) == A_POINTER);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void atomoutlet_copyAtom (t_atomoutlet *x, t_atom *a)
{
    *a = *atomoutlet_getAtom (x);
}

t_error atomoutlet_setAtom (t_atomoutlet *x, t_atom *a)
{
    if (!atom_typesAreEquals (&x->ao_atom, a)) { return PD_ERROR; }
    else {
    //
    switch (atom_getType (&x->ao_atom)) {
        case A_FLOAT   : SET_FLOAT (&x->ao_atom,  GET_FLOAT (a));               break;
        case A_SYMBOL  : SET_SYMBOL (&x->ao_atom, GET_SYMBOL (a));              break;
        case A_POINTER : gpointer_setByCopy (&x->ao_gpointer, GET_POINTER (a)); break;
        default        : PD_BUG; return PD_ERROR;
    }
    //
    }
    
    return PD_ERROR_NONE;
}

int atomoutlet_isEqualToAtom (t_atomoutlet *x, t_atom *a)
{
    if (atom_typesAreEquals (&x->ao_atom, a)) {
    //
    if (IS_FLOAT (&x->ao_atom) && (GET_FLOAT (&x->ao_atom) == GET_FLOAT (a)))    { return 1; }
    if (IS_SYMBOL (&x->ao_atom) && (GET_SYMBOL (&x->ao_atom) == GET_SYMBOL (a))) { return 1; }
    
    PD_ASSERT (IS_FLOAT (&x->ao_atom) || IS_SYMBOL (&x->ao_atom));      /* A_POINTER not implemented yet. */
    //
    }
    
    return 0;
}

t_error atomoutlet_broadcastIfTypeMatch (t_atomoutlet *x, t_atom *a)
{
    if (!atom_typesAreEquals (&x->ao_atom, a)) { return PD_ERROR; }
    else {
    //
    switch (atomoutlet_getType (x)) {
        case A_FLOAT    : outlet_float (x->ao_outlet, GET_FLOAT (a));     break;
        case A_SYMBOL   : outlet_symbol (x->ao_outlet, GET_SYMBOL (a));   break;
        case A_POINTER  : outlet_pointer (x->ao_outlet, GET_POINTER (a)); break;
        default         : PD_BUG; return PD_ERROR;
    }
    //
    }
    
    return PD_ERROR_NONE;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void atomoutlet_init (t_atomoutlet *x)
{
    SET_FLOAT (&x->ao_atom, 0.0);
    gpointer_init (&x->ao_gpointer); 
    x->ao_outlet = NULL;
}

void atomoutlet_release (t_atomoutlet *x)
{
    gpointer_unset (&x->ao_gpointer); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void atomoutlet_makeFloat (t_atomoutlet *x, t_object *owner, int flags, t_symbol *type, t_float f)
{
    atomoutlet_init (x);
    SET_FLOAT (&x->ao_atom, f);
    if (flags & ATOMOUTLET_OUTLET) { x->ao_outlet = outlet_new (owner, type ? type : &s_float); }
    if (flags & ATOMOUTLET_INLET)  { x->ao_inlet  = inlet_newFloat (owner, ADDRESS_FLOAT (&x->ao_atom)); }
}

void atomoutlet_makeSymbol (t_atomoutlet *x, t_object *owner, int flags, t_symbol *type, t_symbol *s)
{
    atomoutlet_init (x);
    SET_SYMBOL (&x->ao_atom, s);
    if (flags & ATOMOUTLET_OUTLET) { x->ao_outlet = outlet_new (owner, type ? type : &s_symbol); }
    if (flags & ATOMOUTLET_INLET)  { x->ao_inlet  = inlet_newSymbol (owner, ADDRESS_SYMBOL (&x->ao_atom)); }
}

void atomoutlet_makePointer (t_atomoutlet *x, t_object *owner, int flags, t_symbol *type, t_gpointer *gp)
{
    atomoutlet_init (x);
    SET_POINTER (&x->ao_atom, &x->ao_gpointer);
    if (gp) { gpointer_setByCopy (&x->ao_gpointer, gp); }
    if (flags & ATOMOUTLET_OUTLET) { x->ao_outlet = outlet_new (owner, type ? type : &s_pointer); }
    if (flags & ATOMOUTLET_INLET)  { x->ao_inlet  = inlet_newPointer (owner, &x->ao_gpointer); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void atomoutlet_make (t_atomoutlet *x, t_object *owner, int flags, t_symbol *type, t_atom *a)
{
    if (IS_SYMBOL (a))       { atomoutlet_makeSymbol (x, owner, flags, type, GET_SYMBOL (a));   }
    else if (IS_POINTER (a)) { atomoutlet_makePointer (x, owner, flags, type, GET_POINTER (a)); }
    else {
        atomoutlet_makeFloat (x, owner, flags, type, atom_getFloat (a));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void atomoutlet_makeSymbolParsed (t_atomoutlet *x, t_object *owner, int flags, t_atom *a)
{
    t_symbol *t = atomoutlet_parseAbbreviated (atom_getSymbol (a));
    
    if (t == &s_) { t = &s_float; }
    
    atomoutlet_makeSymbol (x, owner, flags, t, t);
}

void atomoutlet_makeParsed (t_atomoutlet *x, t_object *owner, int flags, t_atom *a)
{
    t_symbol *t = atomoutlet_parseAbbreviated (atom_getSymbol (a));
    
    if (t == &s_pointer)     { atomoutlet_makePointer (x, owner, flags, NULL, NULL); }
    else if (t == &s_symbol) { atomoutlet_makeSymbol (x, owner, flags, NULL, t);     }
    else if (t == &s_float)  { atomoutlet_makeFloat (x, owner, flags, NULL, 0.0);    }
    else                     { atomoutlet_make (x, owner, flags, NULL, a);           }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

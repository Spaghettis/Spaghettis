
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *pipe_class;                         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _pipecallback {
    t_atom                  *h_atoms;
    t_gpointer              *h_gpointers;
    t_clock                 *h_clock;
    struct _pipe            *h_owner;
    struct _pipecallback    *h_next;
    } t_pipecallback;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _pipeoutlet {
    t_atom                  p_atom;
    t_gpointer              p_gpointer;
    t_outlet                *p_outlet;
    } t_pipeoutlet;

typedef struct _pipe {
    t_object                x_obj;                  /* Must be the first. */
    t_float                 x_delay;
    int                     x_size;
    t_pipeoutlet            *x_vector;
    t_pipecallback          *x_callbacks;
    } t_pipe;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void callback_free (t_pipecallback *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void callback_task (t_pipecallback *h)
{
    t_pipe *owner = h->h_owner;
    int i;
    
    if (owner->x_callbacks == h) { owner->x_callbacks = h->h_next; }
    else {
        t_pipecallback *m = NULL;
        t_pipecallback *n = NULL;
        for (m = owner->x_callbacks; n = m->h_next; m = n) { if (n == h) { m->h_next = n->h_next; break; } }
    }
    
    for (i = owner->x_size - 1; i >= 0; i--) {
    //
    switch (atom_getType (&owner->x_vector[i].p_atom)) {
        case A_FLOAT    : outlet_float (owner->x_vector[i].p_outlet, GET_FLOAT (h->h_atoms + i));     break;
        case A_SYMBOL   : outlet_symbol (owner->x_vector[i].p_outlet, GET_SYMBOL (h->h_atoms + i));   break;
        case A_POINTER  : outlet_pointer (owner->x_vector[i].p_outlet, GET_POINTER (h->h_atoms + i)); break;
    }
    //
    }
    
    callback_free (h);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void callback_new (t_pipe *x, int argc, t_atom *argv)
{
    t_pipecallback *h = (t_pipecallback *)PD_MEMORY_GET (sizeof (t_pipecallback));
    int i;
       
    h->h_atoms      = (t_atom *)PD_MEMORY_GET (x->x_size * sizeof (t_atom));
    h->h_gpointers  = (t_gpointer *)PD_MEMORY_GET (x->x_size * sizeof (t_gpointer));
    h->h_clock      = clock_new ((void *)h, (t_method)callback_task);
    h->h_owner      = x;
    h->h_next       = x->x_callbacks;
    
    for (i = 0; i < x->x_size; i++) {
        if (IS_POINTER (&x->x_vector[i].p_atom)) {
            SET_POINTER (&h->h_atoms[i], h->h_gpointers + i);
            gpointer_setByCopy (&x->x_vector[i].p_gpointer, h->h_gpointers + i);
        } else {
            h->h_atoms[i] = x->x_vector[i].p_atom;
        }
    }
    
    x->x_callbacks = h;

    clock_delay (h->h_clock, PD_MAX (x->x_delay, 0.0));
}

static void callback_free (t_pipecallback *h)
{
    int i;
    
    clock_free (h->h_clock);
    
    for (i = 0; i < h->h_owner->x_size; i++) { gpointer_unset (h->h_gpointers + i); }
    
    PD_MEMORY_FREE (h->h_gpointers);
    PD_MEMORY_FREE (h->h_atoms);
    PD_MEMORY_FREE (h);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void pipe_list (t_pipe *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    
    if (argc > x->x_size) {
    //
    if (IS_FLOAT (argv + x->x_size)) { x->x_delay = GET_FLOAT (argv + x->x_size); }
    else {
        error_invalid (sym_pipe, sym_delay);
        return;
    }
    //
    }
    
    argc = PD_MIN (argc, x->x_size);
    
    for (i = 0; i < argc; i++) {
        if (!atom_typesAreEqual (&x->x_vector[i].p_atom, argv + i)) {  
            error_mismatch (sym_pipe, sym_type); 
            return;
        }
    }

    for (i = 0; i < argc; i++) {
        switch (atom_getType (&x->x_vector[i].p_atom)) {
            case A_FLOAT   : SET_FLOAT (&x->x_vector[i].p_atom, atom_getFloat (argv + i));            break;
            case A_SYMBOL  : SET_SYMBOL (&x->x_vector[i].p_atom, atom_getSymbol (argv + i));          break;
            case A_POINTER : gpointer_setByCopy (GET_POINTER (argv + i), &x->x_vector[i].p_gpointer); break;
        }
    }
    
    callback_new (x, argc, argv);
}

static void pipe_flush (t_pipe *x)      /* FIFO. */
{
    while (x->x_callbacks) {
    //
    t_pipecallback *m = NULL;
    t_pipecallback *n = NULL;
    
    for (m = x->x_callbacks; n = m->h_next; m = n) { }
    
    callback_task (m);
    //
    }
}

static void pipe_clear (t_pipe *x)
{
    t_pipecallback *h = NULL; while (h = x->x_callbacks) { x->x_callbacks = h->h_next; callback_free (h); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *pipe_new (t_symbol *s, int argc, t_atom *argv)
{
    t_pipe *x = (t_pipe *)pd_new (pipe_class);
    int i;
    
    x->x_delay  = 0.0;
        
    if (argc) {
    //
    t_atom *a = argv + (argc - 1);
    if (IS_FLOAT (a)) { x->x_delay = GET_FLOAT (a); argc--; }
    //
    }

    x->x_size   = PD_MAX (1, argc);     
    x->x_vector = (t_pipeoutlet *)PD_MEMORY_GET (x->x_size * sizeof (t_pipeoutlet));

    for (i = 0; i < x->x_size; i++) {
    //
    gpointer_init (&x->x_vector[i].p_gpointer);
    SET_FLOAT (&x->x_vector[i].p_atom, 0.0);
    //
    }
    
    if (!argc) { x->x_vector[0].p_outlet = outlet_new (cast_object (x), &s_float); }
    else {
    //
    for (i = 0; i < argc; i++) {
    //
    t_atom *a = argv + i; t_symbol *t = atom_getSymbol (a);
    
    if (t == sym_f || IS_FLOAT (a)) {
        SET_FLOAT (&x->x_vector[i].p_atom, atom_getFloat (a));
        x->x_vector[i].p_outlet = outlet_new (cast_object (x), &s_float);
        if (i) { inlet_newFloat (cast_object (x), ADDRESS_FLOAT (&x->x_vector[i].p_atom)); }
        
    } else if (t == sym_s) {
        SET_SYMBOL (&x->x_vector[i].p_atom, &s_symbol);
        x->x_vector[i].p_outlet = outlet_new (cast_object (x), &s_symbol);
        if (i) { inlet_newSymbol (cast_object (x), ADDRESS_SYMBOL (&x->x_vector[i].p_atom)); }
        
    } else if (t == sym_p) {
        SET_POINTER (&x->x_vector[i].p_atom, &x->x_vector[i].p_gpointer);
        x->x_vector[i].p_outlet = outlet_new (cast_object (x), &s_pointer);
        if (i) { inlet_newPointer (cast_object (x), &x->x_vector[i].p_gpointer); }
        
    } else { error_invalid (sym_pipe, sym_type); }
    //
    }
    //
    }
    
    inlet_newFloat (cast_object (x), &x->x_delay);
    
    x->x_callbacks = NULL;

    return x;
}

static void pipe_free (t_pipe *x)
{
    int i;
    
    pipe_clear (x);
    
    for (i = 0; i < x->x_size; i++) { gpointer_unset (&x->x_vector[i].p_gpointer); }
    
    PD_MEMORY_FREE (x->x_vector);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pipe_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_pipe, 
            (t_newmethod)pipe_new,
            (t_method)pipe_free,
            sizeof (t_pipe),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addList (c, pipe_list);
    
    class_addMethod (c, (t_method)pipe_flush,   sym_flush,  A_NULL);
    class_addMethod (c, (t_method)pipe_clear,   sym_clear,  A_NULL);
    
    pipe_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

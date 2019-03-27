
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Changes by Thomas Musil IEM KUG Graz Austria 2001. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *message_class;                          /* Shared. */
static t_class *messageresponder_class;                 /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int atom_copyAtomsExpandedWithArguments (t_atom *, int, t_atom *, int, t_glist *, int, t_atom *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void messageresponder_bang (t_messageresponder *x)
{
    outlet_bang (x->mr_outlet);
}

static void messageresponder_float (t_messageresponder *x, t_float f)
{
    outlet_float (x->mr_outlet, f);
}

static void messageresponder_symbol (t_messageresponder *x, t_symbol *s)
{
    outlet_symbol (x->mr_outlet, s);
}

static void messageresponder_pointer (t_messageresponder *x, t_gpointer *gp)
{
    outlet_pointer (x->mr_outlet, gp);
}

static void messageresponder_list (t_messageresponder *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_list (x->mr_outlet, argc, argv);
}

static void messageresponder_anything (t_messageresponder *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_anything (x->mr_outlet, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void message_dirty (t_message *x)
{
    x->m_dirty = 1;
}

static t_buffer *message_getBuffer (t_message *x)
{
    if (x->m_dirty) {
    //
    buffer_clear (x->m_eval);
    buffer_appendBuffer (x->m_eval, object_getBuffer (cast_object (x)));
    
    {
        t_atom *a   = buffer_getAtoms (x->m_eval);
        int i, size = buffer_getSize (x->m_eval);
    
        for (i = 0; i < size; i++) {
            t_symbol *s = atom_getSymbolOrDollarSymbol (a + i);
            if (s != &s_ && string_contains (s->s_name, "$$")) {
                SET_SYMBOL (a + i, symbol_replaceDoubleDollar (s));
            }
        }
    }
    
    x->m_dirty = 0;
    //
    }
    
    return x->m_eval;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void message_eval (t_message *x, int argc, t_atom *argv)
{
    t_buffer *b = message_getBuffer (x);
    t_atom *a   = NULL;
    int n       = buffer_getSize (b);
    
    PD_ATOMS_ALLOCA (a, n);
    
    if (atom_copyAtomsExpandedWithArguments (buffer_getAtoms (b), n, a, n, x->m_owner, argc, argv)) {

        t_buffer *t = buffer_new();
        buffer_append (t, n, a);
        eval_buffer (t, cast_pd (&x->m_responder), argc, argv);
        buffer_free (t);
        
    } else {
        eval_buffer (b, cast_pd (&x->m_responder), argc, argv);
    }
    
    PD_ATOMS_FREEA (a, n);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void message_bang (t_message *x)
{
    message_eval (x, 0, NULL);
}

static void message_float (t_message *x, t_float f)
{
    t_atom a; SET_FLOAT (&a, f);
    
    message_eval (x, 1, &a);
}

static void message_symbol (t_message *x, t_symbol *s)
{
    t_atom a; SET_SYMBOL (&a, s);
    
    message_eval (x, 1, &a);
}

static void message_pointer (t_message *x, t_gpointer *gp)
{
    t_atom a; SET_POINTER (&a, gp);
    
    message_eval (x, 1, &a);
}

static void message_list (t_message *x, t_symbol *s, int argc, t_atom *argv)
{
    message_eval (x, argc, argv);
}

static void message_anything (t_message *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)message_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void message_click (t_message *x, t_symbol *s, int argc, t_atom *argv)
{
    if (glist_hasWindow (x->m_owner)) {         /* Not shown in GOP. */
    //
    if (glist_isOnScreen (x->m_owner)) {
    //
    t_box *text = box_fetch (x->m_owner, cast_object (x));
              
    gui_vAdd ("%s.c itemconfigure %sBORDER -width 5\n", 
                    glist_getTagAsString (x->m_owner), 
                    box_getTag (text));
    
    clock_delay (x->m_clock, 120.0);
    //
    }
    //
    }
    
    message_float (x, 0.0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void message_set (t_message *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_clear (object_getBuffer (cast_object (x)));
    buffer_append (object_getBuffer (cast_object (x)), argc, argv);
    buffer_reparseIfNeeded (object_getBuffer (cast_object (x)));    /* Copy and paste needs it. */
    box_retext (box_fetch (x->m_owner, cast_object (x)));
    glist_updateLinesForObject (x->m_owner, cast_object (x));
    message_dirty (x);
}

static void message_add (t_message *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_append (object_getBuffer (cast_object (x)), argc, argv);
    buffer_appendSemicolon (object_getBuffer (cast_object (x)));
    buffer_reparseIfNeeded (object_getBuffer (cast_object (x)));
    box_retext (box_fetch (x->m_owner, &x->m_obj));
    glist_updateLinesForObject (x->m_owner, cast_object (x));
    message_dirty (x);
}

static void message_append (t_message *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_append (object_getBuffer (cast_object (x)), argc, argv);
    buffer_reparseIfNeeded (object_getBuffer (cast_object (x)));
    box_retext (box_fetch (x->m_owner, cast_object (x)));
    glist_updateLinesForObject (x->m_owner, cast_object (x));
    message_dirty (x);
}

static void message_addComma (t_message *x)
{
    t_atom a; SET_COMMA (&a);
    
    buffer_appendAtom (object_getBuffer (cast_object (x)), &a);
    box_retext (box_fetch (x->m_owner, cast_object (x)));
    glist_updateLinesForObject (x->m_owner, cast_object (x));
    message_dirty (x);
}

static void message_addSemicolon (t_message *x)
{
    message_add (x, NULL, 0, NULL);
}

static void message_addDollar (t_message *x, t_float f)
{
    int n = PD_MAX (0, (int)f);
    t_atom a; SET_DOLLAR (&a, n);
    
    buffer_appendAtom (object_getBuffer (cast_object (x)), &a);
    box_retext (box_fetch (x->m_owner, cast_object (x)));
    glist_updateLinesForObject (x->m_owner, cast_object (x));
    message_dirty (x);
}

static void message_addDollarSymbol (t_message *x, t_symbol *s)
{
    t_atom a;
    
    char t[PD_STRING] = { 0 };
    string_sprintf (t, PD_STRING, "$%s", s->s_name);
    SET_DOLLARSYMBOL (&a, gensym (t));

    buffer_appendAtom (object_getBuffer (cast_object (x)), &a);
    box_retext (box_fetch (x->m_owner, cast_object (x)));
    glist_updateLinesForObject (x->m_owner, cast_object (x));
    message_dirty (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void message_taskTick (t_message *x)
{
    if (glist_hasWindow (x->m_owner)) {                     /* Not shown in GOP. */
    //
    if (glist_isOnScreen (x->m_owner)) {
    //
    t_box *text = box_fetch (x->m_owner, cast_object (x));
    
    gui_vAdd ("%s.c itemconfigure %sBORDER -width 1\n",
                    glist_getTagAsString (x->m_owner),
                    box_getTag (text));
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void message_makeObjectFile (t_message *x, int argc, t_atom *argv)
{
    t_buffer *t = buffer_new(); if (argc > 2) { buffer_deserialize (t, argc - 2, argv + 2); }
    
    object_setBuffer (cast_object (x), t);
    object_setX (cast_object (x), atom_getFloatAtIndex (0, argc, argv));
    object_setY (cast_object (x), atom_getFloatAtIndex (1, argc, argv));
    object_setWidth (cast_object (x), 0);
    object_setType (cast_object (x), TYPE_MESSAGE);
    
    glist_objectAdd (x->m_owner, cast_gobj (x));
}

static void message_makeObjectMenu (t_message *x, int argc, t_atom *argv)
{
    if (glist_isOnScreen (x->m_owner)) {
    //
    int a = instance_getDefaultX (x->m_owner);
    int b = instance_getDefaultY (x->m_owner);
    
    glist_deselectAll (x->m_owner);

    if (atom_getSymbolAtIndex (0, argc, argv) == sym_menu) {
    //
    t_point pt = glist_getPositionForNewObject (x->m_owner);
    
    a = point_getX (&pt);
    b = point_getY (&pt);
    //
    }
    
    object_setBuffer (cast_object (x), buffer_new());
    object_setSnappedX (cast_object (x), a);
    object_setSnappedY (cast_object (x), b);
    object_setWidth (cast_object (x), 0);
    object_setType (cast_object (x), TYPE_MESSAGE);
    
    glist_objectAdd (x->m_owner, cast_gobj (x));
    glist_objectSelect (x->m_owner, cast_gobj (x));
    gobj_activated (cast_gobj (x), x->m_owner, 1);
    //
    }
}

void message_makeObject (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    t_message *x = (t_message *)pd_new (message_class);
    
    x->m_responder.mr_pd     = messageresponder_class;
    x->m_responder.mr_outlet = outlet_newAnything (cast_object (x));
    x->m_eval                = buffer_new();
    x->m_owner               = glist;
    x->m_clock               = clock_new ((void *)x, (t_method)message_taskTick);
    
    if (argc > 1) { message_makeObjectFile (x, argc, argv); }
    else {
        message_makeObjectMenu (x, argc, argv);
    }
    
    message_dirty (x);
}

static void message_free (t_message *x)
{
    buffer_free (x->m_eval);
    clock_free (x->m_clock);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void message_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_message,
            NULL,
            (t_method)message_free,
            sizeof (t_message),
            CLASS_DEFAULT,
            A_NULL);
    
    class_addBang (c, (t_method)message_bang);
    class_addFloat (c, (t_method)message_float);
    class_addSymbol (c, (t_method)message_symbol);
    class_addPointer (c, (t_method)message_pointer);
    class_addList (c, (t_method)message_list);
    class_addAnything (c, (t_method)message_anything);

    class_addClick (c, (t_method)message_click);
        
    class_addMethod (c, (t_method)message_set,              sym_set,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)message_add,              sym_add,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)message_append,           sym_append,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)message_addComma,         sym_addcomma,           A_NULL);
    class_addMethod (c, (t_method)message_addSemicolon,     sym_addsemicolon,       A_NULL);
    class_addMethod (c, (t_method)message_addDollar,        sym_adddollar,          A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)message_addDollarSymbol,  sym_adddollarsymbol,    A_SYMBOL, A_NULL);

    message_class = c;
    
    c = class_new (sym_messageresponder,
            NULL,
            NULL,
            0,
            CLASS_ABSTRACT,
            A_NULL);
            
    class_addBang (c, (t_method)messageresponder_bang);
    class_addFloat (c, (t_method)messageresponder_float);
    class_addSymbol (c, (t_method)messageresponder_symbol);
    class_addPointer (c, (t_method)messageresponder_pointer);
    class_addList (c, (t_method)messageresponder_list);
    class_addAnything (c, (t_method)messageresponder_anything);
    
    messageresponder_class = c;
}

void message_destroy (void)
{
    class_free (message_class);
    class_free (messageresponder_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

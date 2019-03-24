
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "x_text.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *texttolist_class;           /* Shared. */
static t_class *textfromlist_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _texttolist {
    t_textclient    x_textclient;           /* Must be the first. */
    t_outlet        *x_outlet;
    } t_texttolist;

typedef struct _textfromlist {
    t_textclient    x_textclient;           /* Must be the first. */
    } t_textfromlist;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *texttolist_new (t_symbol *s, int argc, t_atom *argv)
{
    t_texttolist *x = (t_texttolist *)pd_new (texttolist_class);
    
    t_error err = textclient_init (&x->x_textclient, &argc, &argv);     /* It may consume arguments. */
    
    if (!err) {
    
        x->x_outlet = outlet_newList (cast_object (x));
        
        if (argc) { warning_unusedArguments (sym_text__space__tolist, argc, argv); }
        
        inlet_newSymbol (cast_object (x), TEXTCLIENT_NAME (&x->x_textclient));
    
    } else {
        
        error_invalidArguments (sym_text__space__tolist, argc, argv); pd_free (cast_pd (x)); x = NULL;
    }
    
    warning_deprecatedObject (sym_text__space__tolist);
    
    return x;
}

static void texttolist_bang (t_texttolist *x)
{
    t_buffer *b = textclient_fetchBuffer (&x->x_textclient);
    
    if (b) {
        t_buffer *t = buffer_new();
        buffer_serialize (t, b);
        outlet_list (x->x_outlet, buffer_getSize (t), buffer_getAtoms (t));
        buffer_free (t);
        
    } else { error_undefined (sym_text__space__tolist, sym_text); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *textfromlist_new (t_symbol *s, int argc, t_atom *argv)
{
    t_textfromlist *x = (t_textfromlist *)pd_new (textfromlist_class);
    
    t_error err = textclient_init (&x->x_textclient, &argc, &argv);     /* It may consume arguments. */
    
    if (!err) {
    
        if (argc) { warning_unusedArguments (sym_text__space__fromlist, argc, argv); }
        
        inlet_newSymbol (cast_object (x), TEXTCLIENT_NAME (&x->x_textclient));
        
    } else {
    
        error_invalidArguments (sym_text__space__fromlist, argc, argv);
        pd_free (cast_pd (x)); x = NULL;
    }
    
    warning_deprecatedObject (sym_text__space__fromlist);
    
    return x;
}

static void textfromlist_list (t_textfromlist *x, t_symbol *s, int argc, t_atom *argv)
{
    t_buffer *b = textclient_fetchBuffer (&x->x_textclient);
    
    if (b) {
        buffer_clear (b);
        buffer_deserialize (b, argc, argv);
        textclient_update (&x->x_textclient);
    
    } else { error_undefined (sym_text__space__fromlist, sym_text); } 
}

static void textfromlist_anything (t_textfromlist *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)textfromlist_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void textlist_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_text__space__tolist,
            (t_newmethod)texttolist_new,
            NULL,
            sizeof (t_texttolist),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addBang (c, (t_method)texttolist_bang);
    class_setHelpName (c, sym_text);

    texttolist_class = c;
    
    c = class_new (sym_text__space__fromlist,
            (t_newmethod)textfromlist_new,
            NULL,
            sizeof (t_textfromlist),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addList (c, (t_method)textfromlist_list);
    class_addAnything (c, (t_method)textfromlist_anything);
    
    class_setHelpName (c, sym_text);
    
    textfromlist_class = c;
}

void textlist_destroy (void)
{
    class_free (texttolist_class);
    class_free (textfromlist_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------


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
#include "m_alloca.h"
#include "g_graphics.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* ---------------- text_tolist object - output text as a list ----------- */
t_class *text_tolist_class;

#define t_text_tolist t_text_client

void *text_tolist_new(t_symbol *s, int argc, t_atom *argv)
{
    t_text_tolist *x = (t_text_tolist *)pd_new(text_tolist_class);
    outlet_new(&x->tc_obj, &s_list);
    text_client_argparse(x, &argc, &argv, "text tolist");
    if (argc)
    {
        post("warning: text tolist ignoring extra argument: ");
        post_atoms(argc, argv);
    }
    if (x->tc_struct)
        inlet_newPointer(&x->tc_obj, &x->tc_gp);
    else inlet_newSymbol(&x->tc_obj, &x->tc_sym);
    return (x);
}

static void text_tolist_bang(t_text_tolist *x)
{
    t_buffer *b = text_client_getbuf(x), *b2;
    int n, i, cnt = 0;
    t_atom *vec;
    if (!b)
       return;
    b2 = buffer_new();
    buffer_serialize(b2, b);
    outlet_list(x->tc_obj.te_outlet, 0, buffer_size(b2), buffer_atoms(b2));
    buffer_free(b2);
}

/* ------------- text_fromlist object - set text from a list -------- */
t_class *text_fromlist_class;

#define t_text_fromlist t_text_client

void *text_fromlist_new(t_symbol *s, int argc, t_atom *argv)
{
    t_text_fromlist *x = (t_text_fromlist *)pd_new(text_fromlist_class);
    text_client_argparse(x, &argc, &argv, "text fromlist");
    if (argc)
    {
        post("warning: text fromlist ignoring extra argument: ");
        post_atoms(argc, argv);
    }
    if (x->tc_struct)
        inlet_newPointer(&x->tc_obj, &x->tc_gp);
    else inlet_newSymbol(&x->tc_obj, &x->tc_sym);
    return (x);
}

static void text_fromlist_list(t_text_fromlist *x,
    t_symbol *s, int argc, t_atom *argv)
{
    t_buffer *b = text_client_getbuf(x);
    if (!b)
       return;
    buffer_reset(b);
    buffer_deserialize(b, argc, argv);
    text_client_senditup(x);
}

void textlist_setup (void)
{
    text_tolist_class = class_new(sym_text__space__tolist,
        (t_newmethod)text_tolist_new, (t_method)text_client_free,
            sizeof(t_text_tolist), 0, A_GIMME, 0);
    class_addBang(text_tolist_class, text_tolist_bang);
    class_setHelpName(text_tolist_class, sym_text);

    text_fromlist_class = class_new(sym_text__space__fromlist,
        (t_newmethod)text_fromlist_new, (t_method)text_client_free,
            sizeof(t_text_fromlist), 0, A_GIMME, 0);
    class_addList(text_fromlist_class, text_fromlist_list);
    class_setHelpName(text_fromlist_class, sym_text);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

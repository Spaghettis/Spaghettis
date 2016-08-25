
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
#include "g_graphics.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd     *pd_newest;
extern t_pd     pd_canvasMaker;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class         *textdefine_class;                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _textdefine {
    t_textbuffer    x_textbuffer;                   /* Must be the first. */
    t_gpointer      x_gpointer;
    int             x_keep;
    t_symbol        *x_name;
    t_scalar        *x_scalar;
    t_outlet        *x_outlet;
    } t_textdefine;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void textdefine_initialize (void)
{
    static char *textTemplateFile = 
        "canvas 0 0 458 153 10;\n"
        "#X obj 43 31 struct text float x float y text t;\n";

    t_buffer *b = buffer_new();
    
    canvas_setActiveFileNameAndDirectory (sym__texttemplate, sym___dot__);
    buffer_withStringUnzeroed (b, textTemplateFile, strlen (textTemplateFile));
    buffer_eval (b, &pd_canvasMaker, 0, NULL);
    pd_vMessage (s__X.s_thing, sym__pop, "i", 0);
    
    canvas_setActiveFileNameAndDirectory (&s_, &s_);
    
    buffer_free (b);  
}

void textdefine_release (void)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void textdefine_clear(t_textdefine *x)
{
    buffer_reset(textbuffer_getBuffer (&x->x_textbuffer));
    textbuffer_update(&x->x_textbuffer);
}

/*********  random utility function to find a binbuf in a datum */

t_buffer *pointertobinbuf(t_pd *x, t_gpointer *gp, t_symbol *s,
    const char *fname)
{
    t_symbol *templatesym = gpointer_getTemplateIdentifier(gp), *arraytype;
    t_template *template;
    int onset, type;
    t_buffer *b;
    t_word *vec;
    if (!templatesym)
    {
        post_error ("%s: bad pointer", fname);
        return (0);
    }
    if (!(template = template_findByIdentifier(templatesym)))
    {
        post_error ("%s: couldn't find template %s", fname,
            templatesym->s_name);
        return (0);
    }
    /* Remove template_findField ASAP !!! */
    if (!template_findField(template, s, &onset, &type, &arraytype))    
    {
        post_error ("%s: %s.%s: no such field", fname,
            templatesym->s_name, s->s_name);
        return (0);
    }
    if (type != DATA_TEXT)
    {
        post_error ("%s: %s.%s: not a list", fname,
            templatesym->s_name, s->s_name);
        return (0);
    }
    vec = gpointer_getData (gp);
    return (vec[onset].w_buffer);
}


    /* these are unused; they copy text from this object to and from a text
        field in a scalar. */
static void textdefine_frompointer(t_textdefine *x, t_gpointer *gp,
    t_symbol *s)
{
    t_buffer *b = pointertobinbuf(cast_pd (x),
        gp, s, "text_frompointer");
    if (b)
    {
        buffer_reset(textbuffer_getBuffer (&x->x_textbuffer));
        buffer_appendBuffer(textbuffer_getBuffer (&x->x_textbuffer), b);
    } 
}

static void textdefine_topointer(t_textdefine *x, t_gpointer *gp, t_symbol *s)
{
    t_buffer *b = pointertobinbuf(cast_pd (x), gp, s, "text_topointer");
    if (b)
    {
        buffer_reset(b);
        buffer_appendBuffer (b, textbuffer_getBuffer (&x->x_textbuffer));
        gpointer_redraw (gp);
    } 
}

    /* bang: output a pointer to a struct containing this text */
void textdefine_bang(t_textdefine *x)
{
    gpointer_setAsScalar(&x->x_gpointer, textbuffer_getView (&x->x_textbuffer), x->x_scalar);
    outlet_pointer(x->x_outlet, &x->x_gpointer);
}

    /* set from a list */
void textdefine_set(t_textdefine *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_deserialize(textbuffer_getBuffer (&x->x_textbuffer), argc, argv);
    textbuffer_update(&x->x_textbuffer);
}


static void textdefine_save(t_gobj *z, t_buffer *bb)
{
    t_textdefine *x = (t_textdefine *)z;
    buffer_vAppend(bb, "ssff", sym___hash__X, sym_obj,
        (float)cast_object (x)->te_xCoordinate, (float)cast_object (x)->te_yCoordinate);
    buffer_serialize(bb, cast_object (x)->te_buffer);
    buffer_appendSemicolon(bb);
    if (x->x_keep)
    {
        buffer_vAppend(bb, "ss", sym___hash__A, sym_set);
        buffer_serialize(bb, textbuffer_getBuffer (&x->x_textbuffer));
        buffer_appendSemicolon(bb);
    }
    object_saveWidth(cast_object (x), bb);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *textdefine_newObject(t_symbol *s, int argc, t_atom *argv)
{
    t_textdefine *x = (t_textdefine *)pd_new(textdefine_class);
    t_symbol *asym = sym___hash__A;
    x->x_keep = 0;
    x->x_name = &s_;
    while (argc && argv->a_type == A_SYMBOL &&
        *argv->a_w.w_symbol->s_name == '-')
    {
        if (!strcmp(argv->a_w.w_symbol->s_name, "-k"))
            x->x_keep = 1;
        else
        {
            post_error ("text define: unknown flag ...");
            post_atoms(argc, argv);
        }
        argc--; argv++;
    }
    if (argc && argv->a_type == A_SYMBOL)
    {
        pd_bind(cast_pd (x), argv->a_w.w_symbol);
        x->x_name = argv->a_w.w_symbol;
        argc--; argv++;
    }
    if (argc)
    {
        post("warning: text define ignoring extra argument: ");
        post_atoms(argc, argv);
    }
    textbuffer_init (&x->x_textbuffer);
        /* set up a scalar and a pointer to it that we can output */
    x->x_scalar = scalar_new(canvas_getCurrent(), sym___TEMPLATE__text);
    buffer_free(x->x_scalar->sc_vector[2].w_buffer);                                /* Encaspulate ASAP. */
    x->x_scalar->sc_vector[2].w_buffer = textbuffer_getBuffer (&x->x_textbuffer);   /* Encaspulate ASAP. */
    x->x_outlet = outlet_new(cast_object (x), &s_pointer);
    gpointer_init(&x->x_gpointer);
           /* bashily unbind #A -- this would create garbage if #A were
           multiply bound but we believe in this context it's at most
           bound to whichever textdefine or array was created most recently */
    asym->s_thing = 0;
        /* and now bind #A to us to receive following messages in the
        saved file or copy buffer */
    pd_bind(cast_pd (x), asym); 
    return (x);
}

static void *textdefine_new (t_symbol *s, int argc, t_atom *argv)
{
    pd_newest = NULL;
    
    if (!argc || !IS_SYMBOL (argv)) { pd_newest = textdefine_newObject (s, argc, argv); }
    else {
    //
    t_symbol *t = atom_getSymbol (argv);
    
    if (t == sym_d || t == sym_define)  { pd_newest = textdefine_newObject (s,  argc - 1, argv + 1); }
    else if (t == sym_get)              { pd_newest = textget_new (s,           argc - 1, argv + 1); }
    else if (t == sym_set)              { pd_newest = textset_new (s,           argc - 1, argv + 1); }
    else if (t == sym_size)             { pd_newest = textsize_new (s,          argc - 1, argv + 1); }
    else if (t == sym_tolist)           { pd_newest = texttolist_new (s,        argc - 1, argv + 1); }
    else if (t == sym_fromlist)         { pd_newest = textfromlist_new (s,      argc - 1, argv + 1); }
    else if (t == sym_search)           { pd_newest = textsearch_new (s,        argc - 1, argv + 1); }
    else if (t == sym_sequence)         { pd_newest = textsequence_new (s,      argc - 1, argv + 1); }
    else {
        post_error (PD_TRANSLATE ("text: unknown function"));
    }
    //
    }
    
    return pd_newest;
}

static void textdefine_free(t_textdefine *x)
{
    textbuffer_free (&x->x_textbuffer);
    
    if (x->x_name != &s_) { pd_unbind (cast_pd (x), x->x_name); }
    
    gpointer_unset (&x->x_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void textdefine_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_text__space__define,
            (t_newmethod)textdefine_new,
            (t_method)textdefine_free,
            sizeof (t_textdefine),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
        
    class_addBang (c, textdefine_bang);
    class_addClick (c, textbuffer_click);
            
    class_addMethod (c, (t_method)textbuffer_close,     sym_close,      A_NULL);
    class_addMethod (c, (t_method)textbuffer_add,       sym__addline,   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)textbuffer_write,     sym_write,      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)textbuffer_read,      sym_read,       A_GIMME, A_NULL);
    
    class_addMethod (c, (t_method)textdefine_set,       sym_set,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)textdefine_clear,     sym_clear,      A_NULL);
        
    class_setSaveFunction (c, textdefine_save);
    class_setHelpName (c, sym_text);

    class_addCreator ((t_newmethod)textdefine_new, sym_text, A_GIMME, A_NULL);
    
    textdefine_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

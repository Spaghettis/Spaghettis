
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *scalar_class;
extern t_class *canvas_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *ptrobj_class;

typedef struct 
{
    t_symbol *to_type;
    t_outlet *to_outlet;
} t_typedout;

typedef struct _ptrobj
{
    t_object x_obj;
    t_gpointer x_gp;
    t_typedout *x_typedout;
    int x_ntypedout;
    t_outlet *x_otherout;
    t_outlet *x_bangout;
} t_ptrobj;

static void *ptrobj_new(t_symbol *classname, int argc, t_atom *argv)
{
    t_ptrobj *x = (t_ptrobj *)pd_new(ptrobj_class);
    t_typedout *to;
    int n;
    gpointer_init(&x->x_gp);
    x->x_typedout = to = (t_typedout *)PD_MEMORY_GET(argc * sizeof (*to));
    x->x_ntypedout = n = argc;
    for (; n--; to++)
    {
        to->to_outlet = outlet_new(&x->x_obj, &s_pointer);
        to->to_type = template_makeIdentifierWithWildcard(atom_getSymbol(argv++));
    }
    x->x_otherout = outlet_new(&x->x_obj, &s_pointer);
    x->x_bangout = outlet_new(&x->x_obj, &s_bang);
    inlet_newPointer(&x->x_obj, &x->x_gp);
    return (x);
}

static void ptrobj_traverse(t_ptrobj *x, t_symbol *s)
{
    t_glist *glist = (t_glist *)pd_findByClass(s, canvas_class);
    if (glist) gpointer_setAsScalar(&x->x_gp, glist, 0);
    else { post_error (x, "pointer: list '%s' not found", s->s_name); }
}

static void ptrobj_vnext(t_ptrobj *x, t_float f)
{
    t_gobj *gobj;
    t_gpointer *gp = &x->x_gp;
    t_glist *glist;
    int wantselected = (f != 0);

    if (!gpointer_isSet (gp))
    {
        post_error ("ptrobj_next: no current pointer");
        return;
    }
    if (!gpointer_isScalar (gp)) {
        post_error ("ptrobj_next: lists only, not arrays");
        return;
    }
    
    glist = gpointer_getParentGlist (gp);
    if (glist->gl_uniqueIdentifier != gpointer_getUniqueIdentifier (gp))    /* isValid ? */
    {
        post_error ("ptrobj_next: stale pointer");
        return;
    }
    
    if (wantselected && !canvas_isMapped(glist))
    {
        post_error ("ptrobj_vnext: next-selected only works for a visible window");
        return;
    }
    
    t_scalar *scalar = gpointer_getScalar (gp);
    gobj = cast_gobj (scalar);
    
    if (!gobj) gobj = glist->gl_graphics;
    else gobj = gobj->g_next;
    while (gobj && ((pd_class(&gobj->g_pd) != scalar_class) ||
        (wantselected && !canvas_isObjectSelected(glist, gobj))))
            gobj = gobj->g_next;
    
    if (gobj)
    {
        t_typedout *to;
        int n;
        t_scalar *sc = (t_scalar *)gobj;
        t_symbol *templatesym = sc->sc_templateIdentifier;

        gpointer_setAsScalar (gp, glist, sc);
        // gp->gp_un.gp_scalar = sc; 
        for (n = x->x_ntypedout, to = x->x_typedout; n--; to++)
        {
            if (to->to_type == templatesym)
            {
                outlet_pointer(to->to_outlet, &x->x_gp);
                return;
            }
        }
        outlet_pointer(x->x_otherout, &x->x_gp);
    }
    else
    {
        gpointer_unset(gp);
        outlet_bang(x->x_bangout);
    }
}

static void ptrobj_next(t_ptrobj *x)
{
    ptrobj_vnext(x, 0);
}

    /* send a message to the window containing the object pointed to */
static void ptrobj_sendwindow(t_ptrobj *x, t_symbol *s, int argc, t_atom *argv)
{
    t_scalar *sc;
    t_symbol *templatesym;
    int n;
    t_typedout *to;
    t_glist *glist;
    t_pd *canvas;
    if (!gpointer_isValidOrHead (&x->x_gp))
    {
        post_error ("send-window: empty pointer");
        return;
    }
    
    glist = gpointer_getView (&x->x_gp);
    canvas = (t_pd *)canvas_getView(glist);
    if (argc && argv->a_type == A_SYMBOL)
        pd_message(canvas, argv->a_w.w_symbol, argc-1, argv+1);
    else { post_error ("send-window: no message?"); }
}


    /* send the pointer to the named object */
static void ptrobj_send(t_ptrobj *x, t_symbol *s)
{
    if (!s->s_thing)
        post_error ("%s: no such object", s->s_name);
    else if (!gpointer_isValidOrHead (&x->x_gp))
        post_error ("pointer_send: empty pointer");
    else pd_pointer(s->s_thing, &x->x_gp);
}

static void ptrobj_bang(t_ptrobj *x)
{
    t_symbol *templatesym;
    int n;
    t_typedout *to;
    if (!gpointer_isValidOrHead(&x->x_gp))
    {
        post_error ("pointer_bang: empty pointer");
        return;
    }
    templatesym = gpointer_getTemplateIdentifier(&x->x_gp);
    for (n = x->x_ntypedout, to = x->x_typedout; n--; to++)
    {
        if (to->to_type == templatesym)
        {
            outlet_pointer(to->to_outlet, &x->x_gp);
            return;
        }
    }
    outlet_pointer(x->x_otherout, &x->x_gp);
}


static void ptrobj_pointer(t_ptrobj *x, t_gpointer *gp)
{
    //gpointer_unset(&x->x_gp);
    gpointer_setByCopy(gp, &x->x_gp);
    ptrobj_bang(x);
}


static void ptrobj_rewind(t_ptrobj *x)
{
    t_scalar *sc;
    t_symbol *templatesym;
    int n;
    t_typedout *to;
    t_glist *glist;
    t_pd *canvas;
    //t_gmaster *gs;
    if (!gpointer_isValidOrHead(&x->x_gp))
    {
        post_error ("pointer_rewind: empty pointer");
        return;
    }

    if (!gpointer_isScalar (&x->x_gp)) {
        post_error ("pointer_rewind: sorry, unavailable for arrays");
        return;
    }
    glist = gpointer_getParentGlist (&x->x_gp);
    gpointer_setAsScalar(&x->x_gp, glist, 0);
    ptrobj_bang(x);
}

static void ptrobj_free(t_ptrobj *x)
{
    PD_MEMORY_FREE(x->x_typedout);
    gpointer_unset(&x->x_gp);
}

void ptrobj_setup(void)
{
    ptrobj_class = class_new(sym_pointer, (t_newmethod)ptrobj_new,
        (t_method)ptrobj_free, sizeof(t_ptrobj), 0, A_GIMME, 0);
    class_addMethod(ptrobj_class, (t_method)ptrobj_next, sym_next, 0); 
    class_addMethod(ptrobj_class, (t_method)ptrobj_send, sym_send, 
        A_SYMBOL, 0); 
    class_addMethod(ptrobj_class, (t_method)ptrobj_traverse, sym_traverse,
        A_SYMBOL, 0); 
    class_addMethod(ptrobj_class, (t_method)ptrobj_vnext, sym_vnext, /* LEGACY !!! */
        A_DEFFLOAT, 0); 
    class_addMethod(ptrobj_class, (t_method)ptrobj_sendwindow,
        sym_send__dash__window, A_GIMME, 0);                    /* LEGACY !!! */
    class_addMethod(ptrobj_class, (t_method)ptrobj_rewind,
        sym_rewind, 0); 
    class_addPointer(ptrobj_class, ptrobj_pointer); 
    class_addBang(ptrobj_class, ptrobj_bang); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_class *get_class;

typedef struct _getvariable
{
    t_symbol *gv_sym;
    t_outlet *gv_outlet;
} t_getvariable;

typedef struct _get
{
    t_object x_obj;
    t_symbol *x_templatesym;
    int x_nout;
    t_getvariable *x_variables;
} t_get;

static void *get_new(t_symbol *why, int argc, t_atom *argv)
{
    t_get *x = (t_get *)pd_new(get_class);
    int varcount, i;
    t_atom at, *varvec;
    t_getvariable *sp;

    x->x_templatesym = template_makeIdentifierWithWildcard(atom_getSymbolAtIndex(0, argc, argv));
    if (argc < 2)
    {
        varcount = 1;
        varvec = &at;
        SET_SYMBOL(&at, &s_);
    }
    else varcount = argc - 1, varvec = argv + 1;
    x->x_variables
        = (t_getvariable *)PD_MEMORY_GET(varcount * sizeof (*x->x_variables));
    x->x_nout = varcount;
    for (i = 0, sp = x->x_variables; i < varcount; i++, sp++)
    {
        sp->gv_sym = atom_getSymbolAtIndex(i, varcount, varvec);
        sp->gv_outlet = outlet_new(&x->x_obj, 0);
            /* LATER connect with the template and set the outlet's type
            correctly.  We can't yet guarantee that the template is there
            before we hit this routine. */
    }
    return (x);
}

static void get_set(t_get *x, t_symbol *templatesym, t_symbol *field)
{
    if (x->x_nout != 1)
        post_error ("get: cannot set multiple fields.");
    else
    {
        x->x_templatesym = template_makeIdentifierWithWildcard(templatesym); 
        x->x_variables->gv_sym = field;
    }
}

static void get_pointer(t_get *x, t_gpointer *gp)
{
    int nitems = x->x_nout, i;
    t_symbol *templatesym;
    t_template *template;
    t_word *vec; 
    t_getvariable *vp;

    if (!gpointer_isValid(gp))
    {
        post_error ("get: stale or empty pointer");
        return;
    }
    if (*x->x_templatesym->s_name)
    {
        if ((templatesym = x->x_templatesym) != gpointer_getTemplateIdentifier(gp))
        {
            post_error ("get %s: got wrong template (%s)",
                templatesym->s_name, gpointer_getTemplateIdentifier(gp)->s_name);
            return;
        } 
    }
    else templatesym = gpointer_getTemplateIdentifier(gp);
    if (!(template = template_findByIdentifier(templatesym)))
    {
        post_error ("get: couldn't find template %s", templatesym->s_name);
        return;
    }
    vec = gpointer_getData (gp);
    for (i = nitems - 1, vp = x->x_variables + i; i >= 0; i--, vp--)
    {
        int onset, type;
        t_symbol *arraytype;
        if (template_findField(template, vp->gv_sym, &onset, &type, &arraytype))
        {
            if (type == DATA_FLOAT) {
                outlet_float(vp->gv_outlet,
                    *(t_float *)(((char *)vec) + onset));
            } else if (type == DATA_SYMBOL) {
                outlet_symbol(vp->gv_outlet,
                    *(t_symbol **)(((char *)vec) + onset));
            } else {
                // post_error ("get: %s.%s is not a number or symbol", template->tp_templateIdentifier->s_name, vp->gv_sym->s_name);
            }
        } else {
            // post_error ("get: %s.%s: no such field", template->tp_templateIdentifier->s_name, vp->gv_sym->s_name);
        }
    }
}

static void get_free(t_get *x)
{
    PD_MEMORY_FREE(x->x_variables);
}

void get_setup(void)
{
    get_class = class_new (sym_get, (t_newmethod)get_new,
        (t_method)get_free, sizeof(t_get), 0, A_GIMME, 0);
    class_addPointer(get_class, get_pointer); 
    class_addMethod(get_class, (t_method)get_set, sym_set,
        A_SYMBOL, A_SYMBOL, 0); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_class *set_class;

typedef struct _setvariable
{
    t_symbol *gv_sym;
    union word gv_w;
} t_setvariable;

typedef struct _set
{
    t_object x_obj;
    t_gpointer x_gp;
    t_symbol *x_templatesym;
    int x_nin;
    int x_issymbol;
    t_setvariable *x_variables;
} t_set;

static void *set_new(t_symbol *why, int argc, t_atom *argv)
{
    t_set *x = (t_set *)pd_new(set_class);
    int i, varcount;
    t_setvariable *sp;
    t_atom at, *varvec;
    if (argc && (argv[0].a_type == A_SYMBOL) &&
        !strcmp(argv[0].a_w.w_symbol->s_name, "-symbol"))
    {
        x->x_issymbol = 1;
        argc--;
        argv++;
    }
    else x->x_issymbol = 0;
    x->x_templatesym = template_makeIdentifierWithWildcard(atom_getSymbolAtIndex(0, argc, argv));
    if (argc < 2)
    {
        varcount = 1;
        varvec = &at;
        SET_SYMBOL(&at, &s_);
    }
    else varcount = argc - 1, varvec = argv + 1;
    x->x_variables
        = (t_setvariable *)PD_MEMORY_GET(varcount * sizeof (*x->x_variables));
    x->x_nin = varcount;
    for (i = 0, sp = x->x_variables; i < varcount; i++, sp++)
    {
        sp->gv_sym = atom_getSymbolAtIndex(i, varcount, varvec);
        if (x->x_issymbol)
            sp->gv_w.w_symbol = &s_;
        else sp->gv_w.w_float = 0;
        if (i)
        {
            if (x->x_issymbol)
                inlet_newSymbol(&x->x_obj, &sp->gv_w.w_symbol);
            else inlet_newFloat(&x->x_obj, &sp->gv_w.w_float);
        }
    }
    inlet_newPointer(&x->x_obj, &x->x_gp);
    gpointer_init(&x->x_gp);
    return (x);
}

static void set_set(t_set *x, t_symbol *templatesym, t_symbol *field)
{
    if (x->x_nin != 1)
        post_error ("set: cannot set multiple fields.");
    else
    {
       x->x_templatesym = template_makeIdentifierWithWildcard(templatesym); 
       x->x_variables->gv_sym = field;
       if (x->x_issymbol)
           x->x_variables->gv_w.w_symbol = &s_;
       else
           x->x_variables->gv_w.w_float = 0;
    }
}

static void set_bang(t_set *x)
{
    int nitems = x->x_nin, i;
    t_symbol *templatesym;
    t_template *template;
    t_setvariable *vp;
    t_gpointer *gp = &x->x_gp;
    t_word *vec;
    if (!gpointer_isValid(gp))
    {
        post_error ("set: empty pointer");
        return;
    }
    if (*x->x_templatesym->s_name)
    {
        if ((templatesym = x->x_templatesym) != gpointer_getTemplateIdentifier(gp))
        {
            post_error ("set %s: got wrong template (%s)",
                templatesym->s_name, gpointer_getTemplateIdentifier(gp)->s_name);
            return;
        } 
    }
    else templatesym = gpointer_getTemplateIdentifier(gp);
    if (!(template = template_findByIdentifier(templatesym)))
    {
        post_error ("set: couldn't find template %s", templatesym->s_name);
        return;
    }
    if (!nitems)
        return;
    vec = gpointer_getData (gp);
    if (x->x_issymbol)
        for (i = 0, vp = x->x_variables; i < nitems; i++, vp++)
            word_setSymbol(vec, template, vp->gv_sym, vp->gv_w.w_symbol);
    else for (i = 0, vp = x->x_variables; i < nitems; i++, vp++)
        word_setFloat(vec, template, vp->gv_sym, vp->gv_w.w_float);
    scalar_redrawByPointer (gp);
}

static void set_float(t_set *x, t_float f)
{
    if (x->x_nin && !x->x_issymbol)
    {
        x->x_variables[0].gv_w.w_float = f;
        set_bang(x);
    }
    else post_error ("type mismatch or no field specified");
}

static void set_symbol(t_set *x, t_symbol *s)
{
    if (x->x_nin && x->x_issymbol)
    {
        x->x_variables[0].gv_w.w_symbol = s;
        set_bang(x);
    }
    else post_error ("type mismatch or no field specified");
}

static void set_free(t_set *x)
{
    PD_MEMORY_FREE(x->x_variables);
    gpointer_unset(&x->x_gp);
}

void set_setup(void)
{
    set_class = class_new(sym_set, (t_newmethod)set_new,
        (t_method)set_free, sizeof(t_set), 0, A_GIMME, 0);
    class_addFloat(set_class, set_float); 
    class_addSymbol(set_class, set_symbol); 
    class_addBang(set_class, set_bang); 
    class_addMethod(set_class, (t_method)set_set, sym_set,
        A_SYMBOL, A_SYMBOL, 0); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_class *elem_class;

typedef struct _elem
{
    t_object x_obj;
    t_symbol *x_templatesym;
    t_symbol *x_fieldsym;
    t_gpointer x_gp;
    t_gpointer x_gparent;
} t_elem;

static void *elem_new(t_symbol *templatesym, t_symbol *fieldsym)
{
    t_elem *x = (t_elem *)pd_new(elem_class);
    x->x_templatesym = template_makeIdentifierWithWildcard(templatesym);
    x->x_fieldsym = fieldsym;
    gpointer_init(&x->x_gp);
    gpointer_init(&x->x_gparent);
    inlet_newPointer(&x->x_obj, &x->x_gparent);
    outlet_new(&x->x_obj, &s_pointer);
    return (x);
}

static void elem_set(t_elem *x, t_symbol *templatesym, t_symbol *fieldsym)
{
    x->x_templatesym = template_makeIdentifierWithWildcard(templatesym);
    x->x_fieldsym = fieldsym;
}

static void elem_float(t_elem *x, t_float f)
{
    int indx = f, nitems, onset;
    t_symbol *templatesym, *fieldsym = x->x_fieldsym, *elemtemplatesym;
    t_template *template;
    t_template *elemtemplate;
    t_gpointer *gparent = &x->x_gparent;
    t_word *w;
    t_array *array;
    int elemsize, type;
    
    if (!gpointer_isValid(gparent))
    {
        post_error ("element: empty pointer");
        return;
    }
    if (*x->x_templatesym->s_name)
    {
        if ((templatesym = x->x_templatesym) !=
            gpointer_getTemplateIdentifier(gparent))
        {
            post_error ("elem %s: got wrong template (%s)",
                templatesym->s_name, gpointer_getTemplateIdentifier(gparent)->s_name);
            return;
        } 
    }
    else templatesym = gpointer_getTemplateIdentifier(gparent);
    if (!(template = template_findByIdentifier(templatesym)))
    {
        post_error ("elem: couldn't find template %s", templatesym->s_name);
        return;
    }
    w = gpointer_getData (gparent);
    if (!template)
    {
        post_error ("element: couldn't find template %s", templatesym->s_name);
        return;
    }
    if (!template_findField(template, fieldsym,
        &onset, &type, &elemtemplatesym))
    {
        post_error ("element: couldn't find array field %s", fieldsym->s_name);
        return;
    }
    if (type != DATA_ARRAY)
    {
        post_error ("element: field %s not of type array", fieldsym->s_name);
        return;
    }
    if (!(elemtemplate = template_findByIdentifier(elemtemplatesym)))
    {
        post_error ("element: couldn't find field template %s",
            elemtemplatesym->s_name);
        return;
    }

    elemsize = template_getSize (elemtemplate) * ARRAY_WORD;

    array = *(t_array **)(((char *)w) + onset);

    nitems = array->a_size;
    if (indx < 0) indx = 0;
    if (indx >= nitems) indx = nitems-1;

    gpointer_setAsWord(&x->x_gp, array, 
        (t_word *)((char *)(array->a_vector) + indx * elemsize));
    outlet_pointer(x->x_obj.te_outlet, &x->x_gp);
}

static void elem_free(t_elem *x, t_gpointer *gp)
{
    gpointer_unset(&x->x_gp);
    gpointer_unset(&x->x_gparent);
}

void elem_setup(void)
{
    elem_class = class_new(sym_element, (t_newmethod)elem_new,
        (t_method)elem_free, sizeof(t_elem), 0, A_DEFSYMBOL, A_DEFSYMBOL, 0);
    class_addFloat(elem_class, elem_float); 
    class_addMethod(elem_class, (t_method)elem_set, sym_set,
        A_SYMBOL, A_SYMBOL, 0); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

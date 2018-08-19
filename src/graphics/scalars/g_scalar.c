
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define SCALAR_SELECT_MARGIN            5

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define SCALAR_WRONG_SIZE               20

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_class *scalar_class;                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void scalar_behaviorGetRectangle         (t_gobj *, t_glist *, t_rectangle *);
static void scalar_behaviorDisplaced            (t_gobj *, t_glist *, int, int);
static void scalar_behaviorSelected             (t_gobj *, t_glist *, int);
static void scalar_behaviorActivated            (t_gobj *, t_glist *, int);
static void scalar_behaviorDeleted              (t_gobj *, t_glist *);
static void scalar_behaviorVisibilityChanged    (t_gobj *, t_glist *, int);
static int  scalar_behaviorMouse                (t_gobj *, t_glist *, t_mouse *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int plot_hitElement (t_gobj *, t_gpointer *, t_float, t_float, t_mouse *, t_symbol **, t_gpointer *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_widgetbehavior scalar_widgetBehavior = /* Shared. */
    {
        scalar_behaviorGetRectangle,
        scalar_behaviorDisplaced,
        scalar_behaviorSelected,
        scalar_behaviorActivated,
        scalar_behaviorDeleted,
        scalar_behaviorVisibilityChanged,
        scalar_behaviorMouse,
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void scalar_drawJob (t_gobj *z, t_glist *glist)
{
    scalar_behaviorVisibilityChanged (z, glist, 0);
    scalar_behaviorVisibilityChanged (z, glist, 1);
}

static void scalar_drawSelectRectangle (t_scalar *x, t_glist *glist, int isSelected)
{
    t_glist *view = glist_getView (glist);
    
    if (isSelected) {
    
        t_rectangle r;
       
        scalar_behaviorGetRectangle (cast_gobj (x), glist, &r);
        
        PD_ASSERT (!rectangle_isNothing (&r));
        
        {
            int a = rectangle_getTopLeftX (&r);
            int b = rectangle_getTopLeftY (&r);
            int c = rectangle_getBottomRightX (&r);
            int d = rectangle_getBottomRightY (&r);
            
            gui_vAdd ("%s.c create line %d %d %d %d %d %d %d %d %d %d"
                            " -width 0"
                            " -fill #%06x"
                            " -dash {2 4}"  // --
                            " -tags %lxHANDLE\n",
                            glist_getTagAsString (view),
                            a,
                            b,
                            a,
                            d,
                            c,
                            d,
                            c,
                            b,
                            a,
                            b,
                            COLOR_SELECTED,
                            x);
        }
                
    } else {
        gui_vAdd ("%s.c delete %lxHANDLE\n", glist_getTagAsString (view), x);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void scalar_notifyProceed (t_scalar *scalar, t_symbol *s, int argc, t_atom *argv)
{
    t_gpointer gp; gpointer_init (&gp);
    
    gpointer_setAsScalar (&gp, scalar);
    gpointer_notify (&gp, s, argc, argv);
    gpointer_unset (&gp);
}

static void scalar_notifyClicked (t_scalar *x,
    t_float positionX,
    t_float positionY,
    t_float mouseX,
    t_float mouseY)
{
    t_atom t[4];
    SET_FLOAT (t + 0, positionX);
    SET_FLOAT (t + 1, positionY);
    SET_FLOAT (t + 2, mouseX);
    SET_FLOAT (t + 3, mouseY);
    scalar_notifyProceed (x, sym_click, 4, t);
}
    
static void scalar_notifyDisplaced (t_scalar *x, t_float deltaX, t_float deltaY)
{
    t_atom t[2];
    SET_FLOAT (t + 0, deltaX);
    SET_FLOAT (t + 1, deltaY);
    scalar_notifyProceed (x, sym_displace, 2, t);
}

static void scalar_notifySelected (t_scalar *x, int isSelected)
{
    if (isSelected) {
        scalar_notifyProceed (x, sym_select,   0, NULL);
    } else {
        scalar_notifyProceed (x, sym_deselect, 0, NULL);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void scalar_redraw (t_scalar *x, t_glist *glist)
{
    gui_jobAdd ((void *)x, glist, scalar_drawJob);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void scalar_behaviorGetRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    t_scalar *x = cast_scalar (z);
    t_glist *view = template_getInstanceViewIfPainters (scalar_getTemplate (x));
    t_float baseX = scalar_getFloat (x, sym_x);
    t_float baseY = scalar_getFloat (x, sym_y);

    rectangle_setNothing (r);
    
    if (view) {
    
        t_gobj *y = NULL;
        
        for (y = view->gl_graphics; y; y = y->g_next) {
        //
        t_painterbehavior *behavior = class_getPainterBehavior (pd_class (y));
        
        if (behavior) {
        //
        t_rectangle t;
        t_gpointer gp; gpointer_init (&gp);
        
        gpointer_setAsScalar (&gp, x);
        (*behavior->w_fnPainterGetRectangle) (y, &gp, baseX, baseY, &t);
        gpointer_unset (&gp);
        
        rectangle_addRectangle (r, &t);
        //
        }
        //
        }
    }
    
    if (rectangle_isNothing (r)) {
    
        int a = glist_valueToPixelX (glist, baseX);
        int b = glist_valueToPixelY (glist, baseY);
        int c = a + SCALAR_WRONG_SIZE;
        int d = b + SCALAR_WRONG_SIZE;
        
        rectangle_set (r, a, b, c, d);
        
    } else if (!rectangle_isEverything (r)) { rectangle_enlarge (r, SCALAR_SELECT_MARGIN); }
}

static void scalar_behaviorDisplaced (t_gobj *z, t_glist *glist, int deltaX, int deltaY)
{
    t_scalar *x = cast_scalar (z);
    t_template *tmpl = scalar_getTemplate (x);
    
    if (template_fieldIsFloat (tmpl, sym_x)) {

        t_float f = word_getFloat (x->sc_element, tmpl, sym_x);
        f += (deltaX * glist_getValueForOnePixelX (glist));
        word_setFloat (x->sc_element, tmpl, sym_x, f);
    }
    
    if (template_fieldIsFloat (tmpl, sym_y)) {

        t_float f = word_getFloat (x->sc_element, tmpl, sym_y);
        f += (deltaY * glist_getValueForOnePixelY (glist));
        word_setFloat (x->sc_element, tmpl, sym_y, f);
    }
    
    scalar_redraw (x, glist);
    glist_redrawRequired (glist);
    scalar_notifyDisplaced (x, (t_float)deltaX, (t_float)deltaY);
}

static void scalar_behaviorSelected (t_gobj *z, t_glist *glist, int isSelected)
{
    scalar_drawSelectRectangle (cast_scalar (z), glist, isSelected);
    scalar_notifySelected (cast_scalar (z), isSelected);
}

static void scalar_behaviorActivated (t_gobj *z, t_glist *glist, int isActivated)
{
}

static void scalar_behaviorDeleted (t_gobj *z, t_glist *glist)
{
}

static void scalar_behaviorVisibilityChanged (t_gobj *z, t_glist *glist, int isVisible)
{
    t_scalar *x = cast_scalar (z);
    t_glist *owner = template_getInstanceViewIfPainters (scalar_getTemplate (x));
    t_float baseX  = scalar_getFloat (x, sym_x);
    t_float baseY  = scalar_getFloat (x, sym_y);

    if (!owner) {
        
        t_glist *view = glist_getView (glist);
        
        if (isVisible) {
        
            int a = glist_valueToPixelX (glist, baseX);
            int b = glist_valueToPixelY (glist, baseY);
            
            gui_vAdd ("%s.c create rectangle %d %d %d %d"
                            " -outline #%06x"
                            " -tags %lxSCALAR\n",
                            glist_getTagAsString (view),
                            a,
                            b,
                            a + SCALAR_WRONG_SIZE,
                            b + SCALAR_WRONG_SIZE,
                            COLOR_SCALAR_WRONG,
                            x);
        } else {
            gui_vAdd ("%s.c delete %lxSCALAR\n", glist_getTagAsString (view), x);
        }
        
    } else {
    
        t_gobj *y = NULL;
        
        for (y = owner->gl_graphics; y; y = y->g_next) {
        
            t_painterbehavior *behavior = class_getPainterBehavior (pd_class (y));
            
            if (behavior) {
                
                t_gpointer gp; gpointer_init (&gp);
                
                gpointer_setAsScalar (&gp, x);
                (*behavior->w_fnPainterVisibilityChanged) (y, &gp, baseX, baseY, isVisible);
                gpointer_unset (&gp);
            }
        }
    }
    
    if (glist_objectIsSelected (glist, cast_gobj (x))) {
        scalar_drawSelectRectangle (x, glist, 0);
        scalar_drawSelectRectangle (x, glist, 1);
    }
}

static int scalar_behaviorMouse (t_gobj *z, t_glist *glist, t_mouse *m)
{
    t_scalar *x = cast_scalar (z);
    t_glist *view = template_getInstanceViewIfPainters (scalar_getTemplate (x));
    
    if (!x->sc_disable && view) {
    //
    t_float baseX = scalar_getFloat (x, sym_x);
    t_float baseY = scalar_getFloat (x, sym_y);
    t_gobj *y = NULL;
        
    if (m->m_clicked) { scalar_notifyClicked (x, baseX, baseY, (t_float)m->m_x, (t_float)m->m_y); }
            
    for (y = view->gl_graphics; y; y = y->g_next) {
    //
    t_painterbehavior *behavior = class_getPainterBehavior (pd_class (y));
    
    if (behavior) { 
        
        t_gpointer gp; gpointer_init (&gp);
        
        gpointer_setAsScalar (&gp, x);
        int k = (*behavior->w_fnPainterMouse) (y, &gp, baseX, baseY, m);
        gpointer_unset (&gp);
        
        if (k) {
            return k;
        }
    }
    //
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void scalar_serialize (t_scalar *x, t_buffer *b)
{
    t_template *tmpl = scalar_getTemplate (x);
    t_symbol *s = template_getUnexpandedName (tmpl);
    int i;
    
    if (!s) { buffer_appendSymbol (b, symbol_stripTemplateIdentifier (x->sc_templateIdentifier)); }
    else {
        buffer_appendDollarSymbol (b, s);
    }
    
    for (i = 0; i < template_getSize (tmpl); i++) {
    
        t_symbol *fieldName = template_getFieldAtIndex (tmpl, i);
        
        if (template_fieldIsFloat (tmpl, fieldName)) {
            t_atom t;
            SET_FLOAT (&t, word_getFloat (x->sc_element, tmpl, fieldName));
            buffer_appendAtom (b, &t);
            
        } else if (template_fieldIsSymbol (tmpl, fieldName)) {
            t_atom t;
            SET_SYMBOL (&t, word_getSymbol (x->sc_element, tmpl, fieldName));
            buffer_appendAtom (b, &t);
        }
    }

    buffer_appendSemicolon (b);

    for (i = 0; i < template_getSize (tmpl); i++) {
    
        t_symbol *fieldName = template_getFieldAtIndex (tmpl, i);
            
        if (template_fieldIsArray (tmpl, fieldName)) {
            array_serialize (word_getArray (x->sc_element, tmpl, fieldName), b);
            buffer_appendSemicolon (b);
        }
    }
}

void scalar_deserialize (t_scalar *x, t_glist *glist, int argc, t_atom *argv)
{
    t_template *tmpl = scalar_getTemplate (x);
    
    if (!template_isValid (tmpl)) { PD_BUG; }
    else {
    //
    t_iterator *iter = iterator_new (argc, argv, 0);
    t_atom *atoms = NULL;
    int count = iterator_next (iter, &atoms);      
    int i;
        
    for (i = 0; i < template_getSize (tmpl); i++) {
    //
    t_symbol *fieldName = template_getFieldAtIndex (tmpl, i);
    
    if (template_fieldIsFloat (tmpl, fieldName)) {
        t_float f = 0.0;
        if (count) { f = atom_getFloat (atoms); atoms++; count--; }
        word_setFloat (x->sc_element, tmpl, fieldName, f);
        
    } else if (template_fieldIsSymbol (tmpl, fieldName)) {
        t_symbol *s = &s_;
        if (count) { s = atom_getSymbol (atoms); atoms++; count--; }
        word_setSymbol (x->sc_element, tmpl, fieldName, s);
    }
    //
    }
    
    PD_ASSERT (count == 0);
    
    for (i = 0; i < template_getSize (tmpl); i++) {
    //
    t_symbol *fieldName = template_getFieldAtIndex (tmpl, i);
    
    if (template_fieldIsArray (tmpl, fieldName)) {
        array_deserialize (word_getArray (x->sc_element, tmpl, fieldName), iter);
    }
    //
    }
    
    iterator_free (iter);
    //
    }
}

/* Note that scalars are usually NOT saved with patch. */
/* This function is required only for copy and paste behavior. */
/* It is required for undo and redo also. */

static void scalar_functionSave (t_gobj *z, t_buffer *b, int flags)
{
    t_scalar *x = cast_scalar (z);
    t_buffer *t = buffer_new();
   
    scalar_serialize (x, t);
    buffer_appendSymbol (b, sym___hash__X);
    buffer_appendSymbol (b, sym_scalar);
    buffer_serialize (b, t);
    buffer_appendSemicolon (b);
    
    if (flags & SAVE_ID) {
        gobj_serializeUnique (z, sym__tagobject, b);
    }
    
    buffer_free (t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int scalar_functionValueFetchIfPlotted (t_scalar *x,
    t_glist *glist,
    t_mouse *m,
    t_symbol **s,
    t_gpointer *e)
{
    int i = -1;
    
    if (m->m_clickedRight && template_containsArray (scalar_getTemplate (x))) {
    //
    t_glist *view = template_getInstanceViewIfPainters (scalar_getTemplate (x));

    if (view) {
    //
    t_gobj *y = NULL;
    
    for (y = view->gl_graphics; y; y = y->g_next) {
    //
    if (pd_class (y) == plot_class) {
    //
    t_float baseX = scalar_getFloat (x, sym_x);
    t_float baseY = scalar_getFloat (x, sym_y);
    
    t_gpointer gp; gpointer_init (&gp);

    gpointer_setAsScalar (&gp, x);
    i = plot_hitElement (y, &gp, baseX, baseY, m, s, e);
    gpointer_unset (&gp);
    
    if (i >= 0) { break; }
    //
    }
    //
    }
    //
    }
    //
    }
    
    return i;
}

void scalar_functionValue (t_gobj *z, t_glist *glist, t_mouse *m)
{
    t_scalar *x = cast_scalar (z);
    t_gpointer gp; gpointer_init (&gp);
    t_symbol *s = sym_invalid;
    int i = scalar_functionValueFetchIfPlotted (x, glist, m, &s, &gp);
    t_heapstring *h = heapstring_new (0);
    
    if (i >= 0) { heapstring_addSprintf (h, "::ui_scalar::show %%s element %d %s ", i, s->s_name); }
    else {
    //
    heapstring_addSprintf (h, "::ui_scalar::show %%s scalar %d %s ", i, s->s_name);
    
    gpointer_setAsScalar (&gp, x);
    //
    }
    
    if (gpointer_isValid (&gp)) {
    //
    if (gpointer_getValues (&gp, h)) {
    //
    heapstring_add (h, "\n"); stub_new (cast_pd (x), (void *)x, heapstring_getRaw (h));
    //
    } else {
        PD_BUG;
    }
    //
    }
    
    heapstring_free (h); gpointer_unset (&gp);
}

void scalar_fromValue (t_scalar *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 5) {
    //
    t_symbol *type = atom_getSymbol (argv + 0);
    int index = atom_getFloat (argv + 1);
    t_symbol *field = atom_getSymbol (argv + 2);
    t_symbol *templateIdentifier = symbol_makeTemplateIdentifier (atom_getSymbol (argv + 3));
    
    argc -= 4;
    argv += 4;
    
    {
        t_gpointer gp; gpointer_init (&gp);
    
        if (type == sym_scalar) {
            PD_ASSERT (templateIdentifier == x->sc_templateIdentifier);
            gpointer_setAsScalar (&gp, x);
            
        } else if (type == sym_element) {
            if (scalar_containsTemplate (x, templateIdentifier)) {
            if (scalar_hasField (x, field)) {
            if (scalar_fieldIsArrayAndValid (x, field)) {
                t_array *array = scalar_getArray (x, field);
                if (index < array_getSize (array)) {
                    gpointer_setAsWord (&gp, array, index);
                }
            }
            }
            }
        }
    
        if (gpointer_isValid (&gp)) {
            gpointer_setValues (&gp, argc, argv, (s == sym__scalardialog) ? 1 : 0);
        }
    
        gpointer_unset (&gp);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_word *scalar_getElement (t_scalar *x)
{
    return x->sc_element;
}

t_symbol *scalar_getTemplateIdentifier (t_scalar *x)
{
    return x->sc_templateIdentifier;
}

t_template *scalar_getTemplate (t_scalar *x)
{
    t_template *tmpl = template_findByIdentifier (x->sc_templateIdentifier);
    
    PD_ASSERT (tmpl);
    
    return tmpl;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int scalar_containsTemplate (t_scalar *x, t_symbol *templateIdentifier)
{
    return template_containsTemplate (scalar_getTemplate (x), templateIdentifier);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int scalar_hasField (t_scalar *x, t_symbol *field)
{
    return template_hasField (scalar_getTemplate (x), field);
}

int scalar_fieldIsFloat (t_scalar *x, t_symbol *fieldName)
{
    return template_fieldIsFloat (scalar_getTemplate (x), fieldName);
}

int scalar_fieldIsArrayAndValid (t_scalar *x, t_symbol *field)
{
    return template_fieldIsArrayAndValid (scalar_getTemplate (x), field);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_array *scalar_getArray (t_scalar *x, t_symbol *fieldName)
{
    return word_getArray (x->sc_element, scalar_getTemplate (x), fieldName);
}

t_float scalar_getFloat (t_scalar *x, t_symbol *fieldName)
{
    return word_getFloat (x->sc_element, scalar_getTemplate (x), fieldName);
}

void scalar_setFloat (t_scalar *x, t_symbol *fieldName, t_float f)
{
    word_setFloat (x->sc_element, scalar_getTemplate (x), fieldName, f);  
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void scalar_disable (t_scalar *x)
{
    x->sc_disable = 1;
}

void scalar_enable (t_scalar *x)
{
    x->sc_disable = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_scalar *scalar_new (t_glist *owner, t_symbol *templateIdentifier)
{
    t_scalar *x = NULL;
    
    t_template *tmpl = template_findByIdentifier (templateIdentifier);

    if (template_isValid (tmpl)) {
    
        t_gpointer gp; gpointer_init (&gp);
        
        x = (t_scalar *)pd_new (scalar_class);
        
        x->sc_owner = owner;
        x->sc_templateIdentifier = templateIdentifier;
        x->sc_element = (t_word *)PD_MEMORY_GET (template_getSize (tmpl) * sizeof (t_word));
        
        gpointer_setAsScalar (&gp, x);
        
        word_init (x->sc_element, tmpl, &gp);
        
        gpointer_unset (&gp);
    }
    
    return x;
}

static void scalar_free (t_scalar *x)
{
    gui_jobRemove ((void *)x);
    
    stub_destroyWithKey ((void *)x);
    
    word_free (x->sc_element, scalar_getTemplate (x));

    PD_MEMORY_FREE (x->sc_element);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void scalar_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_scalar,
        NULL,
        (t_method)scalar_free,
        sizeof (t_scalar),
        CLASS_GRAPHIC,
        A_NULL);
    
    class_addMethod (c, (t_method)scalar_fromValue, sym__scalardialog, A_GIMME, A_NULL);
    
    class_setWidgetBehavior (c, &scalar_widgetBehavior);
    class_setSaveFunction (c, scalar_functionSave);
    class_setValueFunction (c, scalar_functionValue);
    
    scalar_class = c;
}

void scalar_destroy (void)
{
    class_free (scalar_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

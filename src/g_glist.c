
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_glist *glist_newSubpatch (t_symbol *name)
{
    t_glist *x = glist_newPatch (name, NULL, 0, 0);
    
    if (!utils_isNameAllowedForWindow (name)) { warning_badName (sym_pd, name); }
    
    instance_stackPopPatch (x, glist_isOpenedAtLoad (x));
    
    return x;
}

t_glist *glist_newPatch (t_symbol *name, t_rectangle *window, int isVisible, int fontSize)
{
    t_glist *owner = instance_contextGetCurrent();
    
    t_rectangle r1, r2; t_bounds bounds;
    
    t_error err = bounds_set (&bounds, 0, 0, 1, 1);
    
    rectangle_set (&r1, 0, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    rectangle_set (&r2, 0, WINDOW_HEADER, WINDOW_WIDTH, WINDOW_HEIGHT + WINDOW_HEADER);
    
    if (window) { rectangle_setCopy (&r2, window); }
    
    if (!err) {
    //
    t_glist *x = glist_new (owner, name, &bounds, &r1, &r2);
    
    object_setType (cast_object (x), TYPE_OBJECT);
    
    if (fontSize) { glist_setFontSize (x, fontSize); }
    
    glist_setEditMode (x, 0);
    glist_setOpenedAtLoad (x, isVisible);
    glist_bind (x);
    
    if (glist_isRoot (x)) { instance_rootsAdd (x); }
    
    glist_loadBegin (x); instance_stackPush (x);
    
    return x;
    //
    }
    
    return NULL;
}

t_glist *glist_new (t_glist *owner, t_symbol *name, t_bounds *b, t_rectangle *r1, t_rectangle *r2)
{
    t_glist *x = (t_glist *)pd_new (canvas_class);
    
    x->gl_holder            = gmaster_createWithGlist (x);
    x->gl_parent            = owner;
    x->gl_environment       = instance_environmentFetchIfAny();
    x->gl_name              = (name != &s_ ? name : environment_getFileName (x->gl_environment));
    x->gl_editor            = editor_new (x);
    x->gl_uniqueIdentifier  = utils_unique();
    x->gl_fontSize          = (owner ? glist_getFontSize (owner) : font_getDefaultFontSize());

    if (b)  { bounds_setCopy (&x->gl_bounds, b);             }
    if (r1) { rectangle_setCopy (&x->gl_geometryGraph, r1);  }
    if (r2) { rectangle_setCopy (&x->gl_geometryWindow, r2); }
    
    return x;
}

void glist_free (t_glist *glist)
{
    glist_deselectAll (glist);
    glist_objectRemoveAll (glist);
    
    if (glist_hasView (glist)) { canvas_visible (glist, 0); }
    
    stub_destroyWithKey ((void *)glist);
    
    glist_unbind (glist);

    editor_free (glist_getEditor (glist));
    environment_free (glist->gl_environment);
    gmaster_reset (glist_getMaster (glist));
    
    if (glist_isRoot (glist)) { instance_rootsRemove (glist); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* A root has no parent and an environment. */
/* An abstraction has a parent and an environment. */
/* A subpatch has a parent also but no environment. */
/* A top patch is either a root or an abstraction. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int glist_isRoot (t_glist *glist)
{
    return (!glist_hasParent (glist));
}

int glist_isTop (t_glist *glist)
{
    int k = (glist_isRoot (glist) || glist_isAbstraction (glist));
    
    return k;
}

int glist_isAbstraction (t_glist *glist)
{
    return (glist_hasParent (glist) && (glist->gl_environment != NULL));
}

int glist_isSubpatch (t_glist *glist)
{
    return (!glist_isTop (glist));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Array is a GOP patch that contains only a scalar. */
/* This scalar has an array of numbers as unique field. */
/* Dirty bit is always owned by the top patch. */
/* For GOP the view to draw on is owned by a parent. */
/* Note that a GOP can be opened in its own window on demand. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int glist_isArray (t_glist *glist)
{
    return (utils_getFirstAtomOfObjectAsSymbol (cast_object (glist)) == sym_graph);
}

int glist_isDirty (t_glist *glist)
{
    return (glist_getTop (glist)->gl_isDirty != 0);
}

int glist_isOnScreen (t_glist *glist)
{
    return (!glist_isLoading (glist) && glist_getView (glist)->gl_isMapped);
}

int glist_isParentOnScreen (t_glist *g)
{
    return (!glist_isLoading (g) && glist_hasParent (g) && glist_isOnScreen (glist_getParent (g)));
}

int glist_isWindowable (t_glist *glist)
{
    return (!glist_isGraphOnParent (glist) || glist_hasWindow (glist));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist *glist_getTop (t_glist *glist)
{
    if (glist_isTop (glist)) { return glist; } else { return glist_getTop (glist_getParent (glist)); }
}

t_environment *glist_getEnvironment (t_glist *glist)
{
    return (glist_getTop (glist)->gl_environment);
}

t_glist *glist_getView (t_glist *glist)
{
    while (glist_hasParent (glist) && !glist_isWindowable (glist)) { glist = glist_getParent (glist); }
    
    return glist;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void glist_setName (t_glist *glist, t_symbol *name)
{
    glist_unbind (glist);
    
    glist->gl_name = name;
    
    glist_bind (glist);
    
    glist_updateTitle (glist);
}

void glist_setDirty (t_glist *glist, int n)
{
    int isDirty = (n != 0);
        
    t_glist *y = glist_getTop (glist);
        
    if (y->gl_isDirty != isDirty) {
    //
    y->gl_isDirty = isDirty; 
    
    glist_updateTitle (y);
    //
    }
}

void glist_setFontSize (t_glist *g, int n)
{
    g->gl_fontSize = font_getNearestValidFontSize (n);
}

void glist_setMotion (t_glist *glist, t_gobj *y, t_motionfn callback, int a, int b)
{
    editor_motionSet (glist_getEditor (glist_getView (glist)), y, callback, a, b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_bind (t_glist *glist)
{
    t_symbol *s = utils_removeExtension (glist_getName (glist));
    
    if (utils_isNameAllowedForWindow (s)) {
        pd_bind (cast_pd (glist), utils_makeBindSymbol (s));
    }
}

void glist_unbind (t_glist *glist)
{
    t_symbol *s = utils_removeExtension (glist_getName (glist));
    
    if (utils_isNameAllowedForWindow (s)) {
        pd_unbind (cast_pd (glist), utils_makeBindSymbol (s));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Files are searching in the directory of the patch first. */
/* Without success it tries to find it using the search path. */
/* Note that it always contains at first the "extras" folder. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int glist_fileExist (t_glist *glist, char *name, char *extension, t_fileproperties *p)
{
    int f = glist_fileOpen (glist, name, extension, p);
    
    if (f >= 0) { close (f); return 1; }
    
    return 0;
}

/* Caller is responsible to close the file. */

int glist_fileOpen (t_glist *glist, char *name, char *extension, t_fileproperties *p)
{
    char *directory = glist ? environment_getDirectoryAsString (glist_getEnvironment (glist)) : ".";
    
    int f = file_openConsideringSearchPath (directory, name, extension, p);
        
    return f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_objectMake (t_glist *glist, int a, int b, int w, int isSelected, t_buffer *t)
{
    instance_setNewestObject (NULL);
    
    instance_stackPush (glist);
    
    {
    //
    t_environment *e = glist_getEnvironment (instance_contextGetCurrent());
    
    t_object *x = NULL;
    
    buffer_eval (t, 
        instance_getMakerObject(), 
        environment_getNumberOfArguments (e), 
        environment_getArguments (e));

    if (instance_getNewestObject()) { x = cast_objectIfConnectable (instance_getNewestObject()); }

    if (!x) {
        x = (t_object *)pd_new (text_class);    /* If failed create a dummy box. */
        if (buffer_size (t)) {
            error_canNotMake (buffer_size (t), buffer_atoms (t)); 
        }
    }

    object_setBuffer (x, t);
    object_setX (x, a);
    object_setY (x, b);
    object_setWidth (x, w);
    object_setType (x, TYPE_OBJECT);
    
    glist_objectAdd (glist, cast_gobj (x));
    
    if (isSelected) {
    //
    glist_objectSelect (glist, cast_gobj (x));
    gobj_activated (cast_gobj (x), glist, 1);
    //
    }
    
    if (pd_class (x) == vinlet_class)  { glist_inletSort (glist_getView (glist));  }
    if (pd_class (x) == voutlet_class) { glist_outletSort (glist_getView (glist)); }
    //
    }
    
    instance_stackPop (glist);
}

void glist_objectMakeScalar (t_glist *glist, int argc, t_atom *argv)
{
    if (argc > 0 && IS_SYMBOL (argv)) {
    //
    t_symbol *templateIdentifier = utils_makeTemplateIdentifier (GET_SYMBOL (argv));
        
    if (template_isValid (template_findByIdentifier (templateIdentifier))) {
    //
    t_scalar *scalar = scalar_new (glist, templateIdentifier);
    
    PD_ASSERT (scalar);
    
    if (scalar) {
    //
    glist_objectAdd (glist, cast_gobj (scalar));
    scalar_deserialize (scalar, glist, argc - 1, argv + 1);
    if (glist_isOnScreen (glist)) { gobj_visibilityChanged (cast_gobj (scalar), glist, 1); }
    //
    }
    //
    }
    //
    }
}

void glist_objectAddProceed (t_glist *glist, t_gobj *first, t_gobj *next)
{
    next->g_next = NULL;
    
    if (first != NULL) { next->g_next = first->g_next; first->g_next = next; }
    else {
    //    
    if (!glist->gl_graphics) { glist->gl_graphics = next; }
    else {
        t_gobj *t = NULL; for (t = glist->gl_graphics; t->g_next; t = t->g_next) { } 
        t->g_next = next;
    }
    //
    }
}

void glist_objectAddNext (t_glist *glist, t_gobj *first, t_gobj *next)
{
    int needToRepaint = class_hasPainterWidgetBehavior (pd_class (next));
    
    if (needToRepaint) { paint_erase(); }
    
    glist_objectAddProceed (glist, first, next);
    
    if (cast_objectIfConnectable (next)) { editor_boxAdd (glist_getEditor (glist), cast_object (next)); }
    if (glist_isOnScreen (glist_getView (glist))) { gobj_visibilityChanged (next, glist, 1); }
    
    if (needToRepaint) { paint_draw(); }
}

void glist_objectAdd (t_glist *glist, t_gobj *y)
{
    glist_objectAddNext (glist, NULL, y);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void glist_objectRemoveProceed (t_glist *glist, t_gobj *y)
{
    if (glist->gl_graphics == y) { glist->gl_graphics = y->g_next; }
    else {
        t_gobj *t = NULL;
        for (t = glist->gl_graphics; t; t = t->g_next) {
            if (t->g_next == y) { t->g_next = y->g_next; break; }
        }
    }
}

void glist_objectRemove (t_glist *glist, t_gobj *y)
{
    int needToRebuild = class_hasDSP (pd_class (y));
    int needToRepaint = class_hasPainterWidgetBehavior (pd_class (y)) || (pd_class (y) == struct_class);
    
    glist_deleteBegin (glist);
    
    editor_motionUnset (glist_getEditor (glist), y);
    
    if (glist_objectIsSelected (glist, y)) { glist_objectDeselect (glist, y); }
    if (needToRepaint) { paint_erase(); }
    if (glist_isOnScreen (glist)) { gobj_visibilityChanged (y, glist, 0); }
    
    {
        t_box *box = NULL;
        
        if (cast_objectIfConnectable (y)) { box = box_fetch (glist, cast_object (y)); }
        
        gobj_deleted (y, glist); 
        glist_objectRemoveProceed (glist, y); 
        pd_free (cast_pd (y));

        if (box) {
            editor_boxRemove (glist_getEditor (glist), box); 
        }
    }
    
    if (needToRebuild) { dsp_update(); }
    if (needToRepaint) { paint_draw(); }
    
    glist->gl_uniqueIdentifier = utils_unique();    /* Invalidate all pointers. */
    
    glist_deleteEnd (glist);
}

/* If needed the DSP is suspended to avoid multiple rebuilds. */

void glist_objectRemoveAll (t_glist *glist)
{
    int dspState = 0;
    int dspSuspended = 0;
    t_gobj *y = NULL;
    
    while ((y = glist->gl_graphics)) {
    //
    if (!dspSuspended) { 
        if (class_hasDSP (pd_class (y))) { dspState = dsp_suspend(); dspSuspended = 1; }
    }

    glist_objectRemove (glist, y);
    //
    }
    
    if (dspSuspended) { dsp_resume (dspState); }
}

void glist_objectRemoveByTemplate (t_glist *glist, t_template *template)
{
    t_gobj *y = NULL;

    for (y = glist->gl_graphics; y; y = y->g_next) {
    
        if (pd_class (y) == scalar_class) {
            if (scalar_containsTemplate (cast_scalar (y), template_getTemplateIdentifier (template))) {
                glist_objectRemove (glist, y);
            }
        }
        
        if (pd_class (y) == canvas_class) {
            glist_objectRemoveByTemplate (cast_glist (y), template);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_gobj *glist_objectGetAt (t_glist *glist, int n)
{
    t_gobj *t = NULL;
    int i = 0;
    
    for (t = glist->gl_graphics; t; t = t->g_next) {
        if (i == n) { return t; }
        i++;
    }
    
    return NULL;
}

t_gobj *glist_objectHit (t_glist *glist, int a, int b, t_rectangle *r)
{
    t_gobj *y = NULL;
    t_gobj *object = NULL;
    
    t_rectangle t;
    
    rectangle_set (r, 0, 0, 0, 0);
    
    if (glist_objectGetNumberOfSelected (glist) > 1) {
    //
    t_selection *s = NULL;
    for (s = editor_getSelection (glist_getEditor (glist)); s; s = selection_getNext (s)) {
    //
    if (gobj_hit (selection_getObject (s), glist, a, b, &t)) {
        rectangle_setCopy (r, &t);
        object = selection_getObject (s); 
    }
    //
    }
    //
    }
    
    if (!object) {
    //
    int k = -PD_INT_MAX;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (gobj_hit (y, glist, a, b, &t)) {
            if (rectangle_getTopLeftX (&t) > k) {
                rectangle_setCopy (r, &t);
                object = y; k = rectangle_getTopLeftX (&t);
            }
        }
    }
    //
    }

    return object;
}

int glist_objectGetIndexOf (t_glist *glist, t_gobj *y)
{
    t_gobj *t = NULL;
    int n = 0;
    
    for (t = glist->gl_graphics; t && t != y; t = t->g_next) { 
        n++; 
    }
    
    return n;
}

int glist_objectGetNumberOf (t_glist *glist)
{
    return glist_objectGetIndexOf (glist, NULL);
}

void glist_objectDeleteLines (t_glist *glist, t_object *o)
{
    t_outconnect *connection = NULL;
    t_traverser t;

    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
        if (traverser_getSource (&t) == o || traverser_getDestination (&t) == o) {
            glist_eraseLine (glist, traverser_getCord (&t)); traverser_disconnect (&t);
        }
    }
}

static void glist_objectDeleteLinesByInlets (t_glist *glist, t_object *o, t_inlet *inlet, t_outlet *outlet)
{
    t_outconnect *connection = NULL;
    t_traverser t;

    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    int m = (traverser_getSource (&t) == o && traverser_getOutlet (&t) == outlet);
    int n = (traverser_getDestination (&t) == o && traverser_getInlet (&t) == inlet);
    
    if (m || n) { 
        glist_eraseLine (glist, traverser_getCord (&t)); traverser_disconnect (&t); 
    }
    //
    }
}

void glist_objectDeleteLinesByInlet (t_glist *glist, t_object *o, t_inlet *inlet)
{
    glist_objectDeleteLinesByInlets (glist, o, inlet, NULL);
}

void glist_objectDeleteLinesByOutlet (t_glist *glist, t_object *o, t_outlet *outlet)
{
    glist_objectDeleteLinesByInlets (glist, o, NULL, outlet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_inlet *glist_inletAdd (t_glist *glist, t_pd *receiver, int isSignal)
{
    t_inlet *inlet = inlet_new (cast_object (glist), receiver, (isSignal ? &s_signal : NULL), NULL);
    
    if (!glist_isLoading (glist)) {
    //
    if (glist_isParentOnScreen (glist)) {
        gobj_visibilityChanged (cast_gobj (glist), glist_getParent (glist), 0);
        gobj_visibilityChanged (cast_gobj (glist), glist_getParent (glist), 1);
        glist_updateLinesForObject (glist_getParent (glist), cast_object (glist));
    }
    
    glist_inletSort (glist);
    //
    }
    
    return inlet;
}

void glist_inletRemove (t_glist *glist, t_inlet *inlet)
{
    t_glist *owner = glist_getParent (glist);
    
    int redraw = (owner && !glist_isDeleting (owner) && glist_isOnScreen (owner));
    
    if (owner)  { glist_objectDeleteLinesByInlet (owner, cast_object (glist), inlet); }
    if (redraw) { gobj_visibilityChanged (cast_gobj (glist), owner, 0); }
        
    inlet_free (inlet);
    
    if (redraw) { gobj_visibilityChanged (cast_gobj (glist), owner, 1); }
    if (owner)  { glist_updateLinesForObject (owner, cast_object (glist)); }
}

int glist_inletNumberOf (t_glist *glist)
{
    int n = 0;
    
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) { if (pd_class (y) == vinlet_class) { n++; } }
    
    return n;
}

void glist_inletSort (t_glist *glist)
{
    int numberOfInlets = glist_inletNumberOf (glist);

    if (numberOfInlets > 1) {
    //
    int i;
    t_gobj *y = NULL;
    
    /* Fetch all inlets into a list. */
    
    t_gobj **inlets = (t_gobj **)PD_MEMORY_GET (numberOfInlets * sizeof (t_gobj *));
    t_gobj **t = inlets;
    
    for (y = glist->gl_graphics; y; y = y->g_next) { if (pd_class (y) == vinlet_class) { *t++ = y; } }
    
    /* Take the most right inlet and put it first. */
    /* Remove it from the list. */
    /* Do it again. */
    
    for (i = numberOfInlets; i > 0; i--) {
    //
    int j = numberOfInlets;
    int maximumX = -PD_INT_MAX;
    t_gobj **mostRightInlet = NULL;
    
    for (t = inlets; j--; t++) {
        if (*t) {
            t_rectangle r;
            gobj_getRectangle (*t, glist, &r);
            if (rectangle_getTopLeftX (&r) > maximumX) {
                maximumX = rectangle_getTopLeftX (&r); mostRightInlet = t; 
            }
        }
    }
    
    if (mostRightInlet) {
        inlet_moveFirst (vinlet_getInlet (cast_pd (*mostRightInlet))); *mostRightInlet = NULL;
    }
    //
    }
    
    PD_MEMORY_FREE (inlets);
    
    if (glist_isParentOnScreen (glist)) {
        glist_updateLinesForObject (glist_getParent (glist), cast_object (glist));
    }
    //
    }
}

t_outlet *glist_outletAdd (t_glist *glist, t_symbol *s)
{
    t_outlet *outlet = outlet_new (cast_object (glist), s);
    
    if (!glist_isLoading (glist)) {
    //
    if (glist_isParentOnScreen (glist)) {
        gobj_visibilityChanged (cast_gobj (glist), glist_getParent (glist), 0);
        gobj_visibilityChanged (cast_gobj (glist), glist_getParent (glist), 1);
        glist_updateLinesForObject (glist_getParent (glist), cast_object (glist));
    }
    
    glist_outletSort (glist);
    //
    }
    
    return outlet;
}

void glist_outletRemove (t_glist *glist, t_outlet *outlet)
{
    t_glist *owner = glist_getParent (glist);
    
    int redraw = (owner && !glist_isDeleting (owner) && glist_isOnScreen (owner));
    
    if (owner)  { glist_objectDeleteLinesByOutlet (owner, cast_object (glist), outlet); }
    if (redraw) { gobj_visibilityChanged (cast_gobj (glist), owner, 0); }

    outlet_free (outlet);
    
    if (redraw) { gobj_visibilityChanged (cast_gobj (glist), owner, 1); }
    if (owner)  { glist_updateLinesForObject (owner, cast_object (glist)); }
}

int glist_outletNumberOf (t_glist *glist)
{
    int n = 0;
    
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) { if (pd_class (y) == voutlet_class) { n++; } }
    
    return n;
}

void glist_outletSort (t_glist *glist)
{
    int numberOfOutlets = glist_outletNumberOf (glist);
    
    if (numberOfOutlets > 1) {
    //
    int i;
    t_gobj *y = NULL;
    
    /* Fetch all outlets into a list. */
    
    t_gobj **outlets = (t_gobj **)PD_MEMORY_GET (numberOfOutlets * sizeof (t_gobj *));
    t_gobj **t = outlets;
        
    for (y = glist->gl_graphics; y; y = y->g_next) { if (pd_class (y) == voutlet_class) { *t++ = y; } }
    
    /* Take the most right outlet and put it first. */
    /* Remove it from the list. */
    /* Do it again. */
    
    for (i = numberOfOutlets; i > 0; i--) {
    //
    int j = numberOfOutlets;
    int maximumX = -PD_INT_MAX;
    t_gobj **mostRightOutlet = NULL;
    
    for (t = outlets; j--; t++) {
        if (*t) {
            t_rectangle r;
            gobj_getRectangle (*t, glist, &r);
            if (rectangle_getTopLeftX (&r) > maximumX) {
                maximumX = rectangle_getTopLeftX (&r); mostRightOutlet = t; 
            }
        }
    }
    
    if (mostRightOutlet) {
        outlet_moveFirst (voutlet_getOutlet (cast_pd (*mostRightOutlet))); *mostRightOutlet = NULL;
    }
    //
    }
    
    PD_MEMORY_FREE (outlets);
    
    if (glist_isParentOnScreen (glist)) {
        glist_updateLinesForObject (glist_getParent (glist), cast_object (glist));
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_lineSelect (t_glist *glist, t_traverser *t)
{
    glist_deselectAll (glist);
                
    editor_selectedLineSet (glist_getEditor (glist),
        cord_getConnection (traverser_getCord (t)),
        glist_objectGetIndexOf (glist, cast_gobj (traverser_getSource (t))), 
        traverser_getIndexOfOutlet (t),
        glist_objectGetIndexOf (glist, cast_gobj (traverser_getDestination (t))), 
        traverser_getIndexOfInlet (t));
        
    glist_updateLineSelected (glist, 1);
}

void glist_lineDeselect (t_glist *glist)
{
    glist_updateLineSelected (glist, 0);
    
    editor_selectedLineReset (glist_getEditor (glist));
}

int glist_lineExist (t_glist *glist, t_object *o, int m, t_object *i, int n)
{
    t_outconnect *connection = NULL;
    
    t_traverser t;
    
    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) { 
        if (traverser_isItLineBetween (&t, o, m, i, n)) { 
            return 1; 
        } 
    }
    
    return 0;
}

void glist_lineDeleteSelected (t_glist *glist)
{
    if (editor_hasSelectedLine (glist_getEditor (glist))) {
        editor_selectedLineDisconnect (glist_getEditor (glist));
        glist_setDirty (glist, 1);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

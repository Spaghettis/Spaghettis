
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

extern t_widgetbehavior     glist_widgetbehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *canvas_class;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void canvas_dsp                     (t_glist *, t_signal **);

void canvas_makeObject              (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeMessage             (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeArray               (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeArrayFromDialog     (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeFloatAtom           (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeSymbolAtom          (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeComment             (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeScalar              (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeBang                (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeToggle              (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeSliderVertical      (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeSliderHorizontal    (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeRadioVertical       (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeRadioHorizontal     (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeVu                  (t_glist *, t_symbol *, int, t_atom *);
void canvas_makePanel               (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeDial                (t_glist *, t_symbol *, int, t_atom *);

void canvas_key                     (t_glist *, t_symbol *, int, t_atom *);
void canvas_motion                  (t_glist *, t_symbol *, int, t_atom *);
void canvas_mouseDown               (t_glist *, t_symbol *, int, t_atom *);
void canvas_mouseUp                 (t_glist *, t_symbol *, int, t_atom *);

void canvas_close                   (t_glist *, t_float);
void canvas_save                    (t_glist *, t_float);
void canvas_saveAs                  (t_glist *, t_float);
void canvas_saveToFile              (t_glist *, t_symbol *, int, t_atom *);

void canvas_cut                     (t_glist *);
void canvas_copy                    (t_glist *);
void canvas_paste                   (t_glist *);
void canvas_duplicate               (t_glist *);
void canvas_selectAll               (t_glist *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_functionProperties (t_gobj *, t_glist *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_click (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_visible (glist, 1);
}

static void canvas_coords (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    int flags   = (int)atom_getFloatAtIndex (6, argc, argv);
    int a       = (int)atom_getFloatAtIndex (7, argc, argv);
    int b       = (int)atom_getFloatAtIndex (8, argc, argv);
    int width   = (int)atom_getFloatAtIndex (4, argc, argv);
    int height  = (int)atom_getFloatAtIndex (5, argc, argv);
    
    int isGOP   = (flags & 1);
    
    t_rectangle r;
    
    bounds_setByAtoms (glist_getBounds (glist), argc, argv);
    
    #if PD_WITH_LEGACY
    
    if (!isGOP) {
    
        t_float scaleX = glist_getValueForOnePixelX (glist);
        t_float scaleY = glist_getValueForOnePixelY (glist);
        
        bounds_set (glist_getBounds (glist), (t_float)0.0, (t_float)0.0, PD_ABS (scaleX), PD_ABS (scaleY));
    }
    
    #endif
    
    rectangle_setByWidthAndHeight (&r, a, b, width, height);
    
    glist_setGraphGeometry (glist, &r, isGOP);
}

void canvas_restore (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *name = &s_;
    
    if (argc > 3) { name = dollar_getSymbolExpandedIfRequiered (argv + 3, instance_contextGetCurrent()); }
    
    glist_setName (glist, (name == &s_ ? sym_Patch : name));
    
    instance_stackPopPatch (glist, glist_isOpenedAtLoad (glist));

    {
        t_glist *parent = instance_contextGetCurrent();
    
        PD_ASSERT (parent);
        PD_ABORT (!parent);     /* Hot to manage corrupted files? */
        
        glist->gl_parent = parent;
        
        object_setBuffer (cast_object (glist), buffer_new());
        object_setX (cast_object (glist), atom_getFloatAtIndex (0, argc, argv));
        object_setY (cast_object (glist), atom_getFloatAtIndex (1, argc, argv));
        object_setWidth (cast_object (glist), 0);
        object_setType (cast_object (glist), TYPE_OBJECT);
        
        if (argc > 2) { buffer_deserialize (object_getBuffer (cast_object (glist)), argc - 2, argv + 2); }
        
        glist_objectAdd (parent, cast_gobj (glist));
    }
}

static void canvas_width (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (glist->gl_graphics) {
    //
    t_gobj *g1 = NULL;
    t_gobj *g2 = NULL;
    
    t_object *o = NULL;
    
    for ((g1 = glist->gl_graphics); (g2 = g1->g_next); (g1 = g2)) { }
    
    if ((o = cast_objectIfConnectable (g1))) {
    //
    int w = atom_getFloatAtIndex (0, argc, argv);
    
    object_setWidth (o, PD_MAX (1, w));
    
    if (glist_isOnScreen (glist)) {
        gobj_visibilityChanged (g1, glist, 0);
        gobj_visibilityChanged (g1, glist, 1);
    }
    //
    }
    //
    }
}

void canvas_connect (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 4) {
    //
    int indexOfObjectOut = (int)atom_getFloat (argv + 0);
    int indexOfOutlet    = (int)atom_getFloat (argv + 1);
    int indexOfObjectIn  = (int)atom_getFloat (argv + 2);
    int indexOfInlet     = (int)atom_getFloat (argv + 3);
    
    t_gobj *src  = glist_objectGetAt (glist, indexOfObjectOut);
    t_gobj *dest = glist_objectGetAt (glist, indexOfObjectIn);
    t_object *srcObject  = cast_objectIfConnectable (src);
    t_object *destObject = cast_objectIfConnectable (dest);
    
    if (srcObject && destObject) {
    //
    int m = indexOfOutlet;
    int n = indexOfInlet;
    t_outconnect *connection = NULL;
    
    /* Creates dummy outlets and inlets (failure at object creation). */
    
    if (pd_class (srcObject) == text_class && object_isObject (srcObject)) {
        while (m >= object_getNumberOfOutlets (srcObject)) {
            outlet_new (srcObject, NULL);
        }
    }
    
    if (pd_class (destObject) == text_class && object_isObject (destObject)) {
        while (n >= object_getNumberOfInlets (destObject)) {
            inlet_new (destObject, cast_pd (destObject), NULL, NULL);
        }
    }

    if ((connection = object_connect (srcObject, m, destObject, n))) {
    //
    if (glist_isOnScreen (glist)) {
        t_cord t; cord_make (&t, connection, srcObject, m, destObject, n, glist);
        glist_drawLine (glist, &t);
    }
    
    return;
    //
    }
    //
    }
    //
    }
    
    error_failed (sym_connect);
}

void canvas_disconnect (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 4) {
    //
    int indexOfObjectOut = (int)atom_getFloat (argv + 0);
    int indexOfOutlet    = (int)atom_getFloat (argv + 1);
    int indexOfObjectIn  = (int)atom_getFloat (argv + 2);
    int indexOfInlet     = (int)atom_getFloat (argv + 3);
    
    t_outconnect *connection = NULL;
    t_traverser t;
        
    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    if ((traverser_getIndexOfOutlet (&t) == indexOfOutlet)) {
        if ((traverser_getIndexOfInlet (&t) == indexOfInlet)) {

            int m = glist_objectGetIndexOf (glist, cast_gobj (traverser_getSource (&t)));
            int n = glist_objectGetIndexOf (glist, cast_gobj (traverser_getDestination (&t)));

            if (m == indexOfObjectOut && n == indexOfObjectIn) {
                glist_eraseLine (glist, traverser_getCord (&t));
                traverser_disconnect (&t);
                break;
            }
        }
    }
    //
    }
    //
    }
}

void canvas_rename (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *name = &s_;
    
    if (argc) {
    //
    name = dollar_getSymbolExpandedIfRequiered (argv, glist);
    if (name != &s_) { argc--; argv++; }
    if (argc) { warning_unusedArguments (class_getName (pd_class (glist)), argc, argv); }
    //
    }
    
    if (!utils_isNameAllowedForWindow (name)) { warning_badName (sym_pd, name); }
    
    glist_setName (glist, (name == &s_ ? sym_Patch : name));
}

static void canvas_window (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 4) {
    //
    rectangle_setByAtoms (glist_getWindowGeometry (glist), argc, argv);
    
    if (glist_isArray (glist)) { glist_updateWindow (glist); }
    //
    }
}

static void canvas_bounds (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 4) {
    //
    if (glist_isArray (glist)) {
    //
    t_error err = bounds_setByAtoms (glist_getBounds (glist), argc, argv);
    
    if (!err) { glist_updateGraphOnParent (glist); return; }
    //
    }
    //
    }
    
    error_invalid (sym_graph, sym_bounds); 
}

void canvas_remove (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *t = utils_makeTemplateIdentifier (atom_getSymbolAtIndex (0, argc, argv));
    
    if (!template_isPrivate (t)) {
    //
    t_template *template = template_findByIdentifier (t);
    
    if (template) {
        glist_objectRemoveByTemplate (glist, template); 
    }
    //
    }
}

static void canvas_open (t_glist *glist)
{
    /* Opening a GOP in its own window. */
    
    if (glist_isOnScreen (glist) && !glist_isWindowable (glist)) {      
    //
    PD_ASSERT (!glist_hasWindow (glist));
    
    gobj_visibilityChanged (cast_gobj (glist), glist_getParent (glist), 0);
    
    /* Temporary force the window state in order to properly drawn the content below. */
    
    glist_setWindow (glist, 1); 
    
    gobj_visibilityChanged (cast_gobj (glist), glist_getParent (glist), 1);
    
    glist_setWindow (glist, 0);
    //
    }
    
    canvas_visible (glist, 1);
}

static void canvas_loadbang (t_glist *glist)
{
    glist_loadbang (glist);
}

void canvas_clear (t_glist *glist)
{
    glist_objectRemoveAll (glist);
}

static void canvas_editmode (t_glist *glist, t_float f)
{
    int state = (int)(f != 0.0);
     
    if (glist_hasEditMode (glist) != state) {
    //
    glist_setEditMode (glist, state);
    
    if (state) { glist_drawAllCommentBars (glist); }
    else {
        glist_deselectAll (glist); glist_eraseAllCommentBars (glist);
    }
    
    if (glist_isOnScreen (glist)) {
        sys_vGui ("::ui_patch::setEditMode %s %d\n", glist_getTagAsString (glist), glist_hasEditMode (glist));
    }
    //
    }
}

void canvas_dirty (t_glist *glist, t_float f)
{
    glist_setDirty (glist, (int)f);
}

void canvas_visible (t_glist *glist, t_float f)
{
    int isVisible = (f != 0.0);
    
    if (isVisible) {

        if (glist_hasWindow (glist)) { sys_vGui ("::bringToFront .x%lx\n", glist); }
        else {
            
            sys_vGui ("::ui_patch::create .x%lx %d %d +%d+%d %d\n",     // --
                            glist,
                            rectangle_getWidth (glist_getWindowGeometry (glist)),
                            rectangle_getHeight (glist_getWindowGeometry (glist)),
                            rectangle_getTopLeftX (glist_getWindowGeometry (glist)),
                            rectangle_getTopLeftY (glist_getWindowGeometry (glist)),
                            glist_hasEditMode (glist));
                            
            glist_setWindow (glist, 1);
            
            glist_updateTitle (glist);
        }
        
    } else {

        if (glist_hasWindow (glist)) { 

            t_glist *t = NULL;
            
            glist_deselectAll (glist);
            if (glist_isOnScreen (glist)) { canvas_map (glist, 0); }

            sys_vGui ("destroy .x%lx\n", glist);
            
            if (glist_isGraphOnParent (glist) && (t = glist_getParent (glist)) && (!glist_isDeleting (t))) {
                if (glist_isOnScreen (t)) { gobj_visibilityChanged (cast_gobj (glist), t, 0); }
                glist_setWindow (glist, 0);
                if (glist_isOnScreen (t)) { gobj_visibilityChanged (cast_gobj (glist), t, 1); }
            } else {
                glist_setWindow (glist, 0);
            }
        }
    }
}

void canvas_map (t_glist *glist, t_float f)
{
    int isMapped = (f != 0.0);

    if (isMapped != glist_isOnScreen (glist)) {
    //
    if (!isMapped) { sys_vGui (".x%lx.c delete all\n", glist_getView (glist)); glist_setMapped (glist, 0); }
    else {
    
        t_gobj *y = NULL;
        t_selection *s = NULL;
        
        if (!glist_hasWindow (glist)) { PD_BUG; canvas_visible (glist, 1); }
        for (y = glist->gl_graphics; y; y = y->g_next) { gobj_visibilityChanged (y, glist, 1); }
        for (s = editor_getSelection (glist_getEditor (glist)); s; s = selection_getNext (s)) {
            gobj_selected (selection_getObject (s), glist, 1);
        }

        glist_setMapped (glist, 1);
        glist_drawAllLines (glist);
        glist_drawRectangle (glist);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_requireArrayDialog (t_glist *glist)
{
    char t[PD_STRING] = { 0 };
    t_symbol *s = utils_getDefaultBindName (garray_class, sym_array);
    t_error err = string_sprintf (t, PD_STRING, "::ui_array::show %%s %s 100 3\n", s->s_name);
    
    PD_ASSERT (!err);
    
    stub_new (cast_pd (glist), (void *)glist, t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_fromArrayDialog (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeArrayFromDialog (glist, s, argc, argv);
}

static void canvas_fromPopupDialog (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 3) {
    //
    int k = (int)atom_getFloat (argv + 0);
    int a = (int)atom_getFloat (argv + 1);
    int b = (int)atom_getFloat (argv + 2);
    
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
    //
    t_rectangle t;
    
    if (gobj_hit (y, glist, a, b, &t)) {
    //
    if (k == 0) {                                                                  /* Properties. */
        if (class_hasPropertiesFunction (pd_class (y))) {
            (*class_getPropertiesFunction (pd_class (y))) (y, glist); return;
        }
    } 
    if (k == 1) {                                                                  /* Open. */
        if (class_hasMethod (pd_class (y), sym_open)) {
            pd_message (cast_pd (y), sym_open, 0, NULL); return;
        }
    }
    if (k == 2) {                                                                  /* Help. */
    //
    char *directory = NULL;
    char name[PD_STRING] = { 0 };
    t_error err = PD_ERROR_NONE;
    
    if (pd_class (y) == canvas_class && glist_isTop (cast_glist (y))) {
        int ac = buffer_size (object_getBuffer (cast_object (y)));
        t_atom *av = buffer_atoms (object_getBuffer (cast_object (y)));
        if (!(err = (ac < 1))) {
            atom_toString (av, name, PD_STRING);
            directory = environment_getDirectoryAsString (glist_getEnvironment (cast_glist (y)));
        }
        
    } else {
        err = string_copy (name, PD_STRING, class_getHelpNameAsString (pd_class (y)));
        directory = class_getExternalDirectoryAsString (pd_class (y));
    }

    if (!err) { file_openHelp (directory, name); }
    
    return;
    //
    }
    //
    }
    //
    }
    
    if (k == 0) { canvas_functionProperties (cast_gobj (glist), NULL); }
    //
    }
}

static void canvas_fromDialog (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    t_error err = PD_ERROR_NONE;
    
    if (glist_hasParent (glist)) {
        
        /* Activated text box triggering recreation. */
        
        err = glist_objectDeselectIfSelected (glist_getParent (glist), cast_gobj (glist));
    }
    
    PD_ASSERT (argc == 11);
        
    if (!err) {
    //
    t_float scaleX  = atom_getFloatAtIndex (0, argc, argv);
    t_float scaleY  = atom_getFloatAtIndex (1, argc, argv);
    t_float start   = atom_getFloatAtIndex (3, argc, argv);
    t_float up      = atom_getFloatAtIndex (4, argc, argv);
    t_float end     = atom_getFloatAtIndex (5, argc, argv);
    t_float down    = atom_getFloatAtIndex (6, argc, argv);
    int isGOP       = (int)atom_getFloatAtIndex (2, argc,  argv);
    int width       = (int)atom_getFloatAtIndex (7, argc,  argv);
    int height      = (int)atom_getFloatAtIndex (8, argc,  argv);
    int marginX     = (int)atom_getFloatAtIndex (9, argc,  argv);
    int marginY     = (int)atom_getFloatAtIndex (10, argc, argv);
    
    t_rectangle r;
    
    rectangle_setByWidthAndHeight (&r, marginX, marginY, width, height);

    if (scaleX == 0.0) { scaleX = (t_float)1.0; }
    if (scaleY == 0.0) { scaleY = (t_float)1.0; }
    
    if (glist_isArray (glist)) { isGOP = 1; bounds_set (glist_getBounds (glist), start, up, end, down); }
    else {
        bounds_set (glist_getBounds (glist), (t_float)0.0, (t_float)0.0, PD_ABS (scaleX), PD_ABS (scaleY));
    }
    
    glist_setGraphGeometry (glist, &r, isGOP);
    
    glist_setDirty (glist, 1);
    
    if (glist_hasWindow (glist)) { glist_updateWindow (glist); }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_functionSave (t_gobj *x, t_buffer *b)
{
    int saveContents = !(glist_isAbstraction (cast_glist (x)));
    
    if (saveContents) { glist_serialize (cast_glist (x), b); }
    else {
        buffer_vAppend (b, "ssii",
            sym___hash__X,
            sym_obj,
            object_getX (cast_object (x)),
            object_getY (cast_object (x)));
        
        buffer_serialize (b, object_getBuffer (cast_object (x)));
        buffer_appendSemicolon (b);
        object_saveWidth (cast_object (x), b);
    }
}

static void canvas_functionProperties (t_gobj *x, t_glist *dummy)
{
    t_glist *glist = cast_glist (x);
    
    if (glist_isArray (glist)) {
    
        t_gobj *y = NULL;
            
        for (y = glist->gl_graphics; y; y = y->g_next) {
            if (pd_class (y) == garray_class) { garray_functionProperties ((t_garray *)y); }
        }
        
    } else {
    
        char t[PD_STRING] = { 0 };
        
        t_bounds *bounds  = glist_getBounds (glist);
        t_rectangle *r    = glist_getGraphGeometry (glist);
        
        t_error err = string_sprintf (t, PD_STRING, 
                            "::ui_canvas::show %%s %g %g %d %g %g %g %g %d %d %d %d\n",
                            bounds_getRight (bounds),
                            bounds_getBottom (bounds),
                            glist_isGraphOnParent (glist),
                            0.0,
                            0.0,
                            0.0,
                            0.0, 
                            rectangle_getWidth (r),
                            rectangle_getHeight (r),
                            rectangle_getTopLeftX (r),
                            rectangle_getTopLeftY (r));

        PD_ASSERT (!err);
    
        stub_new (cast_pd (glist), (void *)glist, t);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *canvas_newSubpatch (t_symbol *s)
{
    return glist_newPatchPop (s, NULL, NULL, NULL, 0, 0, 0);
}

void canvas_new (void *dummy, t_symbol *s, int argc, t_atom *argv)
{  
    t_rectangle r;
    
    rectangle_setByAtomsByWidthAndHeight (&r, argc, argv);
    
    glist_newPatch (atom_getSymbolAtIndex (4, argc, argv), 
        NULL, 
        NULL, 
        &r, 
        (int)atom_getFloatAtIndex (5, argc, argv), 
        0, 
        (int)atom_getFloatAtIndex (4, argc, argv));
}

static void canvas_free (t_glist *glist)
{
    if (glist_hasView (glist)) { canvas_visible (glist, 0); } 
    
    stub_destroyWithKey ((void *)glist);
    
    glist_deselectAll (glist);
    glist_objectRemoveAll (glist);
    
    glist_free (glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_setup (void)
{
    t_class *c = NULL;
        
    c = class_new (sym_canvas,
            NULL, 
            (t_method)canvas_free, 
            sizeof (t_glist), 
            CLASS_DEFAULT | CLASS_NOINLET,
            A_NULL);

    class_addCreator ((t_newmethod)canvas_newSubpatch, sym_pd, A_DEFSYMBOL, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addCreator ((t_newmethod)canvas_newSubpatch, sym_page, A_DEFSYMBOL, A_NULL);
        
    #endif
    
    class_addDSP (c, (t_method)canvas_dsp);
    class_addClick (c, (t_method)canvas_click);
    
    class_addMethod (c, (t_method)canvas_coords,                sym_coords,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_restore,               sym_restore,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_width,                 sym_f,                  A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_connect,               sym_connect,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_disconnect,            sym_disconnect,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_rename,                sym_rename,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeObject,            sym_obj,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeMessage,           sym_msg,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeArray,             sym_array,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeFloatAtom,         sym_floatatom,          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeSymbolAtom,        sym_symbolatom,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeComment,           sym_comment,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeScalar,            sym_scalar,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeBang,              sym_bng,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeToggle,            sym_tgl,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeSliderVertical,    sym_vslider,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeSliderHorizontal,  sym_hslider,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeRadioVertical,     sym_vradio,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeRadioHorizontal,   sym_hradio,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeVu,                sym_vu,                 A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makePanel,             sym_cnv,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeDial,              sym_nbx,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_key,                   sym_key,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_motion,                sym_motion,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_mouseDown,             sym_mouse,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_mouseUp,               sym_mouseup,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_window,                sym_window,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_bounds,                sym_bounds,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_remove,                sym_destroy,            A_GIMME, A_NULL);
    
    class_addMethod (c, (t_method)canvas_save,                  sym_save,               A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_saveAs,                sym_saveas,             A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_close,                 sym_close,              A_DEFFLOAT, A_NULL);
    
    class_addMethod (c, (t_method)canvas_open,                  sym_open,               A_NULL);
    class_addMethod (c, (t_method)canvas_loadbang,              sym_loadbang,           A_NULL);
    class_addMethod (c, (t_method)canvas_clear,                 sym_clear,              A_NULL);
    class_addMethod (c, (t_method)canvas_editmode,              sym_editmode,           A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_dirty,                 sym_dirty,              A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_visible,               sym_visible,            A_FLOAT, A_NULL);

    class_addMethod (c, (t_method)canvas_map,                   sym__map,               A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_saveToFile,            sym__savetofile,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_cut,                   sym__cut,               A_NULL);
    class_addMethod (c, (t_method)canvas_copy,                  sym__copy,              A_NULL);
    class_addMethod (c, (t_method)canvas_paste,                 sym__paste,             A_NULL);
    class_addMethod (c, (t_method)canvas_duplicate,             sym__duplicate,         A_NULL);
    class_addMethod (c, (t_method)canvas_selectAll,             sym__selectall,         A_NULL);
    
    class_addMethod (c, (t_method)canvas_requireArrayDialog,    sym__array,             A_NULL);
    class_addMethod (c, (t_method)canvas_fromArrayDialog,       sym__arraydialog,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_fromPopupDialog,       sym__popupdialog,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_fromDialog,            sym__canvasdialog,      A_GIMME, A_NULL);
   
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)canvas_open,                  sym_menu__dash__open,   A_NULL);
    class_addMethod (c, (t_method)canvas_makeComment,           sym_text,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeToggle,            sym_toggle,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeVu,                sym_vumeter,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makePanel,             sym_mycnv,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeDial,              sym_numbox,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_visible,               sym_vis,                A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_close,                 sym_menuclose,          A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_save,                  sym_menusave,           A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_saveAs,                sym_menusaveas,         A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_requireArrayDialog,    sym_menuarray,          A_NULL);

    #endif
    
    class_setWidgetBehavior (c, &glist_widgetbehavior);
    class_setSaveFunction (c, canvas_functionSave);
    class_setPropertiesFunction (c, canvas_functionProperties);

    canvas_class = c;
}

void canvas_destroy (void)
{
    CLASS_FREE (canvas_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

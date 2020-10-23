
/* Copyright (c) 1997-2020 Miller Puckette and others. */

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

void glist_updateCursor (t_glist *, int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_box *editor_boxFetch (t_editor *x, t_object *object)
{
    t_box *box = NULL;
    
    for (box = x->e_boxes; box && box->box_object != object; box = box->box_next) { }
    
    PD_ASSERT (box);
    
    return box;
}

void editor_boxAdd (t_editor *x, t_object *object)
{
    t_box *box = (t_box *)PD_MEMORY_GET (sizeof (t_box));

    box->box_next   = x->e_boxes;
    box->box_object = object;
    box->box_owner  = x->e_owner;
    
    buffer_toStringUnzeroed (object_getBuffer (object), &box->box_string, &box->box_stringSizeInBytes);
    
    {
    //
    t_glist *view = glist_getView (x->e_owner);
    t_error err = string_sprintf (box->box_tag, BOX_TAG_SIZE, "%s.%lxBOX", glist_getTagAsString (view), box);
    PD_UNUSED (err); PD_ASSERT (!err);
    //
    }
    
    x->e_boxes = box;
}

void editor_boxRemove (t_editor *x, t_box *box)
{
    editor_boxUnselect (x, box);
    
    if (x->e_boxes == box) { x->e_boxes = box->box_next; }
    else {
        t_box *t = NULL;
        for (t = x->e_boxes; t; t = t->box_next) {
            if (t->box_next == box) { t->box_next = box->box_next; break; }
        }
    }

    PD_MEMORY_FREE (box->box_string);
    PD_MEMORY_FREE (box);
}

void editor_boxSelect (t_editor *x, t_box *box)
{
    x->e_selectedBox = box; x->e_isSelectedBoxDirty = 0;
}

void editor_boxUnselect (t_editor *x, t_box *box)
{
    if (x->e_selectedBox == box) { editor_boxSelect (x, NULL); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void editor_selectionAdd (t_editor *x, t_gobj *y)
{
    t_selection *s = (t_selection *)PD_MEMORY_GET (sizeof (t_selection));
    
    s->sel_next = x->e_selectedObjects;
    s->sel_what = y;
    
    x->e_selectedObjects = s;
}

int editor_selectionRemove (t_editor *x, t_gobj *y)
{
    t_selection *s1 = x->e_selectedObjects;
    t_selection *s2 = NULL;
    
    if (s1) {
    //
    if (selection_getObject (s1) == y) {
        x->e_selectedObjects = selection_getNext (x->e_selectedObjects);
        PD_MEMORY_FREE (s1);
        return 1;
        
    } else {
        for (; (s2 = selection_getNext (s1)); (s1 = s2)) {
            if (selection_getObject (s2) == y) {
                s1->sel_next = selection_getNext (s2);
                PD_MEMORY_FREE (s2);
                return 1;
            }
        }
    }
    //
    }
    
    return 0;
}

void editor_selectionDeplace (t_editor *x)
{
    int deltaX = drag_getMoveX (editor_getDrag (x));
    int deltaY = drag_getMoveY (editor_getDrag (x));
    
    if (snap_hasSnapToGrid()) {
    //
    if (drag_hasMovedOnce (editor_getDrag (x))) { glist_objectSnapSelected (x->e_owner, 0); }
    //
    }
    
    glist_objectDisplaceSelected (x->e_owner, deltaX, deltaY);
        
    drag_close (editor_getDrag (x));
}

void editor_selectionCacheLines (t_editor *x)
{
    t_traverser t;
    
    buffer_clear (x->e_cachedLines);
    
    traverser_start (&t, x->e_owner);
    
    while (traverser_next (&t)) {
    //
    t_gobj *o = cast_gobj (traverser_getSource (&t));
    t_gobj *d = cast_gobj (traverser_getDestination (&t));
    int s1 = glist_objectIsSelected (x->e_owner, o);
    int s2 = glist_objectIsSelected (x->e_owner, d);
    
    if (s1 != s2) {
    //
    buffer_appendSymbol (x->e_cachedLines, sym___hash__X);
    buffer_appendSymbol (x->e_cachedLines, sym_connect);
    buffer_appendFloat (x->e_cachedLines,  glist_objectGetIndexOf (x->e_owner, o));
    buffer_appendFloat (x->e_cachedLines,  traverser_getIndexOfOutlet (&t));
    buffer_appendFloat (x->e_cachedLines,  glist_objectGetIndexOf (x->e_owner, d));
    buffer_appendFloat (x->e_cachedLines,  traverser_getIndexOfInlet (&t));
    buffer_appendSemicolon (x->e_cachedLines);
    //
    }
    //
    }
}

void editor_selectionRestoreLines (t_editor *x)
{
    instance_loadSnippet (x->e_owner, x->e_cachedLines);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void editor_selectedLineSet (t_editor *x, t_outconnect *connection, int m, int i, int n, int j)
{
    x->e_selectedLineConnection = connection;
    
    x->e_selectedLine[0] = m;
    x->e_selectedLine[1] = i;
    x->e_selectedLine[2] = n;
    x->e_selectedLine[3] = j;
}

void editor_selectedLineReset (t_editor *x)
{
    editor_selectedLineSet (x, NULL, 0, 0, 0, 0);
}

void editor_selectedLineDisconnect (t_editor *x)
{
    int m = x->e_selectedLine[0];
    int i = x->e_selectedLine[1];
    int n = x->e_selectedLine[2];
    int j = x->e_selectedLine[3];
    
    glist_lineDisconnect (x->e_owner, m, i, n, j);
    
    editor_selectedLineReset (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void editor_graphSetSelected (t_editor *x, int isSelected)
{
    int old = x->e_isSelectedGraph;
    
    x->e_isSelectedGraph = (isSelected != 0);
    
    if (old != x->e_isSelectedGraph) {
    //
    glist_updateRectangle (x->e_owner);
    
    if (!isSelected) { glist_updateCursor (x->e_owner, CURSOR_NOTHING); }
    //
    }
}

int editor_graphHit (t_editor *x, int a, int b)
{
    t_rectangle *r = glist_getGraphGeometry (x->e_owner);
    
    return rectangle_contains (r, a, b);
}

int editor_graphHitRightSide (t_editor *x, int a, int b)
{
    t_rectangle t1, t2;
    
    rectangle_setCopy (&t1, glist_getGraphGeometry (x->e_owner));
    rectangle_setCopy (&t2, glist_getGraphGeometry (x->e_owner));
    
    rectangle_enlargeRight (&t1, -EDIT_GRIP_SIZE); rectangle_enlargeRight (&t2, EDIT_GRIP_SIZE);
    
    return (rectangle_contains (&t2, a, b) && !rectangle_contains (&t1, a, b));
}

void editor_graphDeplace (t_editor *x, int a, int b)
{
    t_rectangle *r = glist_getGraphGeometry (x->e_owner);
    
    if (a || b) { rectangle_deplace (r, a, b); glist_updateRectangle (x->e_owner); }
}

void editor_graphSetBottomRight (t_editor *x, int c, int d)
{
    t_rectangle *r = glist_getGraphGeometry (x->e_owner);
    
    int a = rectangle_getTopLeftX (r);
    int b = rectangle_getTopLeftY (r);
    
    rectangle_set (r, a, b, c, d);
    
    glist_updateRectangle (x->e_owner);
}

void editor_graphSetRectangle (t_editor *x, t_rectangle *r)
{
    rectangle_setCopy (glist_getGraphGeometry (x->e_owner), r);
    
    glist_updateRectangle (x->e_owner);
}

void editor_graphSnap (t_editor *x)
{
    t_rectangle *r = glist_getGraphGeometry (x->e_owner);
    
    int m = snap_getOffset (rectangle_getTopLeftX (r));
    int n = snap_getOffset (rectangle_getTopLeftY (r));
    
    if (m || n) { rectangle_deplace (r, m, n); glist_updateRectangle (x->e_owner); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void editor_motionSet (t_editor *x, t_gobj *y, t_glist *glist, t_motionfn callback, int a, int b)
{
    PD_ASSERT (callback);
    
    editor_startAction (x, ACTION_PASS, a, b, y);
    
    x->e_grabbed        = y;
    x->e_grabbedOwner   = glist;
    x->e_fnMotion       = callback;
}

void editor_motionReset (t_editor *x)
{
    drag_close (editor_getDrag (x));
    
    x->e_grabbed        = NULL;
    x->e_grabbedOwner   = NULL;
    x->e_fnMotion       = NULL;
    
    editor_resetAction (x);
}

void editor_motionUnset (t_editor *x, t_gobj *y)
{
    if (!x->e_grabbed || x->e_grabbed == y) { editor_motionReset (x); }
}

void editor_motionProceed (t_editor *x, int a, int b, int m)
{
    if (!x->e_fnMotion) { PD_BUG; }
    else {
        int endX = drag_getEndX (editor_getDrag (x));
        int endY = drag_getEndY (editor_getDrag (x));
        int deltaX = a - endX;
        int deltaY = b - endY;
    
        /* Could be NULL (for instance to plot arrays). */
        /* For scalars the grabbed object is the painter. */
        
        if (x->e_grabbed && x->e_grabbedOwner && !class_hasPainterBehavior (pd_class (x->e_grabbed))) {
        //
        t_rectangle r;

        gobj_getRectangle (x->e_grabbed, x->e_grabbedOwner, &r);
    
        if (rectangle_containsX (&r, a)) { m |= MODIFIER_INSIDE_X; }
        if (rectangle_containsY (&r, b)) { m |= MODIFIER_INSIDE_Y; }
        //
        }
        
        (*x->e_fnMotion) (cast_pd (x->e_grabbed), deltaX, deltaY, m);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_editor *editor_new (t_glist *owner)
{
    t_editor *x = (t_editor *)PD_MEMORY_GET (sizeof (t_editor));
    
    t_gobj *y = NULL;
    
    x->e_owner       = owner;
    x->e_proxy       = proxy_new (cast_pd (owner));
    x->e_cachedLines = buffer_new();
    
    for (y = owner->gl_graphics; y; y = y->g_next) {
        if (cast_objectIfConnectable (y)) { editor_boxAdd (x, cast_object (y)); }
    }
    
    return x;
}

void editor_free (t_editor *x)
{
    t_box *box = NULL;
    
    while ((box = x->e_boxes)) { editor_boxRemove (x, box); }
    
    buffer_free (x->e_cachedLines);
    proxy_release (x->e_proxy);
    
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

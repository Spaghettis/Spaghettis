
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void canvas_behaviorVisibilityChanged   (t_gobj *, t_glist *, int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_updateTitle (t_glist *glist)
{
    if (glist_hasWindow (glist)) {

        sys_vGui ("::ui_patch::setTitle %s {%s} {%s} %d\n",  // --
                        glist_getTagAsString (glist),
                        environment_getDirectoryAsString (glist_getEnvironment (glist)),
                        glist_getName (glist)->s_name,
                        glist_getDirty (glist));
    }
}

void glist_updateWindow (t_glist *glist)
{
    if (glist_isWindowable (glist) && glist_isOnScreen (glist)) { 
        canvas_map (glist, 0);
        canvas_map (glist, 1);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void glist_updateGraphOnParent (t_glist *glist)
{  
    if (glist_isOnScreen (glist)) {
    //
    if (glist_hasWindow (glist)) { glist_updateWindow (glist); }
    else {
        PD_ASSERT (glist_hasParentOnScreen (glist));
        canvas_behaviorVisibilityChanged (cast_gobj (glist), glist_getParent (glist), 0); 
        canvas_behaviorVisibilityChanged (cast_gobj (glist), glist_getParent (glist), 1);
    }
    //
    }
}

void glist_updateRectangle (t_glist *glist)
{
    if (glist_isGraphOnParent (glist) && glist_hasWindow (glist)) {
    //
    if (!glist_isArray (glist)) {
    //
    sys_vGui ("%s.c delete RECTANGLE\n", glist_getTagAsString (glist));
    
    glist_drawRectangle (glist);
    //
    }
    //
    }
}

void glist_updateLasso (t_glist *glist, int a, int b)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c coords LASSO %d %d %d %d\n",
                    glist_getTagAsString (glist),
                    drag_getStartX (editor_getDrag (glist_getEditor (glist))),
                    drag_getStartY (editor_getDrag (glist_getEditor (glist))),
                    a,
                    b);
    //
    }
    //
    }
}

void glist_updateTemporary (t_glist *glist, int a, int b, int c, int d)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c coords TEMPORARY %d %d %d %d\n",
                    glist_getTagAsString (glist),
                    a,
                    b,
                    c,
                    d);
    //
    }
    //
    }
}

void glist_updateLineSelected (t_glist *glist, int isSelected)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (isSelected) {
    
    sys_vGui ("%s.c itemconfigure %lxLINE -fill blue\n",
                    glist_getTagAsString (glist),
                    editor_getSelectedLineConnection (glist_getEditor (glist)));
                    
    } else {
    
    sys_vGui ("%s.c itemconfigure %lxLINE -fill black\n",
                    glist_getTagAsString (glist),
                    editor_getSelectedLineConnection (glist_getEditor (glist)));
    }
    //
    }
}

void glist_updateLine (t_glist *glist, t_cord *c)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c coords %lxLINE %d %d %d %d\n",
                    glist_getTagAsString (glist),
                    cord_getConnection (c),
                    cord_getStartX (c),
                    cord_getStartY (c),
                    cord_getEndX (c),
                    cord_getEndY (c));

    sys_vGui ("%s.c itemconfigure %lxLINE -width %d\n",
                    glist_getTagAsString (glist),
                    cord_getConnection (c),
                    cord_isSignal (c) ? 2 : 1);
    //
    }
    //
    }
}

void glist_updateLinesForObject (t_glist *glist, t_object *o)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    t_outconnect *connection = NULL;
    t_traverser t;

    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    if (traverser_getSource (&t) == o || traverser_getDestination (&t) == o) {
        glist_updateLine (glist, traverser_getCord (&t));
    }
    //
    }
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_drawRectangle (t_glist *glist)
{
    if (glist_isGraphOnParent (glist) && glist_hasWindow (glist)) {
    //
    if (!glist_isArray (glist)) {
    //
    int a = rectangle_getTopLeftX (glist_getGraphGeometry (glist));
    int b = rectangle_getTopLeftY (glist_getGraphGeometry (glist));
    int c = rectangle_getBottomRightX (glist_getGraphGeometry (glist));
    int d = rectangle_getBottomRightY (glist_getGraphGeometry (glist));
    
    sys_vGui ("%s.c create line %d %d %d %d %d %d %d %d %d %d"
                    " -dash {2 4}"  // --
                    " -fill #%06x"
                    " -tags RECTANGLE\n",
                    glist_getTagAsString (glist),
                    a,
                    b,
                    c,
                    b,
                    c,
                    d,
                    a,
                    d,
                    a,
                    b, 
                    COLOR_GOP);
    }
    //
    }
}

void glist_drawLasso (t_glist *glist, int a, int b)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c create rectangle %d %d %d %d -tags LASSO\n",
                    glist_getTagAsString (glist),
                    a,
                    b,
                    a,
                    b);
    //
    }
    //
    }
}

void glist_drawTemporary (t_glist *glist, int a, int b, int isSignal)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c create line %d %d %d %d -width %d -tags TEMPORARY\n",
                    glist_getTagAsString (glist),
                    a,
                    b,
                    a,
                    b,
                    isSignal ? 2 : 1);
    //
    }
    //
    }
}

void glist_drawLine (t_glist *glist, t_cord *c)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c create line %d %d %d %d -width %d -tags %lxLINE\n",
                    glist_getTagAsString (glist),
                    cord_getStartX (c),
                    cord_getStartY (c),
                    cord_getEndX (c),
                    cord_getEndY (c),
                    cord_isSignal (c) ? 2 : 1,
                    cord_getConnection (c));
    //
    }
    //
    }
}

void glist_drawAllLines (t_glist *glist)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    t_outconnect *connection = NULL;
    t_traverser t;

    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) { glist_drawLine (glist, traverser_getCord (&t)); }
    //
    }
    //
    }
}

void glist_drawAllCommentBars (t_glist *glist)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        t_object *o = NULL;
        if ((o = cast_objectIfConnectable (y)) && object_isComment (o)) {
            box_draw (box_fetch (glist, o));
        }
    }
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_eraseLasso (t_glist *glist)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c delete LASSO\n", glist_getTagAsString (glist));
    //
    }
    //
    }
}

void glist_eraseTemporary (t_glist *glist)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c delete TEMPORARY\n", glist_getTagAsString (glist));
    //
    }
    //
    }
}

void glist_eraseLine (t_glist *glist, t_cord *c)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c delete %lxLINE\n", glist_getTagAsString (glist), cord_getConnection (c));
    //
    }
    //
    }
}

void glist_eraseAllCommentBars (t_glist *glist)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c delete COMMENTBAR\n", glist_getTagAsString (glist));
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------


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
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *canvas_class;
extern t_class *scalar_class;
extern t_class *garray_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_widgetbehavior     text_widgetBehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void gobj_getRectangle (t_gobj *x, t_glist *owner, int *a, int *b, int *c, int *d)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnGetRectangle) {
        (*(pd_class (x)->c_behavior->w_fnGetRectangle)) (x, owner, a, b, c, d);
    }
}

void gobj_displaced (t_gobj *x, t_glist *owner, int deltaX, int deltaY)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnDisplaced) {
        (*(pd_class (x)->c_behavior->w_fnDisplaced)) (x, owner, deltaX, deltaY);
    }
}

void gobj_selected (t_gobj *x, t_glist *owner, int isSelected)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnSelected) {
        (*(pd_class (x)->c_behavior->w_fnSelected)) (x, owner, isSelected);
    }
}

void gobj_activated (t_gobj *x, t_glist *owner, int isActivated)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnActivated) {
        (*(pd_class (x)->c_behavior->w_fnActivated)) (x, owner, isActivated);
    }
}

void gobj_deleted (t_gobj *x, t_glist *owner)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnDeleted) {
        (*(pd_class (x)->c_behavior->w_fnDeleted)) (x, owner);
    }
}

int gobj_clicked (t_gobj *x, t_glist *owner, int a, int b, int shift, int ctrl, int alt, int dbl, int clicked)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnClicked) {
        return ((*(pd_class (x)->c_behavior->w_fnClicked)) (x, owner, a, b, shift, ctrl, alt, dbl, clicked));
    } else {
        return 0;
    }
}

void gobj_save (t_gobj *x, t_buffer *buffer)
{
    if (pd_class (x)->c_fnSave) {
        (*(pd_class (x)->c_fnSave)) (x, buffer);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int gobj_hit (t_gobj *x,
    t_glist *owner, 
    int positionX,
    int positionY,
    int *a,
    int *b,
    int *c,
    int *d)
{
    if (gobj_isVisible (x, owner)) {
    //
    int x1, y1, x2, y2;
        
    gobj_getRectangle (x, owner, &x1, &y1, &x2, &y2);
    
    if (positionX >= x1 && positionX <= x2 && positionY >= y1 && positionY <= y2) {
    //
    *a = x1;
    *b = y1;
    *c = x2;
    *d = y2;
    
    return 1;
    //
    }
    //
    }
    
    return 0;
}

int gobj_isVisible (t_gobj *x, t_glist *owner)
{
    if (canvas_isDrawnOnParent (owner)) {
    //
    t_object *object = NULL;
            
    /* Is parent visible? */
    
    if (!gobj_isVisible (cast_gobj (owner), owner->gl_parent)) { return 0; }
    
    /* Falling outside the graph rectangle? */
    
    if (pd_class (x) == scalar_class || pd_class (x) == garray_class) { return 1; }
    else {
    //
    int a, b, c, d, e, f, g, h;
        
    gobj_getRectangle (cast_gobj (owner), owner->gl_parent, &a, &b, &c, &d);
    
    if (a > c) { int t = a; a = c; c = t; }
    if (b > d) { int t = b; b = d; d = t; }
    
    gobj_getRectangle (x, owner, &e, &f, &g, &h);
    
    if (e < a || e > c || g < a || g > c || f < b || f > d || h < b || h > d) { return 0; }
    //
    }
    
    if (object = canvas_castToObjectIfPatchable (x)) {
    //
    if (canvas_objectIsBox (object)) {
    // 
    if (object->te_type != TYPE_COMMENT) { return 0; }
    //
    }
    //
    }
    //
    }

    return 1;
}

void gobj_visibilityChanged (t_gobj *x, t_glist *owner, int isVisible)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnVisibilityChanged) {
        if (gobj_isVisible (x, owner)) {
            (*(pd_class (x)->c_behavior->w_fnVisibilityChanged)) (x, owner, isVisible);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

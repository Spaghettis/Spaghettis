
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __g_editor_h_
#define __g_editor_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef void (*t_motionfn)  (void *z, t_float deltaX, t_float deltaY, t_float modifier);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _selection {
    t_gobj              *sel_what;
    struct _selection   *sel_next;
    } t_selection;
    
typedef struct _editor {
    t_glist             *e_owner;
    t_proxy             *e_proxy;
    t_box               *e_boxes;
    t_box               *e_selectedBox;
    t_selection         *e_selectedObjects;
    t_gobj              *e_grabbed;
    t_glist             *e_grabbedOwner;
    t_buffer            *e_cachedLines;
    t_outconnect        *e_selectedLineConnection;
    int                 e_selectedLine[4];
    t_motionfn          e_fnMotion;
    t_drag              e_drag;
    int                 e_action;
    int                 e_isSelectedBoxDirty;
    int                 e_isSelectedGraph;
    } t_editor;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define EDIT_GRIP_SIZE  5

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_editor    *editor_new                         (t_glist *owner);
t_box       *editor_boxFetch                    (t_editor *x, t_object *object);

void        editor_boxAdd                       (t_editor *x, t_object *object);
void        editor_boxRemove                    (t_editor *x, t_box *box);
void        editor_boxSelect                    (t_editor *x, t_box *box);
void        editor_boxUnselect                  (t_editor *x, t_box *box);

void        editor_free                         (t_editor *x);
void        editor_selectionAdd                 (t_editor *x, t_gobj *y);
int         editor_selectionRemove              (t_editor *x, t_gobj *y);
void        editor_selectionDeplace             (t_editor *x);
void        editor_selectionCacheLines          (t_editor *x);
void        editor_selectionRestoreLines        (t_editor *x);

void        editor_selectedLineReset            (t_editor *x);
void        editor_selectedLineDisconnect       (t_editor *x);
void        editor_selectedLineSet              (t_editor *x, 
                                                    t_outconnect *connection,
                                                    int m,
                                                    int i,
                                                    int n,
                                                    int j);

void        editor_graphSetSelected             (t_editor *x, int isSelected);
int         editor_graphHit                     (t_editor *x, int a, int b);
int         editor_graphHitRightSide            (t_editor *x, int a, int b);
void        editor_graphDeplace                 (t_editor *x, int a, int b);
void        editor_graphSetBottomRight          (t_editor *x, int a, int b);
void        editor_graphSetRectangle            (t_editor *x, t_rectangle *r);
void        editor_graphSnap                    (t_editor *x);

void        editor_motionProceed                (t_editor *x, int deltaX, int deltaY, int m);
void        editor_motionSet                    (t_editor *x,
                                                    t_gobj  *y,
                                                    t_glist *glist,
                                                    t_motionfn callback,
                                                    int a,
                                                    int b);
                                                    
void        editor_motionUnset                  (t_editor *x, t_gobj *y);
void        editor_motionReset                  (t_editor *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int editor_hasSelectedBox (t_editor *x)
{
    return (x->e_selectedBox != NULL);
}

static inline int editor_hasSelectedBoxDirty (t_editor *x)
{
    return x->e_isSelectedBoxDirty;
}

static inline int editor_hasSelection (t_editor *x)
{
    return (x->e_selectedObjects != NULL);
}

static inline int editor_hasSelectedLine (t_editor *x)
{
    return (x->e_selectedLineConnection != NULL);
}

static inline int editor_hasSelectedGraph (t_editor *x)
{
    return x->e_isSelectedGraph;
}

static inline int editor_hasAction (t_editor *x)
{
    return (x->e_action != ACTION_NONE);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_box *editor_getSelectedBox (t_editor *x)
{
    return x->e_selectedBox;
}

static inline t_selection *editor_getSelection (t_editor *x)
{
    return x->e_selectedObjects;
}

static inline t_outconnect *editor_getSelectedLineConnection (t_editor *x)
{
    return x->e_selectedLineConnection;
}

static inline t_drag *editor_getDrag (t_editor *x)
{
    return &x->e_drag;
}

static inline int editor_getAction (t_editor *x)
{
    return x->e_action;
}

static inline t_symbol *editor_getTag (t_editor *x)
{
    return proxy_getTag (x->e_proxy);
}

static inline const char *editor_getTagAsString (t_editor *x)
{
    return proxy_getTagAsString (x->e_proxy);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void editor_setSelectedBoxDirty (t_editor *x)
{
    x->e_isSelectedBoxDirty = 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void editor_startAction (t_editor *x, int n, int a, int b, t_gobj *y)
{
    x->e_action = n; drag_begin (editor_getDrag (x), a, b); drag_setObject (editor_getDrag (x), y);
}

static inline void editor_resetAction (t_editor *x)
{
    x->e_action = ACTION_NONE;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_gobj *selection_getObject (t_selection *x)
{
    return x->sel_what;
}

static inline t_selection *selection_getNext (t_selection *x)
{
    return x->sel_next;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_editor_h_


/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __m_object_h_
#define __m_object_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_object *object_setFromEntry               (t_object *x, t_glist *glist, t_box *z);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int     object_setSnappedX                  (t_object *x, int n);
int     object_setSnappedY                  (t_object *x, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_outconnect    *object_connect             (t_object *src, int m, t_object *dest, int n);

t_error object_disconnect                   (t_object *src, int m, t_object *dest, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int     object_getNumberOfInlets            (t_object *x);
int     object_getNumberOfOutlets           (t_object *x);
int     object_getNumberOfSignalInlets      (t_object *x);
int     object_getNumberOfSignalOutlets     (t_object *x);
int     object_getIndexAsSignalOfInlet      (t_object *x, int m);
int     object_getIndexAsSignalOfOutlet     (t_object *x, int m);
int     object_isSignalInlet                (t_object *x, int m);
int     object_isSignalOutlet               (t_object *x, int m);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    object_serializeWidth               (t_object *x, t_buffer *b);
void    object_distributeAtomsOnInlets      (t_object *x, int argc, t_atom *argv);

t_float *object_getSignalAtIndex            (t_object *x, int m);

void    object_getSignalValues              (t_object *x, t_buffer *b, int n);
void    object_setSignalValues              (t_object *x, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Viewed as a box (NOT an IEM and NOT a subpatch GOP). */
/* Note that it can be a comment, a message or an atom. */

int object_isViewedAsBox (t_object *x);

/* A badly created box object. */

int obj_isDummy (t_gobj *x);
int object_isDummy (t_object *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Everything that is NOT a comment, a message, or an atom. */

static inline int object_isObject (t_object *x)
{
    return (x->te_type == TYPE_OBJECT);
}

static inline int object_isComment (t_object *x)
{
    return (x->te_type == TYPE_COMMENT);
}

static inline int object_isMessage (t_object *x)
{
    return (x->te_type == TYPE_MESSAGE);
}

static inline int object_isAtom (t_object *x)
{
    return (x->te_type == TYPE_ATOM);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_buffer *object_getBuffer (t_object *x)
{
    PD_ASSERT (x->te_buffer != NULL); return x->te_buffer;
}

static inline t_inlet *object_getInlets (t_object *x)
{
    return x->te_inlets;
}

static inline t_outlet *object_getOutlets (t_object *x)
{
    return x->te_outlets;
}

static inline int object_getX (t_object *x)
{
    return x->te_x;
}

static inline int object_getY (t_object *x)
{
    return x->te_y;
}

static inline int object_getWidth (t_object *x)
{
    return x->te_width;
}

static inline t_objecttype object_getType (t_object *x)
{
    return x->te_type;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void object_setBuffer (t_object *x, t_buffer *b)      /* Acquires ownership. */
{
    if (x->te_buffer) { buffer_free (x->te_buffer); } 
    
    PD_ASSERT (b);
    
    x->te_buffer = b;
}

static inline void object_setX (t_object *x, int n)
{
    x->te_x = n;
}

static inline void object_setY (t_object *x, int n)
{
    x->te_y = n;
}

static inline void object_setWidth (t_object *x, int n)
{
    x->te_width = n;
}

static inline void object_setType (t_object *x, t_objecttype n)
{
    x->te_type = n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_object_h_

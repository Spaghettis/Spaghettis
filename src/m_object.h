
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __m_object_h_
#define __m_object_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

enum {
    TYPE_COMMENT    = 0,
    TYPE_OBJECT     = 1,
    TYPE_MESSAGE    = 2,
    TYPE_ATOM       = 3
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _outconnect {
    struct _outconnect          *oc_next;
    t_pd                        *oc_receiver;
    };

struct _outlet {
    struct _outlet              *o_next;
    t_object                    *o_owner;
    t_outconnect                *o_connections;
    t_symbol                    *o_type;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _inlet {
    t_pd                        i_pd;                   /* MUST be the first. */
    struct _inlet               *i_next;
    t_object                    *i_owner;
    t_pd                        *i_receiver;
    t_symbol                    *i_type;
    union {
        t_symbol                *i_method;
        t_gpointer              *i_pointer;
        t_float                 *i_float;
        t_symbol                **i_symbol;
        t_float                 i_signal;
    } i_un;
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_outconnect    *object_connect                     (t_object *src, int m, t_object *dest, int n);

void        object_disconnect                       (t_object *src, int m, t_object *dest, int n);

int         object_getNumberOfInlets                (t_object *x);
int         object_getNumberOfOutlets               (t_object *x);
int         object_getNumberOfSignalInlets          (t_object *x);
int         object_getNumberOfSignalOutlets         (t_object *x);
int         object_getSignalIndexOfInlet            (t_object *x, int m);
int         object_getSignalIndexOfOutlet           (t_object *x, int m);
int         object_isSignalInlet                    (t_object *x, int m);
int         object_isSignalOutlet                   (t_object *x, int m);

void        object_saveWidth                        (t_object *x, t_buffer *b);
void        object_distributeOnInlets               (t_object *x, int argc, t_atom *argv);

t_float     *object_getSignalValueAtIndex           (t_object *x, int m);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Viewed as a box (NOT an IEM and NOT a subpatch GOP). */
/* Note that it can be a comment, a message or an atom. */

int object_isBox (t_object *x);

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
#pragma mark -

static inline t_buffer *object_getBuffer (t_object *x)
{
    return x->te_buffer;
}

static inline t_inlet *object_getFirstInlet (t_object *x)
{
    return x->te_inlets;
}

static inline t_outlet *object_getFirstOutlet (t_object *x)
{
    return x->te_outlets;
}

static inline int object_getX (t_object *x)
{
    return x->te_xCoordinate;
}

static inline int object_getY (t_object *x)
{
    return x->te_yCoordinate;
}

static inline int object_getWidth (t_object *x)
{
    return x->te_width;
}

static inline int object_getType (t_object *x)
{
    return x->te_type;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline void object_setBuffer (t_object *x, t_buffer *b)
{
    x->te_buffer = b;       /* Acquires ownership. */
}

static inline void object_setX (t_object *x, int n)
{
    x->te_xCoordinate = n;
}

static inline void object_setY (t_object *x, int n)
{
    x->te_yCoordinate = n;
}

static inline void object_setWidth (t_object *x, int n)
{
    x->te_width = n;
}

static inline void object_setType (t_object *x, int n)
{
    x->te_type = n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_inlet     *inlet_newSignalDefault             (t_object *owner, t_float f);
t_inlet     *inlet_new                          (t_object *owner, t_pd *receiver, t_symbol *t, t_symbol *m);

void        inlet_free                          (t_inlet *x);
void        inlet_moveFirst                     (t_inlet *x);
int         inlet_isSignal                      (t_inlet *x);
int         inlet_getSignalIndex                (t_inlet *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline t_inlet *inlet_getNext (t_inlet *x)
{
    return x->i_next;
}

static inline t_object *inlet_getOwner (t_inlet *x)
{
    return x->i_owner;
}

static inline t_float *inlet_getSignalValue (t_inlet *x)
{
    return &x->i_un.i_signal;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_outconnect    *outlet_addConnection           (t_outlet *x, t_pd *receiver);

void            outlet_removeConnection         (t_outlet *x, t_pd *receiver);
void            outlet_free                     (t_outlet *x);
void            outlet_moveFirst                (t_outlet *x);
int             outlet_isSignal                 (t_outlet *x);
int             outlet_getSignalIndex           (t_outlet *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline t_outlet *outlet_getNext (t_outlet *x)
{
    return x->o_next;
}

static inline t_outconnect *outlet_getConnections (t_outlet *x)
{
    return x->o_connections;
}

static inline t_outconnect *connection_getNext (t_outconnect *x)
{
    return x->oc_next;
}

static inline t_pd *connection_getReceiver (t_outconnect *x)
{
    return x->oc_receiver;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_object_h_

/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __m_core_h_
#define __m_core_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _mouse {
    int         m_x;
    int         m_y;
    int         m_shift;
    int         m_ctrl;
    int         m_alt;
    int         m_dbl;
    int         m_clicked;
    t_atom      m_atoms[7];
    } t_mouse;

typedef struct _rectangle {
    int         rect_topLeftX;
    int         rect_topLeftY;
    int         rect_bottomRightX;
    int         rect_bottomRightY;
    int         rect_isNothing;
    } t_rectangle;

typedef struct _bounds {
    t_float     b_left;
    t_float     b_top;
    t_float     b_right;
    t_float     b_bottom;
    } t_bounds;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _pdinstance {
    t_systime   pd_systime;
    int         pd_dspState;
    int         pd_dspChainSize;
    t_int       *pd_dspChain;
    t_clock     *pd_clocks;
    t_signal    *pd_signals;
    t_glist     *pd_roots;
    t_clock     *pd_polling;
    t_clock     *pd_autorelease;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline int mouse_argc (t_mouse *m)
{
    return 7;
}

static inline t_atom *mouse_argv (t_mouse *m)
{
    SET_FLOAT (m->m_atoms + 0, m->m_x);
    SET_FLOAT (m->m_atoms + 1, m->m_y);
    SET_FLOAT (m->m_atoms + 2, m->m_shift);
    SET_FLOAT (m->m_atoms + 3, m->m_ctrl);
    SET_FLOAT (m->m_atoms + 4, m->m_alt);
    SET_FLOAT (m->m_atoms + 5, m->m_dbl);
    SET_FLOAT (m->m_atoms + 6, m->m_clicked);
    
    return m->m_atoms;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        pd_bang                                     (t_pd *x);
void        pd_pointer                                  (t_pd *x, t_gpointer *gp);
void        pd_float                                    (t_pd *x, t_float f);
void        pd_symbol                                   (t_pd *x, t_symbol *s);
void        pd_list                                     (t_pd *x, int argc, t_atom *argv);
void        pd_message                                  (t_pd *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_pd        *pd_getThing                                (t_symbol *s);
t_pd        *pd_getThingByClass                         (t_symbol *s, t_class *c);
t_pd        *pd_getBoundX                               (void);

int         pd_isThing                                  (t_symbol *s);
int         pd_isThingQuiet                             (t_symbol *s);

void        pd_setBoundN                                (t_pd *x);
void        pd_setBoundX                                (t_pd *x);
void        pd_setBoundA                                (t_pd *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        stack_push                                  (t_pd *x);
void        stack_pop                                   (t_pd *x);
void        stack_proceedLoadbang                       (void);
int         stack_setLoadingAbstraction                 (t_symbol *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        message_initialize                          (void);
void        message_release                             (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        autorelease_run                             (void);
void        autorelease_stop                            (void);
void        autorelease_drain                           (void);
void        autorelease_add                             (t_pd *x);
void        autorelease_proceed                         (t_pd *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        poll_run                                    (void);
void        poll_stop                                   (void);
void        poll_add                                    (t_pd *x);
void        poll_remove                                 (t_pd *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        global_shouldQuit                           (void *dummy);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_atom      *atom_substituteIfPointer                   (t_atom *a);
char        *atom_atomsToString                         (int argc, t_atom *argv);

void        atom_copyAtomsUnchecked                     (int argc, t_atom *src, t_atom *dest);
t_error     atom_withStringUnzeroed                     (t_atom *a, char *s, int size);
t_error     atom_toString                               (t_atom *a, char *dest, int size);
t_atomtype  atom_getType                                (t_atom *a);
int         atom_typesAreEqual                          (t_atom *a, t_atom *b);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        buffer_resize                               (t_buffer *x, int n);
void        buffer_vAppend                              (t_buffer *x, char *fmt, ...);
void        buffer_appendAtom                           (t_buffer *x, t_atom *a);
void        buffer_appendBuffer                         (t_buffer *x, t_buffer *y);
void        buffer_appendFloat                          (t_buffer *x, t_float f);
void        buffer_appendSemicolon                      (t_buffer *x);
t_error     buffer_resizeAtBetween                      (t_buffer *x, int n, int start, int end);
t_error     buffer_getAtomAtIndex                       (t_buffer *x, int n, t_atom *a);
t_error     buffer_setAtomAtIndex                       (t_buffer *x, int n, t_atom *a);
t_atom      *buffer_atomAtIndex                         (t_buffer *x, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void        buffer_toString                             (t_buffer *x, char **s);
void        buffer_toStringUnzeroed                     (t_buffer *x, char **s, int *size);
void        buffer_withStringUnzeroed                   (t_buffer *x, char *s, int size);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int         buffer_isLastMessageProperlyEnded           (t_buffer *x);
int         buffer_getNumberOfMessages                  (t_buffer *x);
int         buffer_getMessageAt                         (t_buffer *x, int n, int *start, int *end);
int         buffer_getMessageAtWithTypeOfEnd            (t_buffer *x,
                                                            int n,
                                                            int *start,
                                                            int *end,
                                                            t_atomtype *type);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void        buffer_serialize                            (t_buffer *x, t_buffer *y);
void        buffer_deserialize                          (t_buffer *x, int argc, t_atom *argv);
void        buffer_eval                                 (t_buffer *x, t_pd *object, int argc, t_atom *argv);
t_error     buffer_read                                 (t_buffer *x, t_symbol *name, t_glist *glist);
t_error     buffer_write                                (t_buffer *x, char *name, char *directory);
t_error     buffer_fileEval                             (t_symbol *name, t_symbol *directory);
void        buffer_fileOpen                             (void *dummy, t_symbol *name, t_symbol *directory);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        setup_initialize                            (void);
void        setup_release                               (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol    *dollar_toHash                              (t_symbol *s);
t_symbol    *dollar_fromHash                            (t_symbol *s);

int         dollar_isDollarNumber                       (const char *s);
int         dollar_isPointingToDollarAndNumber          (const char *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_symbol    *dollar_expandDollarSymbolByEnvironment     (t_symbol *s, t_glist *glist);
t_symbol    *dollar_expandDollarSymbol                  (t_symbol *s, int argc, t_atom *argv, t_glist *glist);

void        dollar_expandDollarNumberByEnvironment      (t_atom *dollar, t_atom *a, t_glist *glist);
void        dollar_expandDollarNumber                   (t_atom *dollar,
                                                            t_atom *a,
                                                            int argc,
                                                            t_atom *argv, 
                                                            t_glist *glist);

void        dollar_copyExpandAtomsByEnvironment         (t_atom *src,
                                                            int m,
                                                            t_atom *dest,
                                                            int n,
                                                            t_glist *glist);
                                                            
void        dollar_copyExpandAtoms                      (t_atom *src,
                                                            int m,
                                                            t_atom *dest,
                                                            int n,
                                                            int argc,
                                                            t_atom *argv, 
                                                            t_glist *glist);

t_symbol    *dollar_expandGetIfSymbolByEnvironment      (t_atom *a, t_glist *glist);
t_symbol    *dollar_expandGetIfSymbol                   (t_atom *a, int argc, t_atom *argv, t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        sys_vGui                                    (char *format, ...);
void        sys_gui                                     (char *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     guistub_new                                 (t_pd *owner, void *key, const char *cmd);
void        guistub_destroyWithKey                      (void *key);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        instance_destroyScalarsByTemplate           (t_template *tmpl);
void        instance_addToRoots                         (t_glist *glist);
void        instance_removeFromRoots                    (t_glist *glist);
void        instance_freeAllRoots                       (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_class.h"
#include "m_object.h"
#include "m_error.h"
#include "m_utils.h"
#include "m_rectangle.h"
#include "m_symbols.h"
#include "h_helpers.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_core_h_

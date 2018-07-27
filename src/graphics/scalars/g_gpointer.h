
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __g_gpointer_h_
#define __g_gpointer_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void gpointer_init (t_gpointer *gp)
{
    gp->gp_un.gp_scalar     = NULL;
    gp->gp_refer            = NULL;
    gp->gp_uniqueIdentifier = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        gpointer_setAsNull                  (t_gpointer *gp, t_glist  *owner);
void        gpointer_setAsScalar                (t_gpointer *gp, t_scalar *scalar);
void        gpointer_setAsWord                  (t_gpointer *gp, t_array  *owner, t_word *w);

t_scalar    *gpointer_getScalar                 (t_gpointer *gp);
t_word      *gpointer_getWord                   (t_gpointer *gp);

void        gpointer_setByCopy                  (t_gpointer *gp, t_gpointer *toCopy);
void        gpointer_unset                      (t_gpointer *gp);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_gpointer  *gpointer_getEmpty                  (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Functions below callable on both types. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         gpointer_isScalar                   (t_gpointer *gp);
int         gpointer_isWord                     (t_gpointer *gp);
int         gpointer_isValid                    (t_gpointer *gp);
int         gpointer_isValidOrNull              (t_gpointer *gp);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_glist     *gpointer_getView                   (t_gpointer *gp);
t_word      *gpointer_getElement                (t_gpointer *gp);
t_symbol    *gpointer_getTemplateIdentifier     (t_gpointer *gp);
t_template  *gpointer_getTemplate               (t_gpointer *gp);
t_garray    *gpointer_getGraphicArray           (t_gpointer *gp);

int         gpointer_isInstanceOf               (t_gpointer *gp, t_symbol *templateIdentifier);
int         gpointer_isValidInstanceOf          (t_gpointer *gp, t_symbol *templateIdentifier);

void        gpointer_erase                      (t_gpointer *gp);
void        gpointer_draw                       (t_gpointer *gp);
void        gpointer_redraw                     (t_gpointer *gp);

void        gpointer_notify                     (t_gpointer *gp, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol    *gpointer_getRepresentation         (t_gpointer *gp);

t_error     gpointer_getFieldAsString           (t_gpointer *gp, t_symbol *field, char *dest, int size);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         gpointer_getProperties              (t_gpointer *gp, t_heapstring *h);
void        gpointer_setProperties              (t_gpointer *gp, int argc, t_atom *argv, int notify);

void        gpointer_setFields                  (t_gpointer *gp, int argc, t_atom *argv);
t_error     gpointer_getFields                  (t_gpointer *gp, t_buffer *b);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int gpointer_hasField (t_gpointer *gp, t_symbol *fieldName)
{
    return template_hasField (gpointer_getTemplate (gp), fieldName);
}

static inline int gpointer_fieldIsFloat (t_gpointer *gp, t_symbol *fieldName)
{
    return template_fieldIsFloat (gpointer_getTemplate (gp), fieldName);
}

static inline int gpointer_fieldIsSymbol (t_gpointer *gp, t_symbol *fieldName)
{
    return template_fieldIsSymbol (gpointer_getTemplate (gp), fieldName);
}

static inline int gpointer_fieldIsArray (t_gpointer *gp, t_symbol *fieldName)
{
    return template_fieldIsArray (gpointer_getTemplate (gp), fieldName);
}

static inline int gpointer_fieldIsArrayAndValid (t_gpointer *gp, t_symbol *fieldName)
{
    return template_fieldIsArrayAndValid (gpointer_getTemplate (gp), fieldName);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_float gpointer_getFloat (t_gpointer *gp, t_symbol *fieldName)
{
    return word_getFloat (gpointer_getElement (gp), gpointer_getTemplate (gp), fieldName);
}

static inline t_symbol *gpointer_getSymbol (t_gpointer *gp, t_symbol *fieldName)
{
    return word_getSymbol (gpointer_getElement (gp), gpointer_getTemplate (gp), fieldName);
}

static inline t_array *gpointer_getArray (t_gpointer *gp, t_symbol *fieldName)
{
    return word_getArray (gpointer_getElement (gp), gpointer_getTemplate (gp), fieldName);
}

static inline void gpointer_setFloat (t_gpointer *gp, t_symbol *fieldName, t_float f)
{
    word_setFloat (gpointer_getElement (gp), gpointer_getTemplate (gp), fieldName, f);
}

static inline void gpointer_setSymbol (t_gpointer *gp, t_symbol *fieldName, t_symbol *s)
{
    word_setSymbol (gpointer_getElement (gp), gpointer_getTemplate (gp), fieldName, s);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_float gpointer_getFloatByDescriptor (t_gpointer *gp, t_fielddescriptor *fd)
{
    return word_getFloatByDescriptor (gpointer_getElement (gp), gpointer_getTemplate (gp), fd);
}

static inline void gpointer_setFloatByDescriptor (t_gpointer *gp, t_fielddescriptor *fd, t_float f)
{
    word_setFloatByDescriptor (gpointer_getElement (gp), gpointer_getTemplate (gp), fd, f);
}
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_gpointer_h_

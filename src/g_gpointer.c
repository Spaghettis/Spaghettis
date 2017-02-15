
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Weak pointer machinery. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define GMASTER_NONE    0
#define GMASTER_GLIST   1
#define GMASTER_ARRAY   2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _gmaster {
    union {
        t_glist     *gm_glist;
        t_array     *gm_array;
    } gm_un;
    int             gm_type;
    int             gm_count;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_gpointer gpointer_empty;     /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_gmaster *gmaster_createWithGlist (t_glist *glist)
{
    t_gmaster *master = PD_MEMORY_GET (sizeof (t_gmaster));
    
    PD_ASSERT (glist);
    
    master->gm_type         = GMASTER_GLIST;
    master->gm_un.gm_glist  = glist;
    master->gm_count        = 0;
    
    return master;
}

t_gmaster *gmaster_createWithArray (t_array *array)
{
    t_gmaster *master = PD_MEMORY_GET (sizeof (t_gmaster));
    
    PD_ASSERT (array);
    
    master->gm_type         = GMASTER_ARRAY;
    master->gm_un.gm_array  = array;
    master->gm_count        = 0;
    
    return master;
}

void gmaster_reset (t_gmaster *master)
{
    PD_ASSERT (master->gm_count >= 0);
    
    master->gm_type = GMASTER_NONE; if (master->gm_count == 0) { PD_MEMORY_FREE (master); }
}

static void gmaster_increment (t_gmaster *master)
{
    master->gm_count++;
}

static void gmaster_decrement (t_gmaster *master)
{
    int count = --master->gm_count;
    
    PD_ASSERT (count >= 0);
    
    if (count == 0 && master->gm_type == GMASTER_NONE) { PD_MEMORY_FREE (master); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int gpointer_isValidProceed (t_gpointer *gp, int nullPointerIsValid)
{
    if (gpointer_isSet (gp)) {
    //
    if (!nullPointerIsValid && gpointer_isNull (gp)) { return 0; }
    else {
    //
    t_gmaster *master = gp->gp_master;
        
    if (master->gm_type == GMASTER_ARRAY) {
        if (master->gm_un.gm_array->a_uniqueIdentifier == gp->gp_uniqueIdentifier)  { return 1; }
        
    } else if (master->gm_type == GMASTER_GLIST) {
        if (master->gm_un.gm_glist->gl_uniqueIdentifier == gp->gp_uniqueIdentifier) { return 1; }
        
    } else {
        PD_ASSERT (master->gm_type == GMASTER_NONE);
    }
    //
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_gpointer *gpointer_getEmpty (void)
{
    PD_ASSERT (!gpointer_isSet (&gpointer_empty));
    
    return &gpointer_empty;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void gpointer_init (t_gpointer *gp)
{
    gp->gp_un.gp_scalar     = NULL;
    gp->gp_master           = NULL;
    gp->gp_uniqueIdentifier = 0;
}

/* Point to a scalar. */

void gpointer_setAsScalar (t_gpointer *gp, t_glist *glist, t_scalar *scalar)
{
    gpointer_unset (gp);
    
    gp->gp_un.gp_scalar     = scalar;
    gp->gp_master           = glist->gl_master;
    gp->gp_uniqueIdentifier = glist->gl_uniqueIdentifier;

    gmaster_increment (gp->gp_master);
}

/* Point to an element (i.e. a chunk of t_word) from an array. */

void gpointer_setAsWord (t_gpointer *gp, t_array *array, t_word *w)
{
    gpointer_unset (gp);
    
    gp->gp_un.gp_w          = w;
    gp->gp_master           = array->a_master;
    gp->gp_uniqueIdentifier = array->a_uniqueIdentifier;

    gmaster_increment (gp->gp_master);
}

void gpointer_setByCopy (t_gpointer *gp, t_gpointer *toSet)
{
    gpointer_unset (toSet);
    
    *toSet = *gp;
    
    if (toSet->gp_master) { gmaster_increment (toSet->gp_master); }
}

void gpointer_unset (t_gpointer *gp)
{
    if (gpointer_isSet (gp)) { gmaster_decrement (gp->gp_master); }
    
    gpointer_init (gp);
}

int gpointer_isSet (t_gpointer *gp)
{
    return (gp->gp_master != NULL);
}

int gpointer_isNull (t_gpointer *gp)
{
    return (gp->gp_un.gp_scalar == NULL);
}

int gpointer_isValid (t_gpointer *gp)
{
    return gpointer_isValidProceed (gp, 0); 
}

int gpointer_isValidNullAllowed (t_gpointer *gp)
{
    return gpointer_isValidProceed (gp, 1); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int gpointer_isScalar (t_gpointer *gp)
{
    return (gp->gp_master->gm_type == GMASTER_GLIST);
}

int gpointer_isWord (t_gpointer *gp)
{
    return (gp->gp_master->gm_type == GMASTER_ARRAY);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_scalar *gpointer_getScalar (t_gpointer *gp)
{
    PD_ASSERT (gpointer_isScalar (gp)); return (gp->gp_un.gp_scalar);
}

t_word *gpointer_getWord (t_gpointer *gp)
{
    PD_ASSERT (gpointer_isWord (gp)); return (gp->gp_un.gp_w);
}

t_glist *gpointer_getParentGlist (t_gpointer *gp)
{
    PD_ASSERT (gp->gp_master->gm_type == GMASTER_GLIST);
    
    return (gp->gp_master->gm_un.gm_glist);
}

t_array *gpointer_getParentArray (t_gpointer *gp)
{
    PD_ASSERT (gp->gp_master->gm_type == GMASTER_ARRAY);
    
    return (gp->gp_master->gm_un.gm_array);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_word *gpointer_getData (t_gpointer *gp)
{
    if (gpointer_isWord (gp))       { return gpointer_getWord (gp); } 
    else if (!gpointer_isNull (gp)) { return scalar_getData (gpointer_getScalar (gp)); }
    
    return NULL;
}

t_glist *gpointer_getView (t_gpointer *gp)
{
    if (gpointer_isScalar (gp)) { return gpointer_getParentGlist (gp); }
    else {
        return gpointer_getParentGlist (array_getTopParent (gpointer_getParentArray (gp)));
    }
}

t_symbol *gpointer_getTemplateIdentifier (t_gpointer *gp)
{
    t_gmaster *master = gp->gp_master;
    
    PD_ASSERT (gpointer_isValidNullAllowed (gp));
    
    if (master->gm_type == GMASTER_GLIST) {
        if (!gpointer_isNull (gp)) { return scalar_getTemplateIdentifier (gpointer_getScalar (gp)); }
        
    } else {
        return array_getTemplateIdentifier (master->gm_un.gm_array);
    }
    
    return &s_;
}

t_template *gpointer_getTemplate (t_gpointer *gp)
{
    t_template *template = template_findByIdentifier (gpointer_getTemplateIdentifier (gp));
    
    PD_ASSERT (template);
    
    return (template);
}

static t_scalar *gpointer_getBase (t_gpointer *gp)
{
    t_scalar *scalar = NULL;
    
    if (gpointer_isScalar (gp)) { scalar = gpointer_getScalar (gp); }
    else {
        scalar = gpointer_getScalar (array_getTopParent (gpointer_getParentArray (gp)));
    }
    
    return scalar;
}

int gpointer_isInstanceOf (t_gpointer *gp, t_symbol *templateIdentifier)
{
    if (templateIdentifier == template_getWildcard())               { return 1; }
    if (templateIdentifier == gpointer_getTemplateIdentifier (gp))  { return 1; }
    
    return 0;
}

int gpointer_isValidInstanceOf (t_gpointer *gp, t_symbol *templateIdentifier)
{
    if (!gpointer_isValid (gp))                             { return 0; }
    if (!gpointer_isInstanceOf (gp, templateIdentifier))    { return 0; }
    if (!gpointer_getTemplate (gp))                         { return 0; }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void gpointer_redraw (t_gpointer *gp)
{
    scalar_redraw (gpointer_getBase (gp), gpointer_getView (gp));
}

void gpointer_setVisibility (t_gpointer *gp, int isVisible)
{
    gobj_visibilityChanged (cast_gobj (gpointer_getBase (gp)), gpointer_getView (gp), isVisible); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error gpointer_fieldToString (t_gpointer *gp, t_symbol *fieldName, char *dest, int size)
{
    t_error err = PD_ERROR_NONE;
    
    PD_ASSERT (size > 0);
    
    if (gpointer_fieldIsFloat (gp, fieldName)) {
        t_atom a;
        SET_FLOAT (&a, gpointer_getFloat (gp, fieldName));
        err = string_addAtom (dest, size, &a);
        
    } else if (gpointer_fieldIsSymbol (gp, fieldName)) {
        t_atom a;
        SET_SYMBOL (&a, gpointer_getSymbol (gp, fieldName));
        err = string_addAtom (dest, size, &a);
            
    } else if (gpointer_fieldIsText (gp, fieldName)) {
        char *t = NULL;
        buffer_toString (gpointer_getText (gp, fieldName), &t);
        err = string_add (dest, size, t);
        PD_MEMORY_FREE (t);
        
    } else {
        err = PD_ERROR; PD_BUG;     /* Not implemented yet. */
    }
    
    return err;
}
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int gpointer_hasField (t_gpointer *gp, t_symbol *fieldName)
{
    return template_hasField (gpointer_getTemplate (gp), fieldName);
}

int gpointer_fieldIsFloat (t_gpointer *gp, t_symbol *fieldName)
{
    return template_fieldIsFloat (gpointer_getTemplate (gp), fieldName);
}

int gpointer_fieldIsSymbol (t_gpointer *gp, t_symbol *fieldName)
{
    return template_fieldIsSymbol (gpointer_getTemplate (gp), fieldName);
}

int gpointer_fieldIsText (t_gpointer *gp, t_symbol *fieldName)
{
    return template_fieldIsText (gpointer_getTemplate (gp), fieldName);
}

int gpointer_fieldIsArray (t_gpointer *gp, t_symbol *fieldName)
{
    return template_fieldIsArray (gpointer_getTemplate (gp), fieldName);
}

int gpointer_fieldIsArrayAndValid (t_gpointer *gp, t_symbol *fieldName)
{
    return template_fieldIsArrayAndValid (gpointer_getTemplate (gp), fieldName);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float gpointer_getFloat (t_gpointer *gp, t_symbol *fieldName)
{
    return word_getFloat (gpointer_getData (gp), gpointer_getTemplate (gp), fieldName);
}

t_symbol *gpointer_getSymbol (t_gpointer *gp, t_symbol *fieldName)
{
    return word_getSymbol (gpointer_getData (gp), gpointer_getTemplate (gp), fieldName);
}

t_buffer *gpointer_getText (t_gpointer *gp, t_symbol *fieldName)
{
    return word_getText (gpointer_getData (gp), gpointer_getTemplate (gp), fieldName);
}

t_array *gpointer_getArray (t_gpointer *gp, t_symbol *fieldName)
{
    return word_getArray (gpointer_getData (gp), gpointer_getTemplate (gp), fieldName);
}

void gpointer_setFloat (t_gpointer *gp, t_symbol *fieldName, t_float f)
{
    word_setFloat (gpointer_getData (gp), gpointer_getTemplate (gp), fieldName, f);
}

void gpointer_setSymbol (t_gpointer *gp, t_symbol *fieldName, t_symbol *s)
{
    word_setSymbol (gpointer_getData (gp), gpointer_getTemplate (gp), fieldName, s);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float gpointer_getFloatByDescriptor (t_gpointer *gp, t_fielddescriptor *fd)
{
    return word_getFloatByDescriptor (gpointer_getData (gp), gpointer_getTemplate (gp), fd);
}

void gpointer_setFloatByDescriptor (t_gpointer *gp, t_fielddescriptor *fd, t_float position)
{
    word_setFloatByDescriptor (gpointer_getData (gp), gpointer_getTemplate (gp), fd, position);
}
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

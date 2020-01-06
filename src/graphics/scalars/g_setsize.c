
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *setsize_class;                      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _setsize {
    t_object    x_obj;                              /* Must be the first. */
    t_gpointer  x_gpointer;
    t_symbol    *x_templateIdentifier;
    t_symbol    *x_fieldName;
    } t_setsize;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void setsize_float (t_setsize *x, t_float f)
{
    if (gpointer_isValidInstanceOf (&x->x_gpointer, x->x_templateIdentifier)) {
        if (gpointer_hasField (&x->x_gpointer, x->x_fieldName)) {
            if (gpointer_fieldIsArrayAndValid (&x->x_gpointer, x->x_fieldName)) {
                t_array *array = gpointer_getArray (&x->x_gpointer, x->x_fieldName);
                array_resizeAndRedraw (array, gpointer_getOwner (&x->x_gpointer), PD_MAX (1, (int)f));
        
    } else { error_invalid (sym_setsize, x->x_fieldName); }
    } else { error_missingField (sym_setsize, x->x_fieldName); }
    } else { error_invalid (sym_setsize, &s_pointer); }
}

static void setsize_set (t_setsize *x, t_symbol *templateName, t_symbol *fieldName)
{
    x->x_templateIdentifier = template_makeIdentifierWithWildcard (templateName);
    x->x_fieldName          = fieldName;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_buffer *setsize_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_setsize *x = (t_setsize *)z;
    t_buffer *b  = buffer_new();
    
    buffer_appendSymbol (b, sym_set);
    buffer_appendSymbol (b, symbol_stripTemplateIdentifier (x->x_templateIdentifier));
    buffer_appendSymbol (b, x->x_fieldName);
    buffer_appendComma (b);
    buffer_appendSymbol (b, sym__restore);
    
    return b;
    //
    }
    
    return NULL;
}

static void setsize_restore (t_setsize *x)
{
    t_setsize *old = (t_setsize *)instance_pendingFetch (cast_gobj (x));
    
    if (old) { gpointer_setByCopy (&x->x_gpointer, &old->x_gpointer); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *setsize_new (t_symbol *templateName, t_symbol *fieldName)
{
    t_setsize *x = (t_setsize *)pd_new (setsize_class);
    
    gpointer_init (&x->x_gpointer);
        
    x->x_templateIdentifier = template_makeIdentifierWithWildcard (templateName);
    x->x_fieldName          = fieldName;
    
    inlet_newPointer (cast_object (x), &x->x_gpointer);
    
    return x;
}

static void setsize_free (t_setsize *x)
{
    gpointer_unset (&x->x_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void setsize_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_setsize,
            (t_newmethod)setsize_new,
            (t_method)setsize_free,
            sizeof (t_setsize),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addFloat (c, (t_method)setsize_float);
    
    class_addMethod (c, (t_method)setsize_set,      sym_set,        A_SYMBOL, A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)setsize_restore,  sym__restore,   A_NULL);

    class_setDataFunction (c, setsize_functionData);
    class_requirePending (c);
    
    setsize_class = c;
}

void setsize_destroy (void)
{
    class_free (setsize_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

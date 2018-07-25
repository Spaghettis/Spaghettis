
/* Copyright (c) 1997-2018 Miller Puckette and others. */

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

static t_class *get_class;                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _getvariable {
    t_symbol        *gv_fieldName;
    t_outlet        *gv_outlet;
    } t_getvariable;

typedef struct _get {
    t_object        x_obj;                  /* Must be the first. */
    int             x_fieldsSize;
    t_getvariable   *x_fields;
    t_symbol        *x_templateIdentifier;
    } t_get;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void get_pointer (t_get *x, t_gpointer *gp)
{
    if (gpointer_isValidInstanceOf (gp, x->x_templateIdentifier)) {
    //
    int i;
    for (i = x->x_fieldsSize - 1; i >= 0; i--) {
    
        t_outlet *o = x->x_fields[i].gv_outlet;
        t_symbol *s = x->x_fields[i].gv_fieldName;
        
        if (gpointer_hasField (gp, s)) {
            if (gpointer_fieldIsFloat (gp, s))       { outlet_float (o, gpointer_getFloat (gp, s));   }
            else if (gpointer_fieldIsSymbol (gp, s)) { outlet_symbol (o, gpointer_getSymbol (gp, s)); }
        } else {
            error_missingField (sym_get, s);
        }
    }
    //
    } else { error_invalid (sym_get, &s_pointer); }
}

#if PD_WITH_LEGACY

static void get_set (t_get *x, t_symbol *templateName, t_symbol *fieldName)
{
    if (x->x_fieldsSize != 1) { error_canNotSetMultipleFields (sym_get); }
    else {
        x->x_templateIdentifier     = template_makeIdentifierWithWildcard (templateName); 
        x->x_fields[0].gv_fieldName = fieldName;
    }
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *get_new (t_symbol *s, int argc, t_atom *argv)
{
    t_get *x = (t_get *)pd_new (get_class);
    int i;

    x->x_fieldsSize         = PD_MAX (1, argc - 1);
    x->x_fields             = (t_getvariable *)PD_MEMORY_GET (x->x_fieldsSize * sizeof (t_getvariable));
    x->x_templateIdentifier = template_makeIdentifierWithWildcard (atom_getSymbolAtIndex (0, argc, argv));
    
    for (i = 0; i < x->x_fieldsSize; i++) {
        x->x_fields[i].gv_fieldName = atom_getSymbolAtIndex (i + 1, argc, argv);
        x->x_fields[i].gv_outlet    = outlet_newAnything (cast_object (x));
    }
    
    return x;
}

static void get_free (t_get *x)
{
    PD_MEMORY_FREE (x->x_fields);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void get_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_get,
            (t_newmethod)get_new,
            (t_method)get_free,
            sizeof (t_get),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addPointer (c, (t_method)get_pointer); 
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)get_set, sym_set, A_SYMBOL, A_SYMBOL, A_NULL); 
    
    #endif
    
    get_class = c;
}

void get_destroy (void)
{
    class_free (get_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

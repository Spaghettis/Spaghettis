
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __g_template_h_
#define __g_template_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _dataslot {
    int         ds_type;
    t_symbol    *ds_fieldName;
    t_symbol    *ds_templateIdentifier;
    } t_dataslot;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct _template {
    t_pd        tp_pd;                              /* MUST be the first. */
    t_error     tp_error;
    int         tp_pending;
    int         tp_size;    
    t_dataslot  *tp_slots;   
    t_symbol    *tp_templateIdentifier;
    t_struct    *tp_instance;
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_template  *template_findByIdentifier              (t_symbol *templateIdentifier);
t_template  *template_new                           (t_symbol *templateIdentifier, int argc, t_atom *argv);

t_symbol    *template_getUnexpandedName             (t_template *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int         template_isValid                        (t_template *x);
void        template_free                           (t_template *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         template_hasInstance                    (t_template *x);
int         template_hasPending                     (t_template *x);
void        template_registerInstance               (t_template *x, t_struct *o);
void        template_unregisterInstance             (t_template *x, t_struct *o);
void        template_forgetPendingInstance          (t_template *x, t_struct *o);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         template_containsArray                  (t_template *x);
int         template_containsTemplate               (t_template *x, t_symbol *templateIdentifier);
int         template_hasField                       (t_template *x, t_symbol *field);
int         template_getIndexOfField                (t_template *x, t_symbol *field);
int         template_getRaw                         (t_template *x,
                                                            t_symbol *field,
                                                            int *index,
                                                            int *type,
                                                            t_symbol **templateIdentifier);

t_symbol      *template_getFieldAtIndex             (t_template *x, int n);

t_glist       *template_getInstanceOwnerIfPainters  (t_template *x);
t_constructor *template_getInstanceConstructorIfAny (t_template *x, t_symbol *field);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         template_fieldIsFloat                   (t_template *x, t_symbol *field);
int         template_fieldIsSymbol                  (t_template *x, t_symbol *field);
int         template_fieldIsArray                   (t_template *x, t_symbol *field);
int         template_fieldIsArrayAndValid           (t_template *x, t_symbol *field);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_glist     *struct_getOwner                        (t_struct *x);
t_symbol    *struct_getUnexpandedName               (t_struct *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void        struct_notify                           (t_struct *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol    *constructor_getField                   (t_constructor *x);
t_symbol    *constructor_evaluateAsSymbol           (t_constructor *x);

t_float     constructor_evaluateAsFloat             (t_constructor *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int template_getSize (t_template *x)
{
    return x->tp_size;
}

static inline t_dataslot *template_getSlots (t_template *x)
{
    return x->tp_slots;
}

static inline t_symbol *template_getTemplateIdentifier (t_template *x)
{
    return x->tp_templateIdentifier;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int template_isPrivate (t_symbol *templateIdentifier)
{
    if (templateIdentifier == sym__TEMPLATE_float__dash__array) { return 1; }
    else if (templateIdentifier == sym__TEMPLATE_float)         { return 1; }
    else {
        return 0;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_symbol *template_getWildcard (void)
{
    return &s_;
}

static inline t_symbol *template_makeIdentifierWithWildcard (t_symbol *s)
{
    if (s == &s_ || s == sym___dash__) { return template_getWildcard(); }
    else { 
        return symbol_makeTemplateIdentifier (s);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_template_h_

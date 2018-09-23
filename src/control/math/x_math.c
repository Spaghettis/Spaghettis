
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *sin_class;          /* Shared. */
static t_class *cos_class;          /* Shared. */
static t_class *tan_class;          /* Shared. */
static t_class *exp_class;          /* Shared. */
static t_class *abs_class;          /* Shared. */
static t_class *sqrt_class;         /* Shared. */
static t_class *wrap_class;         /* Shared. */
static t_class *atan_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _math {
    t_object    x_obj;              /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_math;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define MATH_MAXIMUM_LOGARITHM      87.3365

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *sin_new (void)
{
    t_math *x = (t_math *)pd_new (sin_class);
    
    x->x_outlet = outlet_newFloat (cast_object (x));
    
    return x;
}

static void sin_bang (t_math *x)
{
    outlet_float (x->x_outlet, sin (x->x_f));
}

static void sin_float (t_math *x, t_float f)
{
    x->x_f = f; sin_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *cos_new (void)
{
    t_math *x = (t_math *)pd_new (cos_class);
    
    x->x_outlet = outlet_newFloat (cast_object (x));
    
    return x;
}

static void cos_bang (t_math *x)
{
    outlet_float (x->x_outlet, cos (x->x_f));
}

static void cos_float (t_math *x, t_float f)
{
    x->x_f = f; cos_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *tan_new (void)
{
    t_math *x = (t_math *)pd_new (tan_class);
    
    x->x_outlet = outlet_newFloat (cast_object (x));
    
    return x;
}

static void tan_bang (t_math *x)
{
    t_float c = cos (x->x_f);
    t_float t = (t_float)(c == 0.0 ? 0.0 : sin (x->x_f) / c);
    
    outlet_float (x->x_outlet, t);
}

static void tan_float (t_math *x, t_float f)
{
    x->x_f = f; tan_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *exp_new (void)
{
    t_math *x = (t_math *)pd_new (exp_class);
    
    x->x_outlet = outlet_newFloat (cast_object (x));
    
    return x;
}

static void exp_bang (t_math *x)
{
    outlet_float (x->x_outlet, exp ((t_float)PD_MIN (x->x_f, MATH_MAXIMUM_LOGARITHM)));
}

static void exp_float (t_math *x, t_float f)
{
    x->x_f = f; exp_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *abs_new (void)
{
    t_math *x = (t_math *)pd_new (abs_class);
    
    x->x_outlet = outlet_newFloat (cast_object (x));
    
    return x;
}

static void abs_bang (t_math *x)
{
    outlet_float (x->x_outlet, fabs (x->x_f));
}

static void abs_float (t_math *x, t_float f)
{
    x->x_f = f; abs_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *sqrt_new (void)
{
    t_math *x = (t_math *)pd_new (sqrt_class);
    
    x->x_outlet = outlet_newFloat (cast_object (x));
    
    return x;
}

static void sqrt_bang (t_math *x)
{
    outlet_float (x->x_outlet, (t_float)(x->x_f > 0.0 ? sqrt (x->x_f) : 0.0));
}

static void sqrt_float (t_math *x, t_float f)
{
    x->x_f = f; sqrt_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *wrap_new (void)
{
    t_math *x = (t_math *)pd_new (wrap_class);
    
    x->x_outlet = outlet_newFloat (cast_object (x));
    
    return x;
}

static void wrap_bang (t_math *x)
{
    outlet_float (x->x_outlet, (t_float)(x->x_f - floor (x->x_f)));
}

static void wrap_float (t_math *x, t_float f)
{
    x->x_f = f; wrap_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *atan_new (void)
{
    t_math *x = (t_math *)pd_new (atan_class);
    
    x->x_outlet = outlet_newFloat (cast_object (x));
    
    return x;
}

static void atan_bang (t_math *x)
{
    outlet_float (x->x_outlet, atan (x->x_f));
}

static void atan_float (t_math *x, t_float f)
{
    x->x_f = f; atan_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void math_setup (void)
{
    sin_class = class_new (sym_sin,
                    (t_newmethod)sin_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
    
    cos_class = class_new (sym_cos,
                    (t_newmethod)cos_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
                    
    tan_class = class_new (sym_tan,
                    (t_newmethod)tan_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
    
    exp_class = class_new (sym_exp,
                    (t_newmethod)exp_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
                    
    abs_class = class_new (sym_abs,
                    (t_newmethod)abs_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
                    
    sqrt_class = class_new (sym_sqrt,
                    (t_newmethod)sqrt_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
                    
    wrap_class = class_new (sym_wrap,
                    (t_newmethod)wrap_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
                    
    atan_class = class_new (sym_atan,
                    (t_newmethod)atan_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
    
    class_addBang (sin_class,       (t_method)sin_bang);
    class_addBang (cos_class,       (t_method)cos_bang);
    class_addBang (tan_class,       (t_method)tan_bang);
    class_addBang (exp_class,       (t_method)exp_bang);
    class_addBang (abs_class,       (t_method)abs_bang);
    class_addBang (wrap_class,      (t_method)wrap_bang);
    class_addBang (sqrt_class,      (t_method)sqrt_bang);
    class_addBang (atan_class,      (t_method)atan_bang);
    
    class_addFloat (sin_class,      (t_method)sin_float);  
    class_addFloat (cos_class,      (t_method)cos_float);
    class_addFloat (tan_class,      (t_method)tan_float);
    class_addFloat (exp_class,      (t_method)exp_float);
    class_addFloat (abs_class,      (t_method)abs_float); 
    class_addFloat (wrap_class,     (t_method)wrap_float);    
    class_addFloat (sqrt_class,     (t_method)sqrt_float);
    class_addFloat (atan_class,     (t_method)atan_float);
            
    class_setHelpName (sin_class,   sym_math);
    class_setHelpName (cos_class,   sym_math);
    class_setHelpName (tan_class,   sym_math);
    class_setHelpName (exp_class,   sym_math);
    class_setHelpName (abs_class,   sym_math);
    class_setHelpName (sqrt_class,  sym_math);
    class_setHelpName (wrap_class,  sym_math);
    class_setHelpName (atan_class,  sym_math);
}

void math_destroy (void)
{
    class_free (sin_class);
    class_free (cos_class);
    class_free (tan_class);
    class_free (exp_class);
    class_free (abs_class);
    class_free (sqrt_class);
    class_free (wrap_class);
    class_free (atan_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

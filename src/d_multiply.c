
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *multiply_tilde_class;               /* Shared. */
static t_class *multiplyScalar_tilde_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _multiply_tilde {
    t_object    x_obj;                              /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_multiply_tilde;

typedef struct _multiplyscalar_tilde {
    t_object    x_obj;                              /* Must be the first. */
    t_float     x_f;
    t_float     x_scalar;
    t_outlet    *x_outlet;
    } t_multiplyscalar_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_int *times_perform(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *in2 = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    while (n--) *out++ = *in1++ * *in2++; 
    return (w+5);
}

t_int *times_perf8(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *in2 = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    for (; n; n -= 8, in1 += 8, in2 += 8, out += 8)
    {
        t_sample f0 = in1[0], f1 = in1[1], f2 = in1[2], f3 = in1[3];
        t_sample f4 = in1[4], f5 = in1[5], f6 = in1[6], f7 = in1[7];

        t_sample g0 = in2[0], g1 = in2[1], g2 = in2[2], g3 = in2[3];
        t_sample g4 = in2[4], g5 = in2[5], g6 = in2[6], g7 = in2[7];

        out[0] = f0 * g0; out[1] = f1 * g1; out[2] = f2 * g2; out[3] = f3 * g3;
        out[4] = f4 * g4; out[5] = f5 * g5; out[6] = f6 * g6; out[7] = f7 * g7;
    }
    return (w+5);
}

t_int *scalartimes_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_float f = *(t_float *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    while (n--) *out++ = *in++ * f; 
    return (w+5);
}

t_int *scalartimes_perf8(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_float g = *(t_float *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    for (; n; n -= 8, in += 8, out += 8)
    {
        t_sample f0 = in[0], f1 = in[1], f2 = in[2], f3 = in[3];
        t_sample f4 = in[4], f5 = in[5], f6 = in[6], f7 = in[7];

        out[0] = f0 * g; out[1] = f1 * g; out[2] = f2 * g; out[3] = f3 * g;
        out[4] = f4 * g; out[5] = f5 * g; out[6] = f6 * g; out[7] = f7 * g;
    }
    return (w+5);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void multiply_tilde_dsp (t_multiply_tilde *x, t_signal **sp)
{
    if (sp[0]->s_vectorSize&7)
        dsp_add(times_perform, 4,
            sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[0]->s_vectorSize);
    else        
        dsp_add(times_perf8, 4,
            sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[0]->s_vectorSize);
}

static void multiplyScalar_tilde_dsp (t_multiplyscalar_tilde *x, t_signal **sp)
{
    if (sp[0]->s_vectorSize&7)
        dsp_add(scalartimes_perform, 4, sp[0]->s_vector, &x->x_scalar,
            sp[1]->s_vector, sp[0]->s_vectorSize);
    else        
        dsp_add(scalartimes_perf8, 4, sp[0]->s_vector, &x->x_scalar,
            sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *multiply_tilde_newWithScalar (t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) { warning_unusedArguments (s, argc + 1, argv - 1); }
    
    t_multiplyscalar_tilde *x = (t_multiplyscalar_tilde *)pd_new (multiplyScalar_tilde_class);
    
    x->x_scalar = atom_getFloatAtIndex (0, argc, argv);
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
        
    inlet_newFloat (cast_object (x), &x->x_scalar);

    return x;
}

static void *multiply_tilde_newWithSignal (t_symbol *s, int argc, t_atom *argv)
{
    t_multiply_tilde *x = (t_multiply_tilde *)pd_new (multiply_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_new (cast_object (x), cast_pd (x), &s_signal, &s_signal);
        
    return x;
}

static void *multiply_tilde_new (t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
        return multiply_tilde_newWithScalar (s, argc, argv);
    } else {
        return multiply_tilde_newWithSignal (s, argc, argv);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void multiply_tilde_setup (void)
{
    multiply_tilde_class = class_new (sym___asterisk____tilde__,
                                    (t_newmethod)multiply_tilde_new,
                                    NULL,
                                    sizeof (t_multiply_tilde),
                                    CLASS_DEFAULT,
                                    A_GIMME,
                                    A_NULL);
    
    multiplyScalar_tilde_class = class_new (sym___asterisk____tilde__,
                                    NULL,
                                    NULL,
                                    sizeof (t_multiplyscalar_tilde),
                                    CLASS_DEFAULT,
                                    A_NULL);
        
    CLASS_SIGNAL (multiply_tilde_class, t_multiply_tilde, x_f);
    CLASS_SIGNAL (multiplyScalar_tilde_class, t_multiplyscalar_tilde, x_f);
        
    class_addDSP (multiply_tilde_class, multiply_tilde_dsp);
    class_addDSP (multiplyScalar_tilde_class, multiplyScalar_tilde_dsp);
        
    class_setHelpName (multiply_tilde_class, sym_max__tilde__);
    class_setHelpName (multiplyScalar_tilde_class, sym_max__tilde__);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------


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

/* ----------------------------- times ----------------------------- */

static t_class *times_class, *scalartimes_class;

typedef struct _times
{
    t_object x_obj;
    t_float x_f;
} t_times;

typedef struct _scalartimes
{
    t_object x_obj;
    t_float x_f;
    t_float x_g;
} t_scalartimes;

static void *times_new(t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) post("*~: extra arguments ignored");
    if (argc) 
    {
        t_scalartimes *x = (t_scalartimes *)pd_new(scalartimes_class);
        inlet_newFloat(&x->x_obj, &x->x_g);
        x->x_g = atom_getFloatAtIndex(0, argc, argv);
        outlet_new(&x->x_obj, &s_signal);
        x->x_f = 0;
        return (x);
    }
    else
    {
        t_times *x = (t_times *)pd_new(times_class);
        inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal);
        outlet_new(&x->x_obj, &s_signal);
        x->x_f = 0;
        return (x);
    }
}

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

static void times_dsp(t_times *x, t_signal **sp)
{
    if (sp[0]->s_vectorSize&7)
        dsp_add(times_perform, 4,
            sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[0]->s_vectorSize);
    else        
        dsp_add(times_perf8, 4,
            sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[0]->s_vectorSize);
}

static void scalartimes_dsp(t_scalartimes *x, t_signal **sp)
{
    if (sp[0]->s_vectorSize&7)
        dsp_add(scalartimes_perform, 4, sp[0]->s_vector, &x->x_g,
            sp[1]->s_vector, sp[0]->s_vectorSize);
    else        
        dsp_add(scalartimes_perf8, 4, sp[0]->s_vector, &x->x_g,
            sp[1]->s_vector, sp[0]->s_vectorSize);
}

void times_tilde_setup(void)
{
    times_class = class_new(sym___asterisk____tilde__, (t_newmethod)times_new, 0,
        sizeof(t_times), 0, A_GIMME, 0);
    CLASS_SIGNAL(times_class, t_times, x_f);
    class_addMethod(times_class, (t_method)times_dsp, sym_dsp, A_CANT, 0);
    class_setHelpName(times_class, sym_max__tilde__);
    scalartimes_class = class_new(sym___asterisk____tilde__, 0, 0,
        sizeof(t_scalartimes), 0, 0);
    CLASS_SIGNAL(scalartimes_class, t_scalartimes, x_f);
    class_addMethod(scalartimes_class, (t_method)scalartimes_dsp,
        sym_dsp, A_CANT, 0);
    class_setHelpName(scalartimes_class, sym_max__tilde__);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------


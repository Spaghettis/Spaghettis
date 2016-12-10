
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
#include "s_system.h"
#include "d_dsp.h"

/* -------------------------- sig~ ------------------------------ */
static t_class *sig_tilde_class;

typedef struct _sig_tilde
{
    t_object x_obj;
    t_float x_f;
} t_sig;

static void sig_tilde_float(t_sig *x, t_float f)
{
    x->x_f = f;
}

static void sig_tilde_dsp(t_sig *x, t_signal **sp)
{
    dsp_addScalarPerform (&x->x_f, sp[0]->s_vector, sp[0]->s_vectorSize);
    // dsp_add(sig_tilde_perform, 3, &x->x_f, sp[0]->s_vector, sp[0]->s_vectorSize);
}

static void *sig_tilde_new(t_float f)
{
    t_sig *x = (t_sig *)pd_new(sig_tilde_class);
    x->x_f = f;
    outlet_new(&x->x_obj, &s_signal);
    return x;
}

void sig_tilde_setup(void)
{
    sig_tilde_class = class_new(sym_sig__tilde__, (t_newmethod)sig_tilde_new, 0,
        sizeof(t_sig), 0, A_DEFFLOAT, 0);
    class_addFloat(sig_tilde_class, (t_method)sig_tilde_float);
    class_addMethod(sig_tilde_class, (t_method)sig_tilde_dsp,
        sym_dsp, A_CANT, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* -------------------------- line~ ------------------------------ */
static t_class *line_tilde_class;

typedef struct _line_tilde
{
    t_object x_obj;
    t_sample x_target; /* target value of ramp */
    t_sample x_value; /* current value of ramp at block-borders */
    t_sample x_biginc;
    t_sample x_inc;
    t_float x_1overn;
    t_float x_dspticktomsec;
    t_float x_inletvalue;
    t_float x_inletwas;
    int x_ticksleft;
    int x_retarget;
} t_line_tilde;

static t_int *line_tilde_perform(t_int *w)
{
    t_line_tilde *x = (t_line_tilde *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    t_sample f = x->x_value;

    if (PD_BIG_OR_SMALL(f))
            x->x_value = f = 0;
    if (x->x_retarget)
    {
        int nticks = x->x_inletwas * x->x_dspticktomsec;
        if (!nticks) nticks = 1;
        x->x_ticksleft = nticks;
        x->x_biginc = (x->x_target - x->x_value)/(t_float)nticks;
        x->x_inc = x->x_1overn * x->x_biginc;
        x->x_retarget = 0;
    }
    if (x->x_ticksleft)
    {
        t_sample f = x->x_value;
        while (n--) *out++ = f, f += x->x_inc;
        x->x_value += x->x_biginc;
        x->x_ticksleft--;
    }
    else
    {
        t_sample g = x->x_value = x->x_target;
        while (n--)
            *out++ = g;
    }
    return (w+4);
}

/* TB: vectorized version */
static t_int *line_tilde_perf8(t_int *w)
{
    t_line_tilde *x = (t_line_tilde *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    t_sample f = x->x_value;

    if (PD_BIG_OR_SMALL(f))
        x->x_value = f = 0;
    if (x->x_retarget)
    {
        int nticks = x->x_inletwas * x->x_dspticktomsec;
        if (!nticks) nticks = 1;
        x->x_ticksleft = nticks;
        x->x_biginc = (x->x_target - x->x_value)/(t_sample)nticks;
        x->x_inc = x->x_1overn * x->x_biginc;
        x->x_retarget = 0;
    }
    if (x->x_ticksleft)
    {
        t_sample f = x->x_value;
        while (n--) *out++ = f, f += x->x_inc;
        x->x_value += x->x_biginc;
        x->x_ticksleft--;
    }
    else
    {
        t_sample f = x->x_value = x->x_target;
        for (; n; n -= 8, out += 8)
        {
            out[0] = f; out[1] = f; out[2] = f; out[3] = f; 
            out[4] = f; out[5] = f; out[6] = f; out[7] = f;
        }
    }
    return (w+4);
}

static void line_tilde_float(t_line_tilde *x, t_float f)
{
    if (x->x_inletvalue <= 0)
    {
        x->x_target = x->x_value = f;
        x->x_ticksleft = x->x_retarget = 0;
    }
    else
    {
        x->x_target = f;
        x->x_retarget = 1;
        x->x_inletwas = x->x_inletvalue;
        x->x_inletvalue = 0;
    }
}

static void line_tilde_stop(t_line_tilde *x)
{
    x->x_target = x->x_value;
    x->x_ticksleft = x->x_retarget = 0;
}

static void line_tilde_dsp(t_line_tilde *x, t_signal **sp)
{
    if(sp[0]->s_vectorSize&7)
        dsp_add(line_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
    else
        dsp_add(line_tilde_perf8, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
    x->x_1overn = 1./sp[0]->s_vectorSize;
    x->x_dspticktomsec = sp[0]->s_sampleRate / (1000 * sp[0]->s_vectorSize);
}

static void *line_tilde_new(void)
{
    t_line_tilde *x = (t_line_tilde *)pd_new(line_tilde_class);
    outlet_new(&x->x_obj, &s_signal);
    inlet_newFloat(&x->x_obj, &x->x_inletvalue);
    x->x_ticksleft = x->x_retarget = 0;
    x->x_value = x->x_target = x->x_inletvalue = x->x_inletwas = 0;
    return x;
}

void line_tilde_setup(void)
{
    line_tilde_class = class_new(sym_line__tilde__, line_tilde_new, 0,
        sizeof(t_line_tilde), 0, 0);
    class_addFloat(line_tilde_class, (t_method)line_tilde_float);
    class_addMethod(line_tilde_class, (t_method)line_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(line_tilde_class, (t_method)line_tilde_stop,
        sym_stop, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* -------------------------- vline~ ------------------------------ */
static t_class *vline_tilde_class;
#include "s_system.h"    /* for AUDIO_DEFAULT_BLOCKSIZE; this should be in m_pd.h */
typedef struct _vseg
{
    double s_targettime;
    double s_starttime;
    t_sample s_target;
    struct _vseg *s_next;
} t_vseg;

typedef struct _vline
{
    t_object x_obj;
    double x_value;
    double x_inc;
    double x_referencetime;
    double x_lastlogicaltime;
    double x_nextblocktime;
    double x_samppermsec;
    double x_msecpersamp;
    double x_targettime;
    t_sample x_target;
    t_float x_inlet1;
    t_float x_inlet2;
    t_vseg *x_list;
} t_vline;

static t_int *vline_tilde_perform(t_int *w)
{
    t_vline *x = (t_vline *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int n = (int)(w[3]), i;
    double f = x->x_value;
    double inc = x->x_inc;
    double msecpersamp = x->x_msecpersamp;
    double samppermsec = x->x_samppermsec;
    double timenow, logicaltimenow = scheduler_getMillisecondsSince(x->x_referencetime);
    t_vseg *s = x->x_list;
    if (logicaltimenow != x->x_lastlogicaltime)
    {
        int sampstotime = (n > AUDIO_DEFAULT_BLOCKSIZE ? n : AUDIO_DEFAULT_BLOCKSIZE);
        x->x_lastlogicaltime = logicaltimenow;
        x->x_nextblocktime = logicaltimenow - sampstotime * msecpersamp;
    }
    timenow = x->x_nextblocktime;
    x->x_nextblocktime = timenow + n * msecpersamp;
    for (i = 0; i < n; i++)
    {
        double timenext = timenow + msecpersamp;
    checknext:
        if (s)
        {
            /* has starttime elapsed?  If so update value and increment */
            if (s->s_starttime < timenext)
            {
                if (x->x_targettime <= timenext)
                    f = x->x_target, inc = 0;
                    /* if zero-length segment bash output value */
                if (s->s_targettime <= s->s_starttime)
                {
                    f = s->s_target;
                    inc = 0;
                }
                else
                {
                    double incpermsec = (s->s_target - f)/
                        (s->s_targettime - s->s_starttime);
                    f = f + incpermsec * (timenext - s->s_starttime);
                    inc = incpermsec * msecpersamp;
                }
                x->x_inc = inc;
                x->x_target = s->s_target;
                x->x_targettime = s->s_targettime;
                x->x_list = s->s_next;
                PD_MEMORY_FREE(s);
                s = x->x_list;
                goto checknext;
            }
        }
        if (x->x_targettime <= timenext)
            f = x->x_target, inc = x->x_inc = 0, x->x_targettime = 1e20;
        *out++ = f;
        f = f + inc;
        timenow = timenext;
    }
    x->x_value = f;
    return (w+4);
}

static void vline_tilde_stop(t_vline *x)
{
    t_vseg *s1, *s2;
    for (s1 = x->x_list; s1; s1 = s2)
        s2 = s1->s_next, PD_MEMORY_FREE(s1);
    x->x_list = 0;
    x->x_inc = 0;
    x->x_inlet1 = x->x_inlet2 = 0;
    x->x_target = x->x_value;
    x->x_targettime = 1e20;
}

static void vline_tilde_float(t_vline *x, t_float f)
{
    double timenow = scheduler_getMillisecondsSince(x->x_referencetime);
    t_float inlet1 = (x->x_inlet1 < 0 ? 0 : x->x_inlet1);
    t_float inlet2 = x->x_inlet2;
    double starttime = timenow + inlet2;
    t_vseg *s1, *s2, *deletefrom = 0, *snew;
    if (PD_BIG_OR_SMALL(f))
        f = 0;

        /* negative delay input means stop and jump immediately to new value */
    if (inlet2 < 0)
    {
        x->x_value = f;
        vline_tilde_stop(x);
        return;
    }
    snew = (t_vseg *)PD_MEMORY_GET(sizeof(*snew));
        /* check if we supplant the first item in the list.  We supplant
        an item by having an earlier starttime, or an equal starttime unless
        the equal one was instantaneous and the new one isn't (in which case
        we'll do a jump-and-slide starting at that time.) */
    if (!x->x_list || x->x_list->s_starttime > starttime ||
        (x->x_list->s_starttime == starttime &&
            (x->x_list->s_targettime > x->x_list->s_starttime || inlet1 <= 0)))
    {
        deletefrom = x->x_list;
        x->x_list = snew;
    }
    else
    {
        for (s1 = x->x_list; s2 = s1->s_next; s1 = s2)
        {
            if (s2->s_starttime > starttime ||
                (s2->s_starttime == starttime &&
                    (s2->s_targettime > s2->s_starttime || inlet1 <= 0)))
            {
                deletefrom = s2;
                s1->s_next = snew;
                goto didit;
            }
        }
        s1->s_next = snew;
        deletefrom = 0;
    didit: ;
    }
    while (deletefrom)
    {
        s1 = deletefrom->s_next;
        PD_MEMORY_FREE(deletefrom);
        deletefrom = s1;
    }
    snew->s_next = 0;
    snew->s_target = f;
    snew->s_starttime = starttime;
    snew->s_targettime = starttime + inlet1;
    x->x_inlet1 = x->x_inlet2 = 0;
}

static void vline_tilde_dsp(t_vline *x, t_signal **sp)
{
    dsp_add(vline_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
    x->x_samppermsec = ((double)(sp[0]->s_sampleRate)) / 1000;
    x->x_msecpersamp = ((double)1000) / sp[0]->s_sampleRate;
}

static void *vline_tilde_new(void)
{
    t_vline *x = (t_vline *)pd_new(vline_tilde_class);
    outlet_new(&x->x_obj, &s_signal);
    inlet_newFloat(&x->x_obj, &x->x_inlet1);
    inlet_newFloat(&x->x_obj, &x->x_inlet2);
    x->x_inlet1 = x->x_inlet2 = 0;
    x->x_value = x->x_inc = 0;
    x->x_referencetime = x->x_lastlogicaltime = x->x_nextblocktime =
        scheduler_getLogicalTime();
    x->x_list = 0;
    x->x_samppermsec = 0;
    x->x_targettime = 1e20;
    return x;
}

void vline_tilde_setup(void)
{
    vline_tilde_class = class_new(sym_vline__tilde__, vline_tilde_new, 
        (t_method)vline_tilde_stop, sizeof(t_vline), 0, 0);
    class_addFloat(vline_tilde_class, (t_method)vline_tilde_float);
    class_addMethod(vline_tilde_class, (t_method)vline_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(vline_tilde_class, (t_method)vline_tilde_stop,
        sym_stop, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* -------------------------- snapshot~ ------------------------------ */
static t_class *snapshot_tilde_class;

typedef struct _snapshot
{
    t_object x_obj;
    t_sample x_value;
    t_float x_f;
} t_snapshot;

static void *snapshot_tilde_new(void)
{
    t_snapshot *x = (t_snapshot *)pd_new(snapshot_tilde_class);
    x->x_value = 0;
    outlet_new(&x->x_obj, &s_float);
    x->x_f = 0;
    return x;
}

static t_int *snapshot_tilde_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    *out = *in;
    return (w+3);
}

static void snapshot_tilde_dsp(t_snapshot *x, t_signal **sp)
{
    dsp_add(snapshot_tilde_perform, 2, sp[0]->s_vector + (sp[0]->s_vectorSize-1),
        &x->x_value);
}

static void snapshot_tilde_bang(t_snapshot *x)
{
    outlet_float(x->x_obj.te_outlet, x->x_value);
}

static void snapshot_tilde_set(t_snapshot *x, t_float f)
{
    x->x_value = f;
}

void snapshot_tilde_setup(void)
{
    snapshot_tilde_class = class_new(sym_snapshot__tilde__, snapshot_tilde_new, 0,
        sizeof(t_snapshot), 0, 0);
    CLASS_SIGNAL(snapshot_tilde_class, t_snapshot, x_f);
    class_addMethod(snapshot_tilde_class, (t_method)snapshot_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(snapshot_tilde_class, (t_method)snapshot_tilde_set,
        sym_set, A_DEFFLOAT, 0);
    class_addBang(snapshot_tilde_class, snapshot_tilde_bang);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

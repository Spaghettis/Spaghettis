/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  send~, receive~, throw~, catch~ */

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include <string.h>

#define DEFSENDVS 64    /* LATER get send to get this from canvas */

/* ----------------------------- send~ ----------------------------- */
static t_class *sigsend_class;

typedef struct _sigsend
{
    t_object x_obj;
    t_symbol *x_sym;
    int x_n;
    t_sample *x_vec;
    t_float x_f;
} t_sigsend;

static void *sigsend_new(t_symbol *s)
{
    t_sigsend *x = (t_sigsend *)pd_new(sigsend_class);
    pd_bind(&x->x_obj.te_g.g_pd, s);
    x->x_sym = s;
    x->x_n = DEFSENDVS;
    x->x_vec = (t_sample *)PD_MEMORY_GET(DEFSENDVS * sizeof(t_sample));
    memset((char *)(x->x_vec), 0, DEFSENDVS * sizeof(t_sample));
    x->x_f = 0;
    return (x);
}

static t_int *sigsend_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    while (n--)
    {
        *out = (PD_BIG_OR_SMALL(*in) ? 0 : *in);
        out++;
        in++;
    }
    return (w+4);
}

static void sigsend_dsp(t_sigsend *x, t_signal **sp)
{
    if (x->x_n == sp[0]->s_blockSize)
        dsp_add(sigsend_perform, 3, sp[0]->s_vector, x->x_vec, sp[0]->s_blockSize);
    else post_error ("sigsend %s: unexpected vector size", x->x_sym->s_name);
}

static void sigsend_free(t_sigsend *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, x->x_sym);
    PD_MEMORY_FREE(x->x_vec);
}

static void sigsend_setup(void)
{
    sigsend_class = class_new(gensym ("send~"), (t_newmethod)sigsend_new,
        (t_method)sigsend_free, sizeof(t_sigsend), 0, A_DEFSYMBOL, 0);
    class_addCreator((t_newmethod)sigsend_new, gensym ("s~"), A_DEFSYMBOL, 0);
    CLASS_SIGNAL(sigsend_class, t_sigsend, x_f);
    class_addMethod(sigsend_class, (t_method)sigsend_dsp,
        sym_dsp, A_CANT, 0);
}

/* ----------------------------- receive~ ----------------------------- */
static t_class *sigreceive_class;

typedef struct _sigreceive
{
    t_object x_obj;
    t_symbol *x_sym;
    t_sample *x_wherefrom;
    int x_n;
} t_sigreceive;

static void *sigreceive_new(t_symbol *s)
{
    t_sigreceive *x = (t_sigreceive *)pd_new(sigreceive_class);
    x->x_n = DEFSENDVS;             /* LATER find our vector size correctly */
    x->x_sym = s;
    x->x_wherefrom = 0;
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

static t_int *sigreceive_perform(t_int *w)
{
    t_sigreceive *x = (t_sigreceive *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    t_sample *in = x->x_wherefrom;
    if (in)
    {
        while (n--)
            *out++ = *in++; 
    }
    else
    {
        while (n--)
            *out++ = 0; 
    }
    return (w+4);
}

/* tb: vectorized receive function */
static t_int *sigreceive_perf8(t_int *w)
{
    t_sigreceive *x = (t_sigreceive *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    t_sample *in = x->x_wherefrom;
    if (in)
    {
        for (; n; n -= 8, in += 8, out += 8)
        {
            out[0] = in[0]; out[1] = in[1]; out[2] = in[2]; out[3] = in[3]; 
            out[4] = in[4]; out[5] = in[5]; out[6] = in[6]; out[7] = in[7]; 
        }
    }
    else
    {
        for (; n; n -= 8, in += 8, out += 8)
        {
            out[0] = 0; out[1] = 0; out[2] = 0; out[3] = 0; 
            out[4] = 0; out[5] = 0; out[6] = 0; out[7] = 0; 
        }
    }
    return (w+4);
}

static void sigreceive_set(t_sigreceive *x, t_symbol *s)
{
    t_sigsend *sender = (t_sigsend *)pd_findByClass((x->x_sym = s),
        sigsend_class);
    if (sender)
    {
        if (sender->x_n == x->x_n)
            x->x_wherefrom = sender->x_vec;
        else
        {
            post_error ("receive~ %s: vector size mismatch", x->x_sym->s_name);
            x->x_wherefrom = 0;
        }
    }
    else
    {
        post_error ("receive~ %s: no matching send", x->x_sym->s_name);
        x->x_wherefrom = 0;
    }
}

static void sigreceive_dsp(t_sigreceive *x, t_signal **sp)
{
    if (sp[0]->s_blockSize != x->x_n)
    {
        post_error ("receive~ %s: vector size mismatch", x->x_sym->s_name);
    }
    else
    {
        sigreceive_set(x, x->x_sym);
        if (sp[0]->s_blockSize&7)
            dsp_add(sigreceive_perform, 3,
                x, sp[0]->s_vector, sp[0]->s_blockSize);
        else dsp_add(sigreceive_perf8, 3,
            x, sp[0]->s_vector, sp[0]->s_blockSize);
    }
}

static void sigreceive_setup(void)
{
    sigreceive_class = class_new(gensym ("receive~"),
        (t_newmethod)sigreceive_new, 0,
        sizeof(t_sigreceive), 0, A_DEFSYMBOL, 0);
    class_addCreator((t_newmethod)sigreceive_new, gensym ("r~"), A_DEFSYMBOL, 0);
    class_addMethod(sigreceive_class, (t_method)sigreceive_set, sym_set,
        A_SYMBOL, 0);
    class_addMethod(sigreceive_class, (t_method)sigreceive_dsp,
        sym_dsp, A_CANT, 0);
    class_setHelpName(sigreceive_class, gensym ("send~"));
}

/* ----------------------------- catch~ ----------------------------- */
static t_class *sigcatch_class;

typedef struct _sigcatch
{
    t_object x_obj;
    t_symbol *x_sym;
    int x_n;
    t_sample *x_vec;
} t_sigcatch;

static void *sigcatch_new(t_symbol *s)
{
    t_sigcatch *x = (t_sigcatch *)pd_new(sigcatch_class);
    pd_bind(&x->x_obj.te_g.g_pd, s);
    x->x_sym = s;
    x->x_n = DEFSENDVS;
    x->x_vec = (t_sample *)PD_MEMORY_GET(DEFSENDVS * sizeof(t_sample));
    memset((char *)(x->x_vec), 0, DEFSENDVS * sizeof(t_sample));
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

static t_int *sigcatch_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    while (n--) *out++ = *in, *in++ = 0; 
    return (w+4);
}

/* tb: vectorized catch function */
static t_int *sigcatch_perf8(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    for (; n; n -= 8, in += 8, out += 8)
    {
       out[0] = in[0]; out[1] = in[1]; out[2] = in[2]; out[3] = in[3]; 
       out[4] = in[4]; out[5] = in[5]; out[6] = in[6]; out[7] = in[7]; 
    
       in[0] = 0; in[1] = 0; in[2] = 0; in[3] = 0; 
       in[4] = 0; in[5] = 0; in[6] = 0; in[7] = 0; 
    }
    return (w+4);
}

static void sigcatch_dsp(t_sigcatch *x, t_signal **sp)
{
    if (x->x_n == sp[0]->s_blockSize)
    {
        if(sp[0]->s_blockSize&7)
        dsp_add(sigcatch_perform, 3, x->x_vec, sp[0]->s_vector, sp[0]->s_blockSize);
        else
        dsp_add(sigcatch_perf8, 3, x->x_vec, sp[0]->s_vector, sp[0]->s_blockSize);
    }
    else post_error ("sigcatch %s: unexpected vector size", x->x_sym->s_name);
}

static void sigcatch_free(t_sigcatch *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, x->x_sym);
    PD_MEMORY_FREE(x->x_vec);
}

static void sigcatch_setup(void)
{
    sigcatch_class = class_new(gensym ("catch~"), (t_newmethod)sigcatch_new,
        (t_method)sigcatch_free, sizeof(t_sigcatch), CLASS_NOINLET, A_DEFSYMBOL, 0);
    class_addMethod(sigcatch_class, (t_method)sigcatch_dsp,
        sym_dsp, A_CANT, 0);
    class_setHelpName(sigcatch_class, gensym ("throw~"));
}

/* ----------------------------- throw~ ----------------------------- */
static t_class *sigthrow_class;

typedef struct _sigthrow
{
    t_object x_obj;
    t_symbol *x_sym;
    t_sample *x_whereto;
    int x_n;
    t_float x_f;
} t_sigthrow;

static void *sigthrow_new(t_symbol *s)
{
    t_sigthrow *x = (t_sigthrow *)pd_new(sigthrow_class);
    x->x_sym = s;
    x->x_whereto  = 0;
    x->x_n = DEFSENDVS;
    x->x_f = 0;
    return (x);
}

static t_int *sigthrow_perform(t_int *w)
{
    t_sigthrow *x = (t_sigthrow *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    t_sample *out = x->x_whereto;
    if (out)
    {
        while (n--)
        {
            *out += (PD_BIG_OR_SMALL(*in) ? 0 : *in);
            out++;
            in++;
        }
    }
    return (w+4);
}

static void sigthrow_set(t_sigthrow *x, t_symbol *s)
{
    t_sigcatch *catcher = (t_sigcatch *)pd_findByClass((x->x_sym = s),
        sigcatch_class);
    if (catcher)
    {
        if (catcher->x_n == x->x_n)
            x->x_whereto = catcher->x_vec;
        else
        {
            post_error ("throw~ %s: vector size mismatch", x->x_sym->s_name);
            x->x_whereto = 0;
        }
    }
    else
    {
        post_error ("throw~ %s: no matching catch", x->x_sym->s_name);
        x->x_whereto = 0;
    }
}

static void sigthrow_dsp(t_sigthrow *x, t_signal **sp)
{
    if (sp[0]->s_blockSize != x->x_n)
    {
        post_error ("throw~ %s: vector size mismatch", x->x_sym->s_name);
    }
    else
    {
        sigthrow_set(x, x->x_sym);
        dsp_add(sigthrow_perform, 3,
            x, sp[0]->s_vector, sp[0]->s_blockSize);
    }
}

static void sigthrow_setup(void)
{
    sigthrow_class = class_new(gensym ("throw~"), (t_newmethod)sigthrow_new, 0,
        sizeof(t_sigthrow), 0, A_DEFSYMBOL, 0);
    class_addMethod(sigthrow_class, (t_method)sigthrow_set, sym_set,
        A_SYMBOL, 0);
    CLASS_SIGNAL(sigthrow_class, t_sigthrow, x_f);
    class_addMethod(sigthrow_class, (t_method)sigthrow_dsp,
        sym_dsp, A_CANT, 0);
}

/* ----------------------- global setup routine ---------------- */

void d_global_setup(void)
{
    sigsend_setup();
    sigreceive_setup();
    sigcatch_setup();
    sigthrow_setup();
}


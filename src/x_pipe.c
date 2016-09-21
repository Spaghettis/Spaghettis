
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
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* -------------------------- pipe -------------------------- */

static t_class *pipe_class;

typedef struct _hang
{
    t_clock *h_clock;
    struct _hang *h_next;
    struct _pipe *h_owner;
    t_gpointer *h_gp;
    union word h_vec[1];        /* not the actual number. */
} t_hang;

typedef struct pipeout
{
    t_atom p_atom;
    t_outlet *p_outlet;
} t_pipeout;

typedef struct _pipe
{
    t_object x_obj;
    int x_n;
    int x_nptr;
    t_float x_deltime;
    t_pipeout *x_vec;
    t_gpointer *x_gp;
    t_hang *x_hang;
} t_pipe;

static void *pipe_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pipe *x = (t_pipe *)pd_new(pipe_class);
    t_atom defarg, *ap;
    t_pipeout *vec, *vp;
    t_gpointer *gp;
    int nptr = 0;
    int i;
    t_float deltime;
    if (argc)
    {
        if (argv[argc-1].a_type != A_FLOAT)
        {
            char stupid[80];
            atom_toString(&argv[argc-1], stupid, 79);
            post_error ("pipe: %s: bad time delay value", stupid);
            deltime = 0;
        }
        else deltime = argv[argc-1].a_w.w_float;
        argc--;
    }
    else deltime = 0;
    if (!argc)
    {
        argv = &defarg;
        argc = 1;
        SET_FLOAT(&defarg, 0);
    }
    x->x_n = argc;
    vec = x->x_vec = (t_pipeout *)PD_MEMORY_GET(argc * sizeof(*x->x_vec));

    for (i = argc, ap = argv; i--; ap++)
        if (ap->a_type == A_SYMBOL && *ap->a_w.w_symbol->s_name == 'p')
            nptr++;

    gp = x->x_gp = (t_gpointer *)PD_MEMORY_GET(nptr * sizeof (*gp));
    x->x_nptr = nptr;

    for (i = 0, vp = vec, ap = argv; i < argc; i++, ap++, vp++)
    {
        if (ap->a_type == A_FLOAT)
        {
            vp->p_atom = *ap;
            vp->p_outlet = outlet_new(&x->x_obj, &s_float);
            if (i) inlet_newFloat(&x->x_obj, &vp->p_atom.a_w.w_float);
        }
        else if (ap->a_type == A_SYMBOL)
        {
            char c = *ap->a_w.w_symbol->s_name;
            if (c == 's')
            {
                SET_SYMBOL(&vp->p_atom, &s_symbol);
                vp->p_outlet = outlet_new(&x->x_obj, &s_symbol);
                if (i) inlet_newSymbol(&x->x_obj, &vp->p_atom.a_w.w_symbol);
            }
            else if (c == 'p')
            {
                vp->p_atom.a_type = A_POINTER;
                vp->p_atom.a_w.w_gpointer = gp;
                gpointer_init(gp);
                vp->p_outlet = outlet_new(&x->x_obj, &s_pointer);
                if (i) inlet_newPointer(&x->x_obj, gp);
                gp++;
            }
            else
            {
                if (c != 'f') post_error ("pipe: %s: bad type",
                    ap->a_w.w_symbol->s_name);
                SET_FLOAT(&vp->p_atom, 0);
                vp->p_outlet = outlet_new(&x->x_obj, &s_float);
                if (i) inlet_newFloat(&x->x_obj, &vp->p_atom.a_w.w_float);
            }
        }
    }
    inlet_newFloat(&x->x_obj, &x->x_deltime);
    x->x_hang = 0;
    x->x_deltime = deltime;
    return (x);
}

static void hang_free(t_hang *h)
{
    t_pipe *x = h->h_owner;
    t_gpointer *gp;
    int i;
    for (gp = h->h_gp, i = x->x_nptr; i--; gp++)
        gpointer_unset(gp);
    PD_MEMORY_FREE(h->h_gp);
    clock_free(h->h_clock);
    PD_MEMORY_FREE(h);
}

static void hang_tick(t_hang *h)
{
    t_pipe *x = h->h_owner;
    t_hang *h2, *h3;
    t_pipeout *p;
    int i;
    union word *w;
    if (x->x_hang == h) x->x_hang = h->h_next;
    else for (h2 = x->x_hang; h3 = h2->h_next; h2 = h3)
    {
        if (h3 == h)
        {
            h2->h_next = h3->h_next;
            break;
        }
    }
    for (i = x->x_n, p = x->x_vec + (x->x_n - 1), w = h->h_vec + (x->x_n - 1);
        i--; p--, w--)
    {
        switch (p->p_atom.a_type)
        {
        case A_FLOAT: outlet_float(p->p_outlet, w->w_float); break;
        case A_SYMBOL: outlet_symbol(p->p_outlet, w->w_symbol); break;
        case A_POINTER:
            if (gpointer_isValidNullAllowed(w->w_gpointer))
                outlet_pointer(p->p_outlet, w->w_gpointer);
            else post_error ("pipe: stale pointer");
            break;
        }
    }
    hang_free(h);
}

static void pipe_list(t_pipe *x, t_symbol *s, int ac, t_atom *av)
{
    t_hang *h = (t_hang *)
        PD_MEMORY_GET(sizeof(*h) + (x->x_n - 1) * sizeof(*h->h_vec));
    t_gpointer *gp, *gp2;
    t_pipeout *p;
    int i, n = x->x_n;
    t_atom *ap;
    t_word *w;
    h->h_gp = (t_gpointer *)PD_MEMORY_GET(x->x_nptr * sizeof(t_gpointer));
    if (ac > n)
    {
        if (av[n].a_type == A_FLOAT)
            x->x_deltime = av[n].a_w.w_float;
        else post_error ("pipe: symbol or pointer in time inlet");
        ac = n;
    }
    for (i = 0, gp = x->x_gp, p = x->x_vec, ap = av; i < ac;
        i++, p++, ap++)
    {
        switch (p->p_atom.a_type)
        {
        case A_FLOAT: p->p_atom.a_w.w_float = atom_getFloat(ap); break;
        case A_SYMBOL: p->p_atom.a_w.w_symbol = atom_getSymbol(ap); break;
        case A_POINTER:
            if (ap->a_type != A_POINTER) {
                gpointer_unset (gp);
                post_error ("pipe: bad pointer");
            } else {
                gpointer_setByCopy (ap->a_w.w_gpointer, gp);
            }
            gp++;
        }
    }
    for (i = 0, gp = x->x_gp, gp2 = h->h_gp, p = x->x_vec, w = h->h_vec;
        i < n; i++, p++, w++)
    {
        if (p->p_atom.a_type == A_POINTER)
        {
            // gpointer_setByCopy ?
            gpointer_retain (gp);           /* Remove ASAP. */
            gpointer_rawCopy (gp, gp2);     /* Remove ASAP. */
            
            w->w_gpointer = gp2;
            gp++;
            gp2++;
        }
        else *w = p->p_atom.a_w;
    }
    h->h_next = x->x_hang;
    x->x_hang = h;
    h->h_owner = x;
    h->h_clock = clock_new(h, (t_method)hang_tick);
    clock_delay(h->h_clock, (x->x_deltime >= 0 ? x->x_deltime : 0));
}

static void pipe_flush(t_pipe *x)
{
    while (x->x_hang) hang_tick(x->x_hang);
}

static void pipe_clear(t_pipe *x)
{
    t_hang *hang;
    while (hang = x->x_hang)
    {
        x->x_hang = hang->h_next;
        hang_free(hang);
    }
}

static void pipe_free(t_pipe *x)
{
    pipe_clear(x);
    PD_MEMORY_FREE(x->x_vec);
}

void pipe_setup(void)
{
    pipe_class = class_new(sym_pipe, 
        (t_newmethod)pipe_new, (t_method)pipe_free,
        sizeof(t_pipe), 0, A_GIMME, 0);
    class_addList(pipe_class, pipe_list);
    class_addMethod(pipe_class, (t_method)pipe_flush, sym_flush, 0);
    class_addMethod(pipe_class, (t_method)pipe_clear, sym_clear, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

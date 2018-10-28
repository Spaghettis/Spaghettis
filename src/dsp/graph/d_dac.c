
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_sample *audio_soundOut;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class  *dac_tilde_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _dac_tilde {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_f;
    int         x_size;
    int         *x_vector;
    } t_dac_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void dac_tilde_setProceed (t_dac_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    int i, k = PD_MIN (argc, x->x_size);
    
    for (i = 0; i < k; i++) { x->x_vector[i] = (int)atom_getFloatAtIndex (i, argc, argv); }
}

static void dac_tilde_set (t_dac_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    dac_tilde_setProceed (x, s, argc, argv); dsp_update();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void dac_tilde_dsp (t_dac_tilde *x, t_signal **sp)
{
    t_error err  = PD_ERROR_NONE;
    t_signal **s = sp;
    int i;
        
    for (i = 0; i < x->x_size; i++) {
    //
    int channel = x->x_vector[i] - 1;
    int k = audio_getTotalOfChannelsOut();
    t_signal *t = (*s);
    int n = t->s_vectorSize;
    
    if (n != INTERNAL_BLOCKSIZE) { err = PD_ERROR; }
    else {
    //
    if (channel >= 0 && channel < k) {
    //
    t_sample *out = audio_soundOut + (INTERNAL_BLOCKSIZE * channel);
    
    dsp_addPlusPerformAliased (out, t->s_vector, out, INTERNAL_BLOCKSIZE);
    //
    }
    //
    }
        
    s++;
    //
    }
    
    if (err) { error_invalid (sym_dac__tilde__, sym_signal); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

// -- TODO: For now doesn't restore signal values. Is that needed?

static t_buffer *dac_tilde_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_dac_tilde *x = (t_dac_tilde *)z;
    t_buffer *b = buffer_new();
    int i;
    
    buffer_appendSymbol (b, sym__restore);
    
    for (i = 0; i < x->x_size; i++) { buffer_appendFloat (b, x->x_vector[i]); }
    
    return b;
    //
    }
    
    return NULL;
}

static void dac_tilde_restore (t_dac_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    dac_tilde_setProceed (x, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *dac_tilde_new (t_symbol *s, int argc, t_atom *argv)
{
    t_dac_tilde *x = (t_dac_tilde *)pd_new (dac_tilde_class);
    int i;
    
    x->x_size   = argc ? argc : 2;
    x->x_vector = (int *)PD_MEMORY_GET (x->x_size * sizeof (int));
    
    if (!argc) { x->x_vector[0] = 1; x->x_vector[1] = 2; }
    else {
        for (i = 0; i < argc; i++) { x->x_vector[i] = (int)atom_getFloatAtIndex (i, argc, argv); }
    }
    
    for (i = 1; i < x->x_size; i++) { inlet_newSignal (cast_object (x)); }
    
    return x;
}

static void dac_tilde_free (t_dac_tilde *x)
{
    PD_MEMORY_FREE (x->x_vector);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void dac_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_dac__tilde__,
            (t_newmethod)dac_tilde_new,
            (t_method)dac_tilde_free,
            sizeof (t_dac_tilde),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
        
    CLASS_SIGNAL (c, t_dac_tilde, x_f);
    
    class_addDSP (c, (t_method)dac_tilde_dsp);
    
    class_addMethod (c, (t_method)dac_tilde_set,        sym_set,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)dac_tilde_restore,    sym__restore,   A_GIMME, A_NULL);

    class_setDataFunction (c, dac_tilde_functionData);
    class_setHelpName (c, sym_audio);
    
    dac_tilde_class = c;
}

void dac_tilde_destroy (void)
{
    class_free (dac_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

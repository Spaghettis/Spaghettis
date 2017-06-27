
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "s_midi.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *bendout_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _bendout {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_channel;
    } t_bendout;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void bendout_float (t_bendout *x, t_float f)
{
    outmidi_pitchBend (x->x_channel, (int)f + 8192);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *bendout_new (t_float channel)
{
    t_bendout *x = (t_bendout *)pd_new (bendout_class);
    
    x->x_channel = channel;
    
    inlet_newFloat (cast_object (x), &x->x_channel);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void bendout_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_bendout,
            (t_newmethod)bendout_new,
            NULL,
            sizeof (t_bendout),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);
            
    class_addFloat (c, (t_method)bendout_float);
    
    class_setHelpName (c, sym_midiout);
    
    bendout_class = c;
}

void bendout_destroy (void)
{
    class_free (bendout_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------


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

static t_class *line_tilde_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _line_tilde {
    t_object    x_obj;                  /* Must be the first. */
    t_sample    x_target;
    t_sample    x_current;
    t_sample    x_incrementPerTick;
    t_sample    x_incrementPerSample;
    t_float     x_inverseOfVectorSize;
    t_float     x_millisecondsToTicks;
    t_float     x_timeRamp;
    t_float     x_timeRampCurrent;
    int         x_ticksLeft;
    int         x_retarget;
    t_outlet    *x_outlet;
    } t_line_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void line_tilde_float (t_line_tilde *x, t_float f)
{
    if (x->x_timeRamp <= 0) {
    
        x->x_target             = f;
        x->x_current            = f;
        x->x_ticksLeft          = 0;
        x->x_retarget           = 0;
        
    } else {
    
        x->x_target             = f;
        x->x_retarget           = 1;
        x->x_timeRampCurrent    = x->x_timeRamp;
        x->x_timeRamp           = 0.0;
    }
}

static void line_tilde_stop (t_line_tilde *x)
{
    x->x_target     = x->x_current;
    x->x_ticksLeft  = 0;
    x->x_retarget   = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *line_tilde_perform (t_int *w)
{
    t_line_tilde *x = (t_line_tilde *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    t_sample f;
    
    if (x->x_retarget) {
    
        int numberOfTicks = PD_MAX (1, (int)(x->x_timeRampCurrent * x->x_millisecondsToTicks));
        
        x->x_ticksLeft          = numberOfTicks;
        x->x_incrementPerTick   = (x->x_target - x->x_current) / (t_float)(numberOfTicks);
        x->x_incrementPerSample = x->x_incrementPerTick * x->x_inverseOfVectorSize;
        x->x_retarget           = 0;
    }
    
    if (x->x_ticksLeft) {
    
        f = x->x_current;
        while (n--) { *out++ = f; f += x->x_incrementPerSample; }
        x->x_current += x->x_incrementPerTick; x->x_ticksLeft--;
        
    } else {
    
        f = x->x_target;
        while (n--) { *out++ = f; }
        x->x_current = f;
    }
    
    return (w + 4);
}

static void line_tilde_dsp (t_line_tilde *x, t_signal **sp)
{
    x->x_inverseOfVectorSize = (t_float)(1.0 / sp[0]->s_vectorSize);
    x->x_millisecondsToTicks = (t_float)(sp[0]->s_sampleRate / (1000.0 * sp[0]->s_vectorSize));
    
    dsp_add (line_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_buffer *line_tilde_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_line_tilde *x = (t_line_tilde *)z;
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, &s_float);
    buffer_appendFloat (b, x->x_target);
    
    return b;
    //
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *line_tilde_new (void)
{
    t_line_tilde *x = (t_line_tilde *)pd_new (line_tilde_class);
    
    x->x_outlet = outlet_newSignal (cast_object (x));
    
    inlet_newFloat (cast_object (x), &x->x_timeRamp);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void line_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_line__tilde__,
            (t_newmethod)line_tilde_new,
            NULL,
            sizeof (t_line_tilde),
            CLASS_DEFAULT,
            A_NULL);
        
    class_addDSP (c, (t_method)line_tilde_dsp);
    class_addFloat (c, (t_method)line_tilde_float);
    
    class_addMethod (c, (t_method)line_tilde_stop, sym_stop, A_NULL);
    
    class_setDataFunction (c, line_tilde_functionData);

    line_tilde_class = c;
}

void line_tilde_destroy (void)
{
    class_free (line_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------


/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"
#include "../../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Write to buffer. */

static t_int *vinlet_performPrologue (t_int *w)
{
    t_vinlet *x = (t_vinlet *)(w[1]);
    PD_RESTRICTED in = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    t_sample *out = x->vi_bufferWrite;
    
    if (out == x->vi_bufferEnd) {
        t_sample *f1 = x->vi_buffer;
        t_sample *f2 = x->vi_buffer + x->vi_hopSize;
        int shift    = x->vi_bufferSize - x->vi_hopSize;
        out -= x->vi_hopSize;
        // PD_LOG ("SHIFT");
        // PD_LOG_NUMBER (x->vi_hopSize);
        // PD_LOG ("/");
        // PD_LOG_NUMBER (shift);
        while (shift--) { *f1++ = *f2++; }
    }
    
    // PD_LOG ("P");
    // PD_LOG_NUMBER (out - x->vi_buffer);
    // PD_LOG ("/");
    // PD_LOG_NUMBER (n);
    
    while (n--) { *out++ = *in++; }
    
    x->vi_bufferWrite = out;
    
    return (w + 4);
}

/* Read from buffer. */

static t_int *vinlet_perform (t_int *w)
{
    t_vinlet *x = (t_vinlet *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    t_sample *in = x->vi_bufferRead;

    // PD_LOG ("R");
    // PD_LOG_NUMBER (in - x->vi_buffer);
    // PD_LOG ("/");
    // PD_LOG_NUMBER (n);
    
    while (n--) { *out++ = *in++; }
    if (in == x->vi_bufferEnd) { in = x->vi_buffer; }
    
    x->vi_bufferRead = in;
    
    return (w + 4);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void vinlet_dspPrologue (t_vinlet *x, t_signal **signals, t_blockproperties *p)
{
    if (vinlet_isSignal (x)) {
    //
    if (!p->bp_reblocked) {     /* Vector sizes are equal thus no buffering is required. */
    
        PD_ASSERT (signals); x->vi_directSignal = signals[inlet_getIndexAsSignal (x->vi_inlet)];

    } else {                    /* Buffering required. */
    //
    t_signal *s = NULL;
    int parentVectorSize = 1;
    int vectorSize = 1;
    int bufferSize;

    resample_setRatio (&x->vi_resample, p->bp_downsample, p->bp_upsample);
    
    x->vi_directSignal = NULL;
    
    if (signals) {
        s = signals[inlet_getIndexAsSignal (x->vi_inlet)];
        parentVectorSize = s->s_vectorSize;
        vectorSize = parentVectorSize * p->bp_upsample / p->bp_downsample;
    }

    bufferSize = PD_MAX (p->bp_blockSize, vectorSize);
    
    if (bufferSize != x->vi_bufferSize) {
        PD_MEMORY_FREE (x->vi_buffer);
        x->vi_bufferSize = bufferSize;
        x->vi_buffer     = (t_sample *)PD_MEMORY_GET (x->vi_bufferSize * sizeof (t_sample));
        x->vi_bufferEnd  = x->vi_buffer + x->vi_bufferSize;
    }
    
    if (!signals) { memset (x->vi_buffer, 0, x->vi_bufferSize * sizeof (t_sample)); }
    else {
    //
    t_sample *t = NULL;
    int phase = (int)((chain_getPhase (instance_getChain()) - 1) & (t_phase)(p->bp_period - 1));
    
    x->vi_hopSize     = p->bp_period * vectorSize;
    x->vi_bufferWrite = phase ? x->vi_bufferEnd - (x->vi_hopSize - (phase * vectorSize)) : x->vi_bufferEnd;
    
    // PD_LOG ("INLET BUFFER");
    // PD_LOG_NUMBER (bufferSize);
    // PD_LOG ("INLET PHASE");
    // PD_LOG_NUMBER (phase);
    // PD_LOG ("INLET HOP");
    // PD_LOG_NUMBER (x->vi_hopSize);
    
    PD_ASSERT (x->vi_hopSize <= x->vi_bufferSize);
    
    if (!resample_isRequired (&x->vi_resample)) { t = s->s_vector; }    /* Original signal. */
    else {
        t = resample_setBuffer (&x->vi_resample, s->s_vector, parentVectorSize, vectorSize);  /* Resampled. */
    }

    dsp_add (vinlet_performPrologue, 3, x, t, vectorSize);
    //
    }
    //
    }
    //
    }
}

void vinlet_dsp (t_vinlet *x, t_signal **sp)
{
    if (vinlet_isSignal (x)) {
    //
    t_signal *out = sp[0];
            
    if (x->vi_directSignal) { signal_borrow (out, x->vi_directSignal); }    /* By-pass the inlet. */
    else {
    //
    /* No phase required. */ 
    /* Submultiple read is always completed at each tick. */
    
    x->vi_bufferRead = x->vi_buffer;
    
    dsp_add (vinlet_perform, 3, x, out->s_vector, out->s_vectorSize);
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

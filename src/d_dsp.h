
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __d_dsp_h_
#define __d_dsp_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/*

    Ugen's routines build a temporary graph from the DSP objects.
    It is sorted next to obtain a linear list of operations to perform. 
    Memory for signals is allocated according to the interconnections.
    Once that's been done, the graph is deleted (while the signals remain).
    
    Prologue and epilogue functions manage nested graphs relations.
    With resampling and reblocking it could require additional buffers.

    The "block~" object maintains the synchronisation with the parent's DSP process.
    It does NOT do any computation in its own right.
    It triggers associated ugens at a supermultiple or submultiple of the upstream.
    Note that it can also be invoked just as a switch.
    
    The overall order of scheduling is:

        - inlets prologue
        - block prologue
        - the ugens in the graph, including inlets and outlets
        - block epilogue
        - outlets epilogue

    The related functions called are:
 
        - vinlet_performPrologue
        - block_performPrologue
        - vinlet_perform
        - voutlet_perform
        - block_performEpilogue
        - voutlet_performEpilogue
 
    Note that jumps can occurs to by-pass or redo a sequence.
 
*/

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define SOUNDFILE_CHANNELS  64

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "dsp/d_dspthread.h"
#include "dsp/d_chain.h"
#include "dsp/d_closures.h"
#include "dsp/d_block.h"
#include "dsp/d_resample.h"
#include "dsp/d_functions.h"
#include "dsp/d_macros.h"
#include "dsp/d_cos.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct _vinlet {
    t_object                vi_obj;             /* Must be the first. */
    t_resample              vi_resample;        /* Extended buffer if resampling is required. */
    int                     vi_dismissed;
    int                     vi_bufferSize;
    t_sample                *vi_buffer;         /* Handle vector size conversion in a buffer. */
    t_vinletclosure         *vi_closure;
    t_glist                 *vi_owner;
    t_outlet                *vi_outlet;
    t_inlet                 *vi_inlet;
    t_signal                *vi_directSignal;   /* Used to efficiently by-pass the inlet. */
    };

struct _voutlet {
    t_object                vo_obj;             /* Must be the first. */
    t_resample              vo_resample;        /* Extended buffer if resampling is required. */
    int                     vo_dismissed;
    int                     vo_copyOut;         /* Behavior is to perform a copy ("switch~" object). */
    int                     vo_bufferSize;
    t_sample                *vo_buffer;         /* Handle vector size conversion in a buffer. */
    t_voutletclosure        *vo_closure;
    t_glist                 *vo_owner;
    t_outlet                *vo_outlet;
    t_signal                *vo_directSignal;   /* Used to efficiently by-pass the outlet. */
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _sigoutconnect {
    int                     oc_index;
    struct _ugenbox         *oc_to;
    struct _sigoutconnect   *oc_next;
    } t_sigoutconnect;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _sigoutlet {
    int                     o_numberOfConnections;
    t_sigoutconnect         *o_connections;
    t_signal                *o_signal;
    } t_sigoutlet;

typedef struct _siginlet {
    int                     i_numberOfConnections;
    int                     i_numberAlreadyConnected;
    t_signal                *i_signal;
    } t_siginlet;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _ugenbox {
    int                     u_done;
    int                     u_inSize;
    int                     u_outSize;
    t_siginlet              *u_in;
    t_sigoutlet             *u_out;
    t_object                *u_owner;
    struct _ugenbox         *u_next;
    } t_ugenbox;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct _dspcontext {
    int                     dc_numberOfInlets;
    int                     dc_numberOfOutlets;
    t_float                 dc_sampleRate;
    int                     dc_blockSize;
    int                     dc_overlap;
    t_ugenbox               *dc_ugens;
    struct _dspcontext      *dc_parentContext;
    t_signal                **dc_signals;
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        dsp_setState                (int n);
int         dsp_getState                (void);

void        dsp_update                  (void);
int         dsp_suspend                 (void);
void        dsp_resume                  (int oldState);

void        dsp_close                   (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_signal    *signal_newWithSignal       (t_signal *s);
t_signal    *signal_newWithContext      (t_dspcontext *context);

void        signal_borrow               (t_signal *s,  t_signal *toBeBorrowed);
int         signal_isCompatibleWith     (t_signal *s1, t_signal *s2);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_dspcontext    *ugen_graphStart        (int isTopLevel, t_signal **sp, int m, int n);

void        ugen_graphAdd               (t_dspcontext *context, t_object *o);
void        ugen_graphConnect           (t_dspcontext *context, t_object *o1, int m, t_object *o2, int n);
void        ugen_graphClose             (t_dspcontext *context);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        canvas_dspProceed           (t_glist *glist, int isTopLevel, t_signal **sp);
t_float     canvas_getSampleRate        (t_glist *glist);
t_float     canvas_getBlockSize         (t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        vinlet_dsp                  (t_vinlet *x, t_signal **sp);
void        vinlet_dspPrologue          (t_vinlet *x, t_signal **signals,  t_blockproperties *properties);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        voutlet_dsp                 (t_voutlet *x, t_signal **sp);
void        voutlet_dspPrologue         (t_voutlet *x, t_signal **signals, t_blockproperties *properties);
void        voutlet_dspEpilogue         (t_voutlet *x, t_signal **signals, t_blockproperties *properties);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_dsp_h_

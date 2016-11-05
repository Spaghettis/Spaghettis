
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __d_dsp_h_
#define __d_dsp_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _signal {
    t_float         s_sampleRate;
    int             s_count;
    int             s_isBorrowed;
    int             s_vectorSize;
    t_sample        *s_vector;
    struct _signal  *s_borrowedFrom;
    struct _signal  *s_nextReusable;
    struct _signal  *s_nextUsed;
    };

typedef struct _resample {
    int             r_type;
    int             r_downSample;
    int             r_upSample;
    int             r_vectorSize;
    int             r_coefficientsSize;
    int             r_bufferSize;
    t_sample        *r_vector;
    t_sample        *r_coefficients;
    t_sample        *r_buffer;
    } t_resample;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _vinlet {
    t_object        vi_obj;                         /* Must be the first. */
    t_resample      vi_resampling;
    int             vi_hopSize;
    int             vi_bufferSize;
    t_sample        *vi_buffer;
    t_sample        *vi_bufferEnd;
    t_sample        *vi_bufferWrite;
    t_sample        *vi_bufferRead;
    t_glist         *vi_owner;
    t_outlet        *vi_outlet;
    t_inlet         *vi_inlet;
    t_signal        *vi_directSignal;
    };

struct _voutlet {
    t_object        vo_obj;                         /* Must be the first. */
    t_resample      vo_resampling;
    int             vo_hopSize;
    int             vo_copyOut;
    int             vo_bufferSize;
    t_sample        *vo_buffer;
    t_sample        *vo_bufferEnd;
    t_sample        *vo_bufferRead;
    t_sample        *vo_bufferWrite;
    t_glist         *vo_owner;
    t_outlet        *vo_outlet;
    t_signal        *vo_directSignal;
    };
    
typedef struct _block {
    t_object        bk_obj;                         /* Must be the first. */
    int             bk_blockSize;
    int             bk_overlap;
    int             bk_phase;
    int             bk_period;
    int             bk_frequency;
    int             bk_count;
    int             bk_allBlockLength;
    int             bk_outletEpilogLength;
    int             bk_isSwitchObject;
    int             bk_isSwitchedOn;
    int             bk_isReblocked;
    int             bk_upSample;
    int             bk_downSample;
    } t_block;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef t_int *(*t_perform)(t_int *args);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            dsp_state                   (void *dummy, t_symbol *s, int argc, t_atom *argv);
void            dsp_update                  (void);
int             dsp_suspend                 (void);
void            dsp_resume                  (int oldState);
t_int           dsp_done                    (t_int *w);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_signal        *signal_new                 (int blockSize, t_float sampleRate);

void            signal_borrow               (t_signal *s, t_signal *toBeBorrowed);
void            signal_free                 (t_signal *s);
void            signal_clean                (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            ugen_dspInitialize          (void);
void            ugen_dspTick                (void);
void            ugen_dspRelease             (void);
int             ugen_getBuildIdentifier     (void);

t_dspcontext    *ugen_graphStart            (int isTopLevel, t_signal **sp, int m, int n);

void            ugen_graphAdd               (t_dspcontext *context, t_object *o);
void            ugen_graphConnect           (t_dspcontext *context, t_object *o1, int m, t_object *o2, int n);
void            ugen_graphClose             (t_dspcontext *context);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            canvas_dspPerform           (t_glist *glist, int isTopLevel, t_signal **sp);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            vinlet_dsp                  (t_vinlet *x, t_signal **sp);
void            vinlet_dspProlog            (t_vinlet *x,
                                                t_signal **parentSignals,
                                                int vectorSize,
                                                int phase,
                                                int period,
                                                int frequency,
                                                int downSample,
                                                int upSample,
                                                int reblock,
                                                int switched);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            voutlet_dsp                 (t_voutlet *x, t_signal **sp);
void            voutlet_dspProlog           (t_voutlet *x,
                                                t_signal **parentSignals,
                                                int vectorSize,
                                                int phase,
                                                int period,
                                                int frequency,
                                                int downSample,
                                                int upSample,
                                                int reblock,
                                                int switched);
                                                            
void            voutlet_dspEpilog           (t_voutlet *x,
                                                t_signal **parentSignals,
                                                int vectorSize,
                                                int phase,
                                                int period,
                                                int frequency,
                                                int downSample,
                                                int upSample,
                                                int reblock,
                                                int switched);
                                        
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            dsp_addZeroPerform          (t_sample *s, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_int   *plus_perform       (t_int *args);
t_int   *copy_perform       (t_int *args);

void    dsp_add_plus        (t_sample *in1, t_sample *in2, t_sample *out, int n);
void    dsp_add_copy        (t_sample *in, t_sample *out, int n);
void    dsp_add_scalarcopy  (t_float *in, t_sample *out, int n);

void    dsp_add             (t_perform f, int n, ...);
void    pd_fft              (t_float *buffer, int npoints, int inverse);

void    mayer_fht           (t_sample *fz, int n);
void    mayer_fft           (int n, t_sample *real, t_sample *imag);
void    mayer_ifft          (int n, t_sample *real, t_sample *imag);
void    mayer_realfft       (int n, t_sample *real);
void    mayer_realifft      (int n, t_sample *real);

void    resample_init       (t_resample *x, t_symbol *type);
void    resample_free       (t_resample *x);
void    resample_dsp        (t_resample *x, t_sample *in, int insize, t_sample *out, int outsize, int m);
void    resamplefrom_dsp    (t_resample *x, t_sample *in, int insize, int outsize, int m);
void    resampleto_dsp      (t_resample *x, t_sample *out, int insize, int outsize, int m);

t_int   *block_dspProlog    (t_int *w);
t_int   *block_dspEpilog    (t_int *w);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_dsp_h_

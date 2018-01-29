
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __d_resample_h_
#define __d_resample_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _resample {
    int         r_type;
    int         r_downsample;
    int         r_upsample;
    t_sample    r_buffer;
    int         r_allocatedSize;
    t_sample    *r_vector;
    } t_resample;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        resample_init                   (t_resample *x, t_symbol *type);
void        resample_free                   (t_resample *x);
void        resample_setRatio               (t_resample *x, int downsample, int upsample);
int         resample_isRequired             (t_resample *x);
void        resample_getBuffer              (t_resample *x, t_sample *s, int vectorSize, int resampledSize);
t_sample    *resample_setBuffer             (t_resample *x, t_sample *s, int vectorSize, int resampledSize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_sample *resample_vector (t_resample *x)
{
    return x->r_vector;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_resample_h_

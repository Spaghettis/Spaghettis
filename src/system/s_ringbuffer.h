
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __s_ringbuffer_h_
#define __s_ringbuffer_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* A basic ring buffer for a single writer and a single reader. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* The number of elements MUST be a power of two. */

t_ringbuffer    *ringbuffer_new         (int32_t sizeOfElementInBytes, int32_t numberOfElements);

void    ringbuffer_free                 (t_ringbuffer *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Functions below could be called concurrently. */

int32_t ringbuffer_getAvailableWrite    (t_ringbuffer *x);                              /* Usable in DSP. */
int32_t ringbuffer_getAvailableRead     (t_ringbuffer *x);                              /* Usable in DSP. */
int32_t ringbuffer_write                (t_ringbuffer *x, const void *v, int32_t n);    /* Usable in DSP. */
int32_t ringbuffer_read                 (t_ringbuffer *x, void *v, int32_t n);          /* Usable in DSP. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_ringbuffer_h_

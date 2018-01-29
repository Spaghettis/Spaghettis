
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_signal *signal_new (int vectorSize, t_float sampleRate)
{
    PD_ASSERT (PD_IS_POWER_2 (vectorSize)); 
    PD_ABORT (!PD_IS_POWER_2 (vectorSize));
    
    t_signal *s = (t_signal *)PD_MEMORY_GET (sizeof (t_signal));
    
    s->s_sampleRate     = sampleRate;
    s->s_vectorSize     = vectorSize;
    s->s_vector         = (t_sample *)PD_MEMORY_GET (vectorSize * sizeof (t_sample));
    s->s_unused         = NULL;
    
    instance_signalAdd (s);
    
    return s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int signal_isCompatibleWith (t_signal *s1, t_signal *s2)
{
    return (s1->s_vectorSize == s2->s_vectorSize && s1->s_sampleRate == s2->s_sampleRate);
}

void signal_borrow (t_signal *s, t_signal *toBeBorrowed)
{
    PD_ASSERT (s->s_hasBorrowed == 0);
    PD_ASSERT (signal_isCompatibleWith (s, toBeBorrowed));
    
    s->s_hasBorrowed    = 1;
    s->s_unused         = s->s_vector;
    s->s_vectorSize     = toBeBorrowed->s_vectorSize;
    s->s_vector         = toBeBorrowed->s_vector;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_float signal_getSampleRate (t_signal *s)
{
    return s->s_sampleRate;
}

int signal_getVectorSize (t_signal *s)
{
    return s->s_vectorSize;
}

t_sample *signal_getVector (t_signal *s)
{
    return s->s_vector;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------


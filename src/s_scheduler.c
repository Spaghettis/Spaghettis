
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define SCHEDULER_RUN           0
#define SCHEDULER_QUIT          1
#define SCHEDULER_RESTART       2
#define SCHEDULER_ERROR         3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define SCHEDULER_BLOCKING_LAPSE        1000

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static volatile sig_atomic_t scheduler_quit;            /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int      scheduler_audioMode;                    /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_systime scheduler_getLogicalTime (void)
{
    return instance_getLogicalTime();
}

t_systime scheduler_getLogicalTimeAfter (double ms)
{
    return (instance_getLogicalTime() + (SYSTIME_PER_MILLISECOND * ms));
}

double scheduler_getUnitsSince (t_systime systime, double unit, int isSamples)
{
    double d;
    t_systime elapsed = instance_getLogicalTime() - systime;
    
    PD_ASSERT (elapsed >= 0.0);
    
    if (isSamples) { d = SYSTIME_PER_SECOND / audio_getSampleRate(); } 
    else { 
        d = SYSTIME_PER_MILLISECOND;
    }
    
    return (elapsed / (d * unit));
}

double scheduler_getMillisecondsSince (t_systime systime)
{
    t_systime elapsed = instance_getLogicalTime() - systime;
    
    return (elapsed / SYSTIME_PER_MILLISECOND);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void scheduler_setAudioMode (int flag)
{
    scheduler_audioMode = flag;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void scheduler_needToExit (void)
{
    scheduler_quit = SCHEDULER_QUIT;
}

void scheduler_needToExitWithError (void)
{
    scheduler_quit = SCHEDULER_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_systime scheduler_getSystimePerDSPTick (void)
{
    return (SYSTIME_PER_SECOND * ((double)INTERNAL_BLOCKSIZE / audio_getSampleRate()));
}

static void scheduler_pollStuck (int init)
{
    static double idleTime;
    
    if (init) { idleTime = sys_getRealTimeInSeconds(); }
    else {
        if (sys_getRealTimeInSeconds() - idleTime > 1.0) {
            audio_close();
            scheduler_setAudioMode (SCHEDULER_AUDIO_NONE);
            if (!scheduler_quit) { scheduler_quit = SCHEDULER_RESTART; }
            error_ioStuck();
        }
    }
}

static void scheduler_tick (void)
{
    if (!scheduler_quit) { 
    //
    t_systime t = instance_getLogicalTime() + scheduler_getSystimePerDSPTick();
    
    instance_clockTick (t);
    //
    }
    
    if (!scheduler_quit) { ugen_dspTick(); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void scheduler_mainLoop (void)
{
    int idleCount = 0;
    
    double realTimeAtStart = sys_getRealTimeInSeconds();
    t_systime logicalTimeAtStart = scheduler_getLogicalTime();
    
    midi_start();
    
    while (!scheduler_quit) {
    //
    int timeForward, didSomething = 0;

    if (scheduler_audioMode != SCHEDULER_AUDIO_NONE) {
        if ((timeForward = audio_poll())) { idleCount = 0; }
        else {
            if (!(++idleCount % 31)) { 
                scheduler_pollStuck (idleCount == 32);
            }
        }
        
    } else {
        double realLapse = SECONDS_TO_MILLISECONDS (sys_getRealTimeInSeconds() - realTimeAtStart);
        double logicalLapse = scheduler_getMillisecondsSince (logicalTimeAtStart);

        if (realLapse > logicalLapse) { timeForward = DACS_YES; }
        else {
            timeForward = DACS_NO;
        }
    }
    
    if (!scheduler_quit) {
    //
    if (timeForward != DACS_NO)  { scheduler_tick(); }
    if (timeForward == DACS_YES) { didSomething = 1; }

    midi_poll();
    
    if (!scheduler_quit && gui_pollOrFlush()) { didSomething = 1; }
    if (!scheduler_quit && !didSomething) {
        if (timeForward != DACS_SLEPT) {
            monitor_blocking (SCHEDULER_BLOCKING_LAPSE);
        }
    }
    //
    }
    
    if (scheduler_quit == SCHEDULER_RESTART) { scheduler_quit = SCHEDULER_RUN; }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_error scheduler_main (void)
{
    midi_open();
    instance_autoreleaseRun();
    instance_pollingRun();
    
    scheduler_mainLoop();
    
    instance_pollingStop();
    instance_autoreleaseStop();
    dsp_suspend();
    audio_close();
    midi_close();
    
    return (scheduler_quit == SCHEDULER_ERROR);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

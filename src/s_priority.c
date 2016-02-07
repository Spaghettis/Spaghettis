
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if ( PD_LINUX || PD_BSD || PD_HURD )

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_symbol *main_rootDirectory;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern int interface_watchdogPipe; 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if defined ( _POSIX_MEMLOCK ) && ( _POSIX_MEMLOCK > 0 )
    #define PRIORITY_MEMLOCK
#endif

#if defined ( _POSIX_PRIORITY_SCHEDULING ) && ( _POSIX_PRIORITY_SCHEDULING > 0 )
    #define PRIORITY_SCHEDULING
#endif 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifdef PRIORITY_SCHEDULING

static t_error priority_setRealTimeScheduling (int isWatchdog) 
{
    struct sched_param param;
    int p1 = sched_get_priority_min (SCHED_FIFO);
    int p2 = sched_get_priority_max (SCHED_FIFO);
    int p3;
    
    #ifdef USEAPI_JACK    
        param.sched_priority = (isWatchdog ? p1 + 7 : p1 + 5);
    #else
        param.sched_priority = (isWatchdog ? p2 - 5 : p2 - 7);
    #endif

    return (sched_setscheduler (0, SCHED_FIFO, &param) == -1);
}

#endif // PRIORITY_SCHEDULING

#ifdef PRIORITY_MEMLOCK

static t_error priority_setRealTimeMemoryLocking (void) 
{       
    return (mlockall (MCL_CURRENT | MCL_FUTURE) == -1);
}

#endif // PRIORITY_MEMLOCK

static t_error priority_setRealTime (int isWatchdog) 
{
    t_error err = PD_ERROR_NONE;
    
    #ifdef PRIORITY_SCHEDULING
        err |= priority_setRealTimeScheduling (isWatchdog);
    #endif

    #ifdef PRIORITY_MEMLOCK
        err |= priority_setRealTimeMemoryLocking();
    #endif
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // ( PD_LINUX || PD_BSD || PD_HURD )

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_APPLE

static t_error priority_setRTPlatformSpecific (void)
{
    t_error err = PD_ERROR_NONE;
    
    struct sched_param param;
    
    memset (&param, 0, sizeof (struct sched_param));
    
    /* The POSIX thread API is able to set thread priority only within the lowest priority band (0–63). */
    /* Use thread_policy_set instead? */
    /* < https://developer.apple.com/library/ios/technotes/tn2169/_index.html > */
    
    param.sched_priority = 63; 

    err = (pthread_setschedparam (pthread_self(), SCHED_RR, &param) != 0);
    
    PD_ASSERT (!err);
    
    return err;
}

#elif PD_WINDOWS

static t_error priority_setRTPlatformSpecific (void)
{
    if (!SetPriorityClass (GetCurrentProcess(), HIGH_PRIORITY_CLASS)) { PD_BUG; }
}

#elif PD_WATCHDOG

static t_error priority_setRTPlatformSpecific (void)
{
    t_error err = PD_ERROR_NONE;
    
    char command[PD_STRING] = { 0 };
    
    err = string_sprintf (command, "%s/bin/pdwatchdog", main_rootDirectory->s_name);
    
    if (!err && !(err = path_isFileExist (command))) {
    //
    int p[2];

    if (!(err = (pipe (p) < 0))) {
    //
    int pid = fork();
    
    if (pid < 0)   { PD_BUG; err = PD_ERROR; }
    else if (!pid) {                                            /* We're the child. */
        priority_setRealTime (1);
        setuid (getuid());                                      /* Child lose setuid privileges. */
        if (p[1] != 0) { dup2 (p[0], 0); close (p[0]); }
        close (p[1]);
        execl ("/bin/sh", "sh", "-c", command, NULL); 
        _exit(1);

    } else {                                                    /* We're the parent. */
        if (priority_setRealTime (0)) { PD_BUG; }
        close (p[0]);
        fcntl (p[1], F_SETFD, FD_CLOEXEC);
        interface_watchdogPipe = p[1];
    }
    //
    }
    //
    }

    return err;
}

#else

static t_error priority_setRTPlatformSpecific (void)
{
    PD_BUG; return PD_ERROR_NONE;
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WITH_REALTIME

t_error priority_setPolicy (void)
{
    t_error err = priority_setRTPlatformSpecific();
    
    #if PD_WATCHDOG
    #if ! ( PD_WITH_NOGUI )
    
    if (!err) { sys_gui ("::watchdog\n"); }
    
    #endif
    #endif

    return err;
}

#else

t_error priority_setPolicy (void)
{
    return PD_ERROR_NONE;
}

#endif // PD_WITH_REALTIME
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

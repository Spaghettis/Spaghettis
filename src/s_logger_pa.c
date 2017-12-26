
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if ( PD_WITH_DEBUG && PD_WITH_LOGGER )

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "portaudio.h"
#include "pa_ringbuffer.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_symbol *main_directorySupport;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static char                     *logger_buffer;                 /* Static. */

static PaUtilRingBuffer         logger_ring;                    /* Static. */
static int                      logger_file;                    /* Static. */
static pthread_t                logger_thread;                  /* Static. */
static pthread_attr_t           logger_attribute;               /* Static. */
static int                      logger_quit;                    /* Static. */
static int                      logger_running;                 /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PA_LOGGER_CHUNK         (64)
#define PA_LOGGER_SLEEP         (57)
#define PA_LOGGER_BUFFER_SIZE   (1024 * PA_LOGGER_SLEEP)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *logger_task (void *dummy)
{
    while (!logger_quit) {
    //
    usleep ((useconds_t)MILLISECONDS_TO_MICROSECONDS (PA_LOGGER_SLEEP));
    
    {
        char t[PA_LOGGER_CHUNK + 1] = { 0 };
        ring_buffer_size_t size;
                
        while ((size = PaUtil_GetRingBufferReadAvailable (&logger_ring)) > 0) {
            ring_buffer_size_t k = PD_MIN (PA_LOGGER_CHUNK, size);
            PaUtil_ReadRingBuffer (&logger_ring, t, k);
            t[k] = '\n';
            write (logger_file, t, k + 1);
        }
    }
    //
    }
    
    pthread_exit (NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error logger_initializeNative (void)
{
    size_t k = PD_NEXT_POWER_2 (PA_LOGGER_BUFFER_SIZE);
    char t[PD_STRING] = { 0 };
    t_error err = string_sprintf (t, PD_STRING, "%s/log-XXXXXX", main_directorySupport->s_name);
    
    if (!err && (logger_file = mkstemp (t)) != -1) {
    //
    logger_buffer = (char *)PD_MEMORY_GET (k);
    
    if (!PaUtil_InitializeRingBuffer (&logger_ring,     // --
            (ring_buffer_size_t)sizeof (char),
            (ring_buffer_size_t)k,
            (void *)logger_buffer)) {
        pthread_attr_init (&logger_attribute);
        pthread_attr_setdetachstate (&logger_attribute, PTHREAD_CREATE_JOINABLE);
        if (!(err = (pthread_create (&logger_thread, &logger_attribute, logger_task, NULL) != 0))) {
            logger_running = 1;
        }
        return err;
    }
    //
    }
    
    return PD_ERROR;
}

void logger_releaseNative (void)
{
    logger_quit = 1;
    logger_running = 0;
    
    pthread_join (logger_thread, NULL);
    pthread_attr_destroy (&logger_attribute);
    
    if (logger_buffer) { 
        PD_MEMORY_FREE (logger_buffer);
        logger_buffer = NULL; 
    }
    
    if (logger_file != -1) { close (logger_file); }
}

int logger_isRunningNative (void)
{
    return logger_running;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void logger_appendStringNative (const char *s)
{
    PaUtil_WriteRingBuffer (&logger_ring, s, (ring_buffer_size_t)strlen (s));
}

void logger_appendFloatNative (double f)
{
    char t[LOGGER_FLOAT_STRING] = { 0 }; logger_appendStringNative (logger_stringWithFloat (t, f));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

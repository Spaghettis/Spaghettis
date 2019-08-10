
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "jack/jack.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

double audio_getNanosecondsToSleep (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define JACK_MAXIMUM_PORTS  128

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_sample *audio_soundIn;
extern t_sample *audio_soundOut;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static jack_client_t        *jack_client;                                       /* Static. */

static jack_port_t          *jack_portsIn[JACK_MAXIMUM_PORTS];                  /* Static. */
static jack_port_t          *jack_portsOut[JACK_MAXIMUM_PORTS];                 /* Static. */

static t_ringbuffer         *jack_ringIn[JACK_MAXIMUM_PORTS];                   /* Static. */
static t_ringbuffer         *jack_ringOut[JACK_MAXIMUM_PORTS];                  /* Static. */

static int                  jack_numberOfPortsIn;                               /* Static. */
static int                  jack_numberOfPortsOut;                              /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define JACK_GRAIN          5

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define JACK_BUFFER         8192        /* Buffer size (per channel). */
                                        /* MUST be a power of two. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void audio_vectorShrinkIn   (int);
void audio_vectorShrinkOut  (int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void jack_buffersFree (void)
{
    int i;

    for (i = 0; i < JACK_MAXIMUM_PORTS; i++) {
        if (jack_ringIn[i])  { ringbuffer_free (jack_ringIn[i]);  jack_ringIn[i]  = NULL; }
        if (jack_ringOut[i]) { ringbuffer_free (jack_ringOut[i]); jack_ringOut[i] = NULL; }
    }
}

static void jack_buffersAllocate (int numberOfChannelsIn, int numberOfChannelsOut)
{
    int i;

    jack_buffersFree();

    for (i = 0; i < numberOfChannelsIn;  i++) {
        jack_ringIn[i]  = ringbuffer_new (sizeof (t_sample), JACK_BUFFER);
    }
    
    for (i = 0; i < numberOfChannelsOut; i++) {
        jack_ringOut[i] = ringbuffer_new (sizeof (t_sample), JACK_BUFFER);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Take ALL channels or NONE. */

static int jack_pollCallback (jack_nframes_t framesCount, void *dummy)
{
    int i;

    if (jack_numberOfPortsOut) {
    //
    int readable = 1;

    for (i = 0; i < jack_numberOfPortsOut; i++) {
    //
    if (ringbuffer_getAvailableRead (jack_ringOut[i]) < (int32_t)framesCount) { readable = 0; break; }
    //
    }

    for (i = 0; i < jack_numberOfPortsOut; i++) {
    //
    void *t = jack_port_get_buffer (jack_portsOut[i], framesCount);

    if (readable) {
        ringbuffer_read (jack_ringOut[i], t, framesCount);
        
    } else {
        PD_LOG ("*@*");
        memset (t, 0, framesCount * sizeof (t_sample));   /* Fill with zeros. */
    }
    //
    }
    //
    }
    
    if (jack_numberOfPortsIn) {
    //
    int writable = 1;

    for (i = 0; i < jack_numberOfPortsIn; i++) {
    //
    if (ringbuffer_getAvailableWrite (jack_ringIn[i]) < (int32_t)framesCount) { writable = 0; break; }
    //
    }

    for (i = 0; i < jack_numberOfPortsIn; i++) {
    //
    void *t = jack_port_get_buffer (jack_portsIn[i], framesCount);

    if (writable) { ringbuffer_write (jack_ringIn[i], t, framesCount); }    /* Simply drop if full. */
    //
    }
    //
    }
    
    return 0;
}

static void jack_shutdownCallback (void *dummy)
{
    jack_client = NULL; scheduler_needToExitWithError();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

const char *audio_nameNative (void)
{
    static const char *name = "JACK"; return name;      /* Static. */
}

t_error audio_initializeNative (void)
{
    return PD_ERROR_NONE;
}

void audio_releaseNative (void)
{
    jack_buffersFree();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error audio_openNative (t_devices *p)
{
    int numberOfChannelsIn  = devices_getInSize (p)  ? devices_getInChannelsAtIndex (p, 0)  : 0;
    int numberOfChannelsOut = devices_getOutSize (p) ? devices_getOutChannelsAtIndex (p, 0) : 0;
    int sampleRate          = devices_getSampleRate (p);
        
    PD_ASSERT (sizeof (t_sample) == sizeof (jack_default_audio_sample_t));
    PD_ABORT  (sizeof (t_sample) != sizeof (jack_default_audio_sample_t));
    
    if (numberOfChannelsIn) {   /* For now audio in is required to synchronize properly the callback. */
    //
    jack_status_t status;
    
    numberOfChannelsIn  = PD_MIN (numberOfChannelsIn, JACK_MAXIMUM_PORTS);
    numberOfChannelsOut = PD_MIN (numberOfChannelsOut, JACK_MAXIMUM_PORTS);
 
    PD_ASSERT (!jack_client);

    jack_client = jack_client_open (PD_NAME_LOWERCASE, JackNoStartServer, &status, NULL);
    
    if (jack_client) {
    //
    if (jack_get_sample_rate (jack_client) != (jack_nframes_t)sampleRate) {
        jack_client_close (jack_client);
        jack_client = NULL;
        error_invalid (sym_JACK, sym_samplerate);
    }
    //
    }

    if (jack_client) {
    //
    int i;
    
    jack_buffersAllocate (numberOfChannelsIn, numberOfChannelsOut);

    jack_set_process_callback (jack_client, jack_pollCallback, NULL);
    jack_on_shutdown (jack_client, jack_shutdownCallback, NULL);

    for (i = 0; i < numberOfChannelsIn; i++) {
    //
    char t[PD_STRING] = { 0 };
    string_sprintf (t, PD_STRING, "input_%d", i + 1);
    jack_portsIn[i] = jack_port_register (jack_client, t, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if (!jack_portsIn[i]) {
        error_failed (sym_JACK);
        break;
    }
    //
    }

    audio_vectorShrinkIn (jack_numberOfPortsIn = numberOfChannelsIn = i);
    
    for (i = 0; i < numberOfChannelsOut; i++) {
    //
    char t[PD_STRING] = { 0 };
    string_sprintf (t, PD_STRING, "output_%d", i + 1);
    jack_portsOut[i] = jack_port_register (jack_client, t, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if (!jack_portsOut[i]) {
        error_failed (sym_JACK);
        break;  
    }
    //
    }
    
    audio_vectorShrinkOut (jack_numberOfPortsOut = numberOfChannelsOut = i);
    
    if (!jack_activate (jack_client)) { return PD_ERROR_NONE; }
    //
    }
    //
    }
        
    return PD_ERROR;
}

void audio_closeNative (void) 
{
    if (jack_client) {
    //
    int i;
    jack_deactivate (jack_client);
    for (i = 0; i < jack_numberOfPortsIn; i++)  { 
        jack_port_unregister (jack_client, jack_portsIn[i]); 
        jack_portsIn[i] = NULL;
    }
    for (i = 0; i < jack_numberOfPortsOut; i++) { 
        jack_port_unregister (jack_client, jack_portsOut[i]);
        jack_portsOut[i] = NULL;
    }
    jack_client_close (jack_client);
    jack_client = NULL;
    //
    }

    jack_buffersFree();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int audio_pollNative (void)
{
    int i;
    int status = DACS_YES;
    t_sample *sound = NULL;

    if (!jack_client || (!jack_numberOfPortsIn && !jack_numberOfPortsOut)) { return DACS_NO; }
    else {
    //
    int needToWait = 0; double ns = audio_getNanosecondsToSleep() / (double)JACK_GRAIN;
    
    if (jack_numberOfPortsIn) {
    //
    for (i = 0; i < jack_numberOfPortsIn; i++) {
        while (ringbuffer_getAvailableRead (jack_ringIn[i]) < INTERNAL_BLOCKSIZE) {
            status = DACS_SLEPT;
            if (needToWait < JACK_GRAIN * 2) {
                PD_LOG (".");
                nano_sleep (ns);
            } else { return DACS_NO; }
            needToWait++;
        }
    }
    //
    }
    
    if (jack_numberOfPortsOut) {
    //
    for (i = 0; i < jack_numberOfPortsOut; i++) {
        while (ringbuffer_getAvailableWrite (jack_ringOut[i]) < INTERNAL_BLOCKSIZE) {
            status = DACS_SLEPT;
            if (needToWait < JACK_GRAIN * 2) {
                PD_LOG (".");
                nano_sleep (ns);
            } else { return DACS_NO; }
            needToWait++;
        }
    }
    //
    }

    if (jack_numberOfPortsIn) {
    //
    sound = audio_soundIn;
        
    for (i = 0; i < jack_numberOfPortsIn; i++) {
        ringbuffer_read (jack_ringIn[i], (void *)sound, INTERNAL_BLOCKSIZE);
        sound += INTERNAL_BLOCKSIZE;
    }
    //
    }

    if (jack_numberOfPortsOut) {
    //  
    sound = audio_soundOut;
        
    for (i = 0; i < jack_numberOfPortsOut; i++) {
        ringbuffer_write (jack_ringOut[i], (const void *)sound, INTERNAL_BLOCKSIZE);
        memset ((void *)sound, 0, INTERNAL_BLOCKSIZE * sizeof (t_sample));                  /* Zeroed. */
        sound += INTERNAL_BLOCKSIZE;
    }
    //
    }
    //
    }
    
    return status;
}

// --TODO: Implement it properly.

int audio_getVectorSizeNative (void)
{
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error audio_getListsNative (t_deviceslist *p) 
{
    t_error err = PD_ERROR_NONE;
    
    err |= deviceslist_appendAudioIn (p,  gensym ("JACK ports"), 0);
    err |= deviceslist_appendAudioOut (p, gensym ("JACK ports"), 0);

    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

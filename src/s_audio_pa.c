
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "m_alloca.h"
#include "s_system.h"
#include "s_ringbuffer.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <portaudio.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define PA_WITH_FAKEBLOCKING

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

extern t_sample         *audio_soundIn;
extern t_sample         *audio_soundOut;

extern int              audio_channelsIn;
extern int              audio_channelsOut;
extern t_float          audio_sampleRate;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static PaStream         *pa_stream;
static float            *pa_soundIn;
static float            *pa_soundOut;
static char             *pa_bufferIn;
static char             *pa_bufferOut;

static sys_ringbuf      pa_ringOut;
static sys_ringbuf      pa_ringIn;

static int              pa_channelsIn;
static int              pa_channelsOut;
static int              pa_started;
static int              pa_numberOfBuffers;

static t_audiocallback  pa_callback;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void pa_initialize (void)     /* Initialize PortAudio  */
{
    static int initialized;
    if (!initialized)
    {
#ifdef __APPLE__
        /* for some reason, on the Mac Pa_Initialize() closes file descriptor
        1 (standard output) As a workaround, dup it to another number and dup2
        it back afterward. */
        int newfd = dup(1);
        int another = open("/dev/null", 0);
        dup2(another, 1);
        int err = Pa_Initialize();
        close(1);
        close(another);
        if (newfd >= 0)
        {
            fflush(stdout);
            dup2(newfd, 1);
            close(newfd);
        }
#else
        int err = Pa_Initialize();
#endif


        if ( err != paNoError ) 
        {
            post("Error opening audio: %s", err, Pa_GetErrorText(err));
            return;
        }
        initialized = 1;
    }
}

static int pa_lowlevel_callback(const void *inputBuffer,
    void *outputBuffer, unsigned long nframes,
    const PaStreamCallbackTimeInfo *outTime, PaStreamCallbackFlags myflags, 
    void *userData)
{
    int i; 
    unsigned int n, j;
    float *fbuf, *fp2, *fp3, *soundiop;
    if (nframes % AUDIO_DEFAULT_BLOCKSIZE)
    {
        post("warning: audio nframes %ld not a multiple of blocksize %d",
            nframes, (int)AUDIO_DEFAULT_BLOCKSIZE);
        nframes -= (nframes % AUDIO_DEFAULT_BLOCKSIZE);
    }
    for (n = 0; n < nframes; n += AUDIO_DEFAULT_BLOCKSIZE)
    {
        if (inputBuffer != NULL)
        {
            fbuf = ((float *)inputBuffer) + n*pa_channelsIn;
            soundiop = pa_soundIn;
            for (i = 0, fp2 = fbuf; i < pa_channelsIn; i++, fp2++)
                    for (j = 0, fp3 = fp2; j < AUDIO_DEFAULT_BLOCKSIZE;
                        j++, fp3 += pa_channelsIn)
                            *soundiop++ = *fp3;
        }
        else memset((void *)pa_soundIn, 0,
            AUDIO_DEFAULT_BLOCKSIZE * pa_channelsIn * sizeof(float));
        memset((void *)pa_soundOut, 0,
            AUDIO_DEFAULT_BLOCKSIZE * pa_channelsOut * sizeof(float));
        (*pa_callback)();
        if (outputBuffer != NULL)
        {
            fbuf = ((float *)outputBuffer) + n*pa_channelsOut;
            soundiop = pa_soundOut;
            for (i = 0, fp2 = fbuf; i < pa_channelsOut; i++, fp2++)
                for (j = 0, fp3 = fp2; j < AUDIO_DEFAULT_BLOCKSIZE;
                    j++, fp3 += pa_channelsOut)
                        *fp3 = *soundiop++;
        }
    }
    return 0;
}

#ifdef PA_WITH_FAKEBLOCKING
    /* callback for "non-callback" case in which we actualy open portaudio
    in callback mode but fake "blocking mode". We communicate with the
    main thread via FIFO.  First read the audio output FIFO (which
    we sync on, not waiting for it but supplying zeros to the audio output if
    there aren't enough samples in the FIFO when we are called), then write
    to the audio input FIFO.  The main thread will wait for the input fifo.
    We can either throw it a pthreads condition or just allow the main thread
    to poll for us; so far polling seems to work better. */
static int pa_fifo_callback(const void *inputBuffer,
    void *outputBuffer, unsigned long nframes,
    const PaStreamCallbackTimeInfo *outTime, PaStreamCallbackFlags myflags, 
    void *userData)
{
    /* callback routine for non-callback client... throw samples into
            and read them out of a FIFO */
    int ch;
    long fiforoom;
    float *fbuf;

#if CHECKFIFOS
    if (pa_channelsIn * ringbuffer_getReadAvailable(&pa_ringOut) !=   
        pa_channelsOut * ringbuffer_getWriteAvailable(&pa_ringIn))
            post("warning: in and out rings unequal (%d, %d)",
                ringbuffer_getReadAvailable(&pa_ringOut),
                 ringbuffer_getWriteAvailable(&pa_ringIn));
#endif
    fiforoom = ringbuffer_getReadAvailable(&pa_ringOut);
    if ((unsigned)fiforoom >= nframes*pa_channelsOut*sizeof(float))
    {
        if (outputBuffer)
            ringbuffer_read(&pa_ringOut, outputBuffer,
                nframes*pa_channelsOut*sizeof(float), pa_bufferOut);
        else if (pa_channelsOut)
            post("audio error: no outputBuffer but output channels");
        if (inputBuffer)
            ringbuffer_write(&pa_ringIn, inputBuffer,
                nframes*pa_channelsIn*sizeof(float), pa_bufferIn);
        else if (pa_channelsIn)
            post("audio error: no inputBuffer but input channels");
    }
    else
    {        /* PD could not keep up; generate zeros */
        /* if (pa_started)
            pa_dio_error = 1; */
        if (outputBuffer)
        {
            for (ch = 0; ch < pa_channelsOut; ch++)
            {
                unsigned long frame;
                fbuf = ((float *)outputBuffer) + ch;
                for (frame = 0; frame < nframes; frame++, fbuf += pa_channelsOut)
                    *fbuf = 0;
            }
        }
    }
#ifdef PA_WITH_THREADSIGNAL
    pthread_mutex_lock(&pa_mutex);
    pthread_cond_signal(&pa_cond);
    pthread_mutex_unlock(&pa_mutex);
#endif
    return 0;
}
#endif /* PA_WITH_FAKEBLOCKING */

PaError pa_open_callback(double sampleRate, int inchannels, int outchannels,
    int framesperbuf, int nbuffers, int indeviceno, int outdeviceno, PaStreamCallback *callbackfn)
{
    long   bytesPerSample;
    PaError err;
    PaStreamParameters instreamparams, outstreamparams;
    PaStreamParameters*p_instreamparams=0, *p_outstreamparams=0;

    /* fprintf(stderr, "nchan %d, flags %d, bufs %d, framesperbuf %d\n",
            nchannels, flags, nbuffers, framesperbuf); */

    instreamparams.device = indeviceno;
    instreamparams.channelCount = inchannels;
    instreamparams.sampleFormat = paFloat32;
    instreamparams.hostApiSpecificStreamInfo = 0;
    
    outstreamparams.device = outdeviceno;
    outstreamparams.channelCount = outchannels;
    outstreamparams.sampleFormat = paFloat32;
    outstreamparams.hostApiSpecificStreamInfo = 0;

#ifdef PA_WITH_FAKEBLOCKING
    instreamparams.suggestedLatency = outstreamparams.suggestedLatency = 0;
#else
    instreamparams.suggestedLatency = outstreamparams.suggestedLatency =
        nbuffers*framesperbuf/sampleRate;
#endif /* PA_WITH_FAKEBLOCKING */

    if( inchannels>0 && indeviceno >= 0)
        p_instreamparams=&instreamparams;
    if( outchannels>0 && outdeviceno >= 0)
        p_outstreamparams=&outstreamparams;

    err=Pa_IsFormatSupported(p_instreamparams, p_outstreamparams, sampleRate);

    if (paFormatIsSupported != err)
    {
        /* check whether we have to change the numbers of channel and/or samplerate */
        const PaDeviceInfo* info = 0;
        double inRate=0, outRate=0;

        if (inchannels>0)
        {
            if (NULL != (info = Pa_GetDeviceInfo( instreamparams.device )))
            {
              inRate=info->defaultSampleRate;

              if(info->maxInputChannels<inchannels)
                instreamparams.channelCount=info->maxInputChannels;
            }
        }

        if (outchannels>0)
        {
            if (NULL != (info = Pa_GetDeviceInfo( outstreamparams.device )))
            {
              outRate=info->defaultSampleRate;

              if(info->maxOutputChannels<outchannels)
                outstreamparams.channelCount=info->maxOutputChannels;
            }
        }

        if (err == paInvalidSampleRate)
        {
            sampleRate=outRate;
        }

        err=Pa_IsFormatSupported(p_instreamparams, p_outstreamparams,
            sampleRate);
        if (paFormatIsSupported != err)
        goto error;
    }
    err = Pa_OpenStream(
              &pa_stream,
              p_instreamparams,
              p_outstreamparams,
              sampleRate,
              framesperbuf,
              paNoFlag,      /* portaudio will clip for us */
              callbackfn,
              0);
    if (err != paNoError)
        goto error;

    err = Pa_StartStream(pa_stream);
    if (err != paNoError)
    {
        post("error opening failed; closing audio stream: %s",
            Pa_GetErrorText(err));
        pa_close();
        goto error;
    }
    audio_sampleRate=sampleRate;
    return paNoError;
error:
    pa_stream = NULL;
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int pa_open(int inchans, int outchans, int rate, t_sample *soundin,
    t_sample *soundout, int framesperbuf, int nbuffers,
    int indeviceno, int outdeviceno, t_audiocallback callbackfn)
{
    PaError err;
    int j, devno, pa_indev = -1, pa_outdev = -1;
    
    pa_callback = callbackfn;
    /* fprintf(stderr, "open callback %d\n", (callbackfn != 0)); */
    pa_initialize();
    /* post("in %d out %d rate %d device %d", inchans, outchans, rate, deviceno); */
    
    if (pa_stream)
        pa_close();

    if (inchans > 0)
    {
        for (j = 0, devno = 0; j < Pa_GetDeviceCount(); j++)
        {
            const PaDeviceInfo *info = Pa_GetDeviceInfo(j);
            if (info->maxInputChannels > 0)
            {
                if (devno == indeviceno)
                {
                    if (inchans > info->maxInputChannels)
                      inchans = info->maxInputChannels;

                    pa_indev = j;
                    break;
                }
                devno++;
            }
        }
    }   
    
    if (outchans > 0)
    {
        for (j = 0, devno = 0; j < Pa_GetDeviceCount(); j++)
        {
            const PaDeviceInfo *info = Pa_GetDeviceInfo(j);
            if (info->maxOutputChannels > 0)
            {
                if (devno == outdeviceno)
                {
                    if (outchans > info->maxOutputChannels)
                      outchans = info->maxOutputChannels;

                    pa_outdev = j;
                    break;
                }
                devno++;
            }
        }
    }   

    if (inchans > 0 && pa_indev == -1)
        inchans = 0;
    if (outchans > 0 && pa_outdev == -1)
        outchans = 0;
    
    if (0)
    {
        post("input device %d, channels %d", pa_indev, inchans);
        post("output device %d, channels %d", pa_outdev, outchans);
        post("framesperbuf %d, nbufs %d", framesperbuf, nbuffers);
        post("rate %d", rate);
    }
    pa_channelsIn = audio_channelsIn = inchans;
    pa_channelsOut = audio_channelsOut = outchans;
    pa_soundIn = soundin;
    pa_soundOut = soundout;

#ifdef PA_WITH_FAKEBLOCKING
    if (pa_bufferIn)
        free((char *)pa_bufferIn), pa_bufferIn = 0;
    if (pa_bufferOut)
        free((char *)pa_bufferOut), pa_bufferOut = 0;
#endif

    if (! inchans && !outchans)
        return (0);
    
    if (callbackfn)
    {
        pa_callback = callbackfn;
        err = pa_open_callback(rate, inchans, outchans,
            framesperbuf, nbuffers, pa_indev, pa_outdev, pa_lowlevel_callback);
    }
    else
    {
#ifdef PA_WITH_FAKEBLOCKING
        if (pa_channelsIn)
        {
            pa_bufferIn = malloc(nbuffers*framesperbuf*pa_channelsIn*sizeof(float));
            ringbuffer_initialize(&pa_ringIn,
                nbuffers*framesperbuf*pa_channelsIn*sizeof(float), pa_bufferIn,
                    nbuffers*framesperbuf*pa_channelsIn*sizeof(float));
        }
        if (pa_channelsOut)
        {
            pa_bufferOut = malloc(nbuffers*framesperbuf*pa_channelsOut*sizeof(float));
            ringbuffer_initialize(&pa_ringOut,
                nbuffers*framesperbuf*pa_channelsOut*sizeof(float), pa_bufferOut, 0);
        }
        err = pa_open_callback(rate, inchans, outchans,
            framesperbuf, nbuffers, pa_indev, pa_outdev, pa_fifo_callback);
#else
        err = pa_open_callback(rate, inchans, outchans,
            framesperbuf, nbuffers, pa_indev, pa_outdev, 0);
#endif
    }
    pa_started = 0;
    pa_numberOfBuffers = nbuffers;
    if ( err != paNoError ) 
    {
        post("Error opening audio: %s", Pa_GetErrorText(err));
        /* Pa_Terminate(); */
        return (1);
    }
    else if (0)
        post("... opened OK.");
    return (0);
}

void pa_close( void)
{
    if (pa_stream)
    {
        Pa_AbortStream(pa_stream);
        Pa_CloseStream(pa_stream);
    }
    pa_stream = 0;
#ifdef PA_WITH_FAKEBLOCKING
    if (pa_bufferIn)
        free((char *)pa_bufferIn), pa_bufferIn = 0;
    if (pa_bufferOut)
        free((char *)pa_bufferOut), pa_bufferOut = 0;
#endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int pa_pollDSP(void)
{
    t_sample *fp;
    float *fp2, *fp3;
    float *conversionbuf;
    int j, k;
    int rtnval =  DACS_YES;
#ifndef PA_WITH_FAKEBLOCKING
    double timebefore;
#endif /* PA_WITH_FAKEBLOCKING */
    if (!audio_channelsIn && !audio_channelsOut || !pa_stream)
        return (DACS_NO); 
    conversionbuf = (float *)alloca((audio_channelsIn > audio_channelsOut?
        audio_channelsIn:audio_channelsOut) * AUDIO_DEFAULT_BLOCKSIZE * sizeof(float));

#ifdef PA_WITH_FAKEBLOCKING
    if (!audio_channelsIn)    /* if no input channels sync on output */
    {
#ifdef PA_WITH_THREADSIGNAL
        pthread_mutex_lock(&pa_mutex);
#endif
        while (ringbuffer_getWriteAvailable(&pa_ringOut) <
            (long)(audio_channelsOut * AUDIO_DEFAULT_BLOCKSIZE * sizeof(float)))
        {
            rtnval = DACS_SLEPT;
#ifdef PA_WITH_THREADSIGNAL
            pthread_cond_wait(&pa_cond, &pa_mutex);
#else
#ifdef _WIN32
            Sleep(1);
#else
            usleep(1000);
#endif /* _WIN32 */
#endif /* PA_WITH_THREADSIGNAL */
        }
#ifdef PA_WITH_THREADSIGNAL
        pthread_mutex_unlock(&pa_mutex);
#endif
    }
        /* write output */
    if (audio_channelsOut)
    {
        for (j = 0, fp = audio_soundOut, fp2 = conversionbuf;
            j < audio_channelsOut; j++, fp2++)
                for (k = 0, fp3 = fp2; k < AUDIO_DEFAULT_BLOCKSIZE;
                    k++, fp++, fp3 += audio_channelsOut)
                        *fp3 = *fp;
        ringbuffer_write(&pa_ringOut, conversionbuf,
            audio_channelsOut*(AUDIO_DEFAULT_BLOCKSIZE*sizeof(float)), pa_bufferOut);
    }
    if (audio_channelsIn)    /* if there is input sync on it */
    {
#ifdef PA_WITH_THREADSIGNAL
        pthread_mutex_lock(&pa_mutex);
#endif
        while (ringbuffer_getReadAvailable(&pa_ringIn) <
            (long)(audio_channelsIn * AUDIO_DEFAULT_BLOCKSIZE * sizeof(float)))
        {
            rtnval = DACS_SLEPT;
#ifdef PA_WITH_THREADSIGNAL
            pthread_cond_wait(&pa_cond, &pa_mutex);
#else
#ifdef _WIN32
            Sleep(1);
#else
            usleep(1000);
#endif /* _WIN32 */
#endif /* PA_WITH_THREADSIGNAL */
        }
#ifdef PA_WITH_THREADSIGNAL
        pthread_mutex_unlock(&pa_mutex);
#endif
    }
    if (audio_channelsIn)
    {
        ringbuffer_read(&pa_ringIn, conversionbuf,
            audio_channelsIn*(AUDIO_DEFAULT_BLOCKSIZE*sizeof(float)), pa_bufferIn);
        for (j = 0, fp = audio_soundIn, fp2 = conversionbuf;
            j < audio_channelsIn; j++, fp2++)
                for (k = 0, fp3 = fp2; k < AUDIO_DEFAULT_BLOCKSIZE;
                    k++, fp++, fp3 += audio_channelsIn)
                        *fp = *fp3;
    }

#else /* PA_WITH_FAKEBLOCKING */
    timebefore = sys_getRealTimeInSeconds();
        /* write output */
    if (audio_channelsOut)
    {
        if (!pa_started)
        {
            memset(conversionbuf, 0,
                audio_channelsOut * AUDIO_DEFAULT_BLOCKSIZE * sizeof(float));
            for (j = 0; j < pa_numberOfBuffers-1; j++)
                Pa_WriteStream(pa_stream, conversionbuf, AUDIO_DEFAULT_BLOCKSIZE);
        }
        for (j = 0, fp = audio_soundOut, fp2 = conversionbuf;
            j < audio_channelsOut; j++, fp2++)
                for (k = 0, fp3 = fp2; k < AUDIO_DEFAULT_BLOCKSIZE;
                    k++, fp++, fp3 += audio_channelsOut)
                        *fp3 = *fp;
        Pa_WriteStream(pa_stream, conversionbuf, AUDIO_DEFAULT_BLOCKSIZE);
    }

    if (audio_channelsIn)
    {
        Pa_ReadStream(pa_stream, conversionbuf, AUDIO_DEFAULT_BLOCKSIZE);
        for (j = 0, fp = audio_soundIn, fp2 = conversionbuf;
            j < audio_channelsIn; j++, fp2++)
                for (k = 0, fp3 = fp2; k < AUDIO_DEFAULT_BLOCKSIZE;
                    k++, fp++, fp3 += audio_channelsIn)
                        *fp = *fp3;
    }
    if (sys_getRealTimeInSeconds() - timebefore > 0.002)
    {
        rtnval = DACS_SLEPT;
    }
#endif /* PA_WITH_FAKEBLOCKING */
    pa_started = 1;

    memset(audio_soundOut, 0, AUDIO_DEFAULT_BLOCKSIZE*sizeof(t_sample)*audio_channelsOut);
    return (rtnval);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

    /* scanning for devices */
void pa_getLists(char *indevlist, int *nindevs,
    char *outdevlist, int *noutdevs, int *canmulti, int *canCallback)
{
    int maxndev = MAXIMUM_DEVICES;
    int devdescsize = MAXIMUM_DESCRIPTION;
    int i, nin = 0, nout = 0, ndev;
    
    *canmulti = 1;  /* one dev each for input and output */
    *canCallback = 1;
    
    pa_initialize();
    ndev = Pa_GetDeviceCount();
    for (i = 0; i < ndev; i++)
    {
        const PaDeviceInfo *pdi = Pa_GetDeviceInfo(i);
        if (pdi->maxInputChannels > 0 && nin < maxndev)
        {
                /* LATER figure out how to get API name correctly */
            snprintf(indevlist + nin * devdescsize, devdescsize,
#ifdef _WIN32
     "%s:%s", (pdi->hostApi == 0 ? "MMIO" : (pdi->hostApi == 1 ? "ASIO" : "?")),
#else
#ifdef __APPLE__
             "%s",
#else
            "(%d) %s", pdi->hostApi,
#endif
#endif
                pdi->name);
            nin++;
        }
        if (pdi->maxOutputChannels > 0 && nout < maxndev)
        {
            snprintf(outdevlist + nout * devdescsize, devdescsize,
#ifdef _WIN32
     "%s:%s", (pdi->hostApi == 0 ? "MMIO" : (pdi->hostApi == 1 ? "ASIO" : "?")),
#else
#ifdef __APPLE__
             "%s",
#else
            "(%d) %s", pdi->hostApi,
#endif
#endif
                pdi->name);
            nout++;
        }
    }
    *nindevs = nin;
    *noutdevs = nout;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

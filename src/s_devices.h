
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __s_devices_h_
#define __s_devices_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define DEVICES_MAXIMUM_IO              8
#define DEVICES_MAXIMUM_CHANNELS        32
#define DEVICES_MAXIMUM_BLOCKSIZE       2048

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _devicesproperties {
    int d_blockSize;
    int d_sampleRate;
    int d_inSize;
    int d_outSize;
    int d_in          [DEVICES_MAXIMUM_IO];     // --
    int d_out         [DEVICES_MAXIMUM_IO];     // --
    int d_inChannels  [DEVICES_MAXIMUM_IO];     // --
    int d_outChannels [DEVICES_MAXIMUM_IO];     // --
    int d_isMidi;
    } t_devicesproperties;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    devices_initAsAudio             (t_devicesproperties *p);
void    devices_initAsMidi              (t_devicesproperties *p);
void    devices_setDefaults             (t_devicesproperties *p);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    devices_setBlockSize            (t_devicesproperties *p, int n);
void    devices_setSampleRate           (t_devicesproperties *p, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int     devices_getBlockSize            (t_devicesproperties *p);
int     devices_getSampleRate           (t_devicesproperties *p);
int     devices_getInSize               (t_devicesproperties *p);
int     devices_getOutSize              (t_devicesproperties *p);
int     devices_getInAtIndex            (t_devicesproperties *p, int i);
int     devices_getOutAtIndex           (t_devicesproperties *p, int i);
int     devices_getInChannelsAtIndex    (t_devicesproperties *p, int i);
int     devices_getOutChannelsAtIndex   (t_devicesproperties *p, int i);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    devices_checkDisabled           (t_devicesproperties *p);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error devices_appendMidiIn            (t_devicesproperties *p, char *device);
t_error devices_appendMidiOut           (t_devicesproperties *p, char *device);
t_error devices_appendAudioIn           (t_devicesproperties *p, char *device, int channels);
t_error devices_appendAudioOut          (t_devicesproperties *p, char *device, int channels);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error devices_appendMidiInAsNumber    (t_devicesproperties *p, int n);
t_error devices_appendMidiOutAsNumber   (t_devicesproperties *p, int n);
t_error devices_appendAudioInAsNumber   (t_devicesproperties *p, int n, int channels);
t_error devices_appendAudioOutAsNumber  (t_devicesproperties *p, int n, int channels);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error devices_getInAtIndexAsString    (t_devicesproperties *p, int i, char *dest, size_t size);
t_error devices_getOutAtIndexAsString   (t_devicesproperties *p, int i, char *dest, size_t size);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_devices_h_

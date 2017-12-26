
/* 
    Copyright (c) 1997-2017 Guenter Geiger, Miller Puckette,
    Larry Troxler, Winfried Ritsch, Karl MacMillan, and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "s_midi.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <alsa/asoundlib.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define MIDIALSA_MAXIMUM_EVENTS     512

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int midialsa_numberOfDevicesIn;                  /* Static. */
static int midialsa_numberOfDevicesOut;                 /* Static. */

static int midialsa_devicesIn[DEVICES_MAXIMUM_IO];      /* Static. */
static int midialsa_devicesOut[DEVICES_MAXIMUM_IO];     /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static snd_seq_t            *midialsa_handle;           /* Static. */
static snd_midi_event_t     *midialsa_event;            /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

const char *midi_nameNative (void)
{
    static const char *name = "ALSA"; return name;      /* Static. */
}

void midi_initializeNative (void)
{
}

void midi_releaseNative (void)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void midi_openNative (t_devicesproperties *p)
{
    int numberOfDevicesIn   = devices_getInSize (p);
    int numberOfDevicesOut  = devices_getOutSize (p);
    
    midialsa_numberOfDevicesIn  = 0;
    midialsa_numberOfDevicesOut = 0;

    if (numberOfDevicesOut || numberOfDevicesIn) {
    //
    t_error err = PD_ERROR;
    
    if (numberOfDevicesIn > 0 && numberOfDevicesOut > 0) { 
        err = snd_seq_open (&midialsa_handle, "default", SND_SEQ_OPEN_DUPLEX, 0);
        
    } else if (numberOfDevicesIn > 0) {
        err = snd_seq_open (&midialsa_handle, "default", SND_SEQ_OPEN_INPUT,  0);
        
    } else if (numberOfDevicesOut > 0) {
        err = snd_seq_open (&midialsa_handle, "default", SND_SEQ_OPEN_OUTPUT,  0);
    }
    
    if (err) { PD_BUG; }
    else {
    //
    int i;
    
    for (i = 0; i < numberOfDevicesIn; i++) {
        char portname[PD_STRING] = { 0 };
        err |= string_sprintf (portname, PD_STRING, PD_NAME " Midi-In %d", i + 1);
        if (!err) {
            int  port = snd_seq_create_simple_port (midialsa_handle,
                            portname,
                            SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE, 
                            SND_SEQ_PORT_TYPE_APPLICATION);
            if (!(err |= (port < 0))) { 
                midialsa_devicesIn[midialsa_numberOfDevicesIn] = port; midialsa_numberOfDevicesIn++;
            }
        }
    }

    for (i = 0; i < numberOfDevicesOut; i++) {
        char portname[PD_STRING] = { 0 };
        err |= string_sprintf (portname, PD_STRING, PD_NAME " Midi-Out %d", i + 1);
        if (!err) {
            int  port = snd_seq_create_simple_port (midialsa_handle,
                            portname,
                            SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, 
                            SND_SEQ_PORT_TYPE_APPLICATION);
            if (!(err |= (port < 0))) { 
                midialsa_devicesOut[midialsa_numberOfDevicesOut] = port; midialsa_numberOfDevicesOut++;
            }
        }
    }
   
    if (err) { PD_BUG; }
    else {
        snd_seq_client_info_t *info = NULL;
        snd_seq_client_info_malloc (&info);
        snd_seq_get_client_info (midialsa_handle, info);
        snd_seq_client_info_set_name (info, PD_NAME);
        snd_seq_client_info_get_client (info);
        snd_seq_set_client_info (midialsa_handle, info);
        snd_seq_client_info_free (info);
        snd_midi_event_new (MIDIALSA_MAXIMUM_EVENTS, &midialsa_event);
    }
    //
    }
    //
    }
}

void midi_closeNative()
{
    midialsa_numberOfDevicesIn  = 0;
    midialsa_numberOfDevicesOut = 0;
    
    if (midialsa_handle) {
        snd_seq_close (midialsa_handle);
    }
    
    if (midialsa_event) {
        snd_midi_event_free (midialsa_event);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void midi_pushNextMessageNative (int port, int a, int b, int c)
{
    snd_seq_event_t e;
    snd_seq_ev_clear (&e);
    
    if (port >= 0 && port < midialsa_numberOfDevicesOut) {
    //
    PD_ASSERT (a < MIDI_STARTSYSEX);
    PD_ASSERT (a >= MIDI_NOTEON);
    
    if (a >= MIDI_PITCHBEND) { snd_seq_ev_set_pitchbend (&e, a - MIDI_PITCHBEND, (((c << 7) | b) - 8192)); } 
    else if (a >= MIDI_AFTERTOUCH)      { snd_seq_ev_set_chanpress (&e, a - MIDI_AFTERTOUCH, b);        }
    else if (a >= MIDI_PROGRAMCHANGE)   { snd_seq_ev_set_pgmchange (&e, a - MIDI_PROGRAMCHANGE, b);     }
    else if (a >= MIDI_CONTROLCHANGE)   { snd_seq_ev_set_controller (&e, a - MIDI_CONTROLCHANGE, b, c); }
    else if (a >= MIDI_POLYPRESSURE)    { snd_seq_ev_set_keypress (&e, a - MIDI_POLYPRESSURE, b, c);    }
    else if (a >= MIDI_NOTEON)          {
        if (c) { snd_seq_ev_set_noteon (&e, a - MIDI_NOTEON, b, c); }
        else {
            snd_seq_ev_set_noteoff (&e, a - MIDI_NOTEON, b, c);
        }
    }
    
    snd_seq_ev_set_direct (&e);
    snd_seq_ev_set_subs (&e);
    snd_seq_ev_set_source (&e, midialsa_devicesOut[port]);
    snd_seq_event_output_direct (midialsa_handle, &e);
    //
    }
}

void midi_pushNextByteNative (int port, int a)
{
    snd_seq_event_t e;
    snd_seq_ev_clear (&e);
    
    if (port >= 0 && port < midialsa_numberOfDevicesOut) {
    //
    unsigned char data = (unsigned char)a;
    snd_seq_ev_set_sysex (&e, 1, &data);
    snd_seq_ev_set_direct (&e);
    snd_seq_ev_set_subs (&e);
    snd_seq_ev_set_source (&e, midialsa_devicesOut[port]);
    snd_seq_event_output_direct (midialsa_handle, &e);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void midi_pollNative (void)
{
    if (midialsa_numberOfDevicesOut || midialsa_numberOfDevicesIn) {
    //
    unsigned char t[MIDIALSA_MAXIMUM_EVENTS] = { 0 };
    int pending;

    snd_seq_event_t *midievent = NULL;
   
    snd_midi_event_init (midialsa_event);
    pending = snd_seq_event_input_pending (midialsa_handle, 1);
    
    if (pending != 0) { snd_seq_event_input (midialsa_handle, &midievent); }
    
    if (midievent != NULL) {
    //
    int i;
    int n = (int)snd_midi_event_decode (midialsa_event, t, MIDIALSA_MAXIMUM_EVENTS, midievent);
    for (i = 0; i < n; i++) {
        midi_receive (midievent->dest.port, (int)(t[i] & 0xff));
    }
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error midi_getListsNative (t_deviceslist *p)
{
    int i;
    int m = PD_MIN (4, DEVICES_MAXIMUM_IO);
    int n = PD_MIN (4, DEVICES_MAXIMUM_IO);
    
    t_error err = PD_ERROR_NONE;
    
    for (i = 0; i < m; i++) { err |= deviceslist_appendMidiIn (p, "ALSA virtual device");  }
    for (i = 0; i < n; i++) { err |= deviceslist_appendMidiOut (p, "ALSA virtual device"); }
  
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

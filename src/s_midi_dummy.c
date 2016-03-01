
/* 
    Copyright (c) 2010 Peter Brinkmann.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midi_openNative (int numberOfDevicesIn, int *devicesIn, int numberOfDevicesOut, int *devicesOut)
{
}

void midi_closeNative (void)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midi_pushNextMessageNative (int port, int a, int b, int c)
{
}

void midi_pushNextByteNative (int port, int byte)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midi_pollNative (void)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error midi_getListsNative (char *devicesIn, 
    int *numberOfDevicesIn, 
    char *devicesOut, 
    int *numberOfDevicesOut)
{
    t_error err = PD_ERROR_NONE;
    
    err |= string_copy (devicesIn,  MAXIMUM_DEVICES * MAXIMUM_DESCRIPTION, "NONE");
    err |= string_copy (devicesOut, MAXIMUM_DEVICES * MAXIMUM_DESCRIPTION, "NONE");
    
    *numberOfDevicesIn  = 1;
    *numberOfDevicesOut = 1;
  
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------


/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define SNAP_DEFAULT_GRID       12

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int snap_hasGrid;        /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void snap_setSnapToGrid (int n)
{
    snap_hasGrid = (n != 0);
    
    gui_vAdd ("set ::var(isSnapToGrid) %d\n", snap_hasGrid);    // --
}

int snap_hasSnapToGrid (void)
{
    return snap_hasGrid;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int snap_getStep (void)
{
    return SNAP_DEFAULT_GRID;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int snap_getOffset (int n)
{
    return (snap_getSnapped (n) - n);
}

int snap_getSnapped (int n)
{
    int k = snap_getStep();
    
    if (n > 0) { return ((int)((n / (double)k) + 0.5) * k); }
    else {
        return ((int)((n / (double)k) - 0.5) * k);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

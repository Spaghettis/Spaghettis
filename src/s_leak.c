
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *sys_getMemoryChecked (size_t n, const char *f, const int line)
{
    return sys_getMemory (n);
}

void *sys_getMemoryResizeChecked (void *ptr, size_t oldSize, size_t newSize, const char *f, const int line)
{
    return sys_getMemoryResize (ptr, oldSize, newSize);
}

void sys_freeMemoryChecked (void *ptr, const char *f, const int line)
{
    return sys_freeMemory (ptr);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

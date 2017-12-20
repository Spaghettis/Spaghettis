
/* 
    Copyright 2007-2013 William Andrew Burnson. All rights reserved.

    File modified by Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef BELLE_LIBRARY
#define BELLE_LIBRARY

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef PRIM_WITH_TEST
#define PRIM_WITH_TEST     0
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://github.com/burnson/prim.cc > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef PRIM_LIBRARY
    #ifdef BELLE_COMPILE_INLINE
        #define PRIM_COMPILE_INLINE
    #endif
    #include "Core/Prim/Prim.hpp"
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://github.com/burnson/MICA > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#ifndef MICA_LIBRARY
    #ifdef BELLE_COMPILE_INLINE
        #include "Core/MICA/Mica.cpp"
    #endif
    #include "Core/MICA/Mica.hpp"
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://github.com/burnson/Belle > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "Core/Core.hpp"
#include "Fonts/Fonts.hpp"
#include "Symbols/Symbols.hpp"
#include "Modern/Modern.hpp"
#include "Painters/Painters.hpp"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // BELLE_LIBRARY
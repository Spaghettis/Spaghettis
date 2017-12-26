
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_pathlist *searchpath_list;     /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void searchpath_appendPath (char *filepath)
{
    searchpath_list = pathlist_newAppend (searchpath_list, filepath);
}

void searchpath_setEncoded (int argc, t_atom *argv)
{
    int i;
    
    pathlist_free (searchpath_list);
    
    searchpath_list = NULL;
    
    for (i = 0; i < argc; i++) {
    //
    t_symbol *path = symbol_decode (atom_getSymbolAtIndex (i, argc, argv));
        
    searchpath_list = pathlist_newAppend (searchpath_list, path->s_name);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_pathlist *searchpath_get (void)
{
    return searchpath_list;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void searchpath_release (void)
{
    pathlist_free (searchpath_list);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

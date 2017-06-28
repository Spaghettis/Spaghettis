
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error buffer_fromFile (t_buffer *x, char *name, char *directory)
{
    t_error err = PD_ERROR;
    
    char filepath[PD_STRING] = { 0 };

    if (!(err = path_withDirectoryAndName (filepath, PD_STRING, directory, name))) {
    //
    int f = file_openRead (filepath);
    
    err = (f < 0);
    
    if (err) { PD_BUG; }
    else {
    //
    off_t length;
    
    err |= ((length = lseek (f, 0, SEEK_END)) < 0);
    err |= (lseek (f, 0, SEEK_SET) < 0); 
    
    if (err) { PD_BUG; }
    else {
        char *t = (char *)PD_MEMORY_GET ((size_t)length);
        err = (read (f, t, (size_t)length) != length);
        if (err) { PD_BUG; } else { buffer_withStringUnzeroed (x, t, (int)length); }
        PD_MEMORY_FREE (t);
    }
    
    close (f);
    //
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error buffer_fileRead (t_buffer *x, t_symbol *name, t_glist *glist)
{
    t_error err = PD_ERROR;
    
    t_fileproperties p;
    
    if (glist_fileExist (glist, name->s_name, "", &p)) {
        err = buffer_fromFile (x, fileproperties_getName (&p), fileproperties_getDirectory (&p));
    }
    
    if (err) { error_canNotOpen (name); }
    
    return err;
}

t_error buffer_fileWrite (t_buffer *x, t_symbol *name, t_symbol *directory)
{
    t_error err = PD_ERROR;

    char filepath[PD_STRING] = { 0 };

    if (!(err = path_withDirectoryAndName (filepath, PD_STRING, directory->s_name, name->s_name))) {
    //
    FILE *f = 0;

    err = !(f = file_fopenWrite (filepath));
    
    if (!err) {
    //
    char *s = NULL;
    int size = 0;
    
    buffer_toStringUnzeroed (x, &s, &size);

    err |= (fwrite (s, size, 1, f) < 1);
    err |= (fflush (f) != 0);

    PD_ASSERT (!err);
    PD_MEMORY_FREE (s);
        
    fclose (f);
    //
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
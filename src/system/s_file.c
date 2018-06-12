
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_WINDOWS

static int file_openRawNative (const char *filepath, int oflag)
{
    char t[PD_STRING]           = { 0 };
    wchar_t ucs2path[PD_STRING] = { 0 };
    
    if (string_copy (t, PD_STRING, filepath)) { PD_BUG; }
    path_slashToBackslashIfNecessary (t);
    u8_utf8toucs2 (ucs2path, PD_STRING, t, -1);

    if (oflag & O_CREAT) { return _wopen (ucs2path, oflag | O_BINARY, _S_IREAD | _S_IWRITE); }
    else {
        return _wopen (ucs2path, oflag | O_BINARY);
    }
}

#else

static int file_openRawNative (const char *filepath, int oflag)
{
    if (oflag & O_CREAT) { return open (filepath, oflag, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); }
    else {
        return open (filepath, oflag);
    } 
}

#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int file_openWrite (const char *filepath)
{
    return file_openRawNative (filepath, O_CREAT | O_TRUNC | O_WRONLY);
}

int file_openRead (const char *filepath)
{
    if (!(path_isFileExist (filepath))) { return -1; }
    else if (!(path_isFileExistAsRegularFile (filepath))) { return -1; }
    else {
        return file_openRawNative (filepath, O_RDONLY);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int file_openReadWithDirectoryAndName (const char *directory,
    const char *name, 
    const char *extension,
    t_fileproperties *p)
{
    int f = -1;
    t_error err = PD_ERROR_NONE;
    
    PD_ASSERT (directory);
    PD_ASSERT (name);
    PD_ASSERT (extension);
    
    p->f_directory[0] = 0; p->f_name = p->f_directory;
    
    err |= path_withDirectoryAndName (p->f_directory, PD_STRING, directory, name);
    err |= string_add (p->f_directory, PD_STRING, extension);

    if (!err && (f = file_openRead (p->f_directory)) >= 0) {
    //
    char *slash = NULL;

    if ((slash = strrchr (p->f_directory, '/'))) { *slash = 0; p->f_name = slash + 1; }
    
    return f;  
    //
    }
    
    p->f_directory[0] = 0; p->f_name = p->f_directory;
    
    return -1;
}

int file_openReadConsideringSearchPath (const char *directory, 
    const char *name, 
    const char *extension,
    t_fileproperties *p)
{
    /* Search first (and always) in directory provided (sibling files). */
    
    int f = file_openReadWithDirectoryAndName (directory, name, extension, p);
    int n = 0;
    
    /* For efficiency, test availability before to explore. */
    
    if (f < 0) {
        if (p->f_flag == FILEPROPERTIES_EXTERNAL)    {
            if (!searchpath_isExternalAvailable (p->f_sym))    { return -1; }
        }
        if (p->f_flag == FILEPROPERTIES_ABSTRACTION) {
            if (!searchpath_isAbstractionAvailable (p->f_sym)) { return -1; }
        }
    }
    
    /* At last look for in trees. */
    
    if (f < 0) {
        t_pathlist *l = searchpath_getExtended();
        while (l) {
            const char *path = pathlist_getPath (l);
            l = pathlist_getNext (l);
            f = file_openReadWithDirectoryAndName (path, name, extension, p);
            if (f >= 0) { searchpath_extendedMatchedAtIndex (n); break; }
            n++;
        }
    }
    
    return f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------


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

t_symbol    *main_directoryRoot;                        /* Static. */
t_symbol    *main_directoryBin;                         /* Static. */
t_symbol    *main_directoryTcl;                         /* Static. */
t_symbol    *main_directoryHelp;                        /* Static. */
t_symbol    *main_directorySupport;                     /* Static. */

int         main_portNumber;                            /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int  main_version;                               /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_error     audio_initialize    (void);
void        audio_release       (void);
void        message_initialize  (void);
void        message_release     (void);
void        setup_initialize    (void);
void        setup_release       (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void main_entryNative (void)
{
    #if PD_WINDOWS
    
    #if PD_MSVC
        _set_fmode (_O_BINARY);
    #else
        { extern int _fmode; _fmode = _O_BINARY; }
    #endif
    
    SetConsoleOutputCP (CP_UTF8);
    
    #endif
    
    sys_setSignalHandlers();
}

static t_error main_entryVersion (int console)
{
    char t[PD_STRING] = { 0 };
    t_error err = utils_version (t, PD_STRING);
    
    if (!err) {
        if (!console) { fprintf (stdout, "%s\n", t); }
        else {
            post ("%s", t);
        }
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error main_parseArguments (int argc, char **argv)
{
    t_error err = PD_ERROR_NONE;
    
    while (!err && (argc > 0) && (**argv == '-')) {
    //
    if (!strcmp (*argv, "--version")) { 
        main_version = 1; argc--; argv++; 

    } else if (!strcmp (*argv, "-port") && (argc > 1)) {
        if (sscanf (argv[1], "%d", &main_portNumber) >= 1) { argc -= 2; argv += 2; }
        else {
            err = PD_ERROR;
        }
        
    } else {
        err = PD_ERROR;
    }
    //
    }

    if (err) {
        fprintf (stderr, "Usage: pd [ --version ] [ -port port ]\n");   // --
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* 
    In "simple" installations, the layout is
    
        .../bin/spaghettis
        .../tcl/ui_main.tcl
        .../help/
        
    In "complexe" installations, the layout is
    
        .../bin/spaghettis
        .../lib/spaghettis/tcl/ui_main.tcl
        .../lib/spaghettis/help/

*/

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* < https://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe > */
/* < https://stackoverflow.com/questions/933850/how-to-find-the-location-of-the-executable-in-c > */

#if PD_WINDOWS

static t_error main_getExecutablePathNative (char *dest, size_t length)
{
    GetModuleFileName (NULL, dest, length); dest[length - 1] = 0;
        
    return PD_ERROR_NONE;
}

#elif PD_APPLE

static t_error main_getExecutablePathNative (char *dest, size_t length)
{
    t_error err = PD_ERROR_NONE;

    char path[PATH_MAX];
    uint32_t size = sizeof (path);

    err = (_NSGetExecutablePath (path, &size) != 0);
    
    if (!err) { 
        char *s = NULL;
        if ((s = realpath (path, NULL))) { err |= string_copy (dest, length, s); free (s); }
    }

    return err;
}

#elif PD_LINUX

static t_error main_getExecutablePathNative (char *dest, size_t length)
{
    t_error err = PD_ERROR_NONE;
    
    char path[PATH_MAX];
    
    ssize_t t = readlink ("/proc/self/exe", path, PATH_MAX);
    
    if (!(err = (t < 0 || t >= PATH_MAX))) {
        char *s = NULL;
        path[t] = 0;
        if ((s = realpath (path, NULL))) { err |= string_copy (dest, length, s); free (s); }
    }
    
    return err;
}

#else
    #error
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_error main_getRootDirectory (void)
{
    t_error err = PD_ERROR_NONE;
    char t1[PD_STRING] = { 0 };
    char t2[PD_STRING] = { 0 };
    char *slash = NULL; 
    
    #if PD_WINDOWS
        err |= main_getExecutablePathNative (t1, PD_STRING);
        path_backslashToSlashIfNecessary (t1);
    #else
        err |= main_getExecutablePathNative (t1, PD_STRING);
    #endif
    
    /* Name of the executable's parent directory. */
    
    if (!err) { 
        if (!(err |= !(slash = strrchr (t1, '/')))) { *slash = 0; }
        if (!(err |= !(slash = strrchr (t1, '/')))) { *slash = 0; }
    }

    if (!err) {
    //
    #if PD_WINDOWS
        main_directoryRoot = gensym (t1);
    #else
        err = string_copy (t2, PD_STRING, t1);
        err |= string_add (t2, PD_STRING, "/lib/" PD_NAME_LOWERCASE);
        
        if (!err && path_isFileExist (t2)) { main_directoryRoot = gensym (t2); }    /* Complexe. */
        else {
            main_directoryRoot = gensym (t1);   /* Simple. */
        }
    #endif
    //
    }
    
    return err;
}

t_error main_setPaths (t_symbol *root)
{
    if (root == NULL) { PD_BUG; return PD_ERROR; }
    else {
    //
    t_error err = PD_ERROR_NONE;
    
    char t[PD_STRING] = { 0 };
    
    const char *s = root->s_name;
    const char *home = getenv ("HOME");
    
    err |= (home == NULL);
    
    if (!err) {
    //
    #if PD_APPLE
    
    err |= string_sprintf (t, PD_STRING, "%s/Library/Application Support/" PD_NAME, home);
    
    if (!err && !path_isFileExistAsDirectory (t)) {
        err |= path_createDirectory (t);
    }
    
    #else
    
    err |= string_sprintf (t, PD_STRING, "%s", home);
    
    #endif
    
    if (!err) {
        main_directorySupport = gensym (t);
    }
    
    if (!(err |= string_sprintf (t, PD_STRING, "%s/bin",  s))) { main_directoryBin  = gensym (t); }
    if (!(err |= string_sprintf (t, PD_STRING, "%s/tcl",  s))) { main_directoryTcl  = gensym (t); }
    if (!(err |= string_sprintf (t, PD_STRING, "%s/help", s))) { main_directoryHelp = gensym (t); }
    //
    }
    
    return err;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Note that order of calls below may be critical. */

int main_entry (int argc, char **argv)
{
    t_error err = priority_privilegeStart();
    
    if (!err && !(err = priority_privilegeDrop())) {
    //
    main_entryNative();
    
    #if PD_WITH_DEBUG
        leak_initialize();
    #endif
    
    message_initialize();   /* Preallocate symbols and binding mechanism first. */
    
    err |= main_getRootDirectory();
    err |= main_parseArguments (argc - 1, argv + 1);
    err |= main_setPaths (main_directoryRoot);
    
    PD_ASSERT (main_directoryRoot    != NULL);
    PD_ASSERT (main_directoryBin     != NULL);
    PD_ASSERT (main_directoryTcl     != NULL);
    PD_ASSERT (main_directoryHelp    != NULL);
    PD_ASSERT (main_directorySupport != NULL);
    
    if (main_version) { err |= main_entryVersion (0); }
    else {
    //
    err |= logger_initialize();

    if (!err) {
    //
    err |= audio_initialize();
    
    if (!err) {
    //
    midi_initialize();
    setup_initialize();     /* Instance initialized here. */
    preferences_load();
    
    if (!(err |= interface_start())) {
        if (!(err |= main_entryVersion (1))) { err |= scheduler_main(); }
    }
    
    setup_release();
    midi_release();
    audio_release(); 
    //
    }
    
    logger_release();
    //
    }
    //
    }
    
    message_release();
    
    #if PD_WITH_DEBUG
        leak_release(); post_log ("Shutdown");
    #endif
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
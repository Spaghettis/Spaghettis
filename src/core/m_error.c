
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

// ====================================

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void error__error1 (const char *s)
{
    error__error2 ("", s);
}

void error__error2 (const char *s1, const char *s2)
{
    post_error ("%s: %s", s1, s2);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

const char *error__empty (t_symbol *s)
{
    if (s == &s_) { return "\" \""; }
    
    return s->s_name;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int error__options (t_symbol *s, int argc, t_atom *argv)
{
    int i, k = 0;
    
    for (i = 0; i < argc; i++) {
    //
    if (IS_SYMBOL (argv + i))  {
    //
    t_symbol *t = GET_SYMBOL (argv + i);
    
    if (t != sym___dash__ && string_startWith (t->s_name, sym___dash__->s_name)) {
        warning_unusedOption (s, t);
        k = 1;
    }
    //
    }
    //
    }
    
    return k;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void error_dspLoop (void)
{
    post_error (PD_TRANSLATE ("%s: DSP loop"), PD_NAME_LOWERCASE);
}

void error_stackOverflow (void)
{
    post_error (PD_TRANSLATE ("%s: stack overflow"), PD_NAME_LOWERCASE);
}

void error_recursiveCall (void)
{
    post_warning (PD_TRANSLATE ("%s: recursive call"), PD_NAME_LOWERCASE);
}

void error_stubNotFound (void)
{
    post_error (PD_TRANSLATE ("loader: stub not found"));
}

void error_searchPathOverflow (void)
{
    post_error (PD_TRANSLATE ("scan: search path overflow"));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void error_recursiveInstantiation (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: recursive instantiation"), s->s_name);
}

void error_sendReceiveLoop (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: send/receive loop"), s->s_name);
}

void error_alreadyExists (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: already exists"), s->s_name);
}

void error_canNotOpen (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: can't open"), s->s_name);
}

void error_canNotCreate (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: can't create"), s->s_name);
}

void error_failsToRead (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: fails to read"), s->s_name);
}

void error_failsToWrite (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: fails to write"), s->s_name);
}

void error_fileIsProtected (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: file is protected"), s->s_name);
}

void error_ignored (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: ignored"), s->s_name);
}

void error_failed (t_symbol *s)
{   
    post_error (PD_TRANSLATE ("%s: failed"), s->s_name);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void error_noSuch (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: no such %s"), s1->s_name, error__empty (s2));
}

void error_canNotFind (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: can't find %s"), s1->s_name, error__empty (s2));
}

void error_unknownMethod (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: unknown method %s"), s1->s_name, error__empty (s2));
}

void error_missingField (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: missing field %s"), s1->s_name, error__empty (s2));
}

void error_unexpected (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: unexpected %s"), s1->s_name, error__empty (s2));
}

void error_invalid (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: invalid %s"), s1->s_name, error__empty (s2));
}

void error_mismatch (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: mismatch %s"), s1->s_name, error__empty (s2));
}

void error_unspecified (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: unspecified %s"), s1->s_name, error__empty (s2));
}

void error_undefined (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: undefined %s"), s1->s_name, error__empty (s2));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void error_failsToOpen (t_symbol *s, t_symbol *s1, t_symbol *s2)
{
    const char *i = error__empty (s1);
    const char *o = error__empty (s2);
    
    post_error (PD_TRANSLATE ("%s: fails to open / %s / %s"), s->s_name, i, o);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void error_canNotMake (int argc, t_atom *argv)
{
    char *t = atom_atomsToString (argc, argv);
    
    post_error (PD_TRANSLATE ("%s: can't make [ %s ]"), PD_NAME_LOWERCASE, t);
    
    PD_MEMORY_FREE (t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void error_invalidArguments (t_symbol *s, int argc, t_atom *argv)
{
    char *t = atom_atomsToString (argc, argv);
    
    post_error (PD_TRANSLATE ("%s: [ %s ] invalid argument(s)"), s->s_name, t);
    
    PD_MEMORY_FREE (t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void warning_invalid (t_symbol *s1, t_symbol *s2)
{
    post_warning (PD_TRANSLATE ("%s: invalid %s"), s1->s_name, error__empty (s2));
}

void warning_empty (t_symbol *s1, t_symbol *s2)
{
    post_warning (PD_TRANSLATE ("%s: empty %s"), s1->s_name, error__empty (s2));
}

void warning_badName (t_symbol *s1, t_symbol *s2)
{
    post_warning (PD_TRANSLATE ("%s: bad name %s"), s1->s_name, error__empty (s2));
}

void warning_badType (t_symbol *s1, t_symbol *s2)
{
    post_warning (PD_TRANSLATE ("%s: bad type %s"), s1->s_name, error__empty (s2));
}

void warning_unusedOption (t_symbol *s1, t_symbol *s2)
{
    post_warning (PD_TRANSLATE ("%s: unused option %s"), s1->s_name, error__empty (s2));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void warning_unusedArguments (t_symbol *s, int argc, t_atom *argv)
{
    char *t = atom_atomsToString (argc, argv);
    
    post_warning (PD_TRANSLATE ("%s: [ %s ] unused argument(s)"), s->s_name, t);
    
    PD_MEMORY_FREE (t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void warning_tooManyCharacters (t_symbol *s) 
{ 
    post_warning (PD_TRANSLATE ("%s: too many characters"), s->s_name);
}

void warning_multipleBinding (t_symbol *s)
{ 
    post_warning (PD_TRANSLATE ("%s: multiple binding"), s->s_name);
}

void warning_fileIsCorrupted (t_symbol *s)
{
    post_warning (PD_TRANSLATE ("%s: file is corrupted"), s->s_name);
}

void warning_deprecatedObject (t_symbol *s)
{
    post_warning (PD_TRANSLATE ("%s: deprecated object"), s->s_name);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void warning_containsDuplicates (void)
{
    post_error (PD_TRANSLATE ("scan: contains duplicates"));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

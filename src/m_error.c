
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
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

void error_error1 (char *s)
{
    error_error2 ("", s);
}

void error_error2 (char *s1, char *s2)
{
    post_error ("%s: %s", s1, s2);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_post (int argc, t_atom *argv)
{
    char *s = atom_atomsToString (argc, argv); post_error ("[ %s ]", s); PD_MEMORY_FREE (s);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_stackOverflow (void)
{
    post_error (PD_TRANSLATE (": stack overflow"));
}

void error_ioStuck (void)
{
    post_error (PD_TRANSLATE ("audio: I/O stuck"));
}

void error_stubNotFound (void)
{
    post_error (PD_TRANSLATE ("loader: stub not found"));
}

void error_tooManyCharacters (void) 
{ 
    post_error (PD_TRANSLATE ("console: too many characters"));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_recursiveInstantiation (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: recursive instantiation"), s->s_name);
}

void error_sendReceiveLoop (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: send/receive loop"), s->s_name);
}

void error_canNotSetMultipleFields (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: can't set multiple fields"), s->s_name);
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
#pragma mark -

void error_unexpected (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: unexpected %s"), s1->s_name, s2->s_name);
}

void error_invalid (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: invalid %s"), s1->s_name, s2->s_name);
}

void error_mismatch (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: mismatch %s"), s1->s_name, s2->s_name);
}

void error_unspecified (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: unspecified %s"), s1->s_name, s2->s_name);
}

void error_unknown (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: unknown %s"), s1->s_name, s2->s_name);
}

void error_noSuch (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: no such %s"), s1->s_name, s2->s_name);
}

void error_canNotFind (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: can't find %s"), s1->s_name, s2->s_name);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_canNotMake (int argc, t_atom *argv)
{
    char *t = atom_atomsToString (argc, argv);
    
    post_error (PD_TRANSLATE (": can't make [ %s ]"), t);
    
    PD_MEMORY_FREE (t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_invalidArguments (t_symbol *s, int argc, t_atom *argv)
{
    char *t = atom_atomsToString (argc, argv);
    
    post_error (PD_TRANSLATE ("%s: invalid arguments [ %s ]"), s->s_name, t);
    
    PD_MEMORY_FREE (t);
}

void error_invalidArgumentsForMethod (t_symbol *s1, t_symbol *s2, int argc, t_atom *argv)
{
    char *t = atom_atomsToString (argc, argv);
        
    post_error (PD_TRANSLATE ("%s: invalid arguments for method %s [ %s ]"), s1->s_name, s2->s_name, t);
    
    PD_MEMORY_FREE (t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

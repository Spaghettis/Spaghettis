
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"
#include "../g_graphics.h"
#include "../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* A patch is bind to a context (the symbol #X). */
/* Most of the lines of a file can be considered as messages to the current context. */
/* Nested patch are handled with a stack mechanism. */
/* Contexts are pushed and popped to go down and up in the tree. */
/* Note that abstractions cannot be recursively instantiated. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void instance_contextStore (void)
{
    instance_get()->pd_stack.s_contextCached = instance_contextGetCurrent();
}

static void instance_contextRestore (void)
{
    instance_contextSetCurrent (instance_get()->pd_stack.s_contextCached);
    
    instance_get()->pd_stack.s_contextCached = NULL;
}

static t_glist *instance_contextGetStored (void)
{
    return instance_get()->pd_stack.s_contextCached;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void instance_stackPush (t_glist *x)
{
    t_stackelement *e = instance_get()->pd_stack.s_stack + (instance_get()->pd_stack.s_stackIndex++);
    
    PD_ASSERT (instance_get()->pd_stack.s_stackIndex < INSTANCE_STACK);
    PD_ABORT (instance_get()->pd_stack.s_stackIndex >= INSTANCE_STACK);
    
    e->s_context = instance_contextGetCurrent();
    e->s_loadedAbstraction = instance_get()->pd_loadingAbstraction;
    
    instance_get()->pd_loadingAbstraction = NULL;
    
    instance_contextSetCurrent (x);
}

void instance_stackPop (t_glist *x)
{
    t_stackelement *e = instance_get()->pd_stack.s_stack + (--instance_get()->pd_stack.s_stackIndex);
    
    PD_ASSERT (instance_get()->pd_stack.s_stackIndex >= 0);
    PD_ASSERT (instance_contextGetCurrent() == x);
    
    instance_contextSetCurrent (e->s_context);
    
    instance_get()->pd_stack.s_contextPopped = x;
}

void instance_stackPopPatch (t_glist *glist, int visible)
{
    instance_stackPop (glist);
    
    glist_inletSort (glist); glist_outletSort (glist);
    
    glist_loadEnd (glist);
    glist_undoEnable (glist);
    
    if (visible) { glist_windowOpen (glist); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int instance_loadAbstractionIsValid (t_symbol *filename)
{
    int i;
    
    for (i = 0; i < instance_get()->pd_stack.s_stackIndex; i++) {
    //
    t_stackelement *e = instance_get()->pd_stack.s_stack + i;
    if (e->s_loadedAbstraction == filename) { return 0; }
    //
    }
    
    instance_get()->pd_loadingAbstraction = filename;
    
    return 1;
}

static void instance_loadAbstractionProceed (t_symbol *filename,
    t_symbol *directory,
    int argc,
    t_atom *argv,
    t_buffer *b)
{
    if (instance_loadAbstractionIsValid (filename)) {
    //
    instance_environmentSetArguments (argc, argv);
    
    if (b) { eval_fileByBuffer (filename, directory, b); } else { eval_file (filename, directory); }
    
    if (instance_contextGetCurrent() != instance_contextGetStored()) {
    
        instance_setNewestObject (cast_pd (instance_contextGetCurrent()));
        instance_stackPopPatch (instance_contextGetCurrent(), 0);
    }
    
    instance_environmentResetArguments();
    //
    } else {
        error_recursiveInstantiation (filename);
    }
}

static void instance_loadAbstractionFile (t_symbol *name, int argc, t_atom *argv)
{
    t_fileproperties p; fileproperties_initAbstraction (&p, name);
    
    if (glist_fileExist (instance_contextGetCurrent(), name->s_name, PD_PATCH, &p)) {
    //
    t_symbol *filename  = gensym (fileproperties_getName (&p));
    t_symbol *directory = gensym (fileproperties_getDirectory (&p));
    
    instance_loadAbstractionProceed (filename, directory, argc, argv, NULL);
    //
    }
}

static t_error instance_loadAbstractionSnippet (t_symbol *key, int argc, t_atom *argv)
{
    t_abstractions *t   = glist_getAbstractions (instance_contextGetCurrent());
    t_symbol *filename  = NULL;
    t_symbol *directory = NULL;
    t_buffer *snippet   = abstractions_getSnippet (t, key, &filename, &directory);
    
    if (snippet) {
    //
    instance_loadAbstractionProceed (filename, directory, argc, argv, snippet);
    
    return PD_ERROR_NONE;
    //
    }
    
    return PD_ERROR;
}

void instance_loadAbstraction (t_symbol *name, int argc, t_atom *argv)
{
    if (pool_check (name)) {    /* Abstraction cached (e.g. encapsulation)? */
    //
    if (instance_loadAbstractionSnippet (name, argc, argv) == PD_ERROR_NONE) { return; }
    //
    }
    
    instance_loadAbstractionFile (name, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void instance_loadPatchLoadbang (void)
{
    if (instance_get()->pd_stack.s_contextPopped) {
        glist_loadbang (instance_get()->pd_stack.s_contextPopped);
        instance_get()->pd_stack.s_contextPopped = NULL;
    }
}

static int instance_loadPatchProceed (t_symbol *name, t_symbol *directory, const char *s, int visible)
{
    int done = 0;
    
    instance_contextStore();
    instance_contextSetCurrent (NULL);          /* The root patch do NOT have parent. */
        
    if (s) { eval_fileByString (name, directory, s); }
    else   { eval_file (name, directory); }
    
    if (instance_contextGetCurrent() != NULL) { 
        instance_stackPopPatch (instance_contextGetCurrent(), visible); done = 1;
    }
    
    PD_ASSERT (instance_contextGetCurrent() == NULL);
    
    instance_loadPatchLoadbang();
    instance_contextRestore();
    
    return done;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int instance_loadPatch (t_symbol *name, t_symbol *directory)
{
    return instance_loadPatchProceed (name, directory, NULL, 1);
}

/* Load invisible patches used for built-in templates. */

void instance_loadBuiltIn (t_symbol *name, const char *s)
{
    instance_loadPatchProceed (name, sym___dot__, s, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Context of the stack temporary bypassed to eval the buffer. */

void instance_loadSnippet (t_glist *glist, t_buffer *b)
{
    instance_contextStore();
    instance_contextSetCurrent (glist);
    eval_buffer (b, NULL, 0, NULL);
    instance_contextRestore();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void instance_patchNew (t_symbol *name, t_symbol *directory)
{
    instance_environmentSetFile (name, directory);
    
    glist_newPatchPop (&s_, NULL, NULL, NULL, 1, 1, 0, 0);
    
    instance_environmentResetFile();
}

t_error instance_patchOpen (t_symbol *name, t_symbol *directory)
{
    int state = dsp_suspend();
    int done  = instance_loadPatch (name, directory);
    
    dsp_resume (state);
    
    return (done != 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

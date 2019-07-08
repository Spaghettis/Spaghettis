
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
// MARK: -

void canvas_key                 (t_glist *, t_symbol *, int, t_atom *);
void interface_quit             (void);
void font_withHostMeasured      (int, t_atom *);
void audio_requireDialog        (void);
void audio_fromDialog           (int, t_atom *);
void midi_requireDialog         (void);
void midi_fromDialog            (int, t_atom *);
void canvas_quit                (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *global_class;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void global_newPatch (void *dummy, t_symbol *name, t_symbol *directory)
{
    instance_patchNew (name, directory);
}

static void global_open (void *dummy, t_symbol *name, t_symbol *directory)
{
    if (!instance_patchOpen (name, directory, 1)) { recentfiles_add (name, directory, 0); }
}

static void global_scan (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    t_error err = searchpath_scan();
    
    if (argc && atom_getSymbol (argv) == sym_logged) { searchpath_report(); }
    
    if (searchpath_hasDuplicates()) { warning_containsDuplicates(); }
    if (err) { error_searchPathOverflow(); post ("scan: failed"); }  // --
    else {
        post ("scan: done");  // --
    }
}

static void global_clear (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    if (argc && atom_getSymbol (argv) == sym_recent) { recentfiles_clear(); recentfiles_update(); }
    else {
        gui_add ("::ui_console::clear\n");
    }
}

static void global_dsp (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) { dsp_setState ((int)atom_getFloatAtIndex (0, argc, argv)); }
}

static void global_key (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    canvas_key (NULL, s, argc, argv);
}

static void global_quit (void *dummy)
{
    interface_quit();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void global_font (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    font_withHostMeasured (argc, argv);
}

static void global_grid (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    snap_setSnapToGrid ((int)atom_getFloatAtIndex (0, argc, argv));
}

static void global_audioProperties (void *dummy)
{
    audio_requireDialog();
}

static void global_audioDialog (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    audio_fromDialog (argc, argv);
}

static void global_midiProperties (void *dummy)
{
    midi_requireDialog();
}

static void global_midiDialog (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    midi_fromDialog (argc, argv);
}

static void global_setSearchPath (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    searchpath_setRootsEncoded (argc, argv);
}

static void global_shouldQuit (void *dummy)
{
    canvas_quit();
}

static void global_savePreferences (void *dummy)
{
    preferences_save();
}

static void global_default (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    error_unknownMethod (sym_pd, s);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void global_dummy (void *dummy)
{
    PD_ASSERT (dummy == &global_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void global_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_pd, 
            NULL,
            NULL,
            0,
            CLASS_ABSTRACT,
            A_NULL);

    class_addMethod (c, (t_method)global_newPatch,              sym_new,    A_SYMBOL, A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)global_open,                  sym_open,   A_SYMBOL, A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)global_scan,                  sym_scan,   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_clear,                 sym_clear,  A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_dsp,                   sym_dsp,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_quit,                  sym_quit,   A_NULL);
    
    class_addMethod (c, (t_method)global_key,                   sym__key,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_font,                  sym__font,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_grid,                  sym__grid,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_audioProperties,       sym__audioproperties,   A_NULL);
    class_addMethod (c, (t_method)global_audioDialog,           sym__audiodialog,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_midiProperties,        sym__midiproperties,    A_NULL);
    class_addMethod (c, (t_method)global_midiDialog,            sym__mididialog,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_setSearchPath,         sym__path,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_shouldQuit,            sym__quit,              A_NULL);
    class_addMethod (c, (t_method)global_savePreferences,       sym__savepreferences,   A_NULL);
    class_addMethod (c, (t_method)global_dummy,                 sym__dummy,             A_NULL);
    
    class_addAnything (c, (t_method)global_default);
    
    global_class = c;
        
    pd_bind (&global_class, sym_pd);        /* Fake binding the abstract class. */
}

void global_destroy (void)
{
    pd_unbind (&global_class, sym_pd);
    
    class_free (global_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

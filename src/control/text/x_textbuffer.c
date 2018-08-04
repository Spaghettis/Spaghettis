
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "x_text.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void textbuffer_init (t_textbuffer *x)
{
    x->tb_buffer = buffer_new();
    x->tb_owner  = instance_contextGetCurrent();
}

void textbuffer_free (t_textbuffer *x)
{
    buffer_free (x->tb_buffer);
    
    if (x->tb_proxy) {
        gui_vAdd ("destroy %s\n", proxy_getTagAsString (x->tb_proxy));
        proxy_release (x->tb_proxy);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_glist *textbuffer_getView (t_textbuffer *x)
{
    return x->tb_owner;
}

t_buffer *textbuffer_getBuffer (t_textbuffer *x)
{
    return x->tb_buffer;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void textbuffer_click (t_textbuffer *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->tb_proxy) {
    
        gui_vAdd ("wm deiconify %s\n", proxy_getTagAsString (x->tb_proxy));
        gui_vAdd ("raise %s\n", proxy_getTagAsString (x->tb_proxy));
        gui_vAdd ("focus %s.text\n", proxy_getTagAsString (x->tb_proxy));
        
    } else {
    
        x->tb_proxy = proxy_new (cast_pd (x));
        gui_vAdd ("::ui_text::show %s\n", proxy_getTagAsString (x->tb_proxy));
        textbuffer_update (x);
    }
}

void textbuffer_close (t_textbuffer *x)
{
    if (x->tb_proxy) {
        gui_vAdd ("::ui_text::release %s\n", proxy_getTagAsString (x->tb_proxy));
        proxy_release (x->tb_proxy); 
        x->tb_proxy = NULL;
    }    
}

void textbuffer_update (t_textbuffer *x)
{
    if (x->tb_proxy) {
    //
    int size;
    char *text = NULL;
    const char *tag = proxy_getTagAsString (x->tb_proxy);
    int i = 0;
    
    buffer_toStringUnzeroed (x->tb_buffer, &text, &size);
    
    gui_vAdd ("::ui_text::clear %s\n", tag);
    
    while (i < size) {                              /* Send it line by line. */

        char *start   = text + i;
        char *newline = strchr (start, '\n');
        
        if (!newline) { newline = text + size; }
        
        /* < http://stackoverflow.com/a/13289324 > */
        
        gui_vAdd ("::ui_text::append %s {%.*s\n}\n", tag, (int)(newline - start), start);  // --
        
        i = (int)(newline - text) + 1;
    }
    
    gui_vAdd ("::ui_text::dirty %s 0\n", tag);
    
    PD_MEMORY_FREE (text);
    //
    }
}

void textbuffer_read (t_textbuffer *x, t_symbol *name)
{
    if (buffer_fileRead (x->tb_buffer, name, x->tb_owner)) { error_failsToRead (name); }
    textbuffer_update (x);
}

void textbuffer_write (t_textbuffer *x, t_symbol *name)
{
    t_symbol *directory = environment_getDirectory (glist_getEnvironment (x->tb_owner));
    if (buffer_fileWrite (x->tb_buffer, name, directory)) { error_failsToWrite (name); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Used only by the GUI to set contents of the buffer. */

void textbuffer_addLine (t_textbuffer *x, t_symbol *s, int argc, t_atom *argv)
{
    t_buffer *t = buffer_new();
    buffer_deserialize (t, argc, argv);
    buffer_appendBuffer (x->tb_buffer, t);
    buffer_free (t);
    textbuffer_update (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

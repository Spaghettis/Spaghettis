
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Original "g_7_guis.h" written by Thomas Musil (c) IEM KUG Graz Austria 2000-2001. */

/* Thanks to Miller Puckette, Guenther Geiger and Krzystof Czaja. */

/* < http://iem.kug.ac.at/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define IEM_BANG_DEFAULT_HOLD       250

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define IEM_BANG_MINIMUM_HOLD       10

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void bng_behaviorGetRectangle    (t_gobj *, t_glist *, t_rectangle *);
static int  bng_behaviorMouse           (t_gobj *, t_glist *, t_mouse *);
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_class *bng_class;                          /* Shared. */

static t_widgetbehavior bng_widgetBehavior =        /* Shared. */
    {
        bng_behaviorGetRectangle,
        iemgui_behaviorDisplaced,
        iemgui_behaviorSelected,
        NULL,
        iemgui_behaviorDeleted,
        iemgui_behaviorVisibilityChanged,
        bng_behaviorMouse
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void bng_taskFlash (t_bng *x)
{
    x->x_flashed = 0; IEMGUI_UPDATE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void bng_drawJob (t_gobj *z, t_glist *glist)
{
    t_bng *x = (t_bng *)z;
    
    gui_vAdd ("%s.c itemconfigure %lxBUTTON -fill #%06x\n", 
                    glist_getTagAsString (glist_getView (glist)),
                    x,
                    x->x_flashed ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void bng_drawUpdate (t_bng *x, t_glist *glist)
{
    gui_jobAdd ((void *)x, glist, bng_drawJob);
}

static void bng_drawMove (t_bng *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
        
    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));
    
    gui_vAdd ("%s.c coords %lxBASE %d %d %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    a,
                    b,
                    a + x->x_gui.iem_width,
                    b + x->x_gui.iem_height);
    gui_vAdd ("%s.c coords %lxBUTTON %d %d %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    a + 1,
                    b + 1,
                    a + x->x_gui.iem_width - 1,
                    b + x->x_gui.iem_height - 1);
}

static void bng_drawNew (t_bng *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));
    
    gui_vAdd ("%s.c create rectangle %d %d %d %d -fill #%06x -tags %lxBASE\n",
                    glist_getTagAsString (view),
                    a,
                    b,
                    a + x->x_gui.iem_width,
                    b + x->x_gui.iem_height,
                    x->x_gui.iem_colorBackground,
                    x);
    gui_vAdd ("%s.c create oval %d %d %d %d -fill #%06x -outline #%06x -tags %lxBUTTON\n",
                    glist_getTagAsString (view),
                    a + 1,
                    b + 1,
                    a + x->x_gui.iem_width - 1,
                    b + x->x_gui.iem_height - 1,
                    x->x_flashed ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground,
                    COLOR_NORMAL,
                    x);
}

static void bng_drawSelect (t_bng *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c itemconfigure %lxBASE -outline #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : COLOR_NORMAL);
    gui_vAdd ("%s.c itemconfigure %lxBUTTON -outline #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : COLOR_NORMAL);
}

static void bng_drawErase (t_bng *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c delete %lxBASE\n",
                    glist_getTagAsString (view),
                    x);
    gui_vAdd ("%s.c delete %lxBUTTON\n",
                    glist_getTagAsString (view),
                    x);
}

static void bng_drawConfig (t_bng *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c itemconfigure %lxBASE -fill #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_colorBackground);
    gui_vAdd ("%s.c itemconfigure %lxBUTTON -fill #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_flashed ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void bng_draw (t_bng *x, t_glist *glist, int mode)
{
    switch (mode) {
        case IEM_DRAW_UPDATE    : bng_drawUpdate (x, glist);    break;
        case IEM_DRAW_MOVE      : bng_drawMove (x, glist);      break;
        case IEM_DRAW_NEW       : bng_drawNew (x, glist);       break;
        case IEM_DRAW_SELECT    : bng_drawSelect (x, glist);    break;
        case IEM_DRAW_ERASE     : bng_drawErase (x, glist);     break;
        case IEM_DRAW_CONFIG    : bng_drawConfig (x, glist);    break;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void bng_updateFlash (t_bng *x)
{
    if (!x->x_flashed) { x->x_flashed = 1; IEMGUI_UPDATE (x); }
    
    clock_delay (x->x_clock, x->x_flashTime);
}

static void bng_out (t_bng *x)
{
    outlet_bang (x->x_outlet);
    
    if (x->x_gui.iem_canSend && symbol_hasThing (x->x_gui.iem_send)) {
        pd_bang (symbol_getThing (x->x_gui.iem_send));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void bng_bang (t_bng *x)
{
    bng_updateFlash (x);
    
    if (x->x_gui.iem_goThrough) { 
        bng_out (x);
    }
}

static void bng_float (t_bng *x, t_float f)
{
    bng_bang (x);
}

static void bng_symbol (t_bng *x, t_symbol *s)
{
    bng_bang (x);
}

static void bng_pointer (t_bng *x, t_gpointer *gp)
{
    bng_bang (x);
}

static void bng_list (t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    bng_bang (x);
}

static void bng_anything (t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    bng_bang (x);
}

static void bng_click (t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    bng_updateFlash (x);
    bng_out (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void bng_loadbang (t_bng *x)
{
    if (x->x_gui.iem_loadbang) { bng_bang (x); }
}

static void bng_initialize (t_bng *x, t_float f)
{
    x->x_gui.iem_loadbang = (f != 0.0);
}

static void bng_size (t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    int size = atom_getFloatAtIndex (0, argc, argv);
    x->x_gui.iem_width  = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = PD_MAX (size, IEM_MINIMUM_WIDTH);
    iemgui_boxChanged ((void *)x);
    //
    }
}

static void bng_flashtime (t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    int n = x->x_flashTime;
    
    if (argc == 1) { n = (int)atom_getFloatAtIndex (0, argc, argv); }
    if (argc == 2) { n = (int)atom_getFloatAtIndex (1, argc, argv); }
    
    x->x_flashTime = PD_MAX (n, IEM_BANG_MINIMUM_HOLD);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void bng_behaviorGetRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    int a = glist_getPixelX (glist, cast_object (z));
    int b = glist_getPixelY (glist, cast_object (z));
    int c = a + cast_iem (z)->iem_width;
    int d = b + cast_iem (z)->iem_height;
    
    rectangle_set (r, a, b, c, d);
}

static int bng_behaviorMouse (t_gobj *z, t_glist *glist, t_mouse *m)
{
    if (m->m_clicked) { bng_click ((t_bng *)z, NULL, 0, NULL); }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void bng_functionSave (t_gobj *z, t_buffer *b, int flags)
{
    t_bng *x = (t_bng *)z;
    
    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize (cast_iem (z), &names, &colors);
    
    buffer_appendSymbol (b, sym___hash__X);
    buffer_appendSymbol (b, sym_obj);
    buffer_appendFloat (b,  object_getX (cast_object (z)));
    buffer_appendFloat (b,  object_getY (cast_object (z)));
    buffer_appendSymbol (b, sym_bng);
    buffer_appendFloat (b,  x->x_gui.iem_width);
    buffer_appendFloat (b,  x->x_flashTime);
    buffer_appendFloat (b,  0);                                 /* Legacy. */
    buffer_appendFloat (b,  iemgui_serializeLoadbang (cast_iem (z)));
    buffer_appendSymbol (b, names.n_unexpandedSend);
    buffer_appendSymbol (b, names.n_unexpandedReceive);
    buffer_appendSymbol (b, symbol_nil());                      /* Legacy. */
    buffer_appendFloat (b,  0);                                 /* Legacy. */
    buffer_appendFloat (b,  0);                                 /* Legacy. */
    buffer_appendFloat (b,  0);                                 /* Legacy. */
    buffer_appendFloat (b,  0);                                 /* Legacy. */
    buffer_appendSymbol (b, colors.c_symColorBackground);
    buffer_appendSymbol (b, colors.c_symColorForeground);
    buffer_appendSymbol (b, color_toEncoded (0));               /* Legacy. */
    buffer_appendSemicolon (b);
    
    gobj_saveUniques (z, b, flags);
}

/* Fake dialog message from interpreter. */

static void bng_functionUndo (t_gobj *z, t_buffer *b)
{
    t_bng *x = (t_bng *)z;
    
    t_iemnames names;

    iemgui_serializeNames (cast_iem (z), &names);
    
    buffer_appendSymbol (b, sym__iemdialog);
    buffer_appendFloat (b,  x->x_gui.iem_width);                /* Width. */
    buffer_appendFloat (b,  -1.0);                              /* Height. */
    buffer_appendFloat (b,  x->x_flashTime);                    /* Option1. */
    buffer_appendFloat (b,  -1.0);                              /* Option2. */
    buffer_appendFloat (b,  -1.0);                              /* Check. */
    buffer_appendFloat (b,  x->x_gui.iem_loadbang);             /* Loadbang. */
    buffer_appendFloat (b,  -1.0);                              /* Extra. */
    buffer_appendSymbol (b, names.n_unexpandedSend);            /* Send. */
    buffer_appendSymbol (b, names.n_unexpandedReceive);         /* Receive. */
    buffer_appendFloat (b,  x->x_gui.iem_colorBackground);      /* Background color. */
    buffer_appendFloat (b,  x->x_gui.iem_colorForeground);      /* Foreground color. */
    buffer_appendFloat (b,  -1.0);                              /* Steady. */
    buffer_appendFloat (b,  -1.0);                              /* Save. */
}

static void bng_functionProperties (t_gobj *z, t_glist *owner, t_mouse *dummy)
{
    t_bng *x = (t_bng *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    t_iemnames names;

    iemgui_serializeNames (cast_iem (z), &names);
    
    err = string_sprintf (t, PD_STRING,
            "::ui_iem::create %%s Bang"
            " %d %d Size -1 -1 $::var(nil)"                     // --
            " %d {Flash Time} -1 $::var(nil)"                   // --
            " -1 $::var(nil) $::var(nil)"                       // --
            " %d"
            " -1 -1 $::var(nil)"                                // --
            " %s %s"
            " %d %d"
            " -1"
            " -1\n",
            x->x_gui.iem_width, IEM_MINIMUM_WIDTH,
            x->x_flashTime,
            x->x_gui.iem_loadbang,
            names.n_unexpandedSend->s_name, names.n_unexpandedReceive->s_name,
            x->x_gui.iem_colorBackground, x->x_gui.iem_colorForeground);

    PD_UNUSED (err); PD_ASSERT (!err);
    
    stub_new (cast_pd (x), (void *)x, t);
}

static void bng_fromDialog (t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    int isDirty  = 0;
    int undoable = glist_undoIsOk (cast_iem (x)->iem_owner);
    
    PD_ASSERT (argc == IEM_DIALOG_SIZE);
    
    int t0 = x->x_gui.iem_width;
    int t1 = x->x_flashTime;
    
    t_undosnippet *snippet = NULL;
    
    if (undoable) { snippet = undosnippet_newProperties (cast_gobj (x), cast_iem (x)->iem_owner); }
    
    {
    //
    int size      = (int)atom_getFloatAtIndex (0, argc, argv);
    int flashTime = (int)atom_getFloatAtIndex (2, argc, argv);
    
    isDirty = iemgui_fromDialog (cast_iem (x), argc, argv);

    x->x_gui.iem_width  = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_flashTime      = PD_MAX (flashTime, IEM_BANG_MINIMUM_HOLD);
    //
    }
    
    isDirty |= (t0 != x->x_gui.iem_width);
    isDirty |= (t1 != x->x_flashTime);
    
    iemgui_dirty (cast_iem (x), isDirty, undoable, snippet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void bng_restore (t_bng *x)
{
    t_bng *old = (t_bng *)instance_pendingFetch (cast_gobj (x));
    
    if (old) { iemgui_restore (cast_gobj (x), cast_gobj (old)); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *bng_new (t_symbol *s, int argc, t_atom *argv)
{
    t_bng *x = (t_bng *)pd_new (bng_class);
    
    int size        = IEM_DEFAULT_SIZE;
    int flashHold   = IEM_BANG_DEFAULT_HOLD;
    
    if (argc != 14) { iemgui_deserializeDefault (cast_iem (x)); }
    else {
    //
    size            = (int)atom_getFloatAtIndex (0,  argc, argv);
    flashHold       = (int)atom_getFloatAtIndex (1,  argc, argv);
    
    iemgui_deserializeLoadbang (cast_iem (x), (int)atom_getFloatAtIndex (3, argc, argv));
    iemgui_deserializeNames (cast_iem (x), 4, argv);
    iemgui_deserializeColors (cast_iem (x), argv + 11, argv + 12);
    //
    }

    x->x_gui.iem_owner      = instance_contextGetCurrent();
    x->x_gui.iem_fnDraw     = (t_iemfn)bng_draw;
    x->x_gui.iem_canSend    = symbol_isNil (x->x_gui.iem_send) ? 0 : 1;
    x->x_gui.iem_canReceive = symbol_isNil (x->x_gui.iem_receive) ? 0 : 1;
    x->x_gui.iem_width      = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height     = PD_MAX (size, IEM_MINIMUM_WIDTH);
    
    iemgui_checkSendReceiveLoop (cast_iem (x));
    
    if (x->x_gui.iem_canReceive) { pd_bind (cast_pd (x), x->x_gui.iem_receive); }
    
    x->x_flashTime          = PD_MAX (flashHold, IEM_BANG_MINIMUM_HOLD);
    
    x->x_outlet = outlet_newBang (cast_object (x));
    x->x_clock  = clock_new ((void *)x, (t_method)bng_taskFlash);
    
    return x;
}

static void bng_free (t_bng *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_pd (x), x->x_gui.iem_receive); }
    
    clock_free (x->x_clock);
    
    stub_destroyWithKey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void bng_setup (void) 
{
    t_class *c = NULL;
    
    c = class_new (sym_bng, 
            (t_newmethod)bng_new, 
            (t_method)bng_free, 
            sizeof (t_bng), 
            CLASS_DEFAULT,
            A_GIMME, 
            A_NULL);
    
    class_addBang (c, (t_method)bng_bang);
    class_addFloat (c, (t_method)bng_float);
    class_addSymbol (c, (t_method)bng_symbol);
    class_addPointer (c, (t_method)bng_pointer);
    class_addList (c, (t_method)bng_list);
    class_addAnything (c, (t_method)bng_anything);
    class_addClick (c, (t_method)bng_click);
    class_addLoadbang (c, (t_method)bng_loadbang);
    
    class_addMethod (c, (t_method)bng_initialize,               sym_initialize,         A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)bng_fromDialog,               sym__iemdialog,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_size,                     sym_size,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_flashtime,                sym_flashtime,          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setBackgroundColor,    sym_backgroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setForegroundColor,    sym_foregroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setSend,               sym_send,               A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemgui_setReceive,            sym_receive,            A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)bng_restore,                  sym__restore,           A_NULL);

    class_setWidgetBehavior (c, &bng_widgetBehavior);
    class_setSaveFunction (c, bng_functionSave);
    class_setDataFunction (c, iemgui_functionData);
    class_setUndoFunction (c, bng_functionUndo);
    class_setPropertiesFunction (c, bng_functionProperties);
    class_requirePending (c);
    
    bng_class = c;
}

void bng_destroy (void)
{
    class_free (bng_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

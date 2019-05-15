
/* Copyright (c) 1997-2019 Miller Puckette and others. */

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

/* Note that the grip size has been kept for compatibility with legacy patches only. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define IEM_PANEL_DEFAULT_WIDTH     275
#define IEM_PANEL_DEFAULT_HEIGHT    45

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define IEM_PANEL_MINIMUM_SIZE      1

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void panel_behaviorGetRectangle (t_gobj *, t_glist *, t_rectangle *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_class *panel_class;                                   /* Shared. */

static t_widgetbehavior panel_widgetBehavior =          /* Shared. */
    {
        panel_behaviorGetRectangle,
        iemgui_behaviorDisplaced,
        iemgui_behaviorSelected,
        NULL,
        iemgui_behaviorDeleted,
        iemgui_behaviorVisibilityChanged,
        NULL
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void panel_drawMove (t_panel *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));

    gui_vAdd ("%s.c coords %lxPANEL %d %d %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    a,
                    b,
                    a + x->x_panelWidth,
                    b + x->x_panelHeight);
    gui_vAdd ("%s.c coords %lxBASE %d %d %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    a, 
                    b,
                    a + x->x_panelWidth,
                    b + x->x_panelHeight);
}

static void panel_drawNew (t_panel *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));

    gui_vAdd ("%s.c create rectangle %d %d %d %d -fill #%06x -outline #%06x -tags %lxPANEL\n",
                    glist_getTagAsString (view),
                    a,
                    b,
                    a + x->x_panelWidth,
                    b + x->x_panelHeight,
                    x->x_gui.iem_colorBackground,
                    x->x_gui.iem_colorBackground,
                    x);
    gui_vAdd ("%s.c create rectangle %d %d %d %d -outline #%06x -tags %lxBASE\n",
                    glist_getTagAsString (view),
                    a,
                    b,
                    a + x->x_panelWidth,
                    b + x->x_panelHeight,
                    x->x_gui.iem_colorBackground,
                    x);
}

static void panel_drawSelect (t_panel *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c itemconfigure %lxBASE -outline #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : x->x_gui.iem_colorBackground);
}

static void panel_drawErase (t_panel *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c delete %lxBASE\n",
                    glist_getTagAsString (view),
                    x);
    gui_vAdd ("%s.c delete %lxPANEL\n",
                    glist_getTagAsString (view),
                    x);
}

static void panel_drawConfig (t_panel *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c itemconfigure %lxPANEL -fill #%06x -outline #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_colorBackground,
                    x->x_gui.iem_colorBackground);
    gui_vAdd ("%s.c itemconfigure %lxBASE -outline #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_colorBackground);
    
    panel_drawSelect (x, glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void panel_draw (t_panel *x, t_glist *glist, int mode)
{
    switch (mode) {
        case IEM_DRAW_MOVE      : panel_drawMove (x, glist);    break;
        case IEM_DRAW_NEW       : panel_drawNew (x, glist);     break;
        case IEM_DRAW_SELECT    : panel_drawSelect (x, glist);  break;
        case IEM_DRAW_ERASE     : panel_drawErase (x, glist);   break;
        case IEM_DRAW_CONFIG    : panel_drawConfig (x, glist);  break;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void panel_size (t_panel *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    int i = (int)atom_getFloatAtIndex (0, argc, argv);

    x->x_panelWidth = PD_MAX (i, IEM_PANEL_MINIMUM_SIZE);
    
    if (argc > 1) { 
        i = (int)atom_getFloatAtIndex (1, argc, argv); 
    }
    
    x->x_panelHeight = PD_MAX (i, IEM_PANEL_MINIMUM_SIZE);
    
    (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_MOVE);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void panel_behaviorGetRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    t_panel *x = (t_panel *)z;
    
    int a = glist_getPixelX (glist, cast_object (z));
    int b = glist_getPixelY (glist, cast_object (z));
    int c = a + x->x_panelWidth;
    int d = b + x->x_panelHeight;
    
    rectangle_set (r, a, b, c, d);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void panel_functionSave (t_gobj *z, t_buffer *b, int flags)
{
    t_panel *x = (t_panel *)z;
    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize (cast_iem (z), &names, &colors);
    
    buffer_appendSymbol (b, sym___hash__X);
    buffer_appendSymbol (b, sym_obj);
    buffer_appendFloat (b,  object_getX (cast_object (z)));
    buffer_appendFloat (b,  object_getY (cast_object (z)));
    buffer_appendSymbol (b, sym_cnv);
    buffer_appendFloat (b,  x->x_gui.iem_width);
    buffer_appendFloat (b,  x->x_panelWidth);
    buffer_appendFloat (b,  x->x_panelHeight);
    buffer_appendSymbol (b, names.n_unexpandedSend);
    buffer_appendSymbol (b, names.n_unexpandedReceive);
    buffer_appendSymbol (b, names.n_unexpandedLabel);
    buffer_appendFloat (b,  x->x_gui.iem_labelX);
    buffer_appendFloat (b,  x->x_gui.iem_labelY);
    buffer_appendFloat (b,  iemgui_serializeFontStyle (cast_iem (z)));
    buffer_appendFloat (b,  x->x_gui.iem_fontSize);
    buffer_appendSymbol (b, colors.c_symColorBackground);
    buffer_appendSymbol (b, colors.c_symColorLabel);
    buffer_appendSemicolon (b);
    
    gobj_saveUniques (z, b, flags);
}

static void panel_functionUndo (t_gobj *z, t_buffer *b)
{
    t_panel *x = (t_panel *)z;
    
    t_iemnames names;

    iemgui_serializeNames (cast_iem (z), &names);
    
    buffer_appendSymbol (b, sym__iemdialog);
    buffer_appendFloat (b,  -1.0);                              /* Width. */
    buffer_appendFloat (b,  -1.0);                              /* Height. */
    buffer_appendFloat (b,  x->x_panelWidth);                   /* Option1. */
    buffer_appendFloat (b,  x->x_panelHeight);                  /* Option2. */
    buffer_appendFloat (b,  -1.0);                              /* Check. */
    buffer_appendFloat (b,  -1.0);                              /* Loadbang. */
    buffer_appendFloat (b,  -1.0);                              /* Extra. */
    buffer_appendSymbol (b, names.n_unexpandedSend);            /* Send. */
    buffer_appendSymbol (b, names.n_unexpandedReceive);         /* Receive. */
    buffer_appendFloat (b,  x->x_gui.iem_colorBackground);      /* Background color. */
    buffer_appendFloat (b,  x->x_gui.iem_colorForeground);      /* Foreground color. */
    buffer_appendFloat (b,  -1.0);                              /* Steady. */
    buffer_appendFloat (b,  -1.0);                              /* Save. */
}

static void panel_functionProperties (t_gobj *z, t_glist *owner, t_mouse *dummy)
{
    t_panel *x = (t_panel *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    t_iemnames names;

    iemgui_serializeNames (cast_iem (z), &names);
    
    err = string_sprintf (t, PD_STRING, "::ui_iem::create %%s Panel"
            " -1 -1 $::var(nil) -1 -1 $::var(nil)"              // --
            " %d {Panel Width} %d {Panel Height}"               // --
            " -1 $::var(nil) $::var(nil)"                       // --
            " -1"
            " -1 -1 $::var(nil)"                                // --
            " %s %s"
            " %d %d"
            " -1"
            " -1\n",
            x->x_panelWidth, x->x_panelHeight,
            names.n_unexpandedSend->s_name, names.n_unexpandedReceive->s_name,
            x->x_gui.iem_colorBackground, x->x_gui.iem_colorForeground);
            
    PD_UNUSED (err); PD_ASSERT (!err);
    
    stub_new (cast_pd (x), (void *)x, t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void panel_fromDialog (t_panel *x, t_symbol *s, int argc, t_atom *argv)
{
    int isDirty  = 0;
    int undoable = glist_undoIsOk (cast_iem (x)->iem_owner);
    
    PD_ASSERT (argc == IEM_DIALOG_SIZE);
    
    int t0 = x->x_panelWidth;
    int t1 = x->x_panelHeight;
    
    t_undosnippet *snippet = NULL;
    
    if (undoable) { snippet = undosnippet_newProperties (cast_gobj (x), cast_iem (x)->iem_owner); }

    {
    //
    int panelWidth  = (int)atom_getFloatAtIndex (2, argc, argv);
    int panelHeight = (int)atom_getFloatAtIndex (3, argc, argv);
    
    isDirty = iemgui_fromDialog (cast_iem (x), argc, argv);

    x->x_panelWidth     = PD_MAX (panelWidth,  IEM_PANEL_MINIMUM_SIZE);
    x->x_panelHeight    = PD_MAX (panelHeight, IEM_PANEL_MINIMUM_SIZE);
    //
    }
    
    isDirty |= (t0 != x->x_panelWidth);
    isDirty |= (t1 != x->x_panelHeight);
    
    iemgui_dirty (cast_iem (x), isDirty, undoable, snippet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void panel_restore (t_panel *x)
{
    t_panel *old = (t_panel *)instance_pendingFetch (cast_gobj (x));
    
    if (old) { iemgui_restore (cast_gobj (x), cast_gobj (old)); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *panel_new (t_symbol *s, int argc, t_atom *argv)
{
    t_panel *x = (t_panel *)pd_new (panel_class);
    
    int gripSize        = IEM_DEFAULT_SIZE;
    int panelWidth      = IEM_PANEL_DEFAULT_WIDTH;
    int panelHeight     = IEM_PANEL_DEFAULT_HEIGHT;
    int labelX          = 0;
    int labelY          = 0;
    int labelFontSize   = IEM_DEFAULT_FONT;
        
    if (argc < 12) { iemgui_deserializeDefault (cast_iem (x)); }
    else {
    //
    gripSize        = (int)atom_getFloatAtIndex (0, argc, argv);
    panelWidth      = (int)atom_getFloatAtIndex (1, argc, argv);
    panelHeight     = (int)atom_getFloatAtIndex (2, argc, argv);
    labelX          = (int)atom_getFloatAtIndex (6, argc, argv);
    labelY          = (int)atom_getFloatAtIndex (7, argc, argv);
    labelFontSize   = (int)atom_getFloatAtIndex (9, argc, argv);
    
    iemgui_deserializeNames (cast_iem (x), 3, argv);
    iemgui_deserializeFontStyle (cast_iem (x), (int)atom_getFloatAtIndex (8, argc, argv));
    iemgui_deserializeColors (cast_iem (x), argv + 10, NULL, argv + 11);
    //
    }
    
    x->x_gui.iem_owner      = instance_contextGetCurrent();
    x->x_gui.iem_fnDraw     = (t_iemfn)panel_draw;
    x->x_gui.iem_canSend    = symbol_isNil (x->x_gui.iem_send) ? 0 : 1;
    x->x_gui.iem_canReceive = symbol_isNil (x->x_gui.iem_receive) ? 0 : 1;

    x->x_gui.iem_width      = PD_MAX (gripSize, IEM_PANEL_MINIMUM_SIZE);
    x->x_gui.iem_height     = PD_MAX (gripSize, IEM_PANEL_MINIMUM_SIZE);
    x->x_gui.iem_labelX     = labelX;
    x->x_gui.iem_labelY     = labelY;
    x->x_gui.iem_fontSize   = labelFontSize;
    
    iemgui_checkSendReceiveLoop (cast_iem (x));
    
    if (x->x_gui.iem_canReceive) { pd_bind (cast_pd (x), x->x_gui.iem_receive); }
    
    x->x_panelWidth  = PD_MAX (panelWidth,  IEM_PANEL_MINIMUM_SIZE);
    x->x_panelHeight = PD_MAX (panelHeight, IEM_PANEL_MINIMUM_SIZE);

    SET_FLOAT (&x->x_t[0], 0.0);
    SET_FLOAT (&x->x_t[1], 0.0);
    
    return x;
}

static void panel_free (t_panel *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_pd (x), x->x_gui.iem_receive); }
    
    stub_destroyWithKey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void panel_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_cnv, 
            (t_newmethod)panel_new,
            (t_method)panel_free,
            sizeof (t_panel), 
            CLASS_DEFAULT | CLASS_NOINLET,
            A_GIMME,
            A_NULL);
        
    class_addMethod (c, (t_method)panel_fromDialog,             sym__iemdialog,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)panel_size,                   sym_size,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setBackgroundColor,    sym_backgroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setForegroundColor,    sym_foregroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setSend,               sym_send,               A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemgui_setReceive,            sym_receive,            A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)panel_restore,                sym__restore,           A_NULL);

    class_setWidgetBehavior (c, &panel_widgetBehavior);
    class_setSaveFunction (c, panel_functionSave);
    class_setDataFunction (c, iemgui_functionData);
    class_setUndoFunction (c, panel_functionUndo);
    class_setPropertiesFunction (c, panel_functionProperties);
    class_requirePending (c);
    
    panel_class = c;
}

void panel_destroy (void)
{
    class_free (panel_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

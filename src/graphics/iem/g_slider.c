
/* Copyright (c) 1997-2018 Miller Puckette and others. */

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

#define IEM_HSLIDER_DEFAULT_WIDTH       128
#define IEM_VSLIDER_DEFAULT_WIDTH       15
#define IEM_HSLIDER_DEFAULT_HEIGHT      15
#define IEM_VSLIDER_DEFAULT_HEIGHT      128

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define IEM_SLIDER_STEPS_PER_PIXEL      100

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_APPLE

#define IEM_SLIDER_PIXEL                1

#else

#define IEM_SLIDER_PIXEL                0

#endif // PD_APPLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int slider_stepsToPixels (int n)
{
    return (int)((n / (double)IEM_SLIDER_STEPS_PER_PIXEL) + 0.5);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void slider_set                  (t_slider *, t_float);
static void slider_motion               (t_slider *, t_float, t_float, t_float);
static void slider_behaviorGetRectangle (t_gobj *, t_glist *, t_rectangle *);
static int  slider_behaviorMouse        (t_gobj *, t_glist *, t_mouse *);
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_class *slider_class;                           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_widgetbehavior slider_widgetBehavior =         /* Shared. */
    {
        slider_behaviorGetRectangle,
        iemgui_behaviorDisplaced,
        iemgui_behaviorSelected,
        NULL,
        iemgui_behaviorDeleted,
        iemgui_behaviorVisibilityChanged,
        slider_behaviorMouse
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void slider_drawUpdateVertical (t_slider *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    int a = glist_getPixelX (glist, cast_object (x));
    int k = glist_getPixelY (glist, cast_object (x));
    
    k += x->x_gui.iem_height - slider_stepsToPixels (x->x_position);
        
    gui_vAdd ("%s.c coords %lxKNOB %d %d %d %d\n",
                    glist_getTagAsString (view), 
                    x, 
                    a + 1,
                    k,
                    a + x->x_gui.iem_width - IEM_SLIDER_PIXEL, 
                    k);
}

static void slider_drawUpdateHorizontal (t_slider *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    int k = glist_getPixelX (glist, cast_object (x)) + slider_stepsToPixels (x->x_position);
    int b = glist_getPixelY (glist, cast_object (x));
        
    gui_vAdd ("%s.c coords %lxKNOB %d %d %d %d\n",
                    glist_getTagAsString (view), 
                    x, 
                    k,
                    b + 1,
                    k, 
                    b + x->x_gui.iem_height - IEM_SLIDER_PIXEL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void slider_drawJob (t_gobj *z, t_glist *glist)
{
    t_slider *x = (t_slider *)z;
    
    if (x->x_isVertical) { slider_drawUpdateVertical (x, glist); }
    else {
        slider_drawUpdateHorizontal (x, glist);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void slider_drawUpdate (t_slider *x, t_glist *glist)
{
    gui_jobAdd ((void *)x, glist, slider_drawJob);
}

static void slider_drawMove (t_slider *x, t_glist *glist)
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
                
    slider_drawUpdate (x, glist);
}

static void slider_drawNew (t_slider *x, t_glist *glist)
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
    
    if (x->x_isVertical) {
    //
    int k = b + x->x_gui.iem_height - slider_stepsToPixels (x->x_position);
    gui_vAdd ("%s.c create line %d %d %d %d -width 3 -fill #%06x -tags %lxKNOB\n",
                    glist_getTagAsString (view),
                    a + 1,
                    k, 
                    a + x->x_gui.iem_width - IEM_SLIDER_PIXEL,
                    k,
                    x->x_gui.iem_colorForeground,
                    x);
    //
    } else {
    //
    int k = a + slider_stepsToPixels (x->x_position);
    gui_vAdd ("%s.c create line %d %d %d %d -width 3 -fill #%06x -tags %lxKNOB\n",
                    glist_getTagAsString (view),
                    k,
                    b + 1, 
                    k,
                    b + x->x_gui.iem_height - IEM_SLIDER_PIXEL,
                    x->x_gui.iem_colorForeground,
                    x);
    //
    }
}

static void slider_drawSelect (t_slider *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c itemconfigure %lxBASE -outline #%06x\n", 
                    glist_getTagAsString (view), 
                    x, 
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : COLOR_NORMAL);
}

static void slider_drawErase (t_slider *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c delete %lxBASE\n",
                    glist_getTagAsString (view), 
                    x);
    gui_vAdd ("%s.c delete %lxKNOB\n",
                    glist_getTagAsString (view),
                    x);
}

static void slider_drawConfig (t_slider *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c itemconfigure %lxBASE -fill #%06x\n",
                    glist_getTagAsString (view),
                    x, 
                    x->x_gui.iem_colorBackground);
    gui_vAdd ("%s.c itemconfigure %lxKNOB -fill #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_colorForeground);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void slider_draw (t_slider *x, t_glist *glist, int mode)
{
    switch (mode) {
        case IEM_DRAW_UPDATE    : slider_drawUpdate (x, glist); break;
        case IEM_DRAW_MOVE      : slider_drawMove (x, glist);   break;
        case IEM_DRAW_NEW       : slider_drawNew (x, glist);    break;
        case IEM_DRAW_SELECT    : slider_drawSelect (x, glist); break;
        case IEM_DRAW_ERASE     : slider_drawErase (x, glist);  break;
        case IEM_DRAW_CONFIG    : slider_drawConfig (x, glist); break;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int slider_getNumberOfSteps (t_slider *x)
{
    if (x->x_isVertical) { 
        return ((x->x_gui.iem_height - 1) * IEM_SLIDER_STEPS_PER_PIXEL);
    } else {
        return ((x->x_gui.iem_width - 1) * IEM_SLIDER_STEPS_PER_PIXEL);
    }
}

static double slider_getStepValue (t_slider *x)
{
    if (x->x_isLogarithmic) {
        return (log (x->x_maximum / x->x_minimum) / (double)slider_getNumberOfSteps (x));
    } else {
        return ((x->x_maximum - x->x_minimum) / (double)slider_getNumberOfSteps (x));
    }
}

static t_float slider_getValue (t_slider *x)
{
    double f, t = slider_getStepValue (x) * (double)x->x_position;
    
    if (x->x_isLogarithmic) { 
        f = x->x_minimum * exp (t); 
    } else {
        f = x->x_minimum + t;
    }
    
    if ((f < 1.0e-10) && (f > -1.0e-10)) { f = 0.0; }
    
    return (t_float)f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void slider_setRange (t_slider *x, double minimum, double maximum)
{
    t_error err = PD_ERROR_NONE;
    
    if (x->x_isLogarithmic) {
        err |= (minimum == 0.0);
        err |= (maximum * minimum < 0.0);
    }
    
    if (err) { 
        x->x_isLogarithmic = 0;
        error_invalid (sym_slider, sym_range);
    } else {
        x->x_minimum = minimum;
        x->x_maximum = maximum;
    }
}

static void slider_setWidth (t_slider *x, int width)
{
    x->x_gui.iem_width = PD_MAX (width, IEM_MINIMUM_WIDTH);
    
    if ((x->x_isVertical == 0) && x->x_position > slider_getNumberOfSteps (x)) {
        x->x_position = slider_getNumberOfSteps (x);
    }
}

static void slider_setHeight (t_slider *x, int height)
{
    x->x_gui.iem_height = PD_MAX (height, IEM_MINIMUM_HEIGHT);
    
    if ((x->x_isVertical == 1) && x->x_position > slider_getNumberOfSteps (x)) {
        x->x_position = slider_getNumberOfSteps (x);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void slider_out (t_slider *x)
{
    outlet_float (x->x_outlet, x->x_floatValue);

    if (x->x_gui.iem_canSend && symbol_hasThing (x->x_gui.iem_send)) {
        pd_float (symbol_getThing (x->x_gui.iem_send), x->x_floatValue);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void slider_bang (t_slider *x)
{
    slider_out (x);
}

static void slider_float (t_slider *x, t_float f)
{
    slider_set (x, f);
    
    if (x->x_gui.iem_goThrough) { slider_out (x); }
}

static void slider_click (t_slider *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float a = atom_getFloatAtIndex (0, argc, argv);
    t_float b = atom_getFloatAtIndex (1, argc, argv);
    t_float t;
    
    if (x->x_isVertical) {
        t = glist_getPixelY (x->x_gui.iem_owner, cast_object (x)) + x->x_gui.iem_height - b;
    } else {
        t = a - glist_getPixelX (x->x_gui.iem_owner, cast_object (x));
    }
    
    t *= IEM_SLIDER_STEPS_PER_PIXEL;
    
    if (!x->x_isSteadyOnClick) {
        int numberOfSteps = slider_getNumberOfSteps (x);
        x->x_position = PD_CLAMP ((int)t, 0, numberOfSteps);
        x->x_floatValue = slider_getValue (x);
        (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
    }
    
    glist_setMotion (x->x_gui.iem_owner, cast_gobj (x), (t_motionfn)slider_motion, a, b);
    
    slider_out (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void slider_motion (t_slider *x, t_float deltaX, t_float deltaY, t_float modifier)
{
    int containsX = (int)modifier & MODIFIER_INSIDE_X;
    int containsY = (int)modifier & MODIFIER_INSIDE_Y;
    int k = x->x_isSteadyOnClick || (x->x_isVertical && containsY) || (!x->x_isVertical && containsX);
    
    double f = slider_getValue (x);
    
    k |= (f > x->x_minimum) && (f < x->x_maximum);
        
    if (k) {
    //
    int old = x->x_position;
    int t = old;
    int numberOfSteps = slider_getNumberOfSteps (x);
    int shift = (int)modifier & MODIFIER_SHIFT;
    
    if (shift) { t += (int)(x->x_isVertical ? -deltaY : deltaX); }
    else {
        t += (int)(x->x_isVertical ? -deltaY : deltaX) * IEM_SLIDER_STEPS_PER_PIXEL;
    }
    
    t = PD_CLAMP (t, 0, numberOfSteps);
    
    if (t != old) {
        x->x_position   = t;
        x->x_floatValue = slider_getValue (x);
        (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
        slider_out (x);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void slider_loadbang (t_slider *x)
{
    if (x->x_gui.iem_loadbang) { slider_out (x); }
}

static void slider_initialize (t_slider *x, t_float f)
{
    x->x_gui.iem_loadbang = (f != 0.0);
}

static void slider_size (t_slider *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    slider_setWidth (x, (int)atom_getFloatAtIndex (0, argc, argv));
    if (argc > 1) { slider_setHeight (x, (int)atom_getFloatAtIndex (1, argc, argv)); }
    iemgui_boxChanged ((void *)x);
    //
    }
}

static void slider_range (t_slider *x, t_symbol *s, int argc, t_atom *argv)
{
    double minimum = (double)atom_getFloatAtIndex (0, argc, argv);
    double maximum = (double)atom_getFloatAtIndex (1, argc, argv);
    
    slider_setRange (x, minimum, maximum);
    
    x->x_floatValue = slider_getValue (x);
}

static void slider_set (t_slider *x, t_float f)
{
    int old = x->x_position;
    
    x->x_floatValue = f;
    
    if (x->x_minimum > x->x_maximum) { f = (t_float)PD_CLAMP (f, x->x_maximum, x->x_minimum); }
    else {
        f = (t_float)PD_CLAMP (f, x->x_minimum, x->x_maximum);
    }
    
    if (x->x_isLogarithmic) { 
        x->x_position = (int)(log (f / x->x_minimum) / slider_getStepValue (x));
    } else {
        x->x_position = (int)((f - x->x_minimum) / slider_getStepValue (x));
    }
        
    if (x->x_position != old) { (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE); }
}

static void slider_steady (t_slider *x, t_float f)
{
    x->x_isSteadyOnClick = (f != 0.0);
}

static void slider_logarithmic (t_slider *x)
{
    x->x_isLogarithmic = 1; 
    
    slider_setRange (x, x->x_minimum, x->x_maximum);
    
    x->x_floatValue = slider_getValue (x);
}

static void slider_linear (t_slider *x)
{
    x->x_isLogarithmic = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void slider_behaviorGetRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    int a = glist_getPixelX (glist, cast_object (z));
    int b = glist_getPixelY (glist, cast_object (z));
    int c = a + cast_iem (z)->iem_width;
    int d = b + cast_iem (z)->iem_height;
    
    rectangle_set (r, a, b, c, d);
}

static int slider_behaviorMouse (t_gobj *z, t_glist *glist, t_mouse *m)
{
    if (m->m_clicked) { slider_click ((t_slider *)z, NULL, mouse_argc (m), mouse_argv (m)); }
    
    return 1;
}

static void slider_functionSave (t_gobj *z, t_buffer *b)
{
    t_slider *x = (t_slider *)z;
    
    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize (cast_iem (z), &names, &colors);
    
    buffer_appendSymbol (b, sym___hash__X);
    buffer_appendSymbol (b, sym_obj);
    buffer_appendFloat (b,  object_getX (cast_object (z)));
    buffer_appendFloat (b,  object_getY (cast_object (z)));
    buffer_appendSymbol (b, x->x_isVertical ? sym_vslider : sym_hslider);
    buffer_appendFloat (b,  x->x_gui.iem_width);
    buffer_appendFloat (b,  x->x_gui.iem_height);
    buffer_appendFloat (b,  x->x_minimum);
    buffer_appendFloat (b,  x->x_maximum);
    buffer_appendFloat (b,  x->x_isLogarithmic);
    buffer_appendFloat (b,  iemgui_serializeLoadbang (cast_iem (z)));
    buffer_appendSymbol (b, names.n_unexpandedSend);
    buffer_appendSymbol (b, names.n_unexpandedReceive);
    buffer_appendSymbol (b, names.n_unexpandedLabel);
    buffer_appendFloat (b,  x->x_gui.iem_labelX);
    buffer_appendFloat (b,  x->x_gui.iem_labelY);
    buffer_appendFloat (b,  iemgui_serializeFontStyle (cast_iem (z)));
    buffer_appendFloat (b,  x->x_gui.iem_fontSize);
    buffer_appendSymbol (b, colors.c_symColorBackground);
    buffer_appendSymbol (b, colors.c_symColorForeground);
    buffer_appendSymbol (b, colors.c_symColorLabel);
    buffer_appendFloat (b,  x->x_position);
    buffer_appendFloat (b,  x->x_isSteadyOnClick);
    buffer_appendSemicolon (b);
}

static void slider_functionValue (t_gobj *z, t_glist *owner, t_mouse *dummy)
{
    t_slider *x = (t_slider *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    
    err = string_sprintf (t, PD_STRING,
            "::ui_value::show %%s slider floatatom %.9g\n",      // --
            x->x_floatValue);

    PD_UNUSED (err); PD_ASSERT (!err);
    
    stub_new (cast_pd (x), (void *)x, t);
}

static void slider_functionProperties (t_gobj *z, t_glist *owner, t_mouse *dummy)
{
    t_slider *x = (t_slider *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    t_iemnames names;

    iemgui_serializeNames (cast_iem (z), &names);

    err = string_sprintf (t, PD_STRING, "::ui_iem::create %%s Slider"
            " %d %d {Slider Width} %d %d {Slider Height}"               // --
            " %.9g {Value %s}"                                          // --
            " %.9g {Value %s}"                                          // --
            " %d Linear Logarithmic"
            " %d"
            " -1 -1 $::var(nil)"                                        // --
            " %s %s"
            " %d %d"
            " %d\n",
            x->x_gui.iem_width, IEM_MINIMUM_WIDTH, x->x_gui.iem_height, IEM_MINIMUM_HEIGHT,
            x->x_minimum, x->x_isVertical ? "Bottom" : "Left",
            x->x_maximum, x->x_isVertical ? "Top" : "Right",
            x->x_isLogarithmic, 
            x->x_gui.iem_loadbang,
            names.n_unexpandedSend->s_name, names.n_unexpandedReceive->s_name,
            x->x_gui.iem_colorBackground, x->x_gui.iem_colorForeground,
            x->x_isSteadyOnClick);
    
    PD_UNUSED (err); PD_ASSERT (!err);
    
    stub_new (cast_pd (x), (void *)x, t);
}

static void slider_fromValue (t_slider *x, t_symbol *s, int argc, t_atom *argv)
{
    slider_float (x, atom_getFloatAtIndex (0, argc, argv));
}

static void slider_fromDialog (t_slider *x, t_symbol *s, int argc, t_atom *argv)
{
    int isDirty = 0;
    
    PD_ASSERT (argc == IEM_DIALOG_SIZE);
    
    int t0    = x->x_isLogarithmic;
    int t1    = x->x_isSteadyOnClick;
    int t2    = x->x_gui.iem_width;
    int t3    = x->x_gui.iem_height;
    double t4 = x->x_minimum;
    double t5 = x->x_maximum;
        
    {
    //
    int width         = (int)atom_getFloatAtIndex (0, argc, argv);
    int height        = (int)atom_getFloatAtIndex (1, argc, argv);
    double minimum    = (double)atom_getFloatAtIndex (2, argc, argv);
    double maximum    = (double)atom_getFloatAtIndex (3, argc, argv);
    int isLogarithmic = (int)atom_getFloatAtIndex (4, argc, argv);
    int isSteady      = (int)atom_getFloatAtIndex (11, argc, argv);

    isDirty = iemgui_fromDialog (cast_iem (x), argc, argv);
    
    x->x_isLogarithmic   = (isLogarithmic != 0);
    x->x_isSteadyOnClick = (isSteady != 0);
    
    slider_setHeight (x, height);               /* Must be set at last. */
    slider_setWidth (x, width);                 /* Ditto. */
    slider_setRange (x, minimum, maximum);      /* Ditto. */
    
    x->x_floatValue = slider_getValue (x);
    //
    }
    
    isDirty |= (t0 != x->x_isLogarithmic);
    isDirty |= (t1 != x->x_isSteadyOnClick);
    isDirty |= (t2 != x->x_gui.iem_width);
    isDirty |= (t3 != x->x_gui.iem_height);
    isDirty |= (t4 != x->x_minimum);
    isDirty |= (t5 != x->x_maximum);
    
    if (isDirty) { iemgui_boxChanged ((void *)x); glist_setDirty (cast_iem (x)->iem_owner, 1); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *slider_new (t_symbol *s, int argc, t_atom *argv)
{
    t_slider *x = (t_slider *)pd_new (slider_class);
    
    if (s == sym_vslider) { x->x_isVertical = 1; }
    
    #if PD_WITH_LEGACY 
    
    if (s == sym_vsl)     { x->x_isVertical = 1; }

    #endif
    
    {
    //
    int width           = x->x_isVertical ? IEM_VSLIDER_DEFAULT_WIDTH  : IEM_HSLIDER_DEFAULT_WIDTH;
    int height          = x->x_isVertical ? IEM_VSLIDER_DEFAULT_HEIGHT : IEM_HSLIDER_DEFAULT_HEIGHT;
    int isLogarithmic   = 0;
    int labelX          = 0;
    int labelY          = 0;
    int isSteady        = 0;
    int labelFontSize   = IEM_DEFAULT_FONT;
    double minimum      = 0.0;
    double maximum      = (double)(x->x_isVertical ? (height - 1) : (width - 1));
    t_float position    = 0.0;

    if (argc < 17) { iemgui_deserializeDefault (cast_iem (x)); }
    else {
    //
    width           = (int)atom_getFloatAtIndex (0,  argc, argv);
    height          = (int)atom_getFloatAtIndex (1,  argc, argv);
    minimum         = (double)atom_getFloatAtIndex (2, argc, argv);
    maximum         = (double)atom_getFloatAtIndex (3, argc, argv);
    isLogarithmic   = (int)atom_getFloatAtIndex (4,  argc, argv);
    labelX          = (int)atom_getFloatAtIndex (9,  argc, argv);
    labelY          = (int)atom_getFloatAtIndex (10, argc, argv);
    labelFontSize   = (int)atom_getFloatAtIndex (12, argc, argv);
    position        = atom_getFloatAtIndex (16, argc, argv);
    
    if (argc == 18) { isSteady = (int)atom_getFloatAtIndex (17, argc, argv); }
    
    iemgui_deserializeLoadbang (cast_iem (x), (int)atom_getFloatAtIndex (5, argc, argv));
    iemgui_deserializeNames (cast_iem (x), 6, argv);
    iemgui_deserializeFontStyle (cast_iem (x), (int)atom_getFloatAtIndex (11, argc, argv));
    iemgui_deserializeColors (cast_iem (x), argv + 13, argv + 14, argv + 15);
    //
    }
    
    x->x_gui.iem_owner      = instance_contextGetCurrent();
    x->x_gui.iem_fnDraw     = (t_iemfn)slider_draw;
    x->x_gui.iem_canSend    = symbol_isNil (x->x_gui.iem_send) ? 0 : 1;
    x->x_gui.iem_canReceive = symbol_isNil (x->x_gui.iem_receive) ? 0 : 1;
    x->x_gui.iem_labelX     = labelX;
    x->x_gui.iem_labelY     = labelY;
    x->x_gui.iem_fontSize   = labelFontSize;

    slider_setHeight (x, height);
    slider_setWidth (x, width);
    
    iemgui_checkSendReceiveLoop (cast_iem (x));
    
    if (x->x_gui.iem_canReceive) { pd_bind (cast_pd (x), x->x_gui.iem_receive); }
    
    if (x->x_gui.iem_loadbang) { x->x_position = (int)position; }
    else {
        x->x_position = 0;
    }
    
    x->x_isLogarithmic   = (isLogarithmic != 0);
    x->x_isSteadyOnClick = (isSteady != 0);
    
    slider_setRange (x, minimum, maximum);
        
    x->x_floatValue = slider_getValue (x);
    
    x->x_outlet = outlet_newFloat (cast_object (x));
    //
    }
    
    return x;
}

static void slider_free (t_slider *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_pd (x), x->x_gui.iem_receive); }
        
    stub_destroyWithKey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void slider_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_hslider,
            (t_newmethod)slider_new,
            (t_method)slider_free,
            sizeof (t_slider),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);

    class_addCreator ((t_newmethod)slider_new, sym_vslider, A_GIMME, A_NULL);

    class_addBang (c, (t_method)slider_bang);
    class_addFloat (c, (t_method)slider_float);
    class_addClick (c, (t_method)slider_click);
    class_addLoadbang (c, (t_method)slider_loadbang);
    
    class_addMethod (c, (t_method)slider_initialize,            sym_initialize,         A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)slider_fromValue,             sym__valuedialog,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)slider_fromDialog,            sym__iemdialog,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)slider_size,                  sym_size,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_movePosition,          sym_move,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setPosition,           sym_position,           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setBackgroundColor,    sym_backgroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setForegroundColor,    sym_foregroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)slider_range,                 sym_range,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)slider_set,                   sym_set,                A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)slider_steady,                sym_steady,             A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)slider_logarithmic,           sym_logarithmic,        A_NULL);
    class_addMethod (c, (t_method)slider_linear,                sym_linear,             A_NULL);
    class_addMethod (c, (t_method)iemgui_setSend,               sym_send,               A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemgui_setReceive,            sym_receive,            A_DEFSYMBOL, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)slider_initialize,            sym_init,               A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)iemgui_movePosition,          sym_delta,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setPosition,           sym_pos,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_dummy,                 sym_color,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabelPosition,      sym_label_pos,          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabelFont,          sym_label_font,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabel,              sym_label,              A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)slider_logarithmic,           sym_log,                A_NULL);
    class_addMethod (c, (t_method)slider_linear,                sym_lin,                A_NULL);
    
    class_addCreator ((t_newmethod)slider_new, sym_hsl, A_GIMME, A_NULL);
    class_addCreator ((t_newmethod)slider_new, sym_vsl, A_GIMME, A_NULL);
    
    #endif

    class_setWidgetBehavior (c, &slider_widgetBehavior);
    class_setHelpName (c, sym_slider);
    class_setSaveFunction (c, slider_functionSave);
    class_setValueFunction (c, slider_functionValue);
    class_setPropertiesFunction (c, slider_functionProperties);
    
    slider_class = c;
}

void slider_destroy (void)
{
    class_free (slider_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------


/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void canvas_dspPerform (t_glist *glist, int isTopLevel, t_signal **sp)
{
    t_gobj          *y = NULL;
    t_dspcontext    *context = NULL;
    t_outconnect    *connection = NULL;   
    t_linetraverser t;
    
    int m = object_numberOfSignalInlets (cast_object (glist));
    int n = object_numberOfSignalOutlets (cast_object (glist));
    
    context = ugen_graphStart (isTopLevel, sp, m, n);
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
    //
    t_object *o = cast_objectIfPatchable (y);
    if (o && class_hasMethod (pd_class (y), sym_dsp)) { ugen_graphAdd (context, o); }
    //
    }

    canvas_traverseLinesStart (&t, glist);
    
    while (connection = canvas_traverseLinesNext (&t)) {
        if (object_isSignalOutlet (t.tr_srcObject, t.tr_srcIndexOfOutlet)) {
            ugen_graphConnect (context, 
                t.tr_srcObject, 
                t.tr_srcIndexOfOutlet, 
                t.tr_destObject, 
                t.tr_destIndexOfInlet);
        }
    }

    ugen_graphClose (context);
}

void canvas_dsp (t_glist *x, t_signal **sp)
{
    canvas_dspPerform (x, 0, sp);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

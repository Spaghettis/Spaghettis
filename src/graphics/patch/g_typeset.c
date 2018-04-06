
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Changes by Thomas Musil IEM KUG Graz Austria 2001. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../../system/s_utf8.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Typsetting the text in the boxes. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define BOX_MARGIN_LEFT         2
#define BOX_MARGIN_RIGHT        5
#define BOX_MARGIN_TOP          2
#define BOX_MARGIN_BOTTOM       2
#define BOX_MARGIN_HORIZONTAL   (BOX_MARGIN_LEFT + BOX_MARGIN_RIGHT)
#define BOX_MARGIN_VERTICAL     (BOX_MARGIN_TOP  + BOX_MARGIN_BOTTOM)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define BOX_WIDTH_DEFAULT       60
#define BOX_WIDTH_EMPTY         3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _typesethelper {
    int         p_x;
    int         p_y;
    t_fontsize  p_fontSize;
    double      p_fontWidth;
    double      p_fontHeight;
    int         p_numberOfCharacters;
    int         p_widthOfObject;
    int         p_selectionStart;
    int         p_selectionEnd;
    int         p_widthInPixels; 
    int         p_heightInPixels;
    int         p_lineLengthInCharacters;
    int         p_indexOfMouse;
    int         p_numberOfLines;
    int         p_numberOfColumns;
    int         p_charactersThatRemains;
    int         p_charactersToConsider;
    int         p_bytesToConsider;
    int         p_charactersUntilWrap;
    int         p_bytesUntilWrap;
    int         p_accumulatedOffset;
    int         p_eatCharacter;
    int         p_headIndex;
    int         p_typesetIndex;
    int         p_typesetSize;
    char        *p_typeset;
    char        *p_head;
    } t_typesethelper;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_typesethelper *box_typesetAllocate (t_box *x, int a, int b, t_typesethelper *p)
{
    /* Allocate enough space to insert as many line break as it could be required. */
    
    int size = PD_MAX (BOX_WIDTH_EMPTY, (2 * x->box_stringSizeInBytes)) + 1;
    
    p->p_x                      = a;
    p->p_y                      = b;
    p->p_fontSize               = glist_getFontSize (x->box_owner);
    p->p_fontWidth              = font_getHostFontWidth (p->p_fontSize);
    p->p_fontHeight             = font_getHostFontHeight (p->p_fontSize);
    p->p_numberOfCharacters     = u8_charnum (x->box_string, x->box_stringSizeInBytes);
    p->p_widthOfObject          = object_getWidth (x->box_object);
    p->p_selectionStart         = 0;
    p->p_selectionEnd           = 0;
    p->p_widthInPixels          = 0; 
    p->p_heightInPixels         = 0;
    p->p_lineLengthInCharacters = (p->p_widthOfObject ? p->p_widthOfObject : BOX_WIDTH_DEFAULT);
    p->p_indexOfMouse           = -1;
    p->p_numberOfLines          = 0;
    p->p_numberOfColumns        = 0;
    p->p_charactersThatRemains  = 0;
    p->p_charactersToConsider   = 0;
    p->p_bytesToConsider        = 0;
    p->p_charactersUntilWrap    = 0;
    p->p_bytesUntilWrap         = 0;
    p->p_accumulatedOffset      = 0;
    p->p_eatCharacter           = 0;    /* Remove the character used to wrap (i.e. space and line break). */
    p->p_headIndex              = 0;
    p->p_typesetIndex           = 0;
    p->p_typesetSize            = size;
    p->p_typeset                = (char *)PD_MEMORY_GET (p->p_typesetSize);
    p->p_head                   = NULL;
    
    return p;
}

static void box_typesetFree (t_typesethelper *p)
{
    PD_MEMORY_FREE (p->p_typeset);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Cut string to a given width in case of gatom. */
/* Assumed that the asterisk (0x2A) is a single octet long. */

static void box_typesetEllipsis (t_box *x, t_typesethelper *p)
{
    t_object *o = x->box_object;
    
    if (object_isAtom (o)) {
    //
    if (p->p_widthOfObject > 0) {
    //
    if (p->p_widthOfObject < p->p_numberOfCharacters) {
    //
    int t = u8_offset (x->box_string, p->p_widthOfObject - 1) + 1;
        
    x->box_string = (char *)PD_MEMORY_RESIZE (x->box_string, x->box_stringSizeInBytes, t);
    x->box_stringSizeInBytes = t;
    x->box_string[x->box_stringSizeInBytes - 1] = '*';
    
    /* Note that the string is unzeroed. */
    
    PD_ASSERT (p->p_widthOfObject == u8_charnum (x->box_string, x->box_stringSizeInBytes));
    
    p->p_numberOfCharacters = p->p_widthOfObject;
    //
    }
    //
    }
    //
    }
}

static int box_typesetHasBeenResized (t_box *x, t_typesethelper *p)
{
    if (p->p_widthInPixels != x->box_widthInPixels || p->p_heightInPixels != x->box_heightInPixels) {
    //
    x->box_widthInPixels  = p->p_widthInPixels;
    x->box_heightInPixels = p->p_heightInPixels;
        
    return 1;
    //
    }
    
    return 0;
}

/* If possible wrap at a line break or at the first space from the end. */

static void box_typesetWrap (t_box *x, t_typesethelper *p)
{
    p->p_charactersUntilWrap     = 0;
    p->p_bytesUntilWrap          = string_indexOfFirstOccurrenceUntil (p->p_head, "\n", p->p_bytesToConsider);
    p->p_eatCharacter            = 1;
    
    if (p->p_bytesUntilWrap >= 0) { p->p_charactersUntilWrap = u8_charnum (p->p_head, p->p_bytesUntilWrap); }
    else {
    //
    if (p->p_charactersThatRemains > p->p_lineLengthInCharacters) {
    //
    p->p_bytesUntilWrap = string_indexOfFirstOccurrenceFrom (p->p_head, " ", p->p_bytesToConsider + 1);
    
    if (p->p_bytesUntilWrap >= 0) { p->p_charactersUntilWrap = u8_charnum (p->p_head, p->p_bytesUntilWrap); } 
    else {
        p->p_charactersUntilWrap = p->p_charactersToConsider;
        p->p_bytesUntilWrap      = p->p_bytesToConsider;
        p->p_eatCharacter        = 0;
    }
    //
    } else {
        p->p_charactersUntilWrap = p->p_charactersThatRemains;
        p->p_bytesUntilWrap      = x->box_stringSizeInBytes - p->p_headIndex;
        p->p_eatCharacter        = 0;
    }
    //
    }
}

/* Get the position of the mouse in the box string. */
/* Note that it is an approximation computed with font specifications. */
/* It work best with monospace fonts. */

static void box_typesetLocateCursor (t_box *x, t_typesethelper *p)
{
    int a = (int)((p->p_y / p->p_fontHeight));
    int b = (int)((p->p_x / p->p_fontWidth) + 0.5);
    
    if (p->p_numberOfLines == a) {
    //
    p->p_indexOfMouse = p->p_headIndex + u8_offset (p->p_head, PD_CLAMP (b, 0, p->p_charactersUntilWrap));
    //
    }
}

/* Update extremities of selection points according to cursor. */

static void box_typesetUpdateSelection (t_box *x, t_typesethelper *p)
{
    if (x->box_selectionStart >= p->p_headIndex) {         
        if (x->box_selectionStart <= p->p_headIndex + p->p_bytesUntilWrap + p->p_eatCharacter) {
            p->p_selectionStart = p->p_accumulatedOffset + x->box_selectionStart;
        }
    }
            
    if (x->box_selectionEnd >= p->p_headIndex) {
        if (x->box_selectionEnd <= p->p_headIndex + p->p_bytesUntilWrap + p->p_eatCharacter) {
            p->p_selectionEnd = p->p_accumulatedOffset + x->box_selectionEnd;
        }
    }
}

static void box_typesetAppendLine (t_box *x, t_typesethelper *p)
{
    PD_ASSERT ((p->p_typesetIndex + p->p_bytesUntilWrap) < (p->p_typesetSize - 1));
    
    strncpy (p->p_typeset + p->p_typesetIndex, p->p_head, p->p_bytesUntilWrap);
        
    p->p_typesetIndex          += p->p_bytesUntilWrap;
    p->p_headIndex             += p->p_eatCharacter + p->p_bytesUntilWrap;
    p->p_charactersThatRemains -= p->p_eatCharacter + p->p_charactersUntilWrap;
    
    if (p->p_headIndex < x->box_stringSizeInBytes) {
        p->p_typeset[p->p_typesetIndex++] = '\n'; 
    }
    
    if (p->p_charactersUntilWrap > p->p_numberOfColumns) { 
        p->p_numberOfColumns = p->p_charactersUntilWrap; 
    }
        
    p->p_numberOfLines++;
}

static void box_typesetSlice (t_box *x, t_typesethelper *p)
{
    p->p_head                 = x->box_string + p->p_headIndex;
    p->p_charactersToConsider = PD_MIN (p->p_lineLengthInCharacters, p->p_charactersThatRemains);
    p->p_bytesToConsider      = u8_offset (p->p_head, p->p_charactersToConsider);
    p->p_accumulatedOffset    = p->p_typesetIndex - p->p_headIndex;
    
    box_typesetWrap (x, p);
    box_typesetLocateCursor (x, p);
    box_typesetUpdateSelection (x, p);
    box_typesetAppendLine (x, p);
}

/* An ending backslash breaks the interpreter. */

static char *box_typesetChecked (t_typesethelper *p)
{
    int n = PD_MAX (0, p->p_typesetIndex - 1);
    
    if (p->p_typeset[n] == '\\') {
    //
    p->p_typeset[n] = '/';
    //
    }
    
    return p->p_typeset;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int box_typeset (t_box *x, t_typesethelper *p)
{
    box_typesetEllipsis (x, p);
    
    p->p_charactersThatRemains = p->p_numberOfCharacters;
    
    while (p->p_charactersThatRemains > 0) {     /* Append line break to fit the width. */
        box_typesetSlice (x, p);    
    }
    
    if (p->p_numberOfLines < 1) { p->p_numberOfLines = 1; }
    if (p->p_indexOfMouse < 0)  { p->p_indexOfMouse  = p->p_typesetIndex; }
    
    /* Force the number of columns if the width is provided. */
    /* Otherwise append spaces to ensure a minimum size. */
    
    if (p->p_widthOfObject) { p->p_numberOfColumns = p->p_widthOfObject; } 
    else {
        while (p->p_numberOfColumns < BOX_WIDTH_EMPTY) { 
            p->p_typeset[p->p_typesetIndex++] = ' '; p->p_numberOfColumns++;
        }
    }
    
    p->p_widthInPixels  = (int)(BOX_MARGIN_HORIZONTAL + (p->p_numberOfColumns * p->p_fontWidth));
    p->p_heightInPixels = (int)(BOX_MARGIN_VERTICAL + (p->p_numberOfLines * p->p_fontHeight));
    
    p->p_typeset[p->p_typesetIndex] = 0;
    
    return p->p_indexOfMouse;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void box_sendCreate (t_box *x, t_typesethelper *p)
{
    t_glist *glist = glist_getView (x->box_owner);
    int isSelected = glist_objectIsSelected (x->box_owner, cast_gobj (x->box_object));
    
    gui_vAdd ("::ui_box::newText %s.c %s %d %d {%s} %d #%06x\n",        // --
                    glist_getTagAsString (glist),
                    x->box_tag,
                    (int)(glist_getPixelX (x->box_owner, x->box_object) + BOX_MARGIN_LEFT), 
                    (int)(glist_getPixelY (x->box_owner, x->box_object) + BOX_MARGIN_TOP),
                    box_typesetChecked (p),
                    font_getHostFontSize (p->p_fontSize),
                    (isSelected ? COLOR_SELECTED : COLOR_NORMAL));
}

static void box_sendUpdate (t_box *x, t_typesethelper *p)
{
    t_glist *glist = glist_getView (x->box_owner);
    
    gui_vAdd ("::ui_box::setText %s.c %s {%s}\n",                       // --
                    glist_getTagAsString (glist),
                    x->box_tag,
                    box_typesetChecked (p));
                
    if (x->box_isActivated) {
    
        if (p->p_selectionStart < p->p_selectionEnd) {
        
            gui_vAdd ("%s.c select from %s %d\n",
                            glist_getTagAsString (glist), 
                            x->box_tag,
                            u8_charnum (x->box_string, p->p_selectionStart));
            gui_vAdd ("%s.c select to %s %d\n",
                            glist_getTagAsString (glist), 
                            x->box_tag,
                            u8_charnum (x->box_string, p->p_selectionEnd) - 1);
            gui_vAdd ("%s.c focus \"\"\n",
                            glist_getTagAsString (glist));
            
        } else {
        
            gui_vAdd ("%s.c select clear\n",
                            glist_getTagAsString (glist));
            gui_vAdd ("%s.c icursor %s %d\n",
                            glist_getTagAsString (glist),
                            x->box_tag,
                            u8_charnum (x->box_string, p->p_selectionStart));
            gui_vAdd ("%s.c focus %s\n",
                            glist_getTagAsString (glist),
                            x->box_tag);        
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int box_send (t_box *x, int action, int a, int b)
{
    t_typesethelper p;
    
    int indexOfMouse = box_typeset (x, box_typesetAllocate (x, a, b, &p));
    int resized      = box_typesetHasBeenResized (x, &p);
    
    if (action == BOX_CHECK)       { x->box_checked = 1;     }   /* Required only once at creation time. */
    else if (action == BOX_CREATE) { box_sendCreate (x, &p); }
    else if (action == BOX_UPDATE) { 
    
        if (resized) { box_update (x); }
        
        box_sendUpdate (x, &p);     
    }

    box_typesetFree (&p);
    
    return indexOfMouse;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

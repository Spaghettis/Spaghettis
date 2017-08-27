
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define BUFFER_PREALLOCATED_ATOMS       64

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

enum {
    FLOAT_STATE_ERROR                   = -1,
    FLOAT_STATE_START                   = 0,
    FLOAT_STATE_MINUS                   = 1,
    FLOAT_STATE_INTEGER_DIGIT,
    FLOAT_STATE_LEADING_DOT,
    FLOAT_STATE_DOT,
    FLOAT_STATE_FRACTIONAL_DIGIT,
    FLOAT_STATE_EXPONENTIAL,
    FLOAT_STATE_EXPONENTIAL_SIGN,
    FLOAT_STATE_EXPONENTIAL_DIGIT
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int buffer_parseIsMalformed (char *s, int size)
{
    if (size < 0) { return 1; }
    else {
        int i;
        for (i = 0; i < size; i++) { if (s[i] == 0) { return 1; } }
    }
    
    return 0;
}

static int buffer_parseIsValidCharacter (char c)
{
    return (!char_isWhitespace (c) && !char_isEnd (c));
}

static int buffer_parseIsValidState (int floatState)
{
    return (floatState == FLOAT_STATE_INTEGER_DIGIT
            || floatState == FLOAT_STATE_DOT
            || floatState == FLOAT_STATE_FRACTIONAL_DIGIT
            || floatState == FLOAT_STATE_EXPONENTIAL_DIGIT);
}

static int buffer_parseNextFloatState (int floatState, char c)
{
    int digit       = (c >= '0' && c <= '9');
    int dot         = (c == '.');
    int minus       = (c == '-');
    int plus        = (c == '+');
    int exponential = (c == 'e' || c == 'E');
    
    int k = floatState;
    
    PD_ASSERT (k != FLOAT_STATE_ERROR);
    
    if (floatState == FLOAT_STATE_START)                    {
        if (minus)                                          { k = FLOAT_STATE_MINUS; }
        else if (digit)                                     { k = FLOAT_STATE_INTEGER_DIGIT; }
        else if (dot)                                       { k = FLOAT_STATE_LEADING_DOT; }
        else {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_MINUS)             {
        if (digit)                                          { k = FLOAT_STATE_INTEGER_DIGIT; }
        else if (dot)                                       { k = FLOAT_STATE_LEADING_DOT; }
        else { 
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_INTEGER_DIGIT)     {
        if (dot)                                            { k = FLOAT_STATE_DOT; }
        else if (exponential)                               { k = FLOAT_STATE_EXPONENTIAL; }
        else if (!digit) {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_LEADING_DOT)       {
        if (digit)                                          { k = FLOAT_STATE_FRACTIONAL_DIGIT; }
        else {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_DOT)               {
        if (digit)                                          { k = FLOAT_STATE_FRACTIONAL_DIGIT; }
        else if (exponential)                               { k = FLOAT_STATE_EXPONENTIAL; }
        else {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_FRACTIONAL_DIGIT)  {
        if (exponential)                                    { k = FLOAT_STATE_EXPONENTIAL; }
        else if (!digit) {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_EXPONENTIAL)       {
        if (plus || minus)                                  { k = FLOAT_STATE_EXPONENTIAL_SIGN; }
        else if (digit)                                     { k = FLOAT_STATE_EXPONENTIAL_DIGIT; }
        else {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_EXPONENTIAL_SIGN)  {
        if (digit)                                          { k = FLOAT_STATE_EXPONENTIAL_DIGIT; }
        else {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_EXPONENTIAL_DIGIT) {
        if (!digit) {
            k = FLOAT_STATE_ERROR;
        }
    }

    return k;
}

static void buffer_parseStringUnzeroed (t_buffer *x, char *s, int size, int preallocated)
{
    int length = 0;
    t_atom *a = NULL;
    
    const char *text = s;
    const char *tBound = s + size;

    if (buffer_parseIsMalformed (s, size)) { PD_BUG; return; }
    
    PD_MEMORY_FREE (x->b_vector);
    x->b_vector = (t_atom *)PD_MEMORY_GET (preallocated * sizeof (t_atom));
    a = x->b_vector;
    x->b_size = length;     /* Inconsistency corrected later. */
    
    while (1) {
    //
    while (char_isWhitespace (*text) && (text != tBound)) { text++; }         /* Skip whitespaces. */
    
    if (text == tBound)    { break; }
    else if (*text == ';') { SET_SEMICOLON (a); text++; }
    else if (*text == ',') { SET_COMMA (a);     text++; }
    else {
        
        char buffer[PD_STRING + 1] = { 0 };
        char *p = buffer;
        char *pBound = buffer + PD_STRING;
        
        int floatState = 0;
        int slash = 0;
        int lastSlash = 0;
        int dollar = 0;
        
        do {
        //
        char c = *p = *text++;
        
        lastSlash = slash; slash = char_isEscape (c);

        if (floatState >= 0) { floatState = buffer_parseNextFloatState (floatState, c); }
        if (!lastSlash && text != tBound && string_startWithOneDollarAndOneNumber (text - 1)) { dollar = 1; }
        
        if (!slash)         { p++; }
        else if (lastSlash) { p++; slash = 0; }
        //
        } while (text != tBound && p != pBound && (slash || (buffer_parseIsValidCharacter (*text))));
                
        *p = 0;

        if (buffer_parseIsValidState (floatState)) {
            SET_FLOAT (a, (t_float)atof (buffer));
                        
        } else if (dollar) {
            if (string_containsOneDollarFollowingByNumbers (buffer)) { SET_DOLLAR (a, atoi (buffer + 1)); }
            else { 
                SET_DOLLARSYMBOL (a, gensym (buffer));
            }
            
        } else {
            SET_SYMBOL (a, gensym (buffer));
        }
    }

    a++;
    length++;
    
    if (length == preallocated) {
        size_t oldSize = preallocated * sizeof (t_atom);
        x->b_vector = (t_atom *)PD_MEMORY_RESIZE (x->b_vector, oldSize, oldSize * 2);
        preallocated = preallocated * 2;
        a = x->b_vector + length;
    }
    //
    }
    
    /* Crop to truly used memory. */
    
    {
        int oldSize = preallocated * sizeof (t_atom);
        int newSize = length * sizeof (t_atom);
        x->b_size   = length;
        x->b_vector = (t_atom *)PD_MEMORY_RESIZE (x->b_vector, oldSize, newSize);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void buffer_withStringUnzeroed (t_buffer *x, char *s, int size)
{
    buffer_parseStringUnzeroed (x, s, size, BUFFER_PREALLOCATED_ATOMS);
}

void buffer_toStringUnzeroed (t_buffer *x, char **s, int *size)     /* Caller acquires string ownership. */
{
    char *buffer = (char *)PD_MEMORY_GET (0);
    int i, length = 0;

    for (i = 0; i < x->b_size; i++) {
    //
    int n;
    t_error err;
    char t[PD_STRING] = { 0 };
    t_atom *a = x->b_vector + i;
    
    /* Remove whitespace before a semicolon or a comma for cosmetic purpose. */
    
    if (IS_SEMICOLON_OR_COMMA (a)) {
    //
    if (length && buffer[length - 1] == ' ') { 
        buffer = (char *)PD_MEMORY_RESIZE (buffer, length, length - 1); length--;
    }
    //
    }
    
    err = atom_toString (a, t, PD_STRING); PD_UNUSED (err); PD_ASSERT (!err);
    
    n = (int)(strlen (t) + 1);
    
    if (length > (PD_INT_MAX - n)) { PD_BUG; }
    else {
        buffer = (char *)PD_MEMORY_RESIZE (buffer, length, length + n);
        strcpy (buffer + length, t);
        length += n;
    }
    
    if (IS_SEMICOLON (a)) { buffer[length - 1] = '\n'; }
    else { 
        buffer[length - 1] = ' ';
    }
    //
    }
    
    /* Remove ending whitespace. */
    
    if (length && buffer[length - 1] == ' ') { 
        buffer = (char *)PD_MEMORY_RESIZE (buffer, length, length - 1); length--;
    }
    
    *s = buffer;
    *size = length;
}

char *buffer_toString (t_buffer *x)
{
    char *s = NULL;
    int n, length = 0;
    
    buffer_toStringUnzeroed (x, &s, &length);
    n = length + 1; 
    s = (char *)PD_MEMORY_RESIZE (s, length, n);
    s[n - 1] = 0;
    
    return s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int buffer_isLastMessageProperlyEnded (t_buffer *x)
{
    if (x->b_size) { return IS_SEMICOLON_OR_COMMA (&x->b_vector[x->b_size - 1]); }
    else {
        return 1;
    }
}

int buffer_getNumberOfMessages (t_buffer *x)
{
    int i, count = 0;

    for (i = 0; i < x->b_size; i++) {
        if (IS_SEMICOLON_OR_COMMA (&x->b_vector[i])) { count++; }
    }
    
    if (!buffer_isLastMessageProperlyEnded (x)) { count++; }
        
    return count;
}

t_error buffer_getMessageAt (t_buffer *x, int n, int *start, int *end)
{
    *start = 0; *end = 0;
    
    if (n >= 0) {
    //
    int i, k = 0;
    
    for (i = 0; i < x->b_size; i++) {
    //
    if (k != n) { if (IS_SEMICOLON_OR_COMMA (&x->b_vector[i])) { k++; } }
    else {
        int j = i;
        while (j < x->b_size && !IS_SEMICOLON_OR_COMMA (&x->b_vector[j])) { j++; }
        *start = i;
        *end   = j;
        return PD_ERROR_NONE;
    }
    //
    }
    //
    }
    
    return PD_ERROR;
}

t_error buffer_getMessageAtWithTypeOfEnd (t_buffer *x, int n, int *start, int *end, t_atomtype *type)
{
    t_error err = buffer_getMessageAt (x, n, start, end);
    
    if (!err) {
    //
    if (buffer_getAtomAtIndexChecked (x, *end)) { *type = atom_getType (buffer_getAtomAtIndex (x, *end)); }
    else {
        *type = A_NULL;
    }
    //
    }
    
    return err;
}
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void buffer_serialize (t_buffer *x, t_buffer *y)
{
    t_buffer *copy = buffer_new();
    int i;

    buffer_appendBuffer (copy, y);
    
    for (i = 0; i < copy->b_size; i++) {
    //
    t_atom *a = copy->b_vector + i;
    
    PD_ASSERT (!IS_POINTER (a));
    
    if (!IS_FLOAT (a)) {
        char t[PD_STRING] = { 0 };
        t_error err = atom_toString (a, t, PD_STRING);
        PD_UNUSED (err); PD_ASSERT (!err);
        SET_SYMBOL (a, gensym (t));
    }
    //
    }
    
    buffer_appendBuffer (x, copy);
    buffer_free (copy);
}

void buffer_deserialize (t_buffer *x, int argc, t_atom *argv)
{
    int i, n = x->b_size + argc;

    PD_ASSERT (argc >= 0);
    
    x->b_vector = (t_atom *)PD_MEMORY_RESIZE (x->b_vector, x->b_size * sizeof (t_atom), n * sizeof (t_atom));
    
    for (i = 0; i < argc; i++) {
    //
    t_atom *a = x->b_vector + x->b_size + i;
    
    if (!IS_SYMBOL (argv + i)) { *a = *(argv + i); }
    else {
        char *s = GET_SYMBOL (argv + i)->s_name;
        t_error err = atom_withStringUnzeroed (a, s, (int)strlen (s));
        PD_UNUSED (err); PD_ASSERT (!err);
    }
    //
    }
    
    x->b_size = n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

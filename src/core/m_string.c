
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error string_appendProceed (char *dest, size_t size, const char *src, int n)
{
    size_t d = strlen (dest);
    size_t k = (size - 1) - d;
    size_t s = 0;
    
    PD_ASSERT (n >= -1);
    PD_ASSERT (size > d);
    
    if (n < 0) { s = strlen (src); }
    else {
        const char *t = src; while (*t && s < (size_t)n) { s++; t++; }
    }
    
    strncat (dest, src, PD_MIN (s, k));
    
    if (s <= k) { return PD_ERROR_NONE; }
    else {
        return PD_ERROR;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error string_copy (char *dest, size_t size, const char *src)
{
    size_t s = strlen (src);
    
    PD_ASSERT (size > 0);
    
    strncpy (dest, src, PD_MIN (s, size));
    dest[PD_MIN (size - 1, s)] = 0;
    
    if (s < size) { return PD_ERROR_NONE; }
    else {
        return PD_ERROR;
    }
}

t_error string_add (char *dest, size_t size, const char *src)
{
    return string_appendProceed (dest, size, src, -1);
}

t_error string_append (char *dest, size_t size, const char *src, int n)
{
    if (n < 0) { PD_BUG; return PD_ERROR; }
    else {
        return string_appendProceed (dest, size, src, n);
    }
}

t_error string_sprintf (char *dest, size_t size, const char *format, ...)
{
    int t;
    va_list args;
    
    va_start (args, format);
    t = vsnprintf (dest, size, format, args);
    va_end (args);
    
    if (t >= 0 && (size_t)t < size) { return PD_ERROR_NONE; }
    else {
        return PD_ERROR;
    }
}

t_error string_addSprintf (char *dest, size_t size, const char *format, ...)
{
    int t;
    va_list args;
    size_t d = strlen (dest);
    
    PD_ASSERT (size > d);
    
    va_start (args, format);
    t = vsnprintf (dest + d, size - d, format, args);
    va_end (args);
    
    if (t >= 0 && (size_t)t < (size - d)) { return PD_ERROR_NONE; }
    else {
        return PD_ERROR;
    }
}

t_error string_addAtom (char *dest, size_t size, t_atom *a)
{
    t_error err = PD_ERROR_NONE;
    
    char *t = (char *)PD_MEMORY_GET (size);
    
    err |= atom_toString (a, t, (int)size);
    err |= string_add (dest, size, t);

    PD_MEMORY_FREE (t);
    
    return err;
}

t_error string_clear (char *dest, size_t size)
{
    size_t i; for (i = 0; i < size; i++) { dest[i] = 0; } return PD_ERROR_NONE;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error string_escapeCharacter (char *dest, size_t size, char c)
{
    t_error err = PD_ERROR_NONE;
    
    char *t = (char *)PD_MEMORY_GET (size * 2);
    char *r = dest;
    char *w = t;
    
    while (*r) {
        if (*r == c) { *w = '\\'; w++; }
        *w = *r;
        w++;
        r++;
    }
    
    *w = 0;
    
    err = string_copy (dest, size, t);
    
    PD_MEMORY_FREE (t);
    
    return err;
}

t_error string_escapeOccurrence (char *dest, size_t size, const char *chars)
{
    t_error err = PD_ERROR_NONE;
    
    PD_ASSERT (chars);
    
    while (*chars) { err |= string_escapeCharacter (dest, size, *chars); chars++; }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* < http://www.cse.yorku.ca/~oz/hash.html > */

t_unique string_hash (const char *s)
{
    t_unique hash = 5381;
    int c;
    
    while ((c = *s++)) { hash = ((hash << 5) + hash) + c; }
    
    return hash;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int string_startWith (const char *s, const char *isStartWith)
{
    size_t n = strlen (isStartWith);
    
    if (strlen (s) >= n) { return (strncmp (s, isStartWith, n) == 0); }
    
    return 0;
}

int string_endWith (const char *s, const char *isEndWith)
{
    size_t n = strlen (isEndWith);
    size_t m = strlen (s);
    
    if (m >= n) { return (strncmp (s + (m - n), isEndWith, n) == 0); }
    
    return 0;
}

int string_containsOccurrenceAtStart (const char *s, const char *chars)
{
    return (strchr (chars, *s) != NULL);
}

int string_containsOccurrence (const char *s, const char *chars)
{
    return (string_indexOfFirstOccurrenceFromEnd (s, chars) >= 0);
}

int string_contains (const char *s, const char *isContained)
{
    return (strstr (s, isContained) != NULL);
}

void string_getNumberOfColumnsAndLines (const char *s, int *numberOfColumns, int *numberOfLines)
{
    const char *end = NULL;
    const char *start = NULL;
    int m = 0;
    int n = 1;
        
    for ((start = s); (end = strchr (start, '\n')); (start = end + 1)) {
        ptrdiff_t t = end - start;
        int size = (int)(PD_MIN (t, PD_INT_MAX));
        m = PD_MAX (m, size);
        n++; 
    }
    
    {
        size_t t = strlen (start);
        int size = (int)(PD_MIN (t, PD_INT_MAX));
        m = PD_MAX (m, size);
    }
        
    *numberOfColumns = m;
    *numberOfLines   = n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int string_indexOfFirstCharacterUntil (const char *s, char c, int n)
{
    const char *s2 = s + n;
    
    int i = 0;
    
    while (s != s2) {
        if (*s == c) { return i; } 
        i++; 
        s++;
    }
    
    return -1;
}

static int string_indexOfFirstCharacterFrom (const char *s, char c, int n)
{
    const char *s2 = s + n;
    
    while (s2 != s) { 
        s2--;
        n--;
        if (*s2 == c) { return n; }
    }
    
    return -1;
}

int string_indexOfFirstOccurrenceUntil (const char *s, const char *c, int n)
{
    int k = n;
    int t = 0;
    
    while (*c != 0) { 
        t = string_indexOfFirstCharacterUntil (s, *c, n);
        if ((t >= 0) && (t < k)) { k = t; }
        c++; 
    }
    
    return (k < n ? k : -1);
}

int string_indexOfFirstOccurrenceFrom (const char *s, const char *c, int n)
{
    int k = -1;
    int t = 0;
    
    while (*c != 0) { 
        t = string_indexOfFirstCharacterFrom (s, *c, n);
        if (t > k) { k = t; }
        c++; 
    }
    
    return k;
}

int string_indexOfFirstOccurrenceFromEnd (const char *s, const char *c)
{
    return string_indexOfFirstOccurrenceFrom (s, c, (int)strlen (s));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void string_replaceCharacter (char *s, char toBeReplaced, char c)
{
    PD_ASSERT (c && toBeReplaced);
    
    while (*s) { if (*s == toBeReplaced) { *s = c; } s++; }
}

/* The string must NOT be a static constant. */

void string_removeCharacter (char *s, char toBeRemoved)
{
    PD_ASSERT (toBeRemoved);
    
    char *r = s;
    char *w = s;
    
    while (*r) { *w = *r; r++; w += (*w != toBeRemoved); }
    
    *w = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------


/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __s_time_h_
#define __s_time_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef uint64_t t_time;
typedef uint64_t t_nano;
typedef uint64_t t_stamp;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        time_set                    (t_time *t);
void        time_addNanoseconds         (t_time *t, t_nano ns);
t_error     time_elapsedNanoseconds     (const t_time *t0, const t_time *t1, t_nano *elapsed);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        nano_sleep                  (t_nano ns);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://en.wikipedia.org/wiki/Network_Time_Protocol > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define     STAMP_TAGS_SIZE             16

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        stamp_set                   (t_stamp *stamp);
void        stamp_addNanoseconds        (t_stamp *stamp, t_nano ns);
t_error     stamp_elapsedNanoseconds    (const t_stamp *t0, const t_stamp *t1, t_nano *elapsed);

int         stamp_isTag                 (t_symbol *s);
t_error     stamp_setAsTags             (int argc, t_atom *argv, t_stamp *stamp);
t_error     stamp_getWithTags           (int argc, t_atom *argv, t_stamp *stamp);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void stamp_setImmediately (t_stamp *stamp)
{
    (*stamp) = 1ULL;
}

static inline int stamp_isImmediately (t_stamp *stamp)
{
    return ((*stamp) == 1ULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_time_h_


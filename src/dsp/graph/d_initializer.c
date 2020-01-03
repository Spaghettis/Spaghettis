
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"
#include "../../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void chain_addInitializer (t_chain *, t_initializer *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void initializer_proceed (t_initializer *x)
{
    (*x->s_fn) (x->s_lhs, x->s_rhs);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_initializer *initializer_new (t_initializerfn fn, void *lhs, void *rhs)
{
    t_initializer *x = (t_initializer *)PD_MEMORY_GET (sizeof (t_initializer));
    
    x->s_lhs = lhs;
    x->s_rhs = rhs;
    x->s_fn  = fn;
    
    chain_addInitializer (instance_chainGetTemporary(), x);

    return x;
}

void initializer_free (t_initializer *x)
{
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------


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
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pdinstance     *pd_this;
extern t_class          *canvas_class;
extern t_class          *vinlet_class; 
extern t_class          *voutlet_class;
extern t_class          *block_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_dspcontext     *ugen_context;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int ugen_dspPhase;                       /* Shared. */
static int ugen_buildIdentifier;                /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _sigoutconnect {
    int                     oc_index;
    struct _ugenbox         *oc_to;
    struct _sigoutconnect   *oc_next;
    } t_sigoutconnect;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _sigoutlet {
    int                     o_numberOfConnections;
    t_sigoutconnect         *o_connections;
    t_signal                *o_signal;
    } t_sigoutlet;
    
typedef struct _siginlet {
    int                     i_numberOfConnections;
    int                     i_numberConnected;
    t_signal                *i_signal;
    } t_siginlet;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _ugenbox {
    int                     u_done;
    int                     u_phase;
    int                     u_inSize;
    int                     u_outSize;
    t_siginlet              *u_in;
    t_sigoutlet             *u_out;
    t_object                *u_owner;
    struct _ugenbox         *u_next;
    } t_ugenbox;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _dspcontext {
    int                     dc_numberOfInlets;
    int                     dc_numberOfOutlets;
    t_float                 dc_sampleRate;
    int                     dc_blockSize;
    int                     dc_isTopLevel;
    int                     dc_isReblocked;
    int                     dc_isSwitch;
    t_ugenbox               *dc_ugens;
    struct _dspcontext      *dc_parentContext;
    t_signal                **dc_ioSignals;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void ugen_graphDelete (t_dspcontext *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void ugen_dspInitialize (void)
{
    ugen_dspRelease();
    
    PD_ASSERT (ugen_context == NULL);
    
    pd_this->pd_dspChainSize = 1;
    pd_this->pd_dspChain     = (t_int *)PD_MEMORY_GET (pd_this->pd_dspChainSize * sizeof (t_int));
    pd_this->pd_dspChain[0]  = (t_int)dsp_done;
    
    ugen_buildIdentifier++;
}

void ugen_dspTick (void)
{
    if (pd_this->pd_dspChain) {
    //
    t_int *t = pd_this->pd_dspChain; while (t) { t = (*(t_perform)(*t))(t); } ugen_dspPhase++;
    //
    }
}

void ugen_dspRelease (void)
{
    if (pd_this->pd_dspChain) {
    //
    PD_MEMORY_FREE (pd_this->pd_dspChain); pd_this->pd_dspChain = NULL;
    //
    }
    
    signal_clean();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int ugen_getBuildIdentifier (void)
{
    return ugen_buildIdentifier;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

    /* get the index of a ugenbox or -1 if it's not on the list */
static int ugen_index(t_dspcontext *dc, t_ugenbox *x)
{
    int ret;
    t_ugenbox *u;
    for (u = dc->dc_ugens, ret = 0; u; u = u->u_next, ret++)
        if (u == x) return (ret);
    return (-1);
}

    /* put a ugenbox on the chain, recursively putting any others on that
    this one might uncover. */
static void ugen_doit(t_dspcontext *dc, t_ugenbox *u)
{
    t_sigoutlet *uout;
    t_siginlet *uin;
    t_sigoutconnect *oc, *oc2;
    t_class *class = pd_class(&u->u_owner->te_g.g_pd);
    int i, n;
        /* suppress creating new signals for the outputs of signal
        inlets and subpatchs; except in the case we're an inlet and "blocking"
        is set.  We don't yet know if a subcanvas will be "blocking" so there
        we delay new signal creation, which will be handled by calling
        signal_borrow in the ugen_graphClose routine below. */
    int nonewsigs = (class == canvas_class || 
        (class == vinlet_class) && !(dc->dc_isReblocked));
        /* when we encounter a subcanvas or a signal outlet, suppress freeing
        the input signals as they may be "borrowed" for the super or sub
        patch; same exception as above, but also if we're "switchable" we
        have to do a copy rather than a borrow.  */
    int nofreesigs = (class == canvas_class || 
        (class == voutlet_class) &&  !(dc->dc_isReblocked || dc->dc_isSwitch));
    t_signal **insig, **outsig, **sig, *s1, *s2, *s3;
    t_ugenbox *u2;
    
    if (0) post("doit %s %d %d", class_getNameAsString(class), nofreesigs,
        nonewsigs);
    for (i = 0, uin = u->u_in; i < u->u_inSize; i++, uin++)
    {
        if (!uin->i_numberOfConnections)
        {
            t_float *scalar;
            s3 = signal_new(dc->dc_blockSize, dc->dc_sampleRate);
            /* post("%s: unconnected signal inlet set to zero",
                class_getNameAsString(u->u_owner->te_g.g_pd)); */
            if (scalar = object_getSignalValueAtIndex(u->u_owner, i))
                dsp_add_scalarcopy(scalar, s3->s_vector, s3->s_vectorSize);
            else
                dsp_addZeroPerform(s3->s_vector, s3->s_vectorSize);
            uin->i_signal = s3;
            s3->s_count = 1;
        }
    }
    insig = (t_signal **)PD_MEMORY_GET((u->u_inSize + u->u_outSize) * sizeof(t_signal *));
    outsig = insig + u->u_inSize;
    for (sig = insig, uin = u->u_in, i = u->u_inSize; i--; sig++, uin++)
    {
        int newrefcount;
        *sig = uin->i_signal;
        newrefcount = --(*sig)->s_count;
            /* if the reference count went to zero, we free the signal now,
            unless it's a subcanvas or outlet; these might keep the
            signal around to send to objects connected to them.  In this
            case we increment the reference count; the corresponding decrement
            is in sig_makereusable(). */
        if (nofreesigs)
            (*sig)->s_count++;
        else if (!newrefcount)
            signal_free(*sig);
    }
    for (sig = outsig, uout = u->u_out, i = u->u_outSize; i--; sig++, uout++)
    {
            /* similarly, for outlets of subcanvases we delay creating
            them; instead we create "borrowed" ones so that the refcount
            is known.  The subcanvas replaces the fake signal with one showing
            where the output data actually is, to avoid having to copy it.
            For any other object, we just allocate a new output vector;
            since we've already freed the inputs the objects might get called
            "in place." */
        if (nonewsigs)
        {
            *sig = uout->o_signal =
                signal_new(0, dc->dc_sampleRate);
        }
        else
            *sig = uout->o_signal = signal_new(dc->dc_blockSize, dc->dc_sampleRate);
        (*sig)->s_count = uout->o_numberOfConnections;
    }
        /* now call the DSP scheduling routine for the ugen.  This
        routine must fill in "borrowed" signal outputs in case it's either
        a subcanvas or a signal inlet. */
        
    mess1(&u->u_owner->te_g.g_pd, sym_dsp, insig);
    
        /* if any output signals aren't connected to anyone, free them
        now; otherwise they'll either get freed when the reference count
        goes back to zero, or even later as explained above. */

    for (sig = outsig, uout = u->u_out, i = u->u_outSize; i--; sig++, uout++)
    {
        if (!(*sig)->s_count)
            signal_free(*sig);
    }
    if (0)
    {
        if (u->u_inSize + u->u_outSize == 0) post("put %s %d", 
            class_getNameAsString(u->u_owner->te_g.g_pd), ugen_index(dc, u));
        else if (u->u_inSize + u->u_outSize == 1) post("put %s %d (%lx)", 
            class_getNameAsString(u->u_owner->te_g.g_pd), ugen_index(dc, u), sig[0]);
        else if (u->u_inSize + u->u_outSize == 2) post("put %s %d (%lx %lx)", 
            class_getNameAsString(u->u_owner->te_g.g_pd), ugen_index(dc, u),
                sig[0], sig[1]);
        else post("put %s %d (%lx %lx %lx ...)", 
            class_getNameAsString(u->u_owner->te_g.g_pd), ugen_index(dc, u),
                sig[0], sig[1], sig[2]);
    }

        /* pass it on and trip anyone whose last inlet was filled */
    for (uout = u->u_out, i = u->u_outSize; i--; uout++)
    {
        s1 = uout->o_signal;
        for (oc = uout->o_connections; oc; oc = oc->oc_next)
        {
            u2 = oc->oc_to;
            uin = &u2->u_in[oc->oc_index];
                /* if there's already someone here, sum the two */
            if (s2 = uin->i_signal)
            {
                s1->s_count--;
                s2->s_count--;
                //if (!signal_compatible(s1, s2))
                if (!(s1->s_vectorSize == s2->s_vectorSize && s1->s_sampleRate == s2->s_sampleRate))
                {
                    post_error ("%s: incompatible signal inputs",
                        class_getNameAsString(u->u_owner->te_g.g_pd));
                    return;
                }
                s3 = signal_new(s1->s_vectorSize, s1->s_sampleRate);
                //s3 = signal_newlike(s1);
                dsp_addPlusPerform(s1->s_vector, s2->s_vector, s3->s_vector, s1->s_vectorSize);
                uin->i_signal = s3;
                s3->s_count = 1;
                if (!s1->s_count) signal_free(s1);
                if (!s2->s_count) signal_free(s2);
            }
            else uin->i_signal = s1;
            uin->i_numberConnected++;
                /* if we didn't fill this inlet don't bother yet */
            if (uin->i_numberConnected < uin->i_numberOfConnections)
                goto notyet;
                /* if there's more than one, check them all */
            if (u2->u_inSize > 1)
            {
                for (uin = u2->u_in, n = u2->u_inSize; n--; uin++)
                    if (uin->i_numberConnected < uin->i_numberOfConnections) goto notyet;
            }
                /* so now we can schedule the ugen.  */
            ugen_doit(dc, u2);
        notyet: ;
        }
    }
    PD_MEMORY_FREE(insig);
    u->u_done = 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_ugenbox *ugen_graphFetchUgen (t_dspcontext *context, t_object *o)
{
    t_ugenbox *u = NULL; for (u = context->dc_ugens; u && u->u_owner != o; u = u->u_next); return u;
}

static void ugen_graphConnectUgens (t_ugenbox *u1, int m, t_ugenbox *u2, int n)
{
    t_sigoutlet *o = u1->u_out + m;
    t_siginlet  *i = u2->u_in  + n;
    
    t_sigoutconnect *c = (t_sigoutconnect *)PD_MEMORY_GET (sizeof (t_sigoutconnect));
    
    c->oc_to    = u2;
    c->oc_index = n;
    c->oc_next  = o->o_connections;
    
    o->o_connections = c;

    o->o_numberOfConnections++;
    i->i_numberOfConnections++;
}

static t_block *ugen_graphGetBlockIfContainsAny (t_dspcontext *context)
{
    t_block *block = NULL;
    t_ugenbox *u = NULL;
    
    for (u = context->dc_ugens; u; u = u->u_next) {
    //
    t_object *o = u->u_owner;
    
    if (pd_class (o) == block_class) {
        if (block) { error_unexpected (sym_dsp, sym_block__tilde__); }
        else {
            block = (t_block *)o;
        }
    }
    //
    }
    
    return block;
}

void ugen_graphCreateMissingSignalsForOutlets (t_dspcontext *context,
    int switchable, 
    int reblocked, 
    int blockSize,
    t_float sampleRate)
{
    if (context->dc_ioSignals) {
    //
    if (switchable || reblocked) {

        int i;
        t_signal **t = context->dc_ioSignals + context->dc_numberOfInlets;
        
        for (i = 0; i < context->dc_numberOfOutlets; i++) {
        //
        t_signal *s = (*t);
        
        if (s->s_isVectorBorrowed && !s->s_borrowedFrom) {
            signal_borrow (s, signal_new (blockSize, sampleRate));
            s->s_count++;                                                       /* ??? */
        }
        
        t++;
        //
        }
    }
    //
    }
}

void ugen_graphDspProlog (t_dspcontext *context, 
    int switchable,
    int reblocked, 
    int blockSize,
    int period, 
    int frequency,
    int downsample,
    int upsample)
{
    t_ugenbox *u = NULL;
    
    for (u = context->dc_ugens; u; u = u->u_next) {
    //
    t_object *object = u->u_owner;
    
    t_signal **i = context->dc_ioSignals;
    t_signal **o = i ? i + context->dc_numberOfInlets : NULL;

    if (pd_class (object) == vinlet_class) {
    
        vinlet_dspProlog ((t_vinlet *)object, 
            i,
            switchable,
            reblocked,
            blockSize,
            ugen_dspPhase,
            period,
            frequency,
            downsample,
            upsample);
    }
    
    if (pd_class (object) == voutlet_class) {
    
        voutlet_dspProlog ((t_voutlet *)object, 
            o,
            switchable,
            reblocked,
            blockSize,
            ugen_dspPhase,
            period,
            frequency,
            downsample,
            upsample);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Note that an abstraction can be opened as a toplevel patch. */

t_dspcontext *ugen_graphStart (int isTopLevel, t_signal **sp, int m, int n)
{
    t_dspcontext *context = (t_dspcontext *)PD_MEMORY_GET (sizeof (t_dspcontext));

    context->dc_numberOfInlets  = isTopLevel ? 0 : m;
    context->dc_numberOfOutlets = isTopLevel ? 0 : n;
    context->dc_isTopLevel      = isTopLevel;
    context->dc_ugens           = NULL;
    context->dc_parentContext   = ugen_context;
    context->dc_ioSignals       = sp;
        
    ugen_context = context;
    
    return context;
}

void ugen_graphAdd (t_dspcontext *context, t_object *o)
{
    t_ugenbox *x = (t_ugenbox *)PD_MEMORY_GET (sizeof (t_ugenbox));

    x->u_inSize  = object_numberOfSignalInlets (o);
    x->u_outSize = object_numberOfSignalOutlets (o);
    x->u_in      = PD_MEMORY_GET (x->u_inSize * sizeof (t_siginlet));
    x->u_out     = PD_MEMORY_GET (x->u_outSize * sizeof (t_sigoutlet));
    x->u_owner   = o;
    x->u_next    = context->dc_ugens;
    
    context->dc_ugens = x;
}

void ugen_graphConnect (t_dspcontext *context, t_object *o1, int m, t_object *o2, int n)
{
    t_ugenbox *u1 = ugen_graphFetchUgen (context, o1);
    t_ugenbox *u2 = ugen_graphFetchUgen (context, o2);
    
    m = object_indexAsSignalOutlet (o1, m);
    n = object_indexAsSignalInlet (o2, n);
    
    if (!u1 || !u2 || n < 0) { PD_BUG; }
    else if (m < 0 || m >= u1->u_outSize || n >= u2->u_inSize) { PD_BUG; }
    else {
        ugen_graphConnectUgens (u1, m, u2, n);
    }
}

/* Period is roughly the number of parent's blocks required to filled the child. */
/* Frequency is roughly the number of child's iterations required to equaled the parent. */
/* Note that it is respectively divided and multiplied in case of overlap. */

void ugen_graphClose (t_dspcontext *context)
{
    t_dspcontext *parentContext = context->dc_parentContext;
    t_float parentSampleRate    = parentContext ? parentContext->dc_sampleRate : audio_getSampleRate();
    int parentBlockSize         = parentContext ? parentContext->dc_blockSize  : AUDIO_DEFAULT_BLOCKSIZE;
    t_float sampleRate          = parentSampleRate;
    int blockSize               = parentBlockSize;
    int period                  = 1;
    int frequency               = 1;
    int downsample              = 1;
    int upsample                = 1;
    int switchable              = 0;
    int reblocked               = parentContext ? 0 : 1;
    
    t_block *block = ugen_graphGetBlockIfContainsAny (context);
        
    if (block) {
    //
    block_getParameters (block,
        &switchable,
        &reblocked,
        &blockSize,
        &sampleRate,
        &period,
        &frequency,
        &downsample,
        &upsample,
        ugen_dspPhase,
        parentBlockSize,
        parentSampleRate);
    //
    }

    context->dc_sampleRate  = sampleRate;
    context->dc_blockSize   = blockSize;
    context->dc_isReblocked = reblocked;
    context->dc_isSwitch    = switchable;
    
    ugen_graphCreateMissingSignalsForOutlets (context,
        switchable,
        reblocked,
        parentBlockSize,
        parentSampleRate);
        
    ugen_graphDspProlog (context,
        switchable,
        reblocked,
        blockSize,
        period,
        frequency,
        downsample,
        upsample);
    
    int chainEnd, chainBegin = pd_this->pd_dspChainSize;
    t_ugenbox *u = NULL;
    
    if (block && (reblocked || switchable)) { dsp_add (block_performProlog, 1, block); }   

    for (u = context->dc_ugens; u; u = u->u_next) {
        t_siginlet *uin = NULL;
        int i;
        u->u_done = 0;
        for (uin = u->u_in, i = u->u_inSize; i--; uin++)
            uin->i_numberConnected = 0, uin->i_signal = 0;
    }
    
        /* Do the sort */

    for (u = context->dc_ugens; u; u = u->u_next)
    {
        t_siginlet *uin;
        int i;
            /* check that we have no connected signal inlets */
        if (u->u_done) continue;
        for (uin = u->u_in, i = u->u_inSize; i--; uin++)
            if (uin->i_numberOfConnections) goto next;

        ugen_doit(context, u);
    next: ;
    }

        /* check for a DSP loop, which is evidenced here by the presence
        of ugens not yet scheduled. */
        
    for (u = context->dc_ugens; u; u = u->u_next)
        if (!u->u_done) 
    {
        t_signal **sigp;
        int i;
        post_error ("DSP loop detected (some tilde objects not scheduled)");
                /* this might imply that we have unfilled "borrowed" outputs
                which we'd better fill in now. */
        for (i = 0, sigp = context->dc_ioSignals + context->dc_numberOfInlets; i < context->dc_numberOfOutlets;
            i++, sigp++)
        {
            if ((*sigp)->s_isVectorBorrowed && !(*sigp)->s_borrowedFrom)
            {
                t_signal *s3 = signal_new(parentBlockSize, parentSampleRate);
                signal_borrow(*sigp, s3);
                (*sigp)->s_count++;
                dsp_addZeroPerform(s3->s_vector, s3->s_vectorSize);
                if (0)
                    post("oops, belatedly set %lx->%lx", *sigp,
                        (*sigp)->s_borrowedFrom);
            }
        }
        break;   /* don't need to keep looking. */
    }

    if (block && (reblocked || switchable))    /* add block DSP epilog */
        dsp_add(block_performEpilog, 1, block);
    
    chainEnd = pd_this->pd_dspChainSize;

        /* add epilogs for outlets.  */

    for (u = context->dc_ugens; u; u = u->u_next)
    {
        t_pd *zz = &u->u_owner->te_g.g_pd;
        if (pd_class(zz) == voutlet_class)
        {
            t_signal **iosigs = context->dc_ioSignals;
            if (iosigs) iosigs += context->dc_numberOfInlets;
            voutlet_dspEpilog((t_voutlet *)zz,
                iosigs, switchable, reblocked, blockSize, ugen_dspPhase, period, frequency,
                    downsample, upsample);
        }
    }

    if (block) { block_setPerformLength (block, chainEnd - chainBegin, pd_this->pd_dspChainSize - chainEnd); }

    ugen_graphDelete (context);
}

static void ugen_graphDelete (t_dspcontext *context)
{
    while (context->dc_ugens) {
    //
    t_ugenbox *u = context->dc_ugens;
    t_sigoutlet *o = u->u_out;
    int i;
    
    for (i = 0; i < u->u_outSize; i++) {
    
        t_sigoutconnect *c = o->o_connections;
        
        while (c) {
            t_sigoutconnect *t = c->oc_next;
            PD_MEMORY_FREE (c);
            c = t;
        }
        
        o++;
    }
    
    context->dc_ugens = u->u_next;
    
    PD_MEMORY_FREE (u->u_out);
    PD_MEMORY_FREE (u->u_in);
    PD_MEMORY_FREE (u);
    //
    }
    
    PD_ASSERT (ugen_context == context); ugen_context = context->dc_parentContext;
    
    PD_MEMORY_FREE (context);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------


/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WINDOWS

    #include <process.h>
    #include <winsock.h>

    typedef int socklen_t;
    
    #define EADDRINUSE WSAEADDRINUSE

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if !PD_WINDOWS

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/resource.h>

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_APPLE

#include <glob.h>
#include <pthread.h>

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if ( PD_LINUX || PD_BSD || PD_HURD )
    #define INTERFACE_LOCALHOST     "127.0.0.1"
#else
    #define INTERFACE_LOCALHOST     "localhost"
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

extern int sys_audioapi;

static char *main_commandToLaunchGUI;


#define INBUFSIZE 4096

extern int main_portNumber;

static int sys_nfdpoll;
static t_fdpoll *sys_fdpoll;
static int sys_maxfd;
static int sys_guisock;

static t_buffer *inbinbuf;
static t_socketreceiver *sys_socketreceiver;
void sys_set_searchpath(void);
void sys_set_extrapath(void);
void sys_set_startup(void);

/* ----------- functions for timing, signals, priorities, etc  --------- */

#ifdef _WIN32
static LARGE_INTEGER nt_inittime;
static double nt_freq = 0;

static void sys_initntclock(void)
{
    LARGE_INTEGER f1;
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    if (!QueryPerformanceFrequency(&f1))
    {
          fprintf(stderr, "pd: QueryPerformanceFrequency failed\n");
          f1.QuadPart = 1;
    }
    nt_freq = f1.QuadPart;
    nt_inittime = now;
}

#if 0
    /* this is a version you can call if you did the QueryPerformanceCounter
    call yourself.  Necessary for time tagging incoming MIDI at interrupt
    level, for instance; but we're not doing that just now. */

double nt_tixtotime(LARGE_INTEGER *dumbass)
{
    if (nt_freq == 0) sys_initntclock();
    return (((double)(dumbass->QuadPart - nt_inittime.QuadPart)) / nt_freq);
}
#endif
#endif /* _WIN32 */

    /* get "real time" in seconds; take the
    first time we get called as a reference time of zero. */
double sys_getrealtime(void)    
{
#ifndef _WIN32
    static struct timeval then;
    struct timeval now;
    gettimeofday(&now, 0);
    if (then.tv_sec == 0 && then.tv_usec == 0) then = now;
    return ((now.tv_sec - then.tv_sec) +
        (1./1000000.) * (now.tv_usec - then.tv_usec));
#else
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    if (nt_freq == 0) sys_initntclock();
    return (((double)(now.QuadPart - nt_inittime.QuadPart)) / nt_freq);
#endif
}

static int sys_domicrosleep(int microsec, int pollem)
{
    struct timeval timout;
    int i, didsomething = 0;
    t_fdpoll *fp;
    timout.tv_sec = 0;
    timout.tv_usec = microsec;
    if (pollem)
    {
        fd_set readset, writeset, exceptset;
        FD_ZERO(&writeset);
        FD_ZERO(&readset);
        FD_ZERO(&exceptset);
        for (fp = sys_fdpoll, i = sys_nfdpoll; i--; fp++)
            FD_SET(fp->fdp_fd, &readset);
#ifdef _WIN32
        if (sys_maxfd == 0)
                Sleep(microsec/1000);
        else
#endif
        select(sys_maxfd+1, &readset, &writeset, &exceptset, &timout);
        for (i = 0; i < sys_nfdpoll; i++)
            if (FD_ISSET(sys_fdpoll[i].fdp_fd, &readset))
        {
            //SCHEDULER_LOCK;   /* Wrong. */
            (*sys_fdpoll[i].fdp_fn)(sys_fdpoll[i].fdp_ptr, sys_fdpoll[i].fdp_fd);
            //SCHEDULER_UNLOCK;
            
            didsomething = 1;
        }
        return (didsomething);
    }
    else
    {
#ifdef _WIN32
        if (sys_maxfd == 0)
              Sleep(microsec/1000);
        else
#endif
        select(0, 0, 0, 0, &timout);
        return (0);
    }
}

void sys_microsleep(int microsec)
{
    sys_domicrosleep(microsec, 1);
}

#if !defined(_WIN32) && !defined(__CYGWIN__)
static void sys_signal(int signo, sig_t sigfun)
{
    struct sigaction action;
    action.sa_flags = 0;
    action.sa_handler = sigfun;
    memset(&action.sa_mask, 0, sizeof(action.sa_mask));
#if 0  /* GG says: don't use that */
    action.sa_restorer = 0;
#endif
    if (sigaction(signo, &action, 0) < 0)
        perror("sigaction");
}

static void sys_exithandler(int n)
{
    static int trouble = 0;
    if (!trouble)
    {
        trouble = 1;
        fprintf(stderr, "Pd: signal %d\n", n);
        sys_bail(1);
    }
    else _exit(1);
}

static void sys_alarmhandler(int n)
{
    fprintf(stderr, "Pd: system call timed out\n");
}

static void sys_huphandler(int n)
{
    struct timeval timout;
    timout.tv_sec = 0;
    timout.tv_usec = 30000;
    select(1, 0, 0, 0, &timout);
}

void sys_setalarm(int microsec)
{
    struct itimerval gonzo;
    int sec = (int)(microsec/1000000);
    microsec %= 1000000;
#if 0
    fprintf(stderr, "timer %d:%d\n", sec, microsec);
#endif
    gonzo.it_interval.tv_sec = 0;
    gonzo.it_interval.tv_usec = 0;
    gonzo.it_value.tv_sec = sec;
    gonzo.it_value.tv_usec = microsec;
    if (microsec)
        sys_signal(SIGALRM, sys_alarmhandler);
    else sys_signal(SIGALRM, SIG_IGN);
    setitimer(ITIMER_REAL, &gonzo, 0);
}

#endif /* NOT _WIN32 && NOT __CYGWIN__ */

    /* on startup, set various signal handlers */
void sys_setsignalhandlers( void)
{
#if !defined(_WIN32) && !defined(__CYGWIN__)
    signal(SIGHUP, sys_huphandler);
    signal(SIGINT, sys_exithandler);
    signal(SIGQUIT, sys_exithandler);
    signal(SIGILL, sys_exithandler);
# ifdef SIGIOT
    signal(SIGIOT, sys_exithandler);
# endif
    signal(SIGFPE, SIG_IGN);
    /* signal(SIGILL, sys_exithandler);
    signal(SIGBUS, sys_exithandler);
    signal(SIGSEGV, sys_exithandler); */
    signal(SIGPIPE, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
#if 0  /* GG says: don't use that */
    signal(SIGSTKFLT, sys_exithandler);
#endif
#endif /* NOT _WIN32 && NOT __CYGWIN__ */
}

#if defined(__linux__) || defined(__FreeBSD_kernel__) || defined(__GNU__)

#if defined(_POSIX_PRIORITY_SCHEDULING) || defined(_POSIX_MEMLOCK)
#include <sched.h>
#endif

void sys_set_priority(int higher) 
{
#ifdef _POSIX_PRIORITY_SCHEDULING
    struct sched_param par;
    int p1 ,p2, p3;
    p1 = sched_get_priority_min(SCHED_FIFO);
    p2 = sched_get_priority_max(SCHED_FIFO);
#ifdef USEAPI_JACK    
    p3 = (higher ? p1 + 7 : p1 + 5);
#else
    p3 = (higher ? p2 - 5 : p2 - 7);
#endif
    par.sched_priority = p3;
    if (sched_setscheduler(0,SCHED_FIFO,&par) < 0)
    {
        if (!higher)
            post("priority %d scheduling failed; running at normal priority",
                p3);
        else fprintf(stderr, "priority %d scheduling failed.\n", p3);
    }
    else if (!higher && 0)
        post("priority %d scheduling enabled.\n", p3);
#endif

#ifdef REALLY_POSIX_MEMLOCK /* this doesn't work on Fedora 4, for example. */
#ifdef _POSIX_MEMLOCK
    /* tb: force memlock to physical memory { */
    {
        struct rlimit mlock_limit;
        mlock_limit.rlim_cur=0;
        mlock_limit.rlim_max=0;
        setrlimit(RLIMIT_MEMLOCK,&mlock_limit);
    }
    /* } tb */
    if (mlockall(MCL_FUTURE) != -1) 
        fprintf(stderr, "memory locking enabled.\n");
#endif
#endif
}

#endif /* __linux__ */

/* ------------------ receiving incoming messages over sockets ------------- */

void sys_sockerror(char *s)
{
#ifdef _WIN32
    int err = WSAGetLastError();
    if (err == 10054) return;
    else if (err == 10044)
    {
        fprintf(stderr,
            "Warning: you might not have TCP/IP \"networking\" turned on\n");
        fprintf(stderr, "which is needed for Pd to talk to its GUI layer.\n");
    }
#else
    int err = errno;
#endif
    post("%s: %s (%d)\n", s, strerror(err), err);
}

void sys_addpollfn(int fd, t_pollfn fn, void *ptr)
{
    int nfd = sys_nfdpoll;
    int size = nfd * sizeof(t_fdpoll);
    t_fdpoll *fp;
    sys_fdpoll = (t_fdpoll *)PD_MEMORY_RESIZE(sys_fdpoll, size,
        size + sizeof(t_fdpoll));
    fp = sys_fdpoll + nfd;
    fp->fdp_fd = fd;
    fp->fdp_fn = fn;
    fp->fdp_ptr = ptr;
    sys_nfdpoll = nfd + 1;
    if (fd >= sys_maxfd) sys_maxfd = fd + 1;
}

void sys_rmpollfn(int fd)
{
    int nfd = sys_nfdpoll;
    int i, size = nfd * sizeof(t_fdpoll);
    t_fdpoll *fp;
    for (i = nfd, fp = sys_fdpoll; i--; fp++)
    {
        if (fp->fdp_fd == fd)
        {
            while (i--)
            {
                fp[0] = fp[1];
                fp++;
            }
            sys_fdpoll = (t_fdpoll *)PD_MEMORY_RESIZE(sys_fdpoll, size,
                size - sizeof(t_fdpoll));
            sys_nfdpoll = nfd - 1;
            return;
        }
    }
    post("warning: %d removed from poll list but not found", fd);
}

t_socketreceiver *socketreceiver_new(void *owner, t_socketnotifyfn notifier,
    t_socketreceivefn socketreceivefn, int udp)
{
    t_socketreceiver *x = (t_socketreceiver *)PD_MEMORY_GET(sizeof(*x));
    x->sr_inhead = x->sr_intail = 0;
    x->sr_owner = owner;
    x->sr_notifier = notifier;
    x->sr_socketreceivefn = socketreceivefn;
    x->sr_udp = udp;
    if (!(x->sr_inbuf = malloc(INBUFSIZE))) { PD_BUG; }
    return (x);
}

void socketreceiver_free(t_socketreceiver *x)
{
    free(x->sr_inbuf);
    PD_MEMORY_FREE(x);
}

    /* this is in a separately called subroutine so that the buffer isn't
    sitting on the stack while the messages are getting passed. */
static int socketreceiver_doread(t_socketreceiver *x)
{
    char messbuf[INBUFSIZE], *bp = messbuf;
    int indx, first = 1;
    int inhead = x->sr_inhead;
    int intail = x->sr_intail;
    char *inbuf = x->sr_inbuf;
    for (indx = intail; first || (indx != inhead);
        first = 0, (indx = (indx+1)&(INBUFSIZE-1)))
    {
            /* if we hit a semi that isn't preceeded by a \, it's a message
            boundary.  LATER we should deal with the possibility that the
            preceeding \ might itself be escaped! */
        char c = *bp++ = inbuf[indx];
        if (c == ';' && (!indx || inbuf[indx-1] != '\\'))
        {
            intail = (indx+1)&(INBUFSIZE-1);
            buffer_withStringUnzeroed(inbinbuf, messbuf, bp - messbuf);
            //if (0 /*sys_debuglevel*/ & DEBUG_MESSDOWN)
            //{
            //    write(2,  messbuf, bp - messbuf);
            //    write(2, "\n", 1);
            //}
            x->sr_inhead = inhead;
            x->sr_intail = intail;
            return (1);
        }
    }
    return (0);
}

static void socketreceiver_getudp(t_socketreceiver *x, int fd)
{
    char buf[INBUFSIZE+1];
    int ret = recv(fd, buf, INBUFSIZE, 0);
    if (ret < 0)
    {
        sys_sockerror("recv");
        sys_rmpollfn(fd);
        sys_closesocket(fd);
    }
    else if (ret > 0)
    {
        buf[ret] = 0;
#if 0
        post("%s", buf);
#endif
        if (buf[ret-1] != '\n')
        {
#if 0
            buf[ret] = 0;
            post_error ("dropped bad buffer %s\n", buf);
#endif
        }
        else
        {
            char *semi = strchr(buf, ';');
            if (semi) 
                *semi = 0;
            buffer_withStringUnzeroed(inbinbuf, buf, strlen(buf));
            if (x->sr_socketreceivefn)
                (*x->sr_socketreceivefn)(x->sr_owner, inbinbuf);
            else { PD_BUG; }
        }
    }
}

void socketreceiver_read(t_socketreceiver *x, int fd)
{
    if (x->sr_udp)   /* UDP ("datagram") socket protocol */
        socketreceiver_getudp(x, fd);
    else  /* TCP ("streaming") socket protocol */
    {
        char *semi;
        int readto =
            (x->sr_inhead >= x->sr_intail ? INBUFSIZE : x->sr_intail-1);
        int ret;

            /* the input buffer might be full.  If so, drop the whole thing */
        if (readto == x->sr_inhead)
        {
            fprintf(stderr, "pd: dropped message from gui\n");
            x->sr_inhead = x->sr_intail = 0;
            readto = INBUFSIZE;
        }
        else
        {
            ret = recv(fd, x->sr_inbuf + x->sr_inhead,
                readto - x->sr_inhead, 0);
            if (ret < 0)
            {
                sys_sockerror("recv");
                if (x == sys_socketreceiver) sys_bail(1);
                else
                {
                    if (x->sr_notifier)
                        (*x->sr_notifier)(x->sr_owner, fd);
                    sys_rmpollfn(fd);
                    sys_closesocket(fd);
                }
            }
            else if (ret == 0)
            {
                if (x == sys_socketreceiver)
                {
                    fprintf(stderr, "pd: exiting\n");
                    scheduler_needToExit();
                    return;
                }
                else
                {
                    post("EOF on socket %d\n", fd);
                    if (x->sr_notifier) (*x->sr_notifier)(x->sr_owner, fd);
                    sys_rmpollfn(fd);
                    sys_closesocket(fd);
                }
            }
            else
            {
                x->sr_inhead += ret;
                if (x->sr_inhead >= INBUFSIZE) x->sr_inhead = 0;
                while (socketreceiver_doread(x))
                {
                    if (x->sr_socketreceivefn)
                        (*x->sr_socketreceivefn)(x->sr_owner, inbinbuf);
                    else buffer_eval(inbinbuf, 0, 0, 0);
                    if (x->sr_inhead == x->sr_intail)
                        break;
                }
            }
        }
    }
}

void sys_closesocket(int fd)
{
#ifdef _WIN32
    closesocket(fd);
#else
    close(fd);
#endif
}

/* ---------------------- sending messages to the GUI ------------------ */
#define GUI_ALLOCCHUNK 8192
#define GUI_UPDATESLICE 512 /* how much we try to do in one idle period */
#define GUI_BYTESPERPING 1024 /* how much we send up per ping */

typedef struct _guiqueue
{
    void *gq_client;
    t_glist *gq_glist;
    t_guifn gq_fn;
    struct _guiqueue *gq_next;
} t_guiqueue;

static t_guiqueue *sys_guiqueuehead;
static char *sys_guibuf;
static int sys_guibufhead;
static int sys_guibuftail;
static int sys_guibufsize;
static int sys_waitingforping;
static int sys_bytessincelastping;

static void sys_trytogetmoreguibuf(int newsize)
{
    char *newbuf = realloc(sys_guibuf, newsize);
#if 0
    static int sizewas;
    if (newsize > 70000 && sizewas < 70000)
    {
        int i;
        for (i = sys_guibuftail; i < sys_guibufhead; i++)
            fputc(sys_guibuf[i], stderr);
    }
    sizewas = newsize;
#endif
#if 0
    fprintf(stderr, "new size %d (head %d, tail %d)\n",
        newsize, sys_guibufhead, sys_guibuftail);
#endif

        /* if realloc fails, make a last-ditch attempt to stay alive by
        synchronously writing out the existing contents.  LATER test
        this by intentionally setting newbuf to zero */
    if (!newbuf)
    {
        int bytestowrite = sys_guibuftail - sys_guibufhead;
        int written = 0;
        while (1)
        {
            int res = send(sys_guisock,
                sys_guibuf + sys_guibuftail + written, bytestowrite, 0);
            if (res < 0)
            {
                perror("pd output pipe");
                sys_bail(1);
            }
            else
            {
                written += res;
                if (written >= bytestowrite)
                    break;
            }
        }
        sys_guibufhead = sys_guibuftail = 0;
    }
    else
    {
        sys_guibufsize = newsize;
        sys_guibuf = newbuf;
    }
}

void sys_vgui(char *fmt, ...)
{
    int msglen, bytesleft, headwas, nwrote;
    va_list ap;

    if (PD_WITH_NOGUI)
        return;
    if (!sys_guibuf)
    {
        if (!(sys_guibuf = malloc(GUI_ALLOCCHUNK)))
        {
            fprintf(stderr, "Pd: couldn't allocate GUI buffer\n");
            sys_bail(1);
        }
        sys_guibufsize = GUI_ALLOCCHUNK;
        sys_guibufhead = sys_guibuftail = 0;
    }
    if (sys_guibufhead > sys_guibufsize - (GUI_ALLOCCHUNK/2))
        sys_trytogetmoreguibuf(sys_guibufsize + GUI_ALLOCCHUNK);
    va_start(ap, fmt);
    msglen = vsnprintf(sys_guibuf + sys_guibufhead,
        sys_guibufsize - sys_guibufhead, fmt, ap);
    va_end(ap);
    if(msglen < 0) 
    {
        fprintf(stderr, "Pd: buffer space wasn't sufficient for long GUI string\n");
        return;
    }
    if (msglen >= sys_guibufsize - sys_guibufhead)
    {
        int msglen2, newsize = sys_guibufsize + 1 +
            (msglen > GUI_ALLOCCHUNK ? msglen : GUI_ALLOCCHUNK);
        sys_trytogetmoreguibuf(newsize);

        va_start(ap, fmt);
        msglen2 = vsnprintf(sys_guibuf + sys_guibufhead,
            sys_guibufsize - sys_guibufhead, fmt, ap);
        va_end(ap);
        if (msglen2 != msglen) { PD_BUG; }
        if (msglen >= sys_guibufsize - sys_guibufhead)
            msglen = sys_guibufsize - sys_guibufhead;
    }
    //if (0 /*sys_debuglevel*/ & DEBUG_MESSUP)
    //    fprintf(stderr, "%s",  sys_guibuf + sys_guibufhead);
    sys_guibufhead += msglen;
    sys_bytessincelastping += msglen;
}

void sys_gui(char *s)
{
    sys_vgui("%s", s);
}

static int sys_flushtogui( void)
{
    int writesize = sys_guibufhead - sys_guibuftail, nwrote = 0;
    if (writesize > 0)
        nwrote = send(sys_guisock, sys_guibuf + sys_guibuftail, writesize, 0);

#if 0   
    if (writesize)
        fprintf(stderr, "wrote %d of %d\n", nwrote, writesize);
#endif

    if (nwrote < 0)
    {
        perror("pd-to-gui socket");
        sys_bail(1);
    }
    else if (!nwrote)
        return (0);
    else if (nwrote >= sys_guibufhead - sys_guibuftail)
         sys_guibufhead = sys_guibuftail = 0;
    else if (nwrote)
    {
        sys_guibuftail += nwrote;
        if (sys_guibuftail > (sys_guibufsize >> 2))
        {
            memmove(sys_guibuf, sys_guibuf + sys_guibuftail,
                sys_guibufhead - sys_guibuftail);
            sys_guibufhead = sys_guibufhead - sys_guibuftail;
            sys_guibuftail = 0;
        }
    }
    return (1);
}

void global_ping(void *dummy)
{
    sys_waitingforping = 0;
}

static int sys_flushqueue(void )
{
    int wherestop = sys_bytessincelastping + GUI_UPDATESLICE;
    if (wherestop + (GUI_UPDATESLICE >> 1) > GUI_BYTESPERPING)
        wherestop = 0x7fffffff;
    if (sys_waitingforping)
        return (0);
    if (!sys_guiqueuehead)
        return (0);
    while (1)
    {
        if (sys_bytessincelastping >= GUI_BYTESPERPING)
        {
            sys_gui("::ping\n");
            sys_bytessincelastping = 0;
            sys_waitingforping = 1;
            return (1);
        }
        if (sys_guiqueuehead)
        {
            t_guiqueue *headwas = sys_guiqueuehead;
            sys_guiqueuehead = headwas->gq_next;
            (*headwas->gq_fn)(headwas->gq_client, headwas->gq_glist);
            PD_MEMORY_FREE(headwas);
            if (sys_bytessincelastping >= wherestop)
                break;
        }
        else break;
    }
    sys_flushtogui();
    return (1);
}

    /* flush output buffer and update queue to gui in small time slices */
static int sys_poll_togui(void) /* returns 1 if did anything */
{
    if (PD_WITH_NOGUI)
        return (0);
        /* in case there is stuff still in the buffer, try to flush it. */
    sys_flushtogui();
        /* if the flush wasn't complete, wait. */
    if (sys_guibufhead > sys_guibuftail)
        return (0);
    
        /* check for queued updates */
    if (sys_flushqueue())
        return (1);
    
    return (0);
}

    /* if some GUI object is having to do heavy computations, it can tell
    us to back off from doing more updates by faking a big one itself. */
void sys_pretendguibytes(int n)
{
    sys_bytessincelastping += n;
}

void sys_queuegui(void *client, t_glist *glist, t_guifn f)
{
    t_guiqueue **gqnextptr, *gq;
    if (!sys_guiqueuehead)
        gqnextptr = &sys_guiqueuehead;
    else
    {
        for (gq = sys_guiqueuehead; gq->gq_next; gq = gq->gq_next)
            if (gq->gq_client == client)
                return;
        if (gq->gq_client == client)
            return;
        gqnextptr = &gq->gq_next;
    }
    gq = PD_MEMORY_GET(sizeof(*gq));
    gq->gq_next = 0;
    gq->gq_client = client;
    gq->gq_glist = glist;
    gq->gq_fn = f;
    gq->gq_next = 0;
    *gqnextptr = gq;
}

void sys_unqueuegui(void *client)
{
    t_guiqueue *gq, *gq2;
    while (sys_guiqueuehead && sys_guiqueuehead->gq_client == client)
    {
        gq = sys_guiqueuehead;
        sys_guiqueuehead = sys_guiqueuehead->gq_next;
        PD_MEMORY_FREE(gq);
    }
    if (!sys_guiqueuehead)
        return;
    for (gq = sys_guiqueuehead; gq2 = gq->gq_next; gq = gq2)
        if (gq2->gq_client == client)
    {
        gq->gq_next = gq2->gq_next;
        PD_MEMORY_FREE(gq2);
        break;
    }
}

int sys_pollgui(void)
{
    return (sys_domicrosleep(0, 1) || sys_poll_togui());
}

void sys_init_fdpoll(void)
{
    if (sys_fdpoll)
        return;
    /* create an empty FD poll list */
    sys_fdpoll = (t_fdpoll *)PD_MEMORY_GET(0);
    sys_nfdpoll = 0;
    inbinbuf = buffer_new();
}

/* --------------------- starting up the GUI connection ------------- */

static int sys_watchfd;

#if defined(__linux__) || defined(__FreeBSD_kernel__) || defined(__GNU__)
void global_watchdog(void *dummy)
{
    if (write(sys_watchfd, "\n", 1) < 1)
    {
        fprintf(stderr, "pd: watchdog process died\n");
        sys_bail(1);
    }
}
#endif

#define FIRSTPORTNUM 5400

#define MAXFONTS 21
static int defaultfontshit[MAXFONTS] = {
    8, 5, 9, 
    10, 6, 10, 
    12, 7, 13, 
    14, 9, 17, 
    16, 10, 19, 
    24, 15, 28,
    24, 15, 28};
    
#define NDEFAULTFONT (sizeof(defaultfontshit)/sizeof(*defaultfontshit))

int sys_startgui(const char *libdir)
{
    char cmdbuf[4*PD_STRING];
    struct sockaddr_in server;
    int msgsock;
    char buf[15];
    int len = sizeof(server);
    int ntry = 0, portno = FIRSTPORTNUM;
    int xsock = -1, dumbo = -1;
#ifdef _WIN32
    short version = MAKEWORD(2, 0);
    WSADATA nobby;
#else
    int stdinpipe[2];
    pid_t childpid;
#endif /* _WIN32 */
    sys_init_fdpoll();

#ifdef _WIN32
    if (WSAStartup(version, &nobby)) sys_sockerror("WSAstartup");
#endif /* _WIN32 */

    if (PD_WITH_NOGUI) { }
    else if (main_portNumber)  /* GUI exists and sent us a port number */
    {
        struct sockaddr_in server;
        struct hostent *hp;
#ifdef __APPLE__
            /* sys_guisock might be 1 or 2, which will have offensive results
            if somebody writes to stdout or stderr - so we just open a few
            files to try to fill fds 0 through 2.  (I tried using dup()
            instead, which would seem the logical way to do this, but couldn't
            get it to work.) */
        int burnfd1 = open("/dev/null", 0), burnfd2 = open("/dev/null", 0),
            burnfd3 = open("/dev/null", 0);
        if (burnfd1 > 2)
            close(burnfd1);
        if (burnfd2 > 2)
            close(burnfd2);
        if (burnfd3 > 2)
            close(burnfd3);
#endif
        /* create a socket */
        sys_guisock = socket(AF_INET, SOCK_STREAM, 0);
        if (sys_guisock < 0)
            sys_sockerror("socket");
        
        /* connect socket using hostname provided in command line */
        server.sin_family = AF_INET;

        hp = gethostbyname(INTERFACE_LOCALHOST);

        if (hp == 0)
        {
            fprintf(stderr,
                "localhost not found (inet protocol not installed?)\n");
            return (1);
        }
        memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);

        /* assign client port number */
        server.sin_port = htons((unsigned short)main_portNumber);

            /* try to connect */
        if (connect(sys_guisock, (struct sockaddr *) &server, sizeof (server))
            < 0)
        {
            sys_sockerror("connecting stream socket");
            return (1);
        }
    }
    else    /* default behavior: start up the GUI ourselves. */
    {
#ifdef _WIN32
        char scriptbuf[PD_STRING+30], wishbuf[PD_STRING+30], portbuf[80];
        int spawnret;
        char intarg;
#else
        int intarg;
#endif

        /* create a socket */
        xsock = socket(AF_INET, SOCK_STREAM, 0);
        if (xsock < 0) sys_sockerror("socket");
        intarg = 1;
        if (setsockopt(xsock, IPPROTO_TCP, TCP_NODELAY,
            &intarg, sizeof(intarg)) < 0)
#ifndef _WIN32
                post("setsockopt (TCP_NODELAY) failed\n")
#endif
                    ;
        
        
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;

        /* assign server port number */
        server.sin_port =  htons((unsigned short)portno);

        /* name the socket */
        while (bind(xsock, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
#ifdef _WIN32
            int err = WSAGetLastError();
#else
            int err = errno;
#endif
            if ((ntry++ > 20) || (err != EADDRINUSE))
            {
                perror("bind");
                fprintf(stderr,
                    "Pd needs your machine to be configured with\n");
                fprintf(stderr,
                  "'networking' turned on (see Pd's html doc for details.)\n");
                return (1);
            }
            portno++;
            server.sin_port = htons((unsigned short)(portno));
        }

        if (0) fprintf(stderr, "port %d\n", portno);


#ifndef _WIN32
        if (!main_commandToLaunchGUI)
        {
#ifdef __APPLE__
            int i;
            struct stat statbuf;
            glob_t glob_buffer;
            char *homedir = getenv("HOME");
            char embed_glob[FILENAME_MAX];
            char home_filename[FILENAME_MAX];
            char *wish_paths[10] = {
                "(did not find a home directory)",
                "/Applications/Utilities/Wish.app/Contents/MacOS/Wish",
                "/Applications/Utilities/Wish Shell.app/Contents/MacOS/Wish Shell",
                "/Applications/Wish.app/Contents/MacOS/Wish",
                "/Applications/Wish Shell.app/Contents/MacOS/Wish Shell",
                "/Library/Frameworks/Tk.framework/Resources/Wish.app/Contents/MacOS/Wish",
                "/Library/Frameworks/Tk.framework/Resources/Wish Shell.app/Contents/MacOS/Wish Shell",
                "/System/Library/Frameworks/Tk.framework/Resources/Wish.app/Contents/MacOS/Wish",
                "/System/Library/Frameworks/Tk.framework/Resources/Wish Shell.app/Contents/MacOS/Wish Shell",
                "/usr/bin/wish"
            };
            /* this glob is needed so the Wish executable can have the same
             * filename as the Pd.app, i.e. 'Pd-0.42-3.app' should have a Wish
             * executable called 'Pd-0.42-3.app/Contents/MacOS/Pd-0.42-3' */
            sprintf(embed_glob, "%s/../MacOS/Pd*", libdir);
            glob_buffer.gl_matchc = 1; /* we only need one match */
            glob(embed_glob, GLOB_LIMIT, NULL, &glob_buffer);
            /* If we are using a copy of Wish embedded in the Pd.app, then it
             * will automatically load ui_main.tcl if that embedded Wish can
             * find ../Resources/Scripts/AppMain.tcl, then Wish doesn't want
             * to receive the ui_main.tcl as an argument.  Otherwise it needs
             * to know how to find ui_main.tcl */
            if (glob_buffer.gl_pathc > 0)
                sprintf(cmdbuf, "\"%s\" %d\n", glob_buffer.gl_pathv[0], portno);
            else
            {
                sprintf(home_filename,
                        "%s/Applications/Wish.app/Contents/MacOS/Wish",homedir);
                wish_paths[0] = home_filename;
                for(i=0; i<10; i++)
                {
                    if (0)
                        fprintf(stderr, "Trying Wish at \"%s\"\n", wish_paths[i]);
                    if (stat(wish_paths[i], &statbuf) >= 0)
                        break;
                }
                sprintf(cmdbuf, "\"%s\" \"%s/%sui_main.tcl\" %d\n", 
                        wish_paths[i], libdir, PD_TCL_DIRECTORY, portno);
            }
#else /* __APPLE__ */
            /* sprintf the wish command with needed environment variables.
            For some reason the wish script fails if HOME isn't defined so
            if necessary we put that in here too. */
            sprintf(cmdbuf,
  "TCL_LIBRARY=\"%s/lib/tcl/library\" TK_LIBRARY=\"%s/lib/tk/library\"%s \
  wish \"%s/" PD_TCL_DIRECTORY "/ui_main.tcl\" %d\n",
                 libdir, libdir, (getenv("HOME") ? "" : " HOME=/tmp"),
                    libdir, portno);
#endif /* __APPLE__ */
            main_commandToLaunchGUI = cmdbuf;
        }

        if (0) 
            fprintf(stderr, "%s", main_commandToLaunchGUI);

        childpid = fork();
        if (childpid < 0)
        {
            if (errno) perror("sys_startgui");
            else fprintf(stderr, "sys_startgui failed\n");
            return (1);
        }
        else if (!childpid)                     /* we're the child */
        {
            setuid(getuid());          /* lose setuid priveliges */
#ifndef __APPLE__
// TODO this seems unneeded on any platform hans@eds.org
                /* the wish process in Unix will make a wish shell and
                    read/write standard in and out unless we close the
                    file descriptors.  Somehow this doesn't make the MAC OSX
                        version of Wish happy...*/
            if (pipe(stdinpipe) < 0)
                sys_sockerror("pipe");
            else
            {
                if (stdinpipe[0] != 0)
                {
                    close (0);
                    dup2(stdinpipe[0], 0);
                    close(stdinpipe[0]);
                }
            }
#endif /* NOT __APPLE__ */
            execl("/bin/sh", "sh", "-c", main_commandToLaunchGUI, (char*)0);
            perror("pd: exec");
            fprintf(stderr, "Perhaps tcl and tk aren't yet installed?\n");
            _exit(1);
       }
#else /* NOT _WIN32 */
        /* fprintf(stderr, "%s\n", libdir); */
        
        strcpy(scriptbuf, "\"");
        strcat(scriptbuf, libdir);
        strcat(scriptbuf, "/" PD_TCL_DIRECTORY "ui_main.tcl\"");
        sys_bashfilename(scriptbuf, scriptbuf);
        
        sprintf(portbuf, "%d", portno);

        strcpy(wishbuf, libdir);
        strcat(wishbuf, "/" PD_BIN_DIRECTORY PD_EXE_WISH);
        sys_bashfilename(wishbuf, wishbuf);
        
        spawnret = _spawnl(P_NOWAIT, wishbuf, PD_EXE_WISH, scriptbuf, portbuf, 0);
        if (spawnret < 0)
        {
            perror("spawnl");
            fprintf(stderr, "%s: couldn't load TCL\n", wishbuf);
            return (1);
        }

#endif /* NOT _WIN32 */
    }

#if defined(__linux__) || defined(__FreeBSD_kernel__)
        /* now that we've spun off the child process we can promote
        our process's priority, if we can and want to.  If not specfied
        (-1), we assume real-time was wanted.  Afterward, just in case
        someone made Pd setuid in order to get permission to do this,
        unset setuid and lose root priveliges after doing this.  Starting
        in Linux 2.6 this is accomplished by putting lines like:
                @audio - rtprio 99
                @audio - memlock unlimited
        in the system limits file, perhaps /etc/limits.conf or
        /etc/security/limits.conf */

    sprintf(cmdbuf, "%s/bin/pdwatchdog", libdir);
    if (PD_WITH_REALTIME)
    {
        struct stat statbuf;
        if (stat(cmdbuf, &statbuf) < 0)
        {
            PD_BUG;
            PD_ABORT (1);
        }
    }
    else if (0)
        post("not setting real-time priority");
    
    if (PD_WITH_REALTIME)
    {
            /* To prevent lockup, we fork off a watchdog process with
            higher real-time priority than ours.  The GUI has to send
            a stream of ping messages to the watchdog THROUGH the Pd
            process which has to pick them up from the GUI and forward
            them.  If any of these things aren't happening the watchdog
            starts sending "stop" and "cont" signals to the Pd process
            to make it timeshare with the rest of the system.  (Version
            0.33P2 : if there's no GUI, the watchdog pinging is done
            from the scheduler idle routine in this process instead.) */
        int pipe9[2], watchpid;

        if (pipe(pipe9) < 0)
        {
            setuid(getuid());      /* lose setuid priveliges */
            sys_sockerror("pipe");
            return (1);
        }
        watchpid = fork();
        if (watchpid < 0)
        {
            setuid(getuid());      /* lose setuid priveliges */
            if (errno)
                perror("sys_startgui");
            else fprintf(stderr, "sys_startgui failed\n");
            return (1);
        }
        else if (!watchpid)             /* we're the child */
        {
            sys_set_priority(1);
            setuid(getuid());      /* lose setuid priveliges */
            if (pipe9[1] != 0)
            {
                dup2(pipe9[0], 0);
                close(pipe9[0]);
            }
            close(pipe9[1]);

            if (0) fprintf(stderr, "%s\n", cmdbuf);
            execl("/bin/sh", "sh", "-c", cmdbuf, (char*)0);
            perror("pd: exec");
            _exit(1);
        }
        else                            /* we're the parent */
        {
            sys_set_priority(0);
            setuid(getuid());      /* lose setuid priveliges */
            close(pipe9[0]);
                /* set close-on-exec so that watchdog will see an EOF when we
                close our copy - otherwise it might hang waiting for some
                stupid child process (as seems to happen if jackd auto-starts
                for us.) */
            fcntl(pipe9[1], F_SETFD, FD_CLOEXEC);
            sys_watchfd = pipe9[1];
                /* We also have to start the ping loop in the GUI;
                this is done later when the socket is open. */
        }
    }

    setuid(getuid());          /* lose setuid priveliges */
#endif /* __linux__ */

#ifdef _WIN32
    if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
        fprintf(stderr, "pd: couldn't set high priority class\n");
#endif
#ifdef __APPLE__
    if (PD_WITH_REALTIME)
    {
        struct sched_param param;
        int policy = SCHED_RR;
        int err;
        param.sched_priority = 80; /* adjust 0 : 100 */

        err = pthread_setschedparam(pthread_self(), policy, &param);
        if (err)
            post("warning: high priority scheduling failed\n");
    }
#endif /* __APPLE__ */

    if (!PD_WITH_NOGUI && !main_portNumber)
    {
        if (0)
            fprintf(stderr, "Waiting for connection request... \n");
        if (listen(xsock, 5) < 0) sys_sockerror("listen");

        sys_guisock = accept(xsock, (struct sockaddr *) &server, 
            (socklen_t *)&len);
#ifdef OOPS
        sys_closesocket(xsock);
#endif
        if (sys_guisock < 0) sys_sockerror("accept");
        if (0)
            fprintf(stderr, "... connected\n");
        sys_guibufhead = sys_guibuftail = 0;
    }
    if (!PD_WITH_NOGUI)
    {
        char buf[256], buf2[256];
        sys_socketreceiver = socketreceiver_new(0, 0, 0, 0);
        sys_addpollfn(sys_guisock, (t_pollfn)socketreceiver_read,
            sys_socketreceiver);

            /* here is where we start the pinging. */
#if defined(__linux__) || defined(__FreeBSD_kernel__)
        if (PD_WITH_REALTIME)
            sys_gui("::watchdog\n");
#endif
        sys_get_audio_apis(buf);
        sys_get_midi_apis(buf2);
        sys_set_searchpath();     /* tell GUI about path and startup flags */
        sys_set_extrapath();
        sys_set_startup();
                           /* ... and about font, medio APIS, etc */
        sys_vgui("::initialize %s %s\n",
                 buf, buf2); 
                 /* */
                 /* */
        sys_vgui("set ::var(apiAudio) %d\n", sys_audioapi);
    }
    return (0);
}

/* This is called when something bad has happened, like a segfault.
Call global_quit() below to exit cleanly.
LATER try to save dirty documents even in the bad case. */
void sys_bail(int n)
{
    static int reentered = 0;
    if (!reentered)
    {
        reentered = 1;
#if !defined(__linux__) && !defined(__FreeBSD_kernel__) && !defined(__GNU__) /* sys_close_audio() hangs if you're in a signal? */
        fprintf(stderr ,"sys_guisock %d - ", sys_guisock);
        fprintf(stderr, "closing audio...\n");
        sys_close_audio();
        fprintf(stderr, "closing MIDI...\n");
        sys_close_midi();
        fprintf(stderr, "... done.\n");
#endif
        exit(n);
    }
    else _exit(1);
}

void global_quit(void *dummy)
{
    sys_close_audio();
    sys_close_midi();
    if (!PD_WITH_NOGUI)
    {
        sys_closesocket(sys_guisock);
        sys_rmpollfn(sys_guisock);
    }
    exit(0); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBTAS_THREADS_H_INCL
#define LIBTAS_THREADS_H_INCL

#include "hook.h"
#include "global.h"
#include <pthread.h> // pthread_t
#include <semaphore.h>
#include <SDL2/SDL.h> // SDL_Thread, SDL_ThreadFunction

namespace libtas {

/**
 *  Create a thread.
 */
OVERRIDE SDL_Thread* SDL_CreateThread(SDL_ThreadFunction fn, const char *name, void *data);

/**
 *  Wait for a thread to finish. Threads that haven't been detached will
 *  remain (as a "zombie") until this function cleans them up. Not doing so
 *  is a resource leak.
 *
 *  Once a thread has been cleaned up through this function, the SDL_Thread
 *  that references it becomes invalid and should not be referenced again.
 *  As such, only one thread may call SDL_WaitThread() on another.
 *
 *  The return code for the thread function is placed in the area
 *  pointed to by \c status, if \c status is not NULL.
 *
 *  You may not wait on a thread that has been used in a call to
 *  SDL_DetachThread(). Use either that function or this one, but not
 *  both, or behavior is undefined.
 *
 *  It is safe to pass NULL to this function; it is a no-op.
 */
OVERRIDE void SDL_WaitThread(SDL_Thread * thread, int *status);

/**
 *  A thread may be "detached" to signify that it should not remain until
 *  another thread has called SDL_WaitThread() on it. Detaching a thread
 *  is useful for long-running threads that nothing needs to synchronize
 *  with or further manage. When a detached thread is done, it simply
 *  goes away.
 *
 *  There is no way to recover the return code of a detached thread. If you
 *  need this, don't detach the thread and instead use SDL_WaitThread().
 *
 *  Once a thread is detached, you should usually assume the SDL_Thread isn't
 *  safe to reference again, as it will become invalid immediately upon
 *  the detached thread's exit, instead of remaining until someone has called
 *  SDL_WaitThread() to finally clean it up. As such, don't detach the same
 *  thread more than once.
 *
 *  If a thread has already exited when passed to SDL_DetachThread(), it will
 *  stop waiting for a call to SDL_WaitThread() and clean up immediately.
 *  It is not safe to detach a thread that might be used with SDL_WaitThread().
 *
 *  You may not call SDL_WaitThread() on a thread that has been detached.
 *  Use either that function or this one, but not both, or behavior is
 *  undefined.
 *
 *  It is safe to pass NULL to this function; it is a no-op.
 */
OVERRIDE void SDL_DetachThread(SDL_Thread * thread);

/* Create a new thread, starting with execution of START-ROUTINE
   getting passed ARG.  Creation attributed come from ATTR.  The new
   handle is stored in *NEWTHREAD.  */
OVERRIDE int pthread_create (pthread_t * thread, const pthread_attr_t * attr, void * (* start_routine) (void *), void * arg) throw();

/* Terminate calling thread.

   The registered cleanup handlers are called via exception handling
   so we cannot mark this function with __THROW.*/
OVERRIDE void pthread_exit (void *retval);

/* Make calling thread wait for termination of the thread TH.  The
   exit status of the thread is stored in *THREAD_RETURN, if THREAD_RETURN
   is not NULL.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int pthread_join (pthread_t thread, void **thread_return);

/* Indicate that the thread TH is never to be joined with PTHREAD_JOIN.
   The resources of TH will therefore be freed immediately when it
   terminates, instead of waiting for another thread to perform PTHREAD_JOIN
   on it.  */
OVERRIDE int pthread_detach (pthread_t thread) throw();

/* Check whether thread TH has terminated.  If yes return the status of
   the thread in *THREAD_RETURN, if THREAD_RETURN is not NULL.  */
OVERRIDE int pthread_tryjoin_np(pthread_t thread, void **retval) throw();

/* Make calling thread wait for termination of the thread TH, but only
   until TIMEOUT.  The exit status of the thread is stored in
   *THREAD_RETURN, if THREAD_RETURN is not NULL.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int pthread_timedjoin_np(pthread_t thread, void **retval, const struct timespec *abstime);

/* Wake up one thread waiting for condition variable COND.  */
OVERRIDE int pthread_cond_signal (pthread_cond_t *cond) throw();

/* Wake up all threads waiting for condition variables COND.  */
OVERRIDE int pthread_cond_broadcast (pthread_cond_t *__cond) throw();

/* Wait for condition variable COND to be signaled or broadcast.
   MUTEX is assumed to be locked before.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int pthread_cond_wait (pthread_cond_t *cond, pthread_mutex_t *mutex);

/* Wait for condition variable COND to be signaled or broadcast until
   ABSTIME.  MUTEX is assumed to be locked before.  ABSTIME is an
   absolute time specification; zero is the beginning of the epoch
   (00:00:00 GMT, January 1, 1970).

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int pthread_cond_timedwait (pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);

/* Set cancelability state of current thread to STATE, returning old
   state in *OLDSTATE if OLDSTATE is not NULL.  */
OVERRIDE int pthread_setcancelstate (int state, int *oldstate);

/* Set cancellation state of current thread to TYPE, returning the old
   type in *OLDTYPE if OLDTYPE is not NULL.  */
OVERRIDE int pthread_setcanceltype (int type, int *oldtype);

/* Cancel THREAD immediately or at the next possibility.  */
OVERRIDE int pthread_cancel (pthread_t th);

/* Test for pending cancellation for the current thread and terminate
   the thread as per pthread_exit(PTHREAD_CANCELED) if it has been
   cancelled.  */
OVERRIDE void pthread_testcancel (void);

OVERRIDE int sem_timedwait (sem_t *sem, const struct timespec *abstime);

/* Test whether SEM is posted.  */
OVERRIDE int sem_trywait (sem_t *__sem) throw();

}

#endif

/* inputthread.c -- Threaded generation of input events.
 *
 * Copyright © 2007-2008 Tiago Vignatti <vignatti at freedesktop org>
 * Copyright © 2010 Nokia
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Fernando Carrijo <fcarrijo at freedesktop org>
 * Tiago Vignatti <vignatti at freedesktop org>
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <X11/Xpoll.h>
#include "inputthread.h"
#include "inputstr.h"
#include "opaque.h"

/**
 * An input device as seen by the threaded input facility
 */
typedef struct _InputThreadDevice {
    struct _InputThreadDevice *next;
    void (*readInputProc)(void*);
    void *readInputArgs;
    int fd;
} InputThreadDevice;

/**
 * The threaded input facility.
 *
 * For now, we have one instance for all input devices.
 */
typedef struct {
    pthread_t thread;
    InputThreadDevice *devs;
    fd_set fds;
    int readPipe;
    int writePipe;
} InputThreadInfo;

static InputThreadInfo *inputThreadInfo;

static int hotplugPipeRead = -1;
static int hotplugPipeWrite = -1;

/**
 * Notify a thread about the availability of new asynchronously enqueued input
 * events.
 *
 * @see WaitForSomething()
 */
static void
InputThreadFillPipe(int writeHead)
{
    int ret;
    char byte = 0;
    fd_set writePipe;

    FD_ZERO(&writePipe);

    while (1) {
        ret = write(writeHead, &byte, 1);
        if (!ret)
            FatalError("input-thread: write() returned 0");
        if (ret > 0) {
            break;
        }
        if (errno != EAGAIN)
            FatalError("input-thread: filling pipe");

        DebugF("input-thread: pipe full\n");
        FD_SET(writeHead, &writePipe);
        Select(writeHead + 1, NULL, &writePipe, NULL, NULL);
    }
}

/**
 * Consume eventual notifications left by a thread.
 *
 * @see WaitForSomething()
 * @see InputThreadFillPipe()
 */
static int
InputThreadReadPipe(int readHead)
{
    int ret, array[10];

    ret = read(readHead, &array, sizeof(array));
    if (ret >= 0)
        return ret;

    if (errno != EAGAIN)
        FatalError("input-thread: draining pipe (%d)", errno);

    return 1;
}

/**
 * Register an input device in the threaded input facility
 *
 * @param fd File descriptor which identifies the input device
 * @param readInputProc Procedure used to read input from the device
 * @param readInputArgs Arguments to be consumed by the above procedure
 *
 * return 1 if success; 0 otherwise.
 */
int
InputThreadRegisterDev(int fd,
    void (*readInputProc) (void*),
    void *readInputArgs)
{
    InputThreadDevice *new;

    new = malloc(sizeof(InputThreadDevice));
    if (new == NULL) {
        DebugF("input-thread: could not register device\n");
        return 0;
    }

    new->fd = fd;
    new->readInputProc = readInputProc;
    new->readInputArgs = readInputArgs;
    new->next = inputThreadInfo->devs;

    inputThreadInfo->devs = new;
    FD_SET(fd, &inputThreadInfo->fds);

    InputThreadFillPipe(hotplugPipeWrite);
    DebugF("input-thread: registered device %d\n", fd);

    return 1;
}

/**
 * Unregister a device in the threaded input facility
 *
 * @param fd File descriptor which identifies the input device
 *
 * @return 1 if success; 0 otherwise.
 */
int
InputThreadUnregisterDev(int fd)
{
    InputThreadDevice *prev, *dev;


    /* return silently if input thread is already finished (e.g., at
     * DisableDevice time, evdev tries to call this function again through
     * xf86RemoveEnabledDevice */
    if (!inputThreadInfo)
        return 0;

    prev = NULL;
    dev = inputThreadInfo->devs;
    while (dev != NULL) {
        if (dev->fd == fd)
            break;
        prev = dev;
        dev = dev->next;
    }

    /* fd didn't match any registered device. */
    if (dev == NULL)
        return 0;

    if (prev == NULL)
        inputThreadInfo->devs = dev->next;
    else
        prev->next = dev->next;

    FD_CLR(fd, &inputThreadInfo->fds);
    dev->readInputProc = NULL;
    dev->readInputArgs = NULL;
    dev->fd = 0;
    dev = 0;
    free(dev);

    InputThreadFillPipe(hotplugPipeWrite);
    DebugF("input-thread: unregistered device: %d\n", fd);

    return 1;
}

/**
 * The workhorse of threaded input event generation.
 *
 * Or if you prefer: The WaitForSomething for input devices. :)
 *
 * Runs in parallel with the server main thread, listening to input devices in
 * an endless loop. Whenever new input data is made available, calls the
 * proper device driver's routines which are ultimately responsible for the
 * generation of input events.
 *
 * @see InputThreadPreInit()
 * @see InputThreadInit()
 */

static void*
InputThreadDoWork(void *arg)
{
    fd_set readyFds;
    InputThreadDevice *dev;

    FD_ZERO(&readyFds);

    while (1)
    {
        XFD_COPYSET(&inputThreadInfo->fds, &readyFds);
        FD_SET(hotplugPipeRead, &readyFds);

        DebugF("input-thread: InputThreadDoWork waiting for devices\n");

        if (Select(MaxInputDevices, &readyFds, NULL, NULL, NULL) < 0) {
            if (errno == EINVAL)
                FatalError("input-thread: InputThreadDoWork (%s)", strerror(errno));
            else if (errno != EINTR)
                ErrorF("input-thread: InputThreadDoWork (%s)\n", strerror(errno));
        }

        DebugF("input-thread: InputThreadDoWork generating events\n");
        /* Call the device drivers to generate input events for us */
        for (dev = inputThreadInfo->devs; dev != NULL; dev = dev->next)
            if (FD_ISSET(dev->fd, &readyFds) && dev->readInputProc)
                dev->readInputProc(dev->readInputArgs);

        /* Kick main thread to process the generated input events and drain
         * events from hotplug pipe */
        InputThreadFillPipe(inputThreadInfo->writePipe);
        InputThreadReadPipe(hotplugPipeRead);
    }
}

static void
InputThreadWakeup(pointer blockData, int err, pointer pReadmask)
{
    InputThreadReadPipe(inputThreadInfo->readPipe);
}

/**
 * Pre-initialize the facility used for threaded generation of input events
 *
 */
void
InputThreadPreInit(void)
{
    int fds[2], hotplugPipe[2];

    if (pipe(fds) < 0)
        FatalError("input-thread: could not create pipe");

    if (pipe(hotplugPipe) < 0)
        FatalError("input-thread: could not create pipe");

    inputThreadInfo = malloc(sizeof(InputThreadInfo));
    if (!inputThreadInfo)
        FatalError("input-thread: could not allocate memory");

    inputThreadInfo->thread = 0;
    inputThreadInfo->devs = NULL;
    FD_ZERO(&inputThreadInfo->fds);

    /* By making read head non-blocking, we ensure that while the main thread
     * is busy servicing client requests, the dedicated input thread can work
     * in parallel.
     */
    inputThreadInfo->readPipe = fds[0];
    fcntl(inputThreadInfo->readPipe, F_SETFL, O_NONBLOCK | O_CLOEXEC);
    AddGeneralSocket(inputThreadInfo->readPipe);
    RegisterBlockAndWakeupHandlers((BlockHandlerProcPtr)NoopDDA,
    InputThreadWakeup, NULL);

    inputThreadInfo->writePipe = fds[1];

    hotplugPipeRead = hotplugPipe[0];
    fcntl(hotplugPipeRead, F_SETFL, O_NONBLOCK | O_CLOEXEC);
    hotplugPipeWrite = hotplugPipe[1];
}

/**
 * Start the threaded generation of input events. This routine complements what
 * was previously done by InputThreadPreInit(), being only responsible for
 * creating the dedicated input thread.
 *
 */
void
InputThreadInit(void)
{
    pthread_attr_t attr;

    pthread_attr_init(&attr);

    /* For OSes that differentiate between processes and threads, the following
     * lines have sense. Linux uses the 1:1 thread model. The scheduler handles
     * every thread as a normal process. Therefore this probably has no meaning
     * if we are under Linux.
     */
    if (pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM) != 0)
        ErrorF("input-thread: error setting thread scope\n");

    if (pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN) != 0)
        ErrorF("input-thread: error setting thread stack size\n");

    struct sched_param sched_param;
    /* Get default priority */
    pthread_attr_getschedparam(&attr,&sched_param);
    /* Raise it up by 6 increment - prioity goes from 0 to 99 so this is a little over a decreased -2 niceness */
    sched_param.sched_priority = (sched_param.sched_priority+6 < 100) ? sched_param.sched_priority+6 : 99 ;
    /* set the priority of the event thread a bit higher that the rest of the world */
    pthread_attr_setschedparam(&attr,&sched_param);

    DebugF("input-thread: creating thread\n");
    pthread_create(&inputThreadInfo->thread, &attr,&InputThreadDoWork, NULL);

    pthread_attr_destroy (&attr);
}

/**
 * Stop the threaded generation of input events
 *
 * This function is supposed to be called at server shutdown time only.
 */
void
InputThreadFini(void)
{
    InputThreadDevice *dev, *tmp;

    pthread_cancel(inputThreadInfo->thread);
    pthread_join(inputThreadInfo->thread, NULL);

    for (dev = inputThreadInfo->devs; dev != NULL; dev = tmp) {
        tmp = dev->next;
        dev->next = NULL;
        dev->readInputProc = NULL;
        dev->readInputArgs = NULL;
        FD_CLR(dev->fd, &inputThreadInfo->fds);
        dev->fd = -1;
        free(dev);
    }
    inputThreadInfo->devs = NULL;
    FD_ZERO(&inputThreadInfo->fds);

    RemoveGeneralSocket(inputThreadInfo->readPipe);
    RemoveBlockAndWakeupHandlers((BlockHandlerProcPtr)NoopDDA,
    InputThreadWakeup, NULL);
    close(inputThreadInfo->readPipe);
    close(inputThreadInfo->writePipe);
    inputThreadInfo->readPipe = -1;
    inputThreadInfo->writePipe = -1;

    close(hotplugPipeRead);
    close(hotplugPipeWrite);
    hotplugPipeRead = -1;
    hotplugPipeWrite = -1;

    free(inputThreadInfo);
    inputThreadInfo = NULL;
}

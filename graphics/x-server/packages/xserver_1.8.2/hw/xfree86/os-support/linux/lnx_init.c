/*
 * Copyright 1992 by Orest Zborowski <obz@Kodak.com>
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Orest Zborowski and David Wexelblat
 * not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.  Orest Zborowski
 * and David Wexelblat make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *
 * OREST ZBOROWSKI AND DAVID WEXELBLAT DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL OREST ZBOROWSKI OR DAVID WEXELBLAT BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include <X11/Xmd.h>

#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"

#include <sys/stat.h>

static Bool KeepTty = FALSE;
static int VTnum = -1;
static Bool VTSwitch = TRUE;
static Bool ShareVTs = FALSE;
static int activeVT = -1;

static int vtPermSave[4];
static char vtname[11];
static struct termios tty_attr; /* tty state to restore */
static int tty_mode; /* kbd mode to restore */

static int
saveVtPerms(void)
{
    /* We need to use stat to get permissions. */
    struct stat svtp;

    /* Do them numerically ordered, hard coded tty0 first. */
    if (stat("/dev/tty0", &svtp) != 0)
	return 0;
    vtPermSave[0] = (int)svtp.st_uid;
    vtPermSave[1] = (int)svtp.st_gid;

    /* Now check the console we are dealing with. */
    if (stat(vtname, &svtp) != 0)
	return 0;
    vtPermSave[2] = (int)svtp.st_uid;
    vtPermSave[3] = (int)svtp.st_gid;

    return 1;
}

static void
restoreVtPerms(void)
{
    if (geteuid() == 0) {
	 /* Set the terminal permissions back to before we started. */
	 (void)chown("/dev/tty0", vtPermSave[0], vtPermSave[1]);
	 (void)chown(vtname, vtPermSave[2], vtPermSave[3]);
    }
}

static void *console_handler;

static void
drain_console(int fd, void *closure)
{
    tcflush(fd, TCIOFLUSH);
}

void
xf86OpenConsole(void)
{
}

void
xf86CloseConsole(void)
{
}

int
xf86ProcessArgument(int argc, char *argv[], int i)
{
	/*
	 * Keep server from detaching from controlling tty.  This is useful
	 * when debugging (so the server can receive keyboard signals.
	 */
	if (!strcmp(argv[i], "-keeptty"))
	{
		KeepTty = TRUE;
		return(1);
	}
        if (!strcmp(argv[i], "-novtswitch"))
        {
                VTSwitch = FALSE;
                return(1);
        }
        if (!strcmp(argv[i], "-sharevts"))
        {
                ShareVTs = TRUE;
                return(1);
        }
	if ((argv[i][0] == 'v') && (argv[i][1] == 't'))
	{
		if (sscanf(argv[i], "vt%2d", &VTnum) == 0)
		{
			UseMsg();
			VTnum = -1;
			return(0);
		}
		return(1);
	}
	return(0);
}

void
xf86UseMsg(void)
{
	ErrorF("vtXX                   use the specified VT number\n");
	ErrorF("-keeptty               ");
	ErrorF("don't detach controlling tty (for debugging only)\n");
	ErrorF("-novtswitch            don't immediately switch to new VT\n");
	ErrorF("-sharevts              share VTs with another X server\n");
}

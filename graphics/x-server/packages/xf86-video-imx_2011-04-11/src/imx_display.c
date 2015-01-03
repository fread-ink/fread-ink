/*
 * Copyright (C) 2011 Freescale Semiconductor, Inc.  All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation 
 * files (the "Software"), to deal in the Software without 
 * restriction, including without limitation the rights to use, copy, 
 * modify, merge, publish, distribute, sublicense, and/or sell copies 
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS 
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86Crtc.h"
#include "xf86DDC.h"
#include "fbdevhw.h"
#include "inputstr.h"
#include "xorgVersion.h"

#include "imx.h"
#include "imx_display.h"

#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,7,6,0,0)
#include <X11/extensions/dpmsconst.h>
#else
#ifndef DPMS_SERVER
#define DPMS_SERVER 1
#include <X11/extensions/dpms.h>
#undef DPMS_SERVER
#endif
#endif

#define	IMX_USE_XRANDR		0


/* -------------------------------------------------------------------- */

typedef struct {

#if IMX_USE_XRANDR

	DisplayModePtr	modesList;

	/* TODO - maybe don't need to store these? */
	xf86CrtcPtr	crtcPtr;
	xf86OutputPtr	outputPtr;

#else

	int			fbRotate;
	xf86PointerMovedProc*	prevPointerMovedProc;

#endif

} ImxDisplayRec, *ImxDisplayPtr;

#define IMXDISPLAYPTR(imxPtr) ((ImxDisplayPtr)((imxPtr)->displayPrivate))


/* -------------------------------------------------------------------- */

static void
imxRemoveTrailingNewLines(char* str)
{
	int len = strlen(str);

	while ((len > 0) && ('\n' == str[len-1])) {

		str[--len] = '\0';
	}
}

/* -------------------------------------------------------------------- */

static void
imxDisplayPointerMoved(int index, int x, int y)
{
	ScrnInfoPtr pScrn = xf86Screens[index];
	ImxPtr imxPtr = IMXPTR(pScrn);
	ImxDisplayPtr fPtr = IMXDISPLAYPTR(imxPtr);

	int newX, newY;
	switch (fPtr->fbRotate)
	{
		/* 90 degrees CW rotation. */
		case FB_ROTATE_CW:
			newX = pScrn->pScreen->height - y - 1;
			newY = x;
			break;

		/* 90 degrees CCW rotation. */
		case FB_ROTATE_CCW:
			newX = y;
			newY = pScrn->pScreen->width - x - 1;
			break;

		/* 180 degrees UD rotation. */
		case FB_ROTATE_UD:
			newX = pScrn->pScreen->width - x - 1;
			newY = pScrn->pScreen->height - y - 1;
			break;

		/* No rotation. */
		case FB_ROTATE_UR:
		default:
			newX = x;
			newY = y;
			break;
	}

	/* Pass adjusted pointer coords to wrapped PointerMoved function. */
	(*fPtr->prevPointerMovedProc)(index, newX, newY);
}

/* -------------------------------------------------------------------- */

ImxFbType
imxDisplayGetFrameBufferType(struct fb_fix_screeninfo* pFixInfo)
{
	if (0 == strcmp("mxc_epdc_fb", pFixInfo->id)) {
		return ImxFbTypeEPDC;
	}

	if (0 == strcmp("DISP3 BG", pFixInfo->id)) {
		return ImxFbTypeDISP3_BG;
	}

	if (0 == strcmp("DISP3 FG", pFixInfo->id)) {
		return ImxFbTypeDISP3_FG;
	}

	if (0 == strcmp("DISP3 BG_D1", pFixInfo->id)) {
		return ImxFbTypeDISP3_BG_D1;
	}

	return ImxFbTypeUnknown;
}

/* -------------------------------------------------------------------- */

static const char* imxSysnodeNameMonitorInfoArray[] =
{
	"/sys/devices/platform/mxc_ddc.0/",
	"/sys/devices/platform/sii902x.0/"
};
static const int imxSysnodeNameMonitorInfoCount =
	sizeof(imxSysnodeNameMonitorInfoArray) /
		sizeof(imxSysnodeNameMonitorInfoArray[0]);

static xf86OutputStatus
imxDisplayGetCableState(int scrnIndex, const char* fbId)
{
	/* Loop through each sysnode entry looking for the cable state */
	/* for the frame buffer device matching the specified ID. */
	int iEntry;
	for (iEntry = 0; iEntry < imxSysnodeNameMonitorInfoCount; ++iEntry) {

		char sysnodeName[80];

		/* Look for this sysnode entry which contains the id */
		/* of the associated frame buffer device driver. */
		strcpy(sysnodeName, imxSysnodeNameMonitorInfoArray[iEntry]);
		strcat(sysnodeName, "fb_name");
		FILE* fp = fopen(sysnodeName, "r");
		if (NULL == fp) {

			continue;
		}

		/* The name of the frame buffer device */
		char linebuf[80] = "";
		const BOOL bNoName =
			(NULL == fgets(linebuf, sizeof(linebuf), fp));
		fclose(fp);
		if (bNoName || (0 != strncmp(linebuf, fbId, strlen(fbId)))) {

			continue;
		}

		/* Look for sysnode entry which contains cable state info. */
		strcpy(sysnodeName, imxSysnodeNameMonitorInfoArray[iEntry]);
		strcat(sysnodeName, "cable_state");
		fp = fopen(sysnodeName, "r");
		if (NULL == fp) {

			continue;
		}

		/* Read the line that contains the cable state. */
		char strCableState[80];
		strcpy(strCableState, "");
		const Bool bNoInfo =
			(NULL == fgets(strCableState, sizeof(strCableState), fp));
		fclose(fp);
		if (bNoInfo) {

			break;
		}
	
		imxRemoveTrailingNewLines(strCableState);

		/* Determine cable state from the string. */
		if (0 == strcmp(strCableState, "plugin")) {

			return XF86OutputStatusConnected;

		} else if (0 == strcmp(strCableState, "plugout")) {

			return XF86OutputStatusDisconnected;
		}

		/* No need to keep looking. Found file we were looking for. */
		break;
	}

	return XF86OutputStatusUnknown;
}

static xf86MonPtr
imxDisplayGetEdid(int scrnIndex, const char* fbId)
{
	/* Buffer for reading the EDID info */
	static const int EdidInfoBlockNumBytes = 128;
	Uchar edidInfoBlock[EdidInfoBlockNumBytes];

	/* Loop through each sysnode entry looking for the EDID info. */
	int iEntry;
	for (iEntry = 0; iEntry < imxSysnodeNameMonitorInfoCount; ++iEntry) {

		char sysnodeName[80];

		/* Look for this sysnode entry which contains the id */
		/* of the associated frame buffer device driver. */
		strcpy(sysnodeName, imxSysnodeNameMonitorInfoArray[iEntry]);
		strcat(sysnodeName, "fb_name");
		FILE* fp = fopen(sysnodeName, "r");
		if (NULL == fp) {

			continue;
		}

		/* The name of the frame buffer device */
		char linebuf[80] = "";
		const BOOL bNoName =
			(NULL == fgets(linebuf, sizeof(linebuf), fp));
		fclose(fp);
		if (bNoName || (0 != strncmp(linebuf, fbId, strlen(fbId)))) {

			continue;
		}

		/* Look for this sysnode entry which contains EDID info. */
		strcpy(sysnodeName, imxSysnodeNameMonitorInfoArray[iEntry]);
		strcat(sysnodeName, "edid");
		fp = fopen(sysnodeName, "r");
		if (NULL == fp) {

			continue;
		}

		/* The bytes in the sysnode entry are stored in */
		/* ASCII 0x%02x format. */
		unsigned int byte;
		int nBytes;
		for (nBytes = 0; nBytes < EdidInfoBlockNumBytes; ++nBytes) {

			if (1 != fscanf(fp, "%i", &byte)) {
				break;
			}

			edidInfoBlock[nBytes] = byte;
		}
		fclose(fp);

		/* Were all the bytes successfully read? */
		if (EdidInfoBlockNumBytes != nBytes) {

			xf86DrvMsg(scrnIndex, X_ERROR,
		   		"sysnode '%s' contains only %d of %d bytes\n",
				sysnodeName, nBytes, EdidInfoBlockNumBytes);

			continue;
		}

		/* Interpret the EDID monitor info. */
		xf86MonPtr pMonitor =
			xf86InterpretEDID(scrnIndex, edidInfoBlock);
		if (NULL == pMonitor) {

			xf86DrvMsg(scrnIndex, X_ERROR,
		   		"cannot interpret EDID info in sysnode '%s'\n",
				sysnodeName);

			continue;
		}

		return pMonitor;
	}

	return NULL;
}

/* -------------------------------------------------------------------- */

static Bool
imxDisplayGetCurrentModeName(ScrnInfoPtr pScrn, const char* fbDeviceName,
				char modeName[], size_t modeNameSize)
{
	/* Create the name of the sysnode file that contains the */
	/* name of the currently selected mode. */
	char sysnodeName[80];
	sprintf(sysnodeName, "/sys/class/graphics/%s/mode", fbDeviceName);
	FILE* fp = fopen(sysnodeName, "r");
	if (NULL == fp) {

		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	   		"unable to open sysnode '%s': %s\n",
			sysnodeName, strerror(errno));
		return FALSE;
	}

	/* Read current mode name */
	if (NULL == fgets(modeName, modeNameSize, fp)) {
		
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	   		"unable to read from sysnode '%s': %s\n",
			sysnodeName, strerror(errno));
		return FALSE;
	}

	imxRemoveTrailingNewLines(modeName);

	fclose(fp);
	return TRUE;
}

static Bool
imxDisplaySetCurrentMode(ScrnInfoPtr pScrn, const char* fbDeviceName,
				const char* modeName)
{
	/* Create the name of the sysnode file that contains the */
	/* name of the currently selected mode. */
	char sysnodeName[80];
	sprintf(sysnodeName, "/sys/class/graphics/%s/mode", fbDeviceName);
	int fd = open(sysnodeName, O_RDWR);
	if (-1 == fd) {

		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	   		"unable to open sysnode '%s':%s\n",
			sysnodeName, strerror(errno));
		return FALSE;
	}

	/* Make sure the mode name has a newline at end on the write. */
	char validModeName[80];
	strcpy(validModeName, modeName);
	strcat(validModeName, "\n");

	/* Write the desired mode name */
	if (-1 == write(fd, validModeName, strlen(validModeName))) {
		
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	   		"unable to write to sysnode '%s': %s\n",
			sysnodeName, strerror(errno));
		return FALSE;
	}

	close(fd);
	return TRUE;
}

/* -------------------------------------------------------------------- */

static void
imxConvertFrameBufferTiming(struct fb_var_screeninfo *var, DisplayModePtr mode)
{
	mode->Clock = var->pixclock ? 1000000000/var->pixclock : 0;
	mode->HDisplay = var->xres;
	mode->HSyncStart = mode->HDisplay+var->right_margin;
	mode->HSyncEnd = mode->HSyncStart+var->hsync_len;
	mode->HTotal = mode->HSyncEnd+var->left_margin;
	mode->VDisplay = var->yres;
	mode->VSyncStart = mode->VDisplay+var->lower_margin;
	mode->VSyncEnd = mode->VSyncStart+var->vsync_len;
	mode->VTotal = mode->VSyncEnd+var->upper_margin;
	mode->Flags = 0;
	mode->Flags |= var->sync & FB_SYNC_HOR_HIGH_ACT ? V_PHSYNC : V_NHSYNC;
	mode->Flags |= var->sync & FB_SYNC_VERT_HIGH_ACT ? V_PVSYNC : V_NVSYNC;
	mode->Flags |= var->sync & FB_SYNC_COMP_HIGH_ACT ? V_PCSYNC : V_NCSYNC;
	if (var->sync & FB_SYNC_BROADCAST)
		mode->Flags |= V_BCAST;
	if ((var->vmode & FB_VMODE_MASK) == FB_VMODE_INTERLACED)
		mode->Flags |= V_INTERLACE;
	else if ((var->vmode & FB_VMODE_MASK) == FB_VMODE_DOUBLE)
		mode->Flags |= V_DBLSCAN;
	mode->SynthClock = mode->Clock;
	mode->CrtcHDisplay = mode->HDisplay;
	mode->CrtcHSyncStart = mode->HSyncStart;
	mode->CrtcHSyncEnd = mode->HSyncEnd;
	mode->CrtcHTotal = mode->HTotal;
	mode->CrtcVDisplay = mode->VDisplay;
	mode->CrtcVSyncStart = mode->VSyncStart;
	mode->CrtcVSyncEnd = mode->VSyncEnd;
	mode->CrtcVTotal = mode->VTotal;
	mode->CrtcHAdjusted = FALSE;
	mode->CrtcVAdjusted = FALSE;
}

/* -------------------------------------------------------------------- */

static DisplayModePtr
imxDisplayGetCurrentMode(ScrnInfoPtr pScrn, int fd, const char* modeName)
{
	/* Query the frame buffer variable screen info. */
	struct fb_var_screeninfo fbVarScreenInfo;
	if (-1 == ioctl(fd, FBIOGET_VSCREENINFO, &fbVarScreenInfo)) {

		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			"unable to get VSCREENINFO for mode '%s': %s\n",
			modeName, strerror(errno));
		return NULL;
	}
		
	/* Allocate a new mode structure. */
	DisplayModePtr mode = xalloc(sizeof(DisplayModeRec));

	/* Transfer info from fbdev var screen info */
	/* into the X DisplayModeRec. */
	imxConvertFrameBufferTiming(&fbVarScreenInfo, mode);

	/* Add the new mode to the list. */
	mode->type = M_T_DRIVER;
	mode->status = MODE_OK;
	mode->name = xstrdup(modeName);
	mode->prev = NULL;
	mode->next = NULL;

	return mode;
}

static DisplayModePtr
imxDisplayGetModes(ScrnInfoPtr pScrn, const char* fbDeviceName)
{
	FILE* fpModes = NULL;
	int fdDev = -1;
	DisplayModePtr modesList = NULL;
	char saveModeName[80];

	/* Query the current frame buffer mode. */
	strcpy(saveModeName, "");
	if (!imxDisplayGetCurrentModeName(
		pScrn, fbDeviceName, saveModeName, sizeof(saveModeName))) {

		goto errorGetModes;
	}

	/* Create the name of the sysnode file that contains the */
	/* names of all the frame buffer modes. */
	char sysnodeName[80];
	sprintf(sysnodeName, "/sys/class/graphics/%s/modes", fbDeviceName);
	fpModes = fopen(sysnodeName, "r");
	if (NULL == fpModes) {

		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	   		"unable to open sysnode '%s':%s \n",
			sysnodeName, strerror(errno));
		goto errorGetModes;
	}

	/* Create name for the frame buffer device. */
	char fullDeviceName[80];
	strcpy(fullDeviceName, "/dev/");
	strcat(fullDeviceName, fbDeviceName);

	/* Access the frame buffer device. */
	fdDev = fbdevHWGetFD(pScrn);
	if (-1 == fdDev) {

		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	   		"frame buffer device not available or initialized\n");
		goto errorGetModes;
	}

	/* Turn on frame buffer blanking. */
	if (-1 == ioctl(fdDev, FBIOBLANK, FB_BLANK_NORMAL)) {

		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	   		"unable to blank frame buffer device '%s': %s\n",
			fullDeviceName, strerror(errno));
		goto errorGetModes;
	}

	/* Iterate over all the modes in the frame buffer list. */
	char modeName[80];
	while (NULL != fgets(modeName, sizeof(modeName), fpModes)) {

		imxRemoveTrailingNewLines(modeName);

		/* Attempt to set the mode */
		if (!imxDisplaySetCurrentMode(pScrn, fbDeviceName, modeName)) {

			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	   			"unable to set frame buffer mode '%s'\n",
				modeName);
			continue;
		}

#if 1
		xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	   			"found frame buffer mode: %s\n", modeName);
#endif

		DisplayModePtr mode =
			imxDisplayGetCurrentMode(pScrn, fdDev, modeName);

		if (NULL != mode) {

			modesList = xf86ModesAdd(modesList, mode);
		}
	}

errorGetModes:

	/* Close file with list of modes. */
	if (NULL != fpModes) {

		fclose(fpModes);
	}

	/* Restore the current mode name upon entering this function. */
	if (0 < strlen(saveModeName)) {

		imxDisplaySetCurrentMode(pScrn, fbDeviceName, saveModeName);
	}

	/* Turn off frame buffer blanking */
	if (-1 != fdDev) {

		ioctl(fdDev, FBIOBLANK, FB_BLANK_UNBLANK);
	}

	return modesList;
}

static void
imxDisplayDeleteModes(DisplayModePtr modesList)
{
	while (NULL != modesList) {

		DisplayModePtr mode = modesList;

		modesList = mode->next;
		if (modesList == mode) {
			modesList = NULL;
		}

		if (NULL != mode->name) {
			xfree(mode->name);
		}
		xfree(mode);
	}
}

static void
imxDisplaySetSizeRange(ScrnInfoPtr pScrn, DisplayModePtr modesList)
{
	if (NULL == modesList) {
		return;
	}

	DisplayModePtr mode = modesList;

	int minWidth = mode->HDisplay;
	int maxWidth = minWidth;

	int minHeight = mode->VDisplay;
	int maxHeight = minHeight; 

	while (NULL != (mode = mode->next)) {

		if (mode->HDisplay < minWidth) {

			minWidth = mode->HDisplay;

		} else if (mode->HDisplay > maxWidth) {

			maxWidth = mode->HDisplay;
		}

		if (mode->VDisplay < minHeight) {

			minHeight = mode->VDisplay;

		} else if (mode->VDisplay > maxHeight) {

			maxHeight = mode->VDisplay;
		}
	}

	xf86CrtcSetSizeRange(pScrn, minWidth, minHeight, maxWidth, maxHeight);
}

/* -------------------------------------------------------------------- */

static Bool
imxCrtcResize(ScrnInfoPtr pScrn, int width, int height)
{
	/* TODO */
	return TRUE;
}

static const xf86CrtcConfigFuncsRec imxCrtcConfigFuncs = {
	.resize = imxCrtcResize
};

static void
imxCrtcDPMS(xf86CrtcPtr crtc, int mode)
{
	/* TODO */
}

static Bool
imxCrtcLock(xf86CrtcPtr crtc)
{
	/* TODO */
	return TRUE;
}

static void
imxCrtcUnlock(xf86CrtcPtr crtc)
{
	/* TODO */
}

static Bool
imxCrtcModeFixup(xf86CrtcPtr crtc, DisplayModePtr mode, DisplayModePtr adjMode)
{
	/* TODO */
	return TRUE;
}

static void
imxCrtcPrepare(xf86CrtcPtr crtc)
{
	/* TODO - blank display before changing modes? */
}

static void
imxCrtcModeSet(xf86CrtcPtr crtc, DisplayModePtr mode, DisplayModePtr adjMode,
		int x, int y)
{
	/* TODO */
}

static void
imxCrtcCommit(xf86CrtcPtr crtc)
{
	/* TODO - unblank display after changing modes? */
}

static void
imxCrtcDestroy(xf86CrtcPtr crtc)
{
	/* TODO */
}

static const xf86CrtcFuncsRec imxCrtcFuncs = {
	.dpms = imxCrtcDPMS,
	.lock = imxCrtcLock,
	.unlock = imxCrtcUnlock,
	.mode_fixup = imxCrtcModeFixup,
	.prepare = imxCrtcPrepare,
	.mode_set = imxCrtcModeSet,
	.commit = imxCrtcCommit,
//	.gamma_set = imxCrtcGammaSet,
	.destroy = imxCrtcDestroy
};

/* -------------------------------------------------------------------- */

static void
imxOutputCreateResources(xf86OutputPtr output)
{
	/* TODO */
}

static void
imxOutputDPMS(xf86OutputPtr output, int mode)
{
	/* Access the associated screen info. */
	ScrnInfoPtr pScrn = output->scrn;

	/* Access the frame buffer driver */
	int fd = fbdevHWGetFD(pScrn);
	if (-1 != fd) {

		/* Enable power */
		if (DPMSModeOn == mode) {

			ioctl(fd, FBIOBLANK, FB_BLANK_UNBLANK);

		/* Unsupported intermediate modes drop to lower power setting */
		} else {

			ioctl(fd, FBIOBLANK, FB_BLANK_NORMAL);
		}
	}
}

static void
imxOutputSave(xf86OutputPtr output)
{
	/* TODO */
}

static void
imxOutputRestore(xf86OutputPtr output)
{
	/* TODO */
}

static int
imxOutputModeValid(xf86OutputPtr output, DisplayModePtr mode)
{
	/* TODO */
	return MODE_OK;
}

static Bool
imxOutputModeFixup(xf86OutputPtr output, DisplayModePtr mode,
			DisplayModePtr adjMode)
{
	/* TODO */
	return TRUE;
}

static void
imxOutputPrepare(xf86OutputPtr output)
{
	/* TODO */
}

static void
imxOutputModeSet(xf86OutputPtr output, DisplayModePtr mode,
			DisplayModePtr adjMode)
{
	/* TODO */
}

static void
imxOutputCommit(xf86OutputPtr output)
{
	/* TODO */
}

static xf86OutputStatus
imxOutputDetect(xf86OutputPtr output)
{
	/* Access the associated screen info. */
	ScrnInfoPtr pScrn = output->scrn;

	/* Access driver private screen data */
	ImxPtr imxPtr = IMXPTR(pScrn);

	return imxDisplayGetCableState(pScrn->scrnIndex, imxPtr->fbId);
}

static DisplayModePtr
imxOutputGetModes(xf86OutputPtr output)
{
	/* Access the associated screen info. */
	ScrnInfoPtr pScrn = output->scrn;

	/* Access driver private screen data */
	ImxPtr imxPtr = IMXPTR(pScrn);

	/* Access driver private screen display data */
	ImxDisplayPtr fPtr = IMXDISPLAYPTR(imxPtr);

	/* Try to read the monitor EDID info. */
	xf86MonPtr pMonitor =
		imxDisplayGetEdid(pScrn->scrnIndex, imxPtr->fbId);
	if (NULL != pMonitor) {

		xf86OutputSetEDID(output, pMonitor);
	}

	/* Access all the modes support by frame buffer driver. */
#if 0
	return xf86DuplicateModes(pScrn, fPtr->modesList);
#else
	return imxDisplayGetModes(pScrn, imxPtr->fbDeviceName);
#endif
}

static void
imxOutputDestroy(xf86OutputPtr output)
{
	/* TODO */
}

static const xf86OutputFuncsRec imxOutputFuncs = {
	.create_resources = imxOutputCreateResources,
	.dpms = imxOutputDPMS,
	.save = imxOutputSave,
	.restore = imxOutputRestore,
	.mode_valid = imxOutputModeValid,
	.mode_fixup = imxOutputModeFixup,
	.prepare = imxOutputPrepare,
	.mode_set = imxOutputModeSet,
	.commit = imxOutputCommit,
	.detect = imxOutputDetect,
	.get_modes = imxOutputGetModes,
//	.set_property = imxOutputSetProperty,
	.destroy = imxOutputDestroy
};

/* -------------------------------------------------------------------- */

Bool
imxDisplayPreInit(ScrnInfoPtr pScrn)
{
	/* Access driver private screen data */
	ImxPtr imxPtr = IMXPTR(pScrn);

	/* Private data structure must not already be in use. */
	if (NULL != imxPtr->displayPrivate) {
		return FALSE;
	}
	
	/* Allocate memory for display private data */
	imxPtr->displayPrivate = xcalloc(sizeof(ImxDisplayRec), 1);
	if (NULL == imxPtr->displayPrivate) {
		return FALSE;
	}
	ImxDisplayPtr fPtr = IMXDISPLAYPTR(imxPtr);

	fPtr->fbRotate = FB_ROTATE_UR;
	fPtr->prevPointerMovedProc = NULL;

#if IMX_USE_XRANDR

	/* Initialize display private data structure. */
	fPtr->modesList = NULL;
	fPtr->crtcPtr = NULL;
	fPtr->outputPtr = NULL;

#if 0
	/* Access all the modes support by frame buffer driver. */
	fPtr->modesList = imxDisplayGetModes(pScrn, imxPtr->fbDeviceName);
#endif

	xf86CrtcConfigInit(pScrn, &imxCrtcConfigFuncs);
	imxDisplaySetSizeRange(pScrn, fPtr->modesList);

	/* Allocate and initialize CRTC */
	fPtr->crtcPtr = xf86CrtcCreate(pScrn, &imxCrtcFuncs);
	if (NULL == fPtr->crtcPtr) {

		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				"xf86CrtcCreate failed\n");
		return FALSE;
	}

	/* Allocate and initialize output */
	fPtr->outputPtr =
		xf86OutputCreate(pScrn, &imxOutputFuncs, imxPtr->fbId);
	if (NULL == fPtr->outputPtr) {

		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				"xf86OutputCreate failed\n");
		return FALSE;
	}
	fPtr->outputPtr->possible_crtcs = 1;

	/* Compute initial configuration */
	const Bool bCanGrow = TRUE;
	if (!xf86InitialConfiguration(pScrn, bCanGrow)) {

		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				"xf86InitialConfiguration failed\n");
		return FALSE;
	}

#else
	/* Select video modes */
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		"checking modes against framebuffer device...\n");
	fbdevHWSetVideoModes(pScrn);

	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		"checking modes against monitor...\n");
	{
		DisplayModePtr mode, first = mode = pScrn->modes;
		
		if (mode != NULL) do {
			mode->status = xf86CheckModeForMonitor(mode, pScrn->monitor);
			mode = mode->next;
		} while (mode != NULL && mode != first);

		xf86PruneDriverModes(pScrn);
	}

	if (NULL == pScrn->modes) {
		fbdevHWUseBuildinMode(pScrn);
	}
	pScrn->currentMode = pScrn->modes;
#endif

	return TRUE;
}

Bool
imxDisplayStartScreenInit(int scrnIndex, ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	ImxPtr fPtr = IMXPTR(pScrn);

#if IMX_USE_XRANDR

	if (!xf86SetDesiredModes(pScrn)) {

		xf86DrvMsg(scrnIndex, X_ERROR, "mode initialization failed\n");
		return FALSE;
	}

#else

	fbdevHWSave(pScrn);

	if (!fbdevHWModeInit(pScrn, pScrn->currentMode)) {

		xf86DrvMsg(scrnIndex, X_ERROR, "mode initialization failed\n");
		return FALSE;
	}

	fbdevHWSaveScreen(pScreen, SCREEN_SAVER_ON);
	fbdevHWAdjustFrame(scrnIndex,0,0,0);

#endif

	return TRUE;
}

Bool
imxDisplayFinishScreenInit(int scrnIndex, ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	ImxPtr imxPtr = IMXPTR(pScrn);
	ImxDisplayPtr fPtr = IMXDISPLAYPTR(imxPtr);

#if IMX_USE_XRANDR

	/* Completes the screen initialization for outputs and CRTCs */
	if (!xf86CrtcScreenInit(pScreen)) {
		xf86DrvMsg(scrnIndex, X_ERROR, "xf86CrtcScreenInit failed\n");
		return FALSE;
	}

	/* All DPMS mode switching will be managed by using the dpms */
	/* DPMS functions provided by the outputs and CRTCs */
	xf86DPMSInit(pScreen, xf86DPMSSet, 0);

#else

//	fPtr->prevPointerMovedProc = pScrn->PointerMoved;
//	pScrn->PointerMoved = imxDisplayPointerMoved;

#endif

	return TRUE;
}

/* -------------------------------------------------------------------- */

Bool
imxDisplaySwitchMode(int scrnIndex, DisplayModePtr mode, int flags)
{
#if IMX_USE_XRANDR

	return TRUE;

#else

	return fbdevHWSwitchMode(scrnIndex, mode, flags);

#endif
}

void
imxDisplayAdjustFrame(int scrnIndex, int x, int y, int flags)
{
#if IMX_USE_XRANDR

#else

	fbdevHWAdjustframe(scrnIndex, x, y, flags);

#endif
}

Bool
imxDisplayEnterVT(int scrnIndex, int flags)
{
#if IMX_USE_XRANDR

	return TRUE;

#else

	return fbdevHWEnterVT(scrnIndex, flags);

#endif
}

void
imxDisplayLeaveVT(int scrnIndex, int flags)
{
#if IMX_USE_XRANDR

#else

	fbdevHWLeaveVT(scrnIndex, flags);

#endif
}

ModeStatus
imxDisplayValidMode(int scrnIndex, DisplayModePtr mode, Bool verbose, int flags)
{
#if IMX_USE_XRANDR

	return MODE_OK;

#else

	return fbdevHWValidMode(scrnIndex, mode, verbose, flags);

#endif
}

/* -------------------------------------------------------------------- */

/* ------------------------------------------------------------------------ */
/* THE FOLLOWING CODE TO BE REMOVED WHEN XRANDR IMPLEMENTATION IS COMPLETED */
/* ------------------------------------------------------------------------ */

static Bool
xf86SwitchModePriv(ScreenPtr pScreen, DisplayModePtr mode)
{
  ScrnInfoPtr pScr = XF86SCRNINFO(pScreen);
  ScreenPtr   pCursorScreen;
  Bool        Switched;
  int         px, py;
  DeviceIntPtr dev, it;

  if (!pScr->vtSema || !mode || !pScr->SwitchMode)
    return FALSE;

#ifdef XFreeXDGA
//  if (DGAActive(pScr->scrnIndex))
//    return FALSE;
#endif

  if (mode == pScr->currentMode)
    return TRUE;

  if (mode->HDisplay > pScr->virtualX || mode->VDisplay > pScr->virtualY)
    return FALSE;

  /* Let's take an educated guess for which pointer to take here. And about as
     educated as it gets is to take the first pointer we find.
   */
  for (dev = inputInfo.devices; dev; dev = dev->next)
  {
      if (IsPointerDevice(dev) && dev->spriteInfo->spriteOwner)
          break;
  }

  pCursorScreen = miPointerGetScreen(dev);
  if (pScreen == pCursorScreen)
    miPointerGetPosition(dev, &px, &py);

//  xf86EnterServerState(SETUP);
  Switched = TRUE; // (*pScr->SwitchMode)(pScr->scrnIndex, mode, 0);
  if (Switched) {
    pScr->currentMode = mode;

    /*
     * Adjust frame for new display size.
     * Frame is centered around cursor position if cursor is on same screen.
     */
//    if (pScreen == pCursorScreen)
//      pScr->frameX0 = px - (mode->HDisplay / 2) + 1;
//    else
      pScr->frameX0 = (pScr->frameX0 + pScr->frameX1 + 1 - mode->HDisplay) / 2;

    if (pScr->frameX0 < 0)
      pScr->frameX0 = 0;

    pScr->frameX1 = pScr->frameX0 + mode->HDisplay - 1;
    if (pScr->frameX1 >= pScr->virtualX) {
      pScr->frameX0 = pScr->virtualX - mode->HDisplay;
      pScr->frameX1 = pScr->virtualX - 1;
    }

//    if (pScreen == pCursorScreen)
//      pScr->frameY0 = py - (mode->VDisplay / 2) + 1;
//    else
      pScr->frameY0 = (pScr->frameY0 + pScr->frameY1 + 1 - mode->VDisplay) / 2;

    if (pScr->frameY0 < 0)
      pScr->frameY0 = 0;

    pScr->frameY1 = pScr->frameY0 + mode->VDisplay - 1;
    if (pScr->frameY1 >= pScr->virtualY) {
      pScr->frameY0 = pScr->virtualY - mode->VDisplay;
      pScr->frameY1 = pScr->virtualY - 1;
    }
  }
//  xf86EnterServerState(OPERATING);

//  if (pScr->AdjustFrame)
//    (*pScr->AdjustFrame)(pScr->scrnIndex, pScr->frameX0, pScr->frameY0, 0);

  /* The original code centered the frame around the cursor if possible.
   * Since this is hard to achieve with multiple cursors, we do the following:
   *   - center around the first pointer
   *   - move all other pointers to the nearest edge on the screen (or leave
   *   them unmodified if they are within the boundaries).
   */
//  if (pScreen == pCursorScreen)
//  {
//      xf86WarpCursor(dev, pScreen, px, py);
//  }
  if (pScreen == pCursorScreen) {
    px = 0;
    py = 0;
    miPointerSetPosition(dev, &px, &py);
  }

//  for (it = inputInfo.devices; it; it = it->next)
//  {
//      if (it == dev)
//          continue;
//
//      if (IsPointerDevice(it) && it->spriteInfo->spriteOwner)
//      {
//          pCursorScreen = miPointerGetScreen(it);
//          if (pScreen == pCursorScreen)
//          {
//              miPointerGetPosition(it, &px, &py);
//              if (px < pScr->frameX0)
//                  px = pScr->frameX0;
//              else if (px > pScr->frameX1)
//                  px = pScr->frameX1;
//
//              if(py < pScr->frameY0)
//                  py = pScr->frameY0;
//              else if(py > pScr->frameY1)
//                  py = pScr->frameY1;
//
//              xf86WarpCursor(it, pScreen, px, py);
//          }
//      }
//  }

  return Switched;
}
    
static void
privSendConfigNotify (ScreenPtr pScreen)
{
    WindowPtr	pWin = WindowTable[pScreen->myNum];
    xEvent	event;

    event.u.u.type = ConfigureNotify;
    event.u.configureNotify.window = pWin->drawable.id;
    event.u.configureNotify.aboveSibling = None;
    event.u.configureNotify.x = 0;
    event.u.configureNotify.y = 0;

    /* XXX xinerama stuff ? */
    
    event.u.configureNotify.width = pWin->drawable.width;
    event.u.configureNotify.height = pWin->drawable.height;
    event.u.configureNotify.borderWidth = wBorderWidth (pWin);
    event.u.configureNotify.override = pWin->overrideRedirect;
    DeliverEvents(pWin, &event, 1, NullWindow);
}

static void
privEditConnectionInfo (ScreenPtr pScreen)
{
    xConnSetup	    *connSetup;
    char	    *vendor;
    xPixmapFormat   *formats;
    xWindowRoot	    *root;
    xDepth	    *depth;
    xVisualType	    *visual;
    int		    screen = 0;
    int		    d;

    static const int padlength[4] = {0, 3, 2, 1};

    connSetup = (xConnSetup *) ConnectionInfo;
    vendor = (char *) connSetup + sizeof (xConnSetup);
    formats = (xPixmapFormat *) ((char *) vendor +
				 connSetup->nbytesVendor +
				 padlength[connSetup->nbytesVendor & 3]);
    root = (xWindowRoot *) ((char *) formats +
			    sizeof (xPixmapFormat) * screenInfo.numPixmapFormats);
    while (screen != pScreen->myNum)
    {
	depth = (xDepth *) ((char *) root + 
			    sizeof (xWindowRoot));
	for (d = 0; d < root->nDepths; d++)
	{
	    visual = (xVisualType *) ((char *) depth +
				      sizeof (xDepth));
	    depth = (xDepth *) ((char *) visual +
				depth->nVisuals * sizeof (xVisualType));
	}
	root = (xWindowRoot *) ((char *) depth);
	screen++;
    }
    root->pixWidth = pScreen->width;
    root->pixHeight = pScreen->height;
    root->mmWidth = pScreen->mmWidth;
    root->mmHeight = pScreen->mmHeight;
}

Bool
imxDisplayChangeFrameBufferRotateEPDC(int scrnIndex, int fbRotate)
{
	/* Make sure the screen index is valid */
	if ((0 > scrnIndex) || (scrnIndex >= xf86NumScreens)) {

		return FALSE;
	}

	/* Access the screen info */
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	if (NULL == pScrn) {

		return FALSE;
	}

	/* Access driver private screen data */
	ImxPtr imxPtr = IMXPTR(pScrn);

	/* Access driver private screen display data */
	ImxDisplayPtr fPtr = IMXDISPLAYPTR(imxPtr);

	/* Access the file descriptor associated with the open frame */
	/* buffer driver for this screen. */
	int fd = fbdevHWGetFD(pScrn);
	if (-1 == fd) {

		return FALSE;
	}

	/* Request the frame buffer fixed info. */
	struct fb_fix_screeninfo fbFixScreenInfo;
	if (-1 == ioctl(fd, FBIOGET_FSCREENINFO, &fbFixScreenInfo)) {

		return FALSE;
	}

	/* Determine if the EPDC frame buffer driver is in use. */
	if (ImxFbTypeEPDC != imxDisplayGetFrameBufferType(&fbFixScreenInfo)) {

		return FALSE;
	}

	/* Request the frame buffer variable info. */
	struct fb_var_screeninfo fbVarScreenInfo;
	if (-1 == ioctl(fd, FBIOGET_VSCREENINFO, &fbVarScreenInfo)) {

		return FALSE;
	}

	/* Nothing to do if the requested rotation is already set. */
	if (fbRotate == fbVarScreenInfo.rotate) {

		return TRUE;
	}

	/* Note if the screen rotation is different by 90 degrees. */
	const Bool bWasURorUD =
		(FB_ROTATE_UR == fbVarScreenInfo.rotate) ||
		(FB_ROTATE_UD == fbVarScreenInfo.rotate);
	const Bool bNowURorUD =
		(FB_ROTATE_UR == fbRotate) || (FB_ROTATE_UD == fbRotate);
	const Bool bRotate90 = (bWasURorUD != bNowURorUD);

	/* Need to blank the frame buffer output. */
	if (-1 == ioctl(fd, FBIOBLANK, FB_BLANK_NORMAL)) {

		return FALSE;
	}

	/* Attempt to set the desired rotation. */
	Bool success = TRUE;
	fbVarScreenInfo.rotate = fbRotate;
	if (-1 == ioctl(fd, FBIOPUT_VSCREENINFO, &fbVarScreenInfo)) {

		success = FALSE;
	}

	/* Need to turn off blanking regardless. */
	ioctl(fd, FBIOBLANK, FB_BLANK_UNBLANK);
	if (!success) {

		return FALSE;
	}

	/* Re-acquire the frame buffer fixed and variable info. */
	if (-1 == ioctl(fd, FBIOGET_FSCREENINFO, &fbFixScreenInfo)) {

		return FALSE;
	}
	if (-1 == ioctl(fd, FBIOGET_VSCREENINFO, &fbVarScreenInfo)) {

		return FALSE;
	}

	/* Save the new rotation mode. */
	fPtr->fbRotate = fbRotate;

	/* Access pointer to frame buffer driver built in mode */
	DisplayModePtr modeBuiltIn = fbdevHWGetBuildinMode(pScrn);

	/* Destroy the old video modes, unless built-in mode. */
	if (modeBuiltIn != pScrn->modes) {

		imxDisplayDeleteModes(pScrn->modes);
	}
	pScrn->modes = NULL;
	pScrn->currentMode = NULL;

	/* Get current mode from frame buffer driver. */
	DisplayModePtr mode =
		imxDisplayGetCurrentMode(pScrn, fbdevHWGetFD(pScrn), "current");
	if (NULL == mode) {
		return FALSE;
	}

	/* Save this as the new built in mode. */
	/* Do similar setup for built in mode saved in fbdevHW code as */
	/* is done in fbdevHWInit. */
	*modeBuiltIn = *mode;
	modeBuiltIn->name = "current";
	modeBuiltIn->next  = modeBuiltIn;
	modeBuiltIn->prev  = modeBuiltIn;
	modeBuiltIn->type |= M_T_BUILTIN;

	/* Select video modes */
	xf86DrvMsg(scrnIndex, X_INFO,
			"checking modes against framebuffer device...\n");
	fbdevHWSetVideoModes(pScrn);

	xf86DrvMsg(scrnIndex, X_INFO, "checking modes against monitor...\n");
	{
		DisplayModePtr mode, first = mode = pScrn->modes;
		
		if (mode != NULL) do {
			mode->status =
				xf86CheckModeForMonitor(mode, pScrn->monitor);
			mode = mode->next;
		} while (mode != NULL && mode != first);

		xf86PruneDriverModes(pScrn);
	}
	if (NULL == pScrn->modes) {
		fbdevHWUseBuildinMode(pScrn);
	}

	pScrn->EnableDisableFBAccess(scrnIndex, FALSE);

	xf86PrintModes(pScrn);
	fbdevHWSave(pScrn);

	/* Access the screen data structure */
	ScreenPtr pScreen = pScrn->pScreen;

	/* Access the screen pixmap */
	PixmapPtr pScreenPixmap = (*pScreen->GetScreenPixmap)(pScreen);

	 pScrn->virtualX = mode->HDisplay;
	 pScrn->virtualY = mode->VDisplay;

	/* Need to swap width/height if rotation different by 90 degrees. */
	if (bRotate90) {

		/* compute bytes per pixel */
		const int bytesPerPixel =
			(fbVarScreenInfo.bits_per_pixel + 7) / 8;

		int stride;
		if (fbFixScreenInfo.line_length > 0) {

			stride = fbFixScreenInfo.line_length;
		} else {

			stride = fbVarScreenInfo.xres_virtual * bytesPerPixel;
		}

		pScrn->displayWidth = stride / bytesPerPixel;

		int oldWidth = pScreen->width;
		int oldHeight = pScreen->height;

		pScreen->width = oldHeight;
		pScreen->height = oldWidth;

		int oldmmWidth = pScreen->mmWidth;
		int oldmmHeight = pScreen->mmHeight;

		pScreen->mmWidth = oldmmHeight;
		pScreen->mmHeight = oldmmWidth;

		/* Change the width, height, and stride for screen pixmap. */
		(*pScreen->ModifyPixmapHeader)(
			pScreenPixmap,
			pScreen->width,
			pScreen->height,
			-1,			/* do not change depth */
			-1, 			/* do not change bitsperpixel */
			stride,			/* devKind = stride */
			NULL);			/* do not change memory ptr */
	} else {

	}


//	fbdevHWSaveScreen(pScreen, SCREEN_SAVER_ON);

	if (!xf86SwitchModePriv(pScreen, mode)) {

		return FALSE;
	}

	/*
	* Get the new Screen pixmap ptr as SwitchMode might have called
	* ModifyPixmapHeader and xf86EnableDisableFBAccess will put it back...
	* Unfortunately.
	*/
	if (pScreenPixmap->devPrivate.ptr) {

		pScrn->pixmapPrivate = pScreenPixmap->devPrivate;
	}

	/* Make sure the layout is correct. */
	xf86ReconfigureLayout();

	pScrn->EnableDisableFBAccess(scrnIndex, TRUE);

	privSendConfigNotify(pScreen);
	privEditConnectionInfo(pScreen);
	ScreenRestructured(pScreen);

	return TRUE;
}


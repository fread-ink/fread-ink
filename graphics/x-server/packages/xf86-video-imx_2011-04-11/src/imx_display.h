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

#ifndef __IMX_DISPLAY_H__
#define __IMX_DISPLAY_H__

#include "xf86.h"

/* -------------------------------------------------------------------- */

typedef enum {
	ImxFbTypeUnknown	= -1,
	ImxFbTypeDISP3_BG	= 0,
	ImxFbTypeDISP3_FG	= 1,
	ImxFbTypeDISP3_BG_D1	= 2,
	ImxFbTypeEPDC		= 3
} ImxFbType;

/* -------------------------------------------------------------------- */

extern ImxFbType
imxDisplayGetFrameBufferType(struct fb_fix_screeninfo* pFixInfo);

extern Bool
imxDisplayPreInit(ScrnInfoPtr pScrn);

extern Bool
imxDisplayStartScreenInit(int scrnIndex, ScreenPtr pScreen);

extern Bool
imxDisplayFinishScreenInit(int scrnIndex, ScreenPtr pScreen);

extern ModeStatus
imxDisplayValidMode(int scrnIndex, DisplayModePtr mode,
			Bool verbose, int flags);

extern Bool
imxDisplaySwitchMode(int scrnIndex, DisplayModePtr mode, int flags);

extern void
imxDisplayAdjustFrame(int scrnIndex, int x, int y, int flags);

extern Bool
imxDisplayEnterVT(int scrnIndex, int flags);

extern void
imxDisplayLeaveVT(int scrnIndex, int flags);

extern Bool
imxDisplayChangeFrameBufferRotateEPDC(int scrnIndex, int fbRotate);

#endif

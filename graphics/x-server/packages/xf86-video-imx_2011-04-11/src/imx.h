/*
 * Copyright (C) 2009-2011 Freescale Semiconductor, Inc.  All Rights Reserved.
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

#ifndef __IMX_H__
#define __IMX_H__

#ifndef IMX_XVIDEO_ENABLE
#define	IMX_XVIDEO_ENABLE	0
#endif

#include "xf86.h"


#if IMX_XVIDEO_ENABLE
#include "mxc_ipu_hl_lib.h"
#endif

/* -------------------------------------------------------------------- */

typedef struct {

	char				fbId[80];
	char				fbDeviceName[32];
	unsigned char*			fbstart;
	unsigned char*			fbmem;
	int				fboff;
	int				lineLength;
	CloseScreenProcPtr		saveCloseScreen;
	EntityInfoPtr			pEntity;
	OptionInfoPtr			pOptions;
	Bool				useAccel;
	void*				exaDriverPrivate;
	void*				displayPrivate;

#if IMX_XVIDEO_ENABLE
	/* for xvideo */
	DevUnion	                XVPortPrivate[1];
	Bool                    	isInit;
	short               		SrcX;
	short               		SrcY;
	short               		DstX;
	short               		DstY;
	short               		SrcW;
	short               		SrcH;
	short               		DstW;
	short               		DstH;
	int                 		Width;
	int                 		Height;
	char				fb_background[12];
	/* for using IPU's library */
	ipu_lib_input_param_t  		input_param;
	ipu_lib_output_param_t 		output_para;
	int                    		next_update_idx ;
	ipu_lib_handle_t       		ipu_handle;
	CARD32                 		colour_key;
#endif

} ImxRec, *ImxPtr;

#define IMXPTR(pScrnInfo) ((ImxPtr)((pScrnInfo)->driverPrivate))

#endif

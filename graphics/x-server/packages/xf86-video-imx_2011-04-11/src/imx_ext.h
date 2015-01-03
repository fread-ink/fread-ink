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

#ifndef __IMX_EXT_H__
#define __IMX_EXT_H__

#define	Pixmap	CARD32

#if !defined(IMX_EXT_NAME)
#define	IMX_EXT_NAME	"imx-ext"
#endif

#define	IMX_EXT_NumErrors	0
#define	IMX_EXT_NumEvents	0

#define	X_IMX_EXT_GetPixmapPhysAddr	1

/************************************************************************/

typedef struct {
    CARD8	reqType;	/* always XTestReqCode */
    CARD8	xtReqType;	/* always X_IMX_EXT_GetPixmapPhysAddr */
    CARD16	length B16;
    Pixmap	pixmap B32;
} xIMX_EXT_GetPixmapPhysAddrReq;
#define sz_xIMX_EXT_GetPixmapPhysAddrReq 8

typedef enum
{
	IMX_EXT_PixmapUndefined,	/* pixmap is not defined */
	IMX_EXT_PixmapFramebuffer,	/* pixmap is in framebuffer */
	IMX_EXT_PixmapOther		/* pixmap is not in framebuffer */
} IMX_EXT_PixmapState;

typedef struct {
    CARD8	type;			/* must be X_Reply */
    CARD8	pixmapState;		/* has value of IMX_EXT_PixmapState */
    CARD16	sequenceNumber B16;	/* of last request received by server */
    CARD32	length B32;		/* 4 byte quantities beyond size of GenericReply */
    CARD32	pixmapPhysAddr B32;	/* pixmap phys addr; otherwise NULL */
    CARD32	pixmapPitch B32;	/* bytes between lines in pixmap */
    CARD32	pad0 B32;		/* bytes 17-20 */
    CARD32	pad1 B32;		/* bytes 21-24 */
    CARD32	pad2 B32;		/* bytes 25-28 */
    CARD32	pad3 B32;		/* bytes 29-32 */
} xIMX_EXT_GetPixmapPhysAddrReply;
#define	sz_xIMX_EXT_GetPixmapPhysAddrReply 32

/************************************************************************/

#undef Pixmap

#endif

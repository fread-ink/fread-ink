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

#if IMX_XVIDEO_ENABLE

#include "xf86.h"
#include "xf86_OSproc.h"
#include "X11/extensions/Xv.h"

#include "fourcc.h"
#include "xf86xvpriv.h"
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "mxc_ipu_hl_lib.h"
#include <xorg/fbdevhw.h>
#include "fb.h"

#include "imx.h"

static Bool debug = 0;

#define TRACE_ENTER(str) \
    do { if (debug) ErrorF("imx: " str " %d\n",pScrn->scrnIndex); } while (0)
#define TRACE_EXIT(str) \
    do { if (debug) ErrorF("imx: " str " done\n"); } while (0)
#define TRACE(str) \
    do { if (debug) ErrorF("inx trace: " str "\n"); } while (0)

#define RGB565TOCOLORKEY(rgb)                              \
      ( ((rgb & 0xf800)<<8)  |  ((rgb & 0xe000)<<3)  |     \
        ((rgb & 0x07e0)<<5)  |  ((rgb & 0x0600)>>1)  |     \
        ((rgb & 0x001f)<<3)  |  ((rgb & 0x001c)>>2)  )   
/**************************   Global Setting   ******************************/
/* this mutex only can protect within same process context,
 * for different process, pls add other mutex*/
pthread_mutex_t MXXvMutex = PTHREAD_MUTEX_INITIALIZER;

#define NumberOf(_what) (SizeOf(_what) / SizeOf(_what[0]))
#define SizeOf(_object) ((int)sizeof(_object))
#define MAKE_ATOM(string) MakeAtom(string, strlen(string), TRUE)
static XF86VideoEncodingRec MXVideoEncoding[] =
{
    /* This will be updated after good understanding for Xvideo and IPU Spec */
    { 0, "XV_IMAGE", 2048, 2048, {1, 1} }
};
#define nMXVideoEncoding NumberOf(MXVideoEncoding)

static XF86VideoFormatRec MXVideoFormat[] =
{
	{ 8, TrueColor},
	{ 8, DirectColor},
	{ 8, PseudoColor},
	{ 8, GrayScale},
	{ 8, StaticGray},
	{ 8, StaticColor},
	{15, TrueColor},
	{16, TrueColor},
	{24, TrueColor},
	{15, DirectColor},
	{16, DirectColor},
	{24, DirectColor}
};
#define nMXVideoFormat NumberOf(MXVideoFormat)
static XF86AttributeRec MXAttribute[] =
{
    {
        XvSettable | XvGettable,
        0, (1 << 24) - 1,
        "XV_COLOURKEY"
    },
    {
        XvSettable | XvGettable,
        0, (1 << 24) - 1,
        "XV_COLORKEY"
    },
    {
        XvSettable,
        0, 0,
        "XV_SET_DEFAULTS"
    }
};
#define nMXAttribute NumberOf(MXAttribute)
/* Add NV12 format support */
#define FOURCC_NV12 0x3231564E

#define NoOrder LSBFirst

static XF86ImageRec MXImage[] =
{
	XVIMAGE_YUY2,
	XVIMAGE_UYVY,
	XVIMAGE_YV12,
	XVIMAGE_I420,
	/* NV12 */
	{ FOURCC_NV12, XvYUV, LSBFirst, {'N','V','1','2', \
	0x00,0x00,0x00,0x10,0x80,0x00,0x00,0xAA,0x00,0x38,0x9B,0x71},
	12, XvPlanar, 3, 0, 0, 0, 0,
	8, 8, 8, 1, 2, 2, 1, 2, 2, "NV12", XvTopToBottom },
	/* RGBA 8:8:8:8 */
	{ IPU_PIX_FMT_RGBA32, XvRGB, LSBFirst, { 0 },
	32, XvPacked, 1, 24, 0x0000FF, 0x00FF00, 0xFF0000,
	0, 0, 0,  0, 0, 0,  0, 0, 0, "RGBA", XvTopToBottom },
	/* RGB4 8:8:8:8 */
	{ IPU_PIX_FMT_RGB32, XvRGB, LSBFirst, { 0 },
	32, XvPacked, 1, 24, 0x0000FF, 0x00FF00, 0xFF0000,
	0, 0, 0,  0, 0, 0,  0, 0, 0, "RGB4", XvTopToBottom },
	/* RGB 5:6:5 */
	{ IPU_PIX_FMT_RGB565, XvRGB, LSBFirst, { 0 },
	16, XvPacked, 1, 16, 0x001F, 0x07E0, 0xF800,
	0, 0, 0,  0, 0, 0,  0, 0, 0, "RGB", XvTopToBottom },
	/* RGBA 5:5:5:1 */
	{ IPU_PIX_FMT_RGB555, XvRGB, LSBFirst, { 0 },
	16, XvPacked, 1, 15, 0x001F, 0x03E0, 0x7C00,
	0, 0, 0,  0, 0, 0,  0, 0, 0, "RGBA", XvTopToBottom },
	/* RGB 3:3:2 */
	{ IPU_PIX_FMT_RGB332, XvRGB, NoOrder, { 0 },
	8, XvPacked, 1, 8, 0x07, 0x38, 0xC0,
	0, 0, 0,  0, 0, 0,  0, 0, 0, "RGB", XvTopToBottom },
	/* BGRA 8:8:8:8 */
	{ IPU_PIX_FMT_BGRA32, XvRGB, LSBFirst, { 0 },
	32, XvPacked, 1, 24, 0xFF0000, 0x00FF00, 0x0000FF,
	0, 0, 0,  0, 0, 0,  0, 0, 0, "BGRA", XvTopToBottom },
};
#define nMXImage NumberOf(MXImage)

/* A local XVideo adaptor attribute record */
typedef struct _MXAttribute
{
	Atom  AttributeID;
	INT32 MaxValue;             /* ... for the hardware */
	void  (*SetAttribute) (IMXPtr, INT32);
	INT32 (*GetAttribute) (IMXPtr);
} MXAttributeRec, *MXAttributePtr;

/* Functions to get/set XVideo adaptor attributes */
static void
MXSetColourKeyAttribute
(
	IMXPtr pFB,
	INT32  Value
)
{
	int bg_fd;
	pFB->colour_key = (CARD32)(Value & ((1 << 16) - 1));
	if(pFB->isInit)
	{
		if ((bg_fd = open(pFB->fb_background, O_RDWR, 0)) >=0)
		{
    			struct mxcfb_color_key color_key;
		    	int color;
			color_key.enable = 1;
			color_key.color_key = RGB565TOCOLORKEY( pFB->colour_key);
			ioctl(bg_fd, MXCFB_SET_CLR_KEY, &color_key);
			close(bg_fd);
		}
	}
	TRACE("MXSetColourKeyAttribute!\n");
}

static INT32
MXGetColourKeyAttribute
(
	IMXPtr pFB
)
{
	TRACE("MXGetColourKeyAttribute\n");
	return pFB->colour_key;
}
/*
 * MXSetDefaultAttributes --
 *
 * This function calls other functions to set default values for the various
 * attributes of an XVideo port.
 */
static void
MXSetDefaultAttributes
(
	IMXPtr pFB,
	INT32  Value
)
{
	//MXSetColourKeyAttribute(pFB, Value);
}
static MXAttributeRec MXAttributeInfo[nMXAttribute] =
{
    {   /* COLOURKEY */
        0, (1 << 24) - 1,
        MXSetColourKeyAttribute,
        MXGetColourKeyAttribute
    },
    {   /* COLORKEY */
        0, (1 << 24) - 1,
        MXSetColourKeyAttribute,
        MXGetColourKeyAttribute
    },
    {   /* SET_DEFAULTS */
        0, 0,
        MXSetDefaultAttributes,
        NULL
    }
};
/*
 * MXFindAttribute --
 *
 * This function is called to locate an Xv attribute's table entry.
 */
static int
MXFindPortAttribute
(
	IMXPtr pFB,
	Atom   AttributeID
)
{
	int iAttribute;
	TRACE("Enter MXFindPortAttribute--> nMXAttribute \n");
	iAttribute = 0;

	for (;  iAttribute < nMXAttribute;  iAttribute++)
            if (AttributeID == MXAttributeInfo[iAttribute].AttributeID)
        	return iAttribute;

	return -1;
}

/*
 * MXSetPortAttribute --
 *
 * This function sets the value of a particular port's attribute.
 */
static int
MXSetPortAttribute
(
	ScrnInfoPtr pScreenInfo,
	Atom        AttributeID,
	INT32       Value,
	pointer     pFB
)
{
	INT32 Range;
	int   iAttribute;

	TRACE("Enter MXSetPortAttribute--> AttributeID\n");
	if (((iAttribute = MXFindPortAttribute(pFB, AttributeID)) < 0) ||
            !MXAttributeInfo[iAttribute].SetAttribute)
		return BadMatch;

	Range = MXAttribute[iAttribute].max_value -
            MXAttribute[iAttribute].min_value;

	if (Range >= 0)
	{
        	/* Limit and scale the value */
        	Value -= MXAttribute[iAttribute].min_value;

        	if (Value < 0)
            		Value = 0;
        	else if (Value > Range)
            		Value = Range;

        	if (Range != MXAttributeInfo[iAttribute].MaxValue)
	        {
        	    if (MXAttributeInfo[iAttribute].MaxValue > 0)
                	Value *= MXAttributeInfo[iAttribute].MaxValue;
	            if (Range > 0)
        	        Value /= Range;
        	}
	    }

	(*MXAttributeInfo[iAttribute].SetAttribute)(pFB, Value);

	return Success;
}
/*
 * MXSetPortAttribute --
 *
 * This function retrieves the value of a particular port's attribute.
 */
static int
MXGetPortAttribute
(
	ScrnInfoPtr pScreenInfo,
	Atom        AttributeID,
	INT32       *Value,
	pointer     pFB
)
{
	INT32 Range;
	int   iAttribute;

	TRACE("Enter MXGetPortAttribute--> AttributeID\n");
	if (!Value ||
            ((iAttribute = MXFindPortAttribute(pFB, AttributeID)) < 0) ||
            !MXAttributeInfo[iAttribute].GetAttribute)
    	    return BadMatch;

	*Value = (*MXAttributeInfo[iAttribute].GetAttribute)(pFB);

	Range = MXAttribute[iAttribute].max_value -
        	MXAttribute[iAttribute].min_value;

	if (Range >= 0)
	{
        	if (Range != MXAttributeInfo[iAttribute].MaxValue)
        	{
            		if (Range > 0)
                		*Value *= Range;
            		if (MXAttributeInfo[iAttribute].MaxValue > 0)
                		*Value /= MXAttributeInfo[iAttribute].MaxValue;
        	}

        	*Value += MXAttribute[iAttribute].min_value;
	}
	return Success;
}

static void
MXStopVideo
(
	ScrnInfoPtr pScreenInfo,
	pointer     Data,
	Bool        Cleanup
)
{
	IMXPtr pFB = Data;
	pthread_mutex_lock(&MXXvMutex);
	TRACE("Enter MXStopVideo\n");
	if(pFB->isInit)
	{
		struct mxcfb_pos pos;
		int bg_fd;
		if ((bg_fd = open(pFB->fb_background, O_RDWR, 0)) >=0)
		{
			struct mxcfb_gbl_alpha ga;
			ga.enable = 1;
			ga.alpha = 255;
			ioctl(bg_fd,MXCFB_SET_GBL_ALPHA,&ga);
			close(bg_fd);
		}
		pFB->isInit = 0;
		mxc_ipu_lib_task_uninit(&pFB->ipu_handle);
		TRACE("Close IPU Finished!\n");
	}
	pthread_mutex_unlock(&MXXvMutex);
}
static void
MXQueryBestSize
(
	ScrnInfoPtr  pScreenInfo,
	Bool         Motion,
	short        VideoWidth,
	short        VideoHeight,
	short        DrawableWidth,
	short        DrawableHeight,
	unsigned int *Width,
	unsigned int *Height,
	pointer      pFB
)
{
	TRACE("Enter MXQueryBestSize\n");
	*Width  = DrawableWidth;
	*Height = DrawableHeight;
}
_X_EXPORT void
xf86XVFillKeyHelper1 (ScreenPtr pScreen, CARD32 key, RegionPtr clipboxes)
{
	DrawablePtr root = &WindowTable[pScreen->myNum]->drawable;
	XID pval[2];
	BoxPtr pbox = REGION_RECTS(clipboxes);
	int i, nbox = REGION_NUM_RECTS(clipboxes);
	xRectangle *rects;
	GCPtr gc;

	if(!xf86Screens[pScreen->myNum]->vtSema) return;

	gc = GetScratchGC(root->depth, pScreen);
	pval[0] = key;
	pval[1] = IncludeInferiors;
	(void) ChangeGC(gc, GCForeground|GCSubwindowMode, pval);
	ValidateGC(root, gc);

	rects = xalloc (nbox * sizeof(xRectangle));

	for(i = 0; i < nbox; i++, pbox++) 
	{
		rects[i].x = pbox->x1;
		rects[i].y = pbox->y1;
		rects[i].width = pbox->x2 - pbox->x1;
		rects[i].height = pbox->y2 - pbox->y1;
	}
	(*gc->ops->PolyFillRect)(root, gc, nbox, rects);
	xfree (rects);
	FreeScratchGC (gc);
}

static int MXForeground(void)
{
	int	i;
        int	fd_fb;
        struct	fb_fix_screeninfo fb_fix;
	char	dev_id[12];

	for(i=2;i>0;i--)
	{
		sprintf(dev_id,"/dev/fb%d",i);
	        fd_fb = open(dev_id, O_RDWR, 0);
        	ioctl(fd_fb, FBIOGET_FSCREENINFO, &fb_fix);
	        if (strcmp(fb_fix.id, "DISP3 FG") == 0) {
        	        close(fd_fb);
                	return i;
	        }
	}

        return 0;
}
static int MXBackground(char* bg_name, char* dev_name)
{
	int 	i;
        int 	fd_fb;
        struct	fb_fix_screeninfo fb_fix;
	char	dev_id[12];

	for(i=0;i<3;i++)
	{
		sprintf(dev_id,"/dev/fb%d",i);
	        fd_fb = open(dev_id, O_RDWR, 0);
        	ioctl(fd_fb, FBIOGET_FSCREENINFO, &fb_fix);
	        if (strcmp(fb_fix.id, bg_name) == 0) {
        	        close(fd_fb);
			strcpy(dev_name, dev_id);
                	return 1;
	        }
	}

        return -1;
}
/* return values
**   0 : it's ok
** < 0 : failed for setting up new ipu task
*/
static int MXSetupNewIPUTask(IMXPtr pFB,int ImageID)
{
	int ret = 0;
	int screen_size;
	int mode = OP_STREAM_MODE;
	int bg_fd;
	int blank;
	struct fb_var_screeninfo fb_var;
	struct fb_fix_screeninfo fb_fix;
	struct mxcfb_pos         pos;

	pthread_mutex_lock(&MXXvMutex);

	if ((bg_fd = open(pFB->fb_background, O_RDWR, 0)) >=0)
	{
    		struct mxcfb_gbl_alpha ga;
    		struct mxcfb_color_key color_key;
		int color;

		color_key.enable = 1;
		color_key.color_key = RGB565TOCOLORKEY( pFB->colour_key);
		ioctl(bg_fd, MXCFB_SET_CLR_KEY, &color_key);

    		ga.enable = 1;
    		ga.alpha = 255;
    		ioctl(bg_fd,MXCFB_SET_GBL_ALPHA,&ga);
    		close(bg_fd);
	}
	/* Initialize for one new IPU task */
	memset(&pFB->input_param, 0, sizeof(ipu_lib_input_param_t));
	memset(&pFB->output_para, 0, sizeof(ipu_lib_output_param_t));
	switch(ImageID)
	{
    		case FOURCC_YUY2:
	    	    	TRACE("convert FOURCC_YUY2 ==> IPU_PIX_FMT_YUYV\n");
    		    	pFB->input_param.fmt = IPU_PIX_FMT_YUYV;
	    		break;
    		default:
    	    		pFB->input_param.fmt = ImageID;
	}
	pFB->input_param.width  = pFB->Width;
	pFB->input_param.height = pFB->Height;
	pFB->input_param.input_crop_win.pos.x = pFB->SrcX;
	pFB->input_param.input_crop_win.pos.y = pFB->SrcY;
	pFB->input_param.input_crop_win.win_w = pFB->SrcW;
	pFB->input_param.input_crop_win.win_h = pFB->SrcH;

	TRACE("Output Parameters:\n");
	pFB->output_para.width  = pFB->DstW;
	pFB->output_para.height = pFB->DstH;
	pFB->output_para.rot    = pFB->rotate;
	if (fb_var.bits_per_pixel == 24)
    		pFB->output_para.fmt = v4l2_fourcc('B', 'G', 'R', '3');
	else
    		pFB->output_para.fmt = v4l2_fourcc('R', 'G', 'B', 'P');
	pFB->output_para.show_to_fb  = 1;
	pFB->output_para.fb_disp.pos.x = pFB->DstX;
	pFB->output_para.fb_disp.pos.y = pFB->DstY;
	pFB->output_para.fb_disp.fb_num= MXForeground();
	ret = mxc_ipu_lib_task_init(&pFB->input_param,NULL, &pFB->output_para,
    		NULL, mode, &pFB->ipu_handle);
	if (ret < 0) {
    		TRACE("mxc_ipu_lib_task_init failed!\n");
	    	goto ipu_setup_done;
	}
	TRACE("mxc_ipu_lib_task_init successed !\n");
	pFB->next_update_idx = 0;
	pFB->isInit = 1;
ipu_setup_done:
	pthread_mutex_unlock(&MXXvMutex);
	return ret;
}

static int
MXPutImage
(
	ScrnInfoPtr   pScreenInfo,
	short         SrcX,
	short         SrcY,
	short         DstX,
	short         DstY,
	short         SrcW,
	short         SrcH,
	short         DstW,
	short         DstH,
	int           ImageID,
	unsigned char *Buffer,
	short         Width,
	short         Height,
	Bool          Synchronise,
	RegionPtr     pClip,
	pointer       Data,
	DrawablePtr   pDraw
)
{
	IMXPtr  pFB = (IMXPtr)Data;
	ScreenPtr pScreen;
	int       ret = 0;

	TRACE("Enter: MXPutImage\n");
	pScreen = pScreenInfo->pScreen;
	DstW = DstW - DstW%8;
begin:
	pthread_mutex_lock(&MXXvMutex);
	if(!pFB->isInit)
	{
    		pFB->SrcX = SrcX;
       		pFB->SrcY = SrcY;
       		pFB->DstX = DstX;
	       	pFB->DstY = DstY;
       		pFB->SrcW = SrcW;
	       	pFB->SrcH = SrcH;
       		pFB->DstW = DstW;
	       	pFB->DstH = DstH;
       		pFB->Width  = Width;
	       	pFB->Height = Height;
        	TRACE("From MXPutImage to MXSetupNewIPUTask because isInit=0\n");
	        pthread_mutex_unlock(&MXXvMutex);
        	if((ret=MXSetupNewIPUTask(pFB,ImageID))<0)
	            goto done;
        	TRACE("Return from MXSetupNewIPUTask !\n");
	        pthread_mutex_lock(&MXXvMutex);
	}
	/* Check Size Changed */
	if(pFB->DstW != DstW || pFB->DstH != DstH )
	{
    		pFB->SrcX = SrcX;
	    	pFB->SrcY = SrcY;
    		pFB->DstX = DstX;
	    	pFB->DstY = DstY;
    		pFB->SrcW = SrcW;
	    	pFB->SrcH = SrcH;
    		pFB->DstW = DstW;
	    	pFB->DstH = DstH;
        	pFB->Width = Width;
	    	pFB->Height = Height;
        	TRACE("From MXPutImage to MXStopVideo! \n");
	        pthread_mutex_unlock(&MXXvMutex);
    		MXStopVideo(pScreenInfo,pFB,FALSE);
	        TRACE("Return from MXStopVideo! \n");
    		goto begin;
	}
	/* Check Position Change */
	if(DstX != pFB->DstX || DstY != pFB->DstY )
	{
        	pthread_mutex_unlock(&MXXvMutex);
	    	MXStopVideo(pScreenInfo,pFB,FALSE);
        	TRACE("Return from MXStopVideo! \n");
	    	goto begin;
	}

	xf86XVFillKeyHelper1(pScreen, pFB->colour_key, pClip);

	if(pFB->isInit)
	{
		memcpy(pFB->ipu_handle.inbuf_start[pFB->next_update_idx],Buffer,pFB->ipu_handle.ifr_size);
	        if((pFB->next_update_idx = mxc_ipu_lib_task_buf_update(&pFB->ipu_handle, 0, 0, 0,NULL, NULL)) < 0)
        	{
	            mxc_ipu_lib_task_uninit(&pFB->ipu_handle);
		    pFB->isInit = 0;
	        }
	}
	pthread_mutex_unlock(&MXXvMutex);
done:
	return Success;
}

static int
MXQueryImageAttributes
(
	ScrnInfoPtr    pScreenInfo,
	int            ImageID,
	unsigned short *Width,
	unsigned short *Height,
	int            *pPitch,
	int            *pOffset
)
{
	int Size, tmp;

	TRACE("Enter MXQueryImageAttributes\n");
	if (!Width || !Height)
        	return 0;

	if (*Width > 1024)
        	*Width = 1024;
	else
        	*Width = (*Width + 1) & ~1;

	if (*Height > 1024)
        	*Height = 1024;

	if (pOffset)
        	pOffset[0] = 0;

	switch (ImageID)
	{
        	case FOURCC_YV12:
	        case FOURCC_I420:
        	case FOURCC_NV12:
        		*Height = (*Height + 1) & ~1;
         		Size = (*Width + 3) & ~3;
        		if (pPitch)
		                pPitch[0] = Size;
		        Size *= *Height;
		        if (pOffset)
                		pOffset[1] = Size;
		        tmp = ((*Width >> 1) + 3) & ~3;
		        if (pPitch)
                		pPitch[1] = pPitch[2] = tmp;
		        tmp *= (*Height >> 1);
		        Size += tmp;
		        if (pOffset)
		                pOffset[2] = Size;
		        Size += tmp;
		        break;
	        case FOURCC_UYVY:
        	case FOURCC_YUY2:
	        case IPU_PIX_FMT_RGB565:
        	case IPU_PIX_FMT_RGB555:
		        Size = *Width << 1;
			if (pPitch)
		                pPitch[0] = Size;
		        Size *= *Height;
		        break;
	        case IPU_PIX_FMT_RGBA32:
        	case IPU_PIX_FMT_RGB32:
	        case IPU_PIX_FMT_BGRA32:
        		Size = *Width << 2;
		        if (pPitch)
                		pPitch[0] = Size;
		        Size *= *Height;
		        break;
	        case IPU_PIX_FMT_RGB332:
        		Size = *Width ;
		        if (pPitch)
                		pPitch[0] = Size;
		        Size *= *Height;
		        break;
        	default:
		        Size = 0;
		        break;
	}

	return Size;
}
int
FBXVInitialiseAdaptor
(
	ScreenPtr           pScreen,
	ScrnInfoPtr         pScreenInfo,
	IMXPtr              pIMX,
	XF86VideoAdaptorPtr **pppAdaptor
)
{
	XF86VideoAdaptorPtr pAdaptor;
	int                 Index;
	int		    ret_num;

	TRACE("Enter FBXVInitialiseAdaptor\n");
	if (!(pAdaptor = xf86XVAllocateVideoAdaptorRec(pScreenInfo)))
        	return 0;
    	*pppAdaptor = xnfalloc(sizeof(pAdaptor));
	**pppAdaptor = pAdaptor;
	pAdaptor->nPorts = 1;
	pAdaptor->pPortPrivates = pIMX->XVPortPrivate;
	pIMX->XVPortPrivate[0].ptr = pIMX;

	/* Get fb num for background*/
	ret_num = MXBackground(fbdevHWGetName(pScreenInfo), pIMX->fb_background);
	if(ret_num == -1)	return 0;

	pAdaptor->type = XvInputMask | XvImageMask | XvWindowMask;
	pAdaptor->flags = VIDEO_OVERLAID_IMAGES;// |VIDEO_NO_CLIPPING;//| VIDEO_CLIP_TO_VIEWPORT;
	pAdaptor->name = "Freescale Overlay Scaler";
	pAdaptor->nEncodings = nMXVideoEncoding;
	pAdaptor->pEncodings = MXVideoEncoding;

	pAdaptor->nFormats = nMXVideoFormat;
	pAdaptor->pFormats = MXVideoFormat;

	pAdaptor->nAttributes = nMXAttribute;
	pAdaptor->pAttributes = MXAttribute;

	pAdaptor->nImages = nMXImage;
	pAdaptor->pImages = MXImage;

	pAdaptor->StopVideo            = MXStopVideo;
	pAdaptor->SetPortAttribute     = MXSetPortAttribute;
	pAdaptor->GetPortAttribute     = MXGetPortAttribute;
	pAdaptor->QueryBestSize        = MXQueryBestSize;
	pAdaptor->PutImage             = MXPutImage;
	pAdaptor->QueryImageAttributes = MXQueryImageAttributes;

	for (Index = 0;  Index < nMXAttribute;  Index++)
        	MXAttributeInfo[Index].AttributeID =
            		MAKE_ATOM(MXAttribute[Index].name);
	return 1;
}
int
MXXVInitializeAdaptor
(
	ScrnInfoPtr         pScreenInfo,
	XF86VideoAdaptorPtr **pppAdaptor
)
{
	ScreenPtr           pScreen    = screenInfo.screens[pScreenInfo->scrnIndex];
	XF86VideoAdaptorPtr *ppAdaptor = NULL;
	IMXPtr fPtr = IMXPTR(pScreenInfo);
	int                 nAdaptor;
	TRACE("Enter MXXVInitializeAdaptor\n");

	nAdaptor = FBXVInitialiseAdaptor(pScreen, pScreenInfo, fPtr,
                &ppAdaptor);

	if (pppAdaptor)
        	*pppAdaptor = ppAdaptor;

	return nAdaptor;
}

#endif


/****************************************************************************
** Copyright (C) 2004-2009 Mazatech S.r.l. All rights reserved.
**
** This file is part of AmanithVG software, an OpenVG implementation.
** This file is strictly confidential under the signed Mazatech Software
** Non-disclosure agreement and it's provided according to the signed
** Mazatech Software licensing agreement.
**
** Khronos and OpenVG are trademarks of The Khronos Group Inc.
** OpenGL is a registered trademark and OpenGL ES is a trademark of
** Silicon Graphics, Inc.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** For any information, please contact info@mazatech.com
**
****************************************************************************/

/*!
	\file vgdrawingsurface.c
	\brief OpenVG drawing surface, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined(AM_SRE)
	#include "sredrawingsurface.h"
#elif defined(AM_GLE)
	#include "gldrawingsurface.h"
	#include "gledrawingsurface.h"
#elif defined(AM_GLS)
	#include "gldrawingsurface.h"
	#include "glsdrawingsurface.h"
#else
	#error Unreachable point.
#endif
#include "vgdrawingsurface.h"
#include "vgcontext.h"

#if defined (RIM_VG_SRC) && defined (RIM_OPENVG_USES_QC_7KMDP_BLT)

typedef enum image_rotation_ {
  IMG_NOROT,         // Do not rotate
  IMG_ROT90,         // Rotate clockwise 90 degrees
  IMG_ROT180,        // Rotate clockwise 180 degrees
  IMG_ROT270,        // Rotate clockwise 270 degrees
  IMG_NOROT_XREF,    // Rotate 0 degrees, reflect about x axis
  IMG_NOROT_YREF,    // Rotate 0 degrees, reflect about y axis
  IMG_ROT90_XREF,    // Rotate 90 degrees, reflect about x axis
  IMG_ROT90_YREF,    // Rotate 90 degrees, reflect about y axis
  IMG_ROT180_XREF,   // Rotate 180 degrees, reflect about x axis
  IMG_ROT180_YREF,   // Rotate 180 degrees, reflect about y axis
  IMG_ROT270_XREF,   // Rotate 270 degrees, reflect about x axis
  IMG_ROT270_YREF,   // Rotate 270 degrees, reflect about y axis
  IMG_ROT_MAX        // Maximum number of rotating modes
} img_rot_enum;



extern void RasterCopyARGB8888_7kmdp2(
   DWORD         *src,
   DWORD         srcStride,
   DWORD         srcx,
   DWORD         *dst,
   DWORD         dstStride,
   DWORD         dstx,
   DWORD         width,
   DWORD         height,
   BOOL          alpha,
   DWORD         alpha_val,
   img_rot_enum  rotation);


/*!
        \brief This function copies or blends pixel data from the source image onto the drawing surface. Pixels whose source lies outside
	of the bounds of source image or whose destination lies outside the bounds of the drawing
	surface are ignored. Pixel format conversion is not applied since this is a fast case. Scissoring takes place normally if only
	one scissor rect is present.
	Most transformations (except those mapped to pointer arithmetics or image flips) are not applied.
	Masking is not applied as well.
	\param context context containing the drawing surface.
	\param dx x-coordinate of the rectangular region to write on the drawing surface.
	\param dy y-coordinate of the rectangular region to write on the drawing surface.
	\param src source image handle.
	\param sx x-coordinate of the rectangular region to read.
	\param sy y-coordinate of the rectangular region to read.
	\param width width of the rectangular region to set, in pixels.
	\param height height of the rectangular region to set, in pixels.
	\pre width, height > 0.
*/
AMbool amDrawingSurfacePixelsBlend(AMContext *context,
								 AMDrawingSurface *surface,
								 AMint32 dx,
								 AMint32 dy,
								 VGImage src,
								 AMint32 sx,
								 AMint32 sy,
								 AMint32 width,
								 AMint32 height,
								 AMbool blend,
								 AMbool flip) {
    
     
	AMImage *srcImg;
    DWORD *srcPtr, *dstPtr, srcx, dstx, srcStride, dstStride;
#if defined ( RIM_VG_SRC )     // Suppress compiler warning.
    AMint32 xmin, ymin, widthS, heightS;
#else
    AMint32 xmin, ymin, widthS, heightS, tmpW, tmpH;
#endif    
    AMAABox2i intersectionBox, imageBox, scissorRect;
    AMint32 sxNew, syNew, widthNew, heightNew, dxNew, dyNew;
    AMInt32DynArray *scissorRects;

	AM_ASSERT(context);
	AM_ASSERT(surface);
	AM_ASSERT(src > 0 && src < context->handles->createdHandlesList.size);
	AM_ASSERT(width > 0 && height > 0);

	srcImg = (AMImage *)context->handles->createdHandlesList.data[src];
	AM_ASSERT(srcImg);

	// clip specified bound to the current drawing surface
    
    if (sx < 0) {
		width += sx;
		if (width <= 0)
			return AM_TRUE;
		dx -= sx;
		sx = 0;
	}
	if (sy < 0) {
		height += sy;
		if (height <= 0)
			return AM_TRUE;
		dy -= sy;
		sy = 0;
	}
	if (sx + width > srcImg->width) {
		width = srcImg->width - sx;
		if (width <= 0)
			return AM_TRUE;
	}
	if (sy + height > srcImg->height) {
		height = srcImg->height - sy;
		if (height <= 0)
			return AM_TRUE;
	}

	if (dx < 0) {
		width += dx;
		if (width <= 0)
			return AM_TRUE;
		sx -= dx;
		AM_ASSERT(sx < srcImg->width);
		dx = 0;
	}
	if (dy < 0) {
		height += dy;
		if (height <= 0)
			return AM_TRUE;
		sy -= dy;
		AM_ASSERT(sy < srcImg->height);
		dy = 0;
	}
	if (dx + width > amSrfWidthGet(surface)) {
		width = amSrfWidthGet(surface) - dx;
		if (width <= 0)
			return AM_TRUE;
	}
	if (dy + height > amSrfHeightGet(surface)) {
		height = amSrfHeightGet(surface) - dy;
		if (height <= 0)
			return AM_TRUE;
	}
 
    if (context->scissoring == VG_FALSE)
    {           
        dstPtr = (DWORD*) surface->pixels;
        dstStride = (DWORD) surface->dataStride;
        dstStride = dstStride >> 2;
        srcPtr = (DWORD*)srcImg->pixels;
        srcStride = (DWORD) srcImg->dataStride;

        //Since image specifies its stride in bytes, convert to pixels
        srcStride = srcStride >> 2;
        
        //Adjust src and dst pointers
        srcx = sx;
        srcPtr += sy * srcStride;
        dstx = (DWORD)(dstStride * (surface->height - dy - height) + dx);

        if (surface->wholeCleared){
        	AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, dx)
		    AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, dy)
	    	AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, width)
    		AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, height)
        }
        
        if (flip)
        {
            RasterCopyARGB8888_7kmdp2( srcPtr, srcStride, srcx, dstPtr, dstStride, dstx, width, height, (BOOL) blend, 0 , IMG_NOROT);
        }else
        {
            RasterCopyARGB8888_7kmdp2( srcPtr, srcStride, srcx, dstPtr, dstStride, dstx, width, height, (BOOL) blend, 0 , IMG_NOROT_XREF);
        }    
    }
    else
    {
        //If scissoring is enabled, it means that we have to clip the image to the scissor rect. 
        //We know that we are dealing only with only a single scissor rect since this is a fast case, if more than one scissor rects 
        //are present - generic case must have been used.
        scissorRects = &context->scissorRects;    
        
        xmin = scissorRects->data[0];
		ymin =  scissorRects->data[1];
		// width and height
		widthS =  scissorRects->data[2];
		heightS =  scissorRects->data[3];

        //clip dst
		AM_VECT2_SET(&imageBox.minPoint, dx, dy)
		AM_VECT2_SET(&imageBox.maxPoint, dx + width, dy + height)

        AM_VECT2_SET(&scissorRect.minPoint, xmin, ymin)
		AM_VECT2_SET(&scissorRect.maxPoint, xmin + widthS, ymin + heightS)
     
        dxNew = dx;
        dyNew = dy;
               
        if (amAABox2iIntersect(&intersectionBox, &imageBox, &scissorRect)) {
    		// update dirty regions                      
            if (surface->wholeCleared){
                AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, intersectionBox.minPoint.x)
                AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, intersectionBox.minPoint.y)
		    	AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, intersectionBox.maxPoint.x - intersectionBox.minPoint.x)
			    AM_DYNARRAY_PUSH_BACK(surface->dirtyRegions, AMint32, intersectionBox.maxPoint.y - intersectionBox.minPoint.y)
            }
            dxNew = intersectionBox.minPoint.x;
            dyNew = intersectionBox.minPoint.y;  
            
#if !defined ( RIM_VG_SRC )     // Suppress compiler warning.
            tmpW = intersectionBox.maxPoint.x - intersectionBox.minPoint.x;
            tmpH = intersectionBox.maxPoint.y - intersectionBox.minPoint.y;
#endif
            //clip src            
            AM_VECT2_SET(&imageBox.minPoint, sx, sy)
            AM_VECT2_SET(&imageBox.maxPoint, sx + width, sy + height)
            
            AM_VECT2_SET(&scissorRect.minPoint, xmin - dx, ymin - dy)
            AM_VECT2_SET(&scissorRect.maxPoint, xmin - dx + widthS, ymin - dy + heightS)

            if (amAABox2iIntersect(&intersectionBox, &imageBox, &scissorRect)){
           
                //it means that the two boxes intersect
                sxNew = intersectionBox.minPoint.x;
                syNew = intersectionBox.minPoint.y;
                widthNew = intersectionBox.maxPoint.x - intersectionBox.minPoint.x;
                heightNew = intersectionBox.maxPoint.y - intersectionBox.minPoint.y;

                //Since we have already cliped 
                dstPtr = (DWORD*) surface->pixels;
                dstStride = (DWORD) surface->dataStride;
                dstStride = dstStride >> 2;
                srcPtr = (DWORD*)srcImg->pixels;
                srcStride = (DWORD) srcImg->dataStride;
    
                //Since image specifies its stride in bytes, convert to pixels
                srcStride = srcStride >> 2;
        
                //Adjust src and dst pointers
                if (!flip){
                    //if we need to flip the image - readjust srcx
                    syNew = srcImg->height - syNew - heightNew;

                    if (syNew < 0){
                        syNew = - syNew;
                    }             
                }       
                srcx = (DWORD)(srcStride * (srcImg->height - syNew - heightNew) + sxNew);
                dstx = (DWORD)(dstStride * (surface->height - dyNew - heightNew) + dxNew);

                if (flip)
                {
                    RasterCopyARGB8888_7kmdp2( srcPtr, srcStride, srcx, dstPtr, dstStride, dstx, widthNew, heightNew, (BOOL) (blend), 0 , IMG_NOROT);
                }
                else
                {
                    RasterCopyARGB8888_7kmdp2( srcPtr, srcStride, srcx, dstPtr, dstStride, dstx, widthNew, heightNew, (BOOL) (blend), 0 , IMG_NOROT_XREF);

                }           
            }
         }else{
            return FALSE;
         }       
    }
    return TRUE;
}
#endif

/*!
	\brief It copies pixel data from the source image onto the drawing surface. Pixels whose source lies outside
	of the bounds of source image or whose destination lies outside the bounds of the drawing
	surface are ignored. Pixel format conversion is applied as needed. Scissoring takes place normally.
	Transformations, masking, and blending are not applied.
	\param context context containing the handle list (used to create / remove a temporary image, when needed) and the scissoring rectangles.
	\param surface the drawing surface where pixels are written to.
	\param dx x-coordinate of the rectangular region to write on the drawing surface.
	\param dy y-coordinate of the rectangular region to write on the drawing surface.
	\param src source image handle.
	\param sx x-coordinate of the rectangular region to read.
	\param sy y-coordinate of the rectangular region to read.
	\param width width of the rectangular region to set, in pixels.
	\param height height of the rectangular region to set, in pixels.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
	\pre width, height > 0.
*/
AMbool amDrawingSurfacePixelsSet(AMContext *context,
								 AMDrawingSurface *surface,
								 AMint32 dx,
								 AMint32 dy,
								 VGImage src,
								 AMint32 sx,
								 AMint32 sy,
								 AMint32 width,
								 AMint32 height) {

	AMImage *srcImg;

	AM_ASSERT(context);
	AM_ASSERT(surface);
	AM_ASSERT(src > 0 && src < context->handles->createdHandlesList.size);
	AM_ASSERT(width > 0 && height > 0);

	srcImg = (AMImage *)context->handles->createdHandlesList.data[src];
	AM_ASSERT(srcImg);

	// clip specified bound to the current drawing surface
	if (sx < 0) {
		width += sx;
		if (width <= 0)
			return AM_TRUE;
		dx -= sx;
		sx = 0;
	}
	if (sy < 0) {
		height += sy;
		if (height <= 0)
			return AM_TRUE;
		dy -= sy;
		sy = 0;
	}
	if (sx + width > srcImg->width) {
		width = srcImg->width - sx;
		if (width <= 0)
			return AM_TRUE;
	}
	if (sy + height > srcImg->height) {
		height = srcImg->height - sy;
		if (height <= 0)
			return AM_TRUE;
	}

	if (dx < 0) {
		width += dx;
		if (width <= 0)
			return AM_TRUE;
		sx -= dx;
		AM_ASSERT(sx < srcImg->width);
		dx = 0;
	}
	if (dy < 0) {
		height += dy;
		if (height <= 0)
			return AM_TRUE;
		sy -= dy;
		AM_ASSERT(sy < srcImg->height);
		dy = 0;
	}
	if (dx + width > amSrfWidthGet(surface)) {
		width = amSrfWidthGet(surface) - dx;
		if (width <= 0)
			return AM_TRUE;
	}
	if (dy + height > amSrfHeightGet(surface)) {
		height = amSrfHeightGet(surface) - dy;
		if (height <= 0)
			return AM_TRUE;
	}

#if defined(AM_SRE)
	return amSreDrawingSurfacePixelsSet(context, surface, dx, dy, src, sx, sy, width, height);
#elif defined(AM_GLE)
	return amGleDrawingSurfacePixelsSet(context, surface, dx, dy, src, sx, sy, width, height);
#elif defined(AM_GLS)
	return amGlsDrawingSurfacePixelsSet(context, surface, dx, dy, src, sx, sy, width, height);
#else
	#error Unreachable point.
#endif
}

/*!
	\brief It copies pixel data onto the drawing surface, without the creation of a VGImage object.
	The pixel values to be drawn are taken from the data pointer at the time of the amDrawingSurfacePixelsWrite
	call, so future changes to the data have no effect. Pixels whose destination coordinate lies outside the
	bounds of the drawing surface are ignored. Pixel format conversion is applied as needed. Scissoring takes
	place normally. Transformations, masking, and blending are not applied.
	\param context context containing the handle list (used to create / remove a temporary image, when needed) and the scissoring rectangles.
	\param surface the drawing surface where pixels are written to.
	\param dx x-coordinate of the rectangular region to write on the drawing surface.
	\param dy y-coordinate of the rectangular region to write on the drawing surface.
	\param data source pixels.
	\param dataFormat format of the source pixels.
	\param dataStride data stride of the source pixels.
	\param width width of the rectangular region to write, in pixels.
	\param height height of the rectangular region to write, in pixels.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
	\pre width, height > 0.
*/
AMbool amDrawingSurfacePixelsWrite(AMContext *context,
								   AMDrawingSurface *surface,
								   AMint32 dx,
								   AMint32 dy,
								   const void *data,
								   const VGImageFormat dataFormat,
								   const AMint32 dataStride,
								   AMint32 width,
								   AMint32 height) {

	AM_ASSERT(context);
	AM_ASSERT(surface);
	AM_ASSERT(data);
	AM_ASSERT(data != amSrfPixelsGet(surface));
	AM_ASSERT(width > 0 && height > 0);

#if defined(AM_SRE)
	return amSreDrawingSurfacePixelsWrite(context, surface, dx, dy, data, dataFormat, dataStride, width, height);
#elif defined(AM_GLE)
	return amGleDrawingSurfacePixelsWrite(context, surface, dx, dy, data, dataFormat, dataStride, width, height);
#elif defined(AM_GLS)
	return amGlsDrawingSurfacePixelsWrite(context, surface, dx, dy, data, dataFormat, dataStride, width, height);
#else
	#error Unreachable point.
#endif
}

/*!
	\brief It retrieves pixel data from the drawing surface into the destination image. Pixels whose source
	lies outside of the bounds of the drawing surface or whose destination lies outside
	the bounds of destination image are ignored. Pixel format conversion is applied as needed. The
	scissoring region does not affect the reading of pixels.
	\param dst destination image.
	\param dx x-coordinate of the rectangular region to write on the destination image.
	\param dy y-coordinate of the rectangular region to write on the destination image.
	\param context context containing the handle list (used to create / remove a temporary image, when needed).
	\param surface the drawing surface where pixels are read from.
	\param sx x-coordinate of the rectangular region to read from the drawing surface.
	\param sy y-coordinate of the rectangular region to read from the drawing surface.
	\param width width of the rectangular region to get, in pixels.
	\param height height of the rectangular region to get, in pixels.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
	\pre width, height > 0.
*/
AMbool amDrawingSurfacePixelsGet(AMImage *dst,
								 AMint32 dx,
								 AMint32 dy,
								 AMContext *context,
								 const AMDrawingSurface *surface,
								 AMint32 sx,
								 AMint32 sy,
								 AMint32 width,
								 AMint32 height) {

	AM_ASSERT(dst);
	AM_ASSERT(context);
	AM_ASSERT(surface);
	AM_ASSERT(width > 0 && height > 0);

	// clip specified bound to the current viewport
	if (sx < 0) {
		width += sx;
		if (width <= 0)
			return AM_TRUE;
		dx -= sx;
		sx = 0;
	}
	if (sy < 0) {
		height += sy;
		if (height <= 0)
			return AM_TRUE;
		dy -= sy;
		sy = 0;
	}
	if (sx + width > amSrfWidthGet(surface)) {
		width = amSrfWidthGet(surface) - sx;
		if (width <= 0)
			return AM_TRUE;
	}
	if (sy + height > amSrfHeightGet(surface)) {
		height = amSrfHeightGet(surface) - sy;
		if (height <= 0)
			return AM_TRUE;
	}

	if (dx < 0) {
		width += dx;
		if (width <= 0)
			return AM_TRUE;
		sx -= dx;
		AM_ASSERT(sx < amSrfWidthGet(surface));
		dx = 0;
	}
	if (dy < 0) {
		height += dy;
		if (height <= 0)
			return AM_TRUE;
		sy -= dy;
		AM_ASSERT(sy < amSrfHeightGet(surface));
		dy = 0;
	}
	if (dx + width > dst->width) {
		width = dst->width - dx;
		if (width <= 0)
			return AM_TRUE;
	}
	if (dy + height > dst->height) {
		height = dst->height - dy;
		if (height <= 0)
			return AM_TRUE;
	}

#if defined(AM_SRE)
	return amSreDrawingSurfacePixelsGet(dst, dx, dy, context, surface, sx, sy, width, height);
#elif defined(AM_GLE) || defined(AM_GLS)
	return amGlDrawingSurfacePixelsGet(dst, dx, dy, context, surface, sx, sy, width, height);
#else
	#error Unreachable point.
#endif
}

/*!
	\brief It allows pixel data to be copied from the drawing surface without the creation of a VGImage object.
	Pixels whose source lies outside of the bounds of the drawing surface are ignored. Pixel format conversion
	is applied as needed. The scissoring region does not affect the reading of pixels.
	\param data destination pixels.
	\param dataFormat data format of destination pixels.
	\param dataStride data stride of destination pixels.
	\param context context containing the handle list (used to create / remove a temporary image, when needed).
	\param surface the drawing surface where pixels are read from.
	\param sx x-coordinate of the rectangular region to read from the drawing surface.
	\param sy y-coordinate of the rectangular region to read from the drawing surface.
	\param width width of the rectangular region to read, in pixels.
	\param height height of the rectangular region to read, in pixels.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
	\pre width, height > 0.
*/
AMbool amDrawingSurfacePixelsRead(void *data,
								  const VGImageFormat dataFormat,
								  const AMint32 dataStride,
								  AMContext *context,
								  const AMDrawingSurface *surface,
								  AMint32 sx,
								  AMint32 sy,
								  AMint32 width,
								  AMint32 height) {

	AMint32 dx, dy;

	AM_ASSERT(data);
	AM_ASSERT(width > 0 && height > 0);
	AM_ASSERT(context);
	AM_ASSERT(surface);

	dx = 0;
	dy = 0;
	// clip specified bound to the current viewport
	if (sx < 0) {
		width += sx;
		if (width <= 0)
			return AM_TRUE;
		dx += (-sx);
		sx = 0;
	}
	if (sy < 0) {
		height += sy;
		if (height <= 0)
			return AM_TRUE;
		dy += (-sy);
		sy = 0;
	}
	if (sx + width > amSrfWidthGet(surface)) {
		width = amSrfWidthGet(surface) - sx;
		if (width <= 0)
			return AM_TRUE;
	}
	if (sy + height > amSrfHeightGet(surface)) {
		height = amSrfHeightGet(surface) - sy;
		if (height <= 0)
			return AM_TRUE;
	}

#if defined(AM_SRE)
	return amSreDrawingSurfacePixelsRead(data, dataFormat, dataStride, dx, dy, context, surface, sx, sy, width, height);
#elif defined(AM_GLE) || defined(AM_GLS)
	return amGlDrawingSurfacePixelsRead(data, dataFormat, dataStride, dx, dy, context, surface, sx, sy, width, height);
#else
	#error Unreachable point.
#endif
}

/*!
	\brief It copies pixels from one region of the drawing surface to another. Copies between overlapping
	regions are allowed and always produce consistent results identical to copying the entire source region
	to a scratch buffer followed by copying the scratch buffer into the destination region.
	Pixels whose source or destination lies outside of the bounds of the drawing surface are ignored.
	Transformations, masking, and blending are not applied. Scissoring is applied to the destination, but
	does not affect the reading of pixels.
	\param context context containing the handle list (used to create / remove a temporary image, when needed).
	\param surface the drawing surface where the copy operation is performed on.
	\param dx x-coordinate of the rectangular region to write onto the drawing surface.
	\param dy y-coordinate of the rectangular region to write onto the drawing surface.
	\param sx x-coordinate of the rectangular region to read from the drawing surface.
	\param sy y-coordinate of the rectangular region to read from the drawing surface.
	\param width width of the rectangular region to copy, in pixels.
	\param height height of the rectangular region to copy, in pixels.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
	\pre width, height > 0.
*/
AMbool amDrawingSurfacePixelsCopy(AMContext *context,
								  AMDrawingSurface *surface,
								  AMint32 dx,
								  AMint32 dy,
								  AMint32 sx,
								  AMint32 sy,
								  AMint32 width,
								  AMint32 height) {

	AM_ASSERT(context);
	AM_ASSERT(surface);
	AM_ASSERT(width >0 && height > 0);

	// clip specified bound
	if (sx < 0) {
		width += sx;
		if (width <= 0)
			return AM_TRUE;
		dx -= sx;
		sx = 0;
	}
	if (sy < 0) {
		height += sy;
		if (height <= 0)
			return AM_TRUE;
		dy -= sy;
		sy = 0;
	}
	if (sx + width > amSrfWidthGet(surface)) {
		width = amSrfWidthGet(surface) - sx;
		if (width <= 0)
			return AM_TRUE;
	}
	if (sy + height > amSrfHeightGet(surface)) {
		height = amSrfHeightGet(surface) - sy;
		if (height <= 0)
			return AM_TRUE;
	}

	if (dx < 0) {
		width += dx;
		if (width <= 0)
			return AM_TRUE;
		sx -= dx;
		dx = 0;
	}
	if (dy < 0) {
		height += dy;
		if (height <= 0)
			return AM_TRUE;
		sy -= dy;
		dy = 0;
	}
	if (dx + width > amSrfWidthGet(surface)) {
		width = amSrfWidthGet(surface) - dx;
		if (width <= 0)
			return AM_TRUE;
	}
	if (dy + height > amSrfHeightGet(surface)) {
		height = amSrfHeightGet(surface) - dy;
		if (height <= 0)
			return AM_TRUE;
	}

#if defined(AM_SRE)
	return amSreDrawingSurfacePixelsCopy(context, surface, dx, dy, sx, sy, width, height);
#elif defined(AM_GLE) || defined(AM_GLS)
	return amGlDrawingSurfacePixelsCopy(context, surface, dx, dy, sx, sy, width, height);
#else
	#error Unreachable point.
#endif
}

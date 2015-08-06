/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://raw.githubusercontent.com/awpe/xwinclone/master/LICENSE.md>.  */

#include <xwcUtil.h>

imgLibCreateCrSc_t imgLibCreateCrSc = & imlib_create_cropped_scaled_image;

Bool
bgImgPrepare (XWCContext * ctx,
              Pixmap     * bgImgPm,
              int        * bgImgWidth,
              int        * bgImgHeight)
{
    Imlib_Image imgSrc, imgScaled;
    char buf[1024];

    if (ctx->bgImgFileSet == True)
    {
        logCtrl ("Reading background image file:", LOG_LVL_NO, False);
    }
    else
    {
        logCtrl ("Reading background image file:", LOG_LVL_1, False);
    }

    imgSrc = imlib_load_image (ctx->bgImgFilePath);

    if (imgSrc == NULL)
    {
        snprintf (buf, sizeof (buf), "\tcannot load background image file"
                  " '%s'!", ctx->bgImgFilePath);

        if (ctx->bgImgFileSet == True)
        {
            logCtrl (buf, LOG_LVL_NO, True);
            return False;
        }
        else
        {
            logCtrl (buf, LOG_LVL_1, True);
        }
    }
    else
    {
        imlib_context_set_image (imgSrc);
        *bgImgWidth  = imlib_image_get_width ();
        *bgImgHeight = imlib_image_get_height ();

        if (   *bgImgWidth  > ctx->rootWAttr.width
            || * bgImgHeight > ctx->rootWAttr.height)
        {
            float scaleFactor = (float) *bgImgWidth / (float) *bgImgHeight;

            int newWidth  = ctx->rootWAttr.width;
            int newHeight = (float) newWidth / scaleFactor;

            snprintf (buf, sizeof (buf), "Image scaled to:\n\twidth:\t%d\n\t"
                      "height:\t%d", newWidth, newHeight);
            logCtrl (buf, LOG_LVL_1, True);

            imgScaled = imgLibCreateCrSc (0, 0, *bgImgWidth, *bgImgHeight,
                                          newWidth, newHeight);
            //imlib_free_image_and_decache ();
            imlib_free_image ();
            imlib_context_set_image (imgScaled);
            *bgImgWidth  = imlib_image_get_width ();
            *bgImgHeight = imlib_image_get_height ();
        }
        else
        {
            imgScaled = imgSrc;
        }


        *bgImgPm = XCreatePixmap (ctx->xDpy, ctx->trgW, *bgImgWidth,
                                  *bgImgHeight, ctx->trgWAttr.depth);

        imlib_context_set_display (ctx->xDpy);
        imlib_context_set_visual (ctx->trgWAttr.visual);
        imlib_context_set_colormap (ctx->trgWAttr.colormap);
        imlib_context_set_drawable (*bgImgPm);

        imlib_render_image_on_drawable (0, 0);

        //imlib_free_image_and_decache ();
        imlib_free_image ();

        if (getXErrState (ctx) == True)
        {
            return False;
        }

        ctx->bgImgStatus = True;

        if (ctx->bgImgFileSet == True)
        {
            logCtrl ("\tsuccess", LOG_LVL_NO, True);
        }
        else
        {
            logCtrl ("\tsuccess", LOG_LVL_1, True);
        }
    }
    return True;
}

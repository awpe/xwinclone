#include "xwc.h"

/*Global X_ERROR initialization*/
Bool X_ERROR = False;

imgLibCreateCrSc_t   imgLibCreateCrSc = & imlib_create_cropped_scaled_image;

XCompRedirWin_t      redirWin         = & XCompositeRedirectWindow;
XCompUnRedirWin_t    unRedirWin       = & XCompositeUnredirectWindow;
XCompRedirSubWin_t   redirSubWin      = & XCompositeRedirectSubwindows;
XCompUnRedirSubWin_t unRedirSubWin    = & XCompositeUnredirectSubwindows;

/*Global LOG_LVL initialization*/
int LOG_LVL = DEFAULT_LOG_LVL;

void
printVersion (void)
{
    char buf[1024];
    snprintf (buf, sizeof (buf), "%s version %s", WM_CLASS_PRG_NAME_STR,
              XWINCLONE_VERSION_STR);
    logCtr (buf, LOG_LVL_NO, False);
}

void
logCtr (const char * msg,
        int          lvl,
        Bool         sequenced)
{
    if (msg == NULL)
    {
        printf ("Tryng to log NULL msg!\n");
        return;
    }

    if (lvl < 0)
    {
        printf ("Tryng to log with unknown lvl!\n");
        return;
    }

    if (LOG_LVL >= lvl)
    {
        if (sequenced == False)
        {
            printf ("\n%s\n", msg);
        }
        else
        {
            printf ("%s\n", msg);
        }
    }
}

Bool
ifSingleInst (XWCContext * ctx)
{
    char buf[1024];
    int  rc;

    logCtr ("Creating lock file:", LOG_LVL_1, False);

    if (ctx == NULL)
    {
        logCtr ("\tError: null pointer to context!", LOG_LVL_NO, True);
    }

    ctx->lckFD = open (ctx->lckFPath, O_CREAT | O_RDWR, 0666);

    if (ctx == NULL)
    {
        snprintf (buf, sizeof (buf), "\tError: cannot create lock file %s!",
                  ctx->lckFPath);
        logCtr (buf, LOG_LVL_NO, True);
    }

    rc = flock (ctx->lckFD, LOCK_EX | LOCK_NB);

    if (rc != 0 && EWOULDBLOCK == errno)
    {
        snprintf (buf, sizeof (buf), "File %s seems to be already created and"
                  " locked,\nassuming other instance of this program is"
                  " running", ctx->lckFPath);
        logCtr (buf, LOG_LVL_NO, False);
        return False;
    }

    return True;
}

Bool
bgImgPrepare (XWCContext        * ctx,
              Pixmap            * bgImgPm,
              unsigned int      * bgImgWidth,
              unsigned int      * bgImgHeight)
{
    Imlib_Image imgSrc, imgScaled;
    char buf[1024];

    if (ctx->bgImgFileSet == True)
    {
        logCtr ("Reading background image file:", LOG_LVL_NO, False);
    }
    else
    {
        logCtr ("Reading background image file:", LOG_LVL_1, False);
    }

    imgSrc = imlib_load_image (ctx->bgImgFilePath);

    if (imgSrc == NULL)
    {
        snprintf (buf, sizeof (buf), "\tcannot load background image file"
                  " '%s'!", ctx->bgImgFilePath);

        if (ctx->bgImgFileSet == True)
        {
            logCtr (buf, LOG_LVL_NO, True);
            return False;
        }
        else
        {
            logCtr (buf, LOG_LVL_1, True);
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
            logCtr (buf, LOG_LVL_1, True);

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

        XSync (ctx->xDpy, 0);

        //imlib_free_image_and_decache ();
        imlib_free_image ();
        ctx->bgImgStatus = True;

        if (ctx->bgImgFileSet == True)
        {
            logCtr ("\tsuccess", LOG_LVL_NO, True);
        }
        else
        {
            logCtr ("\tsuccess", LOG_LVL_1, True);
        }
    }
    return True;
}

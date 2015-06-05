#include "xwclib.h"

int
main (int     argc,
      char ** argv)
{
    /**************************************************************************/
    /*Variables declaration*/
    /**************************************************************************/
    XWCOptions           * ctx;
    Screen               * srcScreen;
    Window                 srcWin, rootWinOfSrc, trgWin;
    XWindowAttributes      srcWinAttr, trgWinAttr;
    XSetWindowAttributes   trgWinSetAttr;
    GC                     xGraphicsCtx;
    XVisualInfo            xVisInfo;
    Visual               * xVis;
    Pixmap                 pm, srcWinCompPm, bgImgPm, srcWinCompPmOld;
    char                   buf[1024];
    int                    retVal, trgWinLeftOff, trgWinTOff, pressedKey;
    unsigned int           bgImgWidth, bgImgHeight;
    /**************************************************************************/


    /**************************************************************************/
    /*Variables initialization*/
    /**************************************************************************/
    retVal               = EXIT_SUCCESS;
    trgWinLeftOff     = 0;
    trgWinTOff        = 0;
    bgImgPm              = 0;
    bgImgWidth           = 0;
    bgImgHeight          = 0;
    pressedKey           = NO_KEY_PRESSED;
    /**************************************************************************/


    /**************************************************************************/
    /*Print program version*/
    /**************************************************************************/
    printVersion ();
    /**************************************************************************/


    /**************************************************************************/
    /*Initialize program context*/
    /**************************************************************************/
    if ((ctx = init (argc, (const char **) argv)) == NULL)
    {
        return EXIT_FAILURE;
    }
    /**************************************************************************/


    /**************************************************************************/
    /*Program cycle*/
    /**************************************************************************/
    while (pressedKey != EXIT_COMBINATION)
    {
        /**********************************************************************/
        /*Wait for user input if in daemon mode*/
        /**********************************************************************/
        if (ctx->isDaemon == True)
        {
            snprintf (buf, sizeof (buf), "Move focus to desired window and"
                      " press %s to start translation", ctx->transCtrlKeyStr);

            logCtr (buf, LOG_LVL_NO, False);

            while ((pressedKey = getPressedComb (ctx)) == NO_KEY_PRESSED)
            {
                nanosleep (&ctx->frameDelay, NULL);
            }

            if (pressedKey == EXIT_COMBINATION)
            {
                break;
            }
        }
        /**********************************************************************/


        /**********************************************************************/
        /*Get source window and its attributes*/
        /**********************************************************************/
        if ((srcWin = getActiveWindow (ctx)) == None)
        {
            XCloseDisplay (ctx->xDpy);
            free (ctx);
            return EXIT_FAILURE;
        }

        XGetWindowAttributes (ctx->xDpy, srcWin, &srcWinAttr);

        if (getXErrState () == True)
        {
            retVal = EXIT_FAILURE;
            break;
        }

        printWindowInfo (ctx->xDpy, srcWin, &srcWinAttr);
        /**********************************************************************/


        /**********************************************************************/
        /*Check if source window screen is still the same*/
        /**********************************************************************/
        if ((srcScreen = getScreenByWindowAttr (ctx, &srcWinAttr)) == NULL)
        {
            retVal = EXIT_FAILURE;
            break;
        }

        if (srcScreen != ctx->xScr)
        {
            ctx->xScr = srcScreen;

            if (parseColor (ctx) == False)
            {
                retVal = EXIT_FAILURE;
                break;
            }

            if (( rootWinOfSrc = getRootWinOfScr (ctx->xScr) ) == None)
            {
                retVal = EXIT_FAILURE;
                break;
            }

            if (rootWinOfSrc != ctx->rootWin)
            {
                ungrabKeys (ctx);

                ctx->rootWin = rootWinOfSrc;

                XGetWindowAttributes (ctx->xDpy, ctx->rootWin,
                                      &ctx->rootWinAttr);

                if (getXErrState () == True)
                {
                    retVal = EXIT_FAILURE;
                    break;
                }

                if (grabKeys (ctx) == False)
                {
                    retVal = EXIT_FAILURE;
                    break;
                }
            }
        }
        /**********************************************************************/


        /**********************************************************************/
        /*Create translation window*/
        /**********************************************************************/
        logCtr ("Creating translation window:", LOG_LVL_NO, False);

        if (getVisualOfScr (ctx->xScr, srcWinAttr.depth, &xVisInfo) == False)
        {
            logCtr ("\tError: no such visual", LOG_LVL_NO, False);
            retVal = EXIT_FAILURE;
            break;
        }

        xVis                           = xVisInfo.visual;
        trgWinSetAttr.colormap         = XCreateColormap (ctx->xDpy, srcWin,
                                                          xVis, AllocNone);
        trgWinSetAttr.background_pixel = ctx->bgColor.pixel;
        trgWinSetAttr.border_pixel     = 0;
        trgWinSetAttr.bit_gravity      = NorthWestGravity;

        trgWin = XCreateWindow (ctx->xDpy, ctx->rootWin, 0, 0, srcWinAttr.width,
                                srcWinAttr.height, 0, srcWinAttr.depth,
                                InputOutput, xVis, CWBackPixel | CWColormap
                                | CWBorderPixel | CWBitGravity, &trgWinSetAttr);

        if (getXErrState () == True)
        {
            logCtr ("\tfailed to create window!", LOG_LVL_NO, False);
            if (trgWin != None)
            {
                XDestroyWindow (ctx->xDpy, trgWin);
            }
            retVal = EXIT_FAILURE;
            break;
        }

        if (setWinTitlebar (ctx->xDpy, trgWin, WM_CLASS_PRG_NAME_STR) == False)
        {
            XDestroyWindow (ctx->xDpy, trgWin);
            retVal = EXIT_FAILURE;
            break;
        }

        if (setWindowClass (ctx->xDpy, trgWin, WM_CLASS_PRG_NAME_STR,
                            WM_CLASS_CLASS_NAME_STR ) == False)
        {
            XDestroyWindow (ctx->xDpy, trgWin);
            retVal = EXIT_FAILURE;
            break;
        }

        XMapWindow (ctx->xDpy, trgWin);
        XSync (ctx->xDpy, 0);

        if (getXErrState () == True)
        {
            logCtr ("\tfailed to map window!", LOG_LVL_NO, False);
            XUnmapWindow (ctx->xDpy, trgWin);
            XDestroyWindow (ctx->xDpy, trgWin);
            retVal = EXIT_FAILURE;
            break;
        }

        XGetWindowAttributes (ctx->xDpy, trgWin, &trgWinAttr);

        if (getXErrState () == True)
        {
            XUnmapWindow (ctx->xDpy, trgWin);
            XDestroyWindow (ctx->xDpy, trgWin);
            retVal = EXIT_FAILURE;
            break;
        }

        printWindowInfo (ctx->xDpy, trgWin, &trgWinAttr);

        XSync (ctx->xDpy, 0);

        logCtr ("\tsuccess", LOG_LVL_NO, True);
        /**********************************************************************/


        /**********************************************************************/
        /*Prepare temporary pixmap*/
        /**********************************************************************/
        pm = XCreatePixmap (ctx->xDpy, trgWin, ctx->rootWinAttr.width,
                            ctx->rootWinAttr.height, srcWinAttr.depth);
        xGraphicsCtx = XCreateGC (ctx->xDpy, pm, 0, NULL);
        XSetForeground (ctx->xDpy, xGraphicsCtx, ctx->bgColor.pixel);
        XFillRectangle (ctx->xDpy, pm, xGraphicsCtx, 0, 0,
                        ctx->rootWinAttr.width, ctx->rootWinAttr.height);

        XSync (ctx->xDpy, 0);

        if (getXErrState () == True)
        {
            XFreeGC (ctx->xDpy, xGraphicsCtx);
            XFreePixmap (ctx->xDpy, pm);
            XUnmapWindow (ctx->xDpy, trgWin);
            XDestroyWindow (ctx->xDpy, trgWin);
            retVal = EXIT_FAILURE;
            break;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*Prepare background image*/
        /**********************************************************************/
        if (bgImgPrepare (ctx, &bgImgPm, &bgImgWidth, &bgImgHeight, trgWin,
                          &trgWinAttr) == False)
        {
            XFreeGC (ctx->xDpy, xGraphicsCtx);
            XFreePixmap (ctx->xDpy, pm);
            XUnmapWindow (ctx->xDpy, trgWin);
            XDestroyWindow (ctx->xDpy, trgWin);
            retVal = EXIT_FAILURE;
            break;
        }

        if (ctx->bgImgStatus == True)
        {
            XCopyArea (ctx->xDpy, bgImgPm, pm, xGraphicsCtx, 0, 0, bgImgWidth,
                       bgImgHeight, 0, 0);
        }

        XSync (ctx->xDpy, 0);

        if (getXErrState () == True)
        {
            XFreeGC (ctx->xDpy, xGraphicsCtx);
            XFreePixmap (ctx->xDpy, pm);
            if (bgImgPm != 0)
            {
                XFreePixmap (ctx->xDpy, bgImgPm);
            }
            XUnmapWindow (ctx->xDpy, trgWin);
            XDestroyWindow (ctx->xDpy, trgWin);
            retVal = EXIT_FAILURE;
            break;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*Redirect source window to pixmap and check it*/
        /**********************************************************************/
        XCompRedirWin (ctx->xDpy, srcWin, CompositeRedirectAutomatic);
        XCompRedirSubWin (ctx->xDpy, srcWin, CompositeRedirectAutomatic);
        XSync (ctx->xDpy, 0);
        srcWinCompPm = XCompositeNameWindowPixmap (ctx->xDpy, srcWin);

        XSync (ctx->xDpy, 0);

        if (getXErrState () == True)
        {
            XCompUnRedirWin (ctx->xDpy, srcWin, CompositeRedirectAutomatic);
            XCompUnRedirSubWin (ctx->xDpy, srcWin, CompositeRedirectAutomatic);
            XFreeGC (ctx->xDpy, xGraphicsCtx);
            XFreePixmap (ctx->xDpy, pm);
            XFreePixmap (ctx->xDpy, srcWinCompPm);
            if (bgImgPm != 0)
            {
                XFreePixmap (ctx->xDpy, bgImgPm);
            }
            XUnmapWindow (ctx->xDpy, trgWin);
            XDestroyWindow (ctx->xDpy, trgWin);
            retVal = EXIT_FAILURE;
            break;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*Start drawing frames */
        /**********************************************************************/
        while ((pressedKey = getPressedComb (ctx)) == NO_KEY_PRESSED)
        {
            nanosleep (&ctx->frameDelay, NULL);

            XGetWindowAttributes (ctx->xDpy, trgWin, &trgWinAttr);
            XGetWindowAttributes (ctx->xDpy, srcWin, &srcWinAttr);

            if (srcWinAttr.map_state == IsViewable)
            {
                srcWinCompPmOld = srcWinCompPm;

                srcWinCompPm = XCompositeNameWindowPixmap (ctx->xDpy, srcWin);
                XSync (ctx->xDpy, 0);
                XGetWindowAttributes (ctx->xDpy, srcWin, &srcWinAttr);

                if (getXErrState () == True)
                {
                    if (srcWinAttr.map_state != IsViewable)
                    {
                        srcWinCompPm = None;
                        logCtr ("Spurious error: An attempt to remap window"
                                " during pixmap creation!", LOG_LVL_1, False);
                        nanosleep (&ctx->longDelay, NULL);
                        continue;
                    }
                    retVal = EXIT_FAILURE;
                    break;
                }

                if (srcWinCompPmOld != None)
                {
                    XFreePixmap (ctx->xDpy, srcWinCompPmOld);
                }

                XFillRectangle (ctx->xDpy, pm, xGraphicsCtx, 0, 0,
                                ctx->rootWinAttr.width,
                                ctx->rootWinAttr.height);

                if (ctx->bgImgStatus == True)
                {
                    XCopyArea (ctx->xDpy, bgImgPm, pm, xGraphicsCtx, 0, 0,
                               bgImgWidth, bgImgHeight, 0, 0);
                }

                if (ctx->autoCenter == True)
                {
                    trgWinLeftOff = (trgWinAttr.width - srcWinAttr.width) / 2;
                    trgWinTOff  = (trgWinAttr.height - srcWinAttr.height +
                        ctx->topOffset) / 2;
                }

                XCopyArea (ctx->xDpy, srcWinCompPm, pm, xGraphicsCtx,
                           0,                0 + ctx->topOffset,
                           srcWinAttr.width, srcWinAttr.height - ctx->topOffset,
                           trgWinLeftOff, trgWinTOff);
            }

            if (trgWinAttr.map_state == IsViewable)
            {
                XCopyArea (ctx->xDpy, pm, trgWin, xGraphicsCtx, 0, 0,
                           trgWinAttr.width, trgWinAttr.height, 0, 0);
            }

            if (   trgWinAttr.map_state != IsViewable
                || srcWinAttr.map_state != IsViewable)
            {
                nanosleep (&ctx->longDelay, NULL);
            }

            if (getXErrState () == True)
            {
                retVal = EXIT_FAILURE;
                break;
            }
        }
        /**********************************************************************/


        /**********************************************************************/
        /*Clear old data*/
        /**********************************************************************/
        XCompUnRedirWin (ctx->xDpy, srcWin, CompositeRedirectAutomatic);
        XCompUnRedirSubWin (ctx->xDpy, srcWin, CompositeRedirectAutomatic);
        XFreeGC (ctx->xDpy, xGraphicsCtx);
        XFreePixmap (ctx->xDpy, pm);
        if (srcWinCompPm != None)
        {
            XFreePixmap (ctx->xDpy, srcWinCompPm);
        }
        if (bgImgPm != 0)
        {
            XFreePixmap (ctx->xDpy, bgImgPm);
        }
        XUnmapWindow (ctx->xDpy, trgWin);
        XDestroyWindow (ctx->xDpy, trgWin);
        /**********************************************************************/


        /**********************************************************************/
        /*Check exit conditions*/
        /**********************************************************************/
        if (   pressedKey    == EXIT_COMBINATION
            || ctx->isDaemon == 0
            || retVal        == EXIT_FAILURE)
        {
            break;
        }
        /**********************************************************************/
    }

    ungrabKeys (ctx);
    XCloseDisplay (ctx->xDpy);
    free (ctx);

    return retVal;
}

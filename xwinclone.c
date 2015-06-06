#include "xwclib.h"

int
main (int     argc,
      char ** argv)
{
    /**************************************************************************/
    /*Variables declaration*/
    /**************************************************************************/
    XWCContext           * ctx;
    Screen               * scrOfSrc;
    Window                 rootWinOfSrc;
    GC                     xGraphicsCtx;
    XGCValues              xGCtxVals;
    Pixmap                 pm, srcWinPm, bgImgPm, srcWinPmOld;
    char                   buf[1024];
    int                    retVal, trgWinLeftOff, trgWinTOff, pressedKey,
        trgWinW, trgWinH, srcWinW, srcWinH;
    unsigned int           bgImgWidth, bgImgHeight;
    /**************************************************************************/


    /**************************************************************************/
    /*Variables initialization*/
    /**************************************************************************/
    retVal               = EXIT_SUCCESS;
    trgWinLeftOff        = 0;
    trgWinTOff           = 0;
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
        if ((ctx->srcWin = getActiveWindow (ctx)) == None)
        {
            XCloseDisplay (ctx->xDpy);
            free (ctx);
            return EXIT_FAILURE;
        }

        XGetWindowAttributes (ctx->xDpy, ctx->srcWin, &ctx->srcWinAttr);

        if (getXErrState () == True)
        {
            retVal = EXIT_FAILURE;
            break;
        }

        printWindowInfo (ctx->xDpy, ctx->srcWin, &ctx->srcWinAttr);
        /**********************************************************************/


        /**********************************************************************/
        /*Check if source window screen is still the same*/
        /**********************************************************************/
        if ((scrOfSrc = getScreenByWindowAttr (ctx, &ctx->srcWinAttr)) == NULL)
        {
            retVal = EXIT_FAILURE;
            break;
        }

        if (scrOfSrc != ctx->xScr)
        {
            ctx->xScr = scrOfSrc;

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
        logCtr ("Creating translation window:", LOG_LVL_1, False);

        if (createTrgWindow (ctx) == False)
        {
            retVal = EXIT_FAILURE;
            break;
        }

        XMapWindow (ctx->xDpy, ctx->trgWin);
        
        if (getXErrState () == True)
        {
            logCtr ("\tfailed to map window!", LOG_LVL_NO, False);
            XUnmapWindow (ctx->xDpy, ctx->trgWin);
            XDestroyWindow (ctx->xDpy, ctx->trgWin);
            retVal = EXIT_FAILURE;
            break;
        }

        XGetWindowAttributes (ctx->xDpy, ctx->trgWin, &ctx->trgWinAttr);

        if (getXErrState () == True)
        {
            XUnmapWindow (ctx->xDpy, ctx->trgWin);
            XDestroyWindow (ctx->xDpy, ctx->trgWin);
            retVal = EXIT_FAILURE;
            break;
        }

        printWindowInfo (ctx->xDpy, ctx->trgWin, &ctx->trgWinAttr);

        logCtr ("\tsuccess", LOG_LVL_1, True);
        /**********************************************************************/


        /**********************************************************************/
        /*Prepare temporary pixmap*/
        /**********************************************************************/
        pm = XCreatePixmap (ctx->xDpy, ctx->trgWin, ctx->rootWinAttr.width,
                            ctx->rootWinAttr.height, ctx->srcWinAttr.depth);
        xGCtxVals.graphics_exposures = False;
        xGraphicsCtx = XCreateGC (ctx->xDpy, pm, GCGraphicsExposures,
                                  &xGCtxVals);

        XSetForeground (ctx->xDpy, xGraphicsCtx, ctx->bgColor.pixel);
        XFillRectangle (ctx->xDpy, pm, xGraphicsCtx, 0, 0,
                        ctx->rootWinAttr.width, ctx->rootWinAttr.height);

        if (getXErrState () == True)
        {
            XFreeGC (ctx->xDpy, xGraphicsCtx);
            XFreePixmap (ctx->xDpy, pm);
            XUnmapWindow (ctx->xDpy, ctx->trgWin);
            XDestroyWindow (ctx->xDpy, ctx->trgWin);
            retVal = EXIT_FAILURE;
            break;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*Prepare background image*/
        /**********************************************************************/
        if (bgImgPrepare (ctx, &bgImgPm, &bgImgWidth, &bgImgHeight, ctx->trgWin,
                          &ctx->trgWinAttr) == False)
        {
            XFreeGC (ctx->xDpy, xGraphicsCtx);
            XFreePixmap (ctx->xDpy, pm);
            XUnmapWindow (ctx->xDpy, ctx->trgWin);
            XDestroyWindow (ctx->xDpy, ctx->trgWin);
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
            XUnmapWindow (ctx->xDpy, ctx->trgWin);
            XDestroyWindow (ctx->xDpy, ctx->trgWin);
            retVal = EXIT_FAILURE;
            break;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*Redirect source window to pixmap and check it*/
        /**********************************************************************/
        redirWin (ctx->xDpy, ctx->srcWin, CompositeRedirectAutomatic);
        redirSubWin (ctx->xDpy, ctx->srcWin, CompositeRedirectAutomatic);
        XSync (ctx->xDpy, 0);
        srcWinPm = XCompositeNameWindowPixmap (ctx->xDpy, ctx->srcWin);

        if (getXErrState () == True)
        {
            unRedirWin (ctx->xDpy, ctx->srcWin, CompositeRedirectAutomatic);
            unRedirSubWin (ctx->xDpy, ctx->srcWin, CompositeRedirectAutomatic);
            XFreeGC (ctx->xDpy, xGraphicsCtx);
            XFreePixmap (ctx->xDpy, pm);
            XFreePixmap (ctx->xDpy, srcWinPm);
            if (bgImgPm != 0)
            {
                XFreePixmap (ctx->xDpy, bgImgPm);
            }
            XUnmapWindow (ctx->xDpy, ctx->trgWin);
            XDestroyWindow (ctx->xDpy, ctx->trgWin);
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
            printf ("works1\n");
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            XGetWindowAttributes (ctx->xDpy, ctx->trgWin, &ctx->trgWinAttr);
            nanosleep (&ctx->frameDelay, NULL);
            printf ("works2\n");
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            XGetWindowAttributes (ctx->xDpy, ctx->srcWin, &ctx->srcWinAttr);
            nanosleep (&ctx->frameDelay, NULL);
            printf ("works3\n");
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            nanosleep (&ctx->frameDelay, NULL);
            trgWinW = ctx->trgWinAttr.width;
            srcWinW = ctx->srcWinAttr.width;
            trgWinH = ctx->trgWinAttr.height;
            srcWinH = ctx->srcWinAttr.height;

            if (ctx->srcWinAttr.map_state == IsViewable)
            {
                srcWinPmOld = srcWinPm;
                printf ("works4\n");


                srcWinPm = XCompositeNameWindowPixmap (ctx->xDpy, ctx->srcWin);
                nanosleep (&ctx->frameDelay, NULL);
                printf ("works5\n");

                XSync (ctx->xDpy, 0);
                printf ("works6\n");

                XGetWindowAttributes (ctx->xDpy, ctx->srcWin, &ctx->srcWinAttr);
                nanosleep (&ctx->frameDelay, NULL);
                printf ("works7\n");

                if (getXErrState () == True)
                {
                    if (ctx->srcWinAttr.map_state == IsViewable)
                    {
                        retVal = EXIT_FAILURE;
                        break;
                    }
                    srcWinPm = None;
                    logCtr ("Spurious error: An attempt to remap window"
                            " during pixmap creation!", LOG_LVL_1, False);
                    nanosleep (&ctx->longDelay, NULL);
                    continue;
                }

                nanosleep (&ctx->frameDelay, NULL);
                printf ("works8\n");

                if (srcWinPmOld != None)
                {
                    XFreePixmap (ctx->xDpy, srcWinPmOld);
                }

                XFillRectangle (ctx->xDpy, pm, xGraphicsCtx, 0, 0,
                                ctx->rootWinAttr.width,
                                ctx->rootWinAttr.height);
                nanosleep (&ctx->frameDelay, NULL);
                printf ("works9\n");
                if (ctx->bgImgStatus == True)
                {
                    XCopyArea (ctx->xDpy, bgImgPm, pm, xGraphicsCtx, 0, 0,
                               bgImgWidth, bgImgHeight, 0, 0);
                }
                nanosleep (&ctx->frameDelay, NULL);
                printf ("works10\n");
                if (ctx->autoCenter == True)
                {
                    trgWinLeftOff = trgWinW - srcWinW;
                    trgWinTOff    = trgWinH - srcWinH + ctx->topOffset;

                    trgWinLeftOff >>= 1;
                    trgWinTOff    >>= 1;
                }

                XCopyArea (ctx->xDpy, srcWinPm, pm, xGraphicsCtx,
                           0,             0       + ctx->topOffset,
                           srcWinW,       srcWinH - ctx->topOffset,
                           trgWinLeftOff, trgWinTOff);
            }

            if (ctx->trgWinAttr.map_state == IsViewable)
            {
                XCopyArea (ctx->xDpy, pm, ctx->trgWin, xGraphicsCtx, 0, 0,
                           trgWinW, trgWinH, 0, 0);
            }

            if (   ctx->trgWinAttr.map_state != IsViewable
                || ctx->srcWinAttr.map_state != IsViewable)
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
        /*Free old resources*/
        /**********************************************************************/
        unRedirWin (ctx->xDpy, ctx->srcWin, CompositeRedirectAutomatic);
        unRedirSubWin (ctx->xDpy, ctx->srcWin, CompositeRedirectAutomatic);
        XFreeGC (ctx->xDpy, xGraphicsCtx);
        XFreePixmap (ctx->xDpy, pm);
        if (srcWinPm != None)
        {
            XFreePixmap (ctx->xDpy, srcWinPm);
        }
        if (bgImgPm != 0)
        {
            XFreePixmap (ctx->xDpy, bgImgPm);
        }
        XUnmapWindow (ctx->xDpy, ctx->trgWin);
        XSync (ctx->xDpy, 0);
        XDestroyWindow (ctx->xDpy, ctx->trgWin);
        XSync (ctx->xDpy, 0);
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

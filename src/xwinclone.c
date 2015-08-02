#include <defines.h>
#include <systemHeaders.h>
#include <xwcHeaders.h>

int
main (int     argc,
      char ** argv)
{
    /**************************************************************************/
    /*Variables declaration*/
    /**************************************************************************/
    XWCContext           * ctx;
    GC                     xGC;
    XGCValues              xGCVals;
    Pixmap                 pm, srcWinPm, bgImgPm, srcWinPmOld;
    char                   buf[1024];
    int                    retVal, trgWinLOff, trgWinTOff, pressedKey, trgWinW;
    int                    trgWinH, srcWinW, srcWinH, bgImgW, bgImgH;
    /**************************************************************************/


    /**************************************************************************/
    /*Variables initialization*/
    /**************************************************************************/
    pressedKey                 = NO_KEY_PRESSED;
    retVal                     = EXIT_FAILURE;
    xGCVals.graphics_exposures = False;
    LOG_FILE                   = stdout;
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
        /*All breaks considered to be caused by some error*/
        /**********************************************************************/
        retVal = EXIT_FAILURE;
        /**********************************************************************/


        /**********************************************************************/
        /*Reset resources*/
        /**********************************************************************/
        memset (&ctx->srcWAttr, 0 , sizeof (ctx->srcWAttr));
        memset (&ctx->trgWAttr, 0 , sizeof (ctx->srcWAttr));
        xGC           = NULL;
        ctx->srcW     = None;
        ctx->trgW     = None;
        pm            = None;
        srcWinPm      = None;
        bgImgPm       = None;
        srcWinPmOld   = None;
        trgWinLOff    = 0;
        trgWinTOff    = 0;
        pressedKey    = 0;
        trgWinW       = 0;
        trgWinH       = 0;
        srcWinW       = 0;
        srcWinH       = 0;
        bgImgW        = 0;
        bgImgH        = 0;
        /**********************************************************************/


        /**********************************************************************/
        /*Wait for user input if in daemon mode*/
        /**********************************************************************/

        if (ctx->isDaemon == True)
        {
            snprintf (buf, sizeof (buf), "Move focus to desired window and"
                      " press %s to start translation", ctx->transCtrlKeyStr);

            logCtrl (buf, LOG_LVL_NO, False);

            while ((pressedKey = getPressedComb (ctx)) == NO_KEY_PRESSED)
            {
                nanosleep (&ctx->raiseDelay, NULL);
            }

            if (pressedKey == ERROR_GETTING_COMBINATION)
            {
                break;
            }

            if (pressedKey == EXIT_COMBINATION)
            {
                retVal = EXIT_SUCCESS;
                break;
            }
        }
        /**********************************************************************/


        /**********************************************************************/
        /*Get source window and its attributes*/
        /**********************************************************************/
        if (ctx->isDaemon == False)
        {
            logCtrl ("Waiting for focus to be moved to source window",
                     LOG_LVL_NO, False);
            nanosleep (&ctx->focusDelay, NULL);
        }

        if ( (ctx->srcW = getActiveWindow (ctx, ctx->srcW)) == None)
        {
            goto freeResources;
        }

        XGetWindowAttributes (ctx->xDpy, ctx->srcW, &ctx->srcWAttr);

        if (getXErrState (ctx) == True)
        {
            goto freeResources;
        }

        printWindowInfo (ctx, ctx->srcW, &ctx->srcWAttr);
        /**********************************************************************/


        /**********************************************************************/
        /*Check if new source window screen is still the same*/
        /**********************************************************************/
        if (ctx->srcWAttr.screen != ctx->xScr)
        {
            ctx->xScr = ctx->srcWAttr.screen;

            if (parseColor (ctx) == False)
            {
                goto freeResources;
            }

            if (ctx->srcWAttr.root != ctx->rootW)
            {
                ctx->rootW = ctx->srcWAttr.root;

                XGetWindowAttributes (ctx->xDpy, ctx->rootW, &ctx->rootWAttr);

                if (getXErrState (ctx) == True)
                {
                    goto freeResources;
                }
            }
        }
        /**********************************************************************/


        /**********************************************************************/
        /*Redirect source window to pixmap and check it*/
        /**********************************************************************/
        redirWin (ctx->xDpy, ctx->srcW, CompositeRedirectAutomatic);
        redirSubWin (ctx->xDpy, ctx->srcW, CompositeRedirectAutomatic);
        srcWinPm = XCompositeNameWindowPixmap (ctx->xDpy, ctx->srcW);

        if (getXErrState (ctx) == True)
        {
            goto freeResources;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*Create translation window*/
        /**********************************************************************/
        if (createTrgWin (ctx) == False)
        {
            goto freeResources;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*Prepare temporary pixmap*/
        /**********************************************************************/
        pm = XCreatePixmap (ctx->xDpy, ctx->trgW, ctx->rootWAttr.width,
                            ctx->rootWAttr.height, ctx->srcWAttr.depth);

        xGC = XCreateGC (ctx->xDpy, pm, GCGraphicsExposures, &xGCVals);

        XSetForeground (ctx->xDpy, xGC, ctx->bgColor.pixel);
        XFillRectangle (ctx->xDpy, pm, xGC, 0, 0,
                        ctx->rootWAttr.width, ctx->rootWAttr.height);

        if (getXErrState (ctx) == True)
        {
            goto freeResources;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*Prepare background image*/
        /**********************************************************************/
        if (bgImgPrepare (ctx, &bgImgPm, &bgImgW, &bgImgH) == False)
        {
            goto freeResources;
        }

        if (ctx->bgImgStatus == True)
        {
            XCopyArea (ctx->xDpy, bgImgPm, pm, xGC, 0, 0, bgImgW, bgImgH, 0, 0);
        }

        if (getXErrState (ctx) == True)
        {
            goto freeResources;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*Start drawing frames */
        /**********************************************************************/
        while ((pressedKey = getPressedComb (ctx)) == NO_KEY_PRESSED)
        {
            nanosleep (&ctx->frameDelay, NULL);

            if (   getWAttr (ctx, &ctx->srcW, &ctx->srcWAttr) == False
                || getWAttr (ctx, &ctx->trgW, &ctx->trgWAttr) == False)
            {
                break;
            }

            trgWinW = ctx->trgWAttr.width;
            srcWinW = ctx->srcWAttr.width;
            trgWinH = ctx->trgWAttr.height;
            srcWinH = ctx->srcWAttr.height;

            if (ctx->srcWAttr.map_state == IsViewable)
            {
                srcWinPmOld = srcWinPm;
                srcWinPm    = XCompositeNameWindowPixmap (ctx->xDpy, ctx->srcW);

                if (getXErrState (ctx) == True)
                {
                    XGetWindowAttributes (ctx->xDpy, ctx->srcW, &ctx->srcWAttr);
                    if (ctx->srcWAttr.map_state == IsViewable)
                    {
                        break;
                    }
                    srcWinPm = None;
                    logCtrl ("Spurious error: An attempt to remap window "
                             "during pixmap creation!", LOG_LVL_2, False);
                    nanosleep (&ctx->longWait, NULL);
                    continue;
                }

                if (srcWinPmOld != None)
                {
                    XFreePixmap (ctx->xDpy, srcWinPmOld);
                }

                XFillRectangle (ctx->xDpy, pm, xGC, 0, 0,
                                ctx->rootWAttr.width,
                                ctx->rootWAttr.height);

                if (ctx->bgImgStatus == True)
                {
                    XCopyArea (ctx->xDpy, bgImgPm, pm, xGC, 0, 0,
                               bgImgW, bgImgH, 0, 0);
                }

                if (ctx->autoCenter == True)
                {
                    trgWinLOff = trgWinW - srcWinW;
                    trgWinTOff = trgWinH - srcWinH + ctx->topOffset;

                    trgWinLOff /= 2;
                    trgWinTOff /= 2;
                }

                XCopyArea (ctx->xDpy, srcWinPm, pm, xGC,
                           0,             0       + ctx->topOffset,
                           srcWinW,       srcWinH - ctx->topOffset,
                           trgWinLOff,    trgWinTOff);
            }

            if (ctx->trgWAttr.map_state == IsViewable)
            {
                XCopyArea (ctx->xDpy, pm, ctx->trgW, xGC, 0, 0,
                           trgWinW, trgWinH, 0, 0);
            }

            if (   ctx->trgWAttr.map_state != IsViewable
                || ctx->srcWAttr.map_state != IsViewable)
            {
                nanosleep (&ctx->longWait, NULL);
            }

            if (getXErrState (ctx) == True)
            {
                break;
            }

        }
        /**********************************************************************/


        /**********************************************************************/
        /*Free resources*/
        /**********************************************************************/
freeResources:
        if (ctx->srcW != None)
        {
            unRedirWin (ctx->xDpy, ctx->srcW, CompositeRedirectAutomatic);
            unRedirSubWin (ctx->xDpy, ctx->srcW, CompositeRedirectAutomatic);
        }
        if (xGC != NULL)
        {
            XFreeGC (ctx->xDpy, xGC);
        }
        if (pm != None)
        {
            XFreePixmap (ctx->xDpy, pm);
        }
        if (srcWinPm != None && ctx->srcW != None)
        {
            XFreePixmap (ctx->xDpy, srcWinPm);
        }
        if (bgImgPm != None)
        {
            XFreePixmap (ctx->xDpy, bgImgPm);
        }
        if (ctx->trgW != None)
        {
            XUnmapWindow (ctx->xDpy, ctx->trgW);
            XDestroyWindow (ctx->xDpy, ctx->trgW);
        }

        if (getXErrState (ctx) == True)
        {
            logCtrl ("Error freeing resources\n", LOG_LVL_1, False);
        }

        /**********************************************************************/


        /**********************************************************************/
        /*Check exit conditions*/
        /**********************************************************************/
        if (pressedKey == NO_KEY_PRESSED)
        {
            break;
        }

        retVal = EXIT_SUCCESS;

        if (ctx->isDaemon == 0)
        {
            break;
        }
        /**********************************************************************/
    }

    ungrabAllKeys (ctx);

    XCloseDisplay (ctx->xDpy);

    if (ctx->multiInst == True)
    {
        flock (ctx->lckFD, LOCK_UN);
    }

    freeXWCContext (ctx);

    return retVal;
}

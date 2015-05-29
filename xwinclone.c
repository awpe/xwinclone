#include "xwclib.h"

int
main (int     argc,
      char ** argv)
{
    XWCOptions           * prgCfg;
    Display              * xDpy;
    Screen               * xScr;
    Window                 srcWin, rootWin , trgWin;
    XWindowAttributes      srcWinAttr, trgWinAttr, rootWinAttr;
    XSetWindowAttributes   trgWinSetAttr;
    GC                     xGraphicsCtx;
    XVisualInfo            xVisInfo;
    Visual               * xVis;
    Pixmap                 pm, srcWinCompPixmap;
    XEvent                 xEvent;
    char                   exitKeyPressed;

    /*Make a program to be portable to all locales*/
    setlocale (LC_ALL, "");

    if (( xDpy = openDefaultDisplay () )  == NULL)
    {
        return EXIT_FAILURE;
    }

    XSetErrorHandler (errorHandlerBasic);

    if (chkCompExt (xDpy) == False)
    {
        XCloseDisplay (xDpy);
        return EXIT_FAILURE;
    }

    if (( prgCfg = processArgs (xDpy, argc, (const char **) argv) ) == NULL)
    {
        XCloseDisplay (xDpy);
        return EXIT_FAILURE;
    }

    if (( srcWin = getActiveWindow (xDpy, prgCfg) ) == None)
    {
        XCloseDisplay (xDpy);
        free (prgCfg);
        return EXIT_FAILURE;
    }

    XGetWindowAttributes (xDpy, srcWin, &srcWinAttr);

    if (getXErrState () == True)
    {
        XCloseDisplay (xDpy);
        free (prgCfg);
        return EXIT_FAILURE;
    }

    printWindowInfo (xDpy, srcWin, &srcWinAttr);

    if (( xScr = getScreenByWindowAttr (xDpy, &srcWinAttr) ) == None)
    {
        XCloseDisplay (xDpy);
        free (prgCfg);
        return EXIT_FAILURE;
    }

    if (parseColor (xDpy, prgCfg, xScr) == False)
    {
        XCloseDisplay (xDpy);
        free (prgCfg);
        return EXIT_FAILURE;
    }

    if (( rootWin = getRootWinOfScr (xScr) ) == None)
    {
        XCloseDisplay (xDpy);
        free (prgCfg);
        return EXIT_FAILURE;
    }

    XGetWindowAttributes (xDpy, rootWin, &rootWinAttr);

    if (getXErrState () == True)
    {
        XCloseDisplay (xDpy);
        free (prgCfg);
        return EXIT_FAILURE;
    }

    if (grabExitKey (xDpy, rootWin, prgCfg) == False)
    {
        XCloseDisplay (xDpy);
        free (prgCfg);
        return EXIT_FAILURE;
    }

    logCtr ("Creating translation window ... ", LOG_LVL_1);

    if (! XMatchVisualInfo (xDpy, XScreenNumberOfScreen (xScr),
                            srcWinAttr.depth, TrueColor, &xVisInfo))
    {
        logCtr ("Error: no such visual\n", LOG_LVL_NO);
        XCloseDisplay (xDpy);
        free (prgCfg);
        return EXIT_FAILURE;
    }

    /* if verbose_enable==1 ...
        printf ("Matched visual 0x%lx class %d (%s) depth %d\n",
                vinfo.visualid,
                vinfo.class,
                vinfo.class == TrueColor ? "TrueColor" : "unknown",
                vinfo.depth);
     */

    xVis                           = xVisInfo.visual;
    trgWinSetAttr.colormap         = XCreateColormap (xDpy,
                                                      XDefaultRootWindow (xDpy),
                                                      xVis, AllocNone);
    trgWinSetAttr.background_pixel = prgCfg->bgColor.pixel;
    trgWinSetAttr.border_pixel     = 0;
    trgWinSetAttr.bit_gravity      = NorthWestGravity;

    trgWin = XCreateWindow (xDpy, rootWin, 0, 0, srcWinAttr.width,
                            srcWinAttr.height, 0, srcWinAttr.depth,
                            InputOutput, xVis, CWBackPixel | CWColormap
                            | CWBorderPixel | CWBitGravity, &trgWinSetAttr);

    if (getXErrState () == True)
    {
        logCtr ("failed to create window!\n", LOG_LVL_NO);
        if (trgWin != None)
        {
            XDestroyWindow (xDpy, trgWin);
        }
        XCloseDisplay (xDpy);
        free (prgCfg);
        return EXIT_FAILURE;
    }

    if (setWinTitlebar (xDpy, trgWin, WM_CLASS_PRG_NAME_STR) == False)
    {
        XDestroyWindow (xDpy, trgWin);
        XCloseDisplay (xDpy);
        free (prgCfg);
        return EXIT_FAILURE;
    }

    if (setWindowClass (xDpy, trgWin, WM_CLASS_PRG_NAME_STR,
                        WM_CLASS_CLASS_NAME_STR ) == False)
    {
        XDestroyWindow (xDpy, trgWin);
        XCloseDisplay (xDpy);
        free (prgCfg);
        return EXIT_FAILURE;
    }

    //XSelectInput (xDisp, trgWin, ExposureMask);

    XMapWindow (xDpy, trgWin);

    if (getXErrState () == True)
    {
        logCtr ("failed to map window!\n", LOG_LVL_NO);
        XDestroyWindow (xDpy, trgWin);
        XCloseDisplay (xDpy);
        free (prgCfg);
        return EXIT_FAILURE;
    }

    logCtr ("success\n", LOG_LVL_1);

    XSync (xDpy, 0);

    XGetWindowAttributes (xDpy, trgWin, &trgWinAttr);

    if (getXErrState () == True)
    {
        XDestroyWindow (xDpy, trgWin);
        XCloseDisplay (xDpy);
        free (prgCfg);
        return EXIT_FAILURE;
    }

    printWindowInfo (xDpy, trgWin, &trgWinAttr);

    pm = XCreatePixmap (xDpy, rootWin, rootWinAttr.width, rootWinAttr.height,
                        srcWinAttr.depth);

    xGraphicsCtx = XCreateGC (xDpy, pm, 0, NULL);
    XSetForeground (xDpy, xGraphicsCtx, prgCfg->bgColor.pixel);
    //XSetBackground (xDisp, xGraphicsCtx, trgWinBgClr.pixel);
    //XSetFillStyle (xDisp, xGraphicsCtx, FillSolid);

    XCompositeRedirectWindow (xDpy, srcWin, CompositeRedirectAutomatic);
    XCompositeRedirectSubwindows (xDpy, srcWin, CompositeRedirectAutomatic);

    exitKeyPressed = 0;

    srcWinCompPixmap = XCompositeNameWindowPixmap (xDpy, srcWin);

    while (1)
    {
        while (XPending (xDpy))
        {
            XNextEvent (xDpy, &xEvent);
            switch (xEvent.type)
            {
                case KeyPress:
                    /*Now this is redundant check, but it will be useful when 
                     * further event processing arrives*/
                    if (xEvent.xkey.keycode == prgCfg->exitKeyCode
                        && xEvent.xkey.state & prgCfg->exitKeyMask)
                    {
                        logCtr ("Exit key combination catched!\n", LOG_LVL_1);
                        exitKeyPressed = 1;
                        XUngrabKey (xDpy, prgCfg->exitKeyCode,
                                    prgCfg->exitKeyMask, rootWin);
                    }
                    else
                    {
                        /*We don't need this event so send it somewhere else*/
                        XAllowEvents (xDpy, ReplayKeyboard, xEvent.xkey.time);
                        XFlush (xDpy);
                    }
                    break;
                default:
                    break;
            }

            if (exitKeyPressed)
            {
                break;
            }
        }

        if (exitKeyPressed)
        {
            break;
        }

        int trgWinLeftOffset = 0;
        int trgWinTopOffset  = 0;

        XGetWindowAttributes (xDpy, trgWin, &trgWinAttr);

        if (prgCfg->autoCenter == True)
        {
            XGetWindowAttributes (xDpy, srcWin, &srcWinAttr);
            trgWinLeftOffset = (trgWinAttr.width - srcWinAttr.width) / 2;
            trgWinTopOffset  = (trgWinAttr.height - srcWinAttr.height +
                                prgCfg->topOffset) / 2;
        }

        if (getXErrState () == True)
        {
            ungrabExitKey (xDpy, rootWin, prgCfg);
            XCompositeUnredirectWindow (xDpy, srcWin,
                                        CompositeRedirectAutomatic);
            XCompositeUnredirectSubwindows (xDpy, srcWin,
                                            CompositeRedirectAutomatic);
            XFreeGC (xDpy, xGraphicsCtx);
            XFreePixmap (xDpy, srcWinCompPixmap);
            XFreePixmap (xDpy, pm);
            XDestroyWindow (xDpy, trgWin);
            XCloseDisplay (xDpy);
            free (prgCfg);
            return EXIT_FAILURE;
        }

        XFillRectangle (xDpy, pm, xGraphicsCtx, 0, 0, rootWinAttr.width,
                        rootWinAttr.height);

        srcWinCompPixmap = XCompositeNameWindowPixmap (xDpy, srcWin);

        XCopyArea (xDpy, srcWinCompPixmap, pm, xGraphicsCtx,
                   0,                0 + prgCfg->topOffset,
                   srcWinAttr.width, srcWinAttr.height - prgCfg->topOffset,
                   trgWinLeftOffset, trgWinTopOffset);

        XCopyArea (xDpy, pm, trgWin, xGraphicsCtx, 0, 0, trgWinAttr.width,
                   trgWinAttr.height, 0, 0);

        nanosleep (&prgCfg->frameDelay, NULL);
    }

    ungrabExitKey (xDpy, rootWin, prgCfg);
    XCompositeUnredirectWindow (xDpy, srcWin,
                                CompositeRedirectAutomatic);
    XCompositeUnredirectSubwindows (xDpy, srcWin,
                                    CompositeRedirectAutomatic);
    XFreeGC (xDpy, xGraphicsCtx);
    XFreePixmap (xDpy, pm);
    XFreePixmap (xDpy, srcWinCompPixmap);
    XDestroyWindow (xDpy, trgWin);
    XCloseDisplay (xDpy);
    free (prgCfg);

    return EXIT_SUCCESS;
}

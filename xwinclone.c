#include "xwclib.h"

int
main (int     argc,
      char ** argv)
{
    XWCOptions           * prgCfg;
    Display              * xDpy;
    Screen               * xScr;
    Window                 srcWin, rootWin , trgWin;
    XColor                 trgWinBgClr;
    Colormap               xClrMap;
    XWindowAttributes      srcWinAttr, trgWinAttr, rootWinAttr;
    XSetWindowAttributes   trgWinSetAttr;
    int                    srcWinWidth, srcWinHeight, srcWinDepth, trgWinWidth,
        trgWinHeight, rootWinHeight, rootWinWidth;
    GC                     xGraphicsCtx;
    //XEvent                 xEvent;
    XVisualInfo            xVisInfo;
    Visual               * xVis;
    Pixmap                 pm;
    struct timespec        focusDelay, frameDelay;
    /*
        int                 srcWinWidth, srcWinHeight, trgWinWidth, trgWinHeight,
            widthDelta, heightDelta, ptrMode, kbdMode, keycode, x11_fd;
        unsigned int        modifiers;
        Bool                ownerEvents;
        XEvent              xEvent;
        XImage            * xImage, * xImageTmp;
        fd_set              in_fds;
        char                shouldQuit;
     */


    if (( prgCfg = processArgs (argc, (const char **) argv) ) == NULL)
    {
        return EXIT_FAILURE;
    }

    if (( xDpy = openDefaultDisplay () )  == NULL)
    {
        free (prgCfg);
        return EXIT_FAILURE;
    }

    XSetErrorHandler (errorHandlerBasic);

    if (chkCompExt (xDpy) == False)
    {
        XCloseDisplay (xDpy);
        free (prgCfg);
        return EXIT_FAILURE;
    }

    focusDelay.tv_nsec = 0;
    focusDelay.tv_sec  = prgCfg->focusTime;

    frameDelay.tv_nsec = ( 1.0 / prgCfg->frameRate ) * 1000000000L;
    frameDelay.tv_sec  = 0;

    printf ("Selected focustime is %d\n"
            "Selected framerate is %d, this results in %ldns frame delay\n"
            "Selected autocentering mode is %d(bool)\n"
            "Selected top offset is %d(px)\n"
            "Selected bgcolor is %s(#rrggbb)\n\n",
            prgCfg->focusTime, prgCfg->frameRate,
            frameDelay.tv_nsec, prgCfg->autoCenter,
            prgCfg->topOffset, prgCfg->bgColor);

    printf ("Now move focus to window you want to be cloned...\n");
    nanosleep (&focusDelay, NULL);

    if (( srcWin = getActiveWindow (xDpy) ) == None)
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

    srcWinHeight = srcWinAttr.height;
    srcWinWidth  = srcWinAttr.width;
    srcWinDepth  = srcWinAttr.depth;

    if (( xScr = getScreenByWindowAttr (xDpy, &srcWinAttr) ) == None)
    {
        XCloseDisplay (xDpy);
        free (prgCfg);
        return EXIT_FAILURE;
    }

    if (( rootWin = getScrRootWin (xScr) ) == None)
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

    rootWinWidth  = rootWinAttr.width;
    rootWinHeight = rootWinAttr.height;

    if (grabExitKey (xDpy, rootWin) == False)
    {
        XCloseDisplay (xDpy);
        free (prgCfg);
        return EXIT_FAILURE;
    }

    memset (&trgWinBgClr, 0, sizeof (trgWinBgClr ));
    xClrMap = DefaultColormapOfScreen (xScr);

    /*
     * Maybe this color string parsing must take place 
     * while processing options...
     */
    printf ("\nParsing the window background color string \"%s\" ... ",
            prgCfg->bgColor);

    if (!XParseColor (xDpy, xClrMap, prgCfg->bgColor, &trgWinBgClr)
        || !XAllocColor (xDpy, xClrMap, &trgWinBgClr))
    {
        printf ("Error:\n\tXParseColor and/or XAllocColor error\n");
        XCloseDisplay (xDpy);
        free (prgCfg);
        return EXIT_FAILURE;
    }
    printf ("Success\n");

    /* redundant information
    printf ("Color parsing result: pixel=%ld, red=%d, green=%d, blue=%d\n",
            xColor.pixel, xColor.red, xColor.green, xColor.blue);
     */
    printf ("Creating translation window ... ");

    if (!XMatchVisualInfo (xDpy, XScreenNumberOfScreen (xScr), srcWinDepth,
                           TrueColor, &xVisInfo))
    {
        printf ("Error: no such visual\n");
        XCloseDisplay (xDpy);
        free (prgCfg);
        return EXIT_FAILURE;
    }

    /*
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
    trgWinSetAttr.background_pixel = trgWinBgClr.pixel;
    trgWinSetAttr.border_pixel     = 0;
    trgWinSetAttr.bit_gravity      = NorthWestGravity;

    trgWin = XCreateWindow (xDpy, rootWin, 0, 0, srcWinWidth, srcWinHeight, 0,
                            srcWinDepth, InputOutput, xVis, CWBackPixel |
                            CWColormap | CWBorderPixel | CWBitGravity,
                            &trgWinSetAttr);

    if (getXErrState () == True)
    {
        printf ("failed to create window!\n");
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
        printf ("failed to map window!\n");
        XDestroyWindow (xDpy, trgWin);
        XCloseDisplay (xDpy);
        free (prgCfg);
        return EXIT_FAILURE;
    }

    printf ("success\n");

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

    pm = XCreatePixmap (xDpy, rootWin, rootWinWidth, rootWinHeight,
                        srcWinDepth);
    //Pixmap pmDB = XCreatePixmap (xDisp, rootWin, 1920, 1200, srcWinDepth);
    xGraphicsCtx = XCreateGC (xDpy, pm, 0, NULL);

    XSetForeground (xDpy, xGraphicsCtx, trgWinBgClr.pixel);
    //XSetBackground (xDisp, xGraphicsCtx, trgWinBgClr.pixel);
    //XSetFillStyle (xDisp, xGraphicsCtx, FillSolid);

    XCompositeRedirectWindow (xDpy, srcWin, CompositeRedirectAutomatic);
    XCompositeRedirectSubwindows (xDpy, srcWin, CompositeRedirectAutomatic);

    Pixmap pm2;

    while (1)
    {
/*
        while (XPending (xDpy))
        {
            XNextEvent (xDpy, &xEvent);
        }
*/

        XGetWindowAttributes (xDpy, srcWin, &srcWinAttr);

        if (getXErrState () == True)
        {
            XFreePixmap (xDpy, pm);
            XDestroyWindow (xDpy, trgWin);
            XCloseDisplay (xDpy);
            free (prgCfg);
            return EXIT_FAILURE;
        }

        srcWinHeight = srcWinAttr.height;
        srcWinWidth  = srcWinAttr.width;

        XGetWindowAttributes (xDpy, trgWin, &trgWinAttr);

        if (getXErrState () == True)
        {
            XFreePixmap (xDpy, pm);
            XDestroyWindow (xDpy, trgWin);
            XCloseDisplay (xDpy);
            free (prgCfg);
            return EXIT_FAILURE;
        }

        trgWinHeight = trgWinAttr.height;
        trgWinWidth  = trgWinAttr.width;

        XFillRectangle (xDpy, pm, xGraphicsCtx, 0, 0, trgWinWidth,
                        trgWinHeight);

        pm2 = XCompositeNameWindowPixmap (xDpy, srcWin);

        XCopyArea (xDpy, pm2, pm, xGraphicsCtx, 0, 0, trgWinWidth,
                   trgWinHeight, 0, 0);

/*
        XCopyArea (xDpy, srcWin, pm, xGraphicsCtx, 0, 0, trgWinWidth,
                   trgWinHeight, 0, 0);
*/

        XCopyArea (xDpy, pm, trgWin, xGraphicsCtx, 0, 0, trgWinWidth,
                   trgWinHeight, 0, 0);

        nanosleep (&frameDelay, NULL);
    }
    XFreePixmap (xDpy, pm);
    XDestroyWindow (xDpy, trgWin);
    XCloseDisplay (xDpy);
    free (prgCfg);
    return EXIT_SUCCESS;
}

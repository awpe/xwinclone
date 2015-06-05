#include "xwclib.h"

int
main (int     argc,
      char ** argv)
{
    XWCOptions           * cfg;
    Display              * xDpy;
    Screen               * xScr;
    Window                 srcWin, defaultRootWin, rootWinOfSrc, trgWin;
    XWindowAttributes      srcWinAttr, trgWinAttr, rootWinAttr;
    XSetWindowAttributes   trgWinSetAttr;
    GC                     xGraphicsCtx;
    XVisualInfo            xVisInfo;
    Visual               * xVis;
    Pixmap                 pm, srcWinCompPixmap, bgImgPm;
    char                   buf[1024];
    int                    retVal, trgWinLeftOffset, trgWinTopOffset, pressedKey;
    unsigned int           bgImgWidth, bgImgHeight;

    retVal               = EXIT_SUCCESS;
    trgWinLeftOffset     = 0;
    trgWinTopOffset      = 0;
    bgImgPm              = 0;
    bgImgWidth           = 0;
    bgImgHeight          = 0;
    pressedKey           = NO_KEY_PRESSED;

    /*Make a program to be portable to all locales*/
    setlocale (LC_ALL, "");

    printVersion ();

    if (( xDpy = openDefaultDisplay () )  == NULL)
    {
        return EXIT_FAILURE;
    }

    if (chkCompExt (xDpy) == False)
    {
        XCloseDisplay (xDpy);
        return EXIT_FAILURE;
    }

    if ((defaultRootWin = getDefaultRootWindow (xDpy)) == None)
    {
        XCloseDisplay (xDpy);
        return EXIT_FAILURE;
    }

    XGetWindowAttributes (xDpy, defaultRootWin, &rootWinAttr);

    if (getXErrState () == True)
    {
        XCloseDisplay (xDpy);
        return EXIT_FAILURE;
    }

    if (( xScr = getScreenByWindowAttr (xDpy, &rootWinAttr) ) == NULL)
    {
        XCloseDisplay (xDpy);
        return EXIT_FAILURE;
    }

    if (( cfg = processArgs (xDpy, argc, (const char **) argv) ) == NULL)
    {
        XCloseDisplay (xDpy);
        return EXIT_FAILURE;
    }

    if (parseColor (xDpy, cfg, xScr) == False)
    {
        XCloseDisplay (xDpy);
        free (cfg);
        return EXIT_FAILURE;
    }

    if (grabExitKey (xDpy, defaultRootWin, cfg) == False)
    {
        XCloseDisplay (xDpy);
        return EXIT_FAILURE;
    }

    if (grabTranslationCtrlKey (xDpy, defaultRootWin, cfg) == False)
    {
        XCloseDisplay (xDpy);
        return EXIT_FAILURE;
    }

    if (cfg->isSingleton == True && ifSingleInst () == False)
    {
        XCloseDisplay (xDpy);
        free (cfg);
        return EXIT_FAILURE;
    }

    while (pressedKey != EXIT_COMBINATION)
    {
        if (cfg->isDaemon == True)
        {
            snprintf (buf, sizeof (buf), "Move focus to desired window and"
                      " press %s to start translation",
                      cfg->translationCtrlKeyStr);

            logCtr (buf, LOG_LVL_NO, False);

            while ((pressedKey = getPressedComb (xDpy, cfg)) == NO_KEY_PRESSED)
            {
                nanosleep (&cfg->frameDelay, NULL);
            }

            if (pressedKey == EXIT_COMBINATION)
            {
                break;
            }
        }

        if (( srcWin = getActiveWindow (xDpy, cfg) ) == None)
        {
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        XGetWindowAttributes (xDpy, srcWin, &srcWinAttr);

        if (getXErrState () == True)
        {
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        printWindowInfo (xDpy, srcWin, &srcWinAttr);

        if (( xScr = getScreenByWindowAttr (xDpy, &srcWinAttr) ) == NULL)
        {
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        if (parseColor (xDpy, cfg, xScr) == False)
        {
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        if (( rootWinOfSrc = getRootWinOfScr (xScr) ) == None)
        {
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        if (rootWinOfSrc != defaultRootWin)
        {
            ungrabExitKey (xDpy, defaultRootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, defaultRootWin, cfg);

            defaultRootWin = rootWinOfSrc;

            XGetWindowAttributes (xDpy, defaultRootWin, &rootWinAttr);

            if (getXErrState () == True)
            {
                XCloseDisplay (xDpy);
                free (cfg);
                return EXIT_FAILURE;
            }

            if (grabExitKey (xDpy, defaultRootWin, cfg) == False)
            {
                XCloseDisplay (xDpy);
                free (cfg);
                return EXIT_FAILURE;
            }

            if (grabTranslationCtrlKey (xDpy, defaultRootWin, cfg) == False)
            {
                ungrabExitKey (xDpy, defaultRootWin, cfg);
                XCloseDisplay (xDpy);
                free (cfg);
                return EXIT_FAILURE;
            }
        }

        logCtr ("Creating translation window:", LOG_LVL_NO, False);

        if (getVisualOfScr (xScr, srcWinAttr.depth, &xVisInfo) == False)
        {
            ungrabExitKey (xDpy, defaultRootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, defaultRootWin, cfg);
            logCtr ("\tError: no such visual", LOG_LVL_NO, False);
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        xVis                           = xVisInfo.visual;
        trgWinSetAttr.colormap         = XCreateColormap (xDpy, srcWin, xVis, AllocNone);
        trgWinSetAttr.background_pixel = cfg->bgColor.pixel;
        trgWinSetAttr.border_pixel     = 0;
        trgWinSetAttr.bit_gravity      = NorthWestGravity;

        trgWin = XCreateWindow (xDpy, defaultRootWin, 0, 0, srcWinAttr.width,
                                srcWinAttr.height, 0, srcWinAttr.depth,
                                InputOutput, xVis, CWBackPixel | CWColormap
                                | CWBorderPixel | CWBitGravity, &trgWinSetAttr);

        if (getXErrState () == True)
        {
            logCtr ("\tfailed to create window!", LOG_LVL_NO, False);
            ungrabExitKey (xDpy, defaultRootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, defaultRootWin, cfg);
            if (trgWin != None)
            {
                XDestroyWindow (xDpy, trgWin);
                XSync (xDpy, 0);
            }
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        if (setWinTitlebar (xDpy, trgWin, WM_CLASS_PRG_NAME_STR) == False)
        {
            ungrabExitKey (xDpy, defaultRootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, defaultRootWin, cfg);
            XDestroyWindow (xDpy, trgWin);
            XSync (xDpy, 0);
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        if (setWindowClass (xDpy, trgWin, WM_CLASS_PRG_NAME_STR,
                            WM_CLASS_CLASS_NAME_STR ) == False)
        {
            ungrabExitKey (xDpy, defaultRootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, defaultRootWin, cfg);
            XDestroyWindow (xDpy, trgWin);
            XSync (xDpy, 0);
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        XMapWindow (xDpy, trgWin);
        XSync (xDpy, 0);

        if (getXErrState () == True)
        {
            logCtr ("\tfailed to map window!", LOG_LVL_NO, False);
            ungrabExitKey (xDpy, defaultRootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, defaultRootWin, cfg);
            XUnmapWindow (xDpy, trgWin);
            XDestroyWindow (xDpy, trgWin);
            XSync (xDpy, 0);
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        logCtr ("\tsuccess", LOG_LVL_NO, True);

        XGetWindowAttributes (xDpy, trgWin, &trgWinAttr);

        if (getXErrState () == True)
        {
            ungrabExitKey (xDpy, defaultRootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, defaultRootWin, cfg);
            XUnmapWindow (xDpy, trgWin);
            XDestroyWindow (xDpy, trgWin);
            XSync (xDpy, 0);
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        printWindowInfo (xDpy, trgWin, &trgWinAttr);

        if (bgImgPrepare (xDpy, cfg, &bgImgPm, &bgImgWidth, &bgImgHeight,
                          trgWin, &trgWinAttr, &rootWinAttr) == False)
        {
            ungrabExitKey (xDpy, defaultRootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, defaultRootWin, cfg);
            XUnmapWindow (xDpy, trgWin);
            XDestroyWindow (xDpy, trgWin);
            XSync (xDpy, 0);
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        pm = XCreatePixmap (xDpy, defaultRootWin, rootWinAttr.width,
                            rootWinAttr.height, srcWinAttr.depth);
        xGraphicsCtx = XCreateGC (xDpy, pm, 0, NULL);
        XSetForeground (xDpy, xGraphicsCtx, cfg->bgColor.pixel);
        XFillRectangle (xDpy, pm, xGraphicsCtx, 0, 0, rootWinAttr.width,
                        rootWinAttr.height);

        if (cfg->bgImgStatus == True)
        {
            XCopyArea (xDpy, bgImgPm, pm, xGraphicsCtx, 0, 0, bgImgWidth,
                       bgImgHeight, 0, 0);
        }

        XSync (xDpy, 0);

        if (getXErrState () == True)
        {
            ungrabExitKey (xDpy, defaultRootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, defaultRootWin, cfg);
            XFreeGC (xDpy, xGraphicsCtx);
            XFreePixmap (xDpy, pm);
            if (bgImgPm != 0)
            {
                XFreePixmap (xDpy, bgImgPm);
            }
            XUnmapWindow (xDpy, trgWin);
            XDestroyWindow (xDpy, trgWin);
            XSync (xDpy, 0);
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        XCompositeRedirectWindow (xDpy, srcWin, CompositeRedirectAutomatic);
        XCompositeRedirectSubwindows (xDpy, srcWin, CompositeRedirectAutomatic);
        XSync (xDpy, 0);
        srcWinCompPixmap = XCompositeNameWindowPixmap (xDpy, srcWin);

        if (getXErrState () == True)
        {
            ungrabExitKey (xDpy, defaultRootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, defaultRootWin, cfg);
            XCompositeUnredirectWindow (xDpy, srcWin, CompositeRedirectAutomatic);
            XCompositeUnredirectSubwindows (xDpy, srcWin, CompositeRedirectAutomatic);
            XFreeGC (xDpy, xGraphicsCtx);
            XFreePixmap (xDpy, pm);
            XFreePixmap (xDpy, srcWinCompPixmap);
            if (bgImgPm != 0)
            {
                XFreePixmap (xDpy, bgImgPm);
            }
            XUnmapWindow (xDpy, trgWin);
            XDestroyWindow (xDpy, trgWin);
            XSync (xDpy, 0);
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        while ((pressedKey = getPressedComb (xDpy, cfg)) == NO_KEY_PRESSED)
        {
            nanosleep (&cfg->frameDelay, NULL);

            XGetWindowAttributes (xDpy, trgWin, &trgWinAttr);
            if (getXErrState () == True)
            {
                retVal = EXIT_FAILURE;
                break;
            }

            XGetWindowAttributes (xDpy, srcWin, &srcWinAttr);
            if (getXErrState () == True)
            {
                retVal = EXIT_FAILURE;
                break;
            }

            if (cfg->autoCenter == True)
            {
                trgWinLeftOffset = (trgWinAttr.width - srcWinAttr.width) / 2;
                trgWinTopOffset  = (trgWinAttr.height - srcWinAttr.height +
                    cfg->topOffset) / 2;
            }

            if (srcWinAttr.map_state == IsViewable)
            {
                XSync (xDpy, 0);
                XFreePixmap (xDpy, srcWinCompPixmap);
                srcWinCompPixmap = XCompositeNameWindowPixmap (xDpy, srcWin);
                if (getXErrState () == True)
                {
                    retVal = EXIT_FAILURE;
                    break;
                }

                XFillRectangle (xDpy, pm, xGraphicsCtx, 0, 0, rootWinAttr.width,
                                rootWinAttr.height);

                if (cfg->bgImgStatus == True)
                {
                    XCopyArea (xDpy, bgImgPm, pm, xGraphicsCtx, 0, 0, bgImgWidth,
                               bgImgHeight, 0, 0);
                }

                XCopyArea (xDpy, srcWinCompPixmap, pm, xGraphicsCtx,
                           0,                0 + cfg->topOffset,
                           srcWinAttr.width, srcWinAttr.height - cfg->topOffset,
                           trgWinLeftOffset, trgWinTopOffset);

                if (getXErrState () == True)
                {
                    retVal = EXIT_FAILURE;
                    break;
                }
            }

            if (trgWinAttr.map_state == IsViewable)
            {
                XCopyArea (xDpy, pm, trgWin, xGraphicsCtx, 0, 0, trgWinAttr.width, trgWinAttr.height, 0, 0);

                if (getXErrState () == True)
                {
                    retVal = EXIT_FAILURE;
                    break;
                }
            }
        }

        XCompositeUnredirectWindow (xDpy, srcWin, CompositeRedirectAutomatic);
        XCompositeUnredirectSubwindows (xDpy, srcWin, CompositeRedirectAutomatic);
        XFreeGC (xDpy, xGraphicsCtx);
        XFreePixmap (xDpy, pm);
        XFreePixmap (xDpy, srcWinCompPixmap);
        if (bgImgPm != 0)
        {
            XFreePixmap (xDpy, bgImgPm);
        }
        XUnmapWindow (xDpy, trgWin);
        XDestroyWindow (xDpy, trgWin);
        XSync (xDpy, 0);
        if (pressedKey == EXIT_COMBINATION || cfg->isDaemon == 0)
        {
            break;
        }
    }
    ungrabExitKey (xDpy, defaultRootWin, cfg);
    ungrabTranslationCtrlKey (xDpy, defaultRootWin, cfg);
    XCloseDisplay (xDpy);
    free (cfg);

    return retVal;
}

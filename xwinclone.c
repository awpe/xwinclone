#include "xwclib.h"

int
main (int     argc,
      char ** argv)
{
    XWCOptions           * cfg;
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
    char                   transCtrlCombPressed, exitKeyCombPressed;
    int                    retVal, trgWinLeftOffset, trgWinTopOffset;

    retVal               = EXIT_SUCCESS;
    trgWinLeftOffset     = 0;
    trgWinTopOffset      = 0;
    exitKeyCombPressed   = 0;

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

    if (( cfg = processArgs (xDpy, argc, (const char **) argv) ) == NULL)
    {
        XCloseDisplay (xDpy);
        return EXIT_FAILURE;
    }

    while (exitKeyCombPressed == 0)
    {
        transCtrlCombPressed = 0;

        if (cfg->isDaemon == True)
        {
            if ((rootWin = getDefaultRootWindow (xDpy)) == None)
            {
                XCloseDisplay (xDpy);
                free (cfg);
                return EXIT_FAILURE;
            }

            if (grabExitKey (xDpy, rootWin, cfg) == False)
            {
                XCloseDisplay (xDpy);
                free (cfg);
                return EXIT_FAILURE;
            }

            if (grabTranslationCtrlKey (xDpy, rootWin, cfg) == False)
            {
                XCloseDisplay (xDpy);
                free (cfg);
                return EXIT_FAILURE;
            }

            printf ("\nMove focus to desired window and press %s to start"
                    " translation\n", cfg->translationCtrlKeyStr);

            while (transCtrlCombPressed == 0 && exitKeyCombPressed == 0)
            {
                while (XPending (xDpy))
                {
                    XNextEvent (xDpy, &xEvent);
                    switch (xEvent.type)
                    {
                        case KeyPress:
                            if (xEvent.xkey.keycode == cfg->exitKeyCode
                                && (xEvent.xkey.state ^ cfg->exitKeyMask) == 0)
                            {
                                logCtr ("Exit key combination catched"
                                        "!\n", LOG_LVL_NO);
                                exitKeyCombPressed = 1;
                                ungrabExitKey (xDpy, rootWin, cfg);
                            }
                            else  if (xEvent.xkey.keycode == cfg->translationCtrlKeyCode
                                      && (xEvent.xkey.state ^ cfg->translationCtrlKeyMask) == 0)
                            {
                                logCtr ("Grab window key combination catched"
                                        "!\n", LOG_LVL_NO);
                                transCtrlCombPressed = 1;
                                ungrabTranslationCtrlKey (xDpy, rootWin, cfg);
                            }
                            else
                            {
                                XAllowEvents (xDpy, ReplayKeyboard,
                                              xEvent.xkey.time);
                                XFlush (xDpy);
                            }
                            break;
                        default:
                            break;
                    }
                }
                nanosleep (&cfg->frameDelay, NULL);
            }

            if (exitKeyCombPressed == 1)
            {
                break;
            }
        }

        transCtrlCombPressed = 0;

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

        if (( xScr = getScreenByWindowAttr (xDpy, &srcWinAttr) ) == None)
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

        if (( rootWin = getRootWinOfScr (xScr) ) == None)
        {
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        XGetWindowAttributes (xDpy, rootWin, &rootWinAttr);

        if (getXErrState () == True)
        {
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        if (grabExitKey (xDpy, rootWin, cfg) == False)
        {
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        if (grabTranslationCtrlKey (xDpy, rootWin, cfg) == False)
        {
            ungrabExitKey (xDpy, rootWin, cfg);
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        logCtr ("Creating translation window ... ", LOG_LVL_1);

        if (! XMatchVisualInfo (xDpy, XScreenNumberOfScreen (xScr),
                                srcWinAttr.depth, TrueColor, &xVisInfo))
        {
            ungrabExitKey (xDpy, rootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, rootWin, cfg);
            logCtr ("Error: no such visual\n", LOG_LVL_NO);
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        xVis                           = xVisInfo.visual;
        trgWinSetAttr.colormap         = XCreateColormap (xDpy,
                                                          XDefaultRootWindow (xDpy),
                                                          xVis, AllocNone);
        trgWinSetAttr.background_pixel = cfg->bgColor.pixel;
        trgWinSetAttr.border_pixel     = 0;
        trgWinSetAttr.bit_gravity      = NorthWestGravity;

        trgWin = XCreateWindow (xDpy, rootWin, 0, 0, srcWinAttr.width,
                                srcWinAttr.height, 0, srcWinAttr.depth,
                                InputOutput, xVis, CWBackPixel | CWColormap
                                | CWBorderPixel | CWBitGravity, &trgWinSetAttr);

        if (getXErrState () == True)
        {
            logCtr ("failed to create window!\n", LOG_LVL_NO);
            ungrabExitKey (xDpy, rootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, rootWin, cfg);
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
            ungrabExitKey (xDpy, rootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, rootWin, cfg);
            XDestroyWindow (xDpy, trgWin);
            XSync (xDpy, 0);
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        if (setWindowClass (xDpy, trgWin, WM_CLASS_PRG_NAME_STR,
                            WM_CLASS_CLASS_NAME_STR ) == False)
        {
            ungrabExitKey (xDpy, rootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, rootWin, cfg);
            XDestroyWindow (xDpy, trgWin);
            XSync (xDpy, 0);
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        XMapWindow (xDpy, trgWin);

        if (getXErrState () == True)
        {
            logCtr ("failed to map window!\n", LOG_LVL_NO);
            ungrabExitKey (xDpy, rootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, rootWin, cfg);
            XUnmapWindow (xDpy, trgWin);
            XDestroyWindow (xDpy, trgWin);
            XSync (xDpy, 0);
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        logCtr ("success\n", LOG_LVL_1);

        XSync (xDpy, 0);

        XGetWindowAttributes (xDpy, trgWin, &trgWinAttr);

        if (getXErrState () == True)
        {
            ungrabExitKey (xDpy, rootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, rootWin, cfg);
            XUnmapWindow (xDpy, trgWin);
            XDestroyWindow (xDpy, trgWin);
            XSync (xDpy, 0);
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        printWindowInfo (xDpy, trgWin, &trgWinAttr);

        pm = XCreatePixmap (xDpy, rootWin, rootWinAttr.width, rootWinAttr.height,
                            srcWinAttr.depth);
        xGraphicsCtx = XCreateGC (xDpy, pm, 0, NULL);
        XSetForeground (xDpy, xGraphicsCtx, cfg->bgColor.pixel);
        XFillRectangle (xDpy, pm, xGraphicsCtx, 0, 0, rootWinAttr.width,
                        rootWinAttr.height);

        if (getXErrState () == True)
        {
            ungrabExitKey (xDpy, rootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, rootWin, cfg);
            XFreeGC (xDpy, xGraphicsCtx);
            XFreePixmap (xDpy, pm);
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
            ungrabExitKey (xDpy, rootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, rootWin, cfg);
            XCompositeUnredirectWindow (xDpy, srcWin,
                                        CompositeRedirectAutomatic);
            XCompositeUnredirectSubwindows (xDpy, srcWin,
                                            CompositeRedirectAutomatic);
            XFreeGC (xDpy, xGraphicsCtx);
            XFreePixmap (xDpy, pm);
            XFreePixmap (xDpy, srcWinCompPixmap);
            XUnmapWindow (xDpy, trgWin);
            XDestroyWindow (xDpy, trgWin);
            XSync (xDpy, 0);
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        while (transCtrlCombPressed == 0 && exitKeyCombPressed == 0)
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
                srcWinCompPixmap = XCompositeNameWindowPixmap (xDpy, srcWin);
                if (getXErrState () == True)
                {
                    retVal = EXIT_FAILURE;
                    break;
                }

                XFillRectangle (xDpy, pm, xGraphicsCtx, 0, 0, rootWinAttr.width,
                                rootWinAttr.height);

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
                XCopyArea (xDpy, pm, trgWin, xGraphicsCtx, 0, 0, trgWinAttr.width,
                           trgWinAttr.height, 0, 0);

                if (getXErrState () == True)
                {
                    retVal = EXIT_FAILURE;
                    break;
                }
            }

            while (XPending (xDpy) && transCtrlCombPressed == 0 && exitKeyCombPressed == 0)
            {
                XNextEvent (xDpy, &xEvent);
                switch (xEvent.type)
                {
                    case KeyPress:
                        if (xEvent.xkey.keycode == cfg->exitKeyCode
                            && (xEvent.xkey.state ^ cfg->exitKeyMask) == 0)
                        {
                            logCtr ("Exit key combination catched"
                                    "!\n", LOG_LVL_NO);
                            exitKeyCombPressed = 1;
                            ungrabExitKey (xDpy, rootWin, cfg);
                        }
                        else  if (xEvent.xkey.keycode == cfg->translationCtrlKeyCode
                                  && (xEvent.xkey.state ^ cfg->translationCtrlKeyMask) == 0)
                        {
                            logCtr ("Grab window key combination catched"
                                    "!\n", LOG_LVL_NO);
                            transCtrlCombPressed = 1;
                            ungrabTranslationCtrlKey (xDpy, rootWin, cfg);
                        }
                        else
                        {
                            XAllowEvents (xDpy, ReplayKeyboard,
                                          xEvent.xkey.time);
                            XFlush (xDpy);
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        ungrabExitKey (xDpy, rootWin, cfg);
        ungrabTranslationCtrlKey (xDpy, rootWin, cfg);
        XCompositeUnredirectWindow (xDpy, srcWin,
                                    CompositeRedirectAutomatic);
        XCompositeUnredirectSubwindows (xDpy, srcWin,
                                        CompositeRedirectAutomatic);
        XFreeGC (xDpy, xGraphicsCtx);
        XFreePixmap (xDpy, pm);
        XFreePixmap (xDpy, srcWinCompPixmap);
        XUnmapWindow (xDpy, trgWin);
        XDestroyWindow (xDpy, trgWin);
        XSync (xDpy, 0);
        if (exitKeyCombPressed == 1)
        {
            break;
        }
    }
    XCloseDisplay (xDpy);
    free (cfg);

    return retVal;
}

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
    Pixmap                 pm, srcWinCompPixmap, bgImgPm;
    XEvent                 xEvent;
    Imlib_Image            imgSrc, imgScaled;
    char                   buf[1024], transCtrlCombPressed, exitKeyCombPressed;
    int                    retVal, trgWinLeftOffset, trgWinTopOffset;
    unsigned int           bgImgWidth, bgImgHeight;

    retVal               = EXIT_SUCCESS;
    trgWinLeftOffset     = 0;
    trgWinTopOffset      = 0;
    exitKeyCombPressed   = 0;
    bgImgPm              = 0;
    bgImgWidth           = 0;
    bgImgHeight          = 0;
    imgScaled            = NULL;


    /*Make a program to be portable to all locales*/
    setlocale (LC_ALL, "");

    printVersion ();

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

    if (cfg->isSingleton == True && ifSingleInst () == False)
    {
        XCloseDisplay (xDpy);
        free (cfg);
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

            snprintf (buf, sizeof (buf), "Move focus to desired window and"
                      " press %s to start translation",
                      cfg->translationCtrlKeyStr);

            logCtr (buf, LOG_LVL_NO, False);

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
                                logCtr ("Exit key sequence received!",
                                        LOG_LVL_NO, False);
                                exitKeyCombPressed = 1;
                                ungrabExitKey (xDpy, rootWin, cfg);
                            }
                            else if (xEvent.xkey.keycode
                                     == cfg->translationCtrlKeyCode

                                     && (xEvent.xkey.state
                                         ^ cfg->translationCtrlKeyMask) == 0

                                     && cfg->isDaemon == True)
                            {
                                logCtr ("Grab window key sequence received!",
                                        LOG_LVL_NO, False);
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

        logCtr ("Creating translation window:", LOG_LVL_NO, False);

        if (! XMatchVisualInfo (xDpy, XScreenNumberOfScreen (xScr),
                                srcWinAttr.depth, TrueColor, &xVisInfo))
        {
            ungrabExitKey (xDpy, rootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, rootWin, cfg);
            logCtr ("\tError: no such visual", LOG_LVL_NO, False);
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        xVis                           = xVisInfo.visual;
        trgWinSetAttr.colormap         = XCreateColormap (xDpy,
                                                          rootWin,
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
            logCtr ("\tfailed to create window!", LOG_LVL_NO, False);
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
        XSync (xDpy, 0);

        if (getXErrState () == True)
        {
            logCtr ("\tfailed to map window!", LOG_LVL_NO, False);
            ungrabExitKey (xDpy, rootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, rootWin, cfg);
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
            ungrabExitKey (xDpy, rootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, rootWin, cfg);
            XUnmapWindow (xDpy, trgWin);
            XDestroyWindow (xDpy, trgWin);
            XSync (xDpy, 0);
            XCloseDisplay (xDpy);
            free (cfg);
            return EXIT_FAILURE;
        }

        if (cfg->bgImgFileSet == True)
        {
            logCtr ("Reading background image file:", LOG_LVL_NO, False);
        }
        else
        {
            logCtr ("Reading background image file:", LOG_LVL_1, False);
        }

        imgSrc = imlib_load_image (cfg->bgImgFileStr);

        if (imgSrc == NULL)
        {
            snprintf (buf, sizeof (buf), "\tcannot load background image file"
                      " '%s'!", cfg->bgImgFileStr);

            if (cfg->bgImgFileSet == True)
            {
                logCtr (buf, LOG_LVL_NO, True);

                ungrabExitKey (xDpy, rootWin, cfg);
                ungrabTranslationCtrlKey (xDpy, rootWin, cfg);
                XUnmapWindow (xDpy, trgWin);
                XDestroyWindow (xDpy, trgWin);
                XSync (xDpy, 0);
                XCloseDisplay (xDpy);
                free (cfg);
                return EXIT_FAILURE;
            }
            else
            {
                logCtr (buf, LOG_LVL_1, True);
            }
        }
        else
        {
            imlib_context_set_image (imgSrc);
            bgImgWidth  = imlib_image_get_width ();
            bgImgHeight = imlib_image_get_height ();

            if (   bgImgWidth  > rootWinAttr.width
                || bgImgHeight > rootWinAttr.height)
            {
                float scaleFactor = (float) bgImgWidth / (float) bgImgHeight;

                int newWidth  = rootWinAttr.width;
                int newHeight = (float) newWidth / scaleFactor;

                snprintf (buf, sizeof (buf), "Image scaled to:\n\twidth:\t%d\n\theight:\t%d", newWidth, newHeight);
                logCtr (buf, LOG_LVL_1, True);

                imgScaled = imlib_create_cropped_scaled_image (0, 0, bgImgWidth, bgImgHeight, newWidth, newHeight);
                imlib_free_image_and_decache ();
                imlib_context_set_image (imgScaled);
                bgImgWidth  = imlib_image_get_width ();
                bgImgHeight = imlib_image_get_height ();
            }
            else
            {
                imgScaled = imgSrc;
            }


            bgImgPm = XCreatePixmap (xDpy, rootWin, bgImgWidth, bgImgHeight,
                                     srcWinAttr.depth);

            imlib_context_set_display (xDpy);
            imlib_context_set_visual (xVis);
            imlib_context_set_colormap (trgWinSetAttr.colormap);
            imlib_context_set_drawable (bgImgPm);

            imlib_render_image_on_drawable (0, 0);

            XSync (xDpy, 0);

            imlib_free_image_and_decache ();

            cfg->bgImgStatus = True;

            if (cfg->bgImgFileSet == True)
            {
                logCtr ("\tsuccess", LOG_LVL_NO, True);
            }
            else
            {
                logCtr ("\tsuccess", LOG_LVL_1, True);
            }

            //            Window root_return;
            //            int x_return, y_return;
            //            unsigned int width_return, height_return;
            //            unsigned int border_width_return;
            //            unsigned int depth_return;
            //
            //            Status st = XGetGeometry (xDpy, bgImgPm, &root_return, &x_return, &y_return, &width_return,
            //                                      &height_return, &border_width_return, &depth_return);
            //
            //            printf ("\nget geometry status = %d\n", st);
            //            printf ("\nroot win = %lX\n", rootWin);
            //            printf ("\nsrc win = %lX\n", srcWin);
            //            printf ("\ntrg win = %lX\n", trgWin);
            //
            //            snprintf (buf, sizeof (buf), "Background image parameters:\n\tWidth:\t"
            //                      "%d\n\tHeight:\t%d\n\tDepth:\t%u\n\troot win:\t%lX",
            //                      bgImgWidth, bgImgHeight, depth_return, root_return);
            //
            //            logCtr (buf, LOG_LVL_1, False);

        }
        printWindowInfo (xDpy, trgWin, &trgWinAttr);

        pm = XCreatePixmap (xDpy, rootWin, rootWinAttr.width,
                            rootWinAttr.height, srcWinAttr.depth);
        xGraphicsCtx = XCreateGC (xDpy, pm, 0, NULL);
        XSetForeground (xDpy, xGraphicsCtx, cfg->bgColor.pixel);
        XFillRectangle (xDpy, pm, xGraphicsCtx, 0, 0, rootWinAttr.width,
                        rootWinAttr.height);

        //        Window root_return;
        //        int x_return, y_return;
        //        unsigned int width_return, height_return;
        //        unsigned int border_width_return;
        //        unsigned int depth_return;
        //
        //        Status st = XGetGeometry (xDpy, pm, &root_return, &x_return, &y_return, &width_return,
        //                                  &height_return, &border_width_return, &depth_return);
        //
        //        printf ("\nget geometry status = %d\n", st);
        //        printf ("\nroot win = %lX\n", rootWin);
        //        printf ("\nsrc win = %lX\n", srcWin);
        //        printf ("\ntrg win = %lX\n", trgWin);
        //
        //        snprintf (buf, sizeof (buf), "pm parameters:\n\tWidth:\t"
        //                  "%d\n\tHeight:\t%d\n\tHotspot X:\t%d\n\tHotspot Y:\t%d\n\tDepth:\t%u\n\troot win:\t%lX",
        //                  width_return, height_return, x_return, y_return, depth_return, root_return);
        //
        //        logCtr (buf, LOG_LVL_1, False);
        //
        //        printf ("\nbg img copying\n");

        if (cfg->bgImgStatus == True)
        {
            XCopyArea (xDpy, bgImgPm, pm, xGraphicsCtx, 0, 0, bgImgWidth,
                       bgImgHeight, 0, 0);
        }

        XSync (xDpy, 0);

        if (getXErrState () == True)
        {
            ungrabExitKey (xDpy, rootWin, cfg);
            ungrabTranslationCtrlKey (xDpy, rootWin, cfg);
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

        //        st = XGetGeometry (xDpy, srcWinCompPixmap, &root_return, &x_return, &y_return, &width_return,
        //                           &height_return, &border_width_return, &depth_return);
        //
        //        printf ("\nget geometry status = %d\n", st);
        //        printf ("\nroot win = %lX\n", rootWin);
        //        printf ("\nsrc win = %lX\n", srcWin);
        //        printf ("\ntrg win = %lX\n", trgWin);
        //
        //        snprintf (buf, sizeof (buf), "srcWinCompPixmap parameters:\n\tWidth:\t"
        //                  "%d\n\tHeight:\t%d\n\tHotspot X:\t%d\n\tHotspot Y:\t%d\n\tDepth:\t%u\n\troot win:\t%lX",
        //                  width_return, height_return, x_return, y_return, depth_return, root_return);
        //
        //        logCtr (buf, LOG_LVL_1, False);

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
                XCopyArea (xDpy, pm, trgWin, xGraphicsCtx, 0, 0,
                           trgWinAttr.width, trgWinAttr.height, 0, 0);

                if (getXErrState () == True)
                {
                    retVal = EXIT_FAILURE;
                    break;
                }
            }

            while (   XPending (xDpy)      != 0
                   && transCtrlCombPressed == 0
                   && exitKeyCombPressed   == 0)
            {
                XNextEvent (xDpy, &xEvent);
                switch (xEvent.type)
                {
                    case KeyPress:
                        if (xEvent.xkey.keycode == cfg->exitKeyCode
                            && (xEvent.xkey.state ^ cfg->exitKeyMask) == 0)
                        {
                            logCtr ("Exit key sequence received!", LOG_LVL_NO,
                                    False);
                            exitKeyCombPressed = 1;
                            ungrabExitKey (xDpy, rootWin, cfg);
                        }
                        else if (xEvent.xkey.keycode
                                 == cfg->translationCtrlKeyCode

                                 && (xEvent.xkey.state ^
                                     cfg->translationCtrlKeyMask) == 0

                                 && cfg->isDaemon == True)
                        {
                            logCtr ("Grab window key sequence received!",
                                    LOG_LVL_NO, False);
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
        if (bgImgPm != 0)
        {
            XFreePixmap (xDpy, bgImgPm);
        }
        XUnmapWindow (xDpy, trgWin);
        XDestroyWindow (xDpy, trgWin);
        XSync (xDpy, 0);
        if (exitKeyCombPressed == 1 || cfg->isDaemon == 0)
        {
            break;
        }
    }
    XCloseDisplay (xDpy);
    free (cfg);

    return retVal;
}

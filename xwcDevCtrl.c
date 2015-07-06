#include "xwc.h"

Bool
chckXI2Ext (XWCContext * ctx)
{
    int event, error, major, minor, rc;
    char buf[1024];

    logCtr ("Checking XInput 2 extension:", LOG_LVL_1, False);

    if (ctx == NULL)
    {
        logCtr ("Cannot check XInput 2 extension: NULL pointer to XWC context"
                " received.", LOG_LVL_NO, False);
        return False;
    }
    //GetReqSized();
    if (XQueryExtension (ctx->xDpy, "XInputExtension", &ctx->xiOp, &event,
                         &error) == 0)
    {
        logCtr ("X Input extension not available.", LOG_LVL_NO, False);
        return False;
    }

    major = 2;
    minor = 9;

    if ((rc = XIQueryVersion (ctx->xDpy, &major, &minor)) == BadRequest)
    {
        snprintf (buf, sizeof (buf), "XI2 not available. Server supports %d.%d",
                  major, minor);
        logCtr (buf,  LOG_LVL_NO, False);
        return False;
    }
    else if (rc != Success)
    {
        logCtr ("Xlib internal error!", LOG_LVL_NO, False);
        return False;
    }
    else if ((major * 1000 + minor) < 2002)
    {
        snprintf (buf, sizeof (buf), "Available XI2 extension version (%d.%d)"
                  " is not supported.", major, minor);
        logCtr (buf,  LOG_LVL_NO, False);
        return False;
    }

    logCtr ("\tSuccess", LOG_LVL_1, True);

    snprintf (buf, sizeof (buf), "\tXI2 extension version is (%d.%d)", major,
              minor);
    logCtr (buf,  LOG_LVL_2, True);

    return True;
}

Bool
grabKeyCtrl (XWCContext      * ctx,
             Window            w,
             KeyCode           xKCode,
             int               nMods,
             XIGrabModifiers * mods,
             Bool              grab)
{
    XIGrabModifiers * failMods;
    XIEventMask       evmask;
    int               nfailMods, i;
    unsigned char     mask[(XI_LASTEVENT + 7) / 8];
    char              buf[1024];

    snprintf (buf, sizeof (buf), "\tGrabbing keycode %d on window 0x%lX:",
              xKCode, w);
    logCtr (buf, LOG_LVL_1, False);

    if (ctx == NULL)
    {
        logCtr ("Cannot grab key: NULL pointer to XWC context received.",
                LOG_LVL_NO, False);
        return False;
    }

    if (ctx->kbds == NULL)
    {
        logCtr ("Cannot grab key: NULL pointer to keyboard device list"
                " received.", LOG_LVL_NO, False);
        return False;
    }

    if (ctx->kbds->nDevs < 0 || ctx->kbds->nDevs > 127)
    {
        logCtr ("Cannot grab key: bad device count.", LOG_LVL_NO, False);
        return False;
    }

    if (ctx->kbds->devs == NULL)
    {
        logCtr ("Cannot grab key: NULL pointer to device array received.",
                LOG_LVL_NO, False);
        return False;
    }

    if (nMods < 0 || nMods > 127)
    {
        logCtr ("Cannot grab key: bad mods count.", LOG_LVL_NO, False);
        return False;
    }



    if (mods == NULL)
    {
        logCtr ("Cannot grab key: NULL pointer to modifiers array received.",
                LOG_LVL_NO, False);
        return False;
    }

    if (w == None)
    {
        logCtr ("Cannot grab key: No window specified.", LOG_LVL_NO, False);
        return False;
    }

    if (grab == False)
    {
        for (int i = 0; i < ctx->kbds->nDevs; ++ i)
        {
            XIUngrabKeycode (ctx->xDpy, ctx->kbds->devs[i], xKCode, w, nMods,
                             mods);

            snprintf (buf, sizeof (buf), "\t\tkeycode ungrabbed on device %d.",
                      ctx->kbds->devs[i]);
            logCtr (buf, LOG_LVL_2, True);
        }
        return True;
    }

    failMods = (XIGrabModifiers*) malloc (sizeof (XIGrabModifiers) * nMods);

    if (failMods == NULL)
    {
        logCtr ("Cannot grab key: cannot allocate array for failed mods.",
                LOG_LVL_NO, False);
        return False;
    }

    memcpy (failMods, mods, sizeof (XIGrabModifiers) * nMods);

    memset (mask, 0, sizeof (mask));
    XISetMask (mask, XI_KeyRelease);
    XISetMask (mask, XI_KeyPress);

    memset (&evmask, 0, sizeof (evmask));
    evmask.mask_len = sizeof (mask);
    evmask.mask     = mask;

    nfailMods       = 0;

    for (i = 0; i < ctx->kbds->nDevs && nfailMods == 0; ++ i)
    {
        nfailMods = XIGrabKeycode (ctx->xDpy, ctx->kbds->devs[i], xKCode, w,
                                   GrabModeAsync, GrabModeAsync, False, &evmask,
                                   nMods, failMods);
        if (nfailMods == 0)
        {
            snprintf (buf, sizeof (buf), "\t\tkeycode grabbed on device %d.",
                      ctx->kbds->devs[i]);
            logCtr (buf, LOG_LVL_2, True);
        }
    }

    if (nfailMods != 0)
    {
        for (i = 0; i < nfailMods; ++ i)
        {
            snprintf (buf, sizeof (buf), "Modifier %x failed with error %d\n",
                      failMods[i].modifiers, failMods[i].status);
            logCtr (buf, LOG_LVL_NO, False);
        }

        free (failMods);

        return False;
    }

    snprintf (buf, sizeof (buf), "\t\tSuccess");
    logCtr (buf, LOG_LVL_2, True);

    free (failMods);

    return True;
}

Bool
getInputDevices (XWCContext * ctx)
{
    XIDeviceInfo * allDevsInfo, * dev;
    char           buf[1024];
    int            i, * kbdIds, masterKbdCnt, nAllDevs;

    if (ctx == NULL)
    {
        logCtr ("Cannot get list of all devices: NULL pointer to program"
                " context received!", LOG_LVL_NO, False);
        return False;
    }

    nAllDevs    = 0;
    kbdIds      = NULL;
    allDevsInfo = XIQueryDevice (ctx->xDpy, XIAllDevices, &nAllDevs);

    if (allDevsInfo != NULL)
    {
        kbdIds = (int*) malloc (sizeof (int) * nAllDevs);

        if (kbdIds == NULL)
        {
            logCtr ("Cannot get list of all devices: cannot allocate memory"
                    " for device's id array!", LOG_LVL_NO, False);
            XIFreeDeviceInfo (allDevsInfo);
            return False;
        }

        masterKbdCnt = 0;

        for (i = 0; i < nAllDevs; ++ i)
        {
            dev = & allDevsInfo[i];

            if (dev->use == XIMasterKeyboard)
            {
                kbdIds[masterKbdCnt] = dev->deviceid;
                ++ masterKbdCnt;
            }

            if (dev->use == XISlavePointer)
            {
                if (strncmp (dev->name, ctx->ptrDevName,
                             MAX_POINTER_DEVICE_NAME_LENGTH) == STR_EQUAL)
                {
                    ctx->slavePtrDevId  = dev->deviceid;
                    ctx->masterPtrDevId = dev->attachment;
                }
            }

        }

        XIFreeDeviceInfo (allDevsInfo);

        if (ctx->slavePtrDevId == NO_DEVICE)
        {
            snprintf (buf, sizeof (buf), "Cannot get list of all devices: no "
                      "slave pointer with name('%s') found, see '$ xinput list'"
                      " to find propriate pointer name!", ctx->ptrDevName);
            logCtr (buf, LOG_LVL_NO, False);
            free (kbdIds);
            return False;
        }

        ctx->kbds = (DevList *) malloc (sizeof (DevList));

        if (ctx->kbds == NULL)
        {
            logCtr ("Cannot get list of all devices: Cannot allocate memory"
                    " for device list!", LOG_LVL_NO, False);
            free (kbdIds);
            return False;
        }

        ctx->kbds->devs  = kbdIds;
        ctx->kbds->nDevs = masterKbdCnt;
    }
    else
    {
        logCtr ("Cannot get list of all devices: XIQueryDevice error!",
                LOG_LVL_NO, False);
        return False;
    }

    return True;
}

Bool
grabAllKeys (XWCContext * ctx)
{
    size_t sizeTmp;

    logCtr ("Trying to grab all control keys:", LOG_LVL_1, False);

    /**************************************************************************/
    /*prepare modifiers*/
    /**************************************************************************/
    ctx->nMods = 4;

    sizeTmp = sizeof (XIGrabModifiers);

    ctx->clMods   = (XIGrabModifiers *) malloc (sizeTmp * ctx->nMods);

    if (ctx->clMods == NULL)
    {
        logCtr ("Error allocating memory for XIGrabModifiers struct!",
                LOG_LVL_NO, False);
        return False;
    }

    ctx->exitMods = (XIGrabModifiers *) malloc (sizeTmp * ctx->nMods);

    if (ctx->exitMods == NULL)
    {
        logCtr ("Error allocating memory for XIGrabModifiers struct!",
                LOG_LVL_NO, False);
        return False;
    }

    ctx->clMods[0].modifiers = ctx->cloneKeyMask;
    ctx->clMods[1].modifiers = ctx->cloneKeyMask | LockMask;
    ctx->clMods[2].modifiers = ctx->cloneKeyMask | Mod2Mask;
    ctx->clMods[3].modifiers = ctx->cloneKeyMask | Mod2Mask  | LockMask;

    ctx->exitMods[0].modifiers = ctx->exitKeyMask;
    ctx->exitMods[1].modifiers = ctx->exitKeyMask | LockMask;
    ctx->exitMods[2].modifiers = ctx->exitKeyMask | Mod2Mask;
    ctx->exitMods[3].modifiers = ctx->exitKeyMask | Mod2Mask  | LockMask;
    /**************************************************************************/


    /**************************************************************************/
    /*Grab exit key*/
    /**************************************************************************/
    if (grabKeyCtrl (ctx, ctx->rootW, ctx->exitKeyCode, ctx->nMods,
                     ctx->exitMods, True)
        == False)
    {
        logCtr ("Error grabbing exit key!", LOG_LVL_NO, False);
        return False;
    }
    /**************************************************************************/


    /**************************************************************************/
    /*Grab translation control key*/
    /**************************************************************************/
    if (grabKeyCtrl (ctx, ctx->rootW, ctx->cloneKeyCode, ctx->nMods,
                     ctx->clMods, True)
        == False)
    {
        logCtr ("Error grabbing clone key!", LOG_LVL_NO, False);
        grabKeyCtrl (ctx, ctx->rootW, ctx->exitKeyCode, ctx->nMods,
                     ctx->exitMods, False);
        return False;
    }
    /**************************************************************************/

    logCtr ("\tsuccess", LOG_LVL_1, False);

    return True;
}

void
ungrabAllKeys (XWCContext * ctx)
{
    if (ctx->devsAcquired)
    {
        grabKeyCtrl (ctx, ctx->rootW, ctx->exitKeyCode, ctx->nMods,
                     ctx->exitMods, False);
        grabKeyCtrl (ctx, ctx->rootW, ctx->cloneKeyCode, ctx->nMods,
                     ctx->clMods, False);
    }
}

Bool
mvPtrWRel (XWCContext * ctx, Window window, int x, int y)
{
    int ret;

    ret = 0;

    ret = XIWarpPointer ( ctx->xDpy, ctx->masterPtrDevId, None, window,
                         0, 0, 0, 0, x, y);

    XFlush (ctx->xDpy);

    return ret == 0 ? True : False;
}

int
getMouseLoc (XWCContext * ctx,
             int        * xR,
             int        * yR,
             int        * scrNumR,
             Window     * wR)
{
    int          ret, x, y, screen_num, dummy_int;
    Window       window;
    Window       root;
    unsigned int dummy_uint;
    //char         buf[1024];

    ret         = False;
    window      = None;
    root        = None;

    ret = XQueryPointer (ctx->xDpy, ctx->rootW, &root, &window, &x, &y,
                         &dummy_int, &dummy_int, &dummy_uint);

    if (ret == True)
    {
        screen_num = XScreenNumberOfScreen (ctx->xScr);
    }
    else
    {
        logCtr ("Cannot get mouse location", LOG_LVL_NO, False);
        return False;
    }

    if (wR != NULL)
    {
        /* Find the client window if we are not root. */
        if (window != root && window != 0)
        {
            int findret;
            Window client = 0;

            /* Search up the stack for a client window for this window */
            findret = findWClient (ctx, window, &client, FIND_PARENTS);
            if (findret == False)
            {
                /* If no client found, search down the stack */
                findret = findWClient (ctx, window, &client, FIND_CHILDREN);
            }

            //fprintf (stderr, "%lX, %lX, %lX, %X\n", window, root, client,
            //findret);

            if (findret == True)
            {
                window = client;
            }
        }
        else
        {
            window = root;
        }
    }

    //printf ("mouseloc root: %lX\n", root);
    //printf ("mouseloc window: %lX\n", window);

    if (ret == True)
    {
        if (xR != NULL)
        {
            *xR = x;
        }
        if (yR != NULL)
        {
            *yR = y;
        }
        if (scrNumR != NULL)
        {
            *scrNumR = screen_num;
        }
        if (wR != NULL)
        {
            *wR = window;
        }
    }

    return ret;
}

unsigned int
getInSt (XWCContext * ctx)
{
    Window       dummy;
    int          rX, rY, wX, wY;
    unsigned int mask;

    XQueryPointer (ctx->xDpy, ctx->rootW, &dummy, &dummy, &rX, &rY, &wX, &wY,
                   &mask);

    return mask;
}

int
mouseBtnCtrl (XWCContext * ctx, int button, int is_press)
{
    int          res;
    int          screen;
    XButtonEvent xbpe;
    Window       z;

    memset (&xbpe, 0, sizeof (xbpe));

    z = ctx->srcW;

    getMouseLoc (ctx, &xbpe.x_root, &xbpe.y_root, &screen, &z);

    //printf ("\tmouse is in %d, %d relative to root\n", xbpe.x_root,
    //xbpe.y_root);

    xbpe.window      = ctx->srcW;
    xbpe.button      = button;
    xbpe.display     = ctx->xDpy;
    xbpe.root        = ctx->rootW;
    xbpe.same_screen = True;
    xbpe.state       = getInSt (ctx);
    xbpe.subwindow   = None;
    xbpe.time        = CurrentTime;
    xbpe.type        = (is_press ? ButtonPress : ButtonRelease);

    XTranslateCoordinates (ctx->xDpy, ctx->rootW, ctx->srcW, xbpe.x_root,
                           xbpe.y_root, &xbpe.x, &xbpe.y, &xbpe.subwindow);

    //printf ("\t1relative to source it is %d, %d\n", xbpe.x, xbpe.y);

    //printf ("\t1src is %lX, subw is %lX\n", ctx->srcW, xbpe.subwindow);

    if (xbpe.subwindow != None)
    {
        XTranslateCoordinates (ctx->xDpy, ctx->rootW, xbpe.subwindow,
                               xbpe.x_root, xbpe.y_root, &xbpe.x, &xbpe.y,
                               &xbpe.subwindow);

        //printf ("\t1relative to source it is %d, %d\n", xbpe.x, xbpe.y);

        //printf ("\t1src is %lX, subw is %lX\n", ctx->srcW, xbpe.subwindow);
    }

    if (is_press == False)
    {
        xbpe.state |= Button1MotionMask;
    }

    res = XSendEvent (ctx->xDpy, ctx->srcW, True, ButtonPressMask,
                      (XEvent *) & xbpe);
    XFlush (ctx->xDpy);

    if (res == 0 || getXErrState () == True)
    {
        logCtr ("Error sending click event", LOG_LVL_NO, False);
        return False;
    }

    return True;
}

int
setMouseUp (XWCContext * ctx,
            int          button)
{
    return mouseBtnCtrl (ctx, button, False);
}

int
setMouseDown (XWCContext * ctx,
              int          button)
{
    return mouseBtnCtrl (ctx, button, True);
}

int
clickW (XWCContext * ctx,
        int          button)
{
    int ret;

    ret = setMouseDown (ctx, button);

    if (ret != True)
    {
        logCtr ("Click failed!", LOG_LVL_NO, False);
        return ret;
    }

    //nanosleep (&ctx->clickDelay, NULL);

    //ret = setMouseUp (ctx, button);

    return ret;
}

int
getPressedComb (XWCContext * ctx)
{
    XEvent               xEvent;
    //Bool                  res;
    //Window                rRetW, chRetW;
    XGenericEventCookie * cookie;
    XIDeviceEvent       * xide;
    int retVal;
    //int                   rX, rY, trgX, trgY, retVal;
    //unsigned int          mask_return;

    Bool                  res;
    Window                rootRet, childRet;
    double                rX, rY, wX, wY;
    XIButtonState         btnSt;
    XIModifierState       modsSt;
    XIGroupState          grpSt;

    /**************************************************************************/
    /*Start event processing*/
    /**************************************************************************/

    retVal = NO_KEY_PRESSED;

    while (XPending (ctx->xDpy) != 0)
    {

        xEvent.xbutton.button = 0;

        XNextEvent (ctx->xDpy, &xEvent);

        if (retVal != NO_KEY_PRESSED)
        {
            continue;
        }

        cookie = & xEvent.xcookie;

        if (   cookie->type                      != GenericEvent
            || cookie->extension                 != ctx->xiOp
            || XGetEventData (ctx->xDpy, cookie) == 0)
        {
            continue;
        }

        if (   cookie->evtype == XI_KeyPress
            || cookie->evtype == XI_ButtonPress)
        {
            XFreeEventData (ctx->xDpy, cookie);
            continue;
        }

        logCtr ("Got generic event", LOG_LVL_2, False);

        xide = (XIDeviceEvent*) xEvent.xcookie.data;

        if (cookie->evtype == XI_KeyRelease)
        {
            if (xide->detail == ctx->cloneKeyCode)
            {
                logCtr ("Grab window key sequence received", LOG_LVL_NO, False);

                retVal = TRANSLATION_COMB;
            }

            if (xide->detail == ctx->exitKeyCode)
            {
                logCtr ("Exit key sequence received", LOG_LVL_NO, False);

                retVal = EXIT_COMBINATION;
            }
        }
        else if (cookie->evtype == XI_ButtonRelease)
        {

            logCtr ("Got click", LOG_LVL_2, False);

            xide = cookie->data;

            printf ("btn = %d; tracked = %d\n", xide->buttons.mask[0],
                    1 << (TRACKED_BUTTON - 1));

            if (xide == NULL)
            {
                logCtr ("Very bad error that should have never happened:"
                        " XGetEventData faled completely!", LOG_LVL_NO, False);

                retVal = EXIT_COMBINATION;
            }
            else if (xide->buttons.mask == NULL)
            {
                logCtr ("Very bad error that should have never happened:"
                        " devEv->buttons.mask surprisingly is NULL!",
                        LOG_LVL_NO, False);

                retVal = EXIT_COMBINATION;
            }
            else if (xide->buttons.mask[0] == 1 << (TRACKED_BUTTON - 1))
            {
                memset (&btnSt, 0, sizeof (btnSt));
                btnSt.mask = NULL;

                res = XIQueryPointer (ctx->xDpy, ctx->masterPtrDevId, ctx->trgW,
                                      &rootRet, &childRet, &rX, &rY, &wX, &wY,
                                      &btnSt, &modsSt, &grpSt);

                if (res == False)
                {
                    //printf ("Error\n");
                    retVal = EXIT_COMBINATION;
                }
                else
                {
                    /*
                    printf ("\nResults of quering pointer for device id %d on"
                            " window %lX:\n"
                            "\troot:\t%lX\n"
                            "\tchild:\t%lX\n"
                            "\trootX:\t%f\n"
                            "\trootY:\t%f\n"
                            "\twinX:\t%f\n"
                            "\twinY:\t%f\n"
                            "\tbtnSt[0]:\t%X\n", ctx->masterPtrDevId, ctx->trgW,
                            rootRet, childRet, rX, rY, wX, wY,
                            btnSt.mask == NULL ? 0 : btnSt.mask[0]);
                    */
                    int rXret, rYret;

                    XTranslateCoordinates (ctx->xDpy,
                                           ctx->srcW,
                                           ctx->rootW,
                                           (int) (wX + 0.5), (int) (wY + 0.5),
                                           &rXret, &rYret,
                                           &childRet);

                    //printf ("Translated coords are %d %d, subw is %lX\n", rXret, rYret, childRet);

                    /*
                                        printf ("Current screen is %lX\n", (unsigned long)ctx->xScr);
                     */
                    /*
                                        printf ("Screen 0 is %lX\n", (unsigned long)ScreenOfDisplay(ctx->xDpy, 0));
                                        printf ("Screen 1 is %lX\n", (unsigned long)ScreenOfDisplay(ctx->xDpy, 1));
                     */

                    //printf ("Current root window is %lX\n", ctx->rootW);
                    /*
                                        printf ("Root window of screen 0 is %lX\n", RootWindowOfScreen(ScreenOfDisplay(ctx->xDpy, 0)));
                                        printf ("Root window of screen 1 is %lX\n", RootWindowOfScreen(ScreenOfDisplay(ctx->xDpy, 1)));
                     */

                    res = XIWarpPointer (ctx->xDpy,
                                         ctx->masterPtrDevId,
                                         None,
                                         ctx->rootW,
                                         0.0,   0.0,
                                         0,     0,
                                         (double) rXret, (double) rYret);

                    //printf ("Warping pointer to root window on coords %d %d\n", rXret, rYret);

                    //XWarpPointer(ctx->xDpy, None, ctx->rootW, 0, 0, 0, 0, 100, 100);

                    XFlush (ctx->xDpy);

                    nanosleep (&ctx->clickDelay, NULL);




                    if (res != Success)
                    {
                        //printf ("Error moving pointer\n");
                        if (getXErrState () == True)
                        {
                            //printf ("X err\n");
                        }
                    }

                    //clickW (ctx, Button1);

                    xEvent.xbutton.button = Button1;

                    xEvent.xbutton.x_root = (int) rX;
                    xEvent.xbutton.y_root = (int) rY;

                    xEvent.xbutton.window      = ctx->srcW;
                    xEvent.xbutton.display     = ctx->xDpy;
                    xEvent.xbutton.root        = ctx->rootW;
                    xEvent.xbutton.same_screen = True;
                    xEvent.xbutton.state       = getInSt (ctx);
                    xEvent.xbutton.subwindow   = None;
                    xEvent.xbutton.time        = CurrentTime;
                    xEvent.xbutton.type        = ButtonPress;

                    xEvent.xcookie.evtype = XI_ButtonPress;

                    XSendEvent (ctx->xDpy, ctx->srcW, True, ButtonPressMask, &xEvent);
                    XFlush (ctx->xDpy);

                    nanosleep (&ctx->clickDelay, NULL);

                    xEvent.xbutton.type        = ButtonRelease;
                    xEvent.xbutton.state |= Button1MotionMask;

                    xEvent.xcookie.evtype = XI_ButtonRelease;

                    XSendEvent (ctx->xDpy, ctx->srcW, True, ButtonReleaseMask, &xEvent);
                    XFlush (ctx->xDpy);

                    nanosleep (&ctx->clickDelay, NULL);


                    XTranslateCoordinates (ctx->xDpy,
                                           ctx->trgW,
                                           ctx->rootW,
                                           (int) (wX + 0.5), (int) (wY + 0.5),
                                           &rXret, &rYret,
                                           &childRet);

                    //printf ("Translated coords are %d %d, subw is %lX\n", rXret, rYret, childRet);


                    res = XIWarpPointer (ctx->xDpy,
                                         ctx->masterPtrDevId,
                                         None,
                                         ctx->rootW,
                                         0.0,   0.0,
                                         0,     0,
                                         (double) rXret, (double) rYret);

                    //printf ("Warping pointer to root window on coords %d %d\n", rXret, rYret);

                    //XWarpPointer(ctx->xDpy, None, ctx->rootW, 0, 0, 0, 0, 100, 100);

                    XFlush (ctx->xDpy);

                    nanosleep (&ctx->frameDelay, NULL);

                    //calculate coords for source window
                    //move pointer to sourc window
                    //click
                    //move pointer back to translation window

                    //                    res = XQueryPointer (ctx->xDpy, ctx->trgW, &rRetW, 
                    //                                         &chRetW,
                    //                                         &rX, &rY, &trgX, &trgY,
                    //                                         &mask_return);
                    //
                    //                    if (res == False)
                    //                    {
                    //                        logCtr ("Error getting pointer position\n",
                    //                                LOG_LVL_NO, False);
                    //                        continue;
                    //                    }
                    //
                    //                    mvPtrWRel (ctx, ctx->srcW, trgX, trgY);
                    //
                    //                    clickW (ctx, Button1);
                    //
                    //                    mvPtrWRel (ctx, ctx->trgW, trgX, trgY);

                    retVal = SKIP_COMBINATION;
                }

                if (btnSt.mask != NULL)
                {
                    free (btnSt.mask);
                }
            }
        }

        XFreeEventData (ctx->xDpy, cookie);
    }

    if (retVal == SKIP_COMBINATION)
    {
        retVal = NO_KEY_PRESSED;
    }
    /**************************************************************************/



    return retVal;
}

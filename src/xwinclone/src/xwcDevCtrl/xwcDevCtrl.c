#include <xwcDevCtrl.h>

Bool 
chckXI2Ext (XWCContext * ctx)
{
    int event, error, major, minor, rc, majorSupported, minorSupported;
    char buf[1024];

    logCtrl ("\tChecking XInput 2 extension", LOG_LVL_1, False);

    if (ctx == NULL)
    {
        logCtrl ("\t\tCannot check XInput 2 extension: NULL pointer to XWC context"
                " received.", LOG_LVL_NO, True);
        return False;
    }

    if (XQueryExtension (ctx->xDpy, "XInputExtension", &ctx->xiOp, &event,
                         &error) == 0)
    {
        logCtrl ("\t\tX Input extension not available.", LOG_LVL_NO, True);
        return False;
    }

    majorSupported = major = SUPPORTED_XI2_VERSION_MAJOR;
    minorSupported = minor = SUPPORTED_XI2_VERSION_MINOR;

    if ((rc = XIQueryVersion (ctx->xDpy, &major, &minor)) == BadRequest)
    {
        snprintf (buf, sizeof (buf), "\t\tXI2 not available. Server supports %d.%d",
                  major, minor);
        logCtrl (buf,  LOG_LVL_NO, True);
        return False;
    }
    else if (rc != Success)
    {
        logCtrl ("\t\tXlib internal error!", LOG_LVL_NO, True);
        return False;
    }
    else if ((major * 1000 + minor) < 1000 * majorSupported + minorSupported)
    {
        snprintf (buf, sizeof (buf), "\t\tAvailable XI2 extension version (%d.%d)"
                  " is not supported.", major, minor);
        logCtrl (buf,  LOG_LVL_NO, True);
        return False;
    }

    snprintf (buf, sizeof (buf), "\t\tXI2 extension version is (%d.%d)", major,
              minor);
    logCtrl (buf,  LOG_LVL_2, True);

    logCtrl ("\t\tSuccess", LOG_LVL_1, True);

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

    snprintf (buf, sizeof (buf), "\t\tGrabbing keycode %d on window 0x%lX:",
              xKCode, w);
    logCtrl (buf, LOG_LVL_1, False);

    if (ctx == NULL)
    {
        logCtrl ("\t\t\tCannot grab key: NULL pointer to XWC context received.",
                LOG_LVL_NO, False);
        return False;
    }

    if (ctx->kbds == NULL)
    {
        logCtrl ("\t\t\tCannot grab key: NULL pointer to keyboard device list"
                " received.", LOG_LVL_NO, False);
        return False;
    }

    if (ctx->kbds->nDevs < 0 || ctx->kbds->nDevs > 127)
    {
        logCtrl ("\t\t\tCannot grab key: bad device count.", LOG_LVL_NO, False);
        return False;
    }

    if (ctx->kbds->devs == NULL)
    {
        logCtrl ("\t\t\tCannot grab key: NULL pointer to device array received.",
                LOG_LVL_NO, False);
        return False;
    }

    if (nMods < 0 || nMods > 127)
    {
        logCtrl ("\t\t\tCannot grab key: bad mods count.", LOG_LVL_NO, False);
        return False;
    }

    if (mods == NULL)
    {
        logCtrl ("\t\t\tCannot grab key: NULL pointer to modifiers array received.",
                LOG_LVL_NO, False);
        return False;
    }

    if (w == None)
    {
        logCtrl ("\t\t\tCannot grab key: No window specified.", LOG_LVL_NO, False);
        return False;
    }

    if (grab == False)
    {
        for (i = 0; i < ctx->kbds->nDevs; ++ i)
        {
            XIUngrabKeycode (ctx->xDpy, ctx->kbds->devs[i], xKCode, w, nMods,
                             mods);

            snprintf (buf, sizeof (buf), "\t\t\tkeycode ungrabbed on device %d.",
                      ctx->kbds->devs[i]);
            logCtrl (buf, LOG_LVL_2, True);
        }
        return True;
    }

    failMods = (XIGrabModifiers*) malloc (sizeof (XIGrabModifiers) * nMods);

    if (failMods == NULL)
    {
        logCtrl ("\t\t\tCannot grab key: cannot allocate array for failed mods.",
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
            snprintf (buf, sizeof (buf), "\t\t\tkeycode grabbed on device %d.",
                      ctx->kbds->devs[i]);
            logCtrl (buf, LOG_LVL_2, True);
        }
    }

    if (nfailMods != 0)
    {
        for (i = 0; i < nfailMods; ++ i)
        {
            snprintf (buf, sizeof (buf), "\t\t\tModifier %x failed with error %d\n",
                      failMods[i].modifiers, failMods[i].status);
            logCtrl (buf, LOG_LVL_NO, False);
        }

        free (failMods);

        return False;
    }

    snprintf (buf, sizeof (buf), "\t\t\tSuccess");
    logCtrl (buf, LOG_LVL_2, True);

    free (failMods);

    return True;
}

Bool
getInputDevices (XWCContext * ctx)
{
    XIDeviceInfo * allDevsInfo, * dev;
    char           buf[1024];
    int            i, * kbdIds, masterKbdCnt, nAllDevs;

    logCtrl ("\tBuiding list of input devices", LOG_LVL_1, False);

    if (ctx == NULL)
    {
        logCtrl ("\t\tCannot get list of all devices: NULL pointer to program"
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
            logCtrl ("\t\tCannot get list of all devices: cannot allocate memory"
                    " for device's id array!", LOG_LVL_NO, False);
            XIFreeDeviceInfo (allDevsInfo);
            return False;
        }

        masterKbdCnt = 0;

        for (i = 0; i < nAllDevs; ++ i)
        {
            dev = & allDevsInfo[i];

            switch (dev->use)
            {
                case XIMasterKeyboard:
                    kbdIds[masterKbdCnt] = dev->deviceid;
                    ++ masterKbdCnt;
                    break;

                case XISlavePointer:
                    if (ctx->translateOnly == False)
                    {
                        if (strncmp (dev->name, ctx->ptrDevName,
                                     MAX_POINTER_DEV_NAME_LENGTH) == STR_EQUAL)
                        {
                            ctx->slavePtrDevId  = dev->deviceid;
                            ctx->masterPtrDevId = dev->attachment;
                        }
                    }
                    break;

                default:
                    break;
            }
        }

        XIFreeDeviceInfo (allDevsInfo);

        if (ctx->slavePtrDevId == NO_DEVICE && ctx->translateOnly == False)
        {
            snprintf (buf, sizeof (buf), "\t\tCannot get list of all devices:"
                      " no slave pointer with name('%s') found, see '$ xinput "
                      "list' to find propriate pointer name for slave pointer!",
                      ctx->ptrDevName);
            logCtrl (buf, LOG_LVL_NO, False);
            free (kbdIds);
            return False;
        }

        ctx->kbds = (DevList *) malloc (sizeof (DevList));

        if (ctx->kbds == NULL)
        {
            logCtrl ("\t\tCannot get list of all devices: Cannot allocate memory"
                    " for device list!", LOG_LVL_NO, False);
            free (kbdIds);
            return False;
        }

        ctx->kbds->devs  = kbdIds;
        ctx->kbds->nDevs = masterKbdCnt;
    }
    else
    {
        logCtrl ("\t\tCannot get list of all devices: XIQueryDevice error!",
                LOG_LVL_NO, False);
        return False;
    }

    logCtrl ("\t\tsuccess",
            LOG_LVL_2, True);

    return True;
}

Bool
grabAllKeys (XWCContext * ctx)
{
    size_t sizeTmp;

    logCtrl ("\tTrying to grab all control keys:", LOG_LVL_1, False);

    /**************************************************************************/
    /*prepare modifiers*/
    /**************************************************************************/
    ctx->nMods = 4;

    sizeTmp = sizeof (XIGrabModifiers);

    ctx->clMods   = (XIGrabModifiers *) malloc (sizeTmp * ctx->nMods);

    if (ctx->clMods == NULL)
    {
        logCtrl ("\t\tError allocating memory for XIGrabModifiers struct!",
                LOG_LVL_NO, False);
        return False;
    }

    ctx->exitMods = (XIGrabModifiers *) malloc (sizeTmp * ctx->nMods);

    if (ctx->exitMods == NULL)
    {
        logCtrl ("\t\tError allocating memory for XIGrabModifiers struct!",
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
        logCtrl ("\t\tError grabbing exit key!", LOG_LVL_NO, False);
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
        logCtrl ("\t\tError grabbing clone key!", LOG_LVL_NO, False);
        grabKeyCtrl (ctx, ctx->rootW, ctx->exitKeyCode, ctx->nMods,
                     ctx->exitMods, False);
        return False;
    }
    /**************************************************************************/

    logCtrl ("\t\tsuccess", LOG_LVL_1, False);

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

int
procKeySeqEv (XWCContext    * ctx,
              XIDeviceEvent * xide)
{

    if (xide == NULL || ctx == NULL)
    {
        logCtrl ("Cannot process pressed key: NULL pointer(s) received!",
                LOG_LVL_NO, False);
        return EXIT_COMBINATION;
    }

    /* For button events, detail is the button number (after mapping
     * applies of course). For key events, detail is the keycode. 
     * XI2 supports 32-bit keycodes, btw. For motion events, 
     * detail is 0. 
     * From http://who-t.blogspot.cz/2009/07/xi2-recipes-part-4.html*/
    if (xide->detail == ctx->cloneKeyCode)
    {
        logCtrl ("Grab window key sequence received", LOG_LVL_NO, False);

        return TRANSLATION_COMB;
    }

    if (xide->detail == ctx->exitKeyCode)
    {
        logCtrl ("Exit key sequence received", LOG_LVL_NO, False);

        return EXIT_COMBINATION;
    }

    return NO_KEY_PRESSED;
}

Bool
getTrgWPtrData (XWCContext      * ctx,
                int             * trgX,
                int             * trgY,
                XIModifierState * modsSt,
                int             * trgRX,
                int             * trgRY)
{
    XIButtonState btnSt;
    XIGroupState  grpSt;
    Window        rootRet, childRet;
    Bool          res;
    double        rX, rY, wX, wY;

    /*simple check*/
    /*TODO change to asserts*/
    if (ctx == NULL || trgX == NULL || trgY == NULL || modsSt == NULL)
    {
        logCtrl ("Cannot get target window pointer data: "
                "NULL pointer received!", LOG_LVL_NO, True);
        return False;
    }

    memset (&btnSt, 0, sizeof (btnSt));
    btnSt.mask = NULL;

    res = XIQueryPointer (ctx->xDpy, ctx->masterPtrDevId, ctx->trgW,
                          &rootRet, &childRet, &rX, &rY, &wX,
                          &wY, &btnSt, modsSt, &grpSt);

    /*free btnSt.mask after XIQueryPointer call*/
    if (btnSt.mask != NULL)
    {
        free (btnSt.mask);
    }

    if (res == False || getXErrState (ctx) == True)
    {
        logCtrl ("Cannot get target window pointer data: "
                "XIQueryPointer error!", LOG_LVL_NO, True);
        return False;
    }

    *trgX  = (int) (wX + 0.5);
    *trgY  = (int) (wY + 0.5);
    *trgRX = (int) (rX + 0.5);
    *trgRY = (int) (rY + 0.5);

    return True;
}

Bool
adjPtrLoc (XWCContext * ctx,
           int          inX,
           int          inY,
           int        * adjX,
           int        * adjY,
           int        * adjRX,
           int        * adjRY)
{
    Window childRet;
    int trgWinLOff, trgWinTOff, trgWinW, trgWinH, srcWinW, srcWinH;

    if (ctx == NULL || adjX == NULL || adjY == NULL || adjRX == NULL
        || adjRY == NULL)
    {
        logCtrl ("Cannot translate source window coordinates to root window:"
                " NULL pointer received!", LOG_LVL_NO, False);
        return False;
    }

    *adjX = inX;
    *adjY = inY;

    if (ctx->autoCenter == True)
    {
        if (   getWAttr (ctx, &ctx->srcW, &ctx->srcWAttr) == False
            || getWAttr (ctx, &ctx->trgW, &ctx->trgWAttr) == False)
        {
            return False;
        }

        trgWinW = ctx->trgWAttr.width;
        srcWinW = ctx->srcWAttr.width;
        trgWinH = ctx->trgWAttr.height;
        srcWinH = ctx->srcWAttr.height;

        trgWinLOff = trgWinW - srcWinW;
        trgWinTOff = trgWinH - srcWinH - ctx->topOffset;

        trgWinLOff /= 2;
        trgWinTOff /= 2;

        *adjX -= trgWinLOff;
        *adjY -= trgWinTOff;
    }
    else
    {
        *adjY += ctx->topOffset;
    }

    XTranslateCoordinates (ctx->xDpy, ctx->srcW, ctx->rootW,
                           *adjX, *adjY,
                           adjRX, adjRY,
                           &childRet);

    if (getXErrState (ctx) == True)
    {
        logCtrl ("Cannot translate source window coordinates to root window:"
                " XTranslateCoordinates error!", LOG_LVL_NO, False);
        return False;
    }

    return True;
}

Bool
getBtnEv (XWCContext      * ctx,
          XIModifierState * modsSt,
          XEvent          * xev,
          int               adjX,
          int               adjY,
          int               adjRX,
          int               adjRY)
{
    if (ctx == NULL || modsSt == NULL || xev == NULL)
    {
        logCtrl ("Cannot prepare event structure: NULL pointer received!",
                LOG_LVL_NO, False);
        return False;
    }

    memset (xev, 0, sizeof (*xev));

    xev->xbutton.serial      = 0;
    xev->xbutton.send_event  = True;
    xev->xbutton.display     = ctx->xDpy;
    xev->xbutton.window      = ctx->srcW;
    xev->xbutton.root        = ctx->rootW;
    xev->xbutton.subwindow   = None;
    xev->xbutton.time        = CurrentTime;
    xev->xbutton.x           = adjX;
    xev->xbutton.y           = adjY;
    xev->xbutton.x_root      = adjRX;
    xev->xbutton.y_root      = adjRY;
    xev->xbutton.state       = modsSt->effective;
    xev->xbutton.button      = TRACKED_BUTTON - 1;
    xev->xbutton.same_screen = True;

    return True;
}

Bool
mvPtr (XWCContext * ctx,
       int          x,
       int          y)
{
    Bool res;

    if (ctx == NULL)
    {
        logCtrl ("Cannot move pointer: ctx is NULL!", LOG_LVL_NO, False);
        return False;
    }

    res = XIWarpPointer (ctx->xDpy,
                         ctx->masterPtrDevId,
                         None,
                         ctx->rootW,
                         0.0,            0.0,
                         0,              0,
                         (double) x, (double) y);

    if (res != Success || getXErrState (ctx) == True)
    {
        logCtrl ("Cannot move pointer: XIWarpPointer error!", LOG_LVL_NO, False);
        return False;
    }

    return True;
}

Bool
btnClick (XWCContext * ctx,
          XEvent     * btnEv)
{
    Bool res;

    if (ctx == NULL || btnEv == NULL)
    {
        logCtrl ("Cannot emulate click: "
                "NULL pointer received!", LOG_LVL_NO, True);
        return False;
    }

    /**************************************************************************/
    /*Send button press event*/
    /**************************************************************************/
    btnEv->xbutton.type = ButtonPress;
    res = XSendEvent (ctx->xDpy, ctx->srcW, False,
                      ButtonPressMask, btnEv);

    if (res == 0 || getXErrState (ctx) == True)
    {
        logCtrl ("Cannot emulate click: XSendEvent error!", LOG_LVL_NO, True);
        return False;
    }
    else
    {
        logCtrl ("Button press event sent\n", LOG_LVL_2, True);
    }
    /**************************************************************************/


    if (ctx->clickDelay.tv_nsec + ctx->clickDelay.tv_sec != 0)
    {
        nanosleep (&ctx->clickDelay, NULL);
    }


    /**************************************************************************/
    /*Send button release event*/
    /**************************************************************************/
    btnEv->xbutton.type = ButtonRelease;
    res = XSendEvent (ctx->xDpy, ctx->srcW, False,
                      ButtonReleaseMask, btnEv);

    if (res == 0 || getXErrState (ctx) == True)
    {
        logCtrl ("Cannot emulate click: XSendEvent error!", LOG_LVL_NO, True);
        return False;
    }
    else
    {
        logCtrl ("Button release event sent\n", LOG_LVL_2, True);
    }
    /**************************************************************************/

    return True;
}

int
procBtnEv (XWCContext    * ctx,
           XIDeviceEvent * xide)
{
    XIModifierState modSt;
    XEvent          btnEv;
    Window          focused;
    triState        visRes;
    int             trgRX, trgRY, adjRX, adjRY, adjX, adjY, trgX, trgY;
    char            buf[1024];

    logCtrl ("Got click:", LOG_LVL_2, False);

    if (ctx == NULL)
    {
        logCtrl ("Cannot process pressed button event: ctx is NULL!",
                LOG_LVL_NO, True);
        return EXIT_COMBINATION;
    }

    if (xide == NULL)
    {
        logCtrl ("Very bad error that should have never happened: XGetEventData "
                "failed completely at procBtnEv!", LOG_LVL_NO, True);
        return EXIT_COMBINATION;
    }

    if (xide->buttons.mask == NULL)
    {
        logCtrl ("Very bad error that should have never happened: "
                "xide->buttons.mask surprisingly is NULL at procBtnEv!",
                LOG_LVL_NO, True);
        return EXIT_COMBINATION;
    }

    snprintf (buf, sizeof (buf), "btn = %d; tracked = %d\n",
              xide->buttons.mask[0], 1 << (TRACKED_BUTTON - 1));
    logCtrl (buf, LOG_LVL_2, True);

    /* button is expressed as 1 shifted to the left by button number,
     * XIButtonState structure has an array of bytes to store pressed 
     * button number, but buttons we track will always fit into 
     * 1 byte. If you need to track buttons with numbers greater
     * than 7 then change next code to check all bytes */
    if (xide->buttons.mask[0] == 1 << (TRACKED_BUTTON - 1))
    {
        /**********************************************************************/
        /*check source window state*/
        /**********************************************************************/
        visRes = isWinVis (ctx, ctx->srcW);

        if (visRes == False)
        {
            logCtrl ("Source seems to be hidden", LOG_LVL_2, True);

            if (toggleHiddenState (ctx, ctx->srcW) == False )
            {
                logCtrl ("Cannot process pressed button event: "
                        "toggleHiddenState error!", LOG_LVL_NO, True);
                return EXIT_COMBINATION;
            }

            if (ctx->restoreDelay.tv_nsec + ctx->restoreDelay.tv_sec != 0)
            {
                nanosleep (&ctx->restoreDelay, NULL);
            }

            visRes = isWinVis (ctx, ctx->srcW);

            if (visRes == False)
            {
                logCtrl ("Cannot process pressed button event: problem occured"
                        " while restoring hidden window, if last is true, check"
                        " if source window was on visible(active) desktop!",
                        LOG_LVL_NO, True);
                return EXIT_COMBINATION;
            }
        }

        if (visRes == UNDEFINED)
        {
            logCtrl ("Error getting source window visibility state: "
                    "isWinVis error!", LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*Get pointer coordinates and modifiers state in target window*/
        /**********************************************************************/
        if (getTrgWPtrData (ctx, &trgX, &trgY, &modSt, &trgRX, &trgRY) == False)
        {
            logCtrl ("Cannot process pressed button event: "
                    "getTrgWPtrData error!", LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*Adjust pointer coordinates for source window*/
        /**********************************************************************/
        if (adjPtrLoc (ctx, trgX, trgY, &adjX, &adjY, &adjRX, &adjRY) == False)
        {
            logCtrl ("Cannot process pressed button event: adjPtrLoc error!",
                    LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*raise source if needed*/
        /**********************************************************************/
        focused = ctx->srcW;

        if (wRaiseCtrl (ctx, &focused) == False)
        {
            logCtrl ("Cannot process pressed button event: wRaiseCtrl error!",
                    LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*move pointer to source window*/
        /**********************************************************************/
        if (mvPtr (ctx, adjRX, adjRY) == False)
        {
            logCtrl ("Cannot process pressed button event: mvPtr error!",
                    LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        else
        {
            logCtrl ("Pointer moved to source\n", LOG_LVL_2, True);
        }
        /**********************************************************************/


        /**********************************************************************/
        /*prepare event structure*/
        /**********************************************************************/
        if (getBtnEv (ctx, &modSt, &btnEv, adjX, adjY, adjRX, adjRY) == False)
        {
            logCtrl ("Cannot process pressed button event: getBtnEv"
                    " error!", LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*emulate click*/
        /**********************************************************************/
        if (btnClick (ctx, &btnEv) == False)
        {
            logCtrl ("Cannot process pressed button event: btnClick error!",
                    LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*move pointer to target window*/
        /**********************************************************************/
        if (mvPtr (ctx, trgRX, trgRY) == False)
        {
            logCtrl ("Cannot process pressed button event: mvPtr error!",
                    LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        else
        {
            logCtrl ("Pointer moved to target\n", LOG_LVL_2, True);
        }
        /**********************************************************************/


        /**********************************************************************/
        /*raise last focused if needed*/
        /**********************************************************************/
        if (wRaiseCtrl (ctx, &focused) == False)
        {
            logCtrl ("Cannot process pressed button event: wRaiseCtrl error!",
                    LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        /**********************************************************************/
    }

    return SKIP_OTHER_EVENTS;
}

int
getPressedComb (XWCContext * ctx)
{
    XEvent                xEvent;
    XGenericEventCookie * cookie;
    int                   retVal;

    retVal = NO_KEY_PRESSED;

    while (XPending (ctx->xDpy) != 0)
    {
        XNextEvent (ctx->xDpy, &xEvent);

        if (retVal != NO_KEY_PRESSED)
        {
            /* if some event has been processed (so retVal has changed), skip
             * all other events for this client in X server queue */
            continue;
        }

        cookie = & xEvent.xcookie;

        if (   cookie->type                      != GenericEvent
            || cookie->extension                 != ctx->xiOp
            || XGetEventData (ctx->xDpy, cookie) == False)
        {
            continue;
        }

        logCtrl ("Got generic event", LOG_LVL_2, False);

        if (   cookie->evtype == XI_KeyPress
            || cookie->evtype == XI_ButtonPress)
        {
            /*free cookie after XGetEventData call*/
            XFreeEventData (ctx->xDpy, cookie);
            /*skip press events*/
            continue;
        }

        /* according to documentation for XI_KeyRelease and XI_ButtonRelease 
         * event types there is XIDeviceEvent data structure 
         * in the data field of cookie structure*/
        switch (cookie->evtype)
        {
            case XI_KeyRelease:
                retVal = procKeySeqEv (ctx, (XIDeviceEvent*) cookie->data);
                break;

            case XI_ButtonRelease:
                if (ctx->translateOnly == False)
                {
                    retVal = procBtnEv (ctx, (XIDeviceEvent*) cookie->data);
                }
                break;

            default:
                break;
        }

        /*free cookie after XGetEventData call*/
        XFreeEventData (ctx->xDpy, cookie);
    }

    if (retVal == SKIP_OTHER_EVENTS)
    {
        retVal = NO_KEY_PRESSED;
    }

    return retVal;
}

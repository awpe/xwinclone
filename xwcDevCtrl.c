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

int
procKeySeqEv (XWCContext    * ctx,
              XIDeviceEvent * xide)
{
    /* For button events, detail is the button number (after mapping
     * applies of course). For key events, detail is the keycode. 
     * XI2 supports 32-bit keycodes, btw. For motion events, 
     * detail is 0. 
     * From http://who-t.blogspot.cz/2009/07/xi2-recipes-part-4.html*/
    if (xide->detail == ctx->cloneKeyCode)
    {
        logCtr ("Grab window key sequence received", LOG_LVL_NO, False);

        return TRANSLATION_COMB;
    }

    if (xide->detail == ctx->exitKeyCode)
    {
        logCtr ("Exit key sequence received", LOG_LVL_NO, False);

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
        logCtr ("Cannot get target window pointer data: "
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

    if (res == False)
    {
        logCtr ("Cannot get target window pointer data: "
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

    *adjX = inX;
    *adjY = inY;

    XTranslateCoordinates (ctx->xDpy, ctx->srcW, ctx->rootW,
                           *adjX, *adjY,
                           adjRX, adjRY,
                           &childRet);

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
        logCtr ("Cannot prepare event structure: NULL pointer received!",
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
        logCtr ("Cannot move pointer: ctx is NULL!", LOG_LVL_NO, True);
        return False;
    }

    res = XIWarpPointer (ctx->xDpy,
                         ctx->masterPtrDevId,
                         None,
                         ctx->rootW,
                         0.0,            0.0,
                         0,              0,
                         (double) x, (double) y);

    if (res != Success)
    {
        logCtr ("Cannot move pointer: XIWarpPointer error!", LOG_LVL_NO, True);
        return False;
    }

    XSync (ctx->xDpy, False);

    return True;
}

Bool
btnClick (XWCContext * ctx,
          XEvent     * btnEv)
{
    Bool res;

    if (ctx == NULL || btnEv == NULL)
    {
        logCtr ("Cannot emulate click: "
                "NULL pointer received!", LOG_LVL_NO, True);
        return False;
    }

    /**************************************************************************/
    /*Send button press event*/
    /**************************************************************************/
    btnEv->xbutton.type = ButtonPress;
    res = XSendEvent (ctx->xDpy, ctx->srcW, False,
                      ButtonPressMask, btnEv);

    if (res == 0)
    {
        logCtr ("Cannot emulate click: XSendEvent error!", LOG_LVL_NO, True);
        return False;
    }
    else
    {
        logCtr ("Button press event sent\n", LOG_LVL_2, True);
    }

    XSync (ctx->xDpy, False); //make sure X server received press event
    /**************************************************************************/


#if (BTN_CLICK_DELAY_ENABLE==1)
    nanosleep (&ctx->clickDelay, NULL);
#endif


    /**************************************************************************/
    /*Send button release event*/
    /**************************************************************************/
    btnEv->xbutton.type = ButtonRelease;
    res = XSendEvent (ctx->xDpy, ctx->srcW, False,
                      ButtonReleaseMask, btnEv);

    if (res == 0)
    {
        logCtr ("Cannot emulate click: XSendEvent error!", LOG_LVL_NO, True);
        return False;
    }
    else
    {
        logCtr ("Button release event sent\n", LOG_LVL_2, True);
    }

    XSync (ctx->xDpy, False); //make sure X server received release event
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
    Atom            srcWStateAtom;
    int             trgRX, trgRY, adjRX, adjRY, adjX, adjY, trgX, trgY;
    char            buf[1024];

    logCtr ("Got click:", LOG_LVL_2, False);

    if (ctx == NULL)
    {
        logCtr ("Cannot process pressed button event: ctx is NULL!",
                LOG_LVL_NO, True);
        return EXIT_COMBINATION;
    }

    if (xide == NULL)
    {
        logCtr ("Very bad error that should have never happened: XGetEventData "
                "failed completely at procBtnEv!", LOG_LVL_NO, True);
        return EXIT_COMBINATION;
    }

    if (xide->buttons.mask == NULL)
    {
        logCtr ("Very bad error that should have never happened: "
                "xide->buttons.mask surprisingly is NULL at procBtnEv!",
                LOG_LVL_NO, True);
        return EXIT_COMBINATION;
    }

    snprintf (buf, sizeof (buf), "btn = %d; tracked = %d\n",
              xide->buttons.mask[0], 1 << (TRACKED_BUTTON - 1));
    logCtr (buf, LOG_LVL_2, True);

    /* button is expressed as 1 shifted to the left by button number,
     * XIButtonState structure has an array of bytes to store pressed 
     * button number, but buttons we track will always fit into 
     * 1 byte. If you need to track buttons with numbers greater
     * than 7 then change next code to check all bytes */
    if (xide->buttons.mask[0] == 1 << (TRACKED_BUTTON - 1))
    {
        /**********************************************************************/
        /*check source window state*/
        /*TODO move this to xwcWinUtil*/
        /**********************************************************************/
        XSync (ctx->xDpy, 0);
        XGetWindowAttributes (ctx->xDpy, ctx->srcW, &ctx->srcWAttr);

        if (getXErrState () == True)
        {
            logCtr ("Cannot process pressed button event: "
                    "XGetWindowAttributes error!", LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }

        Atom property = XInternAtom (ctx->xDpy, "_NET_WM_STATE", False);
        Atom actual_type;
        int actual_format;
        unsigned long nitems;
        unsigned long leftover;
        unsigned char *data = NULL;
        if (XGetWindowProperty (ctx->xDpy, ctx->srcW, property, 0L,
                                (long) BUFSIZ, False, AnyPropertyType, &actual_type, &actual_format,
                                &nitems, &leftover, &data) != Success)
        {
            printf ("Cannot read property\n");
            return EXIT_COMBINATION;
        }
        char * tmp;
        Bool hidden = False;
        Atom hiddenProperty = XInternAtom (ctx->xDpy, "_NET_WM_STATE_HIDDEN", False);
        for (int i = 0; i < nitems; ++i)
        {
            printf ("%ld\n", ((long *) (data))[i]);
            tmp = XGetAtomName (ctx->xDpy, ((long *) (data))[i]);
            printf ("%s\n\n", tmp);
            XFree (tmp);
            if (hiddenProperty == ((long *) (data))[i])
            {
                hidden = True;
                printf ("actual_type = %ld\n", actual_type);
                printf ("actual_format = %d\n", actual_format);
                break;
            }
        }
        XFree (data);



        /*
                        if (ctx->srcWAttr.map_state != IsViewable || hidden == True)
                        {
                            logCtr ("Cannot process pressed button event: "
                                    "source window is not visible!", LOG_LVL_1, True);
                            return NO_KEY_PRESSED;
                        }
         */

        /*
                    XChangeProperty (ctx->xDpy, ctx->srcW, property, XA_ATOM,
                                     32, PropModeReplace, (unsigned char *) name,
                                     name ? strlen (name) : 0);
         */
        if (hidden == True || ctx->srcWAttr.map_state != IsViewable)
        {
            printf ("Source is hidden\n");
            
            XEvent event;
            memset (&event, 0, sizeof (event));
            long mask = SubstructureRedirectMask | SubstructureNotifyMask;

            event.xclient.type = ClientMessage;
            event.xclient.serial = 0;
            event.xclient.send_event = True;
            event.xclient.message_type = XInternAtom (ctx->xDpy, "_NET_WM_STATE", False);
            event.xclient.window = ctx->srcW;
            event.xclient.format = 32;
            event.xclient.data.l[0] = 2;
            event.xclient.data.l[1] = XInternAtom (ctx->xDpy, "_NET_WM_STATE_HIDDEN", False);
            event.xclient.data.l[2] = 0;
            event.xclient.data.l[3] = 0;
            event.xclient.data.l[4] = 0;

            if (XSendEvent (ctx->xDpy, ctx->rootW, False, mask, &event))
            {
                XSync (ctx->xDpy, False);
            }
            else
            {
                printf ("Cannot send event.\n");
                return EXIT_COMBINATION;
            }

            nanosleep (&ctx->raiseDelay, NULL);
            
            Bool b;
            property = XInternAtom (ctx->xDpy, "WM_STATE", False);
            while (1)
            {
                if (XGetWindowProperty (ctx->xDpy, ctx->srcW, property, 0L,
                                        (long) BUFSIZ, False, AnyPropertyType, &actual_type, &actual_format,
                                        &nitems, &leftover, &data) != Success)
                {
                    printf ("Cannot read property\n");
                    return EXIT_COMBINATION;
                }
                printf ("actual_type = %ld\n", actual_type);
                tmp = XGetAtomName (ctx->xDpy, actual_type);
                printf ("actual type name %s\n", tmp);
                free (tmp);
                printf ("actual_format = %d\n", actual_format);
                b = False;
                printf ("nitems %ld\n", nitems);
                for (int i = 0; i < nitems - 1; ++i)
                {
                    printf ("%ld\n", ((long *) (data))[i]);
                    tmp = XGetAtomName (ctx->xDpy, ((long *) (data))[i]);
                    printf ("%s\n\n", tmp);
                    XFree (tmp);
                    if (hiddenProperty == ((long *) (data))[i])
                    {
                        b = True;
                        printf ("still hidden\n");
                        break;
                    }
                }
                XFree (data);
                if (b == False)
                {
                    printf ("visible\n");
                    break;
                }

                nanosleep (&ctx->raiseDelay, NULL);

            }

        }
        /**********************************************************************/


        /**********************************************************************/
        /*Get pointer coordinates and modifiers state in target window*/
        /**********************************************************************/
        if (getTrgWPtrData (ctx, &trgX, &trgY, &modSt, &trgRX, &trgRY) == False)
        {
            logCtr ("Cannot process pressed button event: "
                    "getTrgWPtrData error!", LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*Adjust pointer coordinates for source window*/
        /**********************************************************************/
        if (adjPtrLoc (ctx, trgX, trgY, &adjX, &adjY, &adjRX, &adjRY) == False)
        {
            logCtr ("Cannot process pressed button event: adjPtrLoc error!",
                    LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*raise source if needed*/
        /**********************************************************************/
        focused = ctx->srcW;
        if (wRaiseCtrl (ctx, &focused, &srcWStateAtom) == False)
        {
            logCtr ("Cannot process pressed button event: wRaiseCtrl error!",
                    LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*move pointer to source window*/
        /**********************************************************************/
        if (mvPtr (ctx, adjRX, adjRY) == False)
        {
            logCtr ("Cannot process pressed button event: mvPtr error!",
                    LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        else
        {
            logCtr ("Pointer moved to source\n", LOG_LVL_2, True);
        }
        /**********************************************************************/


        /**********************************************************************/
        /*prepare event structure*/
        /**********************************************************************/
        if (getBtnEv (ctx, &modSt, &btnEv, adjX, adjY, adjRX, adjRY) == False)
        {
            logCtr ("Cannot process pressed button event: getBtnEv"
                    " error!", LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*emulate click*/
        /**********************************************************************/
        if (btnClick (ctx, &btnEv) == False)
        {
            logCtr ("Cannot process pressed button event: btnClick error!",
                    LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*move pointer to target window*/
        /**********************************************************************/
        if (mvPtr (ctx, trgRX, trgRY) == False)
        {
            logCtr ("Cannot process pressed button event: mvPtr error!",
                    LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        else
        {
            logCtr ("Pointer moved to target\n", LOG_LVL_2, True);
        }
        /**********************************************************************/



        /**********************************************************************/
        /*raise last focused if needed*/
        /**********************************************************************/


        if (wRaiseCtrl (ctx, &focused, &srcWStateAtom) == False)
        {
            logCtr ("Cannot process pressed button event: wRaiseCtrl error!",
                    LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        
        
        
        if (hidden == True)
        {
            nanosleep (&ctx->raiseDelay, NULL);
            XEvent event;
            memset (&event, 0, sizeof (event));
            long mask = SubstructureRedirectMask | SubstructureNotifyMask;

            event.xclient.type = ClientMessage;
            event.xclient.serial = 0;
            event.xclient.send_event = True;
            event.xclient.message_type = XInternAtom (ctx->xDpy, "_NET_WM_STATE", False);
            event.xclient.window = ctx->srcW;
            event.xclient.format = 32;
            event.xclient.data.l[0] = 2;
            event.xclient.data.l[1] = XInternAtom (ctx->xDpy, "_NET_WM_STATE_HIDDEN", False);
            event.xclient.data.l[2] = 0;
            event.xclient.data.l[3] = 0;
            event.xclient.data.l[4] = 0;

            if (XSendEvent (ctx->xDpy, ctx->rootW, False, mask, &event))
            {
                XSync (ctx->xDpy, False);
            }
            else
            {
                printf ("Cannot send event.\n");
                return EXIT_COMBINATION;
            }
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

        logCtr ("Got generic event", LOG_LVL_2, False);

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
                retVal = procBtnEv (ctx, (XIDeviceEvent*) cookie->data);
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

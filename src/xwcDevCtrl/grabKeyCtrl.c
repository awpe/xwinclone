/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://raw.githubusercontent.com/awpe/xwinclone/master/LICENSE.md>.  */

#include <xwcDevCtrl.h>

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
        logCtrl ("\t\t\tCannot grab key: NULL pointer to device array "
                 "received.", LOG_LVL_NO, False);
        return False;
    }

    if (nMods < 0 || nMods > 127)
    {
        logCtrl ("\t\t\tCannot grab key: bad mods count.", LOG_LVL_NO, False);
        return False;
    }

    if (mods == NULL)
    {
        logCtrl ("\t\t\tCannot grab key: NULL pointer to modifiers array "
                 "received.", LOG_LVL_NO, False);
        return False;
    }

    if (w == None)
    {
        logCtrl ("\t\t\tCannot grab key: No window specified.", LOG_LVL_NO,
                 False);
        return False;
    }

    if (grab == False)
    {
        for (i = 0; i < ctx->kbds->nDevs; ++ i)
        {
            XIUngrabKeycode (ctx->xDpy, ctx->kbds->devs[i], xKCode, w, nMods,
                             mods);

            snprintf (buf, sizeof (buf), "\t\t\tkeycode ungrabbed on device "
                      "%d.", ctx->kbds->devs[i]);
            logCtrl (buf, LOG_LVL_2, True);
        }
        return True;
    }

    failMods = (XIGrabModifiers*) malloc (sizeof (XIGrabModifiers) * nMods);

    if (failMods == NULL)
    {
        logCtrl ("\t\t\tCannot grab key: cannot allocate array for failed "
                 "mods.", LOG_LVL_NO, False);
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
            snprintf (buf, sizeof (buf), "\t\t\tModifier %x failed with error "
                      "%d\n", failMods[i].modifiers, failMods[i].status);
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

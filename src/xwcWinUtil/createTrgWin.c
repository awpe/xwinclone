/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://raw.githubusercontent.com/awpe/xwinclone/master/LICENSE.md>.  */

#include <xwcWinUtil.h>

Bool
createTrgWin (XWCContext * ctx)
{
    XSetWindowAttributes  trgWinSetAttr;
    XVisualInfo           xVisInfo;
    long long int         mask;
    char                  buf[1024];

    logCtrl ("Creating translation window:", LOG_LVL_1, False);

    if (getVisOfScr (ctx, &xVisInfo) == False)
    {
        logCtrl ("\tError: no such visual", LOG_LVL_NO, False);
        return False;
    }

    trgWinSetAttr.colormap         = createColormap (ctx, xVisInfo.visual);
    trgWinSetAttr.background_pixel = ctx->bgColor.pixel;
    trgWinSetAttr.border_pixel     = 0;
    trgWinSetAttr.bit_gravity      = NorthWestGravity;
    mask                           = CWBackPixel | CWColormap | CWBorderPixel |
        CWBitGravity | CWEventMask;

    if (ctx->procBtnEv == True)
    {
        trgWinSetAttr.event_mask = ButtonPressMask | ButtonReleaseMask |
            ButtonMotionMask;
    }
    else
    {
        trgWinSetAttr.event_mask = 0;
    }

    ctx->trgW = createWindow (ctx, xVisInfo.visual, mask, &trgWinSetAttr);

    if (getXErrState (ctx) == True)
    {
        if (ctx->trgW != None)
        {
            XDestroyWindow (ctx->xDpy, ctx->trgW);
        }
        logCtrl ("\tfailed to create window!", LOG_LVL_NO, False);
        return None;
    }

    if (setWinTitlebar (ctx, ctx->trgW, WM_CLASS_PRG_NAME_STR) == False
        || setWinClass (ctx, ctx->trgW, WM_CLASS_PRG_NAME_STR,
                        WM_CLASS_CLASS_NAME_STR) == False )
    {
        return False;
    }

    XMapWindow (ctx->xDpy, ctx->trgW);

    if (getXErrState (ctx) == True)
    {
        logCtrl ("\tfailed to map window!", LOG_LVL_NO, False);
        return False;
    }

    XGetWindowAttributes (ctx->xDpy, ctx->trgW, &ctx->trgWAttr);

    if (getXErrState (ctx) == True)
    {
        logCtrl ("\tfailed to get target window attributes!", LOG_LVL_NO,
                 False);
        return False;
    }

    if (ctx->procBtnEv == True)
    {
        XIEventMask evmasks[1];
        unsigned char mask1[(XI_LASTEVENT + 7) / 8];

        memset (mask1, 0, sizeof (mask1));

        XISetMask (mask1, XI_ButtonPress);
        XISetMask (mask1, XI_ButtonRelease);

        evmasks[0].deviceid = ctx->slavePtrDevId;
        evmasks[0].mask_len = sizeof (mask1);
        evmasks[0].mask     = mask1;

        XISelectEvents (ctx->xDpy, ctx->trgW, evmasks, 1);

        if (getXErrState (ctx) == True)
        {
            snprintf (buf, sizeof (buf), "\tfailed to select input events from"
                      " device \"%s\" for target window attributes!",
                      ctx->ptrDevName);
            logCtrl (buf, LOG_LVL_NO, False);
            return False;
        }
    }

    printWindowInfo (ctx, ctx->trgW, &ctx->trgWAttr);

    logCtrl ("\tsuccess", LOG_LVL_1, True);

    return True;
}

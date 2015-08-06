/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://raw.githubusercontent.com/awpe/xwinclone/master/LICENSE.md>.  */

#include <xwcWinUtil.h>

Bool
toggleHiddenState (XWCContext * ctx,
                   Window       win)
{
    XEvent event;
    Atom netWmSt, hiddenProperty;
    long mask;

    if (ctx == NULL)
    {
        logCtrl ("Cannot toggle window hidden state: NULL pointer to context"
                 " received!", LOG_LVL_NO, False);
        return False;
    }

    if (win == None)
    {
        logCtrl ("Cannot toggle window hidden state: Bad window id received!",
                 LOG_LVL_NO, False);
        return False;
    }

    memset (&event, 0, sizeof (event));

    mask = SubstructureRedirectMask | SubstructureNotifyMask;

    netWmSt        = XInternAtom (ctx->xDpy, "_NET_WM_STATE", False);
    hiddenProperty = XInternAtom (ctx->xDpy, "_NET_WM_STATE_HIDDEN", False);

    if (getXErrState (ctx) == True)
    {
        logCtrl ("Cannot toggle window hidden state: XInternAtom error!",
                 LOG_LVL_NO, False);
        return False;
    }

    event.xclient.type         = ClientMessage;
    event.xclient.serial       = 0;
    event.xclient.send_event   = True;
    event.xclient.message_type = netWmSt;
    event.xclient.window       = win;
    event.xclient.format       = 32;
    event.xclient.data.l[0]    = 2;
    event.xclient.data.l[1]    = hiddenProperty;
    event.xclient.data.l[2]    = 0;
    event.xclient.data.l[3]    = 0;
    event.xclient.data.l[4]    = 0;

    if (   XSendEvent (ctx->xDpy, ctx->rootW, False, mask, &event) == 0
        || getXErrState (ctx) == True)
    {
        logCtrl ("Cannot toggle window hidden state: XSendEvent error!",
                 LOG_LVL_NO, False);
        return False;
    }

    return True;
}

/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcWinUtil.h>

Bool
winRaiseCtrl (XWCContext * ctx,
              Window     * ioWin)
{
    Window currentTopWin;

    if (ctx == NULL || ioWin == NULL || * ioWin == None)
    {
        logCtrl ("Cannot raise window: bad input data!", LOG_LVL_NO, False);
        return False;
    }

    currentTopWin = getActiveWindow (ctx, None);

    if (currentTopWin == None)
    {
        logCtrl ("Cannot raise window: Cannot get active window!", LOG_LVL_NO,
                 False);
        return False;
    }

    if (currentTopWin != * ioWin)
    {
        if (raiseWin (ctx, *ioWin) == False)
        {
            logCtrl ("Cannot raise window: XRaiseWindow AND/OR XSetInputFocus "
                     "error!", LOG_LVL_NO, False);
            return False;
        }

        *ioWin = currentTopWin;

        if (ctx->raiseDelay.tv_nsec + ctx->raiseDelay.tv_sec != 0)
        {
            nanosleep (&ctx->raiseDelay, NULL);
        }
    }

    return True;
}

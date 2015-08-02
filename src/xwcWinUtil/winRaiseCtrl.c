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

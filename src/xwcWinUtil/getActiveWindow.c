#include <xwcWinUtil.h>

Window
getActiveWindow (XWCContext * ctx,
                 Window       implicitW)
{
    if (ctx == NULL)
    {
        logCtrl ("Error getting active window: No program options"
                 " data specified!", LOG_LVL_NO, False);
        return None;
    }

    if (ctx->xDpy == NULL)
    {
        logCtrl ("Error getting active window: No display specified!",
                 LOG_LVL_NO, False);
        return None;
    }

    if (implicitW == None)
    {
        implicitW = getFocusedWindow (ctx);
    }

    if (     implicitW                                           == None
        || ( implicitW = getTopWindow (ctx, implicitW)   ) == None
        || ( implicitW = getNamedWindow (ctx, implicitW) ) == None )
    {
        logCtrl ("Error getting active window: No active window found!",
                 LOG_LVL_NO, False);
        return None;
    }

    return implicitW;
}

#include <xwcWinUtil.h> 

Window
getNamedWindow (XWCContext * ctx,
                Window       start)
{
    Window w;
    char   buf[1024];

    if (ctx->xDpy == NULL)
    {
        logCtrl ("Error getting named window: No display specified!",
                 LOG_LVL_NO, False);
        return None;
    }

    if (start == None)
    {
        logCtrl ("Error getting named window: Invalid window specified!",
                 LOG_LVL_NO, False);
        return None;
    }

    logCtrl ("Getting named window:", LOG_LVL_1, False);

    w = XmuClientWindow (ctx->xDpy, start);
    if (w == start)
    {
        logCtrl ("\tfail to get named window or window already "
                 "has WM_STATE property", LOG_LVL_1, True);
    }

    snprintf (buf, sizeof (buf), "\twindow XID is %lX", w);
    logCtrl (buf, LOG_LVL_1, True);

    return w;
}

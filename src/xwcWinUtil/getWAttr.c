#include <xwcWinUtil.h>

Bool
getWAttr (XWCContext        * ctx,
          Window            * win,
          XWindowAttributes * wa)
{
    XGetWindowAttributes (ctx->xDpy, *win, wa);

    if (getXErrState (ctx) == True)
    {
        logCtrl ("Cannot get window attributes(getWAttr): ", LOG_LVL_NO, False);
        if (X_ERROR_CODE == BadWindow)
        {
            *win = None;
            logCtrl ("\tXGetWindowAttributes failed on given window,"
                     " maybe it was closed?", LOG_LVL_NO, True);
        }
        else
        {
            logCtrl ("\tXGetWindowAttributes error!", LOG_LVL_NO, True);
        }
        return False;
    }

    return True;
}

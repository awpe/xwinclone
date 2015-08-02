#include <xwcWinUtil.h>

Bool
raiseWin (XWCContext * ctx,
          Window       w)
{
    if (ctx == NULL || w == None)
    {
        return False;
    }

    XRaiseWindow (ctx->xDpy, w);

    XSetInputFocus (ctx->xDpy, w, RevertToParent, CurrentTime);

    return True;
}

#include <xwcDevCtrl.h>

Bool
mvPtr (XWCContext * ctx,
       int          x,
       int          y)
{
    Bool res;

    if (ctx == NULL)
    {
        logCtrl ("Cannot move pointer: ctx is NULL!", LOG_LVL_NO, False);
        return False;
    }

    res = XIWarpPointer (ctx->xDpy,
                         ctx->masterPtrDevId,
                         None,
                         ctx->rootW,
                         0.0,            0.0,
                         0,              0,
                         (double) x, (double) y);

    if (res != Success || getXErrState (ctx) == True)
    {
        logCtrl ("Cannot move pointer: XIWarpPointer error!", LOG_LVL_NO,
                 False);
        return False;
    }

    return True;
}

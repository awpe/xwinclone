#include <xwcXSrvUtil.h>

/*Global X_ERROR initialization*/
Bool X_ERROR      = False;
int  X_ERROR_CODE = 0;

Bool
getXErrState (XWCContext * ctx)
{
    XSync (ctx->xDpy, False);
    Bool res = X_ERROR;
    X_ERROR  = False;
    return res;
}

#include <xwcXSrvUtil.h>

Bool
openDefaultDisplay (XWCContext * ctx)
{
    logCtrl ("\tconnecting to X server", LOG_LVL_1, False);

    if (( ctx->xDpy = XOpenDisplay (NULL) ) == NULL)
    {
        logCtrl ("\t\tfail to connect to X server", LOG_LVL_NO, True);
        return False;
    }

    logCtrl ("\t\tsuccess", LOG_LVL_2, True);

    XSetErrorHandler (errorHandlerBasic);

    return True;
}

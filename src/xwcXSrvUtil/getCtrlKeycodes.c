#include <xwcXSrvUtil.h>

Bool
getCtrlKeycodes (XWCContext * ctx)
{
    KeySym exitKeySym;
    char   buf[1024];

    logCtrl ("\tgetting control keycodes", LOG_LVL_1, False);

    if (ctx == NULL)
    {
        logCtrl ("\t\tCannot get control keycodes: NULL pointer to program "
                 "context struct received!", LOG_LVL_NO, False);
        return False;
    }

    if (( exitKeySym = XStringToKeysym (ctx->exitKeyStr) ) == NoSymbol)
    {
        snprintf (buf, sizeof (buf), "\t\tError parsing exit key string (%s)",
                  ctx->exitKeyStr);
        logCtrl (buf, LOG_LVL_NO, True);
        return False;
    }

    if ((ctx->exitKeyCode = XKeysymToKeycode (ctx->xDpy, exitKeySym)) == 0)
    {
        snprintf (buf, sizeof (buf), "\t\tUnknown keycode %d",
                  ctx->exitKeyCode);
        logCtrl (buf, LOG_LVL_NO, True);
        return False;
    }

    if (( exitKeySym = XStringToKeysym (ctx->transCtrlKeyStr) )
        == NoSymbol)
    {
        snprintf (buf, sizeof (buf), "\t\tError parsing exit key string (%s)",
                  ctx->transCtrlKeyStr);
        logCtrl (buf, LOG_LVL_NO, True);
        return False;
    }

    if (( ctx->cloneKeyCode = XKeysymToKeycode (ctx->xDpy, exitKeySym) ) == 0)
    {
        snprintf (buf, sizeof (buf), "\t\tUnknown keycode %d",
                  ctx->cloneKeyCode);
        logCtrl (buf, LOG_LVL_NO, True);
        return False;
    }

    logCtrl ("\t\tsuccess", LOG_LVL_2, True);

    return True;
}

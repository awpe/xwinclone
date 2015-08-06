/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

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

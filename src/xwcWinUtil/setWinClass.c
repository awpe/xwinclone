/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://raw.githubusercontent.com/awpe/xwinclone/master/LICENSE.md>.  */

#include <xwcWinUtil.h>

Bool
setWinClass (XWCContext * ctx,
             Window       WID,
             const char * permNameStr,
             const char * classStr)
{
    XClassHint * xClassHint;

    if (ctx->xDpy == NULL)
    {
        logCtrl ("Error changing window class: No display specified!" ,
                 LOG_LVL_NO, False);
        return False;
    }

    if (permNameStr == NULL)
    {
        logCtrl ("Error changing window class: Window permanent name string "
                 "is not specified!", LOG_LVL_NO, False);
        return False;
    }

    if (classStr == NULL)
    {
        logCtrl ("Error changing window class: Class string is not "
                 "specified!", LOG_LVL_NO, False);
        return False;
    }

    if (WID == None)
    {
        logCtrl ("Error changing window class: No window specified!",
                 LOG_LVL_NO, False);
        return False;
    }

    logCtrl ("\tSetting window class strings:", LOG_LVL_1, False);

    if (   ( xClassHint = XAllocClassHint () ) == NULL
        || getXErrState (ctx) == True)
    {
        if (xClassHint != NULL)
        {
            XFree (xClassHint);
        }
        logCtrl ("\t\terror setting window class strings!", LOG_LVL_NO, True);
        return False;
    }

    /* As of Xlib version 1.6.3 XSetClassHint() doesn't affect res_class and
     * res_name fields of xClassHint structure so let this field be treated as
     * constant...*/
    xClassHint->res_class = (char*) classStr;
    xClassHint->res_name  = (char*) permNameStr;

    XSetClassHint (ctx->xDpy, WID, xClassHint);

    xClassHint->res_class = NULL;
    xClassHint->res_name  = NULL;

    XFree (xClassHint);

    if (getXErrState (ctx) == False)
    {
        logCtrl ("\t\tSuccess", LOG_LVL_1, True);
        return True;
    }

    return False;
}

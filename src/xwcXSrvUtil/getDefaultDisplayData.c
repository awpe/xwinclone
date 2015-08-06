/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcXSrvUtil.h>

Bool
getDefaultDisplayData (XWCContext * ctx)
{
    logCtrl ("\tTrying to get default display data: root window, default "
             "screen", LOG_LVL_1, False);

    if (ctx == NULL)
    {
        logCtrl ("\t\tError getting default display data: NULL pointer to "
                 "program context received", LOG_LVL_NO, True);
        return False;
    }

    ctx->rootW = DefaultRootWindow (ctx->xDpy);

    if (ctx->rootW == None)
    {
        logCtrl ("\t\tCannot get default root window of display!",
                 LOG_LVL_NO, False);
        return False;
    }

    XGetWindowAttributes (ctx->xDpy, ctx->rootW, &ctx->rootWAttr);

    if (getXErrState (ctx) == True)
    {
        logCtrl ("\t\tCannot get default root window attributes!",
                 LOG_LVL_NO, False);
        return False;
    }

    if (ctx->xDpy == NULL)
    {
        logCtrl ("\t\tError getting screen by window attributes: "
                 "No display specified", LOG_LVL_NO, True);
        return False;
    }

    if (ctx->rootWAttr.screen == NULL)
    {
        logCtrl ("\t\tError getting default screen: No valid screen pointer "
                 "found in root window's attributes struct", LOG_LVL_NO, True);
        return False;
    }

    ctx->xScr = ctx->rootWAttr.screen;

    logCtrl ("\t\tsuccess", LOG_LVL_2, True);

    return True;
}

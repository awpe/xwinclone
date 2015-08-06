/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

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

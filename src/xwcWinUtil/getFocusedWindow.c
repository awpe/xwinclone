/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://raw.githubusercontent.com/awpe/xwinclone/master/LICENSE.md>.  */

#include <xwcWinUtil.h> 

Window
getFocusedWindow (XWCContext * ctx)
{
    if (ctx == NULL)
    {
        logCtrl ("Error getting focused window: NULL pointer to context"
                 " received!", LOG_LVL_NO, False);
        return None;
    }

    /**
     * @todo find how to utilize revert_to
     */
    int    revert_to;
    Window w;
    char   buf[1024];

    logCtrl ("Searching for focused window:", LOG_LVL_1, False);

    XGetInputFocus (ctx->xDpy, &w, &revert_to);

    if (getXErrState (ctx) == True)
    {
        logCtrl ("\tfail to get focused window", LOG_LVL_NO, True);
        return None;
    }

    if (w == None)
    {
        logCtrl ("\tno focus window", LOG_LVL_NO, True);
    }

    else
    {
        snprintf (buf, sizeof (buf), "\tsuccess, window XID is %lX", w);
        logCtrl (buf, LOG_LVL_1, True);
    }

    return w;
}

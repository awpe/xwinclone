/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://raw.githubusercontent.com/awpe/xwinclone/master/LICENSE.md>.  */

#include <xwcWinUtil.h>

Window
getActiveWindow (XWCContext * ctx,
                 Window       implicitW)
{
    if (ctx == NULL)
    {
        logCtrl ("Error getting active window: No program options"
                 " data specified!", LOG_LVL_NO, False);
        return None;
    }

    if (ctx->xDpy == NULL)
    {
        logCtrl ("Error getting active window: No display specified!",
                 LOG_LVL_NO, False);
        return None;
    }

    if (implicitW == None)
    {
        implicitW = getFocusedWindow (ctx);
    }

    if (     implicitW                                           == None
        || ( implicitW = getTopWindow (ctx, implicitW)   ) == None
        || ( implicitW = getNamedWindow (ctx, implicitW) ) == None )
    {
        logCtrl ("Error getting active window: No active window found!",
                 LOG_LVL_NO, False);
        return None;
    }

    return implicitW;
}

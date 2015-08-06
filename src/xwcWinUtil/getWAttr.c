/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcWinUtil.h>

Bool
getWAttr (XWCContext        * ctx,
          Window            * win,
          XWindowAttributes * wa)
{
    XGetWindowAttributes (ctx->xDpy, *win, wa);

    if (getXErrState (ctx) == True)
    {
        logCtrl ("Cannot get window attributes(getWAttr): ", LOG_LVL_NO, False);
        if (X_ERROR_CODE == BadWindow)
        {
            *win = None;
            logCtrl ("\tXGetWindowAttributes failed on given window,"
                     " maybe it was closed?", LOG_LVL_NO, True);
        }
        else
        {
            logCtrl ("\tXGetWindowAttributes error!", LOG_LVL_NO, True);
        }
        return False;
    }

    return True;
}

/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcWinUtil.h>

void
printWindowClass (XWCContext * ctx,
                  Window       w)
{
    XClassHint * class;
    char         buf[1024];

    if (ctx->xDpy == NULL)
    {
        logCtrl ("Error printing window class: No display specified!",
                 LOG_LVL_NO, False);
        return;
    }

    if (w == None)
    {
        logCtrl ("Error printing window class: Invalid window specified!",
                 LOG_LVL_NO, False);
        return;
    }

    class = XAllocClassHint ();

    if (getXErrState (ctx) == True)
    {
        logCtrl ("Error printing window class info: XAllocClassHint err",
                 LOG_LVL_NO, False);
        if (class != NULL)
        {
            XFree (class);
        }
        return;
    }

    if (getXErrState (ctx) == False && XGetClassHint (ctx->xDpy, w, class) != 0)
    {
        snprintf (buf, sizeof (buf), "application:\n\tname:\t%s\n\tclass:\t%s",
                  class->res_name, class->res_class);

        logCtrl (buf, LOG_LVL_1, False);
    }
    else
    {
        logCtrl ("Error printing window class info: XGetClassHint err",
                 LOG_LVL_NO, False);
    }

    if (class != NULL)
    {
        if (class->res_class != NULL)
        {
            XFree (class->res_class);
        }

        if (class->res_name != NULL)
        {
            XFree (class->res_name);
        }

        XFree (class);
    }
}

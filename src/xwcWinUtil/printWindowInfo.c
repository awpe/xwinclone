/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcWinUtil.h>

void
printWindowInfo (XWCContext        * ctx,
                 Window              w,
                 XWindowAttributes * xWinAttr)
{

    char buf[1024];

    if (ctx == NULL)
    {
        logCtrl ("Error cannot print window information: "
                 "Bad context pointer", LOG_LVL_NO, False);
        return;
    }

    if (w == None)
    {
        logCtrl ("Error cannot print window information: No window specified",
                 LOG_LVL_NO, False);
        return;
    }

    if (xWinAttr == NULL)
    {
        logCtrl ("Error cannot print window information: "
                 "No window attributes data specified", LOG_LVL_NO, False);
        return;
    }

    printWindowName (ctx, w);
    printWindowClass (ctx, w);

    snprintf (buf, sizeof (buf), "Window info:\n\tWidth:\t%d\n\tHeight:\t%d\n\t"
              "Depth:\t%d",
              xWinAttr->width, xWinAttr->height, xWinAttr->depth);

    logCtrl (buf, LOG_LVL_1, False);
}

/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://raw.githubusercontent.com/awpe/xwinclone/master/LICENSE.md>.  */

#include <xwcWinUtil.h> 

Window
getTopWindow (XWCContext * ctx,
              Window       start)
{
    Window         w;
    Window         parent;
    Window         root;
    Window       * children;
    unsigned int   nchildren;
    char           buf[1024];

    w         = start;
    parent    = start;
    root      = None;

    if (ctx->xDpy == NULL)
    {
        logCtrl ("Error getting top window: No display specified!",
                 LOG_LVL_NO, False);
        return None;
    }

    if (start == None)
    {
        logCtrl ("Error getting top window: Invalid window specified!",
                 LOG_LVL_NO, False);
        return None;
    }

    logCtrl ("getting child-of-root window:", LOG_LVL_1, False);

    while (parent != root)
    {
        w = parent;

        if (XQueryTree (ctx->xDpy, w, &root, &parent, &children, &nchildren))
        {
            if (children != NULL)
            {
                XFree (children);
            }
        }

        if (getXErrState (ctx) == True)
        {
            logCtrl ("\tfailed to get top-most window\n", LOG_LVL_NO, True);
            return None;
        }
        snprintf (buf, sizeof (buf), "\tgot parent, window XID is %lX", w);
        logCtrl (buf, LOG_LVL_1, True);
    }

    snprintf (buf, sizeof (buf), "\tsuccess, window XID is %lX", w);
    logCtrl (buf, LOG_LVL_1, True);

    return w;
}

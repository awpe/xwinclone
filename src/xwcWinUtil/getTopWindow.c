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

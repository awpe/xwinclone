#include <xwcUtil.h>

Bool
getUserDir (XWCContext * ctx)
{
    char buf[1024];

    if ((ctx->userDir = getenv ("HOME")) == NULL)
    {
        ctx->userDir = getpwuid (getuid ())->pw_dir;
    }

    if (ctx->userDir == NULL)
    {
        snprintf (buf, sizeof (buf), "Cannot determine user directory to "
                  "search for config file '%s'!", ctx->confFileName);
        logCtrl (buf, LOG_LVL_1, True);
        //        XCloseDisplay (ctx->xDpy);
        //        freeXWCContext (ctx);
        //        delArgs (args);
        //        return NULL;
    }
    return True;
}

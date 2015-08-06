/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://raw.githubusercontent.com/awpe/xwinclone/master/LICENSE.md>.  */

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

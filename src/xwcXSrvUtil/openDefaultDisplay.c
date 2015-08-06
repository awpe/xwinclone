/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcXSrvUtil.h>

Bool
openDefaultDisplay (XWCContext * ctx)
{
    logCtrl ("\tconnecting to X server", LOG_LVL_1, False);

    if (( ctx->xDpy = XOpenDisplay (NULL) ) == NULL)
    {
        logCtrl ("\t\tfail to connect to X server", LOG_LVL_NO, True);
        return False;
    }

    logCtrl ("\t\tsuccess", LOG_LVL_2, True);

    XSetErrorHandler (errorHandlerBasic);

    return True;
}

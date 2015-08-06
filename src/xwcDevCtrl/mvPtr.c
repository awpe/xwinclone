/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://raw.githubusercontent.com/awpe/xwinclone/master/LICENSE.md>.  */

#include <xwcDevCtrl.h>

Bool
mvPtr (XWCContext * ctx,
       int          x,
       int          y)
{
    Bool res;

    if (ctx == NULL)
    {
        logCtrl ("Cannot move pointer: ctx is NULL!", LOG_LVL_NO, False);
        return False;
    }

    res = XIWarpPointer (ctx->xDpy,
                         ctx->masterPtrDevId,
                         None,
                         ctx->rootW,
                         0.0,            0.0,
                         0,              0,
                         (double) x, (double) y);

    if (res != Success || getXErrState (ctx) == True)
    {
        logCtrl ("Cannot move pointer: XIWarpPointer error!", LOG_LVL_NO,
                 False);
        return False;
    }

    return True;
}

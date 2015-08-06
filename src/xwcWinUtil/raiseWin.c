/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://raw.githubusercontent.com/awpe/xwinclone/master/LICENSE.md>.  */

#include <xwcWinUtil.h>

Bool
raiseWin (XWCContext * ctx,
          Window       w)
{
    if (ctx == NULL || w == None)
    {
        return False;
    }

    XRaiseWindow (ctx->xDpy, w);

    XSetInputFocus (ctx->xDpy, w, RevertToParent, CurrentTime);

    return True;
}

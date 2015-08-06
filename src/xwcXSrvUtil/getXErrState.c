/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://raw.githubusercontent.com/awpe/xwinclone/master/LICENSE.md>.  */

#include <xwcXSrvUtil.h>

/*Global X_ERROR initialization*/
Bool X_ERROR      = False;
int  X_ERROR_CODE = 0;

Bool
getXErrState (XWCContext * ctx)
{
    XSync (ctx->xDpy, False);
    Bool res = X_ERROR;
    X_ERROR  = False;
    return res;
}

/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcXSrvUtil.h>

Bool
chkCompExt (Display * d)
{
    int  compositeErrorBase, compositeEventBase, xCompExtVerMin, xCompExtVerMaj;
    char buf[1024];

    logCtrl ("\tChecking composite extension", LOG_LVL_1, False);

    if (XCompositeQueryExtension (d, &compositeEventBase,
                                  &compositeErrorBase) == False)
    {
        logCtrl ("\t\tNo composite extension found, aborting!", LOG_LVL_NO,
                 True);
        return False;
    }
    else if (! XCompositeQueryVersion (d, &xCompExtVerMaj, &xCompExtVerMin))
    {
        logCtrl ("\t\tX Server doesn't support such a version of the X "
                 "Composite Extension which is compatible with the client "
                 "library", LOG_LVL_NO, True);
        return False;
    }
    else if ( ( xCompExtVerMaj < 1 ) &&  ( xCompExtVerMin < 2 ) )
    {
        logCtrl ("\t\tUnsupported version of X composite extension (<0.2)",
                 LOG_LVL_NO, True);
        return False;
    }
    else
    {
        snprintf (buf, sizeof (buf), "\t\tversion %d.%d",
                  xCompExtVerMaj, xCompExtVerMin);
        logCtrl (buf, LOG_LVL_2, True);
    }

    logCtrl ("\t\tSuccess", LOG_LVL_2, True);
    return True;
}

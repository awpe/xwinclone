/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcWinUtil.h>

unsigned char *
getWPropertyByAtom (XWCContext * ctx,
                    Window       window,
                    Atom         atom,
                    long       * nitems,
                    Atom       * type,
                    int        * size)
{
    /*From documenetation examples*/
    Atom            actual_type;
    int             actual_format;
    unsigned long   nitems_;
    unsigned long   bytes_after;
    unsigned char * prop;
    int             status;
    char            buf[1024];

    status = XGetWindowProperty (ctx->xDpy, window, atom, 0, (~ 0L),
                                 False, AnyPropertyType, &actual_type,
                                 &actual_format, &nitems_, &bytes_after,
                                 &prop);

    if (getXErrState (ctx) == True)
    {
        snprintf (buf, sizeof (buf), "Cannot get window property by atom!");
        logCtrl (buf, LOG_LVL_NO, False);
        return NULL;
    }

    if (status != Success)
    {
        logCtrl ("XGetWindowProperty failed!", LOG_LVL_NO, False);
        return NULL;
    }

    if (nitems != NULL)
    {
        *nitems = nitems_;
    }

    if (type != NULL)
    {
        *type = actual_type;
    }

    if (size != NULL)
    {
        *size = actual_format;
    }

    return prop;
}

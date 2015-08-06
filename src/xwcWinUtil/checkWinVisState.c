/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcWinUtil.h>

triState
checkWinVisState (XWCContext * ctx,
                  Window       checkW)
{
    XWindowAttributes   xwa;
    Bool                hiddenPropertySet;
#if (ALLOW_NON_EWMH_COMPILANT_WM == 0)
    Atom                netWmState, actualType, netWmStHidden;
    int                 actualFormat, r;
    unsigned long       itemCnt, leftover;
    unsigned char     * data;
#endif    

    if (ctx == NULL)
    {
        logCtrl ("Cannot check window visibility: "
                 "context pointer is NULL!", LOG_LVL_NO, False);
        return UNDEFINED;
    }

    if (checkW == None)
    {
        logCtrl ("Cannot check window visibility: "
                 "window id is None!", LOG_LVL_NO, False);
        return UNDEFINED;
    }

    XGetWindowAttributes (ctx->xDpy, checkW, &xwa);

    if (getXErrState (ctx) == True)
    {
        logCtrl ("Cannot check window visibility: "
                 "XGetWindowAttributes error!", LOG_LVL_NO, False);
        return UNDEFINED;
    }

    hiddenPropertySet = False;

#if (ALLOW_NON_EWMH_COMPILANT_WM == 0)

    netWmState    = XInternAtom (ctx->xDpy, "_NET_WM_STATE", True);
    netWmStHidden = XInternAtom (ctx->xDpy, "_NET_WM_STATE_HIDDEN", False);

    if (   netWmState         == None
        || netWmStHidden      == None
        || getXErrState (ctx) == True)
    {
        logCtrl ("Cannot check window visibility: "
                 "XInternAtom error!", LOG_LVL_NO, False);
        return UNDEFINED;
    }

    data = NULL;

    r = XGetWindowProperty (ctx->xDpy, ctx->srcW, netWmState, 0L, (long) BUFSIZ,
                            False, XA_ATOM, &actualType, &actualFormat,
                            &itemCnt, &leftover, &data);

    if (getXErrState (ctx) == True || r != Success)
    {
        logCtrl ("Cannot check window visibility: "
                 "XGetWindowProperty error!", LOG_LVL_NO, False);
        if (data != NULL)
        {
            XFree (data);
        }
        return UNDEFINED;
    }

    printf ("actualType = %ld\n", actualType);

    if (actualType == None || actualType != XA_ATOM)
    {
        logCtrl ("Cannot check window visibility: XGetWindowProperty error OR"
                 " current Window manager is not EWMH compilant!", LOG_LVL_NO,
                 False);
        if (data != NULL)
        {
            XFree (data);
        }
        return UNDEFINED;
    }

    for (unsigned long i = 0; i < itemCnt; ++ i)
    {
        if (netWmStHidden == ((unsigned long *) (data))[i])
        {
            hiddenPropertySet = True;
            break;
        }
    }

    if (data != NULL)
    {
        XFree (data);
    }

#endif    

    return (   (((xwa.map_state == IsViewable) ? True : False) == True)
        && (hiddenPropertySet == False) ) ? True : False;
}

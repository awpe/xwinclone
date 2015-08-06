/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcDevCtrl.h>

Bool
getTrgWPtrData (XWCContext      * ctx,
                int             * trgX,
                int             * trgY,
                XIModifierState * modsSt,
                int             * trgRX,
                int             * trgRY)
{
    XIButtonState btnSt;
    XIGroupState  grpSt;
    Window        rootRet, childRet;
    Bool          res;
    double        rX, rY, wX, wY;

    /*simple check*/
    /*TODO change to asserts*/
    if (ctx == NULL || trgX == NULL || trgY == NULL || modsSt == NULL)
    {
        logCtrl ("Cannot get target window pointer data: "
                 "NULL pointer received!", LOG_LVL_NO, True);
        return False;
    }

    memset (&btnSt, 0, sizeof (btnSt));
    btnSt.mask = NULL;

    res = XIQueryPointer (ctx->xDpy, ctx->masterPtrDevId, ctx->trgW,
                          &rootRet, &childRet, &rX, &rY, &wX,
                          &wY, &btnSt, modsSt, &grpSt);

    /*free btnSt.mask after XIQueryPointer call*/
    if (btnSt.mask != NULL)
    {
        free (btnSt.mask);
    }

    if (res == False || getXErrState (ctx) == True)
    {
        logCtrl ("Cannot get target window pointer data: "
                 "XIQueryPointer error!", LOG_LVL_NO, True);
        return False;
    }

    *trgX  = (int) (wX + 0.5);
    *trgY  = (int) (wY + 0.5);
    *trgRX = (int) (rX + 0.5);
    *trgRY = (int) (rY + 0.5);

    return True;
}

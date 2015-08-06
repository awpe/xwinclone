/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcDevCtrl.h>

Bool
getBtnEv (XWCContext      * ctx,
          XIModifierState * modsSt,
          XEvent          * xev,
          int               adjX,
          int               adjY,
          int               adjRX,
          int               adjRY)
{
    if (ctx == NULL || modsSt == NULL || xev == NULL)
    {
        logCtrl ("Cannot prepare event structure: NULL pointer received!",
                 LOG_LVL_NO, False);
        return False;
    }

    memset (xev, 0, sizeof (*xev));

    xev->xbutton.serial      = 0;
    xev->xbutton.send_event  = True;
    xev->xbutton.display     = ctx->xDpy;
    xev->xbutton.window      = ctx->srcW;
    xev->xbutton.root        = ctx->rootW;
    xev->xbutton.subwindow   = None;
    xev->xbutton.time        = CurrentTime;
    xev->xbutton.x           = adjX;
    xev->xbutton.y           = adjY;
    xev->xbutton.x_root      = adjRX;
    xev->xbutton.y_root      = adjRY;
    xev->xbutton.state       = modsSt->effective;
    xev->xbutton.button      = TRACKED_BUTTON - 1;
    xev->xbutton.same_screen = True;

    return True;
}

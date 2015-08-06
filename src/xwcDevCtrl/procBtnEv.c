/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcDevCtrl.h>

int
procBtnEv (XWCContext    * ctx,
           XIDeviceEvent * xide)
{
    XIModifierState modSt;
    XEvent          btnEv;
    Window          focused;
    triState        visRes;
    int             trgRX, trgRY, adjRX, adjRY, adjX, adjY, trgX, trgY;
    char            buf[1024];

    logCtrl ("Got click:", LOG_LVL_2, False);

    if (ctx == NULL)
    {
        logCtrl ("Cannot process pressed button event: ctx is NULL!",
                 LOG_LVL_NO, True);
        return EXIT_COMBINATION;
    }

    if (xide == NULL)
    {
        logCtrl ("Very bad error that should have never happened: "
                 "XGetEventData failed completely at procBtnEv!", LOG_LVL_NO,
                 True);
        return EXIT_COMBINATION;
    }

    if (xide->buttons.mask == NULL)
    {
        logCtrl ("Very bad error that should have never happened: "
                 "xide->buttons.mask surprisingly is NULL at procBtnEv!",
                 LOG_LVL_NO, True);
        return EXIT_COMBINATION;
    }

    snprintf (buf, sizeof (buf), "btn = %d; tracked = %d\n",
              xide->buttons.mask[0], 1 << (TRACKED_BUTTON - 1));
    logCtrl (buf, LOG_LVL_2, True);

    /* button is expressed as 1 shifted to the left by button number,
     * XIButtonState structure has an array of bytes to store pressed 
     * button number, but buttons we track will always fit into 
     * 1 byte. If you need to track buttons with numbers greater
     * than 7 then change next code to check all bytes */
    if (xide->buttons.mask[0] == 1 << (TRACKED_BUTTON - 1))
    {
        /**********************************************************************/
        /*check source window state*/
        /**********************************************************************/
        visRes = checkWinVisState (ctx, ctx->srcW);

        if (visRes == False)
        {
            logCtrl ("Source seems to be hidden", LOG_LVL_2, True);

            if (toggleHiddenState (ctx, ctx->srcW) == False )
            {
                logCtrl ("Cannot process pressed button event: "
                         "toggleHiddenState error!", LOG_LVL_NO, True);
                return EXIT_COMBINATION;
            }

            if (ctx->restoreDelay.tv_nsec + ctx->restoreDelay.tv_sec != 0)
            {
                nanosleep (&ctx->restoreDelay, NULL);
            }

            visRes = checkWinVisState (ctx, ctx->srcW);

            if (visRes == False)
            {
                logCtrl ("Cannot process pressed button event: problem occured"
                         " while restoring hidden window, if last is true, "
                         "check if source window was on visible(active) "
                         "desktop!", LOG_LVL_NO, True);
                return EXIT_COMBINATION;
            }
        }

        if (visRes == UNDEFINED)
        {
            logCtrl ("Error getting source window visibility state: "
                     "isWinVis error!", LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*Get pointer coordinates and modifiers state in target window*/
        /**********************************************************************/
        if (getTrgWPtrData (ctx, &trgX, &trgY, &modSt, &trgRX, &trgRY) == False)
        {
            logCtrl ("Cannot process pressed button event: "
                     "getTrgWPtrData error!", LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*Adjust pointer coordinates for source window*/
        /**********************************************************************/
        if (adjPtrLoc (ctx, trgX, trgY, &adjX, &adjY, &adjRX, &adjRY) == False)
        {
            logCtrl ("Cannot process pressed button event: adjPtrLoc error!",
                     LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*raise source if needed*/
        /**********************************************************************/
        focused = ctx->srcW;

        if (winRaiseCtrl (ctx, &focused) == False)
        {
            logCtrl ("Cannot process pressed button event: wRaiseCtrl error!",
                     LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*move pointer to source window*/
        /**********************************************************************/
        if (mvPtr (ctx, adjRX, adjRY) == False)
        {
            logCtrl ("Cannot process pressed button event: mvPtr error!",
                     LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        else
        {
            logCtrl ("Pointer moved to source\n", LOG_LVL_2, True);
        }
        /**********************************************************************/


        /**********************************************************************/
        /*prepare event structure*/
        /**********************************************************************/
        if (getBtnEv (ctx, &modSt, &btnEv, adjX, adjY, adjRX, adjRY) == False)
        {
            logCtrl ("Cannot process pressed button event: getBtnEv"
                     " error!", LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*emulate click*/
        /**********************************************************************/
        if (btnClick (ctx, &btnEv) == False)
        {
            logCtrl ("Cannot process pressed button event: btnClick error!",
                     LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        /**********************************************************************/


        /**********************************************************************/
        /*move pointer to target window*/
        /**********************************************************************/
        if (mvPtr (ctx, trgRX, trgRY) == False)
        {
            logCtrl ("Cannot process pressed button event: mvPtr error!",
                     LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        else
        {
            logCtrl ("Pointer moved to target\n", LOG_LVL_2, True);
        }
        /**********************************************************************/


        /**********************************************************************/
        /*raise last focused if needed*/
        /**********************************************************************/
        if (winRaiseCtrl (ctx, &focused) == False)
        {
            logCtrl ("Cannot process pressed button event: wRaiseCtrl error!",
                     LOG_LVL_NO, True);
            return EXIT_COMBINATION;
        }
        /**********************************************************************/
    }

    return SKIP_OTHER_EVENTS;
}

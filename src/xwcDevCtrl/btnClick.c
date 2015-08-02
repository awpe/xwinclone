#include <xwcDevCtrl.h>

Bool
btnClick (XWCContext * ctx,
          XEvent     * btnEv)
{
    Bool res;

    if (ctx == NULL || btnEv == NULL)
    {
        logCtrl ("Cannot emulate click: "
                 "NULL pointer received!", LOG_LVL_NO, True);
        return False;
    }

    /**************************************************************************/
    /*Send button press event*/
    /**************************************************************************/
    btnEv->xbutton.type = ButtonPress;
    res = XSendEvent (ctx->xDpy, ctx->srcW, False,
                      ButtonPressMask, btnEv);

    if (res == 0 || getXErrState (ctx) == True)
    {
        logCtrl ("Cannot emulate click: XSendEvent error!", LOG_LVL_NO, True);
        return False;
    }
    else
    {
        logCtrl ("Button press event sent\n", LOG_LVL_2, True);
    }
    /**************************************************************************/


    if (ctx->clickDelay.tv_nsec + ctx->clickDelay.tv_sec != 0)
    {
        nanosleep (&ctx->clickDelay, NULL);
    }


    /**************************************************************************/
    /*Send button release event*/
    /**************************************************************************/
    btnEv->xbutton.type = ButtonRelease;
    res = XSendEvent (ctx->xDpy, ctx->srcW, False,
                      ButtonReleaseMask, btnEv);

    if (res == 0 || getXErrState (ctx) == True)
    {
        logCtrl ("Cannot emulate click: XSendEvent error!", LOG_LVL_NO, True);
        return False;
    }
    else
    {
        logCtrl ("Button release event sent\n", LOG_LVL_2, True);
    }
    /**************************************************************************/

    return True;
}

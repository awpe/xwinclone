#include <xwcDevCtrl.h>

int
getPressedComb (XWCContext * ctx)
{
    XEvent                xEvent;
    XGenericEventCookie * cookie;
    int                   retVal;

    retVal = NO_KEY_PRESSED;

    while (XPending (ctx->xDpy) != 0)
    {
        XNextEvent (ctx->xDpy, &xEvent);

        if (retVal != NO_KEY_PRESSED)
        {
            /* if some event has been processed (so retVal has changed), skip
             * all other events for this client in X server queue */
            continue;
        }

        cookie = & xEvent.xcookie;

        if (   cookie->type                      != GenericEvent
            || cookie->extension                 != ctx->xiOp
            || XGetEventData (ctx->xDpy, cookie) == False)
        {
            continue;
        }

        logCtrl ("Got generic event", LOG_LVL_2, False);

        if (   cookie->evtype == XI_KeyPress
            || cookie->evtype == XI_ButtonPress)
        {
            /*free cookie after XGetEventData call*/
            XFreeEventData (ctx->xDpy, cookie);
            /*skip press events*/
            continue;
        }

        /* according to documentation for XI_KeyRelease and XI_ButtonRelease 
         * event types there is XIDeviceEvent data structure 
         * in the data field of cookie structure*/
        switch (cookie->evtype)
        {
            case XI_KeyRelease:
                retVal = procKeySeqEv (ctx, (XIDeviceEvent*) cookie->data);
                break;

            case XI_ButtonRelease:
                if (ctx->procBtnEv == True)
                {
                    retVal = procBtnEv (ctx, (XIDeviceEvent*) cookie->data);
                }
                break;

            default:
                break;
        }

        /*free cookie after XGetEventData call*/
        XFreeEventData (ctx->xDpy, cookie);
    }

    if (retVal == SKIP_OTHER_EVENTS)
    {
        retVal = NO_KEY_PRESSED;
    }

    return retVal;
}

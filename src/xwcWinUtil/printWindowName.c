#include <xwcWinUtil.h>

void
printWindowName (XWCContext * ctx,
                 Window       w)
{
    XTextProperty    prop;
    int              count, result, i;
    char          ** list;
    char             buf[1024];

    if (ctx->xDpy == NULL)
    {
        logCtrl ("Error printing window name: No display specified!",
                 LOG_LVL_NO, False);
        return;
    }

    if (w == None)
    {
        logCtrl ("Error printing window name: Invalid window specified!",
                 LOG_LVL_NO, False);
        return;
    }

    memset (&prop, 0, sizeof (prop ));
    prop.value = NULL;

    logCtrl ("window name:", LOG_LVL_1, False);

    if (getXErrState (ctx) == False && XGetWMName (ctx->xDpy, w, &prop) != 0)
    {
        count  = 0;
        list   = NULL;
        result = XmbTextPropertyToTextList (ctx->xDpy, &prop, &list, &count);

        if (result == Success)
        {
            snprintf (buf, 1024, "\t%s", list[0]);
            logCtrl (buf, LOG_LVL_1, True);
        }
        else
        {
            logCtrl ("\tError printing window name: "
                     "XmbTextPropertyToTextList err", LOG_LVL_NO, True);
        }
        if (list != NULL)
        {
            for (i = 0; i < count; ++ i)
            {
                if (list[i] != NULL)
                {
                    XFree (list[i]);
                }
            }
            XFree (list);
        }
    }
    else
    {
        logCtrl ("\tError printing window name: XGetWMName err", LOG_LVL_NO,
                 True);
    }

    if (prop.value != NULL)
    {
        XFree (prop.value);
    }
}

#include <xwcWinUtil.h>

Bool
setWinTitlebar (XWCContext * ctx,
                Window       WID,
                const char * name)
{
    XTextProperty tp;

    if (ctx->xDpy == NULL)
    {
        logCtrl ("Error changing window name: No display specified!",
                 LOG_LVL_NO, False);
        return False;
    }

    if (name == NULL)
    {
        logCtrl ("Error changing window name: Name string is not specified!",
                 LOG_LVL_NO, False);
        return False;
    }

    if (WID == None)
    {
        logCtrl ("Error changing window name: No window specified!\n",
                 LOG_LVL_NO, False);
        return False;
    }

    logCtrl ("\tSetting window titlebar text:", LOG_LVL_1, False);

    memset (&tp, 0, sizeof (tp ));
    tp.value = NULL;

    /* As of Xlib version 1.6.3 XStringListToTextProperty doesn't affect its 
     * argv (first char**) argument (Xlib's SetTxtProp.c), 
     * so let's think it is const...*/
    if (XStringListToTextProperty ((char **) &name, 1, &tp) == False)
    {
        logCtrl ("\t\tError setting name of window: "
                 "XStringListToTextProperty err", LOG_LVL_NO, True);
        return False;
    }

    XChangeProperty (ctx->xDpy, WID, XA_WM_NAME, tp.encoding, tp.format,
                     PropModeReplace, tp.value, tp.nitems);
    /*
    free (a);
     */
    if (tp.value != NULL)
    {
        XFree (tp.value);
    }

    if (getXErrState (ctx) == True)
    {
        logCtrl ("\t\tError setting name of window: "
                 "XChangeProperty err", LOG_LVL_NO, True);
        return False;
    }

    logCtrl ("\t\tSuccess", LOG_LVL_1, True);

    return True;
}

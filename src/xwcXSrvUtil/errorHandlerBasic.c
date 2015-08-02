#include <xwcXSrvUtil.h>

int
errorHandlerBasic (Display     * display,
                   XErrorEvent * error)
{
    if (display == NULL)
    {
        /*do something*/
        return 1;
    }

    if (error == NULL)
    {
        /*do something*/
        return 1;
    }

    char buf[1024];

    snprintf (buf, sizeof (buf), "ERROR: X11 error\n\terror code: %d",
              error->error_code);

    logCtrl (buf, LOG_LVL_NO, False);

    XGetErrorText (display, error->error_code, buf + 1, 1024);
    buf[0] = '\t';
    logCtrl (buf, LOG_LVL_NO, True);

    X_ERROR      = True;
    X_ERROR_CODE = error->error_code;
    return 1;
}

#include <xwcXSrvUtil.h>

Bool
chckXI2Ext (XWCContext * ctx)
{
    int event, error, major, minor, rc, majorSupported, minorSupported;
    char buf[1024];

    logCtrl ("\tChecking XInput 2 extension", LOG_LVL_1, False);

    if (ctx == NULL)
    {
        logCtrl ("\t\tCannot check XInput 2 extension: NULL pointer to XWC "
                 "context received.", LOG_LVL_NO, True);
        return False;
    }

    if (XQueryExtension (ctx->xDpy, "XInputExtension", &ctx->xiOp, &event,
                         &error) == 0)
    {
        logCtrl ("\t\tX Input extension not available.", LOG_LVL_NO, True);
        return False;
    }

    majorSupported = major = SUPPORTED_XI2_VERSION_MAJOR;
    minorSupported = minor = SUPPORTED_XI2_VERSION_MINOR;

    if ((rc = XIQueryVersion (ctx->xDpy, &major, &minor)) == BadRequest)
    {
        snprintf (buf, sizeof (buf), "\t\tXI2 not available. Server supports "
                  "%d.%d", major, minor);
        logCtrl (buf,  LOG_LVL_NO, True);
        return False;
    }
    else if (rc != Success)
    {
        logCtrl ("\t\tXlib internal error!", LOG_LVL_NO, True);
        return False;
    }
    else if ((major * 1000 + minor) < 1000 * majorSupported + minorSupported)
    {
        snprintf (buf, sizeof (buf), "\t\tAvailable XI2 extension version "
                  "(%d.%d) is not supported.", major, minor);
        logCtrl (buf,  LOG_LVL_NO, True);
        return False;
    }

    snprintf (buf, sizeof (buf), "\t\tXI2 extension version is (%d.%d)", major,
              minor);
    logCtrl (buf,  LOG_LVL_2, True);

    logCtrl ("\t\tSuccess", LOG_LVL_2, True);

    return True;
}

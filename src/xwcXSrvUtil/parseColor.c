#include <xwcXSrvUtil.h>

Bool
parseColor (XWCContext * cfg)
{
    Colormap            xClrMap;
    char                bgClrStrTmp[8], buf[1024];

    if (cfg == NULL)
    {
        logCtrl ("Error parsing color: Null pointer to cfg struct received!",
                 LOG_LVL_NO, False);
    }

    snprintf (buf, sizeof (buf), "\tParsing window background color string %s",
              cfg->bgColorStr);
    logCtrl (buf, LOG_LVL_1, False);

    xClrMap            = DefaultColormapOfScreen (cfg->xScr);

    bgClrStrTmp[0]     = '#';
    bgClrStrTmp[7]     = '\0';

    memcpy (bgClrStrTmp + 1, cfg->bgColorStr, 6);

    if ( XParseColor (cfg->xDpy, xClrMap, bgClrStrTmp, &cfg->bgColor) == 0
        || XAllocColor (cfg->xDpy, xClrMap, &cfg->bgColor) == 0)
    {
        logCtrl ("\t\tError: XParseColor and/or XAllocColor error", LOG_LVL_NO,
                 True);
        XCloseDisplay (cfg->xDpy);
        free (cfg);
        return False;
    }

    snprintf (buf, sizeof (buf), "\t\tColor parsing result: pixel=%ld, red=%d,"
              " green=%d, blue=%d", cfg->bgColor.pixel, cfg->bgColor.red,
              cfg->bgColor.green, cfg->bgColor.blue);

    logCtrl (buf, LOG_LVL_2, True);

    logCtrl ("\t\tSuccess", LOG_LVL_2, True);

    return True;
}

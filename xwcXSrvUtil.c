#include "xwc.h" 

Display *
openDefaultDisplay (void)
{
    Display * d;
    logCtr ("connecting to X server:", LOG_LVL_1, False);
    if (( d = XOpenDisplay (NULL) ) == NULL)
    {
        logCtr ("\tfail to connect to X server", LOG_LVL_NO, True);
        return NULL;
    }
    logCtr ("\tsuccess", LOG_LVL_1, True);
    XSetErrorHandler (errorHandlerBasic);
    return d;
}

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

    logCtr (buf, LOG_LVL_NO, False);

    XGetErrorText (display, error->error_code, buf + 1, 1024);
    buf[0] = '\t';
    logCtr (buf, LOG_LVL_NO, True);

    X_ERROR = True;
    return 1;
}

Bool
getXErrState (void)
{
    Bool res = X_ERROR;
    X_ERROR  = False;
    return res;
}

void
printDrawableInfo (Display  * xDpy,
                   Drawable   drw)
{
    Window       r;
    Status       st;
    int          x, y;
    unsigned int w, h, b, d;
    char         buf[1024];

    st = XGetGeometry (xDpy, drw, &r, &x, &y, &w, &h, &b, &d);

    if (st == 0 )
    {
        logCtr ("XGetGeometry: Cannot get parameters of specified drawable!",
                LOG_LVL_NO, False);
        return;
    }

    snprintf (buf, sizeof (buf), "Drawble information:\n\tWidth:\t"
              "%d\n\tHeight:\t%d\n\tDepth:\t%u\n\troot win:\t%lX", w, h, d, r);

    logCtr (buf, LOG_LVL_1, False);
}

Bool
getVisOfScr (XWCContext  * ctx,
             XVisualInfo * xVisInfo)
{
    int xScrId = XScreenNumberOfScreen (ctx->xScr);
    int res = XMatchVisualInfo (ctx->xDpy, xScrId, ctx->srcWAttr.depth,
                                TrueColor, xVisInfo);
    return res != 0;
}

Window
getDefaultRootWindow (Display * d)
{
    Window w;

    w = DefaultRootWindow (d);

    if (w == None)
    {
        logCtr ("Cannot get default root window of display!",
                LOG_LVL_NO, False);
    }

    return w;
}

Bool
chkCompExt (Display * d)
{
    int  compositeErrorBase, compositeEventBase, xCompExtVerMin, xCompExtVerMaj;
    char buf[1024];

    logCtr ("Checking composite extension:", LOG_LVL_1, False);

    if (XCompositeQueryExtension (d, &compositeEventBase,
                                  &compositeErrorBase) == False)
    {
        logCtr ("\tNo composite extension found, aborting!", LOG_LVL_NO, True);
        return False;
    }
    else if (! XCompositeQueryVersion (d, &xCompExtVerMaj, &xCompExtVerMin))
    {
        logCtr ("\tX Server doesn't support such a version of the X Composite "
                "Extension which is compatible with the client library",
                LOG_LVL_NO, True);
        return False;
    }
    else if ( ( xCompExtVerMaj < 1 ) &&  ( xCompExtVerMin < 2 ) )
    {
        logCtr ("\tUnsupported version of X composite extension (<0.2)",
                LOG_LVL_NO, True);
        return False;
    }
    else
    {
        snprintf (buf, sizeof (buf), "\tversion %d.%d",
                  xCompExtVerMaj, xCompExtVerMin);
        logCtr (buf, LOG_LVL_1, True);
    }
    return True;
}

Bool
parseColor (XWCContext * cfg)
{
    Colormap            xClrMap;
    char                bgClrStrTmp[8], buf[1024];

    if (cfg == NULL)
    {
        logCtr ("Error parsing color: Null pointer to cfg struct received!",
                LOG_LVL_NO, False);
    }

    snprintf (buf, sizeof (buf), "Parsing window background color string %s:",
              cfg->bgColorStr);
    logCtr (buf, LOG_LVL_1, False);

    xClrMap            = DefaultColormapOfScreen (cfg->xScr);

    bgClrStrTmp[0]     = '#';
    bgClrStrTmp[7]     = '\0';

    memcpy (bgClrStrTmp + 1, cfg->bgColorStr, 6);

    if ( XParseColor (cfg->xDpy, xClrMap, bgClrStrTmp, &cfg->bgColor) == 0
        || XAllocColor (cfg->xDpy, xClrMap, &cfg->bgColor) == 0)
    {
        logCtr ("\tError: XParseColor and/or XAllocColor error", LOG_LVL_NO,
                True);
        XCloseDisplay (cfg->xDpy);
        free (cfg);
        return False;
    }

    snprintf (buf, sizeof (buf), "\tColor parsing result: pixel=%ld, red=%d,"
              " green=%d, blue=%d\n", cfg->bgColor.pixel, cfg->bgColor.red,
              cfg->bgColor.green, cfg->bgColor.blue);

    logCtr (buf, LOG_LVL_1, True);

    logCtr ("\tSuccess", LOG_LVL_1, True);

    return True;
}

Screen *
getScreenByWindowAttr (XWCContext        * ctx,
                       XWindowAttributes * winAttr)
{
    logCtr ("Getting screen using window attributes:", LOG_LVL_1, False);

    if (ctx == NULL)
    {
        logCtr ("\tError getting screen by window attributes: "
                "No display specified", LOG_LVL_NO, True);
        return NULL;
    }

    if (ctx->xDpy == NULL)
    {
        logCtr ("\tError getting screen by window attributes: "
                "No display specified", LOG_LVL_NO, True);
        return NULL;
    }

    if (winAttr == NULL)
    {
        logCtr ("\tError getting screen by window attributes: "
                "No window attributes struct specified", LOG_LVL_NO, True);
        return NULL;
    }

    if (winAttr->screen == NULL)
    {
        logCtr ("\tError getting screen by window attributes: No valid screen "
                "pointer found in  window attributes struct", LOG_LVL_NO, True);
        return NULL;
    }

    logCtr ("\tsuccess", LOG_LVL_1, True);

    return winAttr->screen;
}

Window
getRootWinOfScr (Screen * s)
{
    logCtr ("Getting root window of screen:", LOG_LVL_1, False);
    if (s == NULL)
    {
        logCtr ("\tError getting root window of screen: "
                "Invalid pointer to Screen data struct!", LOG_LVL_NO, True);
        return None;
    }

    if (s->root == None)
    {
        logCtr ("\tError getting root window of screen: "
                "No root wondow specified for given screen!", LOG_LVL_NO, True);
    }
    logCtr ("\tsuccess", LOG_LVL_1, True);
    return s->root;
}

Colormap
createColormap (XWCContext * ctx,
                Visual     * xVis)
{
    return XCreateColormap (ctx->xDpy, ctx->srcW, xVis, AllocNone);
}

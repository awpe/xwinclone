#include <xwcXSrvUtil.h>

Bool
openDefaultDisplay (XWCContext * ctx)
{
    logCtrl ("\tconnecting to X server:", LOG_LVL_1, False);
    
    if (( ctx->xDpy = XOpenDisplay (NULL) ) == NULL)
    {
        logCtrl ("\t\tfail to connect to X server", LOG_LVL_NO, True);
        return False;
    }
    
    logCtrl ("\t\tsuccess", LOG_LVL_1, True);
    
    XSetErrorHandler (errorHandlerBasic);
    
    return True;
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

    logCtrl (buf, LOG_LVL_NO, False);

    XGetErrorText (display, error->error_code, buf + 1, 1024);
    buf[0] = '\t';
    logCtrl (buf, LOG_LVL_NO, True);

    X_ERROR      = True;
    X_ERROR_CODE = error->error_code;
    return 1;
}

Bool
getXErrState (XWCContext * ctx)
{
    XSync (ctx->xDpy, False);
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
        logCtrl ("XGetGeometry: Cannot get parameters of specified drawable!",
                LOG_LVL_NO, False);
        return;
    }

    snprintf (buf, sizeof (buf), "Drawble information:\n\tWidth:\t"
              "%d\n\tHeight:\t%d\n\tDepth:\t%u\n\troot win:\t%lX", w, h, d, r);

    logCtrl (buf, LOG_LVL_1, False);
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

Bool
getDefaultDisplayData (XWCContext * ctx)
{
    logCtrl ("\tTrying to get default display data: root window, default screen",
            LOG_LVL_1, False);

    if (ctx == NULL)
    {
        logCtrl ("\t\tError getting default display data: NULL pointer to "
                "program context received", LOG_LVL_NO, True);
        return NULL;
    }

    ctx->rootW = DefaultRootWindow (ctx->xDpy);

    if (ctx->rootW == None)
    {
        logCtrl ("\t\tCannot get default root window of display!",
                LOG_LVL_NO, False);
        return False;
    }

    XGetWindowAttributes (ctx->xDpy, ctx->rootW, &ctx->rootWAttr);

    if (getXErrState (ctx) == True)
    {
        logCtrl ("\t\tCannot get default root window attributes!",
                LOG_LVL_NO, False);
        return False;
    }

    if (ctx->xDpy == NULL)
    {
        logCtrl ("\t\tError getting screen by window attributes: "
                "No display specified", LOG_LVL_NO, True);
        return NULL;
    }

    if (ctx->rootWAttr.screen == NULL)
    {
        logCtrl ("\t\tError getting default screen: No valid screen pointer "
                "found in root window's attributes struct", LOG_LVL_NO, True);
        return NULL;
    }

    ctx->xScr = ctx->rootWAttr.screen;

    logCtrl ("\t\tsuccess", LOG_LVL_2, True);

    return True;
}

Bool
chkCompExt (Display * d)
{
    int  compositeErrorBase, compositeEventBase, xCompExtVerMin, xCompExtVerMaj;
    char buf[1024];

    logCtrl ("\tChecking composite extension", LOG_LVL_1, False);

    if (XCompositeQueryExtension (d, &compositeEventBase,
                                  &compositeErrorBase) == False)
    {
        logCtrl ("\t\tNo composite extension found, aborting!", LOG_LVL_NO, True);
        return False;
    }
    else if (! XCompositeQueryVersion (d, &xCompExtVerMaj, &xCompExtVerMin))
    {
        logCtrl ("\t\tX Server doesn't support such a version of the X Composite "
                "Extension which is compatible with the client library",
                LOG_LVL_NO, True);
        return False;
    }
    else if ( ( xCompExtVerMaj < 1 ) &&  ( xCompExtVerMin < 2 ) )
    {
        logCtrl ("\t\tUnsupported version of X composite extension (<0.2)",
                LOG_LVL_NO, True);
        return False;
    }
    else
    {
        snprintf (buf, sizeof (buf), "\t\tversion %d.%d",
                  xCompExtVerMaj, xCompExtVerMin);
        logCtrl (buf, LOG_LVL_2, True);
    }

    logCtrl ("\t\tSuccess", LOG_LVL_1, True);
    return True;
}

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

    logCtrl ("\t\tSuccess", LOG_LVL_1, True);

    return True;
}

Window
getRootWinOfScr (Screen * s)
{
    logCtrl ("Getting root window of screen:", LOG_LVL_1, False);
    if (s == NULL)
    {
        logCtrl ("\tError getting root window of screen: "
                "Invalid pointer to Screen data struct!", LOG_LVL_NO, True);
        return None;
    }

    if (s->root == None)
    {
        logCtrl ("\tError getting root window of screen: "
                "No root wondow specified for given screen!", LOG_LVL_NO, True);
    }
    logCtrl ("\tsuccess", LOG_LVL_1, True);
    return s->root;
}

Colormap
createColormap (XWCContext * ctx,
                Visual     * xVis)
{
    return XCreateColormap (ctx->xDpy, ctx->srcW, xVis, AllocNone);
}

Bool
getCtrlKeycodes (XWCContext * ctx)
{
    KeySym exitKeySym;
    char   buf[1024];

    logCtrl ("\tgetting control keycodes", LOG_LVL_1, False);

    if (ctx == NULL)
    {
        logCtrl ("\t\tCannot get control keycodes: NULL pointer to program "
                "context struct received!", LOG_LVL_NO, False);
        return False;
    }

    if (( exitKeySym = XStringToKeysym (ctx->exitKeyStr) ) == NoSymbol)
    {
        snprintf (buf, sizeof (buf), "\t\tError parsing exit key string (%s)",
                  ctx->exitKeyStr);
        logCtrl (buf, LOG_LVL_NO, True);
        return False;
    }

    if ((ctx->exitKeyCode = XKeysymToKeycode (ctx->xDpy, exitKeySym)) == 0)
    {
        snprintf (buf, sizeof (buf), "\t\tUnknown keycode %d", ctx->exitKeyCode);
        logCtrl (buf, LOG_LVL_NO, True);
        return False;
    }

    if (( exitKeySym = XStringToKeysym (ctx->transCtrlKeyStr) )
        == NoSymbol)
    {
        snprintf (buf, sizeof (buf), "\t\tError parsing exit key string (%s)",
                  ctx->transCtrlKeyStr);
        logCtrl (buf, LOG_LVL_NO, True);
        return False;
    }

    if (( ctx->cloneKeyCode = XKeysymToKeycode (ctx->xDpy, exitKeySym) ) == 0)
    {
        snprintf (buf, sizeof (buf), "\t\tUnknown keycode %d",
                  ctx->cloneKeyCode);
        logCtrl (buf, LOG_LVL_NO, True);
        return False;
    }

    logCtrl ("\t\tsuccess", LOG_LVL_2, True);
    
    return True;
}
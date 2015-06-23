#include "xwc.h"

Window
getFocusedWindow (Display * d)
{
    if (d == NULL)
    {
        logCtr ("Error getting focused window: No display specified!",
                LOG_LVL_NO, False);
        return None;
    }

    /**
     * @todo find how to utilize revert_to
     */
    int    revert_to;
    Window w;
    char   buf[1024];

    logCtr ("Searching for focused window:", LOG_LVL_1, False);
    XGetInputFocus (d, &w, &revert_to);
    XSync (d, 0);

    if (getXErrState () == True)
    {
        logCtr ("\tfail to get focused window", LOG_LVL_NO, True);
        return None;
    }

    if (w == None)
    {
        logCtr ("\tno focus window", LOG_LVL_NO, True);
    }

    else
    {
        snprintf (buf, sizeof (buf), "\tsuccess, window XID is %lX", w);
        logCtr (buf, LOG_LVL_1, True);
    }

    return w;
}

Window
getTopWindow (Display * d,
              Window    start)
{
    Window         w;
    Window         parent;
    Window         root;
    Window       * children;
    unsigned int   nchildren;
    char           buf[1024];

    w         = start;
    parent    = start;
    root      = None;

    if (d == NULL)
    {
        logCtr ("Error getting top window: No display specified!",
                LOG_LVL_NO, False);
        return None;
    }

    if (start == None)
    {
        logCtr ("Error getting top window: Invalid window specified!",
                LOG_LVL_NO, False);
        return None;
    }

    logCtr ("getting child-of-root window:", LOG_LVL_1, False);

    while (parent != root)
    {
        w = parent;

        if (XQueryTree (d, w, &root, &parent, &children, &nchildren))
        {
            if (children != NULL)
            {
                XFree (children);
            }
        }

        if (getXErrState () == True)
        {
            logCtr ("\tfailed to get top-most window\n", LOG_LVL_NO, True);
            return None;
        }
        snprintf (buf, sizeof (buf), "\tgot parent, window XID is %lX", w);
        logCtr (buf, LOG_LVL_1, True);
    }

    snprintf (buf, sizeof (buf), "\tsuccess, window XID is %lX", w);
    logCtr (buf, LOG_LVL_1, True);

    return w;
}

Window
getNamedWindow (Display * d,
                Window    start)
{
    Window w;
    char   buf[1024];

    if (d == NULL)
    {
        logCtr ("Error getting named window: No display specified!",
                LOG_LVL_NO, False);
        return None;
    }

    if (start == None)
    {
        logCtr ("Error getting named window: Invalid window specified!",
                LOG_LVL_NO, False);
        return None;
    }

    logCtr ("Getting named window:", LOG_LVL_1, False);

    w = XmuClientWindow (d, start);
    if (w == start)
    {
        logCtr ("\tfail to get named window or window already "
                "has WM_STATE property", LOG_LVL_1, True);
    }

    snprintf (buf, sizeof (buf), "\twindow XID is %lX", w);
    logCtr (buf, LOG_LVL_1, True);

    return w;
}

Window
getActiveWindow (XWCContext * ctx)
{
    if (ctx == NULL)
    {
        logCtr ("Error getting active window: No program options"
                " data specified!", LOG_LVL_NO, False);
        return False;
    }

    if (ctx->xDpy == NULL)
    {
        logCtr ("Error getting active window: No display specified!",
                LOG_LVL_NO, False);
        return False;
    }

    if (ctx->isDaemon == False)
    {
        logCtr ("Waiting for focus to be moved to source window", LOG_LVL_NO,
                False);
        nanosleep (&ctx->focusDelay, NULL);
    }

    if (ctx->srcW == None)
    {
        ctx->srcW = getFocusedWindow (ctx->xDpy);
    }

    if (     ctx->srcW                                          == None
        || ( ctx->srcW = getTopWindow (ctx->xDpy, ctx->srcW)   ) == None
        || ( ctx->srcW = getNamedWindow (ctx->xDpy, ctx->srcW) ) == None )
    {
        return False;
    }

    XGetWindowAttributes (ctx->xDpy, ctx->srcW, &ctx->srcWAttr);

    if (getXErrState () == True)
    {
        return False;
    }

    printWindowInfo (ctx->xDpy, ctx->srcW, &ctx->srcWAttr);

    return True;
}

void
printWindowName (Display * d,
                 Window    w)
{
    XTextProperty    prop;
    int              count, result, i;
    char          ** list;
    char             buf[1024];

    if (d == NULL)
    {
        logCtr ("Error printing window name: No display specified!",
                LOG_LVL_NO, False);
        return;
    }

    if (w == None)
    {
        logCtr ("Error printing window name: Invalid window specified!",
                LOG_LVL_NO, False);
        return;
    }

    memset (&prop, 0, sizeof (prop ));
    prop.value = NULL;

    logCtr ("window name:", LOG_LVL_1, False);

    if (getXErrState () == False && XGetWMName (d, w, &prop) != 0)
    {
        count  = 0;
        list   = NULL;
        result = XmbTextPropertyToTextList (d, &prop, &list, &count);

        if (result == Success)
        {
            snprintf (buf, 1024, "\t%s", list[0]);
            logCtr (buf, LOG_LVL_1, True);
        }
        else
        {
            logCtr ("\tError printing window name: "
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
        logCtr ("\tError printing window name: XGetWMName err", LOG_LVL_NO,
                True);
    }

    if (prop.value != NULL)
    {
        XFree (prop.value);
    }
}

void
printWindowClass (Display * d,
                  Window    w)
{
    XClassHint * class;
    char         buf[1024];

    if (d == NULL)
    {
        logCtr ("Error printing window class: No display specified!",
                LOG_LVL_NO, False);
        return;
    }

    if (w == None)
    {
        logCtr ("Error printing window class: Invalid window specified!",
                LOG_LVL_NO, False);
        return;
    }

    class = XAllocClassHint ();

    if (getXErrState () == True)
    {
        logCtr ("Error printing window class info: XAllocClassHint err",
                LOG_LVL_NO, False);
        if (class != NULL)
        {
            XFree (class);
        }
        return;
    }

    if (getXErrState () == False && XGetClassHint (d, w, class) != 0)
    {
        snprintf (buf, sizeof (buf), "application:\n\tname:\t%s\n\tclass:\t%s",
                  class->res_name, class->res_class);

        logCtr (buf, LOG_LVL_1, False);
    }
    else
    {
        logCtr ("Error printing window class info: XGetClassHint err",
                LOG_LVL_NO, False);
    }

    if (class != NULL)
    {
        if (class->res_class != NULL)
        {
            XFree (class->res_class);
        }

        if (class->res_name != NULL)
        {
            XFree (class->res_name);
        }

        XFree (class);
    }
}


void
printWindowInfo (Display           * d,
                 Window              w,
                 XWindowAttributes * xWinAttr)
{

    char buf[1024];

    if (d == NULL)
    {
        logCtr ("Error cannot print window information: "
                "Bad X display pointer", LOG_LVL_NO, False);
        return;
    }

    if (w == None)
    {
        logCtr ("Error cannot print window information: No window specified",
                LOG_LVL_NO, False);
        return;
    }

    if (xWinAttr == NULL)
    {
        logCtr ("Error cannot print window information: "
                "No window attributes data specified", LOG_LVL_NO, False);
        return;
    }

    printWindowName (d, w);
    printWindowClass (d, w);

    snprintf (buf, sizeof (buf), "Window info:\n\tWidth:\t%d\n\tHeight:\t%d\n\t"
              "Depth:\t%d",
              xWinAttr->width, xWinAttr->height, xWinAttr->depth);

    logCtr (buf, LOG_LVL_1, False);
}

Bool
setWinTitlebar (Display    * d,
                Window       WID,
                const char * name)
{
    XTextProperty tp;

    if (d == NULL)
    {
        logCtr ("Error changing window name: No display specified!",
                LOG_LVL_NO, False);
        return False;
    }

    if (name == NULL)
    {
        logCtr ("Error changing window name: Name string is not specified!",
                LOG_LVL_NO, False);
        return False;
    }

    if (WID == None)
    {
        logCtr ("Error changing window name: No window specified!\n",
                LOG_LVL_NO, False);
        return False;
    }

    logCtr ("\tSetting window titlebar text:", LOG_LVL_1, False);

    memset (&tp, 0, sizeof (tp ));
    tp.value = NULL;

    /* As of Xlib version 1.6.3 XStringListToTextProperty doesn't affect its 
     * argv (first char**) argument (Xlib's SetTxtProp.c), 
     * so let's think it is const...*/
    if (XStringListToTextProperty ((char **) &name, 1, &tp) == False)
    {
        logCtr ("\t\tError setting name of window: "
                "XStringListToTextProperty err", LOG_LVL_NO, True);
        return False;
    }

    XChangeProperty (d, WID, XA_WM_NAME, tp.encoding, tp.format,
                     PropModeReplace, tp.value, tp.nitems);
    /*
    free (a);
     */
    if (tp.value != NULL)
    {
        XFree (tp.value);
    }

    if (getXErrState () == True)
    {
        logCtr ("\t\tError setting name of window: "
                "XChangeProperty err", LOG_LVL_NO, True);
        return False;
    }

    logCtr ("\t\tSuccess", LOG_LVL_1, True);

    return True;
}

Bool
setWindowClass (Display    * d,
                Window       WID,
                const char * permNameStr,
                const char * classStr)
{
    XClassHint * xClassHint;

    if (d == NULL)
    {
        logCtr ("Error changing window class: No display specified!" ,
                LOG_LVL_NO, False);
        return False;
    }

    if (permNameStr == NULL)
    {
        logCtr ("Error changing window class: Window permanent name string "
                "is not specified!", LOG_LVL_NO, False);
        return False;
    }

    if (classStr == NULL)
    {
        logCtr ("Error changing window class: Class string is not "
                "specified!", LOG_LVL_NO, False);
        return False;
    }

    if (WID == None)
    {
        logCtr ("Error changing window class: No window specified!",
                LOG_LVL_NO, False);
        return False;
    }

    logCtr ("\tSetting window class strings:", LOG_LVL_1, False);

    if (( xClassHint = XAllocClassHint () ) == NULL || getXErrState () == True)
    {
        if (xClassHint != NULL)
        {
            XFree (xClassHint);
        }
        logCtr ("\t\terror setting window class strings!", LOG_LVL_NO, True);
        return False;
    }

    /* As of Xlib version 1.6.3 XSetClassHint() doesn't affect res_class and
     * res_name fields of xClassHint structure so let this field be treated as
     * constant...*/
    xClassHint->res_class = (char*) classStr;
    xClassHint->res_name  = (char*) permNameStr;

    XSetClassHint (d, WID, xClassHint);

    xClassHint->res_class = NULL;
    xClassHint->res_name  = NULL;

    XFree (xClassHint);

    if (getXErrState () == False)
    {
        logCtr ("\t\tSuccess", LOG_LVL_1, True);
        return True;
    }

    return False;
}


Window
createWindow (XWCContext           * ctx,
              Visual               * xVis,
              long long int          mask,
              XSetWindowAttributes * attr)
{
    Window w;
    w = XCreateWindow (ctx->xDpy, ctx->rootW, 0, 0, ctx->srcWAttr.width,
                       ctx->srcWAttr.height, 0, ctx->srcWAttr.depth,
                       InputOutput, xVis, mask, attr);
    return w;
}

Bool
createTrgWindow (XWCContext * ctx)
{
    XSetWindowAttributes  trgWinSetAttr;
    XVisualInfo           xVisInfo;
    long long int         mask;

    logCtr ("Creating translation window:", LOG_LVL_1, False);

    if (getVisOfScr (ctx, &xVisInfo) == False)
    {
        logCtr ("\tError: no such visual", LOG_LVL_NO, False);
        return False;
    }

    trgWinSetAttr.colormap         = createColormap (ctx, xVisInfo.visual);
    trgWinSetAttr.background_pixel = ctx->bgColor.pixel;
    trgWinSetAttr.border_pixel     = 0;
    trgWinSetAttr.bit_gravity      = NorthWestGravity;
    trgWinSetAttr.event_mask       = ButtonPressMask | ButtonReleaseMask | ButtonMotionMask;
    mask                           = CWBackPixel | CWColormap | CWBorderPixel |
            CWBitGravity | CWEventMask;

    ctx->trgW = createWindow (ctx, xVisInfo.visual, mask, &trgWinSetAttr);

    if (getXErrState () == True)
    {
        logCtr ("\tfailed to create window!", LOG_LVL_NO, False);
        if (ctx->trgW != None)
        {
            XDestroyWindow (ctx->xDpy, ctx->trgW);
        }
        return None;
    }

    if (setWinTitlebar (ctx->xDpy, ctx->trgW, WM_CLASS_PRG_NAME_STR) == False
        || setWindowClass (ctx->xDpy, ctx->trgW, WM_CLASS_PRG_NAME_STR,
                           WM_CLASS_CLASS_NAME_STR) == False )
    {
        return False;
    }

    XMapWindow (ctx->xDpy, ctx->trgW);

    if (getXErrState () == True)
    {
        logCtr ("\tfailed to map window!", LOG_LVL_NO, False);
        return False;
    }

    XGetWindowAttributes (ctx->xDpy, ctx->trgW, &ctx->trgWAttr);

    if (getXErrState () == True)
    {
        return False;
    }

    printWindowInfo (ctx->xDpy, ctx->trgW, &ctx->trgWAttr);

    logCtr ("\tsuccess", LOG_LVL_1, True);

    return True;
}

unsigned char *
getWPrprtByAtom (XWCContext * ctx, 
                 Window       window, 
                 Atom         atom, 
                 long       * nitems,
                 Atom       * type, 
                 int        * size)
{
    /*From documenetation examples*/
    Atom            actual_type;
    int             actual_format;
    unsigned long   nitems_;
    unsigned long   bytes_after;
    unsigned char * prop;
    int             status;
    char            buf[1024];

    status = XGetWindowProperty (ctx->xDpy, window, atom, 0, (~ 0L),
                                 False, AnyPropertyType, &actual_type,
                                 &actual_format, &nitems_, &bytes_after,
                                 &prop);

    if (getXErrState () == True)
    {
        snprintf (buf, sizeof (buf), "Cannot get window property by atom!");
        logCtr (buf, LOG_LVL_NO, False);
        return NULL;
    }

    if (status != Success)
    {
        logCtr ("XGetWindowProperty failed!", LOG_LVL_NO, False);
        return NULL;
    }

    if (nitems != NULL)
    {
        *nitems = nitems_;
    }

    if (type != NULL)
    {
        *type = actual_type;
    }

    if (size != NULL)
    {
        *size = actual_format;
    }

    return prop;
}

int
findWClient (XWCContext * ctx, 
             Window       window, 
             Window     * window_ret,
             int          direction)
{
    Window       dummy, parent, *children;
    unsigned int nchildren, i;
    Atom         atom_wmstate;
    int          done, ret;
    long         items;
    char         buf[1024];

    atom_wmstate = XInternAtom (ctx->xDpy, "WM_STATE", False);
    done         = False;
    children     = NULL;

    while (done == False)
    {
        if (window == None)
        {
            return False;
        }

        getWPrprtByAtom (ctx, window, atom_wmstate, &items, NULL, NULL);

        if (items == 0)
        {
            /* This window doesn't have WM_STATE property, keep searching. */
            XQueryTree (ctx->xDpy, window, &dummy, &parent, &children, &nchildren);

            if (direction == FIND_PARENTS)
            {
                if (children != NULL)
                {
                    XFree (children);
                }
                window = parent;
            }
            else if (direction == FIND_CHILDREN)
            {
                done = True;

                for (i = 0; i < nchildren; ++ i)
                {
                    ret = findWClient (ctx, children[i], &window, direction);

                    if (ret == True)
                    {
                        *window_ret = window;
                        break;
                    }
                }

                if (nchildren == 0)
                {
                    return False;
                }

                if (children != NULL)
                {
                    XFree (children);
                }
            }
            else
            {
                snprintf (buf, sizeof (buf), "Invalid findWClient direction"
                          " (%d)\n", direction);
                logCtr (buf, LOG_LVL_NO, False);

                *window_ret = 0;

                if (children != NULL)
                {
                    XFree (children);
                }

                return False;
            }
        }
        else
        {
            *window_ret = window;
            done = True;
        }
    }
    return True;
}

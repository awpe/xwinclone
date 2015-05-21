#include "xwclib.h"

/*Here global X_ERROR initialization takes place*/
Bool X_ERROR = False;

Display*
openDefaultDisplay (void)
{
    Display * d;
    printf ("connecting to X server ... ");
    if (( d = XOpenDisplay (NULL) ) == NULL)
    {
        printf ("fail\n");
        return NULL;
    }
    printf ("success\n");
    return d;
}

int
errorHandlerBasic (    Display * display,
                   XErrorEvent * error)
{
    if (!display)
    {
        /*do something*/
        return 1;
    }
    
    if (!error)
    {
        /*do something*/
        return 1;
    }
    
    printf ("ERROR: X11 error\n\terror code: %d\n", error->error_code);
    X_ERROR = True;
    return 1;
}

Bool
getXErrState (void)
{
    Bool res = X_ERROR;
    X_ERROR = False;
    return res;
}

Window
getFocusedWindow (Display * d)
{
    /**
     * @todo find how to utilize revert_to
     */
    int    revert_to;
    Window w;

    if (d == NULL)
    {
        printf ("Error getting focused window: No display specified!\n");
        return None;
    }

    printf ("getting input focus window ... ");
    XGetInputFocus (d, &w, &revert_to);
    if (getXErrState () == True)
    {
        printf ("fail\n");
        return None;
    }
    if (w == None)
    {
        printf ("no focus window\n");
    }
    else
    {
        printf ("success\n\twindow xid: %lX\n", w);
    }
    return w;
}

Window
getTopWindow (Display * d,
               Window   start)
{
    Window         w;
    Window         parent;
    Window         root;
    Window       * children;
    unsigned int   nchildren;
    Status         s;

    w         = start;
    parent    = start;
    root      = None;

    if (d == NULL)
    {
        printf ("Error getting top window: No display specified!\n");
        return None;
    }
    if (start == None)
    {
        printf ("Error getting top window: Invalid window specified!\n");
        return None;
    }

    printf ("getting child-of-root window ... \n");
    while (parent != root)
    {
        w = parent;
        s = XQueryTree (d, w, &root, &parent, &children, &nchildren);

        if (s)
        {
            if (children != NULL)
            {
                XFree (children);
            }
        }

        if (getXErrState () == True)
        {
            printf ("fail\n");
            return None;
        }

        printf (" got parent (window: %X)\n", (int) w);
    }

    printf ("success (window: %X)\n", (int) w);

    return w;
}

Window
getNamedWindow (Display * d,
                 Window   start)
{
    Window w;

    if (d == NULL)
    {
        printf ("Error getting named window: No display specified!\n");
        return None;
    }
    if (start == None)
    {
        printf ("Error getting named window: Invalid window specified!\n");
        return None;
    }

    printf ("getting named window ... ");
    w = XmuClientWindow (d, start);
    if (w == start)
    {
        printf ("fail or window already has WM_STATE property\n");
    }
    printf ("returning window: %X\n", (int) w);
    return w;
}

Window
getActiveWindow (Display * d)
{
    if (d == NULL)
    {
        printf ("Error getting active window: No display specified!\n");
        return None;
    }
    Window w;

    if ( ( w = getFocusedWindow (d) ) == None
        || ( w = getTopWindow (d, w) ) == None
        || ( w = getNamedWindow (d, w) ) == None )
    {
        return None;
    }
    return w;
}

void
printWindowName (Display * d,
                  Window   w)
{
    XTextProperty    prop;
    Status           s;
    int              count, result, i;
    char          ** list;

    if (d == NULL)
    {
        printf ("Error printing window name: No display specified!\n");
        return;
    }

    if (w == None)
    {
        printf ("Error printing window name: Invalid window specified!\n");
        return;
    }

    memset (&prop, 0, sizeof (prop ));
    prop.value = NULL;

    printf ("window name:\n");

    s = XGetWMName (d, w, &prop);

    if (getXErrState () == False && s)
    {
        count  = 0;
        list   = NULL;
        result = XmbTextPropertyToTextList (d, &prop, &list, &count);

        if (result == Success)
        {
            printf ("\t%s\n", list[0]);
        }
        else
        {
            printf ("Error printing window name: "
                    "XmbTextPropertyToTextList err\n");
        }
        if (list != NULL)
        {
            for (i = 0; i < count; ++i)
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
        printf ("Error printing window name: XGetWMName err\n");
    }
    if (prop.value != NULL)
    {

        XFree (prop.value);
    }
}

void
printWindowClass (Display * d,
                   Window   w)
{
    XClassHint * class;
    Status       s;

    if (d == NULL)
    {
        printf ("Error printing window class: No display specified!\n");
        return;
    }

    if (w == None)
    {
        printf ("Error printing window class: Invalid window specified!\n");
        return;
    }

    class = XAllocClassHint ();

    if (getXErrState () == True)
    {
        printf ("Error printing window class info: XAllocClassHint err\n");
        if (class != NULL)
        {
            XFree (class);
        }
        return;
    }

    s = XGetClassHint (d, w, class);

    if (getXErrState () == False && s)
    {
        printf ("application: \n"
                "\tname: %s\n\tclass: %s\n", class->res_name, class->res_class);
    }
    else
    {
        printf ("Error printing window class info: XGetClassHint err\n");
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
printWindowInfo (          Display * d,
                            Window   w,
                 XWindowAttributes * xWinAttr)
{

    if (d == NULL)
    {
        printf ("Error cannot print window information: Bad X display pointer\n");
        return;
    }

    if (w == None)
    {
        printf ("Error cannot print window information: No window specified\n");
        return;
    }

    if (xWinAttr == NULL)
    {
        printf ("Error cannot print window information: "
                "No window attributes data specified\n");
        return;
    }

    printWindowName (d, w);
    printWindowClass (d, w);

    printf ("Width: %d\nHeight: %d\nDepth: %d\n", xWinAttr->width, xWinAttr->height, xWinAttr->depth);

}

Bool
setWinTitlebar (      Display * d,
                       Window   WID,
                const    char * name)
{
    XTextProperty tp;

    if (d == NULL)
    {
        printf ("Error changing window name: No display specified!\n");
        return False;
    }

    if (name == NULL)
    {
        printf ("Error changing window name: Name string is not specified!\n");
        return False;
    }

    if (WID == None)
    {
        printf ("Error changing window name: No window specified!\n");
        return False;
    }

    /*
char * a;
a = (char*) malloc (sizeof (char ) * 1024);
if (!a)
{
    //
}
memset (a, 0, sizeof (char ) * 1024);
strncpy (a, name, strlen (name));
     */
    memset (&tp, 0, sizeof (tp ));
    tp.value = NULL;

    /* As of Xlib version 1.6.3 XStringListToTextProperty doesn't affect its 
     * argv (first char**) argument (Xlib's SetTxtProp.c), 
     * so let's think it is const...*/
    if (XStringListToTextProperty ((char **) &name, 1, &tp) == False)
    {
        printf ("Error setting name of window: "
                "XStringListToTextProperty err\n");
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

        printf ("Error setting name of window: "
                "XChangeProperty err\n");
        return False;
    }

    return True;
}

Bool
setWindowClass (      Display * d,
                       Window   WID,
                const    char * permNameStr,
                const    char * classStr)
{
    XClassHint * xClassHint;

    if (d == NULL)
    {
        printf ("Error changing window class: No display specified!\n");
        return False;
    }

    if (permNameStr == NULL)
    {
        printf ("Error changing window class: Window permanent name string "
                "is not specified!\n");
        return False;
    }
    if (classStr == NULL)
    {
        printf ("Error changing window class: Class string is not "
                "specified!\n");
        return False;
    }

    if (WID == None)
    {
        printf ("Error changing window class: No window specified!\n");
        return False;
    }

    if (( xClassHint = XAllocClassHint () ) == NULL || getXErrState () == True)
    {
        if (xClassHint != NULL)
        {

            XFree (xClassHint);
        }
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

    return getXErrState () != True;
}

XWCOptions *
processArgs (      Display *  d,
                       int    argCnt,
             const    char ** argArr)
{
    int          frameRate, focusTime, autoCenter, topOffset;
    const char * bgColorStr, * exitKeyStr;
    KeySym       exitKeySym;
    unsigned int exitKeyMask;

    if (argCnt == 2 && ( ! strcmp (argArr[1], "-help")
        || ! strcmp (argArr[1], "-h")
        || ! strcmp (argArr[1], "--help") ))
    {
        printf ("\nUSAGE:\n\n"
                "\t%s [-fr FRAMERATE -ft FOCUSTIME "
                "-ac AUTOCENTER -toffset TOPOFFSET -bg BGCOLOR]\n\n"
                "\tthisProgramName [-help | -h | --help]\n\n"
                "\tDefaults:\n"
                "\t\tFRAMERATE is %d(fps)\n"
                "\t\tFOCUSTIME is %d(seconds)\n"
                "\t\tAUTOCENTER is %d(boolean)\n"
                "\t\tTOPOFFSET is %d(pixels)\n"
                "\t\tBGCOLOR is %s(#rrggbb)\n"
                "\tpress %s to exit program\n\n", PROGRAM_NAME_STR,
                FRAMERATE_FPS, TIME_TO_CHANGE_FOCUS_SEC, AUTOCENTERING,
                TOP_OFFSET, DEFAULT_BG_COLOR, DEF_EXIT_KOMBINATION_STR);
        return NULL;
    }
    else if (argCnt == 11)
    {
        if (strcmp (argArr[1], "-fr")
            || ! ( frameRate = strtol (argArr[2], NULL, 10) ))
        {
            printf ("Error parsing FRAMERATE argument!\n\nTry:\n\n"
                    "\t%s [-help | -h | --help]\n\n", PROGRAM_NAME_STR);
            return NULL;
        }
        if (strcmp (argArr[3], "-ft")
            || ! ( focusTime = strtol (argArr[4], NULL, 10) ))
        {
            printf ("Error parsing FOCUSTIME argument!\n\nTry:\n\n"
                    "\t%s [-help | -h | --help]\n\n", PROGRAM_NAME_STR);
            return NULL;
        }
        if (strcmp (argArr[5], "-ac")
            || ! ( autoCenter = strtol (argArr[6], NULL, 10) ))
        {
            printf ("Error parsing AUTOCENTER argument!\n\nTry:\n\n"
                    "\t%s [-help | -h | --help]\n\n", PROGRAM_NAME_STR);
            return NULL;
        }
        if (strcmp (argArr[7], "-toffset")
            || ! ( topOffset = strtol (argArr[8], NULL, 10) ))
        {
            printf ("Error parsing TOPOFFSET argument!\n\nTry:\n\n"
                    "\t%s [-help | -h | --help]\n\n", PROGRAM_NAME_STR);
            return NULL;
        }
        if (strcmp (argArr[9], "-bg")
            || ! ( strlen (bgColorStr = argArr[10]) != 7 ))
        {
            printf ("Error parsing BGCOLOR argument (example '-bg #ffeedd')!\n\nTry:\n\n"
                    "\t%s [-help | -h | --help]\n\n", PROGRAM_NAME_STR);
            return NULL;
        }
    }
    else if (argCnt != 1)
    {
        printf ("Error parsing arguments!\n\nTry:\n\n"
                "\t%s [-help | -h | --help]\n\n", PROGRAM_NAME_STR);
        return NULL;
    }
    else
    {
        focusTime   = TIME_TO_CHANGE_FOCUS_SEC;
        frameRate   = FRAMERATE_FPS;
        autoCenter  = AUTOCENTERING;
        topOffset   = TOP_OFFSET;
        bgColorStr  = DEFAULT_BG_COLOR;
        //exitKey     = EXIT_KEY;
        exitKeyStr  = EXIT_KEY_STR;
        exitKeyMask = EXIT_MASK;
    }

    XWCOptions * ret = (XWCOptions*) malloc (sizeof (XWCOptions ));

    if (ret == NULL)
    {
        printf ("Error allocating memory for options struct!\n");
        return NULL;
    }

    ret->focusDelay.tv_nsec = 0;
    ret->focusDelay.tv_sec  = focusTime;
    ret->frameDelay.tv_nsec = ( 1.0 / frameRate ) * 1000000000L;
    ret->frameDelay.tv_sec  = 0;
    ret->autoCenter         = autoCenter;
    ret->topOffset          = topOffset;
    ret->bgColorStr         = bgColorStr;
    //ret->exitKey            = exitKey;
    ret->exitKeyStr         = exitKeyStr;
    ret->exitKeyMask        = exitKeyMask;

    if (( exitKeySym = XStringToKeysym (exitKeyStr) ) == NoSymbol)
    {
        printf ("Error parsing exit key string (%s)\n", exitKeyStr);
        free (ret);
        return NULL;
    }

    if (!( ret->exitKeyCode = XKeysymToKeycode (d, exitKeySym) ))
    {
        printf ("Unknown keycode %d\n", ret->exitKeyCode);
        free (ret);
        return NULL;
    }

    printf ("Selected focus time is %ld\n"
            "Selected frame rate is %ld, this results in %ldns frame delay\n"
            "Selected autocentering mode is %d(bool)\n"
            "Selected top offset is %d(px)\n"
            "Selected bgcolor is %s(#rrggbb)\n\n",
            ret->focusDelay.tv_sec,
            (__syscall_slong_t) ( 1000000000.0F / ret->frameDelay.tv_nsec ),
            ret->frameDelay.tv_nsec,
            ret->autoCenter,
            ret->topOffset,
            ret->bgColorStr);

    return ret;
}

Screen *
getScreenByWindowAttr (          Display * d,
                       XWindowAttributes * winAttr)
{
    if (d == NULL)
    {
        printf ("Error getting screen by window attrinutes: "
                "No display spicified\n");
        return NULL;
    }

    if (winAttr == NULL)
    {
        printf ("Error getting screen by window attrinutes: "
                "No window attributes struct spicified\n");
        return NULL;
    }

    if (winAttr->screen == NULL)
    {
        printf ("Error getting screen by window attrinutes: "
                "No valid screen pointer found in  window attributes struct\n");
        return NULL;
    }

    return winAttr->screen;
}

Window
getRootWinOfScr (Screen * s)
{
    if (s == NULL)
    {
        printf ("Error getting root window of screen: "
                "Invalid pointer to Screen data struct!\n");
        return None;
    }

    if (s->root == None)
    {
        printf ("Error getting root window of screen: "
                "No root wondow specified for given screen!\n");
    }
    return s->root;
}

Bool
grabExitKey (   Display * d,
                 Window   grabWin,
             XWCOptions * prgCfg)
{

    if (d == NULL)
    {
        printf ("Cannot grab exit key combination: null pointer to X "
                "connection!\n");
        return False;
    }

    if (prgCfg == NULL)
    {
        printf ("Cannot grab exit key combination: invalid pointer to options "
                "data structure!\n");
        return False;
    }

    if (grabWin == None)
    {
        printf ("Cannot grab exit key combination: no window specified!\n");
        return False;
    }

    XGrabKey (d, prgCfg->exitKeyCode, prgCfg->exitKeyMask, grabWin, False,
              GrabModeAsync, GrabModeAsync);

    if (getXErrState () == True)
    {
        printf ("Cannot grab exit key combination: XGrabKey error!\n");
        return False;
    }

    XSelectInput (d, grabWin,  KeyPressMask);
    /*XSelectInput only throws badwindow which we've already checked*/
    return True;
}

Bool
chkCompExt (Display * d)
{
    int compositeErrorBase, compositeEventBase, xCompExtVerMin, xCompExtVerMaj;

    XSetErrorHandler (errorHandlerBasic);

    if (XCompositeQueryExtension (d, &compositeEventBase,
                                  &compositeErrorBase) == False)
    {
        printf ("No composite extension found, aborting...\n");
        return False;
    }
    else if (!XCompositeQueryVersion (d, &xCompExtVerMaj, &xCompExtVerMin))
    {
        printf ("X Server doesn't support such a version of the X Composite "
                "Extension which is compatible with the client library\n");
        return False;
    }
    else if ( ( xCompExtVerMaj < 1 ) &&  ( xCompExtVerMin < 2 ) )
    {
        printf ("Unsupported version of X composite extension (<0.2)\n");
        return False;
    }
    else
    {
        printf ("Composite extension ready, version %d.%d\n", xCompExtVerMaj,
                xCompExtVerMin);
    }
    return True;
}

Bool
parseColor (   Display * d,
            XWCOptions * prgCfg,
                Screen * s)
{
    Colormap xClrMap = DefaultColormapOfScreen (s);
    /*
     * Maybe this color string parsing must take place 
     * while processing options... And now it is here :)
     */
    printf ("\nParsing the window background color string \"%s\" ... ",
            prgCfg->bgColorStr);

    if (! XParseColor (d, xClrMap, prgCfg->bgColorStr, &prgCfg->bgColor)
        || ! XAllocColor (d, xClrMap, &prgCfg->bgColor))
    {
        printf ("Error:\n\tXParseColor and/or XAllocColor error\n");
        return False;
    }

    printf ("Success\n");

    /* redundant information
    printf ("Color parsing result: pixel=%ld, red=%d, green=%d, blue=%d\n",
            xColor.pixel, xColor.red, xColor.green, xColor.blue);
     */
    return True;
}

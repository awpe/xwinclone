#include "xwclib.h"

/*Here global X_ERROR initialization takes place*/
Bool X_ERROR = False;

Display *
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

    printf ("ERROR: X11 error\n\terror code: %d\n", error->error_code);
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
              Window    start)
{
    Window         w;
    Window         parent;
    Window         root;
    Window       * children;
    unsigned int   nchildren;

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

        if (XQueryTree (d, w, &root, &parent, &children, &nchildren))
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
                Window    start)
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
    Window w;

    if (d == NULL)
    {
        printf ("Error getting active window: No display specified!\n");
        return None;
    }

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
                 Window    w)
{
    XTextProperty    prop;
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

    if (getXErrState () == False && XGetWMName (d, w, &prop) != 0)
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
        printf ("Error printing window name: XGetWMName err\n");
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

    if (getXErrState () == False && XGetClassHint (d, w, class) != 0)
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
printWindowInfo (Display           * d,
                 Window              w,
                 XWindowAttributes * xWinAttr)
{

    if (d == NULL)
    {
        printf ("Error cannot print window information: "
                "Bad X display pointer\n");
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

    printf ("Width: %d\nHeight: %d\nDepth: %d\n", xWinAttr->width,
            xWinAttr->height, xWinAttr->depth);

}

Bool
setWinTitlebar (Display    * d,
                Window       WID,
                const char * name)
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
setWindowClass (Display    * d,
                Window       WID,
                const char * permNameStr,
                const char * classStr)
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

    return getXErrState () == False;
}

Bool
parseColor (Display    * d,
            XWCOptions * prgCfg,
            Screen     * s)
{
    Colormap xClrMap = DefaultColormapOfScreen (s);
    /*
     * Maybe this color string parsing must take place 
     * while processing options... :)
     */
    printf ("\nParsing window background color string \"%s\" ... ",
            prgCfg->bgColorStr);

    char * bgClrStrTmp = (char*) malloc (8 * sizeof (char ));
    bgClrStrTmp[0]     = '#';
    bgClrStrTmp[7]     = '\0';
    memcpy (bgClrStrTmp + 1, prgCfg->bgColorStr, 6);

    if ( XParseColor (d, xClrMap, bgClrStrTmp, &prgCfg->bgColor) == 0
        || XAllocColor (d, xClrMap, &prgCfg->bgColor) == 0)
    {
        printf ("Error:\n\tXParseColor and/or XAllocColor error\n");
        free (bgClrStrTmp);
        return False;
    }

    free (bgClrStrTmp);

    printf ("Success\n");

    /* redundant information
    printf ("Color parsing result: pixel=%ld, red=%d, green=%d, blue=%d\n",
            xColor.pixel, xColor.red, xColor.green, xColor.blue);
     */
    return True;
}

void
delArg (argument * arg)
{
    if (arg == NULL)
    {
        return;
    }

    if (arg->m_Value != NULL)
    {
        if (arg->m_Type == INT)
        {
            free (arg->m_Value);
        }
    }

    if ( arg->m_SynStrs != NULL)
    {
        free (arg->m_SynStrs);
    }
}

void
delArgs (arguments * args)
{
    if (args == NULL)
    {
        return;
    }

    if ( args->m_Args != NULL)
    {
        for (int i = 0; i < args->m_ArgCnt; ++i)
        {
            if (args->m_Args[i] != NULL)
            {
                delArg (args->m_Args[i]);
                free (args->m_Args[i]);
            }
        }
        free (args->m_Args);
    }
    free (args);
}

arguments *
initArgs ()
{
    arguments * args = (arguments*) malloc (sizeof (arguments ));

    if (args == NULL)
    {
        return NULL;
    }

    args->m_ArgCnt = OPTIONS_COUNT;
    args->m_Args   = (argument**) malloc (sizeof (argument ) * args->m_ArgCnt);

    for (int i = 0; i < args->m_ArgCnt; ++i)
    {
        args->m_Args[i] = (argument*) malloc (sizeof (argument ));
        memset (args->m_Args[i], 0, sizeof (*args->m_Args[i] ));
        args->m_Args[i]->m_NameStr  = NULL;
        args->m_Args[i]->m_SynStrs  = NULL;
        args->m_Args[i]->m_Value    = NULL;
        args->m_Args[i]->m_HasValue = False;
    }

    return args;
}

Bool
addArg (arguments  * args,
        Bool         hasValue,
        argTypes     type,
        argNames     name,
        const char * nameStr,
        int          argSynCnt, ...)
{
    if (args == NULL)
    {
        return False;
    }

    va_list argStrList;

    va_start (argStrList, argSynCnt);

    argument * arg = args->m_Args[name];

    if (arg == NULL)
    {
        return False;
    }

    arg->m_IsSet    = False;
    arg->m_SynCnt   = argSynCnt;
    arg->m_Type     = type;
    arg->m_Name     = name;
    arg->m_HasValue = hasValue;
    arg->m_NameStr  = nameStr;
    arg->m_SynStrs  = (const char **) malloc (sizeof (const char*) * argSynCnt);

    if (arg->m_SynStrs == NULL)
    {
        return False;
    }

    for (int i = 0; i < argSynCnt; ++i)
    {
        arg->m_SynStrs[i] = va_arg (argStrList, const char *);
    }

    if (arg->m_HasValue == True)
    {
        switch (arg->m_Type)
        {
            case INT:
                arg->m_Value = (int*) malloc (sizeof (int ));
                break;

            case C_STR:
                //arg->m_Value = (char**) malloc (sizeof (char*));
                break;

            default:
                printf ("Error adding argument, bad type specified!\n");
                free (arg->m_SynStrs);
                return False;
        }
    }

    switch (arg->m_Name)
    {
        case HELP:
            break;

        case AUTOCENTER:
            *( (int*) arg->m_Value ) = AUTOCENTERING;
            break;

        case TOPOFFSET:
            *( (int*) arg->m_Value ) = TOP_OFFSET;
            break;

        case FOCUSTIME:
            *( (int*) arg->m_Value ) = TIME_TO_CHANGE_FOCUS_SEC;
            break;

        case BGCOLOR:
            arg->m_Value             = (void*) DEFAULT_BG_COLOR;
            break;

        case FRAMERATE:
            *( (int*) arg->m_Value ) = FRAMERATE_FPS;
            break;

        default:
            printf ("Unknown argument type!\n");
            return False;
            break;
    }

    va_end (argStrList);

    return True;
}

void
printUsage (arguments  * args)
{
    if (args == NULL)
    {
        return;
    }

    printf ("\nUSAGE:\n\n\t%s ", PROGRAM_EXE_NAME_STR);

    for (int i = 0; i < args->m_ArgCnt; ++i)
    {
        printf ("[");
        for (int j = 0; j < args->m_Args[i]->m_SynCnt; ++j)
        {
            printf ("%s", args->m_Args[i]->m_SynStrs[j]);

            if (j + 1 < args->m_Args[i]->m_SynCnt)
            {
                printf (" | ");
            }
        }
        if (args->m_Args[i]->m_HasValue == True)
        {
            printf (" %s", args->m_Args[i]->m_NameStr);
        }
        printf ("] ");
    }

    printf ("\n\n\tDefaults:\n");

    for (int i = 0; i < args->m_ArgCnt; ++i)
    {
        if (args->m_Args[i]->m_HasValue == True)
        {
            printf ("\t\t%s\t", args->m_Args[i]->m_NameStr);

            switch (args->m_Args[i]->m_Type)
            {
                case INT:
                    printf ("%d\n", *( (int*) args->m_Args[i]->m_Value ));
                    break;

                case C_STR:
                    printf ("\t%s\n", (const char*) args->m_Args[i]->m_Value );
                    break;

                default:
                    printf ("Unknown argument type!\n");
                    break;
            }

        }
    }

    printf ("\n\tpress %s to exit program\n\n", DEF_EXIT_KOMBINATION_STR);
}

XWCOptions *
processArgs (Display    *  d,
             int           argCnt,
             const char ** argArr)
{
    KeySym         exitKeySym;
    arguments    * args;
    char         * endPtr;

    args = initArgs ();

    if (args == NULL)
    {
        printf ("Cannot allocate structure to process arguments!\n");
        return NULL;
    }

    if (   addArg (args, True,  C_STR, BGCOLOR,    "BGCOLOR",    1, "-bg"   )
        == False

        || addArg (args, True,  INT,   FRAMERATE,  "FRAMERATE",  1, "-fr"   )
        == False

        || addArg (args, True,  INT,   FOCUSTIME,  "FOCUSTIME",  1, "-ft"   )
        == False

        || addArg (args, True,  INT,   TOPOFFSET,  "TOPOFFSET",  1, "-toff" )
        == False

        || addArg (args, False, C_STR, HELP,       "HELP",       3, "-h",
                   "-help",
                   "--help")
        == False

        || addArg (args, True,  INT,   AUTOCENTER, "AUTOCENTER", 1, "-ac"   )
        == False
        )
    {
        delArgs (args);
        printf ("Cannot allocate structures for argument parameters!\n");
        return NULL;
    }

    int nextArgOffset = 2;

    for (int i = 1; i < argCnt; i += nextArgOffset)
    {
        for (int j = 0; j < OPTIONS_COUNT; ++j)
        {
            Bool argFound = False;
            for (int k = 0; k < args->m_Args[j]->m_SynCnt; ++k)
            {
                if (strcmp (argArr[i], args->m_Args[j]->m_SynStrs[k])
                    == STR_EQUAL)
                {
                    if (args->m_Args[j]->m_IsSet == True)
                    {
                        printf ("Parameter %s has been set several times!\n",
                                args->m_Args[j]->m_NameStr);
                        delArgs (args);
                        return NULL;
                    }

                    args->m_Args[j]->m_IsSet = True;
                    argFound                 = True;

                    switch (args->m_Args[j]->m_Type)
                    {
                        case INT:
                            *( (int*) args->m_Args[j]->m_Value ) =
                                strtol (argArr[i + 1], &endPtr, 10);

                            if (endPtr == argArr[i + 1])
                            {
                                printf ("Error parsing %s argument!\n\nTry:"
                                        "\n\n\t%s [-help | -h | --help]\n\n",
                                        args->m_Args[j]->m_NameStr,
                                        PROGRAM_EXE_NAME_STR);
                                delArgs (args);
                                return NULL;
                            }
                            break;

                        case C_STR:
                            args->m_Args[j]->m_Value = (void*) argArr[i + 1];
                            break;

                        default:
                            break;
                    }

                    break;
                }
            }

            if (argFound == False)
            {

                printf ("Unknown argument specified!\n\nTry:\n\n\t%s "
                        "[-help | -h | --help]\n\n", PROGRAM_EXE_NAME_STR);
                delArgs (args);
                return NULL;
            }
            else
            {
                nextArgOffset = ( args->m_Args[j]->m_HasValue == True ) ? 2 : 1;
                break;
            }

        }
    }

    if (args->m_Args[HELP]->m_IsSet == True)
    {
        printUsage (args);
        delArgs (args);
        return NULL;
    }

    XWCOptions * ret = (XWCOptions*) malloc (sizeof (XWCOptions ));

    if (ret == NULL)
    {
        printf ("Error allocating memory for options struct!\n");
        delArgs (args);
        return NULL;
    }

    memset (&ret->bgColor, 0, sizeof (ret->bgColor ));

    int fr = *(( int*) args->m_Args[FRAMERATE]->m_Value);
    int fd = *(( int*) args->m_Args[FOCUSTIME]->m_Value);

    ret->focusDelay.tv_nsec = 0;
    ret->focusDelay.tv_sec  = fd;
    ret->frameDelay.tv_nsec = ( 1.0 / fr) * 1000000000L;
    ret->frameDelay.tv_sec  = 0;
    ret->autoCenter         = *( ( int*) args->m_Args[AUTOCENTER]->m_Value );
    ret->topOffset          = *( ( int*) args->m_Args[TOPOFFSET]->m_Value );
    ret->bgColorStr         = (const char*) args->m_Args[BGCOLOR]->m_Value;
    //ret->exitKey            = exitKey;
    ret->exitKeyStr         = EXIT_KEY_STR;
    ret->exitKeyMask        = EXIT_MASK;

    if (( exitKeySym = XStringToKeysym (ret->exitKeyStr) ) == NoSymbol)
    {
        printf ("Error parsing exit key string (%s)\n", ret->exitKeyStr);
        free (ret);
        delArgs (args);
        return NULL;
    }

    if (! ( ret->exitKeyCode = XKeysymToKeycode (d, exitKeySym) ))
    {
        printf ("Unknown keycode %d\n", ret->exitKeyCode);
        free (ret);
        delArgs (args);
        return NULL;
    }

    printf ("\nSelected values:\n\n");

    printf ("\tfocus time\t%ld\n"
            "\tframe rate\t%ld, this results in %ldms frame delay\n"
            "\tautocentering\t%d\n"
            "\ttop offset\t%dpx\n"
            "\tbgcolor\t\t%s\n\n",
            ret->focusDelay.tv_sec,
            (__syscall_slong_t) ( 1000000000.0F / ret->frameDelay.tv_nsec ),
            ret->frameDelay.tv_nsec / 1000000,
            ret->autoCenter,
            ret->topOffset,
            ret->bgColorStr);

    delArgs (args);
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
grabExitKey (Display    * d,
             Window       grabWin,
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

void
ungrabExitKey (Display    * d,
               Window       grabWin,
               XWCOptions * prgCfg)
{

    if (d == NULL)
    {
        printf ("Cannot ungrab exit key combination: null pointer to X "
                "connection!\n");
        return;
    }

    if (prgCfg == NULL)
    {
        printf ("Cannot ungrab exit key combination: invalid pointer to options"
                " data structure!\n");
        return;
    }

    if (grabWin == None)
    {
        printf ("Cannot ungrab exit key combination: no window specified!\n");
        return;
    }

    XUngrabKey (d, prgCfg->exitKeyCode, prgCfg->exitKeyMask, grabWin);

    if (getXErrState () == True)
    {
        printf ("Cannot ungrab exit key combination: XUngrabKey error!\n");
        return;
    }
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
    else if (! XCompositeQueryVersion (d, &xCompExtVerMaj, &xCompExtVerMin))
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

#include "xwclib.h"

/*Global X_ERROR initialization*/
Bool X_ERROR = False;

/*Global LOG_LVL initialization*/
int LOG_LVL = DEFAULT_LOG_LVL;

Display *
openDefaultDisplay (void)
{
    Display * d;
    logCtr ("connecting to X server ... ", LOG_LVL_1);
    if (( d = XOpenDisplay (NULL) ) == NULL)
    {
        logCtr ("fail to connect to X server\n", LOG_LVL_NO);
        return NULL;
    }
    logCtr ("success\n", LOG_LVL_1);
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

    snprintf (buf, sizeof (buf), "ERROR: X11 error\n\terror code: %d\n",
              error->error_code);

    logCtr (buf, LOG_LVL_NO);

    XGetErrorText (display, error->error_code, buf, 1024);

    int tmpLen = strlen (buf);
    if (tmpLen < sizeof (buf) - 1 && tmpLen > 0)
    {
        buf[tmpLen] = '\n';
        buf[tmpLen] = '\0';
        logCtr (buf, LOG_LVL_NO);
    }

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
    if (d == NULL)
    {
        logCtr ("Error getting focused window: No display specified!\n",
                LOG_LVL_NO);
        return None;
    }

    /**
     * @todo find how to utilize revert_to
     */
    int    revert_to;
    Window w;
    char   buf[1024];

    logCtr ("getting input focus window ... ", LOG_LVL_1);
    XGetInputFocus (d, &w, &revert_to);

    if (getXErrState () == True)
    {
        logCtr ("fail to get focused window\n", LOG_LVL_NO);
        return None;
    }

    if (w == None)
    {
        logCtr ("no focus window\n", LOG_LVL_NO);
    }

    else
    {
        snprintf (buf, sizeof (buf), "success\n\twindow xid: %lX\n", w);
        logCtr (buf, LOG_LVL_1);
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
        logCtr ("Error getting top window: No display specified!\n",
                LOG_LVL_NO);
        return None;
    }

    if (start == None)
    {
        logCtr ("Error getting top window: Invalid window specified!\n",
                LOG_LVL_NO);
        return None;
    }

    logCtr ("getting child-of-root window ... \n", LOG_LVL_1);

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
            logCtr ("failed to get top-most window\n", LOG_LVL_NO);
            return None;
        }
        snprintf (buf, sizeof (buf), " got parent (window: %lX)\n", w);
        logCtr (buf, LOG_LVL_1);
    }

    snprintf (buf, sizeof (buf), "success (window: %lX)\n", w);
    logCtr (buf, LOG_LVL_1);

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
        logCtr ("Error getting named window: No display specified!\n",
                LOG_LVL_NO);
        return None;
    }

    if (start == None)
    {
        logCtr ("Error getting named window: Invalid window specified!\n",
                LOG_LVL_NO);
        return None;
    }

    logCtr ("getting named window:\n\t", LOG_LVL_1);

    w = XmuClientWindow (d, start);
    if (w == start)
    {
        logCtr ("fail to get named window or window already "
                "has WM_STATE property\n", LOG_LVL_NO);
    }

    snprintf (buf, sizeof (buf), "returning window: %lX\n", w);
    logCtr (buf, LOG_LVL_1);

    return w;
}

Window
getActiveWindow (Display    * d,
                 XWCOptions * prgCfg)
{
    Window w;

    if (d == NULL)
    {
        logCtr ("Error getting active window: No display specified!\n",
                LOG_LVL_NO);
        return None;
    }

    if (prgCfg == NULL)
    {
        logCtr ("Error getting active window: No program options"
                " data specified!\n", LOG_LVL_NO);
        return None;
    }

    if (prgCfg->isDaemon == False)
    {
        logCtr ("Waiting for focus to be moved to source window\n\t", LOG_LVL_NO);
        nanosleep (&prgCfg->focusDelay, NULL);
    }

    if ((w = prgCfg->srcWinId) == None)
    {
        w = getFocusedWindow (d);
    }

    if (     w == None
        || ( w = getTopWindow (d, w)   ) == None
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
    char             buf[1024];

    if (d == NULL)
    {
        logCtr ("Error printing window name: No display specified!\n",
                LOG_LVL_NO);
        return;
    }

    if (w == None)
    {
        logCtr ("Error printing window name: Invalid window specified!\n",
                LOG_LVL_NO);
        return;
    }

    memset (&prop, 0, sizeof (prop ));
    prop.value = NULL;

    logCtr ("window name:\n", LOG_LVL_1);

    if (getXErrState () == False && XGetWMName (d, w, &prop) != 0)
    {
        count  = 0;
        list   = NULL;
        result = XmbTextPropertyToTextList (d, &prop, &list, &count);

        if (result == Success)
        {
            snprintf (buf, 1024, "\t%s\n", list[0]);
            logCtr (buf, LOG_LVL_1);
        }
        else
        {
            logCtr ("Error printing window name: "
                    "XmbTextPropertyToTextList err\n", LOG_LVL_1);
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
        logCtr ("Error printing window name: XGetWMName err\n", LOG_LVL_NO);
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
        logCtr ("Error printing window class: No display specified!\n",
                LOG_LVL_NO);
        return;
    }

    if (w == None)
    {
        logCtr ("Error printing window class: Invalid window specified!\n",
                LOG_LVL_NO);
        return;
    }

    class = XAllocClassHint ();

    if (getXErrState () == True)
    {
        logCtr ("Error printing window class info: XAllocClassHint err\n",
                LOG_LVL_NO);
        if (class != NULL)
        {
            XFree (class);
        }
        return;
    }

    if (getXErrState () == False && XGetClassHint (d, w, class) != 0)
    {
        snprintf (buf, sizeof (buf), "application: \n\tname: %s\n\tclass: %s\n",
                  class->res_name, class->res_class);

        logCtr (buf, LOG_LVL_1);
    }
    else
    {
        logCtr ("Error printing window class info: XGetClassHint err\n",
                LOG_LVL_NO);
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
                "Bad X display pointer\n", LOG_LVL_NO);
        return;
    }

    if (w == None)
    {
        logCtr ("Error cannot print window information: No window specified\n",
                LOG_LVL_NO);
        return;
    }

    if (xWinAttr == NULL)
    {
        logCtr ("Error cannot print window information: "
                "No window attributes data specified\n", LOG_LVL_NO);
        return;
    }

    printWindowName (d, w);
    printWindowClass (d, w);

    snprintf (buf, sizeof (buf), "Width: %d\nHeight: %d\nDepth: %d\n",
              xWinAttr->width, xWinAttr->height, xWinAttr->depth);

    logCtr (buf, LOG_LVL_1);
}

Bool
setWinTitlebar (Display    * d,
                Window       WID,
                const char * name)
{
    XTextProperty tp;

    if (d == NULL)
    {
        logCtr ("Error changing window name: No display specified!\n",
                LOG_LVL_NO);
        return False;
    }

    if (name == NULL)
    {
        logCtr ("Error changing window name: Name string is not specified!\n",
                LOG_LVL_NO);
        return False;
    }

    if (WID == None)
    {
        logCtr ("Error changing window name: No window specified!\n",
                LOG_LVL_NO);
        return False;
    }

    memset (&tp, 0, sizeof (tp ));
    tp.value = NULL;

    /* As of Xlib version 1.6.3 XStringListToTextProperty doesn't affect its 
     * argv (first char**) argument (Xlib's SetTxtProp.c), 
     * so let's think it is const...*/
    if (XStringListToTextProperty ((char **) &name, 1, &tp) == False)
    {
        logCtr ("Error setting name of window: "
                "XStringListToTextProperty err\n", LOG_LVL_NO);
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
        logCtr ("Error setting name of window: "
                "XChangeProperty err\n", LOG_LVL_NO);
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
        logCtr ("Error changing window class: No display specified!\n" ,
                LOG_LVL_NO);
        return False;
    }

    if (permNameStr == NULL)
    {
        logCtr ("Error changing window class: Window permanent name string "
                "is not specified!\n", LOG_LVL_NO);
        return False;
    }

    if (classStr == NULL)
    {
        logCtr ("Error changing window class: Class string is not "
                "specified!\n", LOG_LVL_NO);
        return False;
    }

    if (WID == None)
    {
        logCtr ("Error changing window class: No window specified!\n",
                LOG_LVL_NO);
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
    char     buf[1024];
    /*
     * Maybe this color string parsing must take place 
     * while processing options...
     */
    snprintf (buf, sizeof (buf), "\nParsing window background color string "
              "\"%s\" ... ", prgCfg->bgColorStr);
    logCtr (buf, LOG_LVL_1);

    char * bgClrStrTmp = (char*) malloc (8 * sizeof (char ));
    bgClrStrTmp[0]     = '#';
    bgClrStrTmp[7]     = '\0';
    memcpy (bgClrStrTmp + 1, prgCfg->bgColorStr, 6);

    if ( XParseColor (d, xClrMap, bgClrStrTmp, &prgCfg->bgColor) == 0
        || XAllocColor (d, xClrMap, &prgCfg->bgColor) == 0)
    {
        logCtr ("Error:\n\tXParseColor and/or XAllocColor error\n", LOG_LVL_NO);
        free (bgClrStrTmp);
        return False;
    }

    free (bgClrStrTmp);

    logCtr ("Success\n", LOG_LVL_1);

    /* redundant information
    printf ("Color parsing result: pixel=%ld, red=%d, green=%d, blue=%d\n",
            xColor.pixel, xColor.red, xColor.green, xColor.blue);
     */
    return True;
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
        argument * arg;
        for (int i = 0; i < args->m_ArgCnt; ++ i)
        {
            if ((arg = args->m_Args[i]) != NULL)
            {
                if (arg->m_Value != NULL)
                {
                    if (arg->m_Type == INT)
                    {
                        free (arg->m_Value);
                    }

                    if (arg->m_Type == ULONG)
                    {
                        free (arg->m_Value);
                    }
                }

                if ( arg->m_SynStrs != NULL)
                {
                    free (arg->m_SynStrs);
                }

                free (arg);
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

    for (int i = 0; i < args->m_ArgCnt; ++ i)
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
        va_end (argStrList);
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
        va_end (argStrList);
        return False;
    }

    for (int i = 0; i < argSynCnt; ++ i)
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

            case ULONG:
                arg->m_Value = (unsigned long*) malloc (sizeof (unsigned long));
                break;

            case C_STR:
                //arg->m_Value = (char**) malloc (sizeof (char*));
                break;

            default:
                logCtr ("Error adding argument, bad type specified!\n",
                        LOG_LVL_NO);
                free (arg->m_SynStrs);
                va_end (argStrList);
                return False;
        }
    }

    switch (arg->m_Name)
    {
        case HELP:
            break;

        case DAEMON:
            break;

        case AUTOCENTER:
            *( (int*) arg->m_Value )           = AUTOCENTERING;
            break;

        case TOPOFFSET:
            *( (int*) arg->m_Value )           = TOP_OFFSET;
            break;

        case FOCUSTIME:
            *( (int*) arg->m_Value )           = TIME_TO_CHANGE_FOCUS_SEC;
            break;

        case BGCOLOR:
            arg->m_Value                       = (void*) DEFAULT_BG_COLOR;
            break;

        case FRAMERATE:
            *( (int*) arg->m_Value )           = FRAMERATE_FPS;
            break;

        case LOGLVL:
            *( (int*) arg->m_Value )           = DEFAULT_LOG_LVL;
            break;

        case SOURCEID:
            *( (unsigned long*) arg->m_Value ) = None;
            break;

        default:
            logCtr ("Unknown argument type detected while creating option!\n",
                    LOG_LVL_NO);
            va_end (argStrList);
            return False;
    }

    va_end (argStrList);

    return True;
}

void
printCurValues (arguments  * args)
{
    printf ("\n\n\tCurrent values (default value if no corresponding prompt "
            "arg provided):\n\n");

    for (int i = 0; i < args->m_ArgCnt; ++ i)
    {
        if (args->m_Args[i]->m_HasValue == True)
        {
            if (args->m_Args[i]->m_IsSet == True)
            {
                printf ("\t\t%-10s\t%-10s\t", args->m_Args[i]->m_NameStr,
                        "is set");
            }
            else
            {
                printf ("\t\t%-10s\t%-10s\t", args->m_Args[i]->m_NameStr,
                        "default");
            }
            switch (args->m_Args[i]->m_Type)
            {
                case INT:
                    printf ("%d\n", *( (int*) args->m_Args[i]->m_Value ));
                    break;

                case C_STR:
                    printf ("%s\n", (const char*) args->m_Args[i]->m_Value );
                    break;

                case ULONG:
                    printf ("0x%lx\n",
                            *( (unsigned long*) args->m_Args[i]->m_Value ));
                    break;

                default:
                    logCtr ("Unknown argument type detected during arguments "
                            "list traversing!\n", LOG_LVL_NO);
                    break;
            }
        }
    }

    printf ("\n\tpress %s to exit program\n\n", EXIT_STR);

    printf ("\n\tuse %s combination for translation control\n\n",
            TRANSLATION_CTRL_STR);
}

void
printVersion (void)
{
    printf ("\n%s version %s\n", WM_CLASS_PRG_NAME_STR,
            XWINCLONE_VERSION_STR);
}

void
printUsage (arguments  * args)
{
    if (args == NULL)
    {
        return;
    }

    printVersion ();

    printf ("\nUSAGE:\n\n\t%s ", PROGRAM_EXE_NAME_STR);

    for (int i = 0; i < args->m_ArgCnt; ++ i)
    {
        printf ("[");
        for (int j = 0; j < args->m_Args[i]->m_SynCnt; ++ j)
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

    printCurValues (args);
}

XWCOptions *
processArgs (Display    *  d,
             int           argCnt,
             const char ** argArr)
{
    KeySym          exitKeySym;
    arguments     * args;
    char          * endPtr;
    int             nextArgOffset, i, j, k, fr;
    unsigned long   srcid;
    char            buf[2048];

    args = initArgs ();

    if (args == NULL)
    {
        logCtr ("Cannot allocate structure to process arguments!\n",
                LOG_LVL_NO);
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

        || addArg (args, True,  INT,   LOGLVL,     "LOGLEVEL",   1, "-ll"   )
        == False

        || addArg (args, True,  ULONG, SOURCEID,   "SOURCEID",   1, "-srcid")
        == False

        || addArg (args, False, INT,   DAEMON,     "DAEMON",     2, "-d",
                   "-daemon")
        == False
        )
    {
        delArgs (args);
        logCtr ("Cannot add new argument to list!\n", LOG_LVL_NO);
        return NULL;
    }

    nextArgOffset = 2;

    for (i = 1; i < argCnt; i += nextArgOffset)
    {
        Bool argFound = False;
        for (j = 0; j < OPTIONS_COUNT; ++ j)
        {
            for (k = 0; k < args->m_Args[j]->m_SynCnt; ++ k)
            {
                if (strcmp (argArr[i], args->m_Args[j]->m_SynStrs[k])
                    == STR_EQUAL)
                {
                    if (args->m_Args[j]->m_IsSet == True)
                    {
                        snprintf (buf, sizeof (buf), "Parameter %s has been "
                                  "set several times!\n",
                                  args->m_Args[j]->m_NameStr);
                        logCtr (buf, LOG_LVL_NO);
                        delArgs (args);
                        return NULL;
                    }

                    args->m_Args[j]->m_IsSet = True;
                    argFound                 = True;

                    if ( args->m_Args[j]->m_HasValue == True)
                    {
                        switch (args->m_Args[j]->m_Type)
                        {
                            case INT:
                                *( (int*) args->m_Args[j]->m_Value ) =
                                    strtol (argArr[i + 1], &endPtr, 10);

                                if (endPtr == argArr[i + 1])
                                {
                                    snprintf (buf, sizeof (buf), "Error parsing "
                                              "%s argument!\n\nTry:\n\n\t%s [-help "
                                              "| -h | --help]\n\n",
                                              args->m_Args[j]->m_NameStr,
                                              PROGRAM_EXE_NAME_STR);
                                    logCtr (buf, LOG_LVL_NO);
                                    delArgs (args);
                                    return NULL;
                                }
                                break;

                            case ULONG:
                                *( (unsigned long*) args->m_Args[j]->m_Value ) =
                                    strtol (argArr[i + 1], &endPtr, 0);

                                if (endPtr == argArr[i + 1])
                                {
                                    snprintf (buf, sizeof (buf), "Error parsing "
                                              "%s argument!\n\nTry:\n\n\t%s [-help "
                                              "| -h | --help]\n\n",
                                              args->m_Args[j]->m_NameStr,
                                              PROGRAM_EXE_NAME_STR);
                                    logCtr (buf, LOG_LVL_NO);
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
                    }
                    break;
                }
            }
            if (argFound == True)
            {
                break;
            }
        }
        if (argFound == False)
        {
            snprintf (buf, sizeof (buf), "Unknown argument specified!\n\nTry:\n"
                      "\n\t%s [-help | -h | --help]\n\n", PROGRAM_EXE_NAME_STR);
            logCtr (buf, LOG_LVL_NO);
            delArgs (args);
            return NULL;
        }
        else
        {
            nextArgOffset = ( args->m_Args[j]->m_HasValue == True ) ? 2 : 1;
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
        logCtr ("Error allocating memory for options struct!\n", LOG_LVL_NO);
        delArgs (args);
        return NULL;
    }

    /*boilerplate wa*/
    LOG_LVL = * ((int*)           args->m_Args[LOGLVL]->m_Value);
    fr      = * ((int*)           args->m_Args[FRAMERATE]->m_Value);
    srcid   = * ((unsigned long*) args->m_Args[SOURCEID]->m_Value);

    memset (&ret->bgColor, 0, sizeof (ret->bgColor ));

    ret->focusDelay.tv_nsec     = 0;
    ret->focusDelay.tv_sec      = * ((int*) args->m_Args[FOCUSTIME]->m_Value);
    ret->frameDelay.tv_nsec     = ( 1.00000001 / fr) * 1000000000L;
    ret->frameDelay.tv_sec      = 0;
    ret->autoCenter             = * ((int*) args->m_Args[AUTOCENTER]->m_Value);
    ret->topOffset              = * ((int*) args->m_Args[TOPOFFSET]->m_Value);
    ret->bgColorStr             = (const char*) args->m_Args[BGCOLOR]->m_Value;
    ret->exitKeyStr             = EXIT_KEY;
    ret->exitKeyMask            = EXIT_MASK;
    ret->translationCtrlKeyStr  = TRANSLATION_CTRL_KEY;
    ret->translationCtrlKeyMask = TRANSLATION_CTRL_MASK;
    ret->srcWinId               = srcid;
    ret->isDaemon               = args->m_Args[DAEMON]->m_IsSet;

    if (( exitKeySym = XStringToKeysym (ret->exitKeyStr) ) == NoSymbol)
    {
        snprintf (buf, sizeof (buf), "Error parsing exit key string (%s)\n",
                  ret->exitKeyStr);
        logCtr (buf, LOG_LVL_NO);
        free (ret);
        delArgs (args);
        return NULL;
    }

    if (! ( ret->exitKeyCode = XKeysymToKeycode (d, exitKeySym) ))
    {
        snprintf (buf, sizeof (buf), "Unknown keycode %d\n", ret->exitKeyCode);
        logCtr (buf, LOG_LVL_NO);
        free (ret);
        delArgs (args);
        return NULL;
    }

    if (( exitKeySym = XStringToKeysym (ret->translationCtrlKeyStr) ) == NoSymbol)
    {
        snprintf (buf, sizeof (buf), "Error parsing exit key string (%s)\n",
                  ret->translationCtrlKeyStr);
        logCtr (buf, LOG_LVL_NO);
        free (ret);
        delArgs (args);
        return NULL;
    }

    if (! ( ret->translationCtrlKeyCode = XKeysymToKeycode (d, exitKeySym) ))
    {
        snprintf (buf, sizeof (buf), "Unknown keycode %d\n",
                  ret->translationCtrlKeyCode);
        logCtr (buf, LOG_LVL_NO);
        free (ret);
        delArgs (args);
        return NULL;
    }

    if (LOG_LVL > LOG_LVL_NO)
    {
        printVersion ();
        printCurValues (args);
    }

    delArgs (args);
    return ret;
}

Screen *
getScreenByWindowAttr (Display           * d,
                       XWindowAttributes * winAttr)
{
    if (d == NULL)
    {
        logCtr ("Error getting screen by window attrinutes: "
                "No display spicified\n", LOG_LVL_NO);
        return NULL;
    }

    if (winAttr == NULL)
    {
        logCtr ("Error getting screen by window attrinutes: "
                "No window attributes struct spicified\n", LOG_LVL_NO);
        return NULL;
    }

    if (winAttr->screen == NULL)
    {
        logCtr ("Error getting screen by window attrinutes: No valid screen "
                "pointer found in  window attributes struct\n", LOG_LVL_NO);
        return NULL;
    }

    return winAttr->screen;
}

Window
getRootWinOfScr (Screen * s)
{
    if (s == NULL)
    {
        logCtr ("Error getting root window of screen: "
                "Invalid pointer to Screen data struct!\n", LOG_LVL_NO);
        return None;
    }

    if (s->root == None)
    {
        logCtr ("Error getting root window of screen: "
                "No root wondow specified for given screen!\n", LOG_LVL_NO);
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
        logCtr ("Cannot grab exit key combination: null pointer to X "
                "connection!\n", LOG_LVL_NO);
        return False;
    }

    if (prgCfg == NULL)
    {
        logCtr ("Cannot grab exit key combination: invalid pointer to options "
                "data structure!\n", LOG_LVL_NO);
        return False;
    }

    if (grabWin == None)
    {
        logCtr ("Cannot grab exit key combination: no window specified!\n",
                LOG_LVL_NO);
        return False;
    }

    XGrabKey (d, prgCfg->exitKeyCode, prgCfg->exitKeyMask, grabWin, False,
              GrabModeAsync, GrabModeAsync);

    if (getXErrState () == True)
    {
        logCtr ("Cannot grab exit key combination: XGrabKey error!\n",
                LOG_LVL_NO);
        return False;
    }

    XSelectInput (d, grabWin,  KeyPressMask);
    /*XSelectInput only throws badwindow which we've already checked*/
    return True;
}

Bool
grabTranslationCtrlKey (Display    * d,
                        Window       grabWin,
                        XWCOptions * prgCfg)
{
    if (d == NULL)
    {
        logCtr ("Cannot grab translation control key combination: null pointer to X "
                "connection!\n", LOG_LVL_NO);
        return False;
    }

    if (prgCfg == NULL)
    {
        logCtr ("Cannot grab translation control key combination: invalid pointer to options "
                "data structure!\n", LOG_LVL_NO);
        return False;
    }

    if (grabWin == None)
    {
        logCtr ("Cannot grab translation control key combination: no window specified!\n",
                LOG_LVL_NO);
        return False;
    }

    XGrabKey (d, prgCfg->translationCtrlKeyCode, prgCfg->translationCtrlKeyMask, grabWin, False,
              GrabModeAsync, GrabModeAsync);

    if (getXErrState () == True)
    {
        logCtr ("Cannot grab translation control key combination: XGrabKey error!\n",
                LOG_LVL_NO);
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
        logCtr ("Cannot ungrab exit key combination: null pointer to X "
                "connection!\n", LOG_LVL_NO);
        return;
    }

    if (prgCfg == NULL)
    {
        logCtr ("Cannot ungrab exit key combination: invalid pointer to options"
                " data structure!\n", LOG_LVL_NO);
        return;
    }

    if (grabWin == None)
    {
        logCtr ("Cannot ungrab exit key combination: no window specified!\n",
                LOG_LVL_NO);
        return;
    }

    XUngrabKey (d, prgCfg->exitKeyCode, prgCfg->exitKeyMask, grabWin);

    if (getXErrState () == True)
    {
        logCtr ("Cannot ungrab exit key combination: XUngrabKey error!\n",
                LOG_LVL_NO);
        return;
    }
}

void
ungrabTranslationCtrlKey (Display    * d,
                          Window       grabWin,
                          XWCOptions * prgCfg)
{
    if (d == NULL)
    {
        logCtr ("Cannot ungrab translation control key combination: null pointer to X "
                "connection!\n", LOG_LVL_NO);
        return;
    }

    if (prgCfg == NULL)
    {
        logCtr ("Cannot ungrab translation control key combination: invalid pointer to options"
                " data structure!\n", LOG_LVL_NO);
        return;
    }

    if (grabWin == None)
    {
        logCtr ("Cannot ungrab translation control key combination: no window specified!\n",
                LOG_LVL_NO);
        return;
    }

    XUngrabKey (d, prgCfg->translationCtrlKeyCode, prgCfg->translationCtrlKeyMask, grabWin);

    if (getXErrState () == True)
    {
        logCtr ("Cannot ungrab translation control key combination: XUngrabKey error!\n",
                LOG_LVL_NO);
        return;
    }
}

Bool
chkCompExt (Display * d)
{
    int  compositeErrorBase, compositeEventBase, xCompExtVerMin, xCompExtVerMaj;
    char buf[1024];

    XSetErrorHandler (errorHandlerBasic);

    if (XCompositeQueryExtension (d, &compositeEventBase,
                                  &compositeErrorBase) == False)
    {
        logCtr ("No composite extension found, aborting...\n", LOG_LVL_NO);
        return False;
    }
    else if (! XCompositeQueryVersion (d, &xCompExtVerMaj, &xCompExtVerMin))
    {
        logCtr ("X Server doesn't support such a version of the X Composite "
                "Extension which is compatible with the client library\n",
                LOG_LVL_NO);
        return False;
    }
    else if ( ( xCompExtVerMaj < 1 ) &&  ( xCompExtVerMin < 2 ) )
    {
        logCtr ("Unsupported version of X composite extension (<0.2)\n",
                LOG_LVL_NO);
        return False;
    }
    else
    {
        snprintf (buf, sizeof (buf), "Composite extension ready, version %d.%d"
                  "\n", xCompExtVerMaj, xCompExtVerMin);
        logCtr (buf, LOG_LVL_1);
    }
    return True;
}

void
logCtr (const char * msg,
        int          lvl)
{
    if (msg == NULL)
    {
        printf ("Tryng to log NULL msg!\n");
        return;
    }

    if (lvl < 0)
    {
        printf ("Tryng to log with unknown lvl!\n");
        return;
    }

    if (LOG_LVL >= lvl)
    {
        printf ("%s", msg);
    }
}

Window
getDefaultRootWindow (Display * d)
{
    Window w;

    w = DefaultRootWindow (d);

    if (w == None)
    {
        logCtr ("Cannot get default root window of display\n", LOG_LVL_NO);
    }

    return w;
}

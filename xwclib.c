#include "xwclib.h"

/*Global X_ERROR initialization*/
Bool X_ERROR = False;

imgLibCreateCrSc_t   imgLibCreateCrSc = & imlib_create_cropped_scaled_image;

XCompRedirWin_t      redirWin         = & XCompositeRedirectWindow;
XCompUnRedirWin_t    unRedirWin       = & XCompositeUnredirectWindow;
XCompRedirSubWin_t   redirSubWin      = & XCompositeRedirectSubwindows;
XCompUnRedirSubWin_t unRedirSubWin    = & XCompositeUnredirectSubwindows;

/*Global LOG_LVL initialization*/
int LOG_LVL = DEFAULT_LOG_LVL;

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
    Window w;

    if (ctx == NULL)
    {
        logCtr ("Error getting active window: No program options"
                " data specified!", LOG_LVL_NO, False);
        return None;
    }

    if (ctx->xDpy == NULL)
    {
        logCtr ("Error getting active window: No display specified!",
                LOG_LVL_NO, False);
        return None;
    }

    if (ctx->isDaemon == False)
    {
        logCtr ("Waiting for focus to be moved to source window", LOG_LVL_NO,
                False);
        nanosleep (&ctx->focusDelay, NULL);
    }

    if ((w = ctx->srcWin) == None)
    {
        w = getFocusedWindow (ctx->xDpy);
    }

    if (     w == None
        || ( w = getTopWindow (ctx->xDpy, w)   ) == None
        || ( w = getNamedWindow (ctx->xDpy, w) ) == None )
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
                logCtr ("Error adding argument, bad type specified!",
                        LOG_LVL_NO, False);
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

        case SINGLEINST:
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

        case BGIMAGE:
            arg->m_Value                       = (void*) DEFAULT_BGIMAGE_PATH;
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
            logCtr ("Unknown argument type detected while creating option!",
                    LOG_LVL_NO, False);
            va_end (argStrList);
            return False;
    }

    va_end (argStrList);

    return True;
}

void
printCurValues (arguments  * args)
{
    printf ("\n\tCurrent values (default value if no corresponding prompt "
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
                            "list traversing!\n", LOG_LVL_NO, False);
                    break;
            }
        }
    }

    printf ("\n\tpress %s to exit program\n", EXIT_STR);

    printf ("\n\tuse %s combination for translation control\n\n",
            TRANSLATION_CTRL_STR);
}

void
printVersion (void)
{
    char buf[1024];
    snprintf (buf, sizeof (buf), "%s version %s", WM_CLASS_PRG_NAME_STR,
              XWINCLONE_VERSION_STR);
    logCtr (buf, LOG_LVL_NO, False);
}

void
printUsage (arguments  * args)
{
    if (args == NULL)
    {
        return;
    }

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

XWCContext *
init (int           argCnt,
      const char ** argArr)
{
    KeySym              exitKeySym;
    arguments         * args;
    char              * endPtr;
    int                 nextArgOffset, i, j, k, fr;
    unsigned long       srcid;
    char                buf[2048];
    XWCContext        * ctx;

    ctx = (XWCContext*) malloc (sizeof (XWCContext ));

    if (ctx == NULL)
    {
        logCtr ("\tError allocating memory for options struct!", LOG_LVL_NO,
                True);
        return NULL;
    }

    /*Make a program to be portable to all locales*/
    setlocale (LC_ALL, "");

    if (( ctx->xDpy = openDefaultDisplay () )  == NULL)
    {
        return NULL;
    }

    if (chkCompExt (ctx->xDpy) == False)
    {
        XCloseDisplay (ctx->xDpy);
        return NULL;
    }

    if ((ctx->rootWin = getDefaultRootWindow (ctx->xDpy)) == None)
    {
        XCloseDisplay (ctx->xDpy);
        return NULL;
    }

    XGetWindowAttributes (ctx->xDpy, ctx->rootWin, &ctx->rootWinAttr);

    if (getXErrState () == True)
    {
        XCloseDisplay (ctx->xDpy);
        return NULL;
    }

    if (( ctx->xScr = getScreenByWindowAttr (ctx, &ctx->rootWinAttr) ) == NULL)
    {
        XCloseDisplay (ctx->xDpy);
        return EXIT_FAILURE;
    }

    logCtr ("Processing arguments and building configuration:", LOG_LVL_1,
            False);

    args = initArgs ();

    if (args == NULL)
    {
        logCtr ("\tCannot allocate structure to process arguments!",
                LOG_LVL_NO, True);
        return NULL;
    }

    if (   addArg (args, True,  C_STR, BGCOLOR,    "BGCOLOR",    1, "-bgclr")
        == False

        || addArg (args, True,  C_STR, BGIMAGE,    "BGIMAGE",    1, "-bgimg")
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

        || addArg (args, False, INT,   SINGLEINST, "SINGLEINST", 2, "-si",
                   "-single")
        == False

        || addArg (args, False, INT,   DAEMON,     "DAEMON",     2, "-d",
                   "-daemon")
        == False
        )
    {
        delArgs (args);
        logCtr ("\tCannot add new argument to list!\n", LOG_LVL_NO, True);
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
                                  "set several times!",
                                  args->m_Args[j]->m_NameStr);
                        logCtr (buf, LOG_LVL_NO, True);
                        delArgs (args);
                        return NULL;
                    }

                    args->m_Args[j]->m_IsSet = True;
                    argFound                 = True;

                    if ( args->m_Args[j]->m_HasValue == False)
                    {
                        break;
                    }
                    switch (args->m_Args[j]->m_Type)
                    {
                        case INT:
                            *( (int*) args->m_Args[j]->m_Value ) =
                                strtol (argArr[i + 1], &endPtr, 10);

                            if (endPtr == argArr[i + 1])
                            {
                                snprintf (buf, sizeof (buf), "\tError parsing "
                                          "%s argument!\n\n\t\tTry:\n\n\t%s "
                                          "[-help | -h | --help]\n",
                                          args->m_Args[j]->m_NameStr,
                                          PROGRAM_EXE_NAME_STR);
                                logCtr (buf, LOG_LVL_NO, True);
                                delArgs (args);
                                return NULL;
                            }
                            break;

                        case ULONG:
                            *( (unsigned long*) args->m_Args[j]->m_Value ) =
                                strtol (argArr[i + 1], &endPtr, 0);

                            if (endPtr == argArr[i + 1])
                            {
                                snprintf (buf, sizeof (buf), "\tError parsing "
                                          "%s argument!\n\nTry:\n\n\t%s [-help "
                                          "| -h | --help]\n",
                                          args->m_Args[j]->m_NameStr,
                                          PROGRAM_EXE_NAME_STR);
                                logCtr (buf, LOG_LVL_NO, True);
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
            if (argFound == True)
            {
                break;
            }
        }
        if (argFound == False)
        {
            snprintf (buf, sizeof (buf), "\tUnknown argument specified!\n\nTry:"
                      "\n\n\t%s [-help | -h | --help]\n", PROGRAM_EXE_NAME_STR);
            logCtr (buf, LOG_LVL_NO, True);
            delArgs (args);
            return NULL;
        }
        else
        {
            nextArgOffset = ( args->m_Args[j]->m_HasValue == True ) ? 2 : 1;
        }
    }

    /*boilerplate wa*/
    LOG_LVL = * ((int*)           args->m_Args[LOGLVL]->m_Value);
    fr      = * ((int*)           args->m_Args[FRAMERATE]->m_Value);
    srcid   = * ((unsigned long*) args->m_Args[SOURCEID]->m_Value);

    memset (&ctx->bgColor, 0, sizeof (ctx->bgColor ));

    ctx->focusDelay.tv_nsec     = 0;
    ctx->focusDelay.tv_sec      = * ((int*) args->m_Args[FOCUSTIME]->m_Value);
    ctx->frameDelay.tv_nsec     = ( 1.00000001 / fr) * 1000000000L;
    ctx->frameDelay.tv_sec      = 0;
    ctx->longDelay.tv_sec       = 0;
    ctx->longDelay.tv_nsec      = LONG_WAIT * 1000000L;
    ctx->autoCenter             = * ((int*) args->m_Args[AUTOCENTER]->m_Value);
    ctx->topOffset              = * ((int*) args->m_Args[TOPOFFSET]->m_Value);
    ctx->bgColorStr             = (const char*) args->m_Args[BGCOLOR]->m_Value;
    ctx->bgImgFileStr           = (const char*) args->m_Args[BGIMAGE]->m_Value;
    ctx->bgImgFileSet           = args->m_Args[BGIMAGE]->m_IsSet;
    ctx->bgImgStatus            = False;
    ctx->exitKeyStr             = EXIT_KEY;
    ctx->exitKeyMask            = EXIT_MASK;
    ctx->transCtrlKeyStr        = TRANSLATION_CTRL_KEY;
    ctx->cloneKeyMask       = TRANSLATION_CTRL_MASK;
    ctx->srcWin                 = srcid;
    ctx->isDaemon               = args->m_Args[DAEMON]->m_IsSet;
    ctx->isSingleton            = args->m_Args[SINGLEINST]->m_IsSet;

    ctx->isSingleton            = ctx->isSingleton || ctx->isDaemon;

    if (( exitKeySym = XStringToKeysym (ctx->exitKeyStr) ) == NoSymbol)
    {
        snprintf (buf, sizeof (buf), "\tError parsing exit key string (%s)",
                  ctx->exitKeyStr);
        logCtr (buf, LOG_LVL_NO, True);
        free (ctx);
        delArgs (args);
        return NULL;
    }

    if ((ctx->exitKeyCode = XKeysymToKeycode (ctx->xDpy, exitKeySym)) == 0)
    {
        snprintf (buf, sizeof (buf), "\tUnknown keycode %d", ctx->exitKeyCode);
        logCtr (buf, LOG_LVL_NO, True);
        free (ctx);
        delArgs (args);
        return NULL;
    }

    if (( exitKeySym = XStringToKeysym (ctx->transCtrlKeyStr) )
        == NoSymbol)
    {
        snprintf (buf, sizeof (buf), "\tError parsing exit key string (%s)",
                  ctx->transCtrlKeyStr);
        logCtr (buf, LOG_LVL_NO, True);
        free (ctx);
        delArgs (args);
        return NULL;
    }

    if (( ctx->cloneKeyCode = XKeysymToKeycode (ctx->xDpy, exitKeySym) ) == 0)
    {
        snprintf (buf, sizeof (buf), "\tUnknown keycode %d",
                  ctx->cloneKeyCode);
        logCtr (buf, LOG_LVL_NO, True);
        free (ctx);
        delArgs (args);
        return NULL;
    }

    if (args->m_Args[HELP]->m_IsSet == True)
    {
        printUsage (args);
        delArgs (args);
        return NULL;
    }

    if (ctx->isSingleton == True && ifSingleInst () == False)
    {
        XCloseDisplay (ctx->xDpy);
        free (ctx);
        return EXIT_FAILURE;
    }

    if (parseColor (ctx) == False)
    {
        XCloseDisplay (ctx->xDpy);
        free (ctx);
        return EXIT_FAILURE;
    }

    if (grabKeys (ctx) == False)
    {
        XCloseDisplay (ctx->xDpy);
        return EXIT_FAILURE;
    }

    //XSelectInput (ctx->xDpy, ctx->rootWin,  KeyPressMask);
    /*XSelectInput only throws BadWindow which we've already checked*/

    if (LOG_LVL > LOG_LVL_NO)
    {
        printCurValues (args);
    }

    logCtr ("\tconfiguration successfully completed", LOG_LVL_1, True);

    delArgs (args);
    return ctx;
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

Bool
grabKeys (XWCContext * ctx)
{
    logCtr ("Trying to grab key combinations:", LOG_LVL_1, False);

    if (ctx == NULL)
    {
        logCtr ("\tCannot grab key combinations: null pointer to program"
                " context!", LOG_LVL_NO, True);
        return False;
    }

    if (ctx->xDpy == NULL)
    {
        logCtr ("\tCannot grab key combinations: null pointer to X "
                "connection!", LOG_LVL_NO, True);
        return False;
    }

    if (ctx->rootWin == None)
    {
        logCtr ("\tCannot grab key combinations: no root window specified!",
                LOG_LVL_NO, True);
        return False;
    }

    logCtr ("\tTrying to grab exit key combination:", LOG_LVL_1, False);

    XGrabKey (ctx->xDpy, ctx->exitKeyCode, ctx->exitKeyMask, ctx->rootWin, True,
              GrabModeAsync, GrabModeAsync);

    XSync (ctx->xDpy, 0);

    if (getXErrState () == True)
    {
        logCtr ("\t\tCannot grab exit key combination: XGrabKey error! (Another"
                " program may had already grabbed such a combination)",
                LOG_LVL_NO, True);
        return False;
    }

    logCtr ("\t\tsuccess", LOG_LVL_1, True);


    logCtr ("\tTrying to grab translation control key combination:", LOG_LVL_1,
            False);

    XGrabKey (ctx->xDpy, ctx->cloneKeyCode, ctx->cloneKeyMask,
              ctx->rootWin, True,
              GrabModeAsync, GrabModeAsync);

    XSync (ctx->xDpy, 0);

    if (getXErrState () == True)
    {
        ungrabKeys (ctx);
        logCtr ("\t\tCannot grab translation control key combination: XGrabKey"
                " error! (Another program may had already grabbed such a"
                " combination)", LOG_LVL_NO, True);
        return False;
    }

    logCtr ("\t\tsuccess", LOG_LVL_1, True);

    return True;
}

void
ungrabKeys (XWCContext * ctx)
{

    if (ctx == NULL)
    {
        logCtr ("Cannot ungrab key combinations: invalid pointer to program"
                " context!", LOG_LVL_NO, False);
        return;
    }

    if (ctx->xDpy == NULL)
    {
        logCtr ("Cannot ungrab key combinations: null pointer to X "
                "connection!", LOG_LVL_NO, False);
        return;
    }


    if (ctx->rootWin == None)
    {
        logCtr ("Cannot ungrab key combinations: no window specified!",
                LOG_LVL_NO, False);
        return;
    }

    XUngrabKey (ctx->xDpy, ctx->exitKeyCode, ctx->exitKeyMask, ctx->rootWin);

    if (getXErrState () == True)
    {
        logCtr ("Cannot ungrab exit key combination: XUngrabKey error!",
                LOG_LVL_NO, False);
        return;
    }

    XUngrabKey (ctx->xDpy, ctx->cloneKeyCode, ctx->cloneKeyMask,
                ctx->rootWin);

    if (getXErrState () == True)
    {
        logCtr ("Cannot ungrab translation control key combination: XUngrabKey"
                " error!", LOG_LVL_NO, False);
        return;
    }
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

void
logCtr (const char * msg,
        int          lvl,
        Bool         sequenced)
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
        if (sequenced == False)
        {
            printf ("\n%s\n", msg);
        }
        else
        {
            printf ("%s\n", msg);
        }
    }
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
ifSingleInst (void)
{
    int pid_file = open (LOCK_FILE_PATH, O_CREAT | O_RDWR, 0666);
    int rc = flock (pid_file, LOCK_EX | LOCK_NB);

    if (rc != 0 && EWOULDBLOCK == errno)
    {
        char buf[1024];
        snprintf (buf, 1024, "File %s seems to be already created and locked,\n"
                  "assuming other instance of this program is running",
                  LOCK_FILE_PATH);
        logCtr (buf, LOG_LVL_NO, False);
        return False;
    }

    return True;
}

int
getPressedComb (XWCContext * ctx)
{
    XEvent xEvent;
/*
    Bool   res;
    Window rRetW, chRetW;
    int rX, rY;
    int trgX, trgY;
    unsigned int mask_return;
*/
    char buf[1024];
    while (XPending (ctx->xDpy) != 0)
    {
        XNextEvent (ctx->xDpy, &xEvent);
        switch (xEvent.type)
        {

                /*
                                //printf ("Btn: %d\n", xEvent.xbutton.button);
                                if (xEvent.xbutton.button != Button1)
                                {
                                    continue;
                                }

                                res = XQueryPointer (ctx->xDpy, ctx->trgWin, &rRetW, &chRetW,
                                                     &rX, &rY, &trgX, &trgY, &mask_return);
                                if (res == False)
                                {
                                    logCtr ("Error getting pointer coordinates!\n", LOG_LVL_NO,
                                            False);
                                    continue;
                                }

                                memset (&xEvent, 0x00, sizeof (xEvent));

                                xEvent.xbutton.button      = Button1;
                                xEvent.xbutton.same_screen = True;
                                xEvent.xbutton.root        = ctx->rootWin;
                                xEvent.xbutton.subwindow   = None;
                                xEvent.xbutton.window      = ctx->srcWin;
                                xEvent.xbutton.x           = trgX;
                                xEvent.xbutton.y           = trgY + (ctx->topOffset / 2);



                                if (XSendEvent (ctx->xDpy, ctx->srcWin, True, 0xfff, &xEvent) == 0) fprintf (stderr, "Error\n");

                                XFlush (ctx->xDpy);

                                nanosleep (&ctx->longDelay, NULL);

                                xEvent.type          = ButtonRelease;
                                xEvent.xbutton.state = 0x100;

                                if (XSendEvent (ctx->xDpy, ctx->srcWin, True, ButtonReleaseMask, &xEvent) == 0) fprintf (stderr, "Error\n");

                                XFlush (ctx->xDpy);
                 */

     
                /*
                                //printf ("Btn: %d\n", xEvent.xbutton.button);
                                if (xEvent.xbutton.button != Button1)
                                {
                                    continue;
                                }

                                res = XQueryPointer (ctx->xDpy, ctx->trgWin, &rRetW, &chRetW,
                                                     &rX, &rY, &trgX, &trgY, &mask_return);
                                if (res == False)
                                {
                                    logCtr ("Error getting pointer coordinates!\n", LOG_LVL_NO,
                                            False);
                                    continue;
                                }

                                memset (&xEvent, 0x00, sizeof (xEvent));

                                xEvent.type                = ButtonPress;
                                xEvent.xbutton.button      = Button1;
                                xEvent.xbutton.same_screen = True;
                                xEvent.xbutton.root        = ctx->rootWin;
                                xEvent.xbutton.subwindow   = None;
                                xEvent.xbutton.window      = ctx->srcWin;
                                xEvent.xbutton.x           = trgX;
                                xEvent.xbutton.y           = trgY + (ctx->topOffset / 2);


                                if (XSendEvent (ctx->xDpy, ctx->srcWin, True, ButtonPressMask, &xEvent) == 0) fprintf (stderr, "Error\n");

                                XFlush (ctx->xDpy);
                 */

   
            case KeyPress:
                snprintf (buf, sizeof (buf), "Got key combination\n\tkeycode:"
                          "\t%d\n\tkey state:\t%d\n\texit key code:\t%d\n\texit"
                          " key mask:\t%d\n\ttrans key code:\t%d\n\ttrans key "
                          "mask:%d\n\txEvent.xkey.state ^ cfg->exitKeyMask:\t%d"
                          "\n\txEvent.xkey.state ^ cfg->translationCtrlKeyMask:"
                          "\t%d", xEvent.xkey.keycode, xEvent.xkey.state,
                          ctx->exitKeyCode, ctx->exitKeyMask,
                          ctx->cloneKeyCode,
                          ctx->cloneKeyMask,
                          xEvent.xkey.state ^ ctx->exitKeyMask,
                          xEvent.xkey.state ^ ctx->cloneKeyMask);
                logCtr (buf, LOG_LVL_1, False);

                if (xEvent.xkey.keycode == ctx->exitKeyCode
                    && (xEvent.xkey.state ^ ctx->exitKeyMask) == 0)
                {
                    logCtr ("Exit key sequence received!", LOG_LVL_NO, False);
                    return EXIT_COMBINATION;
                }
                else if (xEvent.xkey.keycode
                         == ctx->cloneKeyCode
                         && (xEvent.xkey.state ^ ctx->cloneKeyMask) == 0
                         && ctx->isDaemon == True)
                {
                    logCtr ("Grab window key sequence received!", LOG_LVL_NO,
                            False);
                    return TRANSLATION_COMBINATION;
                }
                else
                {
                    XAllowEvents (ctx->xDpy, ReplayKeyboard, xEvent.xkey.time);
                    XFlush (ctx->xDpy);
                }
                break;

            default:
                break;
        }
    }
    return NO_KEY_PRESSED;
}

Colormap
createColormap (XWCContext * ctx,
                Visual     * xVis)
{
    return XCreateColormap (ctx->xDpy, ctx->srcWin, xVis, AllocNone);
}

Window
createWindow (XWCContext           * ctx,
              Visual               * xVis,
              long long int          mask,
              XSetWindowAttributes * attr)
{
    Window w;
    w = XCreateWindow (ctx->xDpy, ctx->rootWin, 0, 0, ctx->srcWinAttr.width,
                       ctx->srcWinAttr.height, 0, ctx->srcWinAttr.depth,
                       InputOutput, xVis, mask, attr);
    return w;
}

Bool
createTrgWindow (XWCContext * ctx)
{
    XSetWindowAttributes  * trgWinSetAttr;
    XVisualInfo             xVisInfo;
    long long int           mask;

    if (getVisOfScr (ctx, &xVisInfo) == False)
    {
        logCtr ("\tError: no such visual", LOG_LVL_NO, False);
        return False;
    }
    void * bp = malloc (sizeof (XSetWindowAttributes));

    if (bp == NULL)
    {
        logCtr ("\tError: cannot allocate memory for XSetWindowAttributes",
                LOG_LVL_NO, False);
        return False;
    }

    trgWinSetAttr = (XSetWindowAttributes*) bp;

    trgWinSetAttr->colormap         = createColormap (ctx, xVisInfo.visual);
    trgWinSetAttr->background_pixel = ctx->bgColor.pixel;
    trgWinSetAttr->border_pixel     = 0;
    trgWinSetAttr->bit_gravity      = NorthWestGravity;
    //trgWinSetAttr->event_mask       = ButtonPressMask | ButtonReleaseMask;
    mask                           = CWBackPixel | CWColormap | CWBorderPixel |
        CWBitGravity ;

    ctx->trgWin = createWindow (ctx, xVisInfo.visual, mask, trgWinSetAttr);

    if (getXErrState () == True)
    {
        logCtr ("\tfailed to create window!", LOG_LVL_NO, False);
        if (ctx->trgWin != None)
        {
            XDestroyWindow (ctx->xDpy, ctx->trgWin);
        }
        return None;
    }

    if (setWinTitlebar (ctx->xDpy, ctx->trgWin, WM_CLASS_PRG_NAME_STR) == False
        || setWindowClass (ctx->xDpy, ctx->trgWin, WM_CLASS_PRG_NAME_STR,
                           WM_CLASS_CLASS_NAME_STR) == False )
    {
        return False;
    }
    free (trgWinSetAttr);
    return True;
}

Bool
getVisOfScr (XWCContext  * ctx,
             XVisualInfo * xVisInfo)
{
    int xScrId = XScreenNumberOfScreen (ctx->xScr);
    int res = XMatchVisualInfo (ctx->xDpy, xScrId, ctx->srcWinAttr.depth,
                                TrueColor, xVisInfo);
    return res != 0;
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
bgImgPrepare (XWCContext        * ctx,
              Pixmap            * bgImgPm,
              unsigned int      * bgImgWidth,
              unsigned int      * bgImgHeight,
              Window              bgImgRootWin,
              XWindowAttributes * bgImgRootWinAttr)
{
    Imlib_Image imgSrc, imgScaled;
    char buf[1024];

    if (ctx->bgImgFileSet == True)
    {
        logCtr ("Reading background image file:", LOG_LVL_NO, False);
    }
    else
    {
        logCtr ("Reading background image file:", LOG_LVL_1, False);
    }

    imgSrc = imlib_load_image (ctx->bgImgFileStr);

    if (imgSrc == NULL)
    {
        snprintf (buf, sizeof (buf), "\tcannot load background image file"
                  " '%s'!", ctx->bgImgFileStr);

        if (ctx->bgImgFileSet == True)
        {
            logCtr (buf, LOG_LVL_NO, True);
            return False;
        }
        else
        {
            logCtr (buf, LOG_LVL_1, True);
        }
    }
    else
    {
        imlib_context_set_image (imgSrc);
        *bgImgWidth  = imlib_image_get_width ();
        *bgImgHeight = imlib_image_get_height ();

        if (   *bgImgWidth  > ctx->rootWinAttr.width
            || * bgImgHeight > ctx->rootWinAttr.height)
        {
            float scaleFactor = (float) *bgImgWidth / (float) *bgImgHeight;

            int newWidth  = ctx->rootWinAttr.width;
            int newHeight = (float) newWidth / scaleFactor;

            snprintf (buf, sizeof (buf), "Image scaled to:\n\twidth:\t%d\n\t"
                      "height:\t%d", newWidth, newHeight);
            logCtr (buf, LOG_LVL_1, True);

            imgScaled = imgLibCreateCrSc (0, 0, *bgImgWidth, *bgImgHeight,
                                          newWidth, newHeight);
            //imlib_free_image_and_decache ();
            imlib_free_image ();
            imlib_context_set_image (imgScaled);
            *bgImgWidth  = imlib_image_get_width ();
            *bgImgHeight = imlib_image_get_height ();
        }
        else
        {
            imgScaled = imgSrc;
        }


        *bgImgPm = XCreatePixmap (ctx->xDpy, bgImgRootWin, *bgImgWidth,
                                  *bgImgHeight, bgImgRootWinAttr->depth);

        imlib_context_set_display (ctx->xDpy);
        imlib_context_set_visual (bgImgRootWinAttr->visual);
        imlib_context_set_colormap (bgImgRootWinAttr->colormap);
        imlib_context_set_drawable (*bgImgPm);

        imlib_render_image_on_drawable (0, 0);

        XSync (ctx->xDpy, 0);

        //imlib_free_image_and_decache ();
        imlib_free_image ();
        ctx->bgImgStatus = True;

        if (ctx->bgImgFileSet == True)
        {
            logCtr ("\tsuccess", LOG_LVL_NO, True);
        }
        else
        {
            logCtr ("\tsuccess", LOG_LVL_1, True);
        }
    }
    return True;
}


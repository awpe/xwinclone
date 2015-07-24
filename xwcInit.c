#include "xwc.h"

XWCContext *
init (int           argCnt,
      const char ** argArr)
{
    KeySym              exitKeySym;
    arguments         * args;
    char              * endPtr, buf[2048];
    int                 nextArgOffset, i, j, k, fr, raiseT, clickT;
    int                 resT, longWait, focusTime;
    unsigned long       srcid;
    const char        * ptrDevName;
    XWCContext        * ctx;
    Bool                fileLengthMatch, argFound;

    ctx = (XWCContext*) malloc (sizeof (XWCContext));

    if (ctx == NULL)
    {
        logCtr ("\tError allocating memory for options struct!", LOG_LVL_NO,
                True);
        return NULL;
    }

    memset (ctx, 0, sizeof (*ctx));

    ctx->bgColorStr      = NULL;
    ctx->bgImgFilePath   = NULL;
    ctx->cfgFPath        = NULL;
    ctx->exitKeyStr      = NULL;
    ctx->kbds            = NULL;
    ctx->lckFPath        = NULL;
    ctx->transCtrlKeyStr = NULL;
    ctx->xDpy            = NULL;
    ctx->xScr            = NULL;
    ctx->clMods          = NULL;
    ctx->exitMods        = NULL;

    /*Make a program to be portable to all locales*/
    setlocale (LC_ALL, "");

    if (( ctx->xDpy = openDefaultDisplay () )  == NULL)
    {
        freeXWCContext (ctx);
        return NULL;
    }

    if (chkCompExt (ctx->xDpy) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        return NULL;
    }

    if (chckXI2Ext (ctx) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        return NULL;
    }

    if ((ctx->rootW = getDefaultRootWindow (ctx->xDpy)) == None)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        return NULL;
    }

    XGetWindowAttributes (ctx->xDpy, ctx->rootW, &ctx->rootWAttr);

    if (getXErrState (ctx) == True)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        return NULL;
    }

    if (( ctx->xScr = getScreenByWindowAttr (ctx, &ctx->rootWAttr) ) == NULL)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        return NULL;
    }

    logCtr ("Processing arguments and building configuration:", LOG_LVL_1,
            False);

    args = initArgs ();

    if (args == NULL)
    {
        logCtr ("\tCannot allocate structure to process arguments!",
                LOG_LVL_NO, True);
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        return NULL;
    }

    if (   addArg (args, True,  C_STR, BGCOLOR,    "BGCOLOR",
                   "Sets translation window background color, must be"
                   " specified in RGB format i.e. \"FF0000\" for red"
                   " background",
                   1, "-bgclr")
        == False

        || addArg (args, True,  C_STR, BGIMAGE,    "BGIMAGE",
                   "File name of image to be used as background, relative and"
                   " absolute paths are supported.",
                   1, "-bgimg")
        == False

        || addArg (args, True,  INT,   FRAMERATE,  "FRAMERATE",
                   "Sets framerate in FPS for translation window, choose wisely"
                   " as it takes some resources to copy source window image",
                   1, "-fr"   )
        == False

        || addArg (args, True,  INT,   FOCUSTIME,  "FOCUSTIME",
                   "Time in milliseconds for program to wait for you to move focus"
                   " to window you like to be translated, 0 means disabled",
                   1, "-ft"   )
        == False

        || addArg (args, True,  INT,   TOPOFFSET,  "TOPOFFSET",
                   "Specifies top offset in pixels for translation i.e. how "
                   "many top pixels of source window image you want to hide",
                   1, "-toff" )
        == False

        || addArg (args, False, C_STR, HELP,       "HELP",
                   "Use it to print this message :)",
                   3,  "-h", "-help", "--help")
        == False

        || addArg (args, True,  INT,   AUTOCENTER, "AUTOCENTER",
                   "If enabled, xwinclone will translate only center part of "
                   "source window, size is determined by xwinclone own window ",
                   1, "-ac"   )
        == False

        || addArg (args, True,  INT,   LOGLVL,     "LOGLEVEL",
                   "You can choose how much verbose to print to log file, "
                   "possible values are 0, 1, 2",
                   1, "-ll"   )
        == False

        || addArg (args, True,  ULONG, SOURCEID,   "SOURCEID",
                   "Used to explicity set source window id, possible values are"
                   " HEX numbers as you get them from xwininfo or xprop or"
                   " simular application",
                   1, "-srcid")
        == False

        || addArg (args, True,  C_STR, LCKFPATH,   "LOCKFILEPATH",
                   "Path to the file which is used to make sure only one "
                   "instance of xwinclone is running",
                   1, "-lckf")
        == False

        || addArg (args, False, INT,   SINGLEINST, "SINGLEINST",
                   "Shows if this instance can be executed parallel to other "
                   "intances of this program",
                   2, "-si", "-single")
        == False

        || addArg (args, False, INT,   DAEMON,     "DAEMON",
                   "Using this option you can enable daemon mode, in such a "
                   "mode you are not forced to move focus to source window in "
                   "given time, instead program waits for you to press either "
                   "translation control or exit combination",
                   2, "-d", "-daemon")
        == False

        || addArg (args, True,  C_STR, PTRDEVNAME, "PTRDEVNAME",
                   "You can specify non default pointer device name which will "
                   "be used to grab click events for translation window and "
                   "also for sending click events to source window, see xinput "
                   "output for valid names",
                   1, "-ptrname")
        == False

        || addArg (args, True,  C_STR, LOGFNAME,   "LOGFNAME",
                   "Name of file where logs will be stored by default it is "
                   "\"stdout\" which can be changed to \"stderr\" or text file "
                   "either using absolute or relative path",
                   1, "-logf")
        == False

        || addArg (args, True,  INT,   CLICKTIME,   "CLICKTIME",
                   "Specifies time in milliseconds between pointer button press "
                   "and release events, 0 means disabled",
                   2, "-clicktime", "-clt")
        == False

        || addArg (args, True,  INT,   RAISETIME,   "RAISETIME",
                   "Specifies time in milliseconds to wait for source window to "
                   "become topmost in window manager's window stack, as source "
                   "window must be drawn on screen in the place you want to "
                   "send click event 0 means disabled",
                   2, "-raisetime", "-raiset")
        == False

        || addArg (args, True,  INT,   RESTORETIME, "RESTORETIME",
                   "Specifies time in milliseconds to wait for source window to "
                   "restore from hidden state (after minimizing to taskbar for "
                   "example), 0 means disabled",
                   2, "-restoretime", "-rest")
        == False

        || addArg (args, True,  INT,   LONGWAIT, "LONGWAIT",
                   "Specifies time in milliseconds to wait before new attempt "
                   "to redraw target window in case of source or target window "
                   "is not viewable, 0 means default",
                   2, "-longwait", "-lw")
        == False
        )
    {
        delArgs (args);
        logCtr ("\tCannot add new argument to list!\n", LOG_LVL_NO, True);
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    nextArgOffset = 2;

    for (i = 1; i < argCnt; i += nextArgOffset)
    {
        argFound = False;
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
                        XCloseDisplay (ctx->xDpy);
                        freeXWCContext (ctx);
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
                                XCloseDisplay (ctx->xDpy);
                                freeXWCContext (ctx);
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
                                XCloseDisplay (ctx->xDpy);
                                freeXWCContext (ctx);
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
            XCloseDisplay (ctx->xDpy);
            freeXWCContext (ctx);
            delArgs (args);
            return NULL;
        }
        else
        {
            nextArgOffset = ( args->m_Args[j]->m_HasValue == True ) ? 2 : 1;
        }
    }

    /*boilerplate wa*/
    LOG_LVL     = * ((int*)           args->m_Args[LOGLVL]->m_Value);
    fr          = * ((int*)           args->m_Args[FRAMERATE]->m_Value);
    srcid       = * ((unsigned long*) args->m_Args[SOURCEID]->m_Value);
    ptrDevName  =   (const char*)     args->m_Args[PTRDEVNAME]->m_Value;
    raiseT   = * ((int*)           args->m_Args[RAISETIME]->m_Value);
    clickT   = * ((int*)           args->m_Args[CLICKTIME]->m_Value);
    resT = * ((int*)           args->m_Args[RESTORETIME]->m_Value);
    longWait    = * ((int*)           args->m_Args[LONGWAIT]->m_Value);
    focusTime   = * ((int*)           args->m_Args[FOCUSTIME]->m_Value);

    if (longWait == 0)
    {
        longWait = LONG_WAIT;
    }

    memset (&ctx->bgColor, 0, sizeof (ctx->bgColor ));

    ctx->focusDelay.tv_sec    = focusTime == 0 ? 0 : focusTime / 1000;
    ctx->focusDelay.tv_nsec   = focusTime == 0 ? 0 : focusTime % 1000;
    ctx->frameDelay.tv_sec    = fr == 1 ? 1 : 0;
    ctx->frameDelay.tv_nsec   = fr == 1 ? 0 : (1.00000001 / fr) * 1000000000L;
    ctx->longWait.tv_sec      = longWait / 1000;
    ctx->longWait.tv_nsec     = longWait % 1000;
    ctx->raiseDelay.tv_sec    = raiseT == 0 ? 0 :  raiseT / 1000;
    ctx->raiseDelay.tv_nsec   = raiseT == 0 ? 0 : (raiseT % 1000) * 1000000L;
    ctx->restoreDelay.tv_sec  = resT   == 0 ? 0 :  resT   / 1000;
    ctx->restoreDelay.tv_nsec = resT   == 0 ? 0 : (resT   % 1000) * 1000000L;
    ctx->clickDelay.tv_sec    = clickT == 0 ? 0 :  clickT / 1000;
    ctx->clickDelay.tv_nsec   = clickT == 0 ? 0 : (clickT % 1000) * 1000000L;
    ctx->autoCenter           = * ((int*) args->m_Args[AUTOCENTER]->m_Value);
    ctx->topOffset            = * ((int*) args->m_Args[TOPOFFSET]->m_Value);
    ctx->bgColorStr           = (const char*) args->m_Args[BGCOLOR]->m_Value;
    ctx->bgImgFilePath        = (const char*) args->m_Args[BGIMAGE]->m_Value;
    ctx->bgImgFileSet         = args->m_Args[BGIMAGE]->m_IsSet;
    ctx->bgImgStatus          = False;
    ctx->exitKeyStr           = EXIT_KEY;
    ctx->exitKeyMask          = EXIT_MASK;
    ctx->transCtrlKeyStr      = TRANSLATION_CTRL_KEY;
    ctx->cloneKeyMask         = TRANSLATION_CTRL_MASK;
    ctx->srcW                 = srcid;
    ctx->isDaemon             = args->m_Args[DAEMON]->m_IsSet;
    ctx->lckFPath             = args->m_Args[LCKFPATH]->m_Value;
    ctx->isSingleton          = args->m_Args[SINGLEINST]->m_IsSet;
    ctx->isSingleton          = ctx->isSingleton || ctx->isDaemon;
    ctx->slavePtrDevId        = NO_DEVICE;
    ctx->masterPtrDevId       = NO_DEVICE;
    ctx->ptrDevName           = ptrDevName;
    ctx->logFileName          = (const char*) args->m_Args[LOGFNAME]->m_Value;

    LOG_FILE = NULL;

    if (strncmp (ctx->logFileName, "stdout", LOG_FILE_NAME_MAX_LENGTH)
        == STR_EQUAL)
    {
        LOG_FILE = stdout;
    }

    if (strncmp (ctx->logFileName, "stderr", LOG_FILE_NAME_MAX_LENGTH)
        == STR_EQUAL)
    {
        LOG_FILE = stderr;
    }

    if (LOG_FILE != stdout && LOG_FILE != stderr)
    {
        fileLengthMatch = True;

        for (i = 1; i < LOG_FILE_NAME_MAX_LENGTH; ++ i)
        {
            if (*ctx->logFileName == '\0')
            {
                fileLengthMatch = False;
                break;
            }
        }

        if (fileLengthMatch == False)
        {
            XCloseDisplay (ctx->xDpy);
            freeXWCContext (ctx);
            delArgs (args);
            logCtr ("File name is too long", LOG_LVL_NO, False);
            return NULL;
        }

        LOG_FILE = fopen (ctx->logFileName, "a");

        if (LOG_FILE == NULL)
        {
            snprintf (buf, sizeof (buf), "Error opening log file %s!",
                      ctx->logFileName);
            logCtr (buf, LOG_LVL_NO, False);
            XCloseDisplay (ctx->xDpy);
            freeXWCContext (ctx);
            delArgs (args);
            return NULL;
        }
    }

    if (getInputDevices (ctx) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (( exitKeySym = XStringToKeysym (ctx->exitKeyStr) ) == NoSymbol)
    {
        snprintf (buf, sizeof (buf), "\tError parsing exit key string (%s)",
                  ctx->exitKeyStr);
        logCtr (buf, LOG_LVL_NO, True);
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if ((ctx->exitKeyCode = XKeysymToKeycode (ctx->xDpy, exitKeySym)) == 0)
    {
        snprintf (buf, sizeof (buf), "\tUnknown keycode %d", ctx->exitKeyCode);
        logCtr (buf, LOG_LVL_NO, True);
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (( exitKeySym = XStringToKeysym (ctx->transCtrlKeyStr) )
        == NoSymbol)
    {
        snprintf (buf, sizeof (buf), "\tError parsing exit key string (%s)",
                  ctx->transCtrlKeyStr);
        logCtr (buf, LOG_LVL_NO, True);
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (( ctx->cloneKeyCode = XKeysymToKeycode (ctx->xDpy, exitKeySym) ) == 0)
    {
        snprintf (buf, sizeof (buf), "\tUnknown keycode %d",
                  ctx->cloneKeyCode);
        logCtr (buf, LOG_LVL_NO, True);
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (args->m_Args[HELP]->m_IsSet == True)
    {
        printUsage (args);
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (ctx->isSingleton == True && ifSingleInst (ctx) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (parseColor (ctx) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if ((ctx->devsAcquired = grabAllKeys (ctx)) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (LOG_LVL > LOG_LVL_NO)
    {
        printCurValues (args);
    }

    logCtr ("\tconfiguration successfully completed", LOG_LVL_1, True);

    delArgs (args);
    return ctx;
}

void
freeXWCContext (XWCContext * ctx)
{
    if (ctx->clMods != NULL)
    {
        free (ctx->clMods);
        free (ctx->exitMods);
    }

    if (ctx->kbds != NULL)
    {
        if (ctx->kbds->devs != NULL)
        {
            free (ctx->kbds->devs);
        }
        free (ctx->kbds);
    }

    free (ctx);
}

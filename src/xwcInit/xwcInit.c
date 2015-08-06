/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://raw.githubusercontent.com/awpe/xwinclone/master/LICENSE.md>.  */

#include <xwcInit.h>
#include <xwcHeaders.h>

XWCContext *
init (int           argCnt,
      const char ** argArr)
{
    arguments         * args;
    int                 fr, raiseT, clickT, resT, longWait, focusTime;
    unsigned long       srcid;
    const char        * ptrDevName;
    XWCContext        * ctx;
    Bool                daemonState, multiInst, procbtnev;

    /*Make a program to be portable to all locales*/
    setlocale (LC_ALL, "");

    logCtrl ("Processing arguments and building configuration", LOG_LVL_1,
             False);

    ctx = (XWCContext*) malloc (sizeof (XWCContext));

    if (ctx == NULL)
    {
        logCtrl ("\tError allocating memory for options struct!", LOG_LVL_NO,
                 True);
        return NULL;
    }

    memset (ctx, 0, sizeof (*ctx));

    ctx->bgColorStr      = NULL;
    ctx->bgImgFilePath   = NULL;
    ctx->exitKeyStr      = NULL;
    ctx->kbds            = NULL;
    ctx->lckFPath        = NULL;
    ctx->transCtrlKeyStr = NULL;
    ctx->xDpy            = NULL;
    ctx->xScr            = NULL;
    ctx->clMods          = NULL;
    ctx->exitMods        = NULL;
    ctx->userDir         = NULL;

    args = initArgs ();

    if (args == NULL)
    {
        logCtrl ("\tCannot allocate structure to process arguments!",
                 LOG_LVL_NO, True);
        freeXWCContext (ctx);
        return NULL;
    }

    if (populateArgs (args) == False)
    {
        logCtrl ("\tCannot add argument to list!\n", LOG_LVL_NO, True);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (parseArgs (argArr, args, argCnt) == False)
    {
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (getUserDir (ctx) == False)
    {
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (args->m_Args[CONFFILE]->m_Val != NULL)
    {
        if (readConfFile (ctx, args) == False)
        {
            if (   args->m_Args[CONFFILE]->m_IsSet == True
                && args->m_Args[MKCONFIG]->m_IsSet == False)
            {
                logCtrl ("\t\tError: Cannot process specified config file!\n",
                         LOG_LVL_NO, False);
                freeXWCContext (ctx);
                delArgs (args);
                return NULL;
            }
            else if (   args->m_Args[MKCONFIG]->m_IsSet == False
                     && args->m_Args[CONFFILE]->m_IsSet == False)
            {
                logCtrl ("\t\tWARNING: Cannot process default config file!\n",
                         LOG_LVL_1, True);

                if (CREATE_CONFIG_IF_NOT_FOUND == True)
                {
                    logCtrl ("\t\tTrying to create default config!\n",
                             LOG_LVL_1, False);
                    if (createConfFile (ctx, args) == False)
                    {
                        logCtrl ("\t\t\tError creating config file!",
                                 LOG_LVL_NO, False);
                    }
                    logCtrl ("\t\tsuccess!\n",
                             LOG_LVL_1, False);
                }
            }
        }
    }

    /*boilerplate wa*/
    LOG_LVL     = * ((int*)           args->m_Args[LOGLVL]->m_Val);
    fr          = * ((int*)           args->m_Args[FRAMERATE]->m_Val);
    srcid       = * ((unsigned long*) args->m_Args[SOURCEID]->m_Val);
    ptrDevName  =   (const char*)     args->m_Args[PTRDEVNAME]->m_Val;
    raiseT      = * ((int*)           args->m_Args[RAISETIME]->m_Val);
    clickT      = * ((int*)           args->m_Args[CLICKTIME]->m_Val);
    resT        = * ((int*)           args->m_Args[RESTORETIME]->m_Val);
    longWait    = * ((int*)           args->m_Args[LONGWAIT]->m_Val);
    focusTime   = * ((int*)           args->m_Args[FOCUSTIME]->m_Val);
    daemonState = args->m_Args[DAEMON]->m_IsSet;
    multiInst   = args->m_Args[MULTIINST]->m_IsSet;
    procbtnev   = args->m_Args[PROCBTNEV]->m_IsSet;

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
    ctx->autoCenter           = * ((int*) args->m_Args[AUTOCENTER]->m_Val);
    ctx->topOffset            = * ((int*) args->m_Args[TOPOFFSET]->m_Val);
    ctx->bgColorStr           = (const char*) args->m_Args[BGCOLOR]->m_Val;
    ctx->bgColorNeedFree      = args->m_Args[BGCOLOR]->m_NeedFree;
    ctx->bgImgFilePath        = (const char*) args->m_Args[BGIMAGE]->m_Val;
    ctx->bgImgNeedFree        = args->m_Args[BGIMAGE]->m_NeedFree;
    ctx->bgImgFileSet         = args->m_Args[BGIMAGE]->m_IsSet;
    ctx->bgImgStatus          = False;
    ctx->exitKeyStr           = args->m_Args[EXITKEY]->m_Val;
    ctx->exitKeyStrNeedFree   = args->m_Args[EXITKEY]->m_NeedFree;
    ctx->exitKeyMask          = EXIT_MASK;
    ctx->transCtrlKeyStr      = args->m_Args[CLONEKEY]->m_Val;
    ctx->transCtrlKeyNeedFree = args->m_Args[CLONEKEY]->m_NeedFree;
    ctx->cloneKeyMask         = TRANSLATION_CTRL_MASK;
    ctx->srcW                 = srcid;
    ctx->isDaemon             = daemonState;
    ctx->lckFPath             = args->m_Args[LCKFPATH]->m_Val;
    ctx->lckFileNameNeedFree  = args->m_Args[LCKFPATH]->m_NeedFree;
    ctx->multiInst            = multiInst;
    ctx->multiInst            = ctx->isDaemon == False ? ctx->multiInst : True;
    ctx->slavePtrDevId        = NO_DEVICE;
    ctx->masterPtrDevId       = NO_DEVICE;
    ctx->ptrDevName           = ptrDevName;
    ctx->ptrDevNameNeedFree   = args->m_Args[PTRDEVNAME]->m_NeedFree;
    ctx->logFileName          = (const char*) args->m_Args[LOGFNAME]->m_Val;
    ctx->logFileNameNeedFree  = args->m_Args[LOGFNAME]->m_NeedFree;
    ctx->procBtnEv            = procbtnev;
    ctx->confFileName         = (const char*) args->m_Args[CONFFILE]->m_Val;

    if (args->m_Args[HELP]->m_IsSet == True)
    {
        printUsage (args);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (enableLogFile (ctx) == False)
    {
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (   (ctx->multiInst == False || ctx->isDaemon == True)
        && checkInstCount (ctx) == False)
    {
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (openDefaultDisplay (ctx)  == False)
    {
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (chkCompExt (ctx->xDpy) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (chckXI2Ext (ctx) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (getDefaultDisplayData (ctx) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (getInputDevices (ctx) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (getCtrlKeycodes (ctx) == False)
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

    if (parseColor (ctx) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (args->m_Args[CHECKARGS]->m_IsSet == True)
    {
        printCurValues (args);
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (args->m_Args[MKCONFIG]->m_IsSet == True)
    {
        printCurValues (args);
        if (createConfFile (ctx, args) == False)
        {
            logCtrl ("\tError creating config file!", LOG_LVL_NO, False);
        }
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    logCtrl ("\n\tconfiguration successfully completed", LOG_LVL_1, True);

    if (LOG_LVL > LOG_LVL_1)
    {
        printCurValues (args);
    }

    delArgs (args);
    return ctx;
}

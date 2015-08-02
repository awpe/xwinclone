#include <xwcInit.h>
#include <xwcHeaders.h>

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

    if (ctx->bgImgNeedFree == True)
    {
        free ((char*) ctx->bgImgFilePath);
    }

    if (ctx->logFileNameNeedFree == True)
    {
        free ((char*) ctx->logFileName);
    }

    if (ctx->lckFileNameNeedFree == True)
    {
        free ((char*) ctx->lckFPath);
    }

    if (ctx->bgColorNeedFree == True)
    {
        free ((char*) ctx->bgColorStr);
    }

    if (ctx->exitKeyStrNeedFree == True)
    {
        free ((char*) ctx->exitKeyStr);
    }

    if (ctx->transCtrlKeyNeedFree == True)
    {
        free ((char*) ctx->transCtrlKeyStr);
    }

    if (ctx->ptrDevNameNeedFree == True)
    {
        free ((char*) ctx->ptrDevName);
    }

    free (ctx);
}

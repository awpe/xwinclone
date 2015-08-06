/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

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

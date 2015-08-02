#include <xwcDevCtrl.h>

Bool
adjPtrLoc (XWCContext * ctx,
           int          inX,
           int          inY,
           int        * adjX,
           int        * adjY,
           int        * adjRX,
           int        * adjRY)
{
    Window childRet;
    int trgWinLOff, trgWinTOff, trgWinW, trgWinH, srcWinW, srcWinH;

    if (ctx == NULL || adjX == NULL || adjY == NULL || adjRX == NULL
        || adjRY == NULL)
    {
        logCtrl ("Cannot translate source window coordinates to root window:"
                 " NULL pointer received!", LOG_LVL_NO, False);
        return False;
    }

    *adjX = inX;
    *adjY = inY;

    if (ctx->autoCenter == True)
    {
        if (   getWAttr (ctx, &ctx->srcW, &ctx->srcWAttr) == False
            || getWAttr (ctx, &ctx->trgW, &ctx->trgWAttr) == False)
        {
            return False;
        }

        trgWinW = ctx->trgWAttr.width;
        srcWinW = ctx->srcWAttr.width;
        trgWinH = ctx->trgWAttr.height;
        srcWinH = ctx->srcWAttr.height;

        trgWinLOff = trgWinW - srcWinW;
        trgWinTOff = trgWinH - srcWinH - ctx->topOffset;

        trgWinLOff /= 2;
        trgWinTOff /= 2;

        *adjX -= trgWinLOff;
        *adjY -= trgWinTOff;
    }
    else
    {
        *adjY += ctx->topOffset;
    }

    XTranslateCoordinates (ctx->xDpy, ctx->srcW, ctx->rootW,
                           *adjX, *adjY,
                           adjRX, adjRY,
                           &childRet);

    if (getXErrState (ctx) == True)
    {
        logCtrl ("Cannot translate source window coordinates to root window:"
                 " XTranslateCoordinates error!", LOG_LVL_NO, False);
        return False;
    }

    return True;
}

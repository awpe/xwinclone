#include <xwcDevCtrl.h>

Bool
grabAllKeys (XWCContext * ctx)
{
    size_t sizeTmp;

    logCtrl ("\tTrying to grab all control keys", LOG_LVL_1, False);

    /**************************************************************************/
    /*prepare modifiers*/
    /**************************************************************************/
    ctx->nMods = 4;

    sizeTmp = sizeof (XIGrabModifiers);

    ctx->clMods   = (XIGrabModifiers *) malloc (sizeTmp * ctx->nMods);

    if (ctx->clMods == NULL)
    {
        logCtrl ("\t\tError allocating memory for XIGrabModifiers struct!",
                 LOG_LVL_NO, False);
        return False;
    }

    ctx->exitMods = (XIGrabModifiers *) malloc (sizeTmp * ctx->nMods);

    if (ctx->exitMods == NULL)
    {
        logCtrl ("\t\tError allocating memory for XIGrabModifiers struct!",
                 LOG_LVL_NO, False);
        return False;
    }

    ctx->clMods[0].modifiers = ctx->cloneKeyMask;
    ctx->clMods[1].modifiers = ctx->cloneKeyMask | LockMask;
    ctx->clMods[2].modifiers = ctx->cloneKeyMask | Mod2Mask;
    ctx->clMods[3].modifiers = ctx->cloneKeyMask | Mod2Mask  | LockMask;

    ctx->exitMods[0].modifiers = ctx->exitKeyMask;
    ctx->exitMods[1].modifiers = ctx->exitKeyMask | LockMask;
    ctx->exitMods[2].modifiers = ctx->exitKeyMask | Mod2Mask;
    ctx->exitMods[3].modifiers = ctx->exitKeyMask | Mod2Mask  | LockMask;
    /**************************************************************************/


    /**************************************************************************/
    /*Grab exit key*/
    /**************************************************************************/
    if (grabKeyCtrl (ctx, ctx->rootW, ctx->exitKeyCode, ctx->nMods,
                     ctx->exitMods, True)
        == False)
    {
        logCtrl ("\t\tError grabbing exit key!", LOG_LVL_NO, False);
        return False;
    }
    /**************************************************************************/


    /**************************************************************************/
    /*Grab translation control key*/
    /**************************************************************************/
    if (grabKeyCtrl (ctx, ctx->rootW, ctx->cloneKeyCode, ctx->nMods,
                     ctx->clMods, True)
        == False)
    {
        logCtrl ("\t\tError grabbing clone key!", LOG_LVL_NO, False);
        grabKeyCtrl (ctx, ctx->rootW, ctx->exitKeyCode, ctx->nMods,
                     ctx->exitMods, False);
        return False;
    }
    /**************************************************************************/

    logCtrl ("\t\tsuccess", LOG_LVL_2, False);

    return True;
}

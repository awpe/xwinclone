#include <xwcXSrvUtil.h>

Bool
getVisOfScr (XWCContext  * ctx,
             XVisualInfo * xVisInfo)
{
    int xScrId = XScreenNumberOfScreen (ctx->xScr);
    int res = XMatchVisualInfo (ctx->xDpy, xScrId, ctx->srcWAttr.depth,
                                TrueColor, xVisInfo);
    return res != 0;
}

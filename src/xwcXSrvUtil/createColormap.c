#include <xwcXSrvUtil.h>

Colormap
createColormap (XWCContext * ctx,
                Visual     * xVis)
{
    return XCreateColormap (ctx->xDpy, ctx->srcW, xVis, AllocNone);
}

/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://raw.githubusercontent.com/awpe/xwinclone/master/LICENSE.md>.  */

#include <xwcDevCtrl.h>

int
procKeySeqEv (XWCContext    * ctx,
              XIDeviceEvent * xide)
{

    if (xide == NULL || ctx == NULL)
    {
        logCtrl ("Cannot process pressed key: NULL pointer(s) received!",
                 LOG_LVL_NO, False);
        return EXIT_COMBINATION;
    }

    /* For button events, detail is the button number (after mapping
     * applies of course). For key events, detail is the keycode. 
     * XI2 supports 32-bit keycodes, btw. For motion events, 
     * detail is 0. 
     * From http://who-t.blogspot.cz/2009/07/xi2-recipes-part-4.html*/
    if (xide->detail == ctx->cloneKeyCode)
    {
        logCtrl ("Grab window key sequence received", LOG_LVL_NO, False);

        return TRANSLATION_COMB;
    }

    if (xide->detail == ctx->exitKeyCode)
    {
        logCtrl ("Exit key sequence received", LOG_LVL_NO, False);

        return EXIT_COMBINATION;
    }

    return NO_KEY_PRESSED;
}

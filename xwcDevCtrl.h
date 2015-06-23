#ifndef XWCDEVCTRL_H
#define	XWCDEVCTRL_H

#include "xwcInit.h"

/**
 * Registers exit key combination for a given window.
 * @param[in] d Pointer to Xlib's Display data struct.
 * @param[in] WID Window xid of window where exit event will be grabbed
 * @param[in] prgCfg Data struct with program's configuration
 * @return Xlib's True on success, False otherwise
 */
Bool
grabKeys (XWCContext * prgCfg);

/**
 * Deregisters exit key combination for a given window.
 * @param[in] d Pointer to Xlib's Display data struct.
 * @param[in] WID Window xid of window where exit event was grabbed
 * @param[in] prgCfg Data struct with program's configuration
 */
void
ungrabKeys (XWCContext * prgCfg);

int
mvPtr (XWCContext * ctx, 
       int          x, 
       int          y, 
       int          screen);

int
mvPtrWRel (XWCContext * ctx, Window window, int x, int y);

int
getMouseLoc (XWCContext * ctx,
             int        * xR,
             int        * yR,
             int        * scrNumR,
             Window     * wR);

unsigned int
getInSt (XWCContext * ctx);

int
mouseBtnCtrl (XWCContext * ctx, 
              int          button, 
              int          is_press);

int
setMouseUp (XWCContext * ctx,
            int          button);

int
setMouseDown (XWCContext * ctx,
              int          button);

int
clickW (XWCContext * ctx,
        int          button);

int
getPressedComb (XWCContext * ctx);

#endif


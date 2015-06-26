#ifndef XWCDEVCTRL_H
#define	XWCDEVCTRL_H

#include "xwcInit.h"

/**
 * Registers exit key combination for a given window.
 * @param[in] ctx Pointer to program's context.
 * @param[in] w Window xid of window where key will be grabbed, may be root
 * for system wide grabbing.
 * @param[in] xKCode Keycode to be grabbed.
 * @param[in] kbds Pointer to list of devices where grabbing should take place.
 * @param[in] nMods Number of modifiers.
 * @param[in] mods Array of modifiers.
 * @param[in] grab Specify either key must be grabbed(True) or ungrabbed(False).
 * @return Xlib's True on success, False otherwise.
 */
Bool
grabKeyCtrl (XWCContext      * ctx,
             Window            w,
             KeyCode           xKCode,
             int               nMods,
             XIGrabModifiers * mods,
             Bool              grab);

/**
 * Checks if version 2.0 of XInput2 extension is available.
 * @param[in] ctx Pointer to program's context.
 * @return Xlib's True on success, False otherwise.
 */
Bool
chckXI2Ext (XWCContext * ctx);

/**
 * Tries to get a list of master devices of specified type, creates and fills
 * device list structure on success, nothing is created or filled otherwise.
 * @param[in] ctx Pointer to program's context.
 * @param[in] kbds Address of pointer to the list of devices to store results.
 * @param[in] devType Type of master devices.
 * @return Xlib's True on success, False otherwise.
 * @sa XInput 2 Device types
 */
Bool
getInputDevices (XWCContext *  ctx);

/**
 * Tries to grab all control keys
 * @param[in] ctx Pointer to program's context.
 * @return Xlib's True on success, False otherwise.
 */
Bool
grabAllKeys (XWCContext * ctx);


/**
 * Ungrabs all control keys if possible
 * @param[in] ctx Pointer to program's context.
 * @return Xlib's True on success, False otherwise.
 */
void
ungrabAllKeys (XWCContext * ctx);

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


/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#ifndef XWCDEVCTRL_H
#define	XWCDEVCTRL_H

#include <defines.h>
#include <systemHeaders.h>
#include <xwcHeaders.h>

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


/**
 * adjust target window pointer (cursor) coords to source window proper pointer
 * coords, and get corresponding coords in root window
 * @param[in] ctx Pointer to program's context.
 * @param[in] inX cursor X coordinate in target window to be adjusted
 * @param[in] inY cursor Y coordinate in target window to be adjusted
 * @param[out] adjX Pointer to store adjustment result (X) relative to source
 * @param[out] adjY Pointer to store adjustment result (Y) relative to source
 * @param[out] adjRX Pointer to store adjustment result (X) relative to root
 * @param[out] adjRY Pointer to store adjustment result (Y) relative to root
 * @return Xlib's True on success, False otherwise.
 */
Bool
adjPtrLoc (XWCContext * ctx,
           int          inX,
           int          inY,
           int        * adjX,
           int        * adjY,
           int        * adjRX,
           int        * adjRY);

int
getPressedComb (XWCContext * ctx);

int
procKeySeqEv (XWCContext    * ctx,
              XIDeviceEvent * xide);

int
procBtnEv (XWCContext    * ctx,
           XIDeviceEvent * xide);

Bool
mvPtr (XWCContext * ctx,
       int          x,
       int          y);

Bool
getTrgWPtrData (XWCContext      * ctx,
                int             * trgX,
                int             * trgY,
                XIModifierState * modsSt,
                int             * trgRX,
                int             * trgRY);

Bool
btnClick (XWCContext * ctx,
          XEvent     * btnEv);

Bool
getBtnEv (XWCContext      * ctx,
          XIModifierState * modsSt,
          XEvent          * xev,
          int               adjX,
          int               adjY,
          int               adjRX,
          int               adjRY);

#endif


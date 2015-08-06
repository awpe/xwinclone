/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://raw.githubusercontent.com/awpe/xwinclone/master/LICENSE.md>.  */

#ifndef DEFINES_H
#define	DEFINES_H

#define XWINCLONE_VERSION_STR          "0.1p"

/**************************************************************************/
/*Program definitions*/
#define NO_KEY_PRESSED                 0
#define TRANSLATION_COMB               1
#define EXIT_COMBINATION               2
#define SKIP_OTHER_EVENTS              3
#define ERROR_GETTING_COMBINATION      0xEE
#define STR_EQUAL                      0
#define INT_CONV_OK                    0
#define LOG_LVL_NO                     0
#define LOG_LVL_1                      1
#define LOG_LVL_2                      2
#define FIND_PARENTS                   0 
#define FIND_CHILDREN                  1
#define MAX_POINTER_DEV_NAME_LENGTH    256
#define NO_DEVICE                      0
#define TRACKED_BUTTON                 2 // 2 = lmb
#define LOG_FILE_NAME_MAX_LENGTH       256   
#define SUPPORTED_XI2_VERSION_MAJOR    2  
#define SUPPORTED_XI2_VERSION_MINOR    3
#define OUTPUT_MAX_COLUMNS             80
#define MAX_FILE_LINE_LENGTH           512
/**************************************************************************/

/**************************************************************************/
/*Options*/
#define _POSIX_C_SOURCE                200112L

/*if set will cause program to try maniulating windows even if WM is not EWMH 
 *compiant*/
#define ALLOW_NON_EWMH_COMPILANT_WM    0

#define FRAMERATE_FPS                  30   //fps

#define TIME_TO_CHANGE_FOCUS_SEC       2000 //ms

#define BTN_CLICK_DELAY                10L  //ms

#define RAISE_SOURCE_DELAY             10L  //ms

#define RESTORE_SOURCE_DELAY           250L //ms

#define LONG_WAIT                      500L //ms

#define DEFAULT_BG_COLOR               "000000" //rgb color

#define EXIT_KEY                       "g"
#define EXIT_MASK                      (ControlMask | ShiftMask)

#define TRANSLATION_CTRL_KEY           "u"
#define TRANSLATION_CTRL_MASK          (ControlMask | ShiftMask)

#define WM_CLASS_PRG_NAME_STR          "XWinClone"
#define WM_CLASS_CLASS_NAME_STR        "xwinclone"
#define AUTOCENTERING                  0  //1=true; 0=false
#define TOP_OFFSET                     71 // in pixels
#define PROGRAM_EXE_NAME_STR           "xwinclone"
#define BG_IMAGE_PATH                  "./"
#define DEFAULT_LOG_LVL                LOG_LVL_2
#define LOCK_FILE_PATH                 "/tmp/xwinclone.pid"
#define DEFAULT_BGIMAGE_PATH           "/usr/share/wallpapers/e2e4.jpeg"
#define TRACKED_PTR_NAME               "A4Tech USB Optical Mouse"
#define LOG_FILE_NAME                  "stdout"
#define CREATE_CONFIG_IF_NOT_FOUND     True
#define CONFIG_FILE_PATH               "~/.config/xwinclone/xwc.conf"
#define CONFIG_FILE_DIR_CREATE_MODE    0775
/**************************************************************************/

/**************************************************************************/
/*text*/
#define PROGRAM_BRIEF "clone specified window and send it click events from\
 specified device, the window which content you wish to translate is called\
 \"source\" window, the window where content of source is translated is called\
 \"target\" window"

#define PROGRAM_DESCRIPTION "X window system's WINdow CLONE application.\
\n\n\
Program aimed to be light-weight and utilise few system resources.\
\n\n\
This program can be used to translate another window's content in its own \
window with some features like background color/image, window centering, \
start/stop hotkeys,FPS control.\
\n\n\
In order to translate window content and grab key combination event, program \
communicates with xorg server directly using Xlib and its composite and \
xinput2 extensions. Minimal Xlib's composite extension supported version \
is 0.2, for xinput2 minimal supported version is 2.2\
\n\n\
Window selection mechanism depends on mode, in daemon mode user has to use \
key combinations to start/stop translation/program, in normal mode after \
program starts user have some time to change focus to window which content \
have to be translated. Program registers key press event for root window on \
all keyboard devices, so your window manager or other program must not grab \
this keys or entire keyboard."
/**************************************************************************/

#endif

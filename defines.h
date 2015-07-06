#ifndef DEFINES_H
#define	DEFINES_H

#define XWINCLONE_VERSION_STR          "0.1k"

/**************************************************************************/
/*Program definitions*/
#define NO_KEY_PRESSED                 0
#define TRANSLATION_COMB               1
#define EXIT_COMBINATION               2
#define SKIP_COMBINATION               3
#define ERROR_COMBINATION              0xEE
#define STR_EQUAL                      0
#define INT_CONV_OK                    0
#define LOG_LVL_NO                     0
#define LOG_LVL_1                      1
#define LOG_LVL_2                      2
#define FIND_PARENTS                   0 
#define FIND_CHILDREN                  1
#define MAX_POINTER_DEVICE_NAME_LENGTH 256
#define NO_DEVICE                      0
#define TRACKED_BUTTON                 2 // 2 = lmb
/**************************************************************************/

/**************************************************************************/
/*Options*/
#define _POSIX_C_SOURCE                200112L
#define FRAMERATE_FPS                  30  //fps
#define TIME_TO_CHANGE_FOCUS_SEC       2   //s
#define LONG_WAIT                      500000000L //500ms
#define MOUSE_BTN_DELAY                1000000L //1ms
#define DEFAULT_BG_COLOR               "000000" //rgb color

#define EXIT_KEY                       "g"
#define EXIT_MASK                      (ControlMask | ShiftMask)
#define EXIT_STR                       "CTRL + SHIFT + g"

#define TRANSLATION_CTRL_KEY           "u"
#define TRANSLATION_CTRL_MASK          (ControlMask | ShiftMask)
#define TRANSLATION_CTRL_STR           "CTRL + Shift + u"

#define WM_CLASS_PRG_NAME_STR          "XWinClone"
#define WM_CLASS_CLASS_NAME_STR        "xwinclone"
#define AUTOCENTERING                  1 //1=true; 0=false
#define TOP_OFFSET                     0 // in pixels
#define PROGRAM_EXE_NAME_STR           "xwinclone"
#define BG_IMAGE_PATH                  "./"
#define DEFAULT_LOG_LVL                LOG_LVL_2
#define LOCK_FILE_PATH                 "/tmp/xwinclone.pid"
#define DEFAULT_BGIMAGE_PATH           "/usr/share/wallpapers/e2e4.jpeg"
#define TRACKED_PTR_NAME               "A4Tech USB Optical Mouse"
/**************************************************************************/

#endif


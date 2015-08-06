/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://raw.githubusercontent.com/awpe/xwinclone/master/LICENSE.md>.  */

#include <xwcArgs.h>
#include <xwcHeaders.h>

Bool
argSetDefaultVal (argument * arg)
{
    if (arg == NULL)
    {
        logCtrl ("Cannot set default argumnt value: NULL pointer to argument "
                 "received!", LOG_LVL_NO, False);
        return False;
    }

    switch (arg->m_Name)
    {
        case HELP:
            break;

        case DAEMON:
            break;

        case MULTIINST:
            break;

        case AUTOCENTER:
            *( (int*) arg->m_Val )           = AUTOCENTERING;
            break;

        case TOPOFFSET:
            *( (int*) arg->m_Val )           = TOP_OFFSET;
            break;

        case FOCUSTIME:
            *( (int*) arg->m_Val )           = TIME_TO_CHANGE_FOCUS_SEC;
            break;

        case BGCOLOR:
            arg->m_Val                       = (void*) DEFAULT_BG_COLOR;
            break;

        case LCKFPATH:
            arg->m_Val                       = (void*) LOCK_FILE_PATH;
            break;

        case BGIMAGE:
            arg->m_Val                       = (void*) DEFAULT_BGIMAGE_PATH;
            break;

        case FRAMERATE:
            *( (int*) arg->m_Val )           = FRAMERATE_FPS;
            break;

        case LOGLVL:
            *( (int*) arg->m_Val )           = DEFAULT_LOG_LVL;
            break;

        case SOURCEID:
            *( (unsigned long*) arg->m_Val ) = None;
            break;

        case PTRDEVNAME:
            arg->m_Val                       = (void*) TRACKED_PTR_NAME;
            break;

        case LOGFNAME:
            arg->m_Val                       = (void*) LOG_FILE_NAME;
            break;

        case RAISETIME:
            *( (int*) arg->m_Val )           = RAISE_SOURCE_DELAY;
            break;

        case RESTORETIME:
            *( (int*) arg->m_Val )           = RESTORE_SOURCE_DELAY;
            break;

        case CLICKTIME:
            *( (int*) arg->m_Val )           = BTN_CLICK_DELAY;
            break;

        case LONGWAIT:
            *( (int*) arg->m_Val )           = LONG_WAIT;
            break;

        case PROCBTNEV:
            break;

        case CONFFILE:
            arg->m_Val                       = (void*) CONFIG_FILE_PATH;
            break;

        case MKCONFIG:
            break;

        case EXITKEY:
            arg->m_Val                       = (void*) EXIT_KEY;
            break;

        case CLONEKEY:
            arg->m_Val                       = (void*) TRANSLATION_CTRL_KEY;
            break;

        case CHECKARGS:
            break;

        default:
            logCtrl ("Unknown argument type detected while creating option!",
                     LOG_LVL_NO, False);
            return False;
    }

    return True;
}

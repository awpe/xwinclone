/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcXSrvUtil.h>

Window
getRootWinOfScr (Screen * s)
{
    logCtrl ("Getting root window of screen:", LOG_LVL_1, False);
    if (s == NULL)
    {
        logCtrl ("\tError getting root window of screen: "
                 "Invalid pointer to Screen data struct!", LOG_LVL_NO, True);
        return None;
    }

    if (s->root == None)
    {
        logCtrl ("\tError getting root window of screen: "
                 "No root wondow specified for given screen!", LOG_LVL_NO,
                 True);
    }
    logCtrl ("\tsuccess", LOG_LVL_1, True);
    return s->root;
}

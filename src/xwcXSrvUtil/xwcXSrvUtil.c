/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcXSrvUtil.h>

XCompRedirWin_t      redirWin      = & XCompositeRedirectWindow;
XCompUnRedirWin_t    unRedirWin    = & XCompositeUnredirectWindow;
XCompRedirSubWin_t   redirSubWin   = & XCompositeRedirectSubwindows;
XCompUnRedirSubWin_t unRedirSubWin = & XCompositeUnredirectSubwindows;
/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcArgs.h>
#include <xwcHeaders.h>

arguments *
initArgs ()
{
    arguments * args = (arguments*) malloc (sizeof (arguments ));

    if (args == NULL)
    {
        return NULL;
    }

    args->m_ArgCnt = OPTIONS_COUNT;
    args->m_Args   = (argument**) malloc (sizeof (argument ) * args->m_ArgCnt);

    for (int i = 0; i < args->m_ArgCnt; ++ i)
    {
        args->m_Args[i] = (argument*) malloc (sizeof (argument ));
        memset (args->m_Args[i], 0, sizeof (*args->m_Args[i] ));
        args->m_Args[i]->m_NameStr   = NULL;
        args->m_Args[i]->m_Comment   = NULL;
        args->m_Args[i]->m_SynStrs   = NULL;
        args->m_Args[i]->m_Val       = NULL;
        args->m_Args[i]->m_HasValue  = False;
        args->m_Args[i]->m_IsInConf  = False;
        args->m_Args[i]->m_SetInConf = False;
    }

    return args;
}

/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcUtil.h>

Bool
getArgName (arguments  * args,
            const char * buf,
            argNames   * retName)
{


    if (args == NULL || buf == NULL || retName == NULL)
    {
        return False;
    }

    for (int i = 0; i < args->m_ArgCnt ; ++i)
    {
        if (args->m_Args[i] == NULL)
        {
            return False;
        }

        if (strcmp (buf, args->m_Args[i]->m_NameStr) == STR_EQUAL)
        {
            *retName = args->m_Args[i]->m_Name;
            return True;
            break;
        }

    }

    return False;
}

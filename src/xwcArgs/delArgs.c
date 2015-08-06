/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://raw.githubusercontent.com/awpe/xwinclone/master/LICENSE.md>.  */

#include <xwcArgs.h>
#include <xwcHeaders.h>

void
delArgs (arguments * args)
{
    if (args == NULL)
    {
        return;
    }

    if ( args->m_Args != NULL)
    {
        argument * arg;
        for (int i = 0; i < args->m_ArgCnt; ++ i)
        {
            if ((arg = args->m_Args[i]) != NULL)
            {
                if (arg->m_Val != NULL)
                {
                    if (arg->m_Type == INT)
                    {
                        free (arg->m_Val);
                    }

                    if (arg->m_Type == ULONG)
                    {
                        free (arg->m_Val);
                    }

                    if (arg->m_Type == C_STR)
                    {
                        if (arg->m_NeedFree == True)
                        {
                            //free ((char*) arg->m_Val);
                        }
                    }
                }

                if ( arg->m_SynStrs != NULL)
                {
                    free (arg->m_SynStrs);
                }

                free (arg);
            }
        }
        free (args->m_Args);
    }
    free (args);
}

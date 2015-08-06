/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcUtil.h>

void
printCurValues (arguments  * args)
{
    const char * strTmp;

    if (args == NULL)
    {
        logCtrl ("Cannot print current values: NULL pointer to arguments "
                 "array!", LOG_LVL_NO, False);
        return;
    }

    printf ("\nCURRENT VALUES\n");

    printBlock ("default value shown if no corresponding prompt arg provided, "
                "you can specify option of interest with -ca option and you "
                "will see how program understood it in this section",
                "\t",
                "Notice: ", NULL, 0);

    printf ("\n\tValues:\n");

    for (int i = 0; i < args->m_ArgCnt; ++ i)
    {

        printf ("\t%-8s%-12s\t", " ", args->m_Args[i]->m_NameStr);

        if (   args->m_Args[i]->m_IsSet     == True
            && args->m_Args[i]->m_SetInConf == False)
        {
            printf ("%-12s\t", "prompt");
        }
        else if (args->m_Args[i]->m_SetInConf == True)
        {
            printf ("%-12s\t", "config");
        }
        else
        {
            printf ("%-12s\t", "default");
        }

        if (args->m_Args[i]->m_HasValue == True)
        {
            switch (args->m_Args[i]->m_Type)
            {

                case INT:
                    printf ("%d\n", *( (int*) args->m_Args[i]->m_Val ));
                    break;

                case C_STR:
                    printf ("%s\n", (const char*) args->m_Args[i]->m_Val );
                    break;

                case ULONG:
                    printf ("0x%lx\n",
                            *( (unsigned long*) args->m_Args[i]->m_Val ));
                    break;

                default:
                    logCtrl ("Unknown argument type detected during arguments "
                             "list traversing!\n", LOG_LVL_NO, False);
                    break;
            }
        }
        else
        {
            strTmp = args->m_Args[i]->m_IsSet == True ? "True" : "False";
            printf ("%s\n", strTmp);
        }


    }

    printf ("\n\tpress CTRL + SHIFT + %s to exit program\n",
            (const char*) args->m_Args[EXITKEY]->m_Val);

    printf ("\n\tuse CTRL + SHIFT + %s combination for translation control\n\n",
            (const char*) args->m_Args[CLONEKEY]->m_Val);
}

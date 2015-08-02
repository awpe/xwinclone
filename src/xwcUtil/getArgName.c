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

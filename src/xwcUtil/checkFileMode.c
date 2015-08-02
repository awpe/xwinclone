#include <xwcUtil.h>

Bool
checkFileMode (const char * fname,
               int          mode)
{
    return access (fname, mode) != - 1 ? True : False;
}

#include <xwcArgs.h>
#include <xwcHeaders.h>

Bool
populateArgs (arguments * args)
{
    if (   addArg (args, True,  True,  C_STR, BGCOLOR,     "BGCOLOR",
                   "Sets translation window background color, must be"
                   " specified in RGB format i.e. \"FF0000\" for red"
                   " background",
                   1, "-bgclr")
        == False

        || addArg (args, True,  True,  C_STR, BGIMAGE,     "BGIMAGE",
                   "File name of image to be used as background, relative and"
                   " absolute paths are supported.",
                   1, "-bgimg")
        == False

        || addArg (args, True,  True,  INT,   FRAMERATE,   "FRAMERATE",
                   "Sets framerate in FPS for translation window, choose wisely"
                   " as it takes some resources to copy source window image",
                   1, "-fr"   )
        == False

        || addArg (args, True,  True,  INT,   FOCUSTIME,   "FOCUSTIME",
                   "Time in milliseconds for program to wait for you to move "
                   "focus to window you like to be translated, 0 means "
                   "disabled",
                   1, "-ft"   )
        == False

        || addArg (args, True,  True,  INT,   TOPOFFSET,   "TOPOFFSET",
                   "Specifies top offset in pixels for translation i.e. how "
                   "many top pixels of source window image you want to hide",
                   1, "-toff" )
        == False

        || addArg (args, False, False, C_STR, HELP,        "HELP",
                   "Use it to print this message :)",
                   3,  "-h", "-help", "--help")
        == False

        || addArg (args, True,  True,  INT,   AUTOCENTER,  "AUTOCENTER",
                   "If enabled, xwinclone will translate only center part of "
                   "source window, size is determined by xwinclone own window ",
                   1, "-ac"   )
        == False

        || addArg (args, True,  True,  INT,   LOGLVL,      "LOGLEVEL",
                   "You can choose how much verbose to print to log file, "
                   "possible values are 0, 1, 2",
                   1, "-ll"   )
        == False

        || addArg (args, True,  True,  ULONG, SOURCEID,    "SOURCEID",
                   "Used to explicity set source window id, possible values are"
                   " HEX numbers as you get them from xwininfo or xprop or"
                   " simular application",
                   1, "-srcid")
        == False

        || addArg (args, True,  True,  C_STR, LCKFPATH,    "LOCKFILEPATH",
                   "Path to the file which is used to make sure only one "
                   "instance of xwinclone is running",
                   1, "-lckf")
        == False

        || addArg (args, False, True,  INT,   MULTIINST,   "MULTIINST",
                   "Using this option it is possible to run multiple instances "
                   "of program. It is disabled by default. It is always "
                   "disabled for daemon mode so it will be ignored. You will "
                   "have to specify different control keys for every instance, "
                   "see EXITKEY and CLONEKEY entries.",
                   2, "-m", "-multi")
        == False

        || addArg (args, False, True,  INT,   DAEMON,      "DAEMON",
                   "Using this option you can enable daemon mode, in such a "
                   "mode you are not forced to move focus to source window in "
                   "given time, instead program waits for you to press either "
                   "translation control or exit combination",
                   2, "-d", "-daemon")
        == False

        || addArg (args, True,  True,  C_STR, PTRDEVNAME,  "PTRDEVNAME",
                   "You can specify non default pointer device name which will "
                   "be used to grab click events for translation window and "
                   "also for sending click events to source window, see xinput "
                   "output for valid names",
                   1, "-ptrname")
        == False

        || addArg (args, True,  True,  C_STR, LOGFNAME,    "LOGFNAME",
                   "Name of file where logs will be stored by default it is "
                   "\"stdout\" which can be changed to \"stderr\" or text file "
                   "either using absolute or relative path",
                   1, "-logf")
        == False

        || addArg (args, True,  True,  INT,   CLICKTIME,   "CLICKTIME",
                   "Specifies time in milliseconds between pointer button "
                   "press and release events, 0 means disabled",
                   2, "-clicktime", "-clt")
        == False

        || addArg (args, True,  True,  INT,   RAISETIME,   "RAISETIME",
                   "Specifies time in milliseconds to wait for source window "
                   "to become topmost in window manager's window stack, as "
                   "source window must be drawn on screen in the place you "
                   "want to send click event 0 means disabled",
                   2, "-raisetime", "-raiset")
        == False

        || addArg (args, True,  True,  INT,   RESTORETIME, "RESTORETIME",
                   "Specifies time in milliseconds to wait for source window "
                   "to restore from hidden state (after minimizing to taskbar "
                   "for example), 0 means disabled",
                   2, "-restoretime", "-rest")
        == False

        || addArg (args, True,  True,  INT,   LONGWAIT,    "LONGWAIT",
                   "Specifies time in milliseconds to wait before new attempt "
                   "to redraw target window in case of source or target window "
                   "is not viewable, 0 means default",
                   2, "-longwait", "-lw")
        == False

        || addArg (args, False, True,  INT,   PROCBTNEV,   "PROCBTNEV",
                   "Use this option if you want program to process button "
                   "events from selected device(see PTRDEVNAME).",
                   2, "-procbtnev", "-pbe")
        == False

        || addArg (args, True,  False, C_STR, CONFFILE,    "CONFFILE",
                   "Sets name of config file to be used or created "
                   "(see MKCONFIG).",
                   2, "-conff", "-cf")
        == False

        || addArg (args, False, False, INT,   MKCONFIG,    "MKCONFIG",
                   "Writes out config file. Using CONFFILE argument it is "
                   "possible to provide name of file to be created or "
                   "overwritten. If you haven't specified any other arguments "
                   "default values will be used, otherwise those argument "
                   "values will be stored in created file",
                   1, "-mkconf")
        == False

        || addArg (args, True,  True,  C_STR, EXITKEY,     "EXITKEY",
                   "Sets key to be used with ctrl+shift mask to exit program. "
                   "example $xwinclone -ek \"k\"; #ctrl+shift+k is now exit "
                   "combination.",
                   2, "-exitkey", "-ek")
        == False

        || addArg (args, True,  True,  C_STR, CLONEKEY,    "CLONEKEY",
                   "Sets key to be used with ctrl+shift mask to start/stop "
                   "window translation. See EXITKEY entry for example.",
                   2, "-clonekey", "-ck")
        == False

        || addArg (args, False, False, C_STR, CHECKARGS,   "CHECKARGS",
                   "Print result of arguments parsing.",
                   2, "-ca", "-checkargs")
        == False
        )
    {
        return False;
    }

    return True;
}

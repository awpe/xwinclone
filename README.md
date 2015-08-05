# xwinclone

**X** window system's **WIN**dow **CLONE** application.

Program aimed to be light-weight and utilise few system resources.
    
This program can be used to translate another window's content in its
own window with some features like background color/image, window
centering, start/stop hotkeys,FPS control.

In order to translate window content and grab key combination event,
program communicates with xorg server directly using Xlib and its
composite and xinput2 extensions. Minimal Xlib's composite extension
supported version is 0.2, for xinput2 minimal supported version is 2.2

Window selection mechanism depends on mode, in daemon mode user has to
use key combinations to start/stop translation/program, in normal mode
after program starts user have some time to change focus to window
which content have to be translated. Program registers key press event
for root window on all keyboard devices, so you window manager or other
program must not grab this keys or entire keyboard.

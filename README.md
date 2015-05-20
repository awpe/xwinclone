# xwinclone
This program can be used to translate another window content in its own window.

In order to translate window content and grab key combination event, program 
communicates with xorg server directly using Xlib and its composite extension. 
Minimal Xlib's composite extension supported version is 0.2.

Window selection mechanism: 
    After program starts user have some time to change focus to window which 
    content have to be translated. Program registers key press event for root
    window using this key combination program flow can be stopped.


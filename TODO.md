#Task List

Version 0.1:
- [x] Add exit hotkey
- [x] Show last valid frame if source window becomes unavailable,
 instead of throwing fatal error and killing process.
- [ ] Add configuration file support
- [x] Make command prompt arguments processing routine process any number of 
arguments in any order
- [x] Add background image support
- [x] Add background image name selection option
- [x] Add ability to scale background image
- [ ] Make background image being scaled, add fill option
- [x] Add ability to load jpeg images
- [x] Repair top offset mechanism
- [x] Add log level control option
- [x] Add ability to specify id of source window from command line
- [x] Try to acquire source window pixmap only in case it is in visible state
- [x] Add daemon option
- [x] Send mouse clicks from target window to source window on same coordinates
- [x] Redraw target window with fewer FPS in case of source window being 
unmapped
- [x] If source window was unmapped, make program to wait longer, before 
checking source window state again
- [ ] Add comments to the new part of argument processing routine
- [ ] Add comments to everything
- [x] Clean code
- [x] Document program flow
- [ ] Write dependencies list
- [x] Add singleton option
- [x] Fix error with grabbing same keys from multiple applications
- [x] Fix log text formatting
- [ ] Add option to select exit and translation control key combinations
- [x] Add option to select PID file path
- [x] Add verbose for resource-cleaning routines
- [x] Add option to control writing of log info into specified file
- [x] Control killing program by closing translation window from WM
- [x] Fix coordinates for sending mouse events when in autocentering mode and
 windows are not equal in size
- [x] Split project into multiple files
- [x] Move to XInput 2 extension for keyboard and pointer device grabbing
- [x] Grab control keys for all keyboard devices
- [x] Add option to specify pointer device name for processing mouse-button
 events
- [ ] Use libjpeg-turbo instead of imlib2
- [x] Check if slave pointer is not floating and actually attached to virtual
 master!

Version 0.2:
- [ ] Add routine for automatic conversion of background image from various
 formats to pixmap
- [ ] Add tray support
- [ ] Add rectangle translation support
- [ ] Set propriate logging levels by using all of them (0, 1, 2)
- [ ] Add log file state control or use built-in logging system

Version 0.3:
- [ ] Add remote x server support

Version 0.4:
- [ ] Move to C++11
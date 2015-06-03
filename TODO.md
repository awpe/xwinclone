#Task List

Version 0.1:
- [x] Add exit hotkey
- [x] Show last valid frame if source window becomes unavailable,
 instead of throwing fatal error and killing process.
- [ ] Add configuration file support
- [x] Make command prompt arguments processing routine process any number of 
arguments in any order
- [ ] Add background image support
- [x] Repair top offset mechanism
- [x] Add log level control option
- [x] Add ability to specify id of source window from command line
- [x] Try to acquire source window pixmap only in case it is in visible state
- [x] Add daemon option
- [ ] Send mouse clicks from target window to source window on same coordinates
- [ ] Redraw target window with fewer FPS in case of source window being 
unmapped
- [ ] If source window was unmapped, make program to wait longer, before 
checking source window state again
- [ ] Add comments to the new part of argument processing routine
- [ ] Add comments to everything
- [ ] Clean code
- [ ] Document program flow
- [ ] Write dependencies list
- [x] Add singleton option
- [x] Fix error with grabbing same keys from multiple applications
- [x] Fix log text formatting
- [ ] Add option to select exit and translation control key combinations
- [ ] Add option to select PID file path
- [ ] Add verbose for resource-clearing routines
- [ ] Add option to control writing of log info into specified file
- [ ] add more complex checking of Display pointers, not only for NULL pointer,
 but also for invalid pointer and access errors.

Version 0.2:
- [ ] Add tray support
- [ ] Add rectangle translation support
- [ ] Set propriate logging levels by using all of them (0, 1, 2)

Version 0.3:
- [ ] Add remote x server support

Version 0.4:
- [ ] Move to C++11
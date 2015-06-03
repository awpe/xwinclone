#Task List

Version 0.1:
- [x] Add exit hotkey
- [x] Show last valid frame if source window becomes unavailable,
 instead of throwing fatal error and killing process.
- [x] Add configuration file support
- [x] Make command promt arguments processing routine process any number of 
arguments in any order
- [ ] Add background image support
- [x] Repair top offset mechanism
- [x] Add log level control option
- [ ] Send mouse clicks from target window to source window on same coords
- [x] Add ability to specify id of source window from command line
- [ ] Add comments to the new part of argument processing routine
- [x] Try to acquire source window pixmap only in case it is in visible state
- [ ] Redraw target window with fewer FPS in case of source window being 
unmapped
- [ ] If source window was unmapped, make program to wait longer, before 
checking source window state again

Version 0.2:
- [ ] Add tray support
- [ ] Add rectangle translation support
- [ ] Set propriate logging levels by using all of them (0, 1, 2)

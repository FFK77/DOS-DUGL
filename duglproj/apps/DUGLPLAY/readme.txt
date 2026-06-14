DUGL Player 1.0 Alpha 6 - released 2026-06-14
=============================================

About
-----

DUGL Player is a GUI video/audio player for DOS systems.

License
-------

DUGL Player is (C) by FFK. This a freeware, use it at your own risk.
Source code of player itself is available (C/C++/Assembly language)
and source code/binaries of DUGL+ and binaries of DUGL Core can be
downloaded separately from DUGL website (https://github.com/FFK77/DOS-DUGL)
The player uses mainly FFMPEG library as wrapper to several external libraries 
like LibOGG, LibVorbis and LibTheora by Xiph Foundation, source code is available 
under the BSD-like Xiph license, libx264, libmp3lame, libxml2 .. 
each library under it's own licence

You can use/reuse/modify source code of "DUGL Player" or part of it 
at the following conditions :
1) Don't pretend that you made it or hold copyright over it.


System requirements
-------------------

- DOS or Compatible OS including emulated 
- CPU with MMX support, minimum frequency for playing in real time
  depends from played file, especially video frame size and rate,
  for 640x480 OGG Theora at 30 fps with Vorbis sound 1 GHz is required
- Graphics card with VESA 2.0 support
- Sound card "Sound Blaster 16" or 100% compatible or emulated (tested successfully with VSBHDA)
- Optional Mouse and driver (CTmouse from FreeDOS, Logitech driver, ...)

Features
--------

- Playing video/audio files
- fast forward and rewind using a slider
- Selecting play speed with ratio 1/2, 1(default), 2, 4
- Selectable Aspect Ratio (Built-in list that could be ovewritten by config file)
- Full screen mode
- Nice GUI, control by mouse(optional) or keyboard, volume widget, built-in screenshot feature
..

Supported media fileformats
---------------------------

Any video or audio format supported by current FFMPEG 5.1.8 which is mostly every thing including:
 animated GIF, WEBM, MPEG4, OGG, MOV, MP3, MP2 ...


Missing Features/TODO
---------------------

- Detect/handle video orientation
- Real Time zooming
- Better GUI specially config dialog and better file selection Dialog
- Add in full screen mode a toolbar that shows when mouse is on bottom, with main functions including stop/play, speed, 
  sound volume and fast forward/rewind slider
- Improve Audio/Video synching ..

Keyboard shortcuts :
--------------------

- Alt+X : Exit immedialtely
- Alt+S : Save a BMP screenshot
- Alt+P : Play video from the begining
- Space+Tab or keyboard 'P' : Pause/continue playing
- Space+Enter : Switch between full screen and GUI mode
- F2  : Enable/Disable fast YUV2RGB Mode
- F3  : Show open dialog
- F4  : Close current video file
- F5  : Enable/Disable Video smoothing (full screen mode)
- F6  : Enable/Disable Display FPS (full screen mode)
- F7  : Enable/Disable Display Time (full screen mode)
- F8  : Enable/Disable Vertical sync
- F9  : Enable/Disable Video looping
- F10 : Enable/Disable Frame dropping
- F11 : Switch between blitting video frame into full View or using current selected Aspect Ratio
- F12 : Switch to next Aspect Ratio on the list.

GUI Features :
--------------

Under video view we have From left to Right:
- Play Button: 
Push this button to restart played video audio from begening or Replay an already ended video/audio
- Pause/Continue Button: 
Push this button to stop or continue current playing video/audio
- Play Speed Button: 
Push this button to select playing speed (1/2, x1, x2 or x4)
- Play once or loop Button: 
Push this button to switch between playing video/audio once or looping undefinitely
- Current Aspect Ratio: 
Display current select Aspect Ratio or [Full View] if fit View Select, Mouse Click on this widget to switch next ARatio
- Time: 
Display if there is a current playing Video/Audio else empty. Mouse Click on this Widget to switch between time display modes
- Volume Widget:
Display current volume if sound enabled. Mouse Click/drag on desired volume to update current volume.
- Exit Button:
Push this button to exit immediately.

Performance tuning and "MTRR-hack"
----------------------------------

Playing video is a time consuming task, including file I/O, video
decoding, colorspace conversion, writing to video RAM, and if sound is
present additionally audio decoding. A sufficiently fast CPU is required
(see above) to play in real time. There are a few possibilities to tune
the performance:
- If your BIOS doesn't use DMA, install a DMA driver (XDMA, UIDE or
  similar). This can significantly reduce the time wasted in file I/O.
- To further speed up the file I/O, you can copy the video file into
  a RAM disk, however the space is limited and enough RAM must be left
  free for DUGL Player.
- If your CPU supports "MTRR write combining" (99% of MMX capable CPU's
  do), enable it. DUGL Player will do this itself only if it runs in
  Ring0. You can use the "VESAMTRR" tool by Japheth from his "HXGUI.ZIP"
  package to do this setting in advance. This can significantly speed up
  the write access to video RAM, the effect holds until a CPU reset.
- Disable "Interpolate UV" (F2) can give a speed up for less good image
  quality in some cases.
- Disable "Smoothing" (F5) give a good speed improvement but worst 
  image quality specially for up-sampled videos.
- Disable "VSynch" (F8) 
- With F10 you can enable or disable frame dropping if your CPU is too
  slow. If enabled the video will play in real time but loose frames
  - it will "jump", if the CPU is far insufficient, you won't see
  (almost) anything from your video. If disabled you will see all frames
  but playing will be slow.


History:
--------

28 february 2026: 1.0 alpha 1

2 march 2026 : 1.0 alpha 2
- Add experimental seeking using the horizontal slider. (video is paused once user start seeking)
- Save/restore working directory at player startup/close to avoid changing
  current user directory.
- several fixes and cleanup

26 march 2026 : 1.0 alpha 3
- Add experimental sound decoding/support using Sound Blaster 16 driver
- Improve config file with new sound audio paramters, true|false to enable/disable,
- Add reverting to 640x480 if selected config file video resolution isn't available ...
- Keyboard 'P' now pause continue playing current video

13 April 2026: 1.0 alpha 4
- Implement decoding for audio only files
- Add master volume control widget
- Implement the three usual time progress modes (progress, remaining time, progress / total time)
  user can switch between modes by mouse clicking on the time
- Implement seeking of audio track with video track
- Improve playing mode widget, now image Button (looping or single play)
- severals parameters added to config file
- Improved sound quality, bug fixes

25 April 2026: 1.0 alpha 5
- Add/implement new button to select playing speed with ratio 1/2, 1(default), 2, 4
- Implement audio only files fast forward/rewind with slider
- Improve GUI design with progress slider at full window width and increasing precision to 1000 (was 100)
- Add possibility display audio curve for audio only files, with additional config file fields 
  (enable; background color; curve color; render mode: blit|transluent)
- Revert time display to hh:mm:ss instead of xxhxxmxxs
- Improve Open Dialog File filters

14 June 2026: 1.0 alpha 6
- Now use FFMPEG 5.1.8 with pentium-MMX CPU enabled including assembly optimizations.
- Add support of Aspect Ratio with built-in list that could be overwritten by config file.
- Add fast video decoding (could be switched by config file), to improve decoding speed of old Hardware (Thanks ThorKa!)
- Add Aspect Ratio widget, with capability to switch next one with Mouse Click
- F11 keyboard button now switch between Blitting video frame to full View or blitting according to current Aspect Ratio
- F12 Keyboard button now switch Aspect Ratio to next one one the List.
- Fix Not working VSync on GUI Mode.
- Severals other fixes and speed improvement.
 
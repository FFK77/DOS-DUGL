DUGL Player 1.0 Alpha 1 - released 2026-02-28
=============================================

About
-----

DUGL Player is a GUI video player for DOS systems.

License
-------

DUGL Player is (C) by FFK. This a freeware, use it at your own risk.
Source code of player itself is available (C/C++/Assembly language)
and source code/binaries of DUGL+ and binaries of DUGL Core can be
downloaded separately from DUGL website.
The player uses external libraries LibOGG, LibVorbis and LibTheora
by Xiph Foundation, source code is available under the BSD-like Xiph
license and an updated version of Berkley MPEG1/2 decoder library.
The player is redistribuable, the relevant files
(main executable, support files, this file, source)
should be kept together, but there is no need to include the libraries
or example videos.

You can use/reuse/modify source code if "DUGL Player" or part of it 
at the following conditions :
1) Don't pretend that you made it or hold copyright over it.


System requirements
-------------------

- DOS
- CPU with MMX support, minimum frequency for playing in real time
  depends from played file, especially video frame size and rate,
  for 640x480 OGG Theora at 30 fps with Vorbis sound 1 GHz is required
- Graphics card with VESA 2.0 support
- Optional Mouse and driver (CTmouse from FreeDOS, Logitech driver, ...)

Features
--------

- Playing video files with or without sound
- Nice GUI, control by mouse(optional) or keyboard, built-in screenshot feature

Supported media fileformats
---------------------------

Any format supported by current FFMPEG 5.1.2 which is mostly every thing including animated GIF, WEBM, MPEG4 ...


Missing Features/TODO
---------------------

- Keep Aspect Ratio
- Video seeking
- Playing Video sound
- playing video at full res : currently high-res videos are downsampled to 640x480
- Detect/handle video orientation
- Real Time zooming
- Better GUI specially config dialog and better file selection Dialog
..

Keyboard shortcuts :
--------------------

- Alt+X : Exit immedialtely
- Alt+S : Save a BMP screenshot
- Space+Tab : Pause/continue playing
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
- F11 : Enable/Disable Video Screen Fit

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



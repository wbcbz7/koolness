# koolnESS source code release

A source code for [koolnESS](https://www.pouet.net/prod.php?which=96920) musicdisk for DOS/ESFM by The Furnace Posse, first released at Multimatograf 2024 Wild compo

quite a messy stuff, isn't it? :)

## building instructions

you'll need the following stuff:

* Open Watcom C++ 1.9 (may probably work with 2.0 but 1.9 is what I've used)
* Netwide Assembler 
* Flat Assembler
* [Rocket](https://github.com/emoon/rocket) for synching stuff
* UPX for optional compression (done manually after building)

and a little bit of patience :)

switch to project directory, `wmake` and look for `koolness.exe` in `!polygon` directory. `wmake` may fail on non-DOS/Win32 platforms due to `copy` commands in makefile - you can easily fix that for Unix/etc targets

the need of two different assemblers is because I have some older sources made in FASM (like grid texture mapper) whose were used in this productions, and I had no time to translate the to NASM. Don't worry much - next time i'll use two different C compilers :D

## misc info

As you may have noticed, there are two makefiles for both DOS and Win32 versions - the main difference is that Win32 version was used during intro and GUI development for easier debugging and ability to use Rocket for intro synchronization, but it can't actually play tunes due to ESFM emulation core missing.

Build Win32 .exe by running `wmake -f makewin.mak`. you'll need BASS.DLL to run it, tho :)

## stuff that did not make its way to readme.txt for various reasons

### additional known bugs

* "tapekeep" was misspelled as "tapekeeper"

* 3dfx Banshee/Voodoo 3+ cards can't handle custom 320x200 60hz letterbox tweak in VESA modes, resulting in black screen or "out of range" message, yet reporting themselves as VGA compatible.
  * workaround - use VESA 320x240 mode or VGA 320x200 60hz

### additional credits

* [fast_obj](https://github.com/thisistherk/fast_obj) by thisistherk
* polygon filler is based on fatmap2.zip by MRI\Doomsday
* FLEXPTC is my OpenPTC-like library to handle software rendering by providing common set of screen buffer handling stuff for DOS (VGA/VESA) and Win32 (DirectDraw/DirectX 9/OpenGL)
  * precompiled libraries are available in `lib` folder, and I will opensource them after cleaning all the mess the source code is :)
* `rocket.lib` is based on standard Rocket client libraries, with few modifications for DOS target (like 8.3 filenames for tracks)



--wbcbz7 25.06.2024 



















p.s. happy birthday Natt!
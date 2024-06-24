# quick HOWTO

1. cook a banger choon in furnace (either easiest or hardest part). make sure it's 1xESFM, no other chips should be present. there are no practical limitations

2. either

   1. clone furnace from [the official repo](https://github.com/tildearrow/furnace), rewind to [this](https://github.com/tildearrow/furnace/commit/cf1233002c1825e50d317a173cb07b08781ec5cb) commit, apply `gdp.diff` from `furnace_gdp_export` directory, and recompile furnace.

      sorry for sources in THIS form but as always I messed up :D

   2. or use `furnace_gdp_export.exe` in case you're on Windows and trust prebuilt binaries.

3. open your song, then file->export->govnodump and save as .gdp

4. run `vgm2opl_next.exe -c2 -d2 [filename.gdp] ` to convert .gdp to compressed .opm files.

   - the `-c[x]` and `-d[x]` command line keys control compression method and nested back reference depth, respectively

     `-c1` uses minimum back reference length of 4 tokens during match search, while `-c2` bruteforces the best length (optimal parsing is not implemented yet)

     even if player supports up to 4 nested back reference levels, due to a bug in the compressor match routine, only up to 2 is usable, else compression ratio drops sharply (might have to fix that someday).

5. if you want to get a quick outline of what is the result, run `lxmplay.exe [filename.opm]` and it'll play it in realtime.

6. but! this one is still not quite enough for the music disk, so get a LZ4 and run `lz4 -9 --content-size [filename.opm] [filename.lz4 `. Note that `--content-size` is important, or else LZ4 loader from the disk will refuse to load a module!

7. modify `modulelist` in [player.cpp](https://github.com/wbcbz7/koolness/blob/master/player.cpp) to add a new song to the list, or replace a present one in `!polygon/tunes` (and read the text below)

8. and a last note - please, for the scene gods sake, **don't be a total lamer** by just replacing tunes and logo to pretend it's a new release - use the source code as a guide and make something new and different (and maybe even better!)



## converter credits

apart from usual furnace credits, `lxmplay` (which should be named `opmplay` but that's another story) couldn't have been done without [ESFMu](https://github.com/Kagamiin/ESFMu) emulation core  - thanks a lot! =)

(and it also uses the almighty [PortAudio](https://www.portaudio.com/))
// самая тяжелая вещь - написание текста в скроллер :D

#if 0
const char *scroll_text = 
    "                                                                                  "\
    "hey there! we are proud to present the  "\
    " - needs more scroll text! totally work in progress!                   "\
    "                                                     "\
    "keys: Up/Down - select song, Enter - play it, +/- - volume up/down, Esc - exit"\
    "                                                     "\
    "by the way, it's running at solid 60fps on my Pentium 90 + S3 Trio64 + ES1868, "\
    "and there is still some room for further optimizations (who said 486? :) "\
    "         " \
    " anyway, scroll restarts...."
;

#else

const char *scroll_text = 
    "                                                                                                    "\
    "hey there! we are proud to present the first ever ESFM music disk, made out of the best tunes "\
    "for the ESS AudioDrive/Solo-1 enhanced FM synthesizer, done in Furnace Tracker!                " \
    "                                                                       " \
    "keys: Up/Down - select song, Enter - play it, +/- - volume up/down, "
    "L - select playback mode (loop one song, shuffle entire playlist or loop it), "
    "D - shell to DOS (more on it later), Esc - exit                                                 "\
    "                                                                       " \
    "credits: all code, graphics and design done by wbcbz7 (also known as artemka), featuring music:       "\
    " 1. essneuro (aka Eccentric Super Frequency Modulation) by Abstract 64                "\
    " 2. Cielos ESFuMados by Natt              " \
    " 3. Deadline by Raijin              "
    " 4. Devil Detective by Laggy              "
    " 5. experiment by tapekeeper              "
    " 6. Second Start by Spinning Square Waves              "
    " 7. Walk in the Park by PotaJoe              "
    " 8. Napalm-Loader by GTR3QQ (original by Phandral/Sanxion)              "
    " 9. X EVIL SOUL by dj.tuBIG/MaliceX (original by shyoo)              "
    "                                                                                                            "
    "post-party version - first released at Multimatograf 2024 demoparty"
    "                                                                                                            "
    "a bit of technical background - on the first sight, ESFM looks like just a "
    "ordinary OPL3-compatible clone with subtle differences (i.e. different feedback algorithm), "
    "but in fact it not only performs nearly as well as its Yamaha counterpart, but it also includes "
    "a special native mode with wildly enhanced features. Namely, in native mode, ESFM supports 18 "
    "4-operator channels, each operator can have individual frequency, LFO depth, envelope delay, and instead of using predefined "
    "operator configurations, can both output sound to the mixer and modulate next operator (or, in case of first "
    "operator, modulate itself) with adjustable modulation send. With a good skills, a truly magnificient results can be achieved! "
    "                                                                                                            "
    "during its market heyday, ESFM was mostly underutilized - only two basic MIDI drivers (one for Windows 95, and another for Miles Sound System v3) "
    "were able to use it, and provided the association of ESS chips as 'cheap SBPro clones' and proliferation of "
    "sample-based/wavetable synthesis and, later, streaming music formats, and with the fact that ESFM was not "
    "documented in official ESS datasheets (there are two ESFM-related patents known, tho), the secrets of native mode "
    "were not much known, let alone utilized. However, ESFM was mostly reverse-engineered by 2023, and a software emulator "
    "called ESFMu was developed, which enables the usage of ESFM on any conventional sound device, and in emulators like DOSBox-X (which supports ESFM emulation in latest builds). "
    "As you may known, there is a "
    "open-source multi-chip music editor called Furnace Tracker, which got ESFM support in version 0.6.1, and there was an ESFM demo song contest "
    "held in January-February 2024, the entries of which you are listening to now :)                                           "
    "as for me, I discovered the amazing world of ESFM by pure luck, working on OPL2/3 music packer/player for DOS. I knew about ESFM for a while, but "
    "didn't paid much attention until early April 2024, when I downloaded newest version of Furnace and checked out demo tracks.                         "
    "Honestly speaking, I was totaly blown by ESFM tunes, never have expected such an amazing possibilites from such an underdog chip, so I immediately decided to try "
    "playing these tunes on real hardware (as I have both ES1868 and 1869). Sadly, there were no register dump export for ESFM (for example, the well-known VGM format "
    "supports only the small subset of what Furnace could offer), so I quickly hacked tracker sources to output raw register dump. After writing a prototype player, "
    "I realized that making a DOS music disk out of ESFM tunes might not be a bad idea, so after a couple of sleepless days and nights, I've managed to "
    "implement a compact register dump music format, code a packer and player, and, eventually, this little music disk. In the end, I hope you are satisfied by the result :)"
    "                                                                                                            "
    "now it's time for greets!"
    "                                                                                                            "
    "I like to send my sincere regards to the following people and groups (in almost alphabetical order): "
    ".0b5vr.5711.abaddon.accession.agenda.altair.attentionwhore"
    ".ayce.bitbendaz.buserror.caroline.chlumpie.cocoon.conspiracy.cookiecollective"
    ".copernicum.darkage.dekadence.desire.digimind.digitaldynamite.dub.enhancers"
    ".ephidrena.excess.excessteam.f0x.farbrausch.fenomen.fishbone.floppy"
    ".globalcorp.holon.hprg.jco.jetlag.jin_x.joker"
    ".jumalauta.k2.kpacku.loonies.livecode.demozoo.org.marqueedesign.matt_current.mawi.mayhem"
    ".megus.melondesign.mercury.mfx.mickaleus.moodsplateau.moonshine.moroz1999.nedopc.netro.nihirashi"
    ".nusan.oftenhide.outsiders.oxygene.overlanders.polpo.q-bone.rebels.rift.rkgekk.rpsg"
    ".sands.sceneptallstars.scoopex.sensenstahl.skrju.slipstream.shiru.smfx.speccypl"
    ".softwarefailure.stardust.still.superstande.team210.thesuper.tildearrow.titan(rules!:).trbl"
    ".tbc.tuhb.ukonpower.umlautdesign.unitedforce.vinnny.v-nom.wrighter.xintrea.yamuseum."
    "                                                                                                            "
    "and you!"
    "                                                                                                            "
    "additional respect to everyone in ESFM Discord for support and testing this little prod on various ESS cards, namely Solo-1 and ES1688."
    "                                                                                                  "
    "now a couple of personal messages (because why not?): "
    "                                                                                                            "
    "tildearrow - congratulations with making an ultimate chiptune tool!"
    "                                                                                                  "
    "pator and grongy - thanks for invaluable moral support - without it, I don't even "
    "know if I would finish this project"
    "                                                                                                            "
    "natt - your music is awesome, and cheers for your interest in the music disk!"
    "                                                                                                  "
    "abstract64 - dude you are making incredible stuff, keep it going!"
    "                                                                                                  "
    "polpo - guess we have another emulation to integrate to PicoGUS! this time it can be "
    "a _little_ bit heavyweight for the poor RP2040 :)"
    "                                                                                                  "
    "fatalsnipe - sad that you could't come to the party, we love you!"
    "                                                                                                  "
    "romanrom2 - i heard there is an ISA bus on Sprinter....you know what you need to do ;)"
    "                                                                                                  "
    "ferris - any chance you are still working on a DOS demo? :)"
    "                                                                                                  "
    "tobach - finally finished it! no grass this time (and tic-80 madness), next time probably"
    "                                                                                                  "
    "soda7 - i remember you told in demoscene discord about OPL music you're making, "
    "hope I will finish that DOS OPL executable player so you can use it in the next Revision entry (and not only)"
    "                                                                                                  "
    "0b5vr - hey hey there, great 4k at Revision! any hope for SESSIONS/TDF this year?"
    "                                                                                                  "
    "my pals in sibkrew (rook, n1k-o, r0bat) - when we'll make a new speccy prod? ;)"
    "                                                                                                  "
    "dubmood - with all respect to you and your music, we are not 12 years old anymore :)"
    "                                                                                                  "
    "----------------------end of messages----------------------"
    "                                                                                                  "
    "some more juicy technical bits:                                                    "
    "this musicdisk shell is made using Open Watcom C++ 1.9 (still not 2.0 :shrug:) in protected mode, and uses PMODE/W DOS extender. "
    "The packer itself converts raw ESFM register dump to 19 parallel streams (one for playback control, and 18 per-channel data streams), which are compressed by LZ-like "
    "algorithm. Unlike traditional LZ compression schemes, nothing is actually decompressed in the memory, source literals are used as playback data as-is, and, if "
    "match token (aka back reference) is occuring in the stream, the current position and count of frames to play is saved on call stack, then the position is "
    "stepped N bytes back (where N is defined in back reference parameter), and M frames are played from new position. When M frames are played, the previous position/frames count is pulled "
    "back from stack, and playback contiunes from the next token. This method became popular in ZX Spectrum AY-3-8910 .PSG packers "
    "(namely, my packer was mostly inspired by ayzip by bfox+TmK), "
    "so I've ported it first to OPL2/3, then to ESFM. "
    "As for advantages, there is no additional memory consumption for the history/sliding window buffer (typically up to 64 KB or even higher), you only have to maintain a small (ca. 4 levels) "
    "call stack for nested back references. As for now, I didn't implemented register de-duplication and aggregation yet (like it is done in i.e. PSG2 format on ZX), "
    "so the overall compression ratio is rather poor (like 1:7 on average), and to improve it further, an outer level of compression using LZ4 is implemented. "
    "In summary, this allows to compress the register dump by factor of 1:20-30, and this is not even the final result."
    "                                                                                                         "
    "One of the major ESFM drawbacks is slow register access - each register write require delays to be inserted between each I/O port write (the approximate "
    "minimum delay is about 1.3 microseconds). An ESFM write consists of 3 8-bit register writes, each requiring a delay after I/O access. Since ESFM songs tend to utilize complex "
    "effects, there are lots of register writes in each frame of music data, which leads to horrible frame time fluctualtions (up to 200-500 microseconds per frame, and this is "
    "CPU-independent!). Even with regard to these issues, I've managed to implement smooth 60 Hz GUI on my Pentium 90 + S3 Trio64 + ES1868, and there is still some room for further "
    "optimizations (who said 486? :)"
    "                                                                                                            "
    "There is one trick I have employed to achieve steady and smooth 60 Hz scrolling without vertical retrace IRQ. Remember that VGA does not have a (usable) vertical blank IRQ, so to sync with end of frame, "
    "you need to constantly poll a bit in port 0x3DA to determine if vertical retrace in progress and it is safe to update screen data "
    "without tearing. Unfortunately, the ESFM player works in the background `thread`, which can hold off the main `thread` for enough "
    "time to miss the vertical retrace, and this will result in a skipped frame and visible judder. Of course, I could fix it by "
    "simulating VGA VSync IRQ with IRQ0, but this way I have to run ESFM player at different timer, since some songs use a slightly faster "
    "tempo than 60 Hz. So, i did a simple trick - one I/O delay in the ESFM register write routine is replaced with VGA Input Status (0x3DA) register polling "
    "to determine if there is vertical retrace in progress, and sending it back to the main thread, so if the main thread even misses the retrace because of "
    "ESFM player 'popping' out of blue, it still has a chance to update a frame at 60 Hz. Simple yet effective :)"
    "                                                                                                            "
    "Another issue found is the overflow bug in ES1869 and Solo-1, when the level of all FM channels mixed together "
    "is so high that it overflows an accumulator with audible and harsh sound. As this bug occurs in the "
    "ESFM core itself and not in the mixer, decreasing mixer volume will not fix it. Fortunately, there is a "
    "simple workaround - as any operator has own Output Level register, adjustable by 6 dB steps, it's possible to "
    "tweak this until the resulting audio will not overflow. As Modulation Level and Output Level are "
    "decoupled from each other, there will be no timbre change, and if we change OL for all operators simultaneosly, we will "
    "effectively change the overall level of sound, which can be further adjusted in the mixer. "
    "In my case, I dropped OL of all operators by 12 dB, and the overflow bug no longer appeared on my ES1869, and later it was confirmed "
    "that this trick fixes Solo-1 as well."
    "                                                                                                            "
    "by the way, there are some hidden keys: B - show debug info, F - disable delays for ESFM register writes (may cause audio glitches!), P - reduce graphics "
    "quality, R - show CPU usage raster bar"
    "                                                                                                            "
    "now a couple of words about DOS shell support - yes, you can jam to the ESFM tunes and simultaneously work in DOS! "
    "note that this is wildly experimental stuff, and i take no responsibility for any screw-ups and bugs - use at your own risk :) "
    "                                                                                                  "
    "if any DOS application reprograms IRQ0, music tempo will screw up! in "
    "this case, run musicdisk with 'rtc' switch to use RTC IRQ8 instead of IRQ0."
    "                                                                                                  "
    "by default, PMODE/W is configured to take all the extended memory in "
    "the system, so if you shell out to DOS, EMS/XMS memory is not available. "
    "fortunately, there is a workaround - use external DPMI server such as "
    "HDPMI32, which allocates DPMI memory on demand - see text file for more info. "
    "another neat feature of HDPMI32 is that it can run multiple DPMI apps "
    "at once - including even DOOM ;) "
    "                                                                                                            "
    "alright, enough nonsense, so far this is all I want to tell you in this scrolly. If there is enough interest, I might make a proper final version (with 60 Hz IRQ-driven scroll and "
    "possibly more tunes and stuff), and, or course, a source code and toolchain release. "
    "                                                                                                            "
    "sorry for any typos and bugs in advance! see you in the next production! signing off........... wbcbz7 @ 05.05.2024 22:21"
    "                                                                                                      "
    "................scroller wrapping up................."
    "                                                                                                      "
    "P.S. oh well, as you have already reached to this point - remember demarination by sibkrew and grongy (that Amiga 64k from Revision 2024)?"
    "                                                                      "
    "if you still can't find the hidden game, hold Fire during decrunching until 'bingo' appears, then use joystick and enjoy!"
    "                                                                      "
    "(there is another hidden message in this intro, but it's up to you to find it :)"
    "                                                                                                      "
    "................scroller wrapping up (FOR REAL!)................."
    "                                                                                                      "
;

#endif
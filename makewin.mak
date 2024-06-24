TARGET   = koolness_w
FTARGET  = $(TARGET)

BUILD    = nt
SYSTEM   = nt
DEBUG    = all
DLEVEL   = 0

INCLUDE  = include

AS       = nasm.exe
FAS      = fasm.exe
CC       = wcc386.exe
CP       = wpp386.exe
LD       = wlink.exe
PFLAGS   = -5r -fpi87 -zp16 -zro -oneatxh -s -d$(DLEVEL) -DWIN32 -bt=$(BUILD) -I=$(INCLUDE) -xst
CFLAGS   = -5r -fpi87 -zp16 -zro -oneatxh -s -d$(DLEVEL) -DWIN32 -bt=$(BUILD) -I=$(INCLUDE)
LFLAGS   =
AFLAGS   = -f win32 -l $[&.lst
FAFLAGS  = 

OBJS     = main.obj polydraw.obj polydrwa.obj fonts.obj fontdraw.obj twirldwa.obj bmpload.obj mesh.obj palerp.obj fast_obj.obj linedraw.obj linedrwa.obj 3dclip.obj 3dsort.obj fxmath.obj
OBJS 	 = $(OBJS) rectdrwa.obj unlz4.obj gridlerp.obj introrc.obj
OBJS     = $(OBJS) opmplay.obj player.obj fontdrwa.obj
OBJS     = $(OBJS) menu.obj atmta.obj 
OBJSTR   = file {$(OBJS)}

LIBS     = lib/win32/flexptc.lib lib/win32/rocket.lib lib/win32/bass.lib wsock32.lib
LIBSTR   = library {$(LIBS)}

all: $(FTARGET).exe .symbolic

$(FTARGET).exe : $(OBJS) .symbolic
	%write $(TARGET).lnk debug $(DEBUG)
	%write $(TARGET).lnk name $(FTARGET)
	%write $(TARGET).lnk option map=$(FTARGET).map
	%write $(TARGET).lnk option eliminate
	%write $(TARGET).lnk $(OBJSTR)
	%write $(TARGET).lnk $(LIBSTR)
	%write $(TARGET).lnk system $(SYSTEM)
	$(LD) @$(TARGET).lnk $(LFLAGS)
	wstrip $(FTARGET).exe
	copy /y $(FTARGET).exe !polygon
	del $(FTARGET).exe
	del $(TARGET).lnk

.c.obj:
	$(CC) $< $(CFLAGS)

.cpp.obj:
	$(CP) $< $(PFLAGS)

.asm.obj:
	$(AS) $< $(AFLAGS)

gridlerp.obj:
	$(FAS) gridlerp.fas $(FAFLAGS)

# clean all
clean: .symbolic
	del *.lst
	del *.obj
	del $(FTARGET).exe
	del *.err

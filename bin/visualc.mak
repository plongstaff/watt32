#
#  Makefile for Waterloo TCP sample applications
#
#  Microsoft Visual-C / Win32 executables.
#

#
# Build using static library (1) wattcpvc.lib or use the watt-32.dll
# via the import library wattcpvc_imp.lib (0).
#
STATIC_LIB = 0

#
# Debug-mode or release-mode libraries are used. Make sure wattcp*.lib
# was compiled with the same configuration. If Watt-32 libs where built
# with "-M?d" and "-D_DEBUG" you should set DEBUG_MODE = 1.
#
# These configurations are possible:
#
#  Option Runtime      Thread-safe Debug/release
#  -------------------------------------------------
#  -MD    msvcrt.dll   Yes         release (normal)
#  -MDd   msvcrtd.dll  Yes         debug
#  -ML    libc.lib     No          release
#  -MLd   libcd.lib    No          debug
#  -MT    libcmt.lib   Yes         release
#  -MTd   libcmtd.lib  Yes         debug
#
# The below  '$(CPU)' can be set on cmd-line:
#   nmake CPU=x64 visualc.mak
#
# to force the 64-bit version of the Watt-32 library.
#
!if "$(CPU)." == "."
CPU = x86
!endif

# Build with DEBUG_MODE=1 if you want to profile the program with
# .\vcprof.bat.
#
DEBUG_MODE = 0

#
# Set to 1 to use Mpatrol malloc-debugger.
#
# Notes: The directory of mpatrol.h should be in %INCLUDE.
#        The directory of mpatrol*.lib should be in %LIB path.
#
USE_MPATROL = 0


CC      = cl
CFLAGS  = -nologo -DWIN32 -EHsc -W3 -Gy -Zi -I..\inc \
          -D_CRT_NONSTDC_NO_WARNINGS -D_CRT_OBSOLETE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS # -DUNICODE -D_UNICODE
LDFLAGS = -nologo -fixed:no -mapinfo:exports -incremental:no

!if $(DEBUG_MODE) == 1
DEBUG   = _d
CFLAGS  = $(CFLAGS) -MDd -Os -D_DEBUG -RTCc -GS -GF # -Gr
LDFLAGS = $(LDFLAGS) -debug
!else
DEBUG   =
CFLAGS  = $(CFLAGS) -MD -Ot -GS # -Gr
LDFLAGS = $(LDFLAGS) -debug     #-release
!endif

!if $(STATIC_LIB) == 1
CFLAGS  = $(CFLAGS) -DWATT32_STATIC
WATTLIB = ..\lib\$(CPU)\wattcpvc$(DEBUG).lib
LIBS    = $(WATTLIB) advapi32.lib user32.lib

!else
WATTLIB = ..\lib\$(CPU)\wattcpvc_imp$(DEBUG).lib
LIBS    = $(WATTLIB)
!endif

!if $(USE_MPATROL) == 1
CFLAGS = $(CFLAGS) -MT -DUSE_MPATROL
LIBS   = $(LIBS) libmpatrolmt.lib imagehlp.lib
!endif


PROGRAMS = popdump.exe  rexec.exe   tcpinfo.exe cookie.exe  \
           daytime.exe  dayserv.exe finger.exe  host.exe    \
           lpq.exe      lpr.exe     ntime.exe   ph.exe      \
           stat.exe     htget.exe   revip.exe   vlsm.exe    \
           whois.exe    ping.exe    ident.exe   country.exe \
           tracert.exe con-test.exe gui-test.exe

all:  $(PROGRAMS)
!if $(DEBUG_MODE) == 1
      @echo Visual-C binaries (debug) done
!else
      @echo Visual-C binaries (release) done
!endif

$(PROGRAMS): $(WATTLIB) visualc.mak

gui-test.exe: w32-test.c # tmp.res
      $(CC) -c $(CFLAGS) -DIS_GUI=1 -Fogui-test.obj w32-test.c
      link $(LDFLAGS) -subsystem:windows -map:$*.map -out:$*.exe gui-test.obj $(LIBS)

con-test.exe: w32-test.c # tmp.res
      $(CC) -c $(CFLAGS) -Focon-test.obj w32-test.c
      link $(LDFLAGS) -subsystem:console -map:$*.map -out:$*.exe con-test.obj $(LIBS)

TRACERT_CFLAGS = $(CFLAGS) -DUSE_GEOIP # -DPROBE_PROTOCOL=IPPROTO_TCP

tracert.exe: tracert.c geoip.c # tmp.res
      $(CC) -c $(TRACERT_CFLAGS) tracert.c geoip.c
      link $(LDFLAGS) -verbose -map:$*.map -out:$*.exe tracert.obj geoip.obj $(LIBS) > link.tmp
      cat link.tmp >> tracert.map

.c.exe: # tmp.res
      $(CC) -c $(CFLAGS) $*.c
      link $(LDFLAGS) -map:$*.map -out:$*.exe $*.obj $(LIBS)

.c.i:
      $(CC) -E -c $(CFLAGS) $*.c > $*.i

tmp.res: tmp.rc
      rc -nologo -DDEBUG=0 -D_MSC_VER -DBITS=32 -Fo tmp-32.res tmp.rc

# tmp.rc:

clean:
      - @del *.exe
      - @del *.ilk
      - @del *.map
      - @del *.obj
      - @del *.pdb
      - @del *.pbo
      - @del *.pbt
      - @del *.pbi
      - @del *._xe


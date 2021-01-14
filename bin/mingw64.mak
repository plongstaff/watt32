#
#  GNU Makefile for some Waterloo TCP sample applications
#  Gisle Vanem 2013
#
#  Target:
#    GNU C 4+ (MinGW-w64 or TDM-gcc)
#

#
# If '$(CPU)=x64', build 64-bit programs. Otherwise 32-bit programs.
#
ifeq ($(CPU),)
  CPU = x86
endif

#
# GNU-make is case-sensitive
#
ifeq ($(CPU),x64)
  CPU = X64
endif

#
# Set to 1 to link using static libwatt32.a
#
STATIC_LIB = 0

#
# Define 'MAKE_MAP = 1' if you like a .map-file
#
MAKE_MAP = 1

CC      = gcc
CFLAGS  = -g -Wall -W -Wno-sign-compare -Wno-address -I../inc
LDFLAGS = -s

#
# Define 'NO_OPTIMIZE=1' on make cmd-line to ease debugging
#
NO_OPTIMIZE ?= 0

ifeq ($(NO_OPTIMIZE),1)
  CFLAGS += -O0
else
  CFLAGS += -O2
endif

ifeq ($(CPU),X64)
  CFLAGS += -m64
  LIBDIR = ../lib/x64
else
  CFLAGS += -m32
  LIBDIR = ../lib/x86
endif

ifeq ($(STATIC_LIB),1)
  CFLAGS  += -DWATT32_STATIC
# LDFLAGS += -Wl,--enable-stdcall-fixup
  WATT_LIB = $(LIBDIR)/libwatt32.a
else
  WATT_LIB = $(LIBDIR)/libwatt32.dll.a
endif

ifeq ($(MAKE_MAP),1)
  MAPFILE = -Wl,--print-map,--sort-common,--cref > $(@:.exe=.map)
endif

PROGS = ping.exe     popdump.exe  rexec.exe    tcpinfo.exe  cookie.exe   \
        daytime.exe  dayserv.exe  finger.exe   host.exe     lpq.exe      \
        lpr.exe      ntime.exe    ph.exe       stat.exe     htget.exe    \
        revip.exe    vlsm.exe     whois.exe    wol.exe      eth-wake.exe \
        ident.exe    country.exe  con-test.exe gui-test.exe tracert.exe

all: $(PROGS)
	@echo 'MingW64-w64 binaries done. $$(CPU)=$(CPU).'

tracert.exe: EXTRAS += geoip.c -DUSE_GEOIP # -DPROBE_PROTOCOL=IPPROTO_TCP

con-test.exe: w32-test.c $(WATT_LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o con-test.exe $^ $(EXTRAS) $(MAPFILE)

gui-test.exe: w32-test.c $(WATT_LIB)
	$(CC) -DIS_GUI=1 $(CFLAGS) $(LDFLAGS) -Wl,--subsystem,windows \
	      -o $*.exe $^ $(EXTRAS) $(MAPFILE)

%.exe: %.c $(WATT_LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $*.exe $(EXTRAS) $^ $(MAPFILE)


clean:
	rm -f $(PROGS)

SOURCES = ping.c    popdump.c rexec.c   tcpinfo.c cookie.c   \
          daytime.c dayserv.c finger.c  host.c    lpq.c      \
          lpr.c     ntime.c   ph.c      stat.c    htget.c    \
          revip.c   vlsm.c    whois.c   wol.c     eth-wake.c \
          ident.c   country.c tracert.c w32-test.c

path_find = $(wildcard $(addsuffix /$(1),$(subst ;, ,$(subst \,/,$(PATH)))))

ifneq ($(call path_find,python.exe),)
  DEP_CFLAGS = -M $(CFLAGS) | python.exe normpath.py -
else
  DEP_CFLAGS = -MM $(CFLAGS)
endif

depend:
	$(CC) $(SOURCES) $(DEP_CFLAGS) > .depend.MingW64

-include .depend.MingW64

#
# These are needed for MSYS' make (sigh)
#
ping.exe:     ping.c
popdump.exe:  popdump.c
rexec.exe:    rexec.c
tcpinfo.exe:  tcpinfo.c
cookie.exe:   cookie.c
daytime.exe:  daytime.c
dayserv.exe:  dayserv.c
finger.exe:   finger.c
host.exe:     host.c
lpq.exe:      lpq.c
lpr.exe:      lpr.c
ntime.exe:    ntime.c
ph.exe:       ph.c
stat.exe:     stat.c
htget.exe:    htget.c
revip.exe:    revip.c
vlsm.exe:     vlsm.c
whois.exe:    whois.c
wol.exe:      wol.c
eth-wake.exe: eth-wake.c
ident.exe:    ident.c
country.exe:  country.c
tracert.exe:  tracert.c # geoip.c


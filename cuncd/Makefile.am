## Makefile.am
##
## cuncd/ Makefile.am for Cunctator
##
## (C) Copyright 2011-2022 Fred Gleason <fredg@paravelsystems.com>
##
## Use automake to process this into a Makefile.in
##
##   This program is free software; you can redistribute it and/or modify
##   it under the terms of the GNU General Public License version 2 as
##   published by the Free Software Foundation.
##
##   This program is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY; without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##   GNU General Public License for more details.
##
##   You should have received a copy of the GNU General Public
##   License along with this program; if not, write to the Free Software
##   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
##

AM_CPPFLAGS = -Wall -DPREFIX=\"$(prefix)\" -Wno-strict-aliasing -std=c++11 -fPIC @QT5_GUI_CFLAGS@  @JACK_CFLAGS@ -I$(top_srcdir)/lib
LIBS = -L$(top_srcdir)/lib
MOC = @QT5_MOC@

# QT's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	@MOC@ $< -o $@

sbin_PROGRAMS = cuncd
dist_cuncd_SOURCES = airtools.cpp airtools.h\
                     bd600.cpp bd600.h\
                     connection.cpp connection.h\
                     cuncd.cpp cuncd.h\
                     cuncconfig.cpp cuncconfig.h\
                     delay.cpp delay.h\
                     dummy.cpp dummy.h\
                     jackdelay.cpp jackdelay.h\
                     rmlengine.cpp rmlengine.h\
                     udpqueue.cpp udpqueue.h\
                     globals.h

nodist_cuncd_SOURCES = moc_airtools.cpp\
                       moc_bd600.cpp\
                       moc_connection.cpp\
                       moc_cuncd.cpp\
                       moc_cuncconfig.cpp\
                       moc_delay.cpp\
                       moc_dummy.cpp\
                       moc_jackdelay.cpp\
                       moc_rmlengine.cpp\
                       moc_udpqueue.cpp

cuncd_LDADD = @JACK_LIBS@ @CUNC_LIBS@ @QT5_GUI_LIBS@

EXTRA_DIST = cuncd.pro

CLEANFILES = *~ moc_* *.obj *.exe *.idb *.pdb *.ilk
DISTCLEANFILES = moc_*
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 moc_* *.tar.gz

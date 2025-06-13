## Makefile.am
##
## Makefile.am for lib/
##
## Use automake to process this into a Makefile.in
##
##   (C) Copyright 2011-2025 Fred Gleason <fredg@paravelsystems.com>
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

AM_CPPFLAGS = -Wall -Wno-strict-aliasing -std=c++17 -fPIC @QT6_CFLAGS@
MOC = @QT6_MOC@

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	@QT6_MOC@ $< -o $@

lib_LTLIBRARIES = libcunc.la
dist_libcunc_la_SOURCES = cmdswitch.cpp cmdswitch.h\
                          profile.cpp profile.h\
                          pushbutton.cpp pushbutton.h\
                          ttydevice.cpp ttydevice.h\
                          cunc.h

nodist_libcunc_la_SOURCES = moc_pushbutton.cpp\
                            moc_ttydevice.cpp

libcunc_la_LDFLAGS = -release $(VERSION)

EXTRA_DIST = lib.pro

CLEANFILES = *~ moc_* *.obj *.lib *.idb *.pdb *.ilk
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz moc_*

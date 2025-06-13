## Makefile.am
##
## Makefile.am for cunc/
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
## Use automake to process this into a Makefile.in

AM_CPPFLAGS = -Wall -DPREFIX=\"$(prefix)\" -Wno-strict-aliasing -std=c++17 -fPIC @QT6_CFLAGS@ $(top_srcdir)/lib
LIBS = -L$(top_srcdir)/lib

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	@QT6_MOC@ $< -o $@

bin_PROGRAMS = cunc

dist_cunc_SOURCES = cuncclient.cpp cuncclient.h
nodist_cunc_SOURCES = moc_cuncclient.cpp
cunc_LDADD = @CUNC_LIBS@ @QT6_LIBS@

EXTRA_DIST = cunc.pro

CLEANFILES = *~ moc_* *.qm *.obj *.idb *.pdb *ilk
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz moc_*

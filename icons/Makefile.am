## Makefile.am
##
## icons/Makefile.am for Cunctator
##
## (C) Copyright 2011 Fred Gleason <fredg@salemradiolabs.com>
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
##
## Use automake to process this into a Makefile.in

install-exec-am:	
	mkdir -p $(DESTDIR)@prefix@/share/icons/hicolor/16x16/apps
	cp cunc-16x16.png $(DESTDIR)@prefix@/share/icons/hicolor/16x16/apps/cunc.png
	mkdir -p $(DESTDIR)@prefix@/share/icons/hicolor/32x32/apps
	cp cunc-32x32.png $(DESTDIR)@prefix@/share/icons/hicolor/32x32/apps/cunc.png
	mkdir -p $(DESTDIR)@prefix@/share/icons/hicolor/48x48/apps
	cp cunc-48x48.png $(DESTDIR)@prefix@/share/icons/hicolor/48x48/apps/cunc.png


uninstall-local:	
	rm -f $(DESTDIR)@prefix@/share/icons/hicolor/16x16/apps/cunc.png
	rm -f $(DESTDIR)@prefix@/share/icons/hicolor/32x32/apps/cunc.png
	rm -f $(DESTDIR)@prefix@/share/icons/hicolor/48x48/apps/cunc.png

EXTRA_DIST = cunc-16x16.png\
             cunc-16x16.xpm\
             cunc-32x32.png\
             cunc-32x32.xpm\
             cunc-48x48.png\
             cunc-48x48.xpm

CLEANFILES = *~
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4

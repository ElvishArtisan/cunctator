## automake.am
##
## Top-level Makefile.am for Cunctator
##
## Use automake to process this into a Makefile.in
##
##   (C) Copyright 2011 Fred Gleason <fredg@paravelsystems.com>
##
##     $Id: Makefile.am,v 1.7 2011/03/16 15:32:36 cvs Exp $
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

SUBDIRS = lib cuncd cunc conf docs icons xdg

install-exec-local:	
	mkdir -p /etc/init.d
	cp rc.cuncd /etc/init.d/cuncd

uninstall-exec:
	rm -f /etc/init.d/cuncd

rpm:	dist
	cp cunctator-$(VERSION).tar.gz $(RPM_ROOT)/SOURCES/
	$(RPMBUILD) -ba cunctator.spec
	mv $(RPM_ROOT)/RPMS/$(ARCH)/cunctator-$(VERSION)-@RPM_RELEASE@.$(ARCH).rpm $(top_srcdir)/
	mv $(RPM_ROOT)/SRPMS/cunctator-$(VERSION)-@RPM_RELEASE@.src.rpm $(top_srcdir)/
	rm $(RPM_ROOT)/SOURCES/cunctator-$(VERSION).tar.gz
	rm -rf $(RPM_ROOT)/BUILD/cunctator-$(VERSION)

EXTRA_DIST = cunctator.spec.in cunctator.pro autogen.sh\
             get_target.sh get_distro.sh\
             PACKAGE_VERSION rc.cuncd

zip:	dist
	tar -zxf cunctator-@VERSION@.tar.gz
	zip -r cunctator-@VERSION@.zip cunctator-@VERSION@
	rm -rf cunctator-@VERSION@

CLEANFILES = *~ moc_* *.tgz *.mo *.rpm *.zip
DISTCLEANFILES = *~ moc_* *.tar.gz *.spec dvt-bin.conf *.spec
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 moc_* *.tar.gz\
                       depcomp install-sh missing mkinstalldirs\
                       config.guess config.sub ltmain.sh

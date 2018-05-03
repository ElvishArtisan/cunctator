## automake.am
##
## Top-level Makefile.am for Cunctator
##
## Use automake to process this into a Makefile.in
##
##   (C) Copyright 2011-2018 Fred Gleason <fredg@paravelsystems.com>
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

ACLOCAL_AMFLAGS = -I m4

maintainer-clean-local:	
	rm -rf m4

install-exec-local:	
	mkdir -p /etc/init.d
	cp rc.cuncd /etc/init.d/cuncd

uninstall-exec:
	rm -f /etc/init.d/cuncd

rpm:	dist
	mkdir -p $(HOME)/rpmbuild/SOURCES
	cp cunctator-$(VERSION).tar.gz $(HOME)/rpmbuild/SOURCES/
	$(RPMBUILD) -ba --target $(ARCH)-$(VENDOR)-linux cunctator.spec
	mv $(HOME)/rpmbuild/RPMS/$(ARCH)/cunctator-*.rpm $(top_srcdir)/
	mv $(HOME)/rpmbuild/SRPMS/cunctator-*.src.rpm $(top_srcdir)/
	rm $(HOME)/rpmbuild/SOURCES/cunctator-$(VERSION).tar.gz
	rm -rf $(HOME)/rpmbuild/BUILD/cunctator-$(VERSION)

EXTRA_DIST = cunctator.spec.in cunctator.pro autogen.sh\
             get_target.sh get_distro.pl\
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

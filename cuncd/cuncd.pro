# davit.pro
#
# The davit/ QMake project file for Davit
#
# (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
#
#      $Id: cuncd.pro,v 1.1.1.1 2011/02/16 23:05:34 cvs Exp $
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License version 2 as
#   published by the Free Software Foundation.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public
#   License along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

TEMPLATE = app

TARGET = davit

win32 {
  DEFINES += WIN32
  DEFINES += VERSION=\"$$[VERSION]\"
  DEFINES += PACKAGE=\"davit\" 
  DEFINES += PACKAGE_VERSION=\"$$[VERSION]\" 
  DEFINES += PACKAGE_NAME=\"davit\"
  DEFINES += PACKAGE_BUGREPORT=\"fredg@paravelsystems.com\"
}

SOURCES += add_affiliate.cpp
SOURCES += add_program.cpp
SOURCES += add_provider.cpp
SOURCES += add_remark.cpp
SOURCES += add_user.cpp
SOURCES += affiliate_report.cpp
SOURCES += createdb.cpp
SOURCES += davit.cpp
SOURCES += daypicker.cpp
SOURCES += edit_affiliate.cpp
SOURCES += edit_airing.cpp
SOURCES += edit_feed.cpp
SOURCES += edit_network.cpp
SOURCES += edit_program.cpp
SOURCES += edit_provider.cpp
SOURCES += edit_user.cpp
SOURCES += generate_affadavit.cpp
SOURCES += list_affiliates.cpp
SOURCES += list_networks.cpp
SOURCES += list_programs.cpp
SOURCES += list_providers.cpp
SOURCES += list_reports.cpp
SOURCES += list_users.cpp
SOURCES += migrate_affiliates.cpp
SOURCES += mysql_login.cpp
SOURCES += opendb.cpp
SOURCES += pick_daypart.cpp
SOURCES += pick_fields.cpp
SOURCES += precisiontrak.cpp
SOURCES += program_report.cpp
SOURCES += activity_report.cpp
SOURCES += addedprograms_report.cpp
SOURCES += arbitron_report.cpp
SOURCES += affiliatesbyprogram_report.cpp
SOURCES += raaffiliate_report.cpp
SOURCES += affidavit_report.cpp
SOURCES += missingaffidavit_report.cpp
SOURCES += list_contacts.cpp
SOURCES += contactlistview.cpp
SOURCES += edit_contact.cpp
SOURCES += contact.cpp
SOURCES += showaffidavits.cpp
SOURCES += maildialog.cpp
SOURCES += edit_system.cpp
SOURCES += list_airings.cpp
SOURCES += list_remarks.cpp
SOURCES += geometry.cpp

HEADERS += add_affiliate.h
HEADERS += add_program.h
HEADERS += add_provider.h
HEADERS += add_remark.h
HEADERS += add_user.h
HEADERS += createdb.h
HEADERS += davit.h
HEADERS += daypicker.h
HEADERS += edit_affiliate.h
HEADERS += edit_airing.h
HEADERS += edit_feed.h
HEADERS += edit_network.h
HEADERS += edit_program.h
HEADERS += edit_provider.h 
HEADERS += edit_user.h
HEADERS += generate_affadavit.h
HEADERS += globals.h
HEADERS += list_affiliates.h
HEADERS += list_networks.h
HEADERS += list_programs.h
HEADERS += list_providers.h
HEADERS += list_reports.h
HEADERS += list_users.h
HEADERS += migrate_affiliates.h
HEADERS += mysql_login.h
HEADERS += opendb.h
HEADERS += pick_daypart.h
HEADERS += pick_fields.h
HEADERS += precisiontrak.h
HEADERS += affidavit_report.h
HEADERS += missingaffidavit_report.h
HEADERS += list_contacts.h
HEADERS += contactlistview.h
HEADERS += edit_contact.h
HEADERS += contact.cpp
HEADERS += showaffidavits.h
HEADERS += maildialog.h
HEADERS += edit_system.h
HEADERS += list_airings.h
HEADERS += list_remarks.h
HEADERS += geometry.h

RES_FILE += ..\icons\davit.res

INCLUDEPATH += ..\lib

LIBS = -lqui -L..\lib -llib

CONFIG += qt

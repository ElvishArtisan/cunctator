// profilesection.h
//
// A container class for profile sections.
//
// (C) Copyright 2011 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: profilesection.h,v 1.1 2011/02/17 22:45:41 cvs Exp $
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Library General Public License 
//   version 2 as published by the Free Software Foundation.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//


#ifndef PROFILESECTION_H
#define PROFILESECTION_H

#include <vector>

using namespace std;

#include <qstring.h>

#include <profileline.h>

class ProfileSection
{
 public:
  ProfileSection();
  QString name() const;
  void setName(QString name);
  bool getValue(QString tag,QString *value) const;
  void addValue(QString tag,QString value);
  void clear();

 private:
  QString section_name;
  vector<ProfileLine> section_line;
};


#endif  // PROFILESECTION_H

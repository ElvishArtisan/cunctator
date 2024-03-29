// profile.cpp
//
// A class to read an ini formatted configuration file.
//
// (C) Copyright 2002-2022 Fred Gleason <fredg@paravelsystems.com>
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

#include <QFile>
#include <QStringList>
#include <QTextStream>

#include "profile.h"

ProfileLine::ProfileLine()
{
  clear();
}


QString ProfileLine::tag() const
{
  return line_tag;
}


void ProfileLine::setTag(QString tag)
{
  line_tag=tag;
}


QString ProfileLine::value() const
{
  return line_value;
}


void ProfileLine::setValue(QString value)
{
  line_value=value;
}


void ProfileLine::clear()
{
  line_tag="";
  line_value="";
}





ProfileSection::ProfileSection()
{
  clear();
}


QString ProfileSection::name() const
{
  return section_name;
}


void ProfileSection::setName(QString name)
{
  section_name=name;
}


bool ProfileSection::getValue(QString tag,QString *value) const
{
  for(int i=0;i<section_line.size();i++) {
    if(section_line[i].tag()==tag) {
      *value=section_line[i].value();
      return true;
    }
  }
  return false;
}


void ProfileSection::addValue(QString tag,QString value)
{
  section_line.push_back(ProfileLine());
  section_line.back().setTag(tag);
  section_line.back().setValue(value);
}


void ProfileSection::clear()
{
  section_name="";
  section_line.clear();
}





Profile::Profile()
{
}


QString Profile::source() const
{
  return profile_source;
}


bool Profile::setSource(const QString &filename)
{
  QString section;
  int offset;

  profile_source=filename;
  profile_section.clear();
  profile_section.push_back(ProfileSection());
  profile_section.back().setName("");
  QFile *file=new QFile(filename);
  if(!file->open(QIODevice::ReadOnly)) {
    delete file;
    return false;
  }
  QTextStream *text=new QTextStream(file);
  QString line=text->readLine().trimmed();
  while(!line.isNull()) {
    if((line.left(1)!=";")&&(line.left(1)!="#")) {
      if((line.left(1)=="[")&&(line.right(1)=="]")) {
	section=line.mid(1,line.length()-2);
	profile_section.push_back(ProfileSection());
	profile_section.back().setName(section);
      }
      else if(((offset=line.indexOf('='))!=-1)) {
	profile_section.back().
	  addValue(line.left(offset),
		   line.right(line.length()-offset-1).trimmed());
      }
    }
    line=text->readLine().trimmed();
  }
  delete text;
  delete file;
  return true;
}


void Profile::setSourceString(const QString &str)
{
  QStringList lines;
  QString section;
  int offset;

  profile_source="";
  profile_section.clear();
  profile_section.push_back(ProfileSection());
  profile_section.back().setName("");
  lines=str.split("\n");
  for(int i=0;i<lines.size();i++) {
    QString line=lines[i];
    if((line.left(1)!=";")&&(line.left(1)!="#")) {
      if((line.left(1)=="[")&&(line.right(1)=="]")) {
	section=line.mid(1,line.length()-2);
	profile_section.push_back(ProfileSection());
	profile_section.back().setName(section);
      }
      else if(((offset=line.indexOf('='))!=-1)) {
	profile_section.back().
	  addValue(line.left(offset),
		   line.right(line.length()-offset-1).trimmed());
      }
    }
  }
}


QString Profile::stringValue(QString section,QString tag,
			      QString default_str,bool *ok) const
{
  QString result;

  for(int i=0;i<profile_section.size();i++) {
    if(profile_section[i].name()==section) {
      if(profile_section[i].getValue(tag,&result)) {
	if(ok!=NULL) {
	  *ok=true;
	}
	return result;
      }
      if(ok!=NULL) {
	*ok=false;
      }
      return default_str;
    }
  }
  if(ok!=NULL) {
    *ok=false;
  }
  return default_str;
}


int Profile::intValue(QString section,QString tag,
		       int default_value,bool *ok) const
{
  bool valid;

  int result=stringValue(section,tag).toInt(&valid,10);
  if(!valid) {
    if(ok!=NULL) {
      *ok=false;
    }
    return default_value;
  }
  if(ok!=NULL) {
    *ok=true;
  }
  return result;
}


int Profile::hexValue(QString section,QString tag,
		       int default_value,bool *ok) const
{
  bool valid;

  QString str=stringValue(section,tag);
  if(str.left(2).toLower()=="0x") {
    str=str.right(str.length()-2);
  }
  int result=str.toInt(&valid,16);
  if(!valid) {
    if(ok!=NULL) {
      *ok=false;
    }
    return default_value;
  }
  if(ok!=NULL) {
    *ok=true;
  }
  return result;
}


float Profile::floatValue(QString section,QString tag,
			   float default_value,bool *ok) const
{
  bool valid;

  float result=stringValue(section,tag).toDouble(&valid);
  if(!valid) {
    if(ok!=NULL) {
      *ok=false;
    }
    return default_value;
  }
  if(ok!=NULL) {
    *ok=true;
  }
  return result;
}


double Profile::doubleValue(QString section,QString tag,
			    double default_value,bool *ok) const
{
  bool valid;

  double result=stringValue(section,tag).toDouble(&valid);
  if(!valid) {
    if(ok!=NULL) {
      *ok=false;
    }
    return default_value;
  }
  if(ok!=NULL) {
    *ok=true;
  }
  return result;
}


bool Profile::boolValue(QString section,QString tag,
			 bool default_value,bool *ok) const
{
  bool valid;

  QString str=stringValue(section,tag,"",&valid).toLower();
  if(!valid) {
    if(ok!=NULL) {
      *ok=false;
    }
    return default_value;
  }
  if((str=="yes")||(str=="true")||(str=="on")) {
    if(ok!=NULL) {
      *ok=true;
    }
    return true;
  }
  if((str=="no")||(str=="false")||(str=="off")) {
    if(ok!=NULL) {
      *ok=true;
    }
    return false;
  }
  if(ok!=NULL) {
    *ok=false;
  }
  return default_value;
}


void Profile::clear()
{
  profile_source="";
  profile_section.clear();
}

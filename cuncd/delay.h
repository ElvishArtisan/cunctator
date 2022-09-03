// delay.h
//
// Abstract base class for Cunctator delay devices.
//
//   (C) Copyright 2011-2022 Fred Gleason <fredg@paravelsystems.com>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License version 2 as
//   published by the Free Software Foundation.
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

#ifndef DELAY_H
#define DELAY_H

#include <qobject.h>

#include <profile.h>
#include <cunc.h>

class Delay : public QObject
{
 Q_OBJECT
 public:
  Delay(Profile *p,int id,bool debug,QObject *parent=0);
  ~Delay();
  int id() const;
  bool debugMode() const;
  virtual Cunctator::DelayType type()=0;
  virtual QString description()=0;
  virtual Cunctator::DelayState state()=0;
  virtual int delayLength()=0;
  virtual bool connect()=0;
  virtual void bypass()=0;
  virtual void enter()=0;
  virtual void exit()=0;
  virtual void dump()=0;

 signals:
  void delayStateChanged(int id,Cunctator::DelayState state,int len);
  void dumped(int id);

 private:
  int delay_id;
  bool delay_debug;
};


#endif  // DELAY_H

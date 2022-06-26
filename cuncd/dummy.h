// dummy.h
//
// Dummy delay driver for Cunctator
//
//   (C) Copyright 2011 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef DUMMY_H
#define DUMMY_H

#include <qtimer.h>

#include "delay.h"

#define DUMMY_MAX_DELAY 10000
#define DUMMY_DELAY_INCREMENT 500

class Dummy : public Delay
{
 Q_OBJECT
 public:
  Dummy(Profile *p,int n,bool debug,QObject *parent,const char *name=0);
  ~Dummy();
  Cunctator::DelayType type();
  QString description();
  Cunctator::DelayState state();
  int delayLength();
  bool connect();
  void bypass();
  void enter();
  void exit();
  void dump();

 private slots:
  void buildTimerData();

 private:
  Cunctator::DelayState dummy_state;
  int dummy_delay_length;
  QTimer *dummy_build_timer;
};


#endif  // DUMMY_H

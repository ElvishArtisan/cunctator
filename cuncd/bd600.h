// bd600.h
//
// Cunctator delay driver for the Eventide BD600
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

#ifndef BD600_H
#define BD600_H

//#include <qsocketnotifier.h>
#include <QTimer>

#include <ttydevice.h>

#include "delay.h"

#define BD600_WATCHDOG_INCREMENT 1000

class Bd600 : public Delay
{
 Q_OBJECT
 public:
  Bd600(Profile *p,int n,bool debug,QObject *parent);
  ~Bd600();
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
  void readyReadData();
  void watchdogData();

 private:
  void ProcessCommand(const QString &cmd);
  Cunctator::DelayState bd600_state;
  int bd600_delay_length;
  TTYDevice *bd600_tty;
  QTimer *bd600_watchdog_timer;
  QString bd600_accum;
};


#endif  // BD600_H

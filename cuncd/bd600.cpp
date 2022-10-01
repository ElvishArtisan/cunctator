// bd600.cpp
//
// Bd600 delay driver for Cunctator
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

#include <qtimer.h>

#include "bd600.h"

Bd600::Bd600(Profile *p,int n,bool debug,QObject *parent)
  : Delay(p,n,debug,parent)
{
  bd600_state=Cunctator::StateBypassed;
  bd600_delay_length=0;

  //
  // Open the TTY
  //
  QString section=QString::asprintf("Delay%u",n+1);
  bd600_tty=new TTYDevice();
  bd600_tty->setName(p->stringValue(section,"TtyDevice","/dev/ttyS0"));
  bd600_tty->setSpeed(p->intValue(section,"TtySpeed",9600));

  //
  // Watchdog
  //
  bd600_watchdog_timer=new QTimer(this);
  bd600_watchdog_timer->setSingleShot(true);
  QObject::connect(bd600_watchdog_timer,SIGNAL(timeout()),
		   this,SLOT(watchdogData()));
  bd600_watchdog_timer->start(BD600_WATCHDOG_INCREMENT);
}


Bd600::~Bd600()
{
}


Cunctator::DelayType Bd600::type()
{
  return Cunctator::TypeBd600;
}


QString Bd600::description()
{
  return QString("Bd600 Delay");
}


Cunctator::DelayState Bd600::state()
{
  return bd600_state;
}


int Bd600::delayLength()
{
  return 10*bd600_delay_length;
}


bool Bd600::connect()
{
  if(!bd600_tty->open(QIODevice::ReadWrite)) {
    return false;
  }
  QObject::connect(bd600_tty,SIGNAL(readyRead()),this,SLOT(readyReadData()));
  bd600_tty->write("T",1);
  return true;
}


void Bd600::bypass()
{
  bd600_tty->write("B",1);
}


void Bd600::enter()
{
  bd600_tty->write("R",1);
}


void Bd600::exit()
{
  bd600_tty->write("Z",1);
}


void Bd600::dump()
{
  bd600_tty->write("D",1);
}


void Bd600::readyReadData()
{
  QString data;

  bd600_watchdog_timer->stop();
  data=QString(bd600_tty->readAll());
  for(int i=0;i<data.length();i++) {
    char c=data.at(i).toLatin1();
    switch(c) {
    case '>':
      ProcessCommand(bd600_accum);
      bd600_accum.clear();
      break;

    case '\n':
    case '\r':
      break;

    default:
      bd600_accum+=data.at(i);
      break;
    }
  }
}


void Bd600::watchdogData()
{
  if(bd600_state!=Cunctator::StateBypassed) {
    bd600_state=Cunctator::StateBypassed;
    bd600_delay_length=0;
    emit delayStateChanged(id(),bd600_state,delayLength());
  }
  bd600_tty->close();
  if(connect()) {
    bd600_tty->write("M",1);
  }
  bd600_watchdog_timer->start(BD600_WATCHDOG_INCREMENT);
}


void Bd600::ProcessCommand(const QString &cmd)
{
  int delay_length;
  Cunctator::DelayState state=bd600_state;

  switch(cmd.length()) {
  case 4:   // Delay length
    delay_length=cmd.left(4).toInt();
    if(delay_length!=bd600_delay_length) {
      if(delay_length<(bd600_delay_length-100)) {
	emit dumped(id());
      }
      bd600_delay_length=delay_length;
      emit delayStateChanged(id(),bd600_state,delayLength());
    }
    bd600_tty->write("M",1);
    break;

  case 2:   // Operating mode
    switch(cmd.at(0).toLatin1()) {
    case 'B':  // Bypass
      state=Cunctator::StateBypassed;
      break;

    case 'H':  // Static
      state=Cunctator::StateEntered;
      break;

    case 'I':  // Panic
      break;

    case 'L':  // Live
      state=Cunctator::StateExited;
      break;

    case 'M':  // Mute
      break;

    case 'P':  // Micro precision delay
      break;

    case 'R':  // Rebuild
    case 'W':  // Wait for safe
      state=Cunctator::StateEntering;
      break;

    case 'S':  // Sneeze
      break;
    case 'Z':  // Ramp to zero
    case 'A':  // Wait and exit
      state=Cunctator::StateExiting;
      break;
    }
    if(state!=bd600_state) {
      bd600_state=state;
      emit delayStateChanged(id(),bd600_state,delayLength());
    }
    bd600_tty->write("T",1);
    break;

  default:
    bd600_tty->write("M",1);
    break;
  }
  bd600_watchdog_timer->start(BD600_WATCHDOG_INCREMENT);
}

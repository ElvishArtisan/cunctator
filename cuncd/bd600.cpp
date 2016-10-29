// bd600.cpp
//
// Bd600 delay driver for Cunctator
//
//   (C) Copyright 2011 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: bd600.cpp,v 1.2 2011/08/16 22:26:19 cvs Exp $
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

Bd600::Bd600(Profile *p,int n,bool debug,QObject *parent,const char *name)
  : Delay(p,n,debug,parent,name)
{
  bd600_state=Cunctator::StateBypassed;
  bd600_delay_length=0;

  //
  // Open the TTY
  //
  QString section=QString().sprintf("Delay%u",n+1);
  bd600_tty=new TTYDevice();
  bd600_tty->setName(p->stringValue(section,"TtyDevice","/dev/ttyS0"));
  bd600_tty->setSpeed(p->intValue(section,"TtySpeed",9600));

  //
  // Watchdog
  //
  bd600_watchdog_timer=new QTimer(this);
  QObject::connect(bd600_watchdog_timer,SIGNAL(timeout()),
		   this,SLOT(watchdogData()));
  bd600_watchdog_timer->start(BD600_WATCHDOG_INCREMENT,true);
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
  if(!bd600_tty->open(IO_ReadWrite)) {
    return false;
  }
  bd600_notify=
    new QSocketNotifier(bd600_tty->socket(),QSocketNotifier::Read,this);
  QObject::connect(bd600_notify,SIGNAL(activated(int)),
		   this,SLOT(readyReadData(int)));

  bd600_tty->writeBlock("T",1);
  return true;
}


void Bd600::bypass()
{
  bd600_tty->writeBlock("B",1);
}


void Bd600::enter()
{
  bd600_tty->writeBlock("R",1);
}


void Bd600::exit()
{
  bd600_tty->writeBlock("Z",1);
}


void Bd600::dump()
{
  bd600_tty->writeBlock("D",1);
}


void Bd600::readyReadData(int fd)
{
  char data[1500];
  int n;
  int delay_length=0;
  Cunctator::DelayState state=bd600_state;

  bd600_watchdog_timer->stop();
  while((n=bd600_tty->readBlock(data,1500))>0) {
    data[n]=0;
    switch(n) {
    case 5:   // Delay length
      delay_length=QString(data).left(4).toInt();
      if(delay_length!=bd600_delay_length) {
	if(delay_length<(bd600_delay_length-100)) {
	  emit dumped(id());
	}
	bd600_delay_length=delay_length;
	emit delayStateChanged(id(),bd600_state,delayLength());
      }
      bd600_tty->writeBlock("M",1);
      break;

    case 3:   // Operating mode
      switch(data[0]) {
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
      bd600_tty->writeBlock("T",1);
      break;

    default:
      bd600_tty->writeBlock("M",1);
      break;
    }
    data[n]=0;
  }
  bd600_watchdog_timer->start(BD600_WATCHDOG_INCREMENT,true);
}


void Bd600::watchdogData()
{
  if(bd600_state!=Cunctator::StateBypassed) {
    bd600_state=Cunctator::StateBypassed;
    bd600_delay_length=0;
    emit delayStateChanged(id(),bd600_state,delayLength());
  }
  bd600_tty->writeBlock("M",1);
  bd600_watchdog_timer->start(BD600_WATCHDOG_INCREMENT,true);
}

// dummy.cpp
//
// Dummy delay driver for Cunctator
//
//   (C) Copyright 2011 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: dummy.cpp,v 1.5 2011/08/16 22:26:19 cvs Exp $
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

#include "dummy.h"

Dummy::Dummy(Profile *p,int n,bool debug,QObject *parent,const char *name)
  : Delay(p,n,debug,parent,name)
{
  dummy_state=Cunctator::StateBypassed;
  dummy_delay_length=0;

  dummy_build_timer=new QTimer(this);
  QObject::connect(dummy_build_timer,SIGNAL(timeout()),
		   this,SLOT(buildTimerData()));
}


Dummy::~Dummy()
{
}


Cunctator::DelayType Dummy::type()
{
  return Cunctator::TypeDummy;
}


QString Dummy::description()
{
  return QString("Dummy Delay");
}


Cunctator::DelayState Dummy::state()
{
  return dummy_state;
}


int Dummy::delayLength()
{
  return dummy_delay_length;
}


bool Dummy::connect()
{
  printf("Dummy::connect()\n");
  return true;
}


void Dummy::bypass()
{
  printf("Dummy::bypass()\n");

  if(dummy_state!=Cunctator::StateBypassed) {
    dummy_state=Cunctator::StateBypassed;
    dummy_delay_length=0;
    emit delayStateChanged(id(),dummy_state,dummy_delay_length);
  }
}


void Dummy::enter()
{
  printf("Dummy::enter()\n");

  if(dummy_state!=Cunctator::StateEntering) {
    dummy_state=Cunctator::StateEntering;
    dummy_build_timer->start(500,true);
    emit delayStateChanged(id(),dummy_state,dummy_delay_length);
  }
}


void Dummy::exit()
{
  printf("Dummy::exit()\n");

  if((dummy_state!=Cunctator::StateExiting)&&(dummy_delay_length>0)) {
    dummy_state=Cunctator::StateExiting;
    dummy_build_timer->start(500,true);
    emit delayStateChanged(id(),dummy_state,dummy_delay_length);
  }
}


void Dummy::dump()
{
  printf("Dummy::dump()\n");
  if(dummy_delay_length>0) {
    dummy_delay_length=0;
    emit dumped(id());
    emit delayStateChanged(id(),dummy_state,dummy_delay_length);
  }
  switch(dummy_state) {
  case Cunctator::StateEntering:
  case Cunctator::StateEntered:
    dummy_build_timer->start(500,true);
    break;

  case Cunctator::StateUnknown:
  case Cunctator::StateBypassed:
  case Cunctator::StateExited:
  case Cunctator::StateExiting:
    break;
  }
}


void Dummy::buildTimerData()
{
  switch(dummy_state) {
  case Cunctator::StateEntering:
    if(dummy_delay_length>=DUMMY_MAX_DELAY) {
      dummy_state=Cunctator::StateEntered;
    }
    else {
      dummy_delay_length+=DUMMY_DELAY_INCREMENT;
      dummy_build_timer->start(500,true);
    }
    break;

  case Cunctator::StateExiting:
    if(dummy_delay_length<=0) {
      dummy_state=Cunctator::StateExited;
    }
    else {
      dummy_delay_length-=DUMMY_DELAY_INCREMENT;
      dummy_build_timer->start(500,true);
    }
    break;

  default:
    return;
  }
  emit delayStateChanged(id(),dummy_state,dummy_delay_length);
}

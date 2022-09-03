// delay.cpp
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

#include "delay.h"

Delay::Delay(Profile *p,int id,bool debug,QObject *parent)
  : QObject(parent)
{
  delay_id=id;
  delay_debug=debug;
}


Delay::~Delay()
{
}


bool Delay::debugMode() const
{
  return delay_debug;
}


int Delay::id() const
{
  return delay_id;
}


Cunctator::DelayType Delay::type()
{
  return Cunctator::TypeUnknown;
}


QString Delay::description()
{
  return QString("UNKNOWN");
}


Cunctator::DelayState Delay::state()
{
  return Cunctator::StateUnknown;
}


int Delay::delayLength()
{
  return 0;
}


bool Delay::connect()
{
  return false;
}


void Delay::bypass()
{
}


void Delay::enter()
{
}


void Delay::exit()
{
}


void Delay::dump()
{
}

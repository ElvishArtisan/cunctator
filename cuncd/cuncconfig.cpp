// cuncconfig.cpp
//
// A container class for Cunctator Configuration
//
//   (C) Copyright 2002-2025 Fred Gleason <fredg@paravelsystems.com>
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

#include <syslog.h>

#include <qsettings.h>

#include <profile.h>
#include <cunc.h>

#include "cuncconfig.h"
#include "dummy.h"
#include "airtools.h"
#include "asihpidelay.h"
#include "bd600.h"
#include "jackdelay.h"

CuncConfig::CuncConfig(bool debug,QObject *parent)
  : QObject(parent)
{
  clear();
  conf_debug=debug;
}


CuncConfig::CuncConfig(QString filename,bool debug)
{
  clear();
  conf_filename=filename;
  conf_debug=debug;
}


CuncConfig::~CuncConfig()
{
  clear();
}


QString CuncConfig::filename() const
{
  return conf_filename;
}


void CuncConfig::setFilename(QString filename)
{
  conf_filename=filename;
}


unsigned CuncConfig::tcpPort() const
{
  return conf_tcp_port;
}


unsigned CuncConfig::delays() const
{
  return conf_delays.size();
}


Delay *CuncConfig::delay(unsigned n)
{
  return conf_delays[n];
}


RmlEngine *CuncConfig::rmlEngine(unsigned n)
{
  return conf_rml_engines[n];
}


void CuncConfig::dumpConfig(FILE *stream)
{
  fprintf(stream,"[Global]\n");
  fprintf(stream,"TcpPort=%u\n",conf_tcp_port);
  fprintf(stream,"\n");

  for(int i=0;i<conf_delays.size();i++) {
    fprintf(stream,"[Delay%u]\n",i+1);
    switch(conf_delays[i]->type()) {
    case Cunctator::TypeUnknown:
      fprintf(stream,"Type=UNKNOWN\n");
      break;

    case Cunctator::TypeDummy:
      fprintf(stream,"Type=Dummy\n");
      break;

    case Cunctator::TypeAirTools:
      fprintf(stream,"Type=AirTools\n");
      break;

    case Cunctator::TypeBd600:
      fprintf(stream,"Type=BD600\n");
      break;

    case Cunctator::TypeJackDelay:
      fprintf(stream,"Type=JackDelay\n");
      break;

    case Cunctator::TypeAsihpiDelay:
      fprintf(stream,"Type=AsihpiDelay\n");
      break;
    }
  }
}


bool CuncConfig::load()
{
  QString section;
  unsigned count=1;
  bool ok;
  QString str;
  bool added;
  int num;

  //
  // Host Name
  //
  Profile *p=new Profile();
  if(!p->setSource(conf_filename)) {
    delete p;
    return false;
  }

  //
  // [Global] Section
  //
  conf_tcp_port=p->intValue("Global","TcpPort",CUNC_TCP_PORT);

  //
  // Delays
  //
  section=QString::asprintf("Delay%u",count);
  str=p->stringValue(section,"Type","Dummy",&ok);
  while(ok) {
    added=false;
    if(str.toLower()=="dummy") {
      conf_delays.push_back(new Dummy(p,count-1,conf_debug,this));
      added=true;
    }
    if(str.toLower()=="airtools") {
      conf_delays.push_back(new AirTools(p,count-1,conf_debug,this));
      added=true;
    }
    if(str.toLower()=="bd600") {
      conf_delays.push_back(new Bd600(p,count-1,conf_debug,this));
      added=true;
    }
    if(str.toLower()=="jackdelay") {
      conf_delays.push_back(new JackDelay(p,count-1,conf_debug,this));
      added=true;
    }
    if(str.toLower()=="asihpidelay") {
      conf_delays.push_back(new AsihpiDelay(p,count-1,conf_debug,this));
      added=true;
    }
    if(added) {
      connect(conf_delays.back(),
	      SIGNAL(delayStateChanged(int,Cunctator::DelayState,int)),
	      this,SLOT(delayStateChangedData(int,Cunctator::DelayState,int)));
      connect(conf_delays.back(),SIGNAL(dumped(int)),
	      this,SLOT(delayDumpedData(int)));

      conf_rml_engines.push_back(new RmlEngine(p,count,this));
      connect(conf_delays.back(),
	      SIGNAL(delayStateChanged(int,Cunctator::DelayState,int)),
	      conf_rml_engines.back(),
	      SLOT(sendDelayState(int,Cunctator::DelayState,int)));
      connect(conf_delays.back(),SIGNAL(dumped(int)),
	      conf_rml_engines.back(),SLOT(sendDelayDumped(int)));
    }
    section=QString::asprintf("Delay%u",++count);
    str=p->stringValue(section,"Type","Dummy",&ok);
  }

  //
  // UDP Queues
  //
  count=1;
  section=QString::asprintf("UdpQueue%u",count);
  num=p->intValue(section,"DelayNumber",0,&ok)-1;
  while(ok) {
    if(num<conf_delays.size()) {
      conf_udp_queues.push_back(new UdpQueue(p,count-1,this));
      connect(conf_delays[num],
	      SIGNAL(delayStateChanged(int,Cunctator::DelayState,int)),
	      conf_udp_queues.back(),
	      SLOT(changeDelayState(int,Cunctator::DelayState,int)));
      connect(conf_delays[num],SIGNAL(dumped(int)),
	      conf_udp_queues.back(),
	      SLOT(delayDumped(int)));
    }
    else {
      syslog(LOG_WARNING,
	     "DelayNumber entry is missing/invalid in [UdpQueue%u]",count);
      exit(1);
    }
    section=QString::asprintf("UdpQueue%u",++count);
    num=p->intValue(section,"DelayNumber",0,&ok)-1;
  }

  delete p;
  return true;
}


void CuncConfig::clear()
{
  conf_tcp_port=CUNC_TCP_PORT;
  for(int i=0;i<conf_delays.size();i++) {
    delete conf_delays[i];
  }
  conf_delays.clear();
  for(int i=0;i<conf_udp_queues.size();i++) {
    delete conf_udp_queues[i];
  }
  conf_udp_queues.clear();
  conf_debug=false;
}


void CuncConfig::requestDelayModel(int id,int n)
{
  if(n<(int)conf_delays.size()) {
    emit delayModelSent(id,n,conf_delays[n]->type(),
			conf_delays[n]->description());

  }
}


void CuncConfig::requestDelayState(int id,int n)
{
  if(n<(int)conf_delays.size()) {
    emit delayStateSent(id,n,conf_delays[n]->state(),
			conf_delays[n]->delayLength());
  }
}


void CuncConfig::requestDelayStateChange(int n,Cunctator::DelayState state)
{
  if(n<(int)conf_delays.size()) {
    switch(state) {
    case Cunctator::StateBypassed:
      conf_delays[n]->bypass();
      break;
      
    case Cunctator::StateEntering:
      conf_delays[n]->enter();
      break;
      
    case Cunctator::StateExiting:
      conf_delays[n]->exit();
      break;
      
    case Cunctator::StateEntered:
    case Cunctator::StateExited:
    case Cunctator::StateUnknown:
      break;
    }
  }
}


void CuncConfig::requestDelayDump(int n)
{
  if(n<(int)conf_delays.size()) {
    conf_delays[n]->dump();
  }
}


void CuncConfig::delayStateChangedData(int n,Cunctator::DelayState state,int len)
{
  emit delayStateSent(-1,n,state,len);
}


void CuncConfig::delayDumpedData(int n)
{
  emit delayDumped(n);
}

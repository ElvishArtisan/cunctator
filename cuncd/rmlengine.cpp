// rmlengine.cpp
//
// An RML control engine
//
//   (C) Copyright 2011 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: rmlengine.cpp,v 1.2 2011/08/05 20:59:52 cvs Exp $
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

#include "rmlengine.h"

RmlEngine::RmlEngine(Profile *p, int id,QObject *parent,const char *name)
  : QObject(parent,name)
{
  rml_delay_state=Cunctator::StateUnknown;
  rml_delay_length=-1;

  //
  // Load Configuration
  //
  LoadStack(p,id,"RmlOnBypassed",&rml_on_bypassed_addresses,
	    &rml_on_bypassed_ports,
	    &rml_on_bypassed_commands);
  LoadStack(p,id,"RmlOnEntering",&rml_on_entering_addresses,
	    &rml_on_entering_ports,
	    &rml_on_entering_commands);
  LoadStack(p,id,"RmlOnEntered",&rml_on_entered_addresses,
	    &rml_on_entered_ports,
	    &rml_on_entered_commands);
  LoadStack(p,id,"RmlOnExiting",&rml_on_exiting_addresses,
	    &rml_on_exiting_ports,
	    &rml_on_exiting_commands);
  LoadStack(p,id,"RmlOnExited",&rml_on_exited_addresses,
	    &rml_on_exited_ports,
	    &rml_on_exited_commands);
  LoadStack(p,id,"RmlOnDump",&rml_on_dump_addresses,&rml_on_dump_ports,
	    &rml_on_dump_commands);
  LoadStack(p,id,"RmlOnDelayChange",&rml_on_delay_change_addresses,
	    &rml_on_delay_change_ports,
	    &rml_on_delay_change_commands);

  //
  // Send Socket
  //
  rml_socket=new QSocketDevice(QSocketDevice::Datagram);
  rml_socket->setBlocking(false);
}


RmlEngine::~RmlEngine()
{
  delete rml_socket;
}


void RmlEngine::sendDelayLength()
{
  QString cmd;

  for(unsigned i=0;i<rml_on_delay_change_addresses.size();i++) {
    cmd=ResolveCommand(rml_on_delay_change_commands[i]);
    rml_socket->writeBlock(cmd,cmd.length(),
			   rml_on_delay_change_addresses[i],
			   rml_on_delay_change_ports[i]);
  }
}


void RmlEngine::sendDelayState(int n,Cunctator::DelayState state,int len)
{
  QString cmd;

  if(len!=rml_delay_length) {
    rml_delay_length=len;
    for(unsigned i=0;i<rml_on_delay_change_addresses.size();i++) {
      cmd=ResolveCommand(rml_on_delay_change_commands[i]);
      rml_socket->writeBlock(cmd,cmd.length(),
			     rml_on_delay_change_addresses[i],
			     rml_on_delay_change_ports[i]);
    }
  }
  if((state==Cunctator::StateBypassed)&&(state!=rml_delay_state)) {
    for(unsigned i=0;i<rml_on_bypassed_addresses.size();i++) {
      cmd=ResolveCommand(rml_on_bypassed_commands[i]);
      rml_socket->writeBlock(cmd,cmd.length(),
			     rml_on_bypassed_addresses[i],
			     rml_on_bypassed_ports[i]);
    }
  }
  if((state==Cunctator::StateEntering)&&(state!=rml_delay_state)) {
    for(unsigned i=0;i<rml_on_entering_addresses.size();i++) {
      cmd=ResolveCommand(rml_on_entering_commands[i]);
      rml_socket->writeBlock(cmd,cmd.length(),
			     rml_on_entering_addresses[i],
			     rml_on_entering_ports[i]);
    }
  }
  if((state==Cunctator::StateEntered)&&(state!=rml_delay_state)) {
    for(unsigned i=0;i<rml_on_entering_addresses.size();i++) {
      cmd=ResolveCommand(rml_on_entered_commands[i]);
      rml_socket->writeBlock(cmd,cmd.length(),
			     rml_on_entered_addresses[i],
			     rml_on_entered_ports[i]);
    }
  }
  if((state==Cunctator::StateExiting)&&(state!=rml_delay_state)) {
    for(unsigned i=0;i<rml_on_exiting_addresses.size();i++) {
      cmd=ResolveCommand(rml_on_exiting_commands[i]);
      rml_socket->writeBlock(cmd,cmd.length(),
			     rml_on_exiting_addresses[i],
			     rml_on_exiting_ports[i]);
    }
  }
  if((state==Cunctator::StateExited)&&(state!=rml_delay_state)) {
    for(unsigned i=0;i<rml_on_exited_addresses.size();i++) {
      cmd=ResolveCommand(rml_on_exited_commands[i]);
      rml_socket->writeBlock(cmd,cmd.length(),
			     rml_on_exited_addresses[i],
			     rml_on_exited_ports[i]);
    }
  }
  rml_delay_state=state;
}


void RmlEngine::sendDelayDumped(int n)
{
  QString cmd;

  for(unsigned i=0;i<rml_on_dump_addresses.size();i++) {
    cmd=ResolveCommand(rml_on_dump_commands[i]);
    rml_socket->writeBlock(cmd,cmd.length(),
			   rml_on_dump_addresses[i],
			   rml_on_dump_ports[i]);
  }
}


QString RmlEngine::ResolveCommand(const QString &cmd) const
{
  QString ret=cmd;
  ret.replace("%d",QString().sprintf("%4.1f",(float)rml_delay_length/1000.0));
  return ret;
}


void RmlEngine::LoadStack(Profile *p,int id,const QString &tag,
			  std::vector<QHostAddress> *addrs,
			  std::vector<Q_UINT16> *ports,
			  std::vector<QString> *cmds)
{
  QHostAddress addr;
  int count=1;
  QString section=QString().sprintf("Delay%d",id);
  QString label=tag+QString().sprintf("Address%d",count);

  addr.setAddress(p->stringValue(section,label,"0.0.0.0"));
  while(!addr.isNull()) {
    addrs->push_back(addr);
    cmds->push_back(p->stringValue(section,
				   tag+QString().sprintf("Command%d",count)));
    ports->push_back(p->intValue(section,
				 tag+QString().sprintf("Port%d",count)));
    label=tag+QString().sprintf("Address%d",++count);
    addr.setAddress(p->stringValue(section,label,"0.0.0.0"));
  }
}

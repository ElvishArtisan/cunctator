// connection.cpp
//
// A container class for cuncd(8) connections.
//
//   (C) Copyright 2011-2025 Fred Gleason <fredg@paravelsystems.com>
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

#include <ctype.h>

#include <qstringlist.h>

#include "connection.h"

Connection::Connection(QTcpSocket *sock,QObject *parent)
  : QObject(parent)
{
  conn_socket=sock;
  connect(conn_socket,SIGNAL(readyRead()),this,SLOT(readyReadData()));
}


Connection::~Connection()
{
  delete conn_socket;
}


int Connection::id() const
{
  return conn_socket->socketDescriptor();
}


bool Connection::isZombie() const
{
  return conn_socket->state()!=QAbstractSocket::ConnectedState;
}


void Connection::sendDelayQuantity(int n) const
{
  QString msg=QString::asprintf("DQ %d!",n);
  conn_socket->write(msg.toUtf8(),msg.toUtf8().length());
}


void Connection::sendDelayModel(int id,int n,Cunctator::DelayType type,
				const QString &desc)
{
  if((id>=0)&&(id!=conn_socket->socketDescriptor())) {
    return;
  }
  QString msg=QString::asprintf("DM %d %d %s!",n,type,desc.toUtf8().constData());
  conn_socket->write(msg.toUtf8(),msg.toUtf8().length());
}


void Connection::sendDelayState(int id,int n,Cunctator::DelayState state,int len)
{
  if((id>=0)&&(id!=conn_socket->socketDescriptor())) {
    return;
  }
  QString msg=QString::asprintf("DS %d %d %d!",n,state,len);
  conn_socket->write(msg.toUtf8(),msg.toUtf8().length());
}


void Connection::sendDelayDumped(int n)
{
  QString msg=QString::asprintf("DP %d!",n);
  conn_socket->write(msg.toUtf8(),msg.toUtf8().length());
}


void Connection::readyReadData()
{
  char data[1500];
  int n;

  while((conn_socket->state()==QAbstractSocket::ConnectedState)&&
	((n=conn_socket->read(data,1500))>0)) {
    for(int i=0;i<n;i++) {
      if(isprint(data[i])) {
	if(data[i]=='!') {
	  ProcessCommand(conn_buffer);
	  conn_buffer="";
	}
	else {
	  conn_buffer+=data[i];
	}
      }
    }
  }
}


void Connection::ProcessCommand(const QString &cmd)
{
  QStringList args=cmd.split(" ",Qt::SkipEmptyParts);
  if(args.size()<1) {
    return;
  }

  if(args[0]=="DC") {  // Drop Connection
    conn_socket->close();
  }

  if(args[0]=="DQ") {  // Delay Quantity
    emit delayQuantityRequested(conn_socket->socketDescriptor());
  }

  if(args[0]=="DM") {  // Delay Model
    if(args.size()==2) {
      emit delayModelRequested(conn_socket->socketDescriptor(),args[1].toInt());
    }
  }

  if(args[0]=="DS") {  // Delay State
    if(args.size()==2) {
      emit delayStateRequested(conn_socket->socketDescriptor(),args[1].toInt());
    }
  }

  if(args[0]=="SS") {  // Change Delay State
    if(args.size()==3) {
      emit delayStateChangeRequested(args[1].toInt(),
				     (Cunctator::DelayState)args[2].toInt());
    }
  }

  if(args[0]=="DP") {  // Dump Delay
    if(args.size()==2) {
      emit delayDumpRequested(args[1].toInt());
    }
  }
}

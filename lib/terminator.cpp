// terminator.cpp
//
// Generate a Qt signal before terminating in response to a SIGINT OR SIGTERM
//
// (C) Copyright 2025 Fred Gleason <fredg@paravelsystems.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of version 2.1 of the GNU Lesser General Public
//    License as published by the Free Software Foundation;
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, 
//    Boston, MA  02111-1307  USA
//
// EXEMPLAR_VERSION: 2.0.2
//

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "terminator.h"

int __terminator_socket;
void __Terminator_Callback(int signum)
{
  char data[1]={1};
  send(__terminator_socket,data,1,0);
}


Terminator::Terminator(QObject *parent)
{
  int socks[2];

  if(socketpair(AF_UNIX,SOCK_STREAM,0,socks)!=0) {
    fprintf(stderr,"Terminator::failed to create socket pair [%s]\n",
	    strerror(errno));
    ::exit(1);
  }
  __terminator_socket=socks[0];
  d_socket=new QLocalSocket(this);
  connect(d_socket,SIGNAL(readyRead()),this,SLOT(readyReadData()));
  d_socket->setSocketDescriptor(socks[1]);

  ::signal(SIGINT,__Terminator_Callback);
  ::signal(SIGTERM,__Terminator_Callback);
}


Terminator::~Terminator()
{
  ::signal(SIGINT,SIG_DFL);
  ::signal(SIGTERM,SIG_DFL);
  delete d_socket;
}


void Terminator::readyReadData()
{
  d_socket->readAll();
  emit terminating();
}

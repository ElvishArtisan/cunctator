// cuncd.cpp
//
// Cunctator delay control daemon.
//
//   (C) Copyright 2011 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: cuncd.cpp,v 1.13 2012/07/27 17:16:50 cvs Exp $
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <syslog.h>

#include <qapplication.h>
#include <qtimer.h>
#include <qsocketnotifier.h>
#include <qstringlist.h>

#include <cmdswitch.h>

#include <cunc.h>

#include "globals.h"
#include "cuncd.h"

void SigHandler(int signo)
{
  pid_t pLocalPid;

  switch(signo) {
  case SIGCHLD:
    pLocalPid=waitpid(-1,NULL,WNOHANG);
    while(pLocalPid>0) {
      pLocalPid=waitpid(-1,NULL,WNOHANG);
    }
    ::signal(SIGCHLD,SigHandler);
    ::signal(SIGTERM,SigHandler);
    ::signal(SIGINT,SigHandler);
    return;

  case SIGTERM:
  case SIGINT:
    unlink(CUNCD_PID_FILE);
    exit(0);
  }
}


MainObject::MainObject(QObject *parent,const char *name)
  :QObject(parent,name)
{
  debug=false;
  FILE *f=NULL;

  //
  // Read Command Options
  //
  CmdSwitch *cmd=
    new CmdSwitch(qApp->argc(),qApp->argv(),"cuncd",CUNCD_USAGE);
  for(unsigned i=0;i<cmd->keys();i++) {
    if(cmd->key(i)=="-d") {
      debug=true;
      cmd->setProcessed(i,true);
    }
    if(!cmd->processed(i)) {
      fprintf(stderr,"cuncd: unrecognized option \"%s\"\n",
	      (const char *)cmd->key(i));
      exit(256);
    }
  }
  delete cmd;
  if(debug) {
    openlog("cuncd",LOG_PERROR,LOG_DAEMON);
  }
  else {
    openlog("cuncd",0,LOG_DAEMON);
  }

  //
  // Load Local Configs
  //
  cuncd_config=new CuncConfig(CUNC_CONF_FILE,debug);
  cuncd_config->load();
  //cuncd_config->dumpConfig(stdout);

  //
  // Detach
  //
  if(!debug) {
    daemon(0,0);
  }

  //
  // Configure Signals
  //
  ::signal(SIGCHLD,SigHandler);
  ::signal(SIGTERM,SigHandler);
  ::signal(SIGINT,SigHandler);

  //
  // Write PID File
  //
  if((f=fopen(CUNCD_PID_FILE,"w"))!=NULL) {
    fprintf(f,"%d",getpid());
    fclose(f);
  }

  //
  // Load Delay Drivers
  //
  for(unsigned i=0;i<cuncd_config->delays();i++) {
    if(!cuncd_config->delay(i)->connect()) {
      syslog(LOG_ERR,"unable to connect to \"%s\" in configuration [Delay%u]",
	     (const char *)cuncd_config->delay(i)->description(),i+1);
      exit(256);
    }
  }

  //
  // Initialize Front-End
  //
  cuncd_server=new CuncSocket(cuncd_config->tcpPort(),0,this);
  connect(cuncd_server,SIGNAL(connection(int)),
	  this,SLOT(newConnectionData(int)));
  QTimer *timer=new QTimer(this);
  connect(timer,SIGNAL(timeout()),this,SLOT(garbageCollectionData()));
  timer->start(1000);

  //
  // Initialize RML Interface
  //
  cuncd_rml_socket=new QSocketDevice(QSocketDevice::Datagram);
  cuncd_rml_socket->setBlocking(false);
  cuncd_rml_socket->bind(QHostAddress(),CUNC_TCP_PORT);
  QSocketNotifier *notify=
    new QSocketNotifier(cuncd_rml_socket->socket(),QSocketNotifier::Read,this);
  connect(notify,SIGNAL(activated(int)),this,SLOT(rmlReceivedData(int)));
}


MainObject::~MainObject()
{
  delete cuncd_config;
  delete cuncd_server;
}


void MainObject::newConnectionData(int fd)
{
  cuncd_connections.push_back(new Connection(fd,this));
  Connection *conn=cuncd_connections.back();
  connect(conn,SIGNAL(delayQuantityRequested(int)),
	  this,SLOT(delayQuantityRequestedData(int)));
  connect(conn,SIGNAL(delayModelRequested(int,int)),
	  cuncd_config,SLOT(requestDelayModel(int,int)));
  connect(cuncd_config,SIGNAL(delayModelSent(int,int,Cunctator::DelayType,
					     const QString &)),
	  conn,SLOT(sendDelayModel(int,int,Cunctator::DelayType,
				   const QString &)));
  connect(conn,SIGNAL(delayStateRequested(int,int)),
	  cuncd_config,SLOT(requestDelayState(int,int)));
  connect(cuncd_config,SIGNAL(delayStateSent(int,int,Cunctator::DelayState,int)),
	  conn,SLOT(sendDelayState(int,int,Cunctator::DelayState,int)));
  connect(conn,SIGNAL(delayStateChangeRequested(int,Cunctator::DelayState)),
	  cuncd_config,SLOT(requestDelayStateChange(int,Cunctator::DelayState)));
  connect(conn,SIGNAL(delayDumpRequested(int)),
	  cuncd_config,SLOT(requestDelayDump(int)));
  connect(cuncd_config,SIGNAL(delayDumped(int)),
	  conn,SLOT(sendDelayDumped(int)));
}


void MainObject::garbageCollectionData()
{
  for(int i=(int)cuncd_connections.size()-1;i>=0;i--) {
    if(cuncd_connections[i]->isZombie()) {
      delete cuncd_connections[i];
      cuncd_connections.erase(cuncd_connections.begin()+i);
    }
  }
}


void MainObject::delayQuantityRequestedData(int id)
{
  Connection *conn;
  if((conn=GetConnection(id))!=NULL) {
    conn->sendDelayQuantity(cuncd_config->delays());
  }
}


void MainObject::rmlReceivedData(int fd)
{
  int n;
  char data[1500];

  while((n=cuncd_rml_socket->readBlock(data,1500))>0) {
    int next=0;
    for(int i=0;i<n;i++) {
      if(data[i]=='!') {
	data[i]=0;
	ProcessRml(data+next);
	next=i+1;
      }
    }
  }
}


Connection *MainObject::GetConnection(int id) const
{
  for(unsigned i=0;i<cuncd_connections.size();i++) {
    if(cuncd_connections[i]->id()==id) {
      return cuncd_connections[i];
    }
  }
  syslog(LOG_WARNING,"unknown connection requested, id=%d",id);
  return NULL;
}


void MainObject::ProcessRml(const QString &cmd)
{
  int matrix;
  int line=0;
  bool ok=false;
  QString str;
  
  QStringList args=args.split(" ",cmd);

  if(args[0]=="GO") {
    if((args.size()<5)||(args.size()>6)) {
      return;
    }
    matrix=args[1].toInt(&ok);
    if((!ok)||(matrix<0)||(matrix>=(int)cuncd_config->delays())) {
      return;
    }
    if(args.size()==5) {
      line=args[2].toInt(&ok);
    }
    if(args.size()==6) {
      line=args[3].toInt(&ok);
    }
    if((!ok)||(line<=0)||(matrix>2)) {
      return;
    }
    switch(line) {
    case 1:   // Start Delay
      cuncd_config->requestDelayStateChange(matrix,Cunctator::StateEntering);
      break;
      
    case 2:   // Exit Delay
      cuncd_config->requestDelayStateChange(matrix,Cunctator::StateExiting);
      break;
      
    case 3:   // Dump Delay
      cuncd_config->requestDelayDump(matrix);
      break;

    case 4:   // Dump Delay
      cuncd_config->rmlEngine(matrix)->sendDelayLength();
      break;
    }
  }
}


int main(int argc,char *argv[])
{
  QApplication a(argc,argv,false);
  new MainObject(NULL,"main");
  return a.exec();
}

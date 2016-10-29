// cuncd.h
//
// Cunctator delay control daemon.
//
//   (C) Copyright 2011 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: cuncd.h,v 1.7 2011/03/18 00:56:05 cvs Exp $
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

#ifndef CUNCD_H
#define CUNCD_H

#include <vector>

#include <qsocketdevice.h>
#include <qobject.h>

#include "delay.h"
#include "cuncconfig.h"
#include "connection.h"
#include "cuncsocket.h"

#define CUNCD_USAGE "[-d]\n\n-d\n     Stay in the foreground and print debugging messages."
#define CUNCD_PID_FILE "/var/run/cuncd.pid"

class MainObject : public QObject
{
  Q_OBJECT
 public:
  MainObject(QObject *parent=0,const char *name=0);
  ~MainObject();

 private slots:
  void newConnectionData(int fd);
  void garbageCollectionData();
  void delayQuantityRequestedData(int id);
  void rmlReceivedData(int fd);

 private:
  Connection *GetConnection(int id) const;
  void ProcessRml(const QString &cmd);
  CuncConfig *cuncd_config;
  std::vector<Connection *> cuncd_connections;
  CuncSocket *cuncd_server;
  QSocketDevice *cuncd_rml_socket;
  bool debug;
};


#endif  // CUNCD_H

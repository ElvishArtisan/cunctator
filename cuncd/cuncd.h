// cuncd.h
//
// Cunctator delay control daemon.
//
//   (C) Copyright 2011 Fred Gleason <fredg@paravelsystems.com>
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

#include <QObject>
#include <QTcpServer>
#include <QUdpSocket>

#include "delay.h"
#include "cuncconfig.h"
#include "connection.h"

#define CUNCD_USAGE "[-d]\n\n-d\n     Stay in the foreground and print debugging messages."
#define CUNCD_PID_FILE "/var/run/cuncd.pid"

class MainObject : public QObject
{
  Q_OBJECT
 public:
  MainObject(QObject *parent=0);
  ~MainObject();

 private slots:
  void newConnectionData();
  void garbageCollectionData();
  void delayQuantityRequestedData(int id);
  void rmlReceivedData();

 private:
  Connection *GetConnection(int id) const;
  void ProcessRml(const QString &cmd);
  CuncConfig *cuncd_config;
  std::vector<Connection *> cuncd_connections;
  QTcpServer *cuncd_server;
  QUdpSocket *cuncd_rml_socket;
  bool debug;
};


#endif  // CUNCD_H

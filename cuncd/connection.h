// connection.h
//
// A container class for cuncd(8) connections.
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

#ifndef CONNECTION_H
#define CONNECTION_H

#include <qobject.h>
#include <qsocket.h>

#include <cuncconfig.h>

class Connection : public QObject
{
  Q_OBJECT;
 public:
  Connection(int fd,QObject *parent=0,const char *name=0);
  ~Connection();
  int id() const;
  bool isZombie() const;
  void sendDelayQuantity(int n) const;

 public slots:
  void sendDelayModel(int id,int n,Cunctator::DelayType type,
		      const QString &desc);
  void sendDelayState(int id,int n,Cunctator::DelayState state,int len);
  void sendDelayDumped(int n);

 signals:
  void delayQuantityRequested(int id);
  void delayModelRequested(int id,int n);
  void delayDescriptionRequested(int id,int n);
  void delayStateRequested(int id,int n);
  void delayStateChangeRequested(int n,Cunctator::DelayState state);
  void delayDumpRequested(int n);

 private slots:
  void readyReadData();

 private:
  void ProcessCommand(const QString &cmd);
  QSocket *conn_socket;
  QString conn_buffer;
};


#endif  // CONNECTION_H

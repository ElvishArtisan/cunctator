// cuncsocket.h
//
// Server socket for cuncd(8)
//
//   (C) Copyright 2011 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: cuncsocket.h,v 1.2 2011/02/18 01:36:50 cvs Exp $
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

#ifndef CUNCSOCKET_H
#define CUNCSOCKET_H

#include <qobject.h>
#include <qstring.h>
#include <qserversocket.h>
#include <qhostaddress.h>

class CuncSocket : public QServerSocket
{
  Q_OBJECT
  public:
   CuncSocket(Q_UINT16 port,int backlog=0,QObject *parent=0,
	       const char *name=0);
   CuncSocket(const QHostAddress &address,Q_UINT16 port,int backlog=0,
	       QObject *parent=0,const char *name=0);
   void newConnection(int socket);

  signals:
   void connection(int);

  private:
   QServerSocket *socket;
};


#endif  // CUNCSOCKET_H

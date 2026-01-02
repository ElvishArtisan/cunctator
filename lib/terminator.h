// terminator.h
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

#ifndef TERMINATOR_H
#define TERMINATOR_H

#include <QLocalSocket>
#include <QObject>

class Terminator : public QObject
{
  Q_OBJECT;
 public:
  Terminator(QObject *parent=NULL);
  ~Terminator();

 signals:
  void terminating();

 private slots:
  void readyReadData();
  
 private:
  QLocalSocket *d_socket;
};


#endif  // TERMINATOR_H

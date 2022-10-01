// rmlengine.h
//
// An RML control engine
//
//   (C) Copyright 2011-2022 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef RMLENGINE_H
#define RMLENGINE_H

#include <stdint.h>

#include <QHostAddress>
#include <QList>
#include <QObject>
#include <QString>
#include <QUdpSocket>

#include <profile.h>
#include <cunc.h>

class RmlEngine : public QObject
{
  Q_OBJECT;
 public:
  RmlEngine(Profile *p, int id,QObject *parent=0);
  ~RmlEngine();
  void sendDelayLength();

 public slots:
  void sendDelayState(int n,Cunctator::DelayState state,int len);
  void sendDelayDumped(int n);

 private:
  QString ResolveCommand(const QString &cmd) const;
  void LoadStack(Profile *p,int id,const QString &tag,
		 QList<QHostAddress> *addrs,
		 QList<uint16_t> *ports,
		 QList<QString> *cmds);

  QList<QHostAddress> rml_on_bypassed_addresses;
  QList<QString> rml_on_bypassed_commands;
  QList<uint16_t> rml_on_bypassed_ports;

  QList<QHostAddress> rml_on_entering_addresses;
  QList<QString> rml_on_entering_commands;
  QList<uint16_t> rml_on_entering_ports;

  QList<QHostAddress> rml_on_entered_addresses;
  QList<QString> rml_on_entered_commands;
  QList<uint16_t> rml_on_entered_ports;

  QList<QHostAddress> rml_on_exiting_addresses;
  QList<QString> rml_on_exiting_commands;
  QList<uint16_t> rml_on_exiting_ports;

  QList<QHostAddress> rml_on_exited_addresses;
  QList<QString> rml_on_exited_commands;
  QList<uint16_t> rml_on_exited_ports;

  QList<QHostAddress> rml_on_dump_addresses;
  QList<QString> rml_on_dump_commands;
  QList<uint16_t> rml_on_dump_ports;

  QList<QHostAddress> rml_on_delay_change_addresses;
  QList<QString> rml_on_delay_change_commands;
  QList<uint16_t> rml_on_delay_change_ports;

  QUdpSocket *rml_socket;
  Cunctator::DelayState rml_delay_state;
  int rml_delay_length;
};


#endif  // RMLENGINE_H

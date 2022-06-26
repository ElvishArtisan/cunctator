// rmlengine.h
//
// An RML control engine
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

#ifndef RMLENGINE_H
#define RMLENGINE_H

#include <vector>

#include <qobject.h>
#include <qhostaddress.h>
#include <qstring.h>
#include <qsocketdevice.h>

#include <profile.h>
#include <cunc.h>

class RmlEngine : public QObject
{
  Q_OBJECT;
 public:
  RmlEngine(Profile *p, int id,QObject *parent=0,const char *name=0);
  ~RmlEngine();
  void sendDelayLength();

 public slots:
  void sendDelayState(int n,Cunctator::DelayState state,int len);
  void sendDelayDumped(int n);

 private:
  QString ResolveCommand(const QString &cmd) const;
  void LoadStack(Profile *p,int id,const QString &tag,
		 std::vector<QHostAddress> *addrs,
		 std::vector<Q_UINT16> *ports,
		 std::vector<QString> *cmds);

  std::vector<QHostAddress> rml_on_bypassed_addresses;
  std::vector<QString> rml_on_bypassed_commands;
  std::vector<Q_UINT16> rml_on_bypassed_ports;

  std::vector<QHostAddress> rml_on_entering_addresses;
  std::vector<QString> rml_on_entering_commands;
  std::vector<Q_UINT16> rml_on_entering_ports;

  std::vector<QHostAddress> rml_on_entered_addresses;
  std::vector<QString> rml_on_entered_commands;
  std::vector<Q_UINT16> rml_on_entered_ports;

  std::vector<QHostAddress> rml_on_exiting_addresses;
  std::vector<QString> rml_on_exiting_commands;
  std::vector<Q_UINT16> rml_on_exiting_ports;

  std::vector<QHostAddress> rml_on_exited_addresses;
  std::vector<QString> rml_on_exited_commands;
  std::vector<Q_UINT16> rml_on_exited_ports;

  std::vector<QHostAddress> rml_on_dump_addresses;
  std::vector<QString> rml_on_dump_commands;
  std::vector<Q_UINT16> rml_on_dump_ports;

  std::vector<QHostAddress> rml_on_delay_change_addresses;
  std::vector<QString> rml_on_delay_change_commands;
  std::vector<Q_UINT16> rml_on_delay_change_ports;

  QSocketDevice *rml_socket;
  Cunctator::DelayState rml_delay_state;
  int rml_delay_length;
};


#endif  // RMLENGINE_H

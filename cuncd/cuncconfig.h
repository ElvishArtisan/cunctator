// cuncconfig.h
//
// A container class for Cunctator Configuration
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef CUNCCONFIG_H
#define CUNCCONFIG_H

#include <QList>
#include <QObject>

#include <cunc.h>

#include "rmlengine.h"
#include "delay.h"
#include "udpqueue.h"

class CuncConfig : public QObject
{
  Q_OBJECT;
 public:
  CuncConfig(bool debug,QObject *parent=0);
  CuncConfig(QString filename,bool debug);
  ~CuncConfig();
  QString filename() const;
  void setFilename(QString filename);
  unsigned tcpPort() const;
  unsigned delays() const;
  Delay *delay(unsigned n);
  RmlEngine *rmlEngine(unsigned n);
  void dumpConfig(FILE *stream);
  bool load();
  void clear();

 public slots:
  void requestDelayModel(int id,int n);
  void requestDelayState(int id,int n);
  void requestDelayStateChange(int n,Cunctator::DelayState state);
  void requestDelayDump(int n);

 signals:
  void delayModelSent(int id,int n,Cunctator::DelayType type,
		      const QString &desc);
  void delayDescriptionSent(int id,int n,const QString &desc);
  void delayStateSent(int id,int n,Cunctator::DelayState state,int len);
  void delayDumped(int n);

 private slots:
  void delayStateChangedData(int n,Cunctator::DelayState state,int len);
  void delayDumpedData(int n);

 private:
  unsigned conf_tcp_port;
  QList<Delay *> conf_delays;
  QList<RmlEngine *> conf_rml_engines;
  QList<UdpQueue *> conf_udp_queues;
  QString conf_filename;
  bool conf_debug;
};


#endif  // CUNCCONFIG_H

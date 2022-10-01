// udpqueue.h
//
// A delay queue for UDP packets.
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

#ifndef UDPQUEUE_H
#define UDPQUEUE_H

#include <QObject>
#include <QList>
#include <QQueue>
#include <QTimer>
#include <QUdpSocket>

#include <cunc.h>

#include <profile.h>
#include <ttydevice.h>

class UdpQueue : public QUdpSocket
{
  Q_OBJECT;
 public:
  UdpQueue(Profile *p,int id,QObject *parent=0);
  ~UdpQueue();

 public slots:
  void changeDelayState(int id,Cunctator::DelayState state,int len);
  void delayDumped(int id);

 private slots:
  void readyReadData();
  void cicReadData(int fd);
  void checkQueueData();
  void sendCicHeartbeatData();

 private:
  void SendUdpPacket();
  void SendCicPacket();
  void ProcessCicTty(unsigned id,const QString msg);
  bool Load(Profile *p);
  double GetTimestamp();
  bool ValidateCicPacket(const QString &pack) const;
  unsigned GetCicChecksum(const QString &str) const;
  QUdpSocket *queue_socket;
  int queue_input_port;
  QList<QHostAddress> queue_destination_addresses;
  QList<int> queue_destination_ports;
  QList<int> queue_fixed_delays;
  QList<TTYDevice *> queue_cic_ttys;
  QStringList queue_cic_buffers;
  int delay_id;
  int queue_id;
  double queue_delay;
  double queue_fixed_delay;
  bool queue_destructive_dump;
  QQueue<char *> queue_udp_datas;
  QQueue<int> queue_udp_lengths;
  QQueue<double> queue_udp_timestamps;
  QTimer *queue_udp_timer;
};


#endif  // UDPQUEUE_H

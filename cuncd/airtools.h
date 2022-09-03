// airtools.h
//
// Delay driver the AirTools 6100 Broadcast Audio Delay
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

#ifndef AIRTOOLS_H
#define AIRTOOLS_H

#include <stdint.h>

#include <QTimer>
//#include <qsocketnotifier.h>

#include <ttydevice.h>

#include "delay.h"

#define AIRTOOLS_MAX_DELAY 10000
#define AIRTOOLS_DELAY_INCREMENT 500
#define AIRTOOLS_BUTTON_DEBOUNCE_LIMIT 2

//
// Command Bitmasks
//
#define AIRTOOLS_CMD_EXIT 0x01
#define AIRTOOLS_CMD_START 0x02
#define AIRTOOLS_CMD_COUGH 0x04
#define AIRTOOLS_CMD_DUMP 0x08

//
// Status Bitmasks
//
#define AIRTOOLS_STATUS_BYPASS 0x01
#define AIRTOOLS_STATUS_EXIT 0x02
#define AIRTOOLS_STATUS_START 0x04
#define AIRTOOLS_STATUS_COUGH 0x08
#define AIRTOOLS_STATUS_DUMP 0x10


class AirTools : public Delay
{
 Q_OBJECT
 public:
  AirTools(Profile *p,int n,bool debug,QObject *parent);
  ~AirTools();
  Cunctator::DelayType type();
  QString description();
  Cunctator::DelayState state();
  int delayLength();
  bool connect();
  void bypass();
  void enter();
  void exit();
  void dump();

 private slots:
  void queryDelayData();
  void readyReadData();

 private:
  void SendCommand(unsigned mask);
  void CurrentState(char state);
  void ApplyChecksum(char *data,int len);
  Cunctator::DelayState airtools_state;
  TTYDevice *airtools_tty;
  //  QSocketNotifier *airtools_notify;
  int airtools_unit_address;
  int airtools_delay_length;
  int airtools_msg_istate;
  int airtools_entering_debounce;
  int airtools_exiting_debounce;
  char airtools_status_mask;
  bool airtools_stats_requested;
  int airtools_host_revision_major;
  int airtools_host_revision_minor;
  int airtools_dsp_revision_major;
  int airtools_dsp_revision_minor;
};


#endif  // AIRTOOLS_H

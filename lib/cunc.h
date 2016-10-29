// cunc.h
//
// System-Wide Values for Cunctator.
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

#ifndef CUNC_H
#define CUNC_H

//
// Default Configuration File
//
#define CUNC_CONF_FILE "/etc/cunctator.conf"

//
// Default Daemon TCP Port
//
#define CUNC_TCP_PORT 3749

//
// Polling Interval for UDP Queues (mS)
//
#define CUNC_UDP_QUEUE_POLL_INTERVAL 100

//
// Number of CIC update packets to send
//
#define CUNC_CIC_PACKET_REPEAT_QUAN 2

//
// CIC Heartbeat Interval
//
#define CUNC_CIC_HEARTBEAT_INTERVAL 5000

class Cunctator
{
 public:
  enum DelayType {TypeUnknown=0,TypeDummy=1,TypeAirTools=2,TypeBd600=3,
		  TypeJackDelay=4};
  enum DelayState {StateUnknown=0,StateBypassed=1,StateEntered=2,
		   StateEntering=3,StateExited=4,StateExiting=5};
};

#endif  // CUNC_H

// airtools.cpp
//
// Delay driver the AirTools 6100 Broadcast Audio Delay
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

#include "airtools.h"

AirTools::AirTools(Profile *p,int n,bool debug,QObject *parent)
  : Delay(p,n,debug,parent)
{
  airtools_stats_requested=false;
  airtools_state=Cunctator::StateEntered;
  airtools_delay_length=0;
  airtools_msg_istate=0;
  QString section=QString().sprintf("Delay%u",n+1);

  airtools_tty=new TTYDevice();
  airtools_tty->setName(p->stringValue(section,"TtyDevice","/dev/ttyS0"));
  airtools_tty->setSpeed(p->intValue(section,"TtySpeed",38400));

  airtools_unit_address=p->intValue(section,"UnitAddress",1);
  if(p->boolValue(section,"SuppressBypassMode",false)) {
    airtools_status_mask=0xfe;
  }
  else {
    airtools_status_mask=0xff;
  }

  airtools_entering_debounce=0;
  airtools_exiting_debounce=0;
}


AirTools::~AirTools()
{
  delete airtools_tty;
  //  delete airtools_notify;
}


Cunctator::DelayType AirTools::type()
{
  return Cunctator::TypeAirTools;
}


QString AirTools::description()
{
  return QString("AirTools Delay");
}


Cunctator::DelayState AirTools::state()
{
  return airtools_state;
}


int AirTools::delayLength()
{
  if(airtools_state==Cunctator::StateBypassed) {
    return 0;
  }
  return 100*airtools_delay_length;
}


bool AirTools::connect()
{
  if(!airtools_tty->open(QIODevice::ReadWrite)) {
    return false;
  }
  /*
  airtools_notify=
    new QSocketNotifier(airtools_tty->socketDescriptor(),QSocketNotifier::Read,this);
  QObject::connect(airtools_notify,SIGNAL(activated(int)),
		   this,SLOT(readyReadData(int)));
  */
  QObject::connect(airtools_tty,SIGNAL(readyRead()),this,SLOT(readyReadData()));

  QTimer *timer=new QTimer(this);
  QObject::connect(timer,SIGNAL(timeout()),this,SLOT(queryDelayData()));
  timer->start(AIRTOOLS_DELAY_INCREMENT);

  if(debugMode()) {  // Get Software Statistics
    char buffer[6];
    snprintf(buffer,6,"%c%c%c%c%c",0xFB,0xFF&airtools_unit_address,
	    0x00,0x02,0x10);
    ApplyChecksum(buffer,5);
    airtools_tty->write(buffer,6);
    airtools_stats_requested=true;
    printf("AirTools 6100 Driver\n");
  }

  return true;
}


void AirTools::bypass()
{
  if(airtools_state!=Cunctator::StateBypassed) {
    airtools_state=Cunctator::StateBypassed;
    airtools_delay_length=0;
    emit delayStateChanged(id(),airtools_state,airtools_delay_length);
  }
}


void AirTools::enter()
{
  SendCommand(AIRTOOLS_CMD_START);
}


void AirTools::exit()
{
  SendCommand(AIRTOOLS_CMD_EXIT);
}


void AirTools::dump()
{
  SendCommand(AIRTOOLS_CMD_DUMP);
}


void AirTools::queryDelayData()
{
  char buffer[10];

  /*
   * Query Delay Length
   */
  sprintf(buffer,"%c%c%c%c%c",0xFB,0xFF&airtools_unit_address,
	  0x00,0x02,0x11);
  ApplyChecksum(buffer,5);
  airtools_tty->write(buffer,6);
}


void AirTools::readyReadData()
{
  char data[1500];
  int n;

  int i;

  while((n=airtools_tty->read(data,1500))>0) {
    for(i=0;i<n;i++) {
      //      printf("data[%d]: 0x%02X\n",i,0xFF&data[i]);
      switch(airtools_msg_istate) {
      case 0:  /* Unit Address */
	if(data[i]==airtools_unit_address) {
	  airtools_msg_istate=1;
	}
	break;

      case 1:  /* Device Type */
	airtools_msg_istate=2;
	break;

      case 2:  /* Mfgr's Code */
	airtools_msg_istate=3;
	break;

      case 3:  /* Message Length MSB */
	if(data[i]==0x00) {
	  airtools_msg_istate=4;
	}
	else {
	  airtools_msg_istate=0;
	}
	break;

      case 4:  /* Message Length LSB */
	if(data[i]==0x0C) {
	  if(airtools_stats_requested) {
	    airtools_stats_requested=false;
	    airtools_msg_istate=100;
	  }
	  else {
	    airtools_msg_istate=5;
	  }
	}
	else {
	  airtools_msg_istate=0;
	}
	break;

      case 5:  /* Make up percentage (0.5% steps, 0 - 200) */
	airtools_msg_istate=6;
	break;

      case 6:  /* Make up time (0.1 sec steps, 0 - 200) */
	if((0xFF&data[i])!=airtools_delay_length) {
	  airtools_delay_length=0xFF&data[i];
	  emit delayStateChanged(id(),airtools_state,delayLength());
	}
	airtools_msg_istate=7;
	break;

      case 7:   // CH 1 Level
	airtools_msg_istate=8;
	break;

      case 8:   // CH 2 Level
	airtools_msg_istate=9;
	break;

      case 9:   // Current Source  0=Analog, 1=AES
	airtools_msg_istate=10;
	break;
	
      case 10:   // Sync source 
	airtools_msg_istate=11;
	break;
	
      case 11:   // Front panel LEDs 
	CurrentState(data[i]);
	airtools_msg_istate=12;
	break;
	
      case 12:   // Sample rate MSB
	airtools_msg_istate=13;
	break;
	
      case 13:   // MID 
	airtools_msg_istate=14;
	break;
	
      case 14:   // Sample rate LSB 
	airtools_msg_istate=15;
	break;
	
      case 15:   // Status 
	airtools_msg_istate=16;
	break;
	
      case 16:   // Checksum 
	airtools_msg_istate=0;
	break;
 
      case 100:  // Host Revision Number MSB
	airtools_host_revision_major=0xFF&data[i];
	airtools_msg_istate=101;
	break;

      case 101:  // Host Revision Number LSB
	airtools_host_revision_minor=0xFF&data[i];
	printf("Host Version: R%d.%02d\n",
	       airtools_host_revision_major,
	       airtools_host_revision_minor);
	airtools_msg_istate=102;
	break;

      case 102:  // Host Revision Day
	airtools_msg_istate=103;
	break;

      case 103:  // Host Revision Month
	airtools_msg_istate=104;
	break;

      case 104:  // Host Revision Year
	airtools_msg_istate=105;
	break;

      case 105:  // DSP Revision Number MSB
	airtools_dsp_revision_major=0xFF&data[i];
	airtools_msg_istate=106;
	break;

      case 106:  // DSP Revision Number LSB
	airtools_dsp_revision_minor=0xFF&data[i];
	printf("DSP Version: R%d.%02d\n",
	       airtools_dsp_revision_major,
	       airtools_dsp_revision_minor);
	airtools_msg_istate=107;
	break;

      case 107:  // DSP Revision Number Day
	airtools_msg_istate=108;
	break;

      case 108:  // DSP Revision Number Month
	airtools_msg_istate=109;
	break;

      case 109:  // DSP Revision Number Year
	airtools_msg_istate=110;
	break;

      case 110:  // Status
	airtools_msg_istate=16;
	break;

      default:
	airtools_msg_istate=0;
	break;
      }
    }
  }
}


void AirTools::SendCommand(unsigned mask)
{
  //  printf("SendCommand(0x%02X)\n",0xFF&mask);

  char buffer[10];

  snprintf(buffer,10,"%c%c%c%c%c%c",0xFB,0xFF&airtools_unit_address,
	  0x00,0x03,0x90,0xFF&mask);
  ApplyChecksum(buffer,6);
  airtools_tty->write(buffer,7);
}


void AirTools::CurrentState(char state)
{
  state=state&airtools_status_mask;
  if(airtools_state==Cunctator::StateBypassed) {
    if((state&0x01)==0) {
      airtools_state=Cunctator::StateEntered;
      emit delayStateChanged(id(),airtools_state,delayLength());
      return;
    }
  }
  else {
    if((state&0x01)!=0) {   // Check for bypass
      airtools_state=Cunctator::StateBypassed;
      emit delayStateChanged(id(),airtools_state,delayLength());
      return;
    }
    if((AIRTOOLS_STATUS_START&state)!=0) {
      if(airtools_state!=Cunctator::StateEntering) {
	airtools_state=Cunctator::StateEntering;
	emit delayStateChanged(id(),airtools_state,delayLength());
	airtools_entering_debounce=0;
	return;
      }
    }
    else {
      if(airtools_state==Cunctator::StateEntering) {
	if(airtools_entering_debounce>AIRTOOLS_BUTTON_DEBOUNCE_LIMIT) {
	  airtools_state=Cunctator::StateEntered;
	  emit delayStateChanged(id(),airtools_state,delayLength());
	}
	else {
	  airtools_entering_debounce++;
	}
	return;
      }
    }
    if((AIRTOOLS_STATUS_EXIT&state)!=0) {
      if(airtools_state!=Cunctator::StateExiting) {
	airtools_state=Cunctator::StateExiting;
	emit delayStateChanged(id(),airtools_state,delayLength());
	airtools_exiting_debounce=0;
	return;
      }
    }
    else {
      if(airtools_state==Cunctator::StateExiting) {
	if(airtools_exiting_debounce>AIRTOOLS_BUTTON_DEBOUNCE_LIMIT) {
	  airtools_state=Cunctator::StateEntered;
	  emit delayStateChanged(id(),airtools_state,delayLength());
	}
	else {
	  airtools_exiting_debounce++;
	}
	return;
      }
    }
  }
  if((state&AIRTOOLS_STATUS_DUMP)!=0) {
    SendCommand(0);   // Reset DUMP button
  }
}


void AirTools::ApplyChecksum(char *data,int len)
{
  unsigned cs=0;
  for(int i=2;i<len;i++) {
    cs+=data[i];
  }
  data[len]=0x100-(0xFF&cs);
}

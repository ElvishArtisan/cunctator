// udpqueue.cpp
//
// A delay queue for UDP packets.
//
//   (C) Copyright 2011-2025 Fred Gleason <fredg@paravelsystems.com>
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

#include <syslog.h>
#include <sys/time.h>
#include <ctype.h>

#include <QStringList>
#include <QSocketNotifier>

#include "udpqueue.h"

UdpQueue::UdpQueue(Profile *p,int id,QObject *parent)
  : QUdpSocket(parent)
{
  bool ok=false;
  QString section;
  QString ttydev;
  int count=0;

  queue_id=id;
  queue_delay=0.0;
  queue_fixed_delay=0.0;
  queue_destructive_dump=true;

  //
  // UDP Socket
  //
  queue_socket=new QUdpSocket(this);
  //  queue_socket->setBlocking(false);
  if(!Load(p)) {
    return;
  }
  if(!queue_socket->bind(QHostAddress(),queue_input_port)) {
    syslog(LOG_ERR,"unable to bind input socket for [UdpQueue%d]",id);
    return;
  }
  connect(queue_socket,SIGNAL(readyRead()),this,SLOT(readyReadData()));

  //
  // CIC TTY Devices
  //
  section=QString::asprintf("UdpQueue%d",id+1);
  ttydev=p->stringValue(section,QString::asprintf("DestinationCicDevice%d",
						  count+1),"",&ok);
  while(ok) {
    queue_cic_buffers.push_back(QString(""));
    queue_cic_ttys.push_back(new TTYDevice());
    queue_cic_ttys.back()->setName(ttydev);
    queue_cic_ttys.back()->
      setSpeed(p->intValue(section,QString::asprintf("DestinationCicSpeed%d",
						     count+1),9600));
    if(queue_cic_ttys.back()->open(QIODevice::ReadWrite)) {
      connect(queue_cic_ttys.back(),SIGNAL(activated(int)),
	      this,SLOT(cicReadData(int)));
    }
    else {
      syslog(LOG_ERR,"Unable to open tty device \"%s\".",
	     ttydev.toUtf8().constData());
    }
    count++;
    ttydev=p->stringValue(section,QString::asprintf("DestinationCicDevice%d",
						    count+1).toUtf8(),"",&ok);
  }
  if(queue_cic_ttys.size()>0) {
    QTimer *timer=new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(sendCicHeartbeatData()));
    timer->start(CUNC_CIC_HEARTBEAT_INTERVAL);
  }

  //
  // Queue Timer
  //
  queue_udp_timer=new QTimer(this);
  connect(queue_udp_timer,SIGNAL(timeout()),this,SLOT(checkQueueData()));
}


UdpQueue::~UdpQueue()
{
}


void UdpQueue::changeDelayState(int id,Cunctator::DelayState state,int len)
{
  queue_delay=(double)len/1000.0;
}


void UdpQueue::delayDumped(int id)
{
  if(queue_destructive_dump) {
    while(queue_udp_datas.size()>0) {
      queue_udp_datas.dequeue();
      queue_udp_lengths.dequeue();
      queue_udp_timestamps.dequeue();
    }
  }    
}


void UdpQueue::readyReadData()
{
  int n;
  char data[1500];

  while((n=queue_socket->readDatagram(data,1500))>0) {
    queue_udp_datas.enqueue(new char[n]);
    memcpy(queue_udp_datas.back(),data,n);
    queue_udp_lengths.enqueue(n);
    queue_udp_timestamps.enqueue(GetTimestamp());
  } 
  if(!queue_udp_timer->isActive()) {
    queue_udp_timer->start(CUNC_UDP_QUEUE_POLL_INTERVAL);
  }
}


void UdpQueue::cicReadData(int fd)
{
  int n;
  char data[1500];

  for(int i=0;i<queue_cic_ttys.size();i++) {
    if(queue_cic_ttys[i]->socketDescriptor()==fd) {
      while((n=queue_cic_ttys[i]->readBlock(data,1500))>0) {
	for(int j=0;j<n;j++) {
	  if(isprint(data[j])) {
	    queue_cic_buffers[i]+=data[j];
	  }
	  if(data[j]==13) {
	    ProcessCicTty(i,queue_cic_buffers[i]);
	    queue_cic_buffers[i]="";
	  }
	}
      }
      return;
    }
  }
  syslog(LOG_WARNING,"received unknown tty file descriptor \"%d\"",fd);
}


void UdpQueue::checkQueueData()
{
  double timestamp=GetTimestamp();

  while((queue_udp_timestamps.size()>0)&&
	((queue_udp_timestamps.front()+queue_delay+queue_fixed_delay)<
	 timestamp)) {
    SendUdpPacket();
    SendCicPacket();
    delete queue_udp_datas.front();
    queue_udp_datas.dequeue();
    queue_udp_lengths.dequeue();
    queue_udp_timestamps.dequeue();
  }

  if(queue_udp_datas.size()==0) {
    queue_udp_timer->stop();
  }
}


void UdpQueue::sendCicHeartbeatData()
{
  for(int i=0;i<queue_cic_ttys.size();i++) {
    queue_cic_ttys[i]->write("login\r\n",7);
    queue_cic_ttys[i]->write("atdt\r\n",6);
    queue_cic_ttys[i]->write("PINGSEND\r\n",10);
  }
}


void UdpQueue::SendUdpPacket()
{
  for(int i=0;i<queue_destination_addresses.size();i++) {
    queue_socket->writeDatagram(queue_udp_datas.front(),
				queue_udp_lengths.front(),
				queue_destination_addresses[i],
				queue_destination_ports[i]);
  }
}


void UdpQueue::SendCicPacket()
{
  char cic[1500];
  char pack[1500];

  if(queue_cic_ttys.size()==0) {
    return;
  }
  memset(cic,0,sizeof(cic));
  memcpy(cic,queue_udp_datas.front(),queue_udp_lengths.front());
  if(!ValidateCicPacket(cic)) {
    return;
  }
  snprintf(pack,1500,"NC:%u:%s\r\n",GetCicChecksum(cic),cic);
  for(int i=0;i<queue_cic_ttys.size();i++) {
    for(int j=0;j<CUNC_CIC_PACKET_REPEAT_QUAN;j++) {
      queue_cic_ttys[i]->write(pack,strlen(pack));
    }
  }
}


void UdpQueue::ProcessCicTty(unsigned id,const QString msg)
{
  //printf("msg[%u]: |%s|\n",id,(const char *)msg);

  if(msg=="PINGRECV") {
    queue_cic_ttys[id]->write("PONGRECV\r\n",10);
  }
}


bool UdpQueue::Load(Profile *p)
{
  QString section=QString::asprintf("UdpQueue%d",queue_id+1);
  QHostAddress addr;
  int port;
  int delay=0;
  int count=0;

  if((delay_id=p->intValue(section,"DelayNumber",-1))<0) {
    syslog(LOG_ERR,"missing/invalid DelayNumber= value in %s\n",
	   section.toUtf8().constData());
    return false;
  }
  if((queue_input_port=p->intValue(section,"InputPort",-1))<0) {
    syslog(LOG_ERR,"missing/invalid InputPort= value in %s\n",
	   section.toUtf8().constData());
    return false;
  }
  queue_fixed_delay=(double)p->intValue(section,"FixedDelay",0)/1000;
  queue_destructive_dump=p->boolValue(section,"DestructiveDump",true);

  bool ok=addr.setAddress(p->stringValue(section,
			  QString::asprintf("DestinationAddress%d",count+1)));
  while(ok) {
    port=p->intValue(section,QString::asprintf("DestinationPort%d",count+1),0);
    if((port>0)&&(port<0x10000)) {
      queue_destination_addresses.push_back(addr);
      queue_destination_ports.push_back(port);
      queue_fixed_delays.push_back(delay);
    }
    else {
      syslog(LOG_ERR,"missing/invalid DestinationPort%d value in %s\n",
	     count+1,section.toUtf8().constData());
      return false;
    }
    count++;
    ok=addr.setAddress(p->stringValue(section,
		       QString::asprintf("DestinationAddress%d",count+1)));
  }

  return true;
}


double UdpQueue::GetTimestamp()
{
  struct timeval tv;

  gettimeofday(&tv,NULL);
  return (double)tv.tv_sec+(double)tv.tv_usec/1000000.0;
}


bool UdpQueue::ValidateCicPacket(const QString &pack) const
{
  QStringList f0=pack.split(":",Qt::KeepEmptyParts);
  switch(f0.at(0).toUInt()) {
  case 0:
    if(f0.size()!=4) {
      return false;
    }
    if(f0.at(1).length()>5) {  // Program Code
      return false;
    }
    if((f0.at(2).length()<8)||(f0.at(2).length()>25)) {  // ISCI Code
      return false;
    }
    if((f0.at(3).length()<1)||(f0.at(3).length()>2)) {  // Mode
      return false;
    }
    break;

  default:
    return false;
  }
  return true;
}


unsigned UdpQueue::GetCicChecksum(const QString &str) const
{
  unsigned ret=0;
  for(int i=0;i<str.length();i++) {
    ret+=str.at(i).cell();
  }
  return ret;
}



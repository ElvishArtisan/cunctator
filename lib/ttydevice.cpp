// ttydevice.cpp
//
//   A Qt driver for tty ports.
//
//   (C) Copyright 2010-2025 Fred Gleason <fredg@paravelsystems.com>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Library General Public License 
//   version 2 as published by the Free Software Foundation.
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

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <syslog.h>

#include "ttydevice.h"

TTYDevice::TTYDevice(QObject *parent)
  : QIODevice(parent)
{
  tty_speed=9600;
  tty_length=8;
  tty_parity=TTYDevice::None;
  tty_flow_control=TTYDevice::FlowNone;
  tty_open=false;

  tty_write_timer=new QTimer(this);
  tty_write_timer->setSingleShot(false);
  connect(tty_write_timer,SIGNAL(timeout()),this,SLOT(writeTtyData()));
}

TTYDevice::~TTYDevice()
{
  close();
  delete tty_write_timer;
}


bool TTYDevice::open(QIODevice::OpenMode mode)
{
  int flags=O_NONBLOCK|O_NOCTTY;
  struct termios term;

  QIODevice::open(mode);

  tty_mode=mode;
  if((mode&QIODevice::ReadWrite)==QIODevice::ReadWrite) {
    flags|=O_RDWR;
  }
  else {
    if(((mode&QIODevice::WriteOnly)!=0)) {
      flags|=O_WRONLY;
    }
    if(((mode&QIODevice::ReadOnly)!=0)) {
      flags|=O_RDONLY;
    }
  }
  if((mode&QIODevice::Append)!=0) {
    flags|=O_APPEND;
  }
  if((mode&QIODevice::Truncate)!=0) {
    flags|=O_TRUNC;
  }

  if((tty_fd=::open(tty_name.toUtf8(),flags))<0) {
    return false;
  }
  tty_open=true;

  tcgetattr(tty_fd,&term);

  //
  // Set Speed
  //
  //cfsetispeed(&term,B0);
  cfsetispeed(&term,tty_speed);
  cfsetospeed(&term,tty_speed);

  //
  // Set Mode
  //
  cfmakeraw(&term);
  term.c_iflag |= IGNBRK; 

  //
  // Set Parity
  //
  switch(tty_parity) {
  case TTYDevice::None:
    term.c_iflag |= IGNPAR;
    break;

  case TTYDevice::Even:
    term.c_cflag |= PARENB;
    break;

  case TTYDevice::Odd:
    term.c_cflag |= PARENB|PARODD;
    break;
  }

  //
  // Set Word Length
  //
  //term.c_cflag &= ~CSIZE;
  switch(tty_length) {
  case 5:
    term.c_cflag |= CS5;
    break;

  case 6:
    term.c_cflag |= CS6;
    break;

  case 7:
    term.c_cflag |= CS7;
    break;

  case 8:
    term.c_cflag |= CS8;
    break;
  }

  //
  // Set Flow Control
  //
  switch(tty_flow_control) {
  case TTYDevice::FlowNone:
    term.c_cflag &= ~CRTSCTS;
    term.c_iflag &= ~IXON;
    term.c_iflag &= ~IXOFF;
    break;

  case TTYDevice::FlowRtsCts:
    term.c_cflag |= CRTSCTS;
    term.c_iflag &= ~IXON;
    term.c_iflag &= ~IXOFF;
    break;

  case TTYDevice::FlowXonXoff:
    term.c_cflag &= ~CRTSCTS;
    term.c_iflag |= IXON;
    term.c_iflag |= IXOFF;
    break;
  }

  tcsetattr(tty_fd,TCSADRAIN,&term);

  tty_notifier=new QSocketNotifier(tty_fd,QSocketNotifier::Read,this);
  connect(tty_notifier,SIGNAL(activated(int)),this,SLOT(readTtyData(int)));

  tty_write_timer->start(10);

  return true;
}


void TTYDevice::close()
{
  if(tty_open) {
    emit aboutToClose();
    tty_write_timer->stop();
    delete tty_notifier;
    tty_notifier=NULL;
    ::close(tty_fd);
    if((tty_mode&QIODevice::ReadOnly)!=0) {
      emit readChannelFinished();
    }
  }
  tty_open=false;
}


int TTYDevice::socketDescriptor() const
{
  return tty_notifier->socket();
}


QString TTYDevice::name() const
{
  return tty_name;
}


void TTYDevice::setName(const QString &str)
{
  tty_name=str;
}


qint64 TTYDevice::read(char *data,qint64 maxlen)
{
  return readData(data,maxlen);
}


QByteArray TTYDevice::read(qint64 maxlen)
{
  qint64 n=0;
  char *data=(char *)malloc(maxlen);
  n=readData(data,maxlen);
  QByteArray ret(data,n);
  free(data);
  return ret;
}


QByteArray TTYDevice::readAll()
{
  return read(bytesAvailable());
}


qint64 TTYDevice::readBlock(char *data,qint64 maxlen)
{
  return readData(data,maxlen);
}


qint64 TTYDevice::write(const char *data,qint64 len)
{
  return writeData(data,len);
}


qint64 TTYDevice::write(const QByteArray &array)
{
  return write(array.constData(),array.size());
}


bool TTYDevice::getChar(char *ch)
{
  return readData(ch,1)==1;
}


bool TTYDevice::putChar(char ch)
{
  return writeData(&ch,1)==1;
}


qint64 TTYDevice::size() const
{
  return bytesAvailable();
}


qint64 TTYDevice::bytesAvailable() const
{
  int val=0;
  if(tty_open) {
    ioctl(tty_fd,FIONREAD,&val);
  }
  return val;
}


qint64 TTYDevice::bytesToWrite() const
{
  int val=0;
  if(tty_open) {
    ioctl(tty_fd,TIOCOUTQ,&val);
  }
  return val;
}


bool TTYDevice::isSequential() const
{
  return true;
}


bool TTYDevice::isReadable() const
{
  return((tty_mode&QIODevice::ReadOnly)!=0)||
    ((tty_mode&QIODevice::ReadWrite)!=0);
}


bool TTYDevice::isWritable() const
{
  return ((tty_mode&QIODevice::WriteOnly)!=0)||
    ((tty_mode&QIODevice::ReadWrite)!=0);
}


bool TTYDevice::isOpen() const
{
  return tty_open;
}


QString TTYDevice::deviceName() const
{
  return tty_name;
}


void TTYDevice::setDeviceName(QString name)
{
  tty_name=name;
}


int TTYDevice::speed() const
{
  switch(tty_speed) {
  case B0:
    return 0;
    break;

  case B50:
    return 50;
    break;

  case B75:
    return 75;
    break;

  case B110:
    return 110;
    break;

  case B134:
    return 134;
    break;

  case B150:
    return 150;
    break;

  case B200:
    return 200;
    break;

  case B300:
    return 300;
    break;

  case B600:
    return 600;
    break;

  case B1200:
    return 1200;
    break;

  case B1800:
    return 1800;
    break;

  case B2400:
    return 2400;
    break;

  case B4800:
    return 4800;
    break;

  case B9600:
    return 9600;
    break;

  case B19200:
    return 19200;
    break;

  case B38400:
    return 38400;
    break;

  case B57600:
    return 57600;
    break;

  case B115200:
    return 115200;
    break;

  case B230400:
    return 230400;
    break;
  }
  return 0;
}


void TTYDevice::setSpeed(int speed)
{
  switch(speed) {
  case 0:
    tty_speed=B0;
    break;

  case 50:
    tty_speed=B50;
    break;

  case 75:
    tty_speed=B75;
    break;

  case 110:
    tty_speed=B110;
    break;

  case 134:
    tty_speed=B134;
    break;

  case 150:
    tty_speed=B150;
    break;

  case 200:
    tty_speed=B200;
    break;

  case 300:
    tty_speed=B300;
    break;

  case 600:
    tty_speed=B600;
    break;

  case 1200:
    tty_speed=B1200;
    break;

  case 1800:
    tty_speed=B1800;
    break;

  case 2400:
    tty_speed=B2400;
    break;

  case 4800:
    tty_speed=B4800;
    break;

  case 9600:
    tty_speed=B9600;
    break;

  case 19200:
    tty_speed=B19200;
    break;

  case 38400:
    tty_speed=B38400;
    break;

  case 57600:
    tty_speed=B57600;
    break;

  case 115200:
    tty_speed=B115200;
    break;

  case 230400:
    tty_speed=B230400;
    break;

  default:
    tty_speed=B9600;
    break;
  }
}


int TTYDevice::wordLength() const
{
  switch(tty_length) {
  case CS5:
    return 5;
    break;

  case CS6:
    return 6;
    break;

  case CS7:
    return 7;
    break;

  case CS8:
    return 8;
    break;
  }
  return 0;
}


void TTYDevice::setWordLength(int length)
{
  switch(length) {
  case 5:
    tty_length=CS5;
    break;

  case 6:
    tty_length=CS6;
    break;

  case 7:
    tty_length=CS7;
    break;

  case 8:
    tty_length=CS8;
    break;

  default:
    tty_length=CS8;
    break;
  }
}


TTYDevice::Parity TTYDevice::parity() const
{
  return tty_parity;
}


void TTYDevice::setParity(Parity parity)
{
  tty_parity=parity;
}


TTYDevice::FlowControl TTYDevice::flowControl() const
{
  return tty_flow_control;
}


void TTYDevice::setFlowControl(FlowControl ctrl)
{
  tty_flow_control=ctrl;
}


int TTYDevice::fileDescriptor() const
{
  return tty_fd;
}


qint64 TTYDevice::readData(char *data,qint64 maxlen)
{
  qint64 n;

  if((n=::read(tty_fd,data,(size_t)maxlen))<0) {
    return 0;
  }
  return n;
}


qint64 TTYDevice::writeData(const char *data,qint64 len)
{
  for(qint64 i=0;i<len;i++) {
    tty_write_queue.enqueue(data[i]);
  }
  emit bytesWritten(len);
  return len;
}


void TTYDevice::readTtyData(int sock)
{
  emit readyRead();
}


void TTYDevice::writeTtyData()
{
  char data[2048];
  int bytes=0;
  int s;
  
  ioctl(tty_fd,TIOCOUTQ,&bytes);
  int n=2048-bytes;
  if((int)tty_write_queue.size()<n) {
    n=tty_write_queue.size();
  }
  if(n==0) {
    return;
  }

  for(ssize_t i=0;i<n;i++) {
    data[i]=tty_write_queue.dequeue();
  }
  if((s=::write(tty_fd,data,n))!=n) {
    syslog(LOG_WARNING,"TTYDevice::writeTtyData write lost %c bytes",n-s);
  }
}

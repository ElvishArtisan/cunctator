// cuncclient.cpp
//
// Utility for controlling broadcast delays.
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

#include <stdlib.h>
#include <ctype.h>

#include <qmessagebox.h>
#include <qpixmap.h>

#include <cunc.h>
#include <cmdswitch.h>

#include <cuncclient.h>

//
// Icons
//
#include "../icons/cunc-16x16.xpm"

MainWidget::MainWidget(QWidget *parent,const char *name)
  : QMainWindow(parent,name)
{
  QString hostname=CUNC_DEFAULT_ADDR;
  unsigned port=CUNC_TCP_PORT;
  bool ok;
  cunc_delay_id=0;

  //
  // Read Command Options
  //
  CmdSwitch *cmd=
    new CmdSwitch(qApp->argc(),qApp->argv(),"cunc",CUNC_USAGE);
  for(unsigned i=0;i<cmd->keys();i++) {
    if(cmd->key(i)=="--hostname") {
      hostname=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--port") {
      port=cmd->value(i).toUInt(&ok);
      if((!ok)||(port>0xFFFF)) {
	fprintf(stderr,"cunc: invalid port value\n");
	exit(256);
      }
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--delay") {
      cunc_delay_id=cmd->value(i).toUInt(&ok);
      if(!ok) {
	fprintf(stderr,"cunc: invalid delay number\n");
	exit(256);
      }
      cmd->setProcessed(i,true);
    }
    if(!cmd->processed(i)) {
      fprintf(stderr,"cuncd: unrecognized option \"%s\"\n",
	      (const char *)cmd->key(i));
      exit(256);
    }
  }
  delete cmd;

  //
  // Set Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());

  //
  // Generate Fonts
  //
  QFont button_font("helvetica",14,QFont::Bold);
  button_font.setPixelSize(14);
  QFont label_font("courier",40,QFont::Bold);
  label_font.setPixelSize(40);

  setCaption(tr("Broadcast Delay Control"));

  //
  // Create Icons
  //
  QPixmap *icon_map=new QPixmap(cunc_16x16_xpm);
  setIcon(*icon_map);

  //
  // Enter Button
  //
  cunc_enter_button=new PushButton(tr("Start"),this);
  cunc_enter_button->setGeometry(10,10,80,50);
  cunc_enter_button->setFont(button_font);
  connect(cunc_enter_button,SIGNAL(clicked()),this,SLOT(enterPushed()));

  //
  // Exit Button
  //
  cunc_exit_button=new PushButton(tr("Exit"),this);
  cunc_exit_button->setGeometry(100,10,80,50);
  cunc_exit_button->setFont(button_font);
  connect(cunc_exit_button,SIGNAL(clicked()),this,SLOT(exitPushed()));

  //
  // Delay Time Display
  //
  cunc_delay_label=new QLabel(this);
  cunc_delay_label->setGeometry(200,10,120,50);
  cunc_delay_label->setFont(label_font);
  cunc_delay_label->setAlignment(AlignCenter);
  cunc_delay_label->setFrameStyle(QFrame::Box|QFrame::Raised);

  //
  // Dump Button
  //
  cunc_dump_button=new PushButton(tr("Dump"),this);
  cunc_dump_button->setGeometry(340,10,80,50);
  cunc_dump_button->setFont(button_font);
  cunc_dump_button->setFlashColor(red);
  connect(cunc_dump_button,SIGNAL(clicked()),this,SLOT(dumpPushed()));

  cunc_dump_timer=new QTimer(this);
  connect(cunc_dump_timer,SIGNAL(timeout()),this,SLOT(dumpFlashResetData()));

  //
  // Data Socket
  //
  cunc_socket=new QSocket(this);
  connect(cunc_socket,SIGNAL(connected()),this,SLOT(socketConnectedData()));
  connect(cunc_socket,SIGNAL(connectionClosed()),this,SLOT(socketClosedData()));
  connect(cunc_socket,SIGNAL(readyRead()),this,SLOT(readyReadData()));
  connect(cunc_socket,SIGNAL(error(int)),this,SLOT(errorData(int)));
  cunc_socket->connectToHost(hostname,port);
}


QSize MainWidget::sizeHint() const
{
  return QSize(430,70);
}


QSizePolicy MainWidget::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void MainWidget::enterPushed()
{
  SendCommand(QString().sprintf("SS %u %u!",cunc_delay_id,
				Cunctator::StateEntering));
}


void MainWidget::exitPushed()
{
  SendCommand(QString().sprintf("SS %u %u!",cunc_delay_id,
				Cunctator::StateExiting));
}


void MainWidget::dumpPushed()
{
  SendCommand(QString().sprintf("DP %u!",cunc_delay_id));
}


void MainWidget::dumpFlashResetData()
{
  cunc_dump_button->setFlashingEnabled(false);
}


void MainWidget::socketConnectedData()
{
  SendCommand(QString().sprintf("DS %u!",cunc_delay_id));
  SendCommand(QString().sprintf("DM %u!",cunc_delay_id));
}


void MainWidget::socketClosedData()
{
  QMessageBox::warning(this,tr("Broadcast Delay Control"),
		       tr("Remote system closed connection!"));
  exit(256);
}


void MainWidget::readyReadData()
{
  int n;
  char data[1500];

  while((n=cunc_socket->readBlock(data,1500))>0) {
    for(int i=0;i<n;i++) {
      if(isprint(data[i])) {
	if(data[i]=='!') {
	  ProcessCommand(cunc_buffer);
	  cunc_buffer="";
	}
	else {
	  cunc_buffer+=data[i];
	}
      }
    }
  }
}


void MainWidget::errorData(int err)
{
  switch((QSocket::Error)err) {
  case QSocket::ErrConnectionRefused:
    QMessageBox::warning(this,tr("Broadcast Delay Control"),
			 tr("Connection Refused!"));
    break;

  case QSocket::ErrHostNotFound:
    QMessageBox::warning(this,tr("Broadcast Delay Control"),
			 tr("Host Not Found!"));
    break;

  case QSocket::ErrSocketRead:
    QMessageBox::warning(this,tr("Broadcast Delay Control"),
			 tr("Socket Read Error!"));
    break;

  default:
    QMessageBox::warning(this,tr("Broadcast Delay Control"),
			 tr("An unspecified network error occurred!"));
    break;
  }
  exit(256);
}


void MainWidget::ProcessCommand(const QString &msg)
{
  //printf("msg: %s\n",(const char *)msg);

  QStringList cmds=cmds.split(" ",msg);
  bool ok;
  unsigned id;
  int delay_len;
  QString desc;

  if(cmds.size()<1) {
    return;
  }
  if(cmds[0]=="DS") {   // Delay State
    id=cmds[1].toUInt(&ok);
    if((!ok)||(id!=cunc_delay_id)) {
      return;
    }

    //
    // Update Buttons
    //
    cunc_enter_button->setDisabled((Cunctator::DelayState)cmds[2].toInt()==
				   Cunctator::StateBypassed);
    cunc_exit_button->setDisabled((Cunctator::DelayState)cmds[2].toInt()==
				   Cunctator::StateBypassed);
    cunc_dump_button->setDisabled((Cunctator::DelayState)cmds[2].toInt()==
				   Cunctator::StateBypassed);
    cunc_delay_label->setDisabled((Cunctator::DelayState)cmds[2].toInt()==
				  Cunctator::StateBypassed);
    switch((Cunctator::DelayState)cmds[2].toInt()) {
    case Cunctator::StateBypassed:
    case Cunctator::StateEntered:
    case Cunctator::StateExited:
      cunc_enter_button->setFlashingEnabled(false);
      cunc_exit_button->setFlashingEnabled(false);
      break;

    case Cunctator::StateEntering:
      cunc_enter_button->setFlashingEnabled(true);
      cunc_exit_button->setFlashingEnabled(false);
      break;

    case Cunctator::StateExiting:
      cunc_enter_button->setFlashingEnabled(false);
      cunc_exit_button->setFlashingEnabled(true);
      break;

    case Cunctator::StateUnknown:
      break;
    }

    //
    // Update Delay Time Counter
    //
    delay_len=cmds[3].toInt(&ok);
    if(!ok) {
      return;
    }
    //cunc_delay_lcd->display(delay_len/1000);
    cunc_delay_label->
      setText(QString().sprintf("%4.1f",(float)delay_len/1000.0));
  }

  if(cmds[0]=="DP") {   // Delay Dump
    id=cmds[1].toUInt(&ok);
    if((!ok)||(id!=cunc_delay_id)) {
      return;
    }
    cunc_dump_button->setFlashingEnabled(true);
    cunc_dump_timer->start(5000,true);
  }

  if(cmds[0]=="DM") {   // Delay Model
    id=cmds[1].toUInt(&ok);
    if((!ok)||(id!=cunc_delay_id)) {
      return;
    }
    for(unsigned i=3;i<cmds.size();i++) {
      desc+=(cmds[i]+" ");
    }
    setCaption(desc);
  }
}


void MainWidget::SendCommand(const QString &msg)
{
  cunc_socket->writeBlock(msg,msg.length());
}


int main(int argc,char *argv[])
{
  QApplication a(argc,argv,true);
  MainWidget *w=new MainWidget(NULL,"main");
  a.setMainWidget(w);
  w->show();
  return a.exec();
}

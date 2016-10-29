// cuncclient.h
//
// Utility for controlling broadcast delays.
//
//   (C) Copyright 2011 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: cuncclient.h,v 1.3 2011/03/17 20:48:40 cvs Exp $
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

#ifndef CUNCCLIENT_H
#define CUNCCLIENT_H

#include <qapplication.h>
#include <qmainwindow.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qsocket.h>
#include <qlabel.h>
#include <qtimer.h>

#include <pushbutton.h>

#define CUNC_USAGE "[--hostname=<hostname>] [--port=<port>] [--delay=<delay-num>]\n\nWhere <hostname> is the name or IP address of the host to connect to\n(default = localhost), <port> is the TCP port to connect to\n(default = 3749)\n"
#define CUNC_DEFAULT_ADDR "localhost"

class MainWidget : public QMainWindow
{
  Q_OBJECT
 public:
  MainWidget(QWidget *parent=0,const char *name=0);
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;

 private slots:
  void enterPushed();
  void exitPushed();
  void dumpPushed();
  void dumpFlashResetData();
  void socketConnectedData();
  void socketClosedData();
  void readyReadData();
  void errorData(int err);
  

 private:
  void ProcessCommand(const QString &msg);
  void SendCommand(const QString &msg);
  PushButton *cunc_enter_button;
  PushButton *cunc_exit_button;
  PushButton *cunc_dump_button;
  QLabel *cunc_delay_label;
  QSocket *cunc_socket;
  QString cunc_buffer;
  unsigned cunc_delay_id;
  QTimer *cunc_dump_timer;
};


#endif  // CUNCCLIENT_H

// cuncclient.h
//
// Utility for controlling broadcast delays.
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

#ifndef CUNCCLIENT_H
#define CUNCCLIENT_H

#include <QApplication>
#include <QLabel>
#include <QLCDNumber>
#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>

#include "../lib/pushbutton.h"

#define CUNC_USAGE "[OPTIONS]\n"
#define CUNC_DEFAULT_ADDR "localhost"

class MainWidget : public QMainWindow
{
  Q_OBJECT
 public:
  MainWidget(QWidget *parent=0);
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
  void errorData(QAbstractSocket::SocketError err);

 protected:
  void resizeEvent(QResizeEvent *e);

 private:
  void ProcessCommand(const QString &msg);
  void SendCommand(const QString &msg);
  PushButton *cunc_enter_button;
  PushButton *cunc_exit_button;
  PushButton *cunc_dump_button;
  QLCDNumber *cunc_delay_lcd;
  QTcpSocket *cunc_socket;
  QString cunc_buffer;
  unsigned cunc_delay_id;
  QTimer *cunc_dump_timer;
  bool cunc_list_delays;
  unsigned cunc_delay_quantity;
  char cunc_delimiter;
};


#endif  // CUNCCLIENT_H

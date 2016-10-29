// jackdelay.h
//
// JACK-based audio delay.
//
//   (C) Copyright 2011 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: jackdelay.h,v 1.7 2012/07/26 18:39:15 cvs Exp $
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

#ifndef JACKDELAY_H
#define JACKDELAY_H

#include <vector>

#include <jack/jack.h>
#include <jack/ringbuffer.h>
#include <soundtouch/SoundTouch.h>

#include <qtimer.h>

#include <ringbuffer.h>

#include "delay.h"

#define JACKDELAY_CLIENT_NAME "jackdelay"
#define JACKDELAY_DEFAULT_AUDIO_CHANNELS 2
#define JACKDELAY_DEFAULT_DUMP_PERCENTAGE 50
#define JACKDELAY_MAX_AUDIO_CHANNELS 2
#define JACKDELAY_DEFAULT_MAX_DELAY 10000
#define JACKDELAY_TEMPO_RATIO 0.9

class JackDelay : public Delay
{
 Q_OBJECT
 public:
  JackDelay(Profile *p,int id,bool debug,QObject *parent=0,const char *name=0);
  ~JackDelay();
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
  void scanTimerData();

 private:
   void FreeResources();
   friend int JackProcessCB(jack_nframes_t nframes, void *arg);
   friend void JackShutdownCB(void *arg);
   friend void JackPortConnectCB(jack_port_id_t a,jack_port_id_t b,
				 int connect,void *arg);
   jack_client_t *jd_client;
   jack_nframes_t jd_samplerate;
   jack_nframes_t jd_buffersize;
   Cunctator::DelayState jd_state;
   unsigned jd_delay_length;
   unsigned jd_audio_channels;
   unsigned jd_max_delay;
   unsigned jd_dump_percentage;
   jack_nframes_t jd_dump_frames;
   jack_nframes_t jd_target_frames;
   float jd_tempo;
   soundtouch::SoundTouch *jd_touch;
   Ringbuffer *jd_ring;
   std::vector<jack_port_t *> jd_input_ports;
   std::vector<jack_port_t *> jd_output_ports;
   std::vector<QString> jd_input_names;
   std::vector<QString> jd_output_names;
   jack_default_audio_sample_t *jd_buffer;
   QTimer *jd_scan_timer;

   // Communications Flags
   bool jd_enter;
   bool jd_exit;
   bool jd_dump;
   bool jd_active_on_start;
   Cunctator::DelayState jd_from_state;
   unsigned jd_from_delay_length;
};


#endif  // JACKDELAY_H

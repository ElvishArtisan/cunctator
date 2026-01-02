// asihpidelay.h
//
// Audioscience HPI audio delay.
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

#ifndef ASIHPIDELAY_H
#define ASIHPIDELAY_H

#include <asihpi/hpi.h>
#include <soundtouch/SoundTouch.h>

#include <QList>
#include <QStringList>
#include <QTimer>

#include "delay.h"
#include "ringbuffer.h"

#define ASIHPIDELAY_CLIENT_NAME "asihpidelay"
#define ASIHPIDELAY_DEFAULT_ADAPTER_INDEX 1
#define ASIHPIDELAY_DEFAULT_INPUT_PORT 0
#define ASIHPIDELAY_DEFAULT_OUTPUT_PORT 0
#define ASIHPIDELAY_DEFAULT_AUDIO_CHANNELS 2
#define ASIHPIDELAY_DEFAULT_SAMPLE_RATE 48000
#define ASIHPIDELAY_DEFAULT_DUMP_PERCENTAGE 50
#define ASIHPIDELAY_DEFAULT_DELAY_CHANGE_PERCENT 5
#define ASIHPIDELAY_MAX_AUDIO_CHANNELS 2
#define ASIHPIDELAY_POLLING_INTERVAL 10  // milliseconds
#define ASIHPIDELAY_DEFAULT_MAX_DELAY 10000
#define ASIHPIDELAY_TEMPO_RATIO 0.9

class AsihpiDelay : public Delay
{
 Q_OBJECT
 public:
  AsihpiDelay(Profile *p,int id,bool debug,QObject *parent=0);
  ~AsihpiDelay();
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
  QString HpiErrorText(uint16_t hpi_err) const;
  QString HpiStateText(uint16_t state) const;
  unsigned d_adapter_index;
  unsigned d_adapter_input_port;
  hpi_handle_t d_input_stream;
  unsigned d_adapter_output_port;
  hpi_handle_t d_output_stream;
  unsigned d_samplerate;
  unsigned d_buffersize;
  Cunctator::DelayState d_state;
  unsigned d_delay_length;
  unsigned d_audio_channels;
  unsigned d_max_delay;
  unsigned d_dump_percentage;
  unsigned d_dump_frames;
  float d_change_down;
  float d_change_up;
  unsigned d_target_frames;
  //  soundtouch::SoundTouch *d_touch;
  //  Ringbuffer *d_ring;
  pthread_t d_audio_thread;
  //   QList<jack_port_t *> jd_input_ports;
  //   QList<jack_port_t *> jd_output_ports;
  //   QList<QString> jd_input_names;
  //   QStringList jd_output_names;
  //   jack_default_audio_sample_t *jd_buffer;
  QTimer *d_scan_timer;

  // HPI Stuff
  struct hpi_format d_hpi_format;
  short d_hpi_gain_on[HPI_MAX_CHANNELS];
  short d_hpi_gain_off[HPI_MAX_CHANNELS];
  hpi_handle_t d_hpi_mixer;
  // Communications Flags
  bool d_enter;
  bool d_exit;
  bool d_dump;
  bool d_active_on_start;
  bool d_terminating;
  Cunctator::DelayState d_from_state;
  unsigned d_from_delay_length;
};


#endif  // ASIHPIDELAY_H

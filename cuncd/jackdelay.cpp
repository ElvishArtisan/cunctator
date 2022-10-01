// jackdelay.cpp
//
// JACK-based audio delay.
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

#include <stdlib.h>
#include <syslog.h>
#include <errno.h>

#include "jackdelay.h"

// #define JACKDELAY_SHOW_PORT_CONNECTIONS

//
// JACK Callbacks
//
int JackProcessCB(jack_nframes_t nframes, void *arg)
{
  jack_default_audio_sample_t *in[JACKDELAY_MAX_AUDIO_CHANNELS];
  jack_default_audio_sample_t *out[JACKDELAY_MAX_AUDIO_CHANNELS];
  JackDelay *dvr=(JackDelay *)arg;
  unsigned n=0;

  //
  // Process Dump
  //
  if(dvr->jd_dump) {
    if(dvr->jd_ring->dump(dvr->jd_dump_frames)>0) {
      dvr->jd_enter=true;
    }
    dvr->jd_dump=false;
  }

  //
  // Process Delay Startup
  //
  if(dvr->jd_target_frames>dvr->jd_ring->readSpace()) {
    if(dvr->jd_enter) {
      dvr->jd_touch->setTempo(JACKDELAY_TEMPO_RATIO);
      dvr->jd_tempo=JACKDELAY_TEMPO_RATIO;
      dvr->jd_from_state=Cunctator::StateEntering;
      dvr->jd_enter=false;
    }
  }
  else {
    if(dvr->jd_from_state==Cunctator::StateEntering) {
      dvr->jd_touch->setTempo(1.0);
      dvr->jd_tempo=1.0;
      dvr->jd_from_state=Cunctator::StateEntered;
    }
  }
 
  //
  // Process Delay Shutdown
  //
  if(2*dvr->jd_buffersize>dvr->jd_ring->readSpace()) {
    if(dvr->jd_from_state==Cunctator::StateExiting) {
      dvr->jd_touch->setTempo(1.0);
      dvr->jd_tempo=1.0;
      dvr->jd_from_state=Cunctator::StateExited;
    }
  }
  else {
    if(dvr->jd_exit) {
      dvr->jd_touch->setTempo(1.0/JACKDELAY_TEMPO_RATIO);
      dvr->jd_tempo=1.0/JACKDELAY_TEMPO_RATIO;
      dvr->jd_from_state=Cunctator::StateExiting;
      dvr->jd_exit=false;
    }
  }

  //
  // Update Current Delay Length
  //
  dvr->jd_from_delay_length=10*dvr->jd_ring->readSpace()/dvr->jd_samplerate*100;

  //
  // Get JACK Ports
  //
  for(unsigned i=0;i<dvr->jd_audio_channels;i++) {
    in[i]=(jack_default_audio_sample_t *)
      jack_port_get_buffer(dvr->jd_input_ports[i],nframes);
    out[i]=(jack_default_audio_sample_t *)
      jack_port_get_buffer(dvr->jd_output_ports[i],nframes);
  }

  //
  // Feed Ringbuffer
  //
  for(unsigned i=0;i<dvr->jd_audio_channels;i++) {
    for(unsigned j=0;j<nframes;j++) {
      dvr->jd_buffer[j*dvr->jd_audio_channels+i]=in[i][j];
    }
  }
  dvr->jd_ring->write(dvr->jd_buffer,nframes);

  //
  // Write Output
  //
  n=dvr->jd_ring->read(dvr->jd_buffer,nframes*dvr->jd_tempo);
  dvr->jd_touch->putSamples(dvr->jd_buffer,n);
  if(dvr->jd_touch->numSamples()>=nframes) {
    n=dvr->jd_touch->receiveSamples(dvr->jd_buffer,nframes);
    for(unsigned i=0;i<dvr->jd_audio_channels;i++) {
      for(unsigned j=0;j<n;j++) {
	out[i][j]=dvr->jd_buffer[j*dvr->jd_audio_channels+i];
      }
    }
  }
  else {
    for(unsigned i=0;i<dvr->jd_audio_channels;i++) {
      memset((char *)out[i],0,nframes*sizeof(jack_default_audio_sample_t));
    }
  }

  return 0;      
}


void JackShutdownCB(void *arg)
{
  exit (1);
}


JackDelay::JackDelay(Profile *p,int id,bool debug,QObject *parent)
  : Delay(p,id,debug,parent)
{
  jd_client=NULL;
  jd_state=Cunctator::StateExited;
  jd_from_state=Cunctator::StateExited;
  jd_target_frames=0;
  jd_buffer=NULL;
  jd_delay_length=0;
  jd_from_delay_length=0;
  jd_enter=false;
  jd_exit=false;
  jd_dump=false;

  //
  // Scan Timer
  //
  jd_scan_timer=new QTimer(this);
  QObject::connect(jd_scan_timer,SIGNAL(timeout()),this,SLOT(scanTimerData()));

  //
  // Load Driver Settings
  //
  QString section=QString::asprintf("Delay%d",id+1);
  jd_audio_channels=
    p->intValue(section,"AudioChannels",JACKDELAY_DEFAULT_AUDIO_CHANNELS);
  jd_max_delay=
    p->intValue(section,"DefaultMaxDelay",JACKDELAY_DEFAULT_MAX_DELAY);
  jd_dump_percentage=
    p->intValue(section,"DumpPercentage",JACKDELAY_DEFAULT_DUMP_PERCENTAGE);
  jd_active_on_start=
    p->boolValue(section,"ActiveOnStart",false);
  unsigned cnt=0;
  bool ok=false;
  QString inname=
    p->stringValue(section,QString::asprintf("JackSource%u",cnt+1),"",&ok);
  while(ok) {
    jd_input_names.push_back(inname);
    jd_output_names.push_back(p->stringValue(section,
	       QString::asprintf("JackDestination%u",cnt+1),"",&ok));
    cnt++;
    inname=
      p->stringValue(section,QString::asprintf("JackSource%u",cnt+1),"",&ok);
  }
}


JackDelay::~JackDelay()
{
  FreeResources();
  delete jd_scan_timer;
}


Cunctator::DelayType JackDelay::type()
{
  return Cunctator::TypeJackDelay;
}


QString JackDelay::description()
{
  return QString("JACK Delay");
}


Cunctator::DelayState JackDelay::state()
{
  return jd_state;
}


int JackDelay::delayLength()
{
  return jd_delay_length;
}


bool JackDelay::connect()
{
  jack_status_t status;
  int err;

  //
  // Sanity Check
  //
  if((jd_audio_channels<1)||(jd_audio_channels>JACKDELAY_MAX_AUDIO_CHANNELS)) {
    syslog(LOG_ERR,"invalid AudioChannels parameter");
    return false;
  }

  //
  // Initialize Ringbuffer
  //
  jd_ring=new Ringbuffer(16777216,jd_audio_channels);

  //
  // Initialize JACK
  //
  if((jd_client=
      jack_client_open(QString::asprintf("%s_%u",JACKDELAY_CLIENT_NAME,id()+1).
		       toUtf8(),
		       JackNullOption,&status,0))==NULL) {
    if((status&JackServerFailed)!=0) {
      syslog(LOG_ERR,"Unable to connect to the JACK server");
    }
    FreeResources();
    return false;
  }
  jack_set_process_callback(jd_client,JackProcessCB,this);
  jack_on_shutdown(jd_client,JackShutdownCB,0);

  jd_samplerate=jack_get_sample_rate(jd_client);
  jd_buffersize=jack_get_buffer_size(jd_client);
  jd_buffer=new jack_default_audio_sample_t[4*jd_buffersize*jd_audio_channels];
  if((jd_samplerate<8000)||(jd_samplerate>48000)) {
    FreeResources();
    syslog(LOG_ERR,"unsupported JACK samplerate");
    return false;
  }
  jd_dump_frames=jd_max_delay*jd_dump_percentage/100*jd_samplerate/1000;

  //
  // Initialize SoundTouch
  //
  jd_tempo=1.0;
  jd_touch=new soundtouch::SoundTouch();
  jd_touch->setRate(1.0);
  jd_touch->setTempo(1.0);
  jd_touch->setPitch(1.0);
  jd_touch->setChannels(jd_audio_channels);
  jd_touch->setSampleRate(jd_samplerate);

  //
  // Create JACK Ports
  //
  jack_port_t *port=NULL;
  for(unsigned i=0;i<jd_audio_channels;i++) {
    if((port=jack_port_register(jd_client,
				QString::asprintf("input_%u",i+1).toUtf8(),
				JACK_DEFAULT_AUDIO_TYPE,
				JackPortIsInput,0))==NULL) {
      syslog(LOG_ERR,"no more JACK ports available");
      jack_client_close(jd_client);
      jd_client=NULL;
      FreeResources();
      return false;
    }
    jd_input_ports.push_back(port);

    if((port=jack_port_register(jd_client,
				QString::asprintf("output_%u",i+1).toUtf8(),
				JACK_DEFAULT_AUDIO_TYPE,
				JackPortIsOutput,0))==NULL) {
      syslog(LOG_ERR,"no more JACK ports available");
      jack_client_close(jd_client);
      jd_client=NULL;
      FreeResources();
      return false;
    }
    jd_output_ports.push_back(port);
  }

  //
  // Fire Up JACK
  //
  if(jack_activate(jd_client)!=0) {
    syslog(LOG_ERR,"unable to activate client");
    jack_client_close(jd_client);
    jd_client=NULL;
    FreeResources();
    return false;
  }

  //
  // Connect Ports
  //
  for(int i=0;i<jd_input_names.size();i++) {
    if((err=jack_connect(jd_client,jd_input_names[i].toUtf8(),
			 jd_output_names[i].toUtf8()))!=0) {
      if(err!=EEXIST) {
	syslog(LOG_WARNING,
	      "unable to connect source port \"%s\" to destination port \"%s\"",
	      jd_input_names[i].toUtf8().constData(),
	      jd_output_names[i].toUtf8().constData());
      }
    }
  }

  jd_scan_timer->start(100);

  return true;
}


void JackDelay::bypass()
{
}


void JackDelay::enter()
{
  jd_target_frames=jd_max_delay*jd_samplerate/1000;
  if((jd_state!=Cunctator::StateEntering)&&
     (jd_state!=Cunctator::StateEntered)) {
    jd_enter=true;
  }
}


void JackDelay::exit()
{
  jd_target_frames=0;
  if((jd_state!=Cunctator::StateExiting)&&
     (jd_state!=Cunctator::StateExited)) {
    jd_exit=true;
  }
}


void JackDelay::dump()
{
  jd_dump=true;
  emit dumped(id());
}


void JackDelay::scanTimerData()
{
  if(jd_active_on_start) {
    enter();
    jd_active_on_start=false;
  }
  if((jd_from_state!=jd_state)||(jd_from_delay_length!=jd_delay_length)) {
    jd_state=jd_from_state;
    jd_delay_length=jd_from_delay_length;
    emit delayStateChanged(id(),jd_state,jd_delay_length);
  }
}


void JackDelay::FreeResources()
{
  jd_scan_timer->stop();

  //
  // Free JACK
  //
  if(jd_client!=NULL) {
    jack_client_close(jd_client);
    jd_client=NULL;
  }
  if(jd_buffer!=NULL) {
    delete jd_buffer;
    jd_buffer=NULL;
  }

  //
  // Free RingBuffer
  //
  if(jd_ring!=NULL) {
    delete jd_ring;
    jd_ring=NULL;
  }

  //
  // Free SoundTouch Instances
  //
  if(jd_touch!=NULL) {
    delete jd_touch;
    jd_touch=NULL;
  }
}

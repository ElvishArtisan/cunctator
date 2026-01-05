// asihpidelay.cpp
//
// ASIHPI-based audio delay.
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

#include <errno.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

//#include "hpisoundcard.h"

#include "asihpidelay.h"

AsihpiDelay::AsihpiDelay(Profile *p,int id,bool debug,QObject *parent)
  : Delay(p,id,debug,parent)
{
  d_state=Cunctator::StateExited;
  d_from_state=Cunctator::StateExited;
  d_target_frames=0;
  d_delay_length=0;
  d_from_delay_length=0;
  d_enter=false;
  d_exit=false;
  d_dump=false;
  d_terminating=false;
  //  d_to_state=Cunctator::StateBypassed;
  //  d_delay_dump=false;
  d_tempo_ratio=1.0;

  //
  // Scan Timer
  //
  d_scan_timer=new QTimer(this);
  QObject::connect(d_scan_timer,SIGNAL(timeout()),this,SLOT(scanTimerData()));

  //
  // Load Driver Settings
  //
  QString section=QString::asprintf("Delay%d",id+1);
  d_audio_channels=
    p->intValue(section,"AudioChannels",ASIHPIDELAY_DEFAULT_AUDIO_CHANNELS);
  d_max_delay=
    p->intValue(section,"DefaultMaxDelay",ASIHPIDELAY_DEFAULT_MAX_DELAY);
  d_dump_percentage=
    p->intValue(section,"DumpPercentage",ASIHPIDELAY_DEFAULT_DUMP_PERCENTAGE);
  d_active_on_start=
    p->boolValue(section,"ActiveOnStart",false);
  d_adapter_index=
    p->intValue(section,"AdapterIndex",ASIHPIDELAY_DEFAULT_ADAPTER_INDEX)-1;
  d_adapter_input_port=
    p->intValue(section,"InputPort",ASIHPIDELAY_DEFAULT_INPUT_PORT);
  d_adapter_output_port=
    p->intValue(section,"OutputPort",ASIHPIDELAY_DEFAULT_OUTPUT_PORT);
  d_audio_channels=
    p->intValue(section,"AudioChannels",ASIHPIDELAY_DEFAULT_AUDIO_CHANNELS);
  d_samplerate=
    p->intValue(section,"SampleRate",ASIHPIDELAY_DEFAULT_SAMPLE_RATE);
  d_max_delay=
    p->intValue(section,"DefaultMaxDelay",ASIHPIDELAY_DEFAULT_MAX_DELAY);
  d_dump_percentage=
    p->intValue(section,"DumpPercentage",ASIHPIDELAY_DEFAULT_DUMP_PERCENTAGE);
  d_xfer_bytes=d_samplerate*d_audio_channels*
    sizeof(float)*ASIHPIDELAY_POLLING_INTERVAL/1000;
  d_xfer_frames=d_samplerate*ASIHPIDELAY_POLLING_INTERVAL/1000;
  float ratio=
    (float)(p->intValue(section,"DelayChangePercent",
			ASIHPIDELAY_DEFAULT_DELAY_CHANGE_PERCENT))/100.0;
  float ratio_up=1.0-ratio;
  float ratio_down=1.0/ratio_up;

  d_xfer_bytes_up=(((unsigned)(d_xfer_bytes*(ratio_up))/8)*8);
  d_xfer_frames_up=d_xfer_frames*(ratio_up);

  d_xfer_bytes_down=(((unsigned)(d_xfer_bytes*(ratio_down))/8)*8);
  d_xfer_frames_down=d_xfer_frames*(ratio_down);

  d_timescale_up=HPI_OSTREAM_TIMESCALE_UNITS*(ratio_down);
  d_timescale_down=HPI_OSTREAM_TIMESCALE_UNITS*(ratio_up);

  d_ring=new Ringbuffer(16777216,d_audio_channels);

  d_dump_frames=d_max_delay*d_dump_percentage/100*d_samplerate/1000;
}


AsihpiDelay::~AsihpiDelay()
{
  uint16_t hpi_err;

  //
  // Shutdown HPI
  //
  if((hpi_err=HPI_OutStreamStop(NULL,d_output_stream))!=0) {
    syslog(LOG_ERR,"Delay%d: failed to stop stream for output port %d [%s]",
	   1+id(),d_adapter_output_port,
	   HpiErrorText(hpi_err).toUtf8().constData());
  }
  if((hpi_err=HPI_OutStreamHostBufferFree(NULL,d_output_stream))!=0) {
    syslog(LOG_ERR,
	   "Delay%d: failed to free host buffer for output port %d [%s]",
	   1+id(),d_adapter_output_port,
	   HpiErrorText(hpi_err).toUtf8().constData());
  }
  if((hpi_err=HPI_OutStreamClose(NULL,d_output_stream))!=0) {
    syslog(LOG_ERR,
	   "Delay%d: failed to close stream status for output port %d [%s]",
	   1+id(),d_adapter_output_port,
	   HpiErrorText(hpi_err).toUtf8().constData());
  }
  if((hpi_err=HPI_InStreamStop(NULL,d_input_stream))!=0) {
    syslog(LOG_ERR,"Delay%d: failed to stop stream for input port %d [%s]",
	   1+id(),d_adapter_input_port,
	   HpiErrorText(hpi_err).toUtf8().constData());
  }
  if((hpi_err=HPI_InStreamHostBufferFree(NULL,d_input_stream))!=0) {
    syslog(LOG_ERR,
	   "Delay%d: failed to free host buffer for input port %d [%s]",
	   1+id(),d_adapter_input_port,
	   HpiErrorText(hpi_err).toUtf8().constData());
  }
  if((hpi_err=HPI_InStreamClose(NULL,d_input_stream))!=0) {
    syslog(LOG_ERR,"Delay%d: failed to close stream for input port %d [%s]",
	   1+id(),d_adapter_input_port,
	   HpiErrorText(hpi_err).toUtf8().constData());
  }
  if((hpi_err=HPI_MixerClose(NULL,d_hpi_mixer))!=0) {
    syslog(LOG_ERR,"Delay%d:failed to close mixer [%s]",
	   1+id(),HpiErrorText(hpi_err).toUtf8().constData());
  }

  delete d_ring;
}


Cunctator::DelayType AsihpiDelay::type()
{
  return Cunctator::TypeAsihpiDelay;
}


QString AsihpiDelay::description()
{
  return QString("ASIHPI Delay");
}


Cunctator::DelayState AsihpiDelay::state()
{
  return d_state;
}


int AsihpiDelay::delayLength()
{
  return d_delay_length;
}


bool AsihpiDelay::connect()
{
  uint16_t hpi_out_streams;
  uint16_t hpi_in_streams;
  hpi_handle_t hpi_control;
  uint16_t hpi_type;
  uint32_t hpi_serial;
  uint16_t hpi_version;
  uint32_t hpi_bufsize=0;
  uint16_t hpi_err;

  //
  // Sanity Checks
  //
  if((d_audio_channels<1)||(d_audio_channels>ASIHPIDELAY_MAX_AUDIO_CHANNELS)) {
    syslog(LOG_ERR,"Delay%d: invalid AudioChannels parameter",1+id());
    return false;
  }

  //
  // Open HPI Adapter
  //
  for(int i=0;i<HPI_MAX_CHANNELS;i++) {
    d_hpi_gain_on[i]=0;
    d_hpi_gain_off[i]=HPI_GAIN_OFF;
  }
  if((hpi_err=HPI_AdapterOpen(NULL,d_adapter_index))!=0) {
    syslog(LOG_ERR,"Delay%d: no ASI adapter found at index %d",
	   1+id(),1+d_adapter_index);
    return false;
  }
  if((hpi_err=HPI_AdapterGetInfo(NULL,d_adapter_index,
				 &hpi_out_streams,
				 &hpi_in_streams,
				 &hpi_version,
				 &hpi_serial,
				 &hpi_type))!=0) {
    syslog(LOG_ERR,"Delay%d: failed to read adapter info [%s]",
	   1+id(),HpiErrorText(hpi_err).toUtf8().constData());
    return false;
  }
  if((hpi_err=HPI_MixerOpen(NULL,d_adapter_index,&d_hpi_mixer))!=0) {
    syslog(LOG_ERR,"Delay%d: failed to open adapter mixer [%s]",
	   1+id(),HpiErrorText(hpi_err).toUtf8().constData());
    return false;;
  }
  syslog(LOG_INFO,
    "Delay%d: using AudioScience %04X adapter [s/n %u] at adapter index %d",
	 1+id(),hpi_type,hpi_serial,1+d_adapter_index);

  //
  // Intialize Input
  //
  memset(&d_hpi_format,0,sizeof(d_hpi_format));
  if((hpi_err=HPI_FormatCreate(&d_hpi_format,d_audio_channels,
			       HPI_FORMAT_PCM32_FLOAT,
			       d_samplerate,0,0))!=0) {
    syslog(LOG_ERR,"Delay%d: failed to allocate format buffer at index %d [%s]",
	   1+id(),1+d_adapter_index,HpiErrorText(hpi_err).toUtf8().constData());
    return false;
  }
  if((hpi_err=HPI_StreamEstimateBufferSize(&d_hpi_format,
					    2*ASIHPIDELAY_POLLING_INTERVAL,
					   &hpi_bufsize))!=0) {
    hpi_bufsize=0;
  }
  if((hpi_err=HPI_InStreamOpen(NULL,d_adapter_index,
			       d_adapter_input_port,
			       &d_input_stream))!=0) {
    syslog(LOG_ERR,"Delay%d: unable to open stream for input port %d [%s]",
	   1+id(),d_adapter_input_port,
	   HpiErrorText(hpi_err).toUtf8().constData());
    return false;
  }
  if((hpi_err=HPI_InStreamSetFormat(NULL,d_input_stream,&d_hpi_format))!=0) {
    syslog(LOG_ERR,"Delay%d: unable to set audio format for input port %d [%s]",
	   1+id(),d_adapter_input_port,
	   HpiErrorText(hpi_err).toUtf8().constData());
    return false;
  }
  if(hpi_bufsize>0) {
    if((hpi_err=HPI_InStreamHostBufferAllocate(NULL,d_input_stream,hpi_bufsize))==
       HPI_ERROR_INVALID_DATASIZE) {
      syslog(LOG_DEBUG,
	     "Delay%d: unable to enable bus mastering for input port %d",
	     1+id(),d_adapter_input_port);
    }
  }
  syslog(LOG_DEBUG,"Delay%d: allocated %u bytes for input stream buffer",
	 1+id(),hpi_bufsize);

  // Configure Input Mixer Here!
  if((hpi_err=HPI_InStreamStart(NULL,d_input_stream))!=0) {
    syslog(LOG_ERR,"Delay%d: failed to start stream for input port %d [%s]",
	   1+id(),d_adapter_input_port,
	   HpiErrorText(hpi_err).toUtf8().constData());
    return false;
  }

  //
  // Initialize Output
  //
  for(int i=0;i<hpi_out_streams;i++) {
    fflush(stdout);
    if((hpi_err=HPI_OutStreamOpen(NULL,d_adapter_index,i,&d_output_stream))==0) {
      if((hpi_err=HPI_OutStreamSetTimeScale(NULL,d_output_stream,
					    HPI_OSTREAM_TIMESCALE_PASSTHROUGH))!=0) {
	syslog(LOG_ERR,"Delay%d: failed to initialize adapter timescaling [%s]",
	       1+id(),HpiErrorText(hpi_err).toUtf8().constData());
	return false;
      }
      for(uint16_t j=0;j<HPI_MAX_STREAMS;j++) {
	if((hpi_err=HPI_MixerGetControl(NULL,d_hpi_mixer,
					HPI_SOURCENODE_OSTREAM,i,
					HPI_DESTNODE_LINEOUT,j,
					HPI_CONTROL_VOLUME,
					&hpi_control))==0) {
	  if(j==d_adapter_output_port) {
	    hpi_err=HPI_VolumeSetGain(NULL,hpi_control,d_hpi_gain_on);
	    // printf("found control at %d => %d: ON\n",i,j);
	  }
	  else {
	    hpi_err=HPI_VolumeSetGain(NULL,hpi_control,d_hpi_gain_off);
	    // printf("found control at %d => %d: OFF\n",i,j);
	  }
	}
      }
      break;
    }
  }
  if(d_output_stream==0) {
    syslog(LOG_ERR,"Delay%d: no available output streams",1+id());
    ::exit(1);
  }
  if((hpi_err=HPI_OutStreamSetFormat(NULL,d_output_stream,&d_hpi_format))!=0) {
    syslog(LOG_ERR,
	   "Delay%d: unable to set audio format for output port %d [%s]",
	   1+id(),d_adapter_output_port,
	   HpiErrorText(hpi_err).toUtf8().constData());
    ::exit(1);
  }
  if(hpi_bufsize>0) {
    if((hpi_err=HPI_OutStreamHostBufferAllocate(NULL,d_output_stream,hpi_bufsize))==
       HPI_ERROR_INVALID_DATASIZE) {
      syslog(LOG_DEBUG,
	     "Delay%d: unable to enable bus mastering for output port %d",
	     1+id(),d_adapter_output_port);
    }
  }
  syslog(LOG_DEBUG,"Delay%d: allocated %u bytes for output stream buffer",
	 1+id(),hpi_bufsize);

  if((hpi_err=HPI_InStreamStart(NULL,d_input_stream))!=0) {
    syslog(LOG_ERR,"Delay%d: unable to start stream for input port %d [%s]",
	   1+id(),d_adapter_input_port,
	   HpiErrorText(hpi_err).toUtf8().constData());
    return false;
  }

  d_scan_timer->start(ASIHPIDELAY_POLLING_INTERVAL);

  if(d_active_on_start) {
    enter();
  }

  emit delayStateChanged(id(),d_state,0);

  return true;
}


void AsihpiDelay::bypass()
{
}


void AsihpiDelay::enter()
{
  d_target_frames=d_max_delay*d_samplerate/1000;
  if((d_state!=Cunctator::StateEntering)&&
     (d_state!=Cunctator::StateEntered)) {
    d_state=Cunctator::StateEntering;
    emit delayStateChanged(id(),d_state,1000*d_ring->readSpace()/d_samplerate);
  }
}


void AsihpiDelay::exit()
{
  d_target_frames=0;
  if((d_state!=Cunctator::StateExiting)&&
     (d_state!=Cunctator::StateExited)) {
    d_state=Cunctator::StateExiting;
    emit delayStateChanged(id(),d_state,1000*d_ring->readSpace()/d_samplerate);
  }
}


void AsihpiDelay::dump()
{
  if(d_ring->dump(d_dump_frames)>0) {
    enter();
  }
  emit dumped(id());
}


void AsihpiDelay::scanTimerData()
{
  uint16_t hpi_err;
  uint8_t pcm[d_xfer_bytes_down];  // THIS IS A HACK!!
  uint16_t in_state;
  uint32_t in_buffer_size;
  uint32_t in_data_recorded;
  uint32_t in_samples_recorded;
  uint32_t in_aux_data_recorded;
  uint16_t out_state;
  uint32_t out_buffer_size;
  uint32_t out_data_to_play;
  uint32_t out_samples_played;
  uint32_t out_aux_data_to_play;
  bool emit_update=false;
  unsigned current_delay_frames=d_ring->readSpace();

  if((hpi_err=HPI_InStreamGetInfoEx(NULL,d_input_stream,&in_state,
				    &in_buffer_size,&in_data_recorded,
				    &in_samples_recorded,
				    &in_aux_data_recorded))!=0) {
    syslog(LOG_ERR,
	   "Delay%d: failed to read stream status for input port %d [%s]",
	   1+id(),d_adapter_input_port,
	   HpiErrorText(hpi_err).toUtf8().constData());
    return;
  }
  while(in_data_recorded>=d_xfer_bytes) {
    //
    // Read from input
    //
    if((hpi_err=HPI_InStreamReadBuf(NULL,d_input_stream,pcm,d_xfer_bytes))!=0) {
      syslog(LOG_WARNING,
	     "Delay%d: failed to read %lu frames from input %d [%s]",
	     1+id(),d_xfer_bytes/(d_audio_channels*sizeof(float)),
	     d_adapter_output_port,
	     HpiErrorText(hpi_err).toUtf8().constData());
      break;
    }
    d_ring->write((float *)pcm,d_xfer_frames);
    in_data_recorded-=d_xfer_bytes;

    //
    // Select output parameters
    //
    uint32_t xfer_bytes=d_xfer_bytes;
    uint32_t xfer_frames=d_xfer_frames;
    uint32_t timescale=HPI_OSTREAM_TIMESCALE_PASSTHROUGH;
    switch(d_state) {
    case Cunctator::StateEntering:
      if(d_ring->readSpace()<d_target_frames) {
	xfer_bytes=d_xfer_bytes_up;
	xfer_frames=d_xfer_frames_up;
	timescale=d_timescale_up;
      }
      else {
	d_state=Cunctator::StateEntered;
	emit_update=true;
      }
      break;

    case Cunctator::StateEntered:
      break;

    case Cunctator::StateExiting:
      if(d_ring->readSpace()>d_xfer_frames_down) {
	xfer_bytes=d_xfer_bytes_down;
	xfer_frames=d_xfer_frames_down;
	timescale=d_timescale_down;
      }
      else {
	d_state=Cunctator::StateExited;
	emit_update=true;
      }
      break;

    case Cunctator::StateExited:
      break;

    case Cunctator::StateBypassed:
      break;

    case Cunctator::StateUnknown:
      break;
    }

    //
    // Write to output
    //
    if((hpi_err=HPI_OutStreamSetTimeScale(NULL,d_output_stream,timescale))!=0) {
      syslog(LOG_ERR,"Delay%d: failed to set adapter timescaling [%s]",
	     1+id(),HpiErrorText(hpi_err).toUtf8().constData());
    }
    d_ring->read((float *)pcm,xfer_frames);
    if((hpi_err=HPI_OutStreamWriteBuf(NULL,d_output_stream,pcm,xfer_bytes,
				      &d_hpi_format))!=0) {
      syslog(LOG_WARNING,
	     "Delay%d: failed to write %lu frames to output %d [%s]",
	     1+id(),xfer_bytes/(d_audio_channels*sizeof(float)),
	     d_adapter_output_port,
	     HpiErrorText(hpi_err).toUtf8().constData());
    }
  }
  if((hpi_err=HPI_OutStreamGetInfoEx(NULL,d_output_stream,&out_state,
				    &out_buffer_size,&out_data_to_play,
				    &out_samples_played,
				    &out_aux_data_to_play))!=0) {
    syslog(LOG_ERR,
	   "Delay%d: failed to read stream status for output port %d [%s]",
	   1+id(),d_adapter_output_port,
	   HpiErrorText(hpi_err).toUtf8().constData());
    return;
  }
  if(out_state!=HPI_STATE_PLAYING) {
    if((hpi_err=HPI_OutStreamStart(NULL,d_output_stream))!=0) {
    syslog(LOG_ERR,"Delay%d: failed to start stream for output port %d [%s]",
	   1+id(),d_adapter_output_port,
	   HpiErrorText(hpi_err).toUtf8().constData());
    }
  }
  if((current_delay_frames!=d_ring->readSpace())||emit_update) {
    emit delayStateChanged(id(),d_state,
			   1000*d_ring->readSpace()/d_samplerate);
  }
}


QString AsihpiDelay::HpiErrorText(uint16_t hpi_err) const
{
  char err_txt[200];

  HPI_GetErrorText(hpi_err,err_txt);

  return QString::fromUtf8(err_txt);
}


QString AsihpiDelay::HpiStateText(uint16_t state) const
{
  QString ret=QObject::tr("Unknown")+QString::asprintf(" [%u]",state);;

  switch(state) {
  case HPI_STATE_STOPPED:
    ret=QObject::tr("STOPPED");
    break;

  case HPI_STATE_PLAYING:
    ret=QObject::tr("PLAYING");
    break;

  case HPI_STATE_RECORDING:
    ret=QObject::tr("RECORDING");
    break;

  case HPI_STATE_DRAINED:
    ret=QObject::tr("DRAINED");
    break;

  case HPI_STATE_SINEGEN:
    ret=QObject::tr("SINEGEN");
    break;

  case HPI_STATE_WAIT:
    ret=QObject::tr("WAIT");
    break;
  }
  return ret;
}

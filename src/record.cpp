/**
 *  @file   record.cpp
 *  @author Weston Nielson <wnielson@github>
 *
 */
#ifdef HAVE_PORTAUDIO_H
#include "config.h"

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <list>
#include <map>
#include <signal.h>

#include <portaudio.h>

#include "Sampler.h"
#include "record.h"

typedef float SAMPLE;
bool ABORT = false;

struct Bit {
  int count;
  int state;  // 1 or 0
};

int quit(int rc) {
  Pa_Terminate();
  exit(rc);
};

static void catch_function(int signal) {
  ABORT = true;
};

int run_record(int argc, char **argv) {
  int                 numInputDevices;
  
	PaError             err;
  PaStreamParameters  inputParameters;
  PaStream*           stream;
  const PaDeviceInfo* deviceInfo;
  
  int*                inputDevices;
  int                 dev_id = -1;
  bool                valid_device = false;
  SAMPLE              sampleBlock[FRAMES_PER_BUFFER];
  Sampler             sampler;
  
  signal(SIGINT, catch_function);
  
	err = Pa_Initialize();
	if (err != paNoError) {
    printf("Error: %s\n", Pa_GetErrorText(err));
		quit(1);
	}
	
  numInputDevices = 0;
  for (int i=0; i < Pa_GetDeviceCount(); i++ ) {
		deviceInfo = Pa_GetDeviceInfo(i);
    if (deviceInfo->maxInputChannels > 0) {
      numInputDevices++;
    }
	}
  
  inputDevices = new int[numInputDevices];
  
  numInputDevices = 0;
  for (int i=0; i < Pa_GetDeviceCount(); i++ ) {
		deviceInfo = Pa_GetDeviceInfo(i);
    if (deviceInfo->maxInputChannels > 0) {
      inputDevices[numInputDevices] = i;
      numInputDevices++;
    }
	}
  
	printf("Available input devices\n");
  
  while (!valid_device) {
    for (int i=0; i < numInputDevices; i++) {
      deviceInfo = Pa_GetDeviceInfo(inputDevices[i]);
      printf("  [%d] %s, input channels: %d\n", inputDevices[i], deviceInfo->name, deviceInfo->maxInputChannels);
    }
    
    printf("Select input device: ");
    scanf("%d", &dev_id);
    
    for (int i=0; i < numInputDevices; i++) {
      if (inputDevices[i] == dev_id) {
        valid_device = true;
        break;
      }
    }
    
    if (!valid_device) {
      printf("Invalid input device; valid choices:\n");
    }
    
  }
  
  printf("Listening on device %d (%s)\n", dev_id, Pa_GetDeviceInfo(dev_id)->name);
  
  inputParameters.device = dev_id;
  inputParameters.channelCount = 1;
  inputParameters.sampleFormat = PA_SAMPLE_TYPE;
  inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
  inputParameters.hostApiSpecificStreamInfo = NULL;
  
  err = Pa_OpenStream(
                      &stream,
                      &inputParameters,
                      NULL,                  /* &outputParameters, */
                      SAMPLE_RATE,
                      FRAMES_PER_BUFFER,
                      paClipOff,            /* we won't output out of range samples so don't bother clipping them */
                      NULL,
                      NULL);
  
  if( err != paNoError ) {
    quit(1);
  }
  
  err = Pa_StartStream( stream );
  if( err != paNoError ) {
    quit(1);
  }
  
  while (!ABORT)
  {
    err = Pa_ReadStream(stream, sampleBlock, FRAMES_PER_BUFFER);
    
    if (err != paNoError)
    {
      if (err == paInputOverflowed) {
        // We can ignore input overflows because we will just
        // discard this data and reset the current code capture
        sampler.rewind();
      } else {
        // This is a more serious error, so just spit out the
        // error and bail
        printf("Error message: %s\n", Pa_GetErrorText(err));
        quit(1);
      }
    }
    
    if (sampler.sample(sampleBlock, FRAMES_PER_BUFFER)) {
      break;
    }
  }
  
  if (ABORT) {
    printf("\rSampling aborted\n");
  }
  
  printf("\nDone recording samples\n");
  
  /* -- Now we stop the stream -- */
  err = Pa_StopStream( stream );
  if( err != paNoError ) {
    quit(1);
  }
  
  /* -- don't forget to cleanup! -- */
  err = Pa_CloseStream( stream );
  if( err != paNoError ) {
    quit(1);
  };
  
	return 0;
};

#endif

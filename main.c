#include "portaudio.h"
#include <math.h>
#include <stdio.h>
#include <time.h>

#define NUM_SECONDS (5)
#define SAMPLE_RATE (48000)
#define FRAMES_PER_BUFFER (64)
#define DEFAULT_FREQ 220.0
#define DEFAULT_AMPLITUDE 0.1
#define DEFAULT_WAVE_POSITION 0.5

#ifndef M_PI
#define M_PI (3.14159265)
#endif

#define TABLE_SIZE (2048)
typedef struct {
  float sine[TABLE_SIZE];
  float saw[TABLE_SIZE];
  char message[20];
  float readPointer;
  float frequency;
  float amplitude;
  float detune;
  float wavePosition;
} paTestData;

static int patestCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags, void *userData) {
  paTestData *data = (paTestData *)userData;
  float *out = (float *)outputBuffer;
  unsigned long i;
  (void)timeInfo; /* Prevent unused variable warnings. */
  (void)statusFlags;
  (void)inputBuffer;

  for (i = 0; i < framesPerBuffer; i++) {
    int WaveTableIndexBelow = floorf(data->readPointer);
    int WaveTableIndexAbove = WaveTableIndexBelow + 1;
    if (WaveTableIndexAbove >= TABLE_SIZE) {
      WaveTableIndexAbove = 0;
    };
    float aboveFraction = data->readPointer - WaveTableIndexBelow;
    float belowFraction = 1.0 - aboveFraction;

    float outValue =
        data->amplitude * (
                          (belowFraction * data->saw[WaveTableIndexBelow] +
                           aboveFraction * data->saw[WaveTableIndexAbove])*(1-data->wavePosition))
                        +
                           ((belowFraction * data->sine[WaveTableIndexBelow] +
                           aboveFraction * data->sine[WaveTableIndexAbove])*data->wavePosition);
    data->readPointer += TABLE_SIZE * data->frequency / SAMPLE_RATE;
    while (data->readPointer >= TABLE_SIZE) {
      data->readPointer -= TABLE_SIZE;
    }
    *out++ =  outValue;/* left */
    *out++ = outValue; /* right */
//    printf("General Out Value: %f, Left Out: %f, Right Out: %f\n", outValue, leftOut, rightOut);
  }

  return paContinue;
}

/*
 * This routine is called by portaudio when playback is done.
 */
static void StreamFinished(void *userData) {
  paTestData *data = (paTestData *)userData;
  printf("Stream Completed: %s\n", data->message);
}

/*******************************************************************/
int main(void);
int main(void) {
  PaStreamParameters outputParameters;
  PaStream *stream;
  PaError err;
  paTestData data;
  int i;

  printf("PortAudio Test: output sine wave. SR = %d, BufSize = %d\n",
         SAMPLE_RATE, FRAMES_PER_BUFFER);

  /* initialise sinusoidal wavetable
   * This could also be where you initizlize other wave types for teh table.
   *
   * Eventually you could offload precalculated values into another file, and
   * make it appendable to add a bunch of different wave types.*/

  for (i = 0; i < TABLE_SIZE; i++) {
    data.sine[i] = (float)sin(((double)i / (double)TABLE_SIZE) * M_PI * 2.);
  }
  float sawCount = 0.0;
  for (i = 0; i < TABLE_SIZE; i++){
    data.saw[i] = sawCount;
    sawCount+=0.001;
    if(sawCount >=1.0f) sawCount -= 2.0f;
  }
  data.readPointer = 0;
  data.frequency = DEFAULT_FREQ;
  data.amplitude = DEFAULT_AMPLITUDE;
  data.wavePosition = DEFAULT_WAVE_POSITION;
  data.detune = 0;
  err = Pa_Initialize();
  if (err != paNoError)
    goto error;

  outputParameters.device =
      Pa_GetDefaultOutputDevice(); /* default output device */
  if (outputParameters.device == paNoDevice) {
    fprintf(stderr, "Error: No default output device.\n");
    goto error;
  }
  outputParameters.channelCount = 2;         /* stereo output */
  outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
  outputParameters.suggestedLatency =
      Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
  outputParameters.hostApiSpecificStreamInfo = NULL;

  err = Pa_OpenStream(&stream, NULL, /* no input */
                      &outputParameters, SAMPLE_RATE, FRAMES_PER_BUFFER,
                      paClipOff, /* we won't output out of range samples so
                                    don't bother clipping them */
                      patestCallback, &data);
  if (err != paNoError)
    goto error;

  sprintf(data.message, "No Message");
  err = Pa_SetStreamFinishedCallback(stream, &StreamFinished);
  if (err != paNoError)
    goto error;

  err = Pa_StartStream(stream);
  if (err != paNoError)
    goto error;
  float count = 0;
  float sweep = 0;
  while (count <20000)
  {
    data.wavePosition = 0.5*((sin(sweep)+1)/2);
    data.frequency+=0.05;
    Pa_Sleep(1);
    sweep+=0.005;
    count++;
    printf("freq: %f\n",data.frequency);
  }

  err = Pa_StopStream(stream);
  if (err != paNoError)
    goto error;

  err = Pa_CloseStream(stream);
  if (err != paNoError)
    goto error;

  Pa_Terminate();
  printf("Test finished.\n");

  return err;
error:
  Pa_Terminate();
  fprintf(stderr, "An error occurred while using the portaudio stream\n");
  fprintf(stderr, "Error number: %d\n", err);
  fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
  return err;
}

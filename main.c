#include "portaudio.h"
#include <math.h>
#include <stdio.h>
#include <time.h>

#define NUM_SECONDS (5)
#define SAMPLE_RATE (48000)
#define FRAMES_PER_BUFFER (64)

#ifndef M_PI
#define M_PI (3.14159265)
#endif

#define TABLE_SIZE (512)
typedef struct {
  float sine[TABLE_SIZE];
  char message[20];
  float readPointer;
  float frequency;
  float amplitude;
  float detune;
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
    //  data->readPointer = TABLE_SIZE * data->frequency/SAMPLE_RATE; // 2.55ish
    //  when the freq is 220, table is 512 and sample rate 44100
    int WaveTableIndexBelow = floorf(data->readPointer);
    int WaveTableIndexAbove = WaveTableIndexBelow + 1;
    if (WaveTableIndexAbove >= TABLE_SIZE) {
      WaveTableIndexAbove = 0;
    };
    float aboveFraction = data->readPointer - WaveTableIndexBelow;
    float belowFraction = 1.0 - aboveFraction;

    float leftOutValue =
        data->amplitude * (belowFraction * data->sine[WaveTableIndexBelow] +
                           aboveFraction * data->sine[WaveTableIndexAbove]);
    float rightOutValue =
        data->amplitude * (belowFraction * data->sine[WaveTableIndexBelow] +
                           aboveFraction * data->sine[WaveTableIndexAbove]);
    data->readPointer += TABLE_SIZE * data->frequency / SAMPLE_RATE;
    while (data->readPointer >= TABLE_SIZE) {
      data->readPointer -= TABLE_SIZE;
    }
    *out++ =  leftOutValue;/* left */
    *out++ = leftOutValue; /* right */
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
  data.readPointer = 0;
  data.frequency = 220;
  data.amplitude = 0.5;
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
  while (count <20000)
  {
  data.frequency+=0.02;
  Pa_Sleep(1);
  count++;
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

/******************************************************************************

 	              Copyright (C) 2006-2015 VoiceKey Inc.
                      Cocoa Beach, Florida 32931
			               All rights reserved

			          Confidential and Proprietary


     Module: mel_filter.h
     Author: Veton Kepuska
    Created: June 29, 2006
    Updated:

Description: A modified version of mel_filter class definition based on 2004
             Veton Kepuska's version.

             Inspired by David Shipman's  model of modular software
             architecture implemented in C.

       $Log:$

*******************************************************************************

            Proprietary and Confidential. All rights reserved.
                VoiceKey Inc., Cocoa Beach, Florida 32931.

******************************************************************************/

#ifndef MEL_FILTER_H
#define MEL_FILTER_H

/******************************  Include Files  ******************************/

#include "common/wuw_config.h"
#include "common/wuw_util.h"
#include "common/wuw_common.h"

/*********************************  Defines  *********************************/
//
// Structure for FFT mel scale windows
// stored as a chained list
//
struct FFT_Mel_Scale_Window {
  int    StartingPoint;
  int    Length;
  float *Data;
  struct FFT_Mel_Scale_Window *Next;
};

/*****************************  Class Definition  ****************************/

class MelFilter {

 public:
  // Default Constructor
  MelFilter ();
  // constructor that initializes:
  //	allocates internal buffers
  MelFilter (MF_Config *mfConfig);
  ~MelFilter ();

  // Function that initializes already exisiting object
  void Init(MF_Config *mfConfig);

  // Function that resets already exisiting object's data members
  // to default values
  void Reset();

  // End Of Stream Function that flushes the buffers after end of the input
  // stream
  FLOAT32 *Eos();

  void AllocMemory();
  void FreeMemory();

  // Computation of Mel Spectrum
  // Input sample buffer of length vec_size
  // Output - mel spectrum
  FLOAT32 *Run (FLOAT32 *in_vector);

  // Debugging Functions
  UINT16 PrintTriangles (WuwLogger *wuwLogger);

 private:
  FLOAT32  starting_frequency;
  FLOAT32  sampling_frequency;
  UINT16   fft_vec_length;
  UINT16   num_mel_filters;

  FFT_Mel_Scale_Window  FirstWin;

  UINT16   in_vector_size;	// Length of input spectral vector
  FLOAT32 *out_mel_spectrum;	// Buffer of output mel-scaled spectrum

  void    ComputeMelScaleWindows(FFT_Mel_Scale_Window *FirstWin,
			       FLOAT32 StFreq, FLOAT32 SmplFreq,
			       UINT16 FFTLength, UINT16 NumChannels);
  void    ComputeTriangle (FFT_Mel_Scale_Window *FirstWin);
  UINT16  MelFilterBank (FLOAT32 *SigFFT, FLOAT32 *SigMelFFT,
		       FFT_Mel_Scale_Window *FirstWin);

};

#endif  // MEL_FILTER_H

/*******************************  End of File  *******************************/


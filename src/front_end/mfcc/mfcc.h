/******************************************************************************

 	              Copyright (C) 2006-2015 VoiceKey Inc.
                           Cocoa Beach, Florida 32931
			      All rights reserved

			  Confidential and Proprietary


     Module: mfcc.h
     Author: Veton Kepuska
    Created: September 15, 2006
    Updated:

Description: mfcc module that computes mel-filtred cepstral coefficients.

             The mfcc module calls mel_filter class and dct class that
             perform mel_filtering and discete cos-ine transformation.

             Software architecture was inspired by David Shipman's  model
             of modular software architecture that originaly David has
             implemented in C and I (Veton Kepuska) have extended and used.

       $Log:$

*******************************************************************************

            Proprietary and Confidential. All rights reserved.
                VoiceKey Inc., Cocoa Beach, Florida 32931.

******************************************************************************/

#ifndef MFCC_H
#define MFCC_H

/******************************  Include Files  ******************************/

// They are included in one of the sub-modules
//#include "wuw_config.h"
//#include "wuw_util.h"
//#include "wuw_common.h"

#include "mel_filter.h"
#include "log.h"
#include "dct.h"

/*********************************  Defines  *********************************/

static UINT16     call_no;
static UINT16     num_calls;

/*****************************  Class Definition  ****************************/

class MFCC
{

 public:
  // Default Constructor
  MFCC ();
  // constructor that initializes:
  //	allocates internal buffers
  MFCC (CfgMFCC* cfgMFCC, char* output_basename);
  ~MFCC ();

  // Function that resets already exisiting object's data members
  // to default values
  void Reset();

  // Function that initializes already exisiting object
  void Init(CfgMFCC* cfgMFCC);

  // End Of Stream Function that flushes the buffers after end of the input
  // stream
  FLOAT32 *Eos();

  // Computation of MFCC coefficients
  // Input buffer vector of spectral values of length vec_size
  // Output - out_mel_spectrum
  FLOAT32 *Run (FLOAT32 *in_vector);
  //
  // Utility Functions
  //
 private:

  UINT16     in_vector_size;	        // Length of input spectral vector
  UINT16     out_vector_size;	        // Length of output mfcc vector
  FLOAT32   *mel_filtered_spectrum;	  // Buffer of output mel-scaled spectrum
  FLOAT32   *log_mel_filtered_spectrum;
  FLOAT32   *out_mfcc_vector;
  //
  // Pointers to member objects
  //
  MelFilter *myMFCC_mel_filter;
  VLOG      *myMFCC_vlog;
  DCT       *myMFCC_dct;

  // Debugging Information
  BOOLEAN         bMonitor;
  WuwLogger      *m_melfilt_logger;
  WuwLogger      *m_dctdata_logger;
  vector<WuwLogger *> m_meldata_logger;
  void            Open_MonitorFiles(CfgMFCC* cfgMFCC);
  void            Run_Monitor();
};

#endif  // MFCC_H

/*******************************  End of File  *******************************/


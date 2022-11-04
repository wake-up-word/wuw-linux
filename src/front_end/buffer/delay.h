/******************************************************************************

 	              Copyright (C) 2006-2015 VoiceKey Inc.
                      Cocoa Beach, Florida 32931
			                All rights reserved

			           Confidential and Proprietary


     Module: delay.h
     Author: Veton Kepuska
    Created: October 10, 2006
    Updated:

Description: Module that implements a delay and buffering of features
             necessary for synchronisation of features computed by
             enhancement module and VAD that classifies feature vectors
             into speech (VAD_ON)/no-speech (VAD_OFF) as well as computes
             spectral estimate of the no-speech/background of the signal.
             This computation introduces delay that needs to be bridged by
             this buffering/delay module.

             Inspired by David Shipman's  model of modular software
             architecture originaly implemented in C.

       $Log:$

*******************************************************************************

            Proprietary and Confidential. All rights reserved.
               VoiceKey Inc., Cocoa Beach, Florida 32931.

*******************************************************************************

 Notes:

******************************************************************************/

#ifndef DELAY_H
#define DELAY_H

/******************************  Include Files  ******************************/

#include "common/wuw_config.h"
#include "common/wuw_util.h"
#include "common/wuw_common.h"

/*********************************  Defines  *********************************/

#define MA_WIN_SIZE_FACTOR  (10)

class Delay {

 public:
  // Default Constructor
  Delay();
  // Constructor that uses configuration parameters to intialize the module
  Delay (CfgDelay *cfgDelay, char* output_basename);
  // Destructor
  ~Delay ();

  // Function that initializes already exisiting object
  void      Init(CfgDelay *cfgDelay);

  // Function that resets already exisiting object's data members
  // to default values
  void      Reset();

  // End Of Stream Function that flushes the buffers after end of the input
  // stream
  FE_FEATURES *Eos();

  //
  // 1. Buffering Original Spectral Vectors needed to Estimate Background Spectrum
  // 2. Buffering MFCC Vectors also Needed to Estimate its Mean
  // 3. Buffering Enhanced Spectral Features
  // 4. Computing Gradients of MFCC and Enhanced Spectral Features
  // 5. Packing features into FE_FEATURES structure
  FE_FEATURES *Run (FLOAT32   *in_spec_vector,
                    FLOAT32   *in_lpc_spec_vector,
                    FLOAT32   *in_mfcc_vector,
                    FLOAT32   *in_lpc_mfcc_vector,
                    FLOAT32   *in_enh_mfcc_vector,
                    FLOAT32    frame_energy,
                    FLOAT32    log2_frame_energy,
                    FLOAT32    log_frame_energy,
                    VAD_States  *vad_states);
//                    VAD_STATE   vad_state);

  // Utility Functions
  void       AllocBuffer();

  FE_FEATURES_STATS *getFeatureStats();
  UINT16     getBufferSize();
  void       setFrameRate(UINT16 frame_rate);

  FLOAT32   *compute_MA_estimate(FLOAT32 *in_vector, UINT16 vec_size, FLOAT32 *ma_vector);
  FLOAT32   *CepstralMeanNormalization(FLOAT32 *in_vector, FLOAT32 *mean_estm);
  //FLOAT32   *CepstralMeanNormalization(FLOAT32 *in_vector, FLOAT32 *out_vector, FLOAT32 *mean_estm);
  FLOAT32   *DC_Filter(FLOAT32 *in_vector, FLOAT32 *f32_prev_xVal, FLOAT32 *f32_prev_yVal, FLOAT32 *out_vector, UINT32 vector_size);

  void       GradientI(FLOAT32 **in_vector, UINT16 index, FLOAT32 *out_vector);
  void       GradientII(FLOAT32 **in_vector, UINT16 index, FLOAT32 *out_vector);

 private:
  UINT32     frame_counter;       // frame counter
  UINT32     ma_counter;          // frame counter
  UINT16     counter;             // local counter

  SINT16     buffer_indx;         // buffer index/pointer
  SINT16     delayed_vec_indx;    // index of the vector in the buffer that is delayed
  SINT16     gradI_indx;          // buffer index/circular that points to current gradI center vector
  SINT16     gradII_indx;         // buffer index/circular that points to current gradII center vector

  SINT16     num_gradI;
  SINT16     num_gradII;

  FLOAT32    nf1;
  FLOAT32    nf2;
  FLOAT32   *sum1;               // temporary space for gradient computation
  FLOAT32   *sum2;               // temporary space for gradient computation

  UINT16     delay;              // internal counter of feature vectors
  UINT16     ma_win_size;        // Moving Average Window Size - independent of delay-buffer size
                                 // For implementation purposes it is best to have it multiple of buffer size
  UINT16     spec_vector_size;	 // Length of input spectral vector
  UINT16     mfcc_vector_size;	 // Length of input spectral vector
  UINT16     enh_mfcc_vector_size;

  FLOAT32   **spec_buffer;        // buffer[delay][spec_vector_size]
  FLOAT32   **lpc_spec_buffer;    // buffer[delay][spec_vector_size]
  FLOAT32   **mfcc_buffer;        // buffer[delay][mfcc_vector_size]
  FLOAT32   **lpc_mfcc_buffer;    // buffer[delay][mfcc_vector_size]
  FLOAT32   **enh_mfcc_buffer;    // buffer[delay][mfcc_vector_size]
  //FLOAT32   **out_mfcc_buffer;    // buffer[delay][mfcc_vector_size]
  //FLOAT32   **out_lpc_mfcc_buffer;// buffer[delay][mfcc_vector_size]
  //FLOAT32   **out_enh_mfcc_buffer;// buffer[delay][mfcc_vector_size]
  FLOAT32    *ma_spec_vector;     // moving average vector estimate
  FLOAT32    *ma_lpc_spec_vector; // moving average vector estimate
  FLOAT32    *ma_mfcc_vector;     // moving average vector estimate
  FLOAT32    *ma_lpc_mfcc_vector; // moving average vector estimate
  FLOAT32    *ma_enh_mfcc_vector; // moving average vector estimate

  FLOAT32   *ma_temp_vector;

  FLOAT32    lambda_MAF;

  FLOAT32*   pf32_xCoeffs;    // denominator coeffs
  FLOAT32*   f32_prev_xVal_mfcc;   //
  FLOAT32*   f32_prev_xVal_lpc_mfcc;   //
  FLOAT32*   f32_prev_xVal_enh_mfcc;   //
  UINT16     i16_num_xCoeffs; // Length of buffer in number of samples

  FLOAT32*   pf32_yCoeffs;	   // DC Filter numerator coeffs
  FLOAT32*   f32_prev_yVal_mfcc;   //
  FLOAT32*   f32_prev_yVal_lpc_mfcc;   //
  FLOAT32*   f32_prev_yVal_enh_mfcc;   //
  UINT16     i16_num_yCoeffs; // Length of buffer in number of samples

  VAD_STATE         vad_state;
  FE_FEATURES_STATS features_stats_str;

  FE_FEATURES output; // Delayed Features Vector

  // Monitoring and Debuging Members
  BOOLEAN     bMonitor;
  WuwLogger  *m_omfcc_logger;
  WuwLogger  *m_nmfcc_logger;
  WuwLogger  *m_mmfcc_logger;
  WuwLogger  *m_oenhmfcc_logger;
  WuwLogger  *m_nenhmfcc_logger;
  WuwLogger  *m_menhmfcc_logger;
  WuwLogger  *m_mef_logger;
  WuwLogger  *m_AGC_aver_volume_logger;
  WuwLogger  *m_AGC_peek_volume_logger;

  void        Run_Monitor();
  void        End_Monitor();
  void        Open_MonitorFiles(CfgDelay *cfgDelay);

  BOOLEAN           first_time;
  AGC_Info_Struct   AGC_info_str;
  FLOAT32           VAD_ON_aver_energy;
  FLOAT32           VAD_ON_peek_energy;

};

#endif

/*******************************  End of File  *******************************/

/******************************************************************************

                         Copyright (C) 2006-2015 VoiceKey Inc.
                              Cocoa Beach, Florida 32931
                                 All rights reserved

                             Confidential and Proprietary

     Module: delay.cpp
     Author: Veton Kepuska
    Created: September 10, 2006
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


/******************************  Include Files  ******************************/

#include <cstdlib>
#include <iostream>
using namespace std;

#include "delay.h"

/*********************************  Defines  *********************************/

#define LAMBDA_SLOWER_MA        (FLOAT32)(0.999)

/****************************  Function Members  *****************************/
//
// Default Constructor
//
Delay::Delay()
{
  lpc_spec_buffer    = NULL;
  mfcc_buffer        = NULL;
  lpc_mfcc_buffer    = NULL;
  enh_mfcc_buffer    = NULL;
  //out_mfcc_buffer    = NULL;
  //out_lpc_mfcc_buffer= NULL;
  //out_enh_mfcc_buffer= NULL;
  ma_spec_vector     = NULL;
  ma_lpc_spec_vector = NULL;
  ma_mfcc_vector     = NULL;
  ma_lpc_mfcc_vector = NULL;
  ma_enh_mfcc_vector = NULL;
  sum1               = NULL;
  sum2               = NULL;

  ma_temp_vector     = NULL;

  Reset();
  //
  // Delay Configuration Parameters
  //
  delay            = max((VAD_ON_MIN_COUNT + VAD_ON_LEAD),      // 10 + 5
                         (VAD_OFF_MIN_COUNT + VAD_OFF_TRAIL));  // 25 + 5

  ma_win_size      = MA_WIN_SIZE_FACTOR*delay;

  spec_vector_size = FE_FFT_SIZE/2;                          //  256/2
  mfcc_vector_size = NUM_DCT_ELEMENTS+1;                     //  13+1
  enh_mfcc_vector_size = NUM_DCT_ELEMENTS+1;                 //  13+1
  num_gradI        = NUM_GRAD1_VECS;
  num_gradII       = NUM_GRAD2_VECS;

  bMonitor         = false;

  /* IIR filter: y[n]=x[n]-x[n-1]+0.999*y[n-1] */
  i16_num_yCoeffs  = 2;
  i16_num_xCoeffs  = 2;

  AllocBuffer();
}
//
// Configurable Constructor
//
Delay::Delay(CfgDelay *cfgDelay, char* output_basename)
{
  spec_buffer        = NULL;
  lpc_spec_buffer    = NULL;
  mfcc_buffer        = NULL;
  lpc_mfcc_buffer    = NULL;
  enh_mfcc_buffer    = NULL;
  //out_mfcc_buffer    = NULL;
  //out_lpc_mfcc_buffer= NULL;
  //out_enh_mfcc_buffer= NULL;
  ma_spec_vector     = NULL;
  ma_lpc_spec_vector = NULL;
  ma_mfcc_vector     = NULL;
  ma_lpc_mfcc_vector = NULL;
  ma_enh_mfcc_vector = NULL;
  sum1               = NULL;
  sum2               = NULL;

  ma_temp_vector     = NULL;

  Reset();
  //
  // Delay Configuration Parameters -- must be set from config file or model
  //
  delay            = cfgDelay->delay;
  ma_win_size      = MA_WIN_SIZE_FACTOR*delay;
  spec_vector_size = cfgDelay->spec_vector_size;
  mfcc_vector_size = cfgDelay->mfcc_vector_size+1;
  enh_mfcc_vector_size = cfgDelay->enh_mfcc_vector_size+1;

  num_gradI        = cfgDelay->num_gradI;
  num_gradII       = cfgDelay->num_gradII;

  /* IIR filter: y[n]=x[n]-x[n-1]+0.999*y[n-1] */
  i16_num_yCoeffs  = 2;
  i16_num_xCoeffs  = 2;

  AllocBuffer();

  // Monitor Functions Must be last
  if (cfgDelay->bMonitor) {
     bMonitor = true;
     strcpy(cfgDelay->output_dir, output_basename);
     Open_MonitorFiles(cfgDelay);
  }
  else {
     bMonitor = false;
  }
}
//
// Buffer Allocation
//
void Delay::AllocBuffer() {
  UINT16 i;
  SINT32 c=0;

  spec_buffer     = new FLOAT32 * [delay];
  lpc_spec_buffer = new FLOAT32 * [delay];
  mfcc_buffer     = new FLOAT32 * [delay];
  lpc_mfcc_buffer = new FLOAT32 * [delay];
  enh_mfcc_buffer = new FLOAT32 * [delay];
  //out_mfcc_buffer     = new FLOAT32 * [delay];
  //out_lpc_mfcc_buffer = new FLOAT32 * [delay];
  //out_enh_mfcc_buffer = new FLOAT32 * [delay];

  for (i=0; i<delay; i++) {
    spec_buffer[i]     = new FLOAT32 [spec_vector_size];
    lpc_spec_buffer[i] = new FLOAT32 [spec_vector_size];
    mfcc_buffer[i]     = new FLOAT32 [3*mfcc_vector_size]; // static, gradient I & II
    lpc_mfcc_buffer[i] = new FLOAT32 [3*mfcc_vector_size]; // static, gradient I & II
    enh_mfcc_buffer[i] = new FLOAT32 [3*mfcc_vector_size]; // static, gradient I & II

    //out_mfcc_buffer[i]     = new FLOAT32 [3*mfcc_vector_size]; // static, gradient I & II
    //out_lpc_mfcc_buffer[i] = new FLOAT32 [3*mfcc_vector_size]; // static, gradient I & II
    //out_enh_mfcc_buffer[i] = new FLOAT32 [3*mfcc_vector_size]; // static, gradient I & II

    memset(spec_buffer[i], c, spec_vector_size*sizeof(FLOAT32));
    memset(lpc_spec_buffer[i], c, spec_vector_size*sizeof(FLOAT32));
    memset(mfcc_buffer[i], c, 3*mfcc_vector_size*sizeof(FLOAT32));
    memset(lpc_mfcc_buffer[i], c, 3*mfcc_vector_size*sizeof(FLOAT32));
    memset(enh_mfcc_buffer[i], c, 3*mfcc_vector_size*sizeof(FLOAT32));

    //memset(out_mfcc_buffer[i], c, 3*mfcc_vector_size*sizeof(FLOAT32));
    //memset(out_lpc_mfcc_buffer[i], c, 3*mfcc_vector_size*sizeof(FLOAT32));
    //memset(out_enh_mfcc_buffer[i], c, 3*mfcc_vector_size*sizeof(FLOAT32));
  }
  // DC Filter
  pf32_xCoeffs    = new FLOAT32 [i16_num_xCoeffs];
  pf32_yCoeffs    = new FLOAT32 [i16_num_yCoeffs];
  f32_prev_xVal_mfcc   = new FLOAT32 [mfcc_vector_size];
  f32_prev_yVal_mfcc   = new FLOAT32 [mfcc_vector_size];
  f32_prev_xVal_lpc_mfcc   = new FLOAT32 [enh_mfcc_vector_size];
  f32_prev_yVal_lpc_mfcc   = new FLOAT32 [enh_mfcc_vector_size];
  f32_prev_xVal_enh_mfcc   = new FLOAT32 [enh_mfcc_vector_size];
  f32_prev_yVal_enh_mfcc   = new FLOAT32 [enh_mfcc_vector_size];

  pf32_yCoeffs[0] =  1.0F;    // This is always one and is not used in filtering
  pf32_yCoeffs[1] =  0.9997F;
  pf32_xCoeffs[0] =  1.0F;
  pf32_xCoeffs[1] = -1.0F;

  // initializing vectors to zero
  memset(f32_prev_xVal_mfcc, c, (mfcc_vector_size)*sizeof(FLOAT32));
  memset(f32_prev_yVal_mfcc, c, (mfcc_vector_size)*sizeof(FLOAT32));
  memset(f32_prev_xVal_lpc_mfcc, c, (mfcc_vector_size)*sizeof(FLOAT32));
  memset(f32_prev_yVal_lpc_mfcc, c, (mfcc_vector_size)*sizeof(FLOAT32));
  memset(f32_prev_xVal_enh_mfcc, c, (enh_mfcc_vector_size)*sizeof(FLOAT32));
  memset(f32_prev_yVal_enh_mfcc, c, (enh_mfcc_vector_size)*sizeof(FLOAT32));

  // Recursive Normalizing constant for Gradient computation
  // Computing Normalizing Factor sum of i^2
  for (i=1, nf1=0.0F; i<=num_gradI; i++) {
    nf1 += (FLOAT32)i*i;
  }
  nf1 *= 2.0F;
  for (i=1, nf2=0.0F; i<=num_gradII; i++) {
    nf2 += (FLOAT32)i*i;
  }
  nf2 *= 2.0F;
  sum1               = new FLOAT32 [mfcc_vector_size];
  sum2               = new FLOAT32 [mfcc_vector_size];
  // initializing vectors to zero
  memset(sum1, c, (mfcc_vector_size)*sizeof(FLOAT32));
  memset(sum2, c, (mfcc_vector_size)*sizeof(FLOAT32));

  ma_spec_vector     = new FLOAT32 [spec_vector_size];
  ma_lpc_spec_vector = new FLOAT32 [spec_vector_size];
  ma_temp_vector     = new FLOAT32 [spec_vector_size];
  ma_mfcc_vector     = new FLOAT32 [mfcc_vector_size];
  ma_lpc_mfcc_vector = new FLOAT32 [mfcc_vector_size];
  ma_enh_mfcc_vector = new FLOAT32 [mfcc_vector_size];


  // initializing vectors to zero
  memset(ma_spec_vector, c, spec_vector_size*sizeof(FLOAT32));
  memset(ma_lpc_spec_vector, c, spec_vector_size*sizeof(FLOAT32));
  memset(ma_temp_vector, c, spec_vector_size*sizeof(FLOAT32));
  memset(ma_mfcc_vector, c, mfcc_vector_size*sizeof(FLOAT32));
  memset(ma_lpc_mfcc_vector, c, mfcc_vector_size*sizeof(FLOAT32));
  memset(ma_enh_mfcc_vector, c, mfcc_vector_size*sizeof(FLOAT32));

  // Setting FE_FEATURES_STATS sub-structure pointer of ma_spec_estimate to ma_spec_vector
  features_stats_str.ma_spec_vector     = ma_spec_vector;
  features_stats_str.ma_lpc_spec_vector = ma_lpc_spec_vector;
  features_stats_str.ma_mfcc_vector     = ma_mfcc_vector;
  features_stats_str.ma_lpc_mfcc_vector = ma_lpc_mfcc_vector;
  features_stats_str.ma_enh_mfcc_vector = ma_enh_mfcc_vector;

  // Setting constant structure elements
  features_stats_str.spec_vector_size     = spec_vector_size;
  features_stats_str.mfcc_vector_size     = mfcc_vector_size;
  features_stats_str.enh_mfcc_vector_size = mfcc_vector_size;
}
//
// Delay Destructor
//
Delay::~Delay()
{
   delete [] pf32_xCoeffs;
   delete [] pf32_yCoeffs;
   delete [] f32_prev_xVal_mfcc;
   delete [] f32_prev_yVal_mfcc;
   delete [] f32_prev_xVal_enh_mfcc;
   delete [] f32_prev_yVal_enh_mfcc;

  for (UINT16 i=0; i<delay; i++) {
    delete [] spec_buffer[i];
    delete [] lpc_spec_buffer[i];
    delete [] mfcc_buffer[i];
    delete [] lpc_mfcc_buffer[i];
    delete [] enh_mfcc_buffer[i];
    //delete [] out_mfcc_buffer[i];
    //delete [] out_lpc_mfcc_buffer[i];
    //delete [] out_enh_mfcc_buffer[i];
  }

  delete [] spec_buffer;
  delete [] lpc_spec_buffer;
  delete [] mfcc_buffer;
  delete [] lpc_mfcc_buffer;
  delete [] enh_mfcc_buffer;
  //delete [] out_mfcc_buffer;
  //delete [] out_lpc_mfcc_buffer;
  //delete [] out_enh_mfcc_buffer;

  delete [] sum1;
  delete [] sum2;

  delete [] ma_spec_vector;
  delete [] ma_lpc_spec_vector;
  delete [] ma_mfcc_vector;
  delete [] ma_lpc_mfcc_vector;
  delete [] ma_enh_mfcc_vector;
  delete [] ma_temp_vector;

  if (bMonitor)
     End_Monitor();
}
//
// Initialization; Re-Configuration of the object
//
void Delay::Init(CfgDelay *cfgDelay)
{
  Reset();
  // Remaining Parameters must be intialize from vadConfig
  delay            = cfgDelay->delay;
  spec_vector_size = cfgDelay->spec_vector_size;
  mfcc_vector_size = cfgDelay->mfcc_vector_size;

  AllocBuffer();
}
//
// Reseting Delay object to default configuration
//
void Delay::Reset()
{
  frame_counter         = 0;
  ma_counter            = 0;
  buffer_indx           = 0;
  counter               = 0;
  VAD_ON_aver_energy    = 0;
  VAD_ON_peek_energy    = 0;
  first_time            = true;

  if (spec_buffer) {
    for (UINT16 i=0; i<delay; i++) {
      delete [] spec_buffer[i];
    }
    delete [] spec_buffer;
  }
  if (lpc_spec_buffer) {
    for (UINT16 i=0; i<delay; i++) {
      delete [] lpc_spec_buffer[i];
    }
    delete [] lpc_spec_buffer;
  }
  if (mfcc_buffer) {
    for (UINT16 i=0; i<delay; i++) {
      delete [] mfcc_buffer[i];
    }
    delete [] mfcc_buffer;
  }
  if (lpc_mfcc_buffer) {
    for (UINT16 i=0; i<delay; i++) {
      delete [] lpc_mfcc_buffer[i];
    }
    delete [] lpc_mfcc_buffer;
  }
  if (enh_mfcc_buffer) {
    for (UINT16 i=0; i<delay; i++) {
      delete [] enh_mfcc_buffer[i];
    }
    delete [] enh_mfcc_buffer;
  }

  //if (out_mfcc_buffer) {
  //  for (UINT16 i=0; i<delay; i++) {
  //    delete [] out_mfcc_buffer[i];
  //  }
  //  delete [] out_mfcc_buffer;
  //}
  //if (out_lpc_mfcc_buffer) {
  //  for (UINT16 i=0; i<delay; i++) {
  //    delete [] out_lpc_mfcc_buffer[i];
  //  }
  //  delete [] out_lpc_mfcc_buffer;
  //}
  //if (out_enh_mfcc_buffer) {
  //  for (UINT16 i=0; i<delay; i++) {
  //    delete [] out_enh_mfcc_buffer[i];
  //  }
  //  delete [] out_enh_mfcc_buffer;
  //}

  if (ma_spec_vector) {
    delete [] ma_spec_vector;
  }

  if (ma_lpc_spec_vector) {
    delete [] ma_lpc_spec_vector;
  }

  if (ma_mfcc_vector) {
    delete [] ma_mfcc_vector;
  }

  if (ma_lpc_mfcc_vector) {
    delete [] ma_lpc_mfcc_vector;
  }

  if (ma_enh_mfcc_vector) {
    delete [] ma_enh_mfcc_vector;
  }

  if (sum1) {
    delete [] sum1;
  }

  if (sum2) {
    delete [] sum2;
  }
}
//
// End-of-Stream Processing
//
FE_FEATURES *Delay::Eos() {

   if (bMonitor)
      End_Monitor();

   if (counter++ > delay) {
      return (NULL);
   }
   else {
      return(&output);
   }
}
//
// Setting Frame Rate Parameter of FE_FEATURES structure
//
void Delay::setFrameRate(UINT16 frame_rate)
{
   output.frame_rate = frame_rate;
}
//
// Monitoring Functions
//
void Delay::Open_MonitorFiles(CfgDelay* cfgDelay)
{
    // Build file path
    char filename[_MAX_PATH];
    strcpy(filename, cfgDelay->output_dir);
    char* ext = filename + strlen(filename);

    // Original Features without Cepstral Mean Normalization
    strcpy(ext, ".omfcc");
    m_omfcc_logger  = new WuwLogger(filename, sizeof(FLOAT32), cfgDelay->mfcc_vector_size); // Opening File in Binary Mode
    // Normalized Features without Cepstral Mean Normalization
    strcpy(ext, ".nmfcc");
    m_nmfcc_logger  = new WuwLogger(filename, sizeof(FLOAT32), cfgDelay->mfcc_vector_size); // Opening File in Binary Mode
    // Mean Features
    strcpy(ext, ".mmfcc");
    m_mmfcc_logger  = new WuwLogger(filename, sizeof(FLOAT32), cfgDelay->mfcc_vector_size); // Opening File in Binary Mode

    // AGC Features
    strcpy(ext, "_aver.agc");
    m_AGC_aver_volume_logger = new WuwLogger(filename, sizeof(FLOAT32), 1); // Opening File in Binary Mode
    strcpy(ext, "_peek.agc");
    m_AGC_peek_volume_logger = new WuwLogger(filename, sizeof(FLOAT32), 1); // Opening File in Binary Mode

   // Original ENH Features without Cepstral Mean Normalization
    strcpy(ext, ".oenhmfcc");
    m_oenhmfcc_logger  = new WuwLogger(filename, sizeof(FLOAT32), cfgDelay->enh_mfcc_vector_size); // Opening File in Binary Mode
    // Normalized Features without Cepstral Mean Normalization
    strcpy(ext, ".nenhmfcc");
    m_nenhmfcc_logger  = new WuwLogger(filename, sizeof(FLOAT32), cfgDelay->enh_mfcc_vector_size); // Opening File in Binary Mode
    // Mean Features
    strcpy(ext, ".menhmfcc");
    m_menhmfcc_logger  = new WuwLogger(filename, sizeof(FLOAT32), cfgDelay->enh_mfcc_vector_size); // Opening File in Binary Mode

    // Mean Estimate Flag
    strcpy(ext, ".mef");
    m_mef_logger = new WuwLogger(filename, sizeof(SINT32), 1); // Opening in Binary Mode  - Not Text Mode
}
//
void Delay::Run_Monitor()
{
   //m_nmfcc_logger->WriteBinary();
}
//
void Delay::End_Monitor()
{
   delete m_omfcc_logger;
   delete m_nmfcc_logger;
   delete m_mmfcc_logger;
   delete m_oenhmfcc_logger;
   delete m_nenhmfcc_logger;
   delete m_menhmfcc_logger;
   delete m_AGC_aver_volume_logger;
   delete m_AGC_peek_volume_logger;

   delete m_mef_logger;
}
//
// Computation of Moving Average
//
FLOAT32 *Delay::compute_MA_estimate(FLOAT32 *in_vector, UINT16 vector_size, FLOAT32 *ma_vector) {

  UINT16   i;

  if (ma_counter < ma_win_size)
     lambda_MAF = 1.0F - 1.0F/(FLOAT32)ma_counter;
  else
     lambda_MAF = LAMBDA_SLOWER_MA;

  for (i=0; i<vector_size; i++) {
     ma_vector[i] += (1.0F - lambda_MAF)*(in_vector[i] - ma_vector[i]);
  }

  return(ma_vector);
}
//
// Cepstral Mean Normalization of MFCC's - Using Estimate obtained during VAD_OFF period
//
FLOAT32 * Delay::CepstralMeanNormalization(FLOAT32 *in_vector, FLOAT32 *mean_estm)
{
   UINT16 i=0;
   //
   // Subtracting Estimated Mean Value in place from the Cepstrum
   // ???? Research questions: how does this compare to high-pass filtering
   // ???? of MFCC features
   //

   //for (i=0; i<mfcc_vector_size; i++) {
   //   in_vector[i] -= mean_estm[i];
   //}
   // C0
   i = mfcc_vector_size-2;
   if (in_vector[i] < 0) {
      in_vector[i] = 0.0F;
      // If C0 is negative means that energy is of the signal is zero thus features are set to zero
      for (UINT16 j=0; i<i; j++) {
         in_vector[j] = 0;
      }
   }
   // log2 of frame energy
   i = mfcc_vector_size-1;
   if (in_vector[i] < 0)
      in_vector[i] = 0.0F;

   // Shoudl the mean_est be reset too ????

   return(in_vector);
}
//
// Cepstral Mean Normalization of MFCC's - Using Estimate obtained during VAD_OFF period
//
FLOAT32 * Delay::DC_Filter(FLOAT32 *in_vector,
                           FLOAT32 *f32_prev_xVal,
                           FLOAT32 *f32_prev_yVal,
                           FLOAT32 *out_vector,
                           UINT32 vector_size)
{
   FLOAT32 sumX = 0.0F, sumY = 0.0F;
   UINT16 i=0;

   for (i=0; i<vector_size; i++) {
      /* y[n] = x[n] - x[n-1] + 0.999 * y[n-1] */
      sumX = pf32_xCoeffs[0] * in_vector[i] + pf32_xCoeffs[1] * f32_prev_xVal[i];
      sumY = pf32_yCoeffs[1] * f32_prev_yVal[i];
      out_vector[i] = sumX + sumY;
      f32_prev_xVal[i] = in_vector[i];
      f32_prev_yVal[i] = out_vector[i];
   }

   return(out_vector);
}
//
// Gradient Computation
//
void Delay::GradientI(FLOAT32 **in_vector, UINT16 index, FLOAT32 *out_vector)
{
  UINT16   i, j;
  SINT32   c=0, index1, index2;
  FLOAT32 *vector1;
  FLOAT32 *vector2;

  memset(sum1, c, (mfcc_vector_size)*sizeof(FLOAT32));
  memset(sum2, c, (mfcc_vector_size)*sizeof(FLOAT32));

  for (j=1; j<=num_gradI; j++) {

    index1 = index-j;
    if (index1<0)
       index1 = delay+index1;
    index2 = (index+j)%delay;

    vector1 = in_vector[index1]; // Pointing to Static Values
    vector2 = in_vector[index2]; // Pointing to Static Values

    for (i=0; i<mfcc_vector_size; i++) {
      sum1[i] += j*vector1[i];
      sum2[i] += j*vector2[i];
    }
  }
  for (i=0; i<mfcc_vector_size; i++) {
    out_vector[i] = (sum2[i]-sum1[i])/nf1;
  }
}
//
// Gradient Computation
//
void Delay::GradientII(FLOAT32 **in_vector, UINT16 index, FLOAT32 *out_vector)
{
  UINT16   i, j;
  SINT32   c=0, index1, index2;
  FLOAT32 *vector1;
  FLOAT32 *vector2;

  memset(sum1, c, (mfcc_vector_size)*sizeof(FLOAT32));
  memset(sum2, c, (mfcc_vector_size)*sizeof(FLOAT32));

  for (j=1; j<=num_gradII; j++) {

    index1 = index-j;
    if (index1<0)
       index1 = delay+index1;
    index2 = (index+j)%delay;

    vector1 = &in_vector[index1][mfcc_vector_size]; // Pointing to GradientI Values
    vector2 = &in_vector[index2][mfcc_vector_size]; // Pointing to GradientI Values

    for (i=0; i<mfcc_vector_size; i++) {
      sum1[i] += j*vector1[i];
      sum2[i] += j*vector2[i];
    }
  }
  for (i=0; i<mfcc_vector_size; i++) {
    out_vector[i] = (sum2[i]-sum1[i])/nf2;
  }
}
//
// Returning Internal Structure
//
FE_FEATURES_STATS * Delay::getFeatureStats()
{
  return(&features_stats_str);
}
UINT16 Delay::getBufferSize()
{
  return(delay);
}
//
// Buffering
//
FE_FEATURES * Delay::Run (FLOAT32 *in_spec_vector,
                          FLOAT32 *in_lpc_spec_vector,
                          FLOAT32 *in_mfcc_vector,
                          FLOAT32 *in_lpc_mfcc_vector,
                          FLOAT32 *in_enh_mfcc_vector,
                          FLOAT32  frame_energy,
                          FLOAT32  log2_frame_energy, // log_frame_energy was passed not log2_frame_energy
                          FLOAT32  log_frame_energy,
                          VAD_States *vad_states)
{
                          //VAD_STATE vad_state) {
   UINT16 i;

   // Setting up the pointer to appropriate buffer slice
   // danger if frame_counter exceeds precision after continous 248.5 days
   buffer_indx      = (frame_counter++)%delay;
   delayed_vec_indx = (buffer_indx+1)%delay;

   // Copying current spectral vector into the buffer slice
   memcpy(spec_buffer[buffer_indx], in_spec_vector, spec_vector_size*sizeof(FLOAT32));
   // Copying current spectral vector into the buffer slice
   memcpy(lpc_spec_buffer[buffer_indx], in_lpc_spec_vector, spec_vector_size*sizeof(FLOAT32));
   //
   // DC Filtering for Cepstral Mean Normalization of input mfcc features
   //
   DC_Filter(in_mfcc_vector, f32_prev_xVal_mfcc, f32_prev_yVal_mfcc, mfcc_buffer[buffer_indx], mfcc_vector_size-2);
   DC_Filter(in_lpc_mfcc_vector, f32_prev_xVal_lpc_mfcc, f32_prev_yVal_lpc_mfcc, lpc_mfcc_buffer[buffer_indx], mfcc_vector_size-2);
   DC_Filter(in_enh_mfcc_vector, f32_prev_xVal_enh_mfcc, f32_prev_yVal_enh_mfcc, enh_mfcc_buffer[buffer_indx], enh_mfcc_vector_size-2);

   // Copying current mfcc feature vector into the buffer slice
   //memcpy(mfcc_buffer[buffer_indx], in_mfcc_vector, (mfcc_vector_size-1)*sizeof(FLOAT32));
   // Appending log_frame_energy and unfiltered C0
   mfcc_buffer[buffer_indx][mfcc_vector_size-1] = log_frame_energy;
   mfcc_buffer[buffer_indx][mfcc_vector_size-2] = in_mfcc_vector[mfcc_vector_size-2];
   // Copying current mfcc feature vector into the buffer slice
   //memcpy(lpc_mfcc_buffer[buffer_indx], in_lpc_mfcc_vector, (mfcc_vector_size-1)*sizeof(FLOAT32));
   // Appending log_frame_energy and infiltered C0
   lpc_mfcc_buffer[buffer_indx][mfcc_vector_size-1] = log_frame_energy;
   lpc_mfcc_buffer[buffer_indx][mfcc_vector_size-2] = in_lpc_mfcc_vector[mfcc_vector_size-2];
   // Copying current enh_mfcc feature vector into the buffer slice
   //memcpy(enh_mfcc_buffer[buffer_indx], in_enh_mfcc_vector, (mfcc_vector_size-1)*sizeof(FLOAT32));
   // Appending log_frame_energy
   enh_mfcc_buffer[buffer_indx][mfcc_vector_size-1] = log_frame_energy;
   enh_mfcc_buffer[buffer_indx][mfcc_vector_size-2] = in_enh_mfcc_vector[mfcc_vector_size-2];
   //
   // Gradient Computation
   //
   if (frame_counter > 2*num_gradI) {
      gradI_indx = buffer_indx-num_gradI;
      if (gradI_indx < 0)
         gradI_indx = delay + gradI_indx;

      GradientI(mfcc_buffer, gradI_indx, &mfcc_buffer[gradI_indx][mfcc_vector_size]);
      GradientI(lpc_mfcc_buffer, gradI_indx, &lpc_mfcc_buffer[gradI_indx][mfcc_vector_size]);
      GradientI(enh_mfcc_buffer, gradI_indx, &enh_mfcc_buffer[gradI_indx][mfcc_vector_size]);

      if (frame_counter > 2*(num_gradI+num_gradII)) {
         gradII_indx = buffer_indx-(num_gradI+num_gradII);
         if (gradII_indx < 0)
            gradII_indx = delay + gradII_indx;

         GradientII(mfcc_buffer, gradII_indx, &mfcc_buffer[gradII_indx][2*mfcc_vector_size]);
         GradientII(lpc_mfcc_buffer, gradII_indx, &lpc_mfcc_buffer[gradII_indx][2*mfcc_vector_size]);
         GradientII(enh_mfcc_buffer, gradII_indx, &enh_mfcc_buffer[gradII_indx][2*mfcc_vector_size]);
      }
   }
   // Setting up the pointers of the output structure to correct vector in the buffer
   //output.vad_state               = vad_state;
   output.vad_state               = vad_states->VAD_State_Flag;
   vad_state                      = vad_states->VAD_State_Flag;
   output.num_mfcc_features       = mfcc_vector_size;
   output.fe_mfcc_features_buffer = mfcc_buffer[delayed_vec_indx];
   output.fe_lpc_mfcc_features_buffer = lpc_mfcc_buffer[delayed_vec_indx];
   output.num_enh_features        = enh_mfcc_vector_size;
   output.fe_enh_features_buffer  = enh_mfcc_buffer[delayed_vec_indx];
   output.AGC_info_str            = &AGC_info_str;

   // It assumes that the buffer is full when vad_state == VAD_OFF
   // Until then VAD_STATE will be VAD_INIT or VAD_ON
   // !!!! Perhaps a better place to perform this estimate is in VAD where the
   // delay will be minimal. !!!!!
   if (frame_counter >= delay) {
      if (vad_state != VAD_ON)  {
         if (first_time) {
            AGC_info_str.aver_energy = VAD_ON_aver_energy;
            AGC_info_str.peek_energy = VAD_ON_peek_energy;

            VAD_ON_aver_energy = 0.0F;
            VAD_ON_peek_energy = 0.0F;

            first_time = false;
         }
         if ((vad_states->VAD_EN_State_Flag != VAD_ON) &&
            (vad_states->VAD_MFCC_State_Flag != VAD_ON) &&
            (vad_states->VAD_SPEC_State_Flag != VAD_ON)) {

               if (bMonitor) {
                  SINT32 flag = 1;
                  m_mef_logger->WriteBinary(&flag);
               }

               if (ma_counter < ma_win_size)
                  ma_counter++;

               // spec_buffer not needed only lpc_spec_buffer for enh module
               // compute_MA_estimate(spec_buffer[delayed_vec_indx], spec_vector_size, ma_spec_vector);

               // This estimate is fed-back into enhacement procedure
               //compute_MA_estimate(lpc_spec_buffer[delayed_vec_indx], spec_vector_size, ma_lpc_spec_vector);
               compute_MA_estimate(lpc_spec_buffer[delayed_vec_indx], spec_vector_size, ma_temp_vector);

               ma_lpc_spec_vector[0] = ma_temp_vector[0];
               ma_lpc_spec_vector[spec_vector_size - 1] = ma_temp_vector[spec_vector_size - 1];
               for (int i=1; i<spec_vector_size - 1; i++) {
                  ma_lpc_spec_vector[i] = (ma_temp_vector[i-1] + 2.0*ma_temp_vector[i] + ma_temp_vector[i+1])/4.0;
               }

               // Moving Average of the two sets of MFCC features
               // compute_MA_estimate function for mfcc features must be optimized because
               // it only applies to energy features when DC_Filter is used
               compute_MA_estimate(mfcc_buffer[delayed_vec_indx], mfcc_vector_size, ma_mfcc_vector);
               compute_MA_estimate(lpc_mfcc_buffer[delayed_vec_indx], mfcc_vector_size, ma_lpc_mfcc_vector);
               compute_MA_estimate(enh_mfcc_buffer[delayed_vec_indx], mfcc_vector_size, ma_enh_mfcc_vector);
         }
         else {
            if (bMonitor) {
               SINT32 flag = 0;
               m_mef_logger->WriteBinary(&flag);
            }
         }
      }
      else { // vad_state == VAD_ON
         // AGC Support
         VAD_ON_aver_energy += (1 - LAMBDA_LTE_HIGHER_E) * (log2_frame_energy - VAD_ON_aver_energy);
         if (log2_frame_energy > VAD_ON_peek_energy) {
            VAD_ON_peek_energy = log2_frame_energy;
         }
         first_time = true;

         if (bMonitor) {
            SINT32 flag = 0;
            m_mef_logger->WriteBinary(&flag);
         }
      }

      if (bMonitor) {
         m_omfcc_logger->WriteBinary(mfcc_buffer[delayed_vec_indx]);
         m_oenhmfcc_logger->WriteBinary(enh_mfcc_buffer[delayed_vec_indx]);
      }
      // Applying Cepstral Mean Normalization
      // CepstralMeanNormalization function for mfcc features must be optimized because
      // it only applies to energy features when DC_Filter is used
      CepstralMeanNormalization(mfcc_buffer[delayed_vec_indx], ma_mfcc_vector);
      CepstralMeanNormalization(lpc_mfcc_buffer[delayed_vec_indx], ma_lpc_mfcc_vector);
      CepstralMeanNormalization(enh_mfcc_buffer[delayed_vec_indx], ma_enh_mfcc_vector);

      if (bMonitor) {
         m_nmfcc_logger->WriteBinary(mfcc_buffer[delayed_vec_indx]);
         m_mmfcc_logger->WriteBinary(ma_mfcc_vector);
         m_nenhmfcc_logger->WriteBinary(enh_mfcc_buffer[delayed_vec_indx]);
         m_menhmfcc_logger->WriteBinary(ma_enh_mfcc_vector);
      }
   }
   else {
      if (bMonitor) {
         SINT32 flag = 0;
         m_mef_logger->WriteBinary(&flag);
      }
   }
   if (frame_counter < delay) {
      return(NULL);
   }
   else {
      if (bMonitor) {
         m_AGC_aver_volume_logger->WriteBinary(&VAD_ON_aver_energy);
         m_AGC_peek_volume_logger->WriteBinary(&VAD_ON_peek_energy);
      }
      // returning output structure
      return(&output);
   }
}

/*******************************  End of File  *******************************/

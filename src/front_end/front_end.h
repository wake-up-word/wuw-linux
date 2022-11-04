/******************************************************************************

                    Copyright (C) 2006-2015 VoiceKey Inc.
                         Cocoa Beach, Florida 32931
                            All rights reserved

                      Confidential and Proprietary

     Module: front_end.h
     Author: Veton Kepuska
    Created: May 27, 2006
    Updated: September 9, 2006

Description: A modified version of front_end class definition based on 2004
             Veton Kepuska's version. Inspired by David Shipman's  model
             of modular software architecture implemented in C.

       $Log:$

*******************************************************************************

Notes:

******************************************************************************/

#ifndef FRONT_END_H
#define FRONT_END_H

/******************************  Include Files  ******************************/

#include <cmath>
#include <vector>
#include <algorithm>
using namespace std;

//#include <list>
#include "common/wuw_config.h"
#include "common/wuw_util.h"
#include "common/wuw_common.h"

#include "spectrum/spectrum.h"
#include "buffer/delay.h"
#include "mfcc/mfcc.h"
#include "vad/vad.h"
#include "enh/enh.h"

/*********************************  Defines  *********************************/

/*****************************  Class Definition  *****************************/

class FrontEnd {

 public:
  // Default Constructor
  FrontEnd();
  // FrontEndConfig must be initialized by interface function
  FrontEnd(FrontEndConfig *frontEndConfig);

  ~FrontEnd();

  void Init(FrontEndConfig *frontEndConfig);
  void Reset();

  FE_FEATURES *Run(FLOAT32 *in_samples);
  FE_FEATURES *Eos();

  // Utility Functions
  void      AllocBuffers();
  void      Run_Monitor();
  void      End_Monitor();
  void      Open_MonitorFiles(FrontEndConfig *frontEndConfig);
  void      DumpVADSegSignal (FE_FEATURES       *fe_features_str);
  void      SetAGCGain(FE_FEATURES *fe_features);

  FLOAT32   get_FEvadFeature();

 private:
  //
  // Member objects are created through composition that make-up FE Processing
  //
  //FRONT_END_TYPE front_end_type;   // Front End Type, e.g., ETSI_MFCC
  UINT16         front_end_type;     // Front End Type, e.g., ETSI_MFCC
  WINDOW_TYPE    window_type;        // window type
  UINT16         sample_rate;	       // Number of samples per second
  UINT16         frame_rate;	       // Number of analysis windows per second
  UINT16         frame_shift_size;   // Number of samples analysis window is shifted
  UINT16         frame_shift_sizex2; // 2x Number of samples window is shifted
  UINT16         num_mfcc_features;  // number of elements in a mfcc feature vector
  UINT16         num_enh_features;   // number of elements in a enh feature vector
  UINT32         FrameCounter;
  // Input Samples buffer
  FLOAT32       *fe_in_samples_buffer;
  //
  // For Dumping VAD Segmented sample data.
  //
  CfgFrontEnd*   mycfgFrontEnd;
  //UINT16         n_vads;
  //UINT32         samples_counter;
  //UINT32         n_frames;
  //vector<SINT16 *> vp_i16;
  //SINT16        *samples_frame;

  //
  // Pointers to feature buffers
  //
  FLOAT32       *fe_spectrum_buffer;
  FLOAT32       *fe_lpc_spectrum_buffer;
  FLOAT32       *fe_mfcc_features_buffer;
  FLOAT32       *fe_lpc_mfcc_features_buffer;
  FLOAT32       *fe_enh_features_buffer;
  FLOAT32       *fe_enh_mfcc_features_buffer;
  FLOAT32       *fe_delayed_spectrum;

  SpectMagStr   *p_SpectMagStr;

  FLOAT32        Frame_Energy_Floor;
  FLOAT32        Frame_Energy_Floor_log;
  FLOAT32        fe_frame_energy;
  FLOAT32        log2_fe_frame_energy;
  FLOAT32        log_fe_frame_energy;
  VAD_STATE      fe_vad_state;
  VAD_States    *fe_vad_states;

  ENHANCED_FEATURES *fe_enh_features_str;
  FE_FEATURES       *fe_features_str;
  FE_FEATURES_STATS *fe_delayed_features_str;
  //
  // Configuration Structure
  //
  // Debugging Information
  BOOLEAN         monitor;
  BOOLEAN         agc_flag;
  WuwLogger      *m_log2fe_logger;
  WuwLogger      *m_logfe_logger;
  WuwLogger      *m_enhs_logger;
  WuwLogger      *m_mfcc0_logger;
  WuwLogger      *m_lpc_mfcc0_logger;
  WuwLogger      *m_mfcc_logger;
  WuwLogger      *m_lpc_mfcc_logger;
  WuwLogger      *m_emfcc_logger;
  WuwLogger      *m_vad_logger;
  WuwLogger      *m_vadsegsig_logger;

  char         filename[_MAX_PATH];

  FrontEndConfig *frontEndConfig;
  Spectrum       *myFE_spectrum;
  MFCC           *myFE_mfcc;
  MFCC           *myFE_lpc_mfcc;
  VAD            *myFE_vad;
  Delay          *myFE_delay;
  Enhance        *myFE_enh;
  MFCC           *myFE_enh_mfcc;

  // Member Functions
  void Compute_LOGE (FLOAT32 Energy);
  void Compute_LOGE ();

};

#endif //FRONT_END_H

/*******************************  End of File  *******************************/

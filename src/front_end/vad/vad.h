/******************************************************************************

 	              Copyright (C) 2006-2015 VoiceKey Inc.
                           Cocoa Beach, Florida 32931
			      All rights reserved

			  Confidential and Proprietary


     Module: vad.h
     Author: Veton Kepuska
    Created: September 02, 2006
    Updated:

Description: A modified version of VAD class definition based on 2004
             Veton Kepuska's version. It combines ETSI VAD with proprietary
             solutions based on Veton Kepuska's original idea of VAD feature.

             Inspired by David Shipman's  model of modular software
             architecture originaly implemented in C.

       $Log:$

*******************************************************************************

            Proprietary and Confidential. All rights reserved.
               VoiceKey Inc., Cocoa Beach, Florida 32931.

*******************************************************************************

 Notes:

******************************************************************************/

#ifndef VAD_H
#define VAD_H

/******************************  Include Files  ******************************/

#include "common/defines.h"
#include <list>
#include "common/wuw_config.h"
#include "common/wuw_util.h"
#include "back_end/svm/svm.h"

/*********************************  Defines  *********************************/

class VAD {

 public:
  // Default Constructor
  VAD();
  // Constructor that uses configuration parameters to intialize the module
  VAD (VAD_Config *vadConfig, char* output_basename);
  // Destructor
  ~VAD ();

  // Function that initializes already exisiting object
  void Init(VAD_Config *vadConfig);

  // Function that resets already exisiting object's data members
  // to default values
  void Reset();

  // End Of Stream Function that flushes the buffers after end of the input
  // stream
  VAD_STATE Eos();

  // Computation of VAD State of the input stream
  // Input MFCC Feature Vector, Frame Energy
  // Output - VAD_STATE
  VAD_STATE Run (FLOAT32 *in_spec_vector, FLOAT32 *in_mfcc_vector, FLOAT32 frame_energy);

  VAD_States *getVAD_States();
  FLOAT32     getFrameEnergy();

 private:

  UINT32     frame_counter;       // VAD internal frame counter
  UINT32     n_init_frames;       // VAD initial number of frames
  UINT32     n_speech_frames;     // VAD internal frame counter
  UINT32     n_speech_frames_spec; // VAD frame counter for SPEC module
  UINT32     n_speech_frames_mfcc; // VAD frame counter for SPEC module

  UINT16     in_spec_vector_size; // Length of input spectral vector
  UINT16     spec_vector_size;	 // Number of selected features of spectral vector

  FLOAT32   *spec_vector;         // Pointer to spectrum vector
  FLOAT32    mean_spec;           // mean of selected spectral features
  FLOAT32    var_spec;            // variance of selected spectral features
  FLOAT32    aver_var_spec;       // Moving Average of the Variance of Spectral Vector
  FLOAT32    aver_var_spec_VAD_ON; // Moving Average of the Variance of Spectral Vector

  UINT16     in_mfcc_vector_size; // Length of mfcc feature vector
  UINT16     mfcc_vector_size;	 // Length of mfcc feature vector
  FLOAT32   *mfcc_vector;         // Pointer to vector of mfcc's
  FLOAT32   *vad_mfcc_features_buf;    // VAD Feature Buffer
  FLOAT32    vad_mfcc_feature;         // VAD Feature
  FLOAT32    prev_vad_mfcc_feature;    // VAD Feature
  FLOAT32    mean_vad_mfcc_feature;    // Moving Average vad feature
  FLOAT32    mean_vad_mfcc_feature_VAD_ON;
  FLOAT32    mean_vad_mfcc_feature_real_value;
  FLOAT32    local_aver_mfcc_fea_val;  // Local aver feature value
  FLOAT32    mean_mfcc, prev_mean_mfcc;
  FLOAT32    var_mfcc, var_mfcc_VAD_ON, mean_var_mfcc, var_var_mfcc, prev_mean_var_mfcc;
  FLOAT32    aver_var_mfcc;

  FLOAT32    frame_energy;        // Energy of current frame
  FLOAT32    log2_frame_energy;   // log2 energy of the frame
  FLOAT32    prev_log2_frame_energy;   // log2 energy of the frame
  FLOAT32    smooth_log2_frame_energy;   // log2 energy of the frame
  FLOAT32    mean_log2_fm_en;     // mean of log2_frame_energy
  FLOAT32    mean_log2_fm_en_VAD_ON;
  FLOAT32    delta_log2_frame_energy;
  //
  // Configurable Constant VAD Parameters
  //
  FLOAT32    lambda_LTE;          // mean energy filter constant
  UINT16     vad_mfcc_feature_buf_size;
  UINT16     hangover;            // delay before changing VAD_EN_State to VAD_OFF
  UINT16     hangover_spec;       // delay before changing VAD_SPEC_State to VAD_OFF
  UINT16     hangover_mfcc;       // delay before changing VAD_SPEC_State to VAD_OFF
  UINT16     n_min_frames;
  UINT16     n_min_frames2;
  UINT16     snr_threshold_vad;
  UINT16     snr_threshold_upd_lte;
  UINT16     nb_frame_threshold_lte;
  UINT16     min_speech_frame_hangover;

  FLOAT32    energy_floor;
  FLOAT32    lambda_lte_lower_e;
  FLOAT32    lambda_lte_higher_e;
  FLOAT32    partialSum, partialSum2;

  FLOAT32    VAD_log2_frame_energy_feature;
  FLOAT32    VAD_spec_feature;
  FLOAT32    VAD_mfcc_feature;

  VAD_States VAD_states;
  VAD_STATE  VAD_State_Flag;
  VAD_STATE  myVAD_State_Flag;
  VAD_STATE  VAD_EN_State_Flag;
  VAD_STATE  VAD_SPEC_State_Flag;
  VAD_STATE  VAD_MFCC_State_Flag;
  VAD_STATE  VAD_SVM_State_Flag;
  VAD_STATE  VAD_MFCC_Enhs_State_Flag;

  UINT16     VAD_ON_Count;
  UINT16     VAD_ON_Delay;
  UINT16     VAD_ON_min_count;  // Configurable Constant
  UINT16     VAD_ON_lead_count; // Configurable Constant
  UINT16     VAD_OFF_Count;
  UINT16     VAD_OFF_Delay;
  UINT16     VAD_OFF_min_count;   // Configurable Constant
  UINT16     VAD_OFF_trail_count; // Configurable Constant
  UINT16     VAD_Delay;

  // Debugging variables
  VAD_Config *vadCfg;
  bool		  monitor;
  bool        genVAD_FEF;
  WuwLogger  *v_logger_log2_frame_energy;
  WuwLogger  *v_logger_EN_state_flag;
  WuwLogger  *v_logger_energy_mean_log2;
  WuwLogger  *v_logger_spec_var_spec;
  WuwLogger  *v_logger_spec_avg_var;
  WuwLogger  *v_logger_SPEC_state_flag;
  WuwLogger  *v_logger_mfcc_feature;
  WuwLogger  *v_logger_mfcc_feature_mean;
  WuwLogger  *v_logger_MFCC_state_flag;
  WuwLogger  *v_logger_SVM_state_flag;

  WuwLogger  *v_logger_mfcc_var_spec;
  WuwLogger  *v_logger_mfcc_var_var;
  WuwLogger  *v_logger_MFCC_enhs_state_flag;
  WuwLogger  *v_logger_VAD_state_flag;
  WuwLogger  *v_logger_mfcc_mean;

  FILE       *fp_vadFEF;

  //SVM
  struct svm_model*      svm_model;
  svm_node       svm_features[4];

  void       compute_VAD_mfcc_feature();
  void       compute_signal_energy_stats();
  void       compute_VAD_MFCC_stats();
  void       compute_VAD_MFCC_Enhanced_stats();
  void       compute_VAD_SPEC_stats();
  void       compute_VAD_SVM();

  void       set_VAD_EN_state();
  void       set_VAD_SPEC_state();
  void       set_VAD_MFCC_state();
  void		 set_VAD_MFCC_Enhs_state();

  void       set_VAD_state();
  // Debugging information
  void      Run_Monitor();
  void      End_Monitor();
  void      Open_MonitorFiles(char* output_basename);

};

#endif

/*******************************  End of File  *******************************/

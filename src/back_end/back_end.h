/******************************************************************************

                    Copyright (C) 2006-2015 VoiceKey Inc.
                         Cocoa Beach, Florida 32931
                            All rights reserved

                       Confidential and Proprietary

     Module: back_end.h
     Author: Veton Kepuska
    Created: October 28, 2006
    Updated:

Description: Back_End module of the e-WUW Speech Recognizor.

       $Log:$

*******************************************************************************

Notes:

******************************************************************************/

#ifndef BACK_END_H
#define BACK_END_H

/******************************  Include Files  ******************************/

#include <cmath>
#include <vector>
#include <algorithm>
using namespace std;

#include "common/wuw_config.h"
#include "common/wuw_util.h"
#include "common/wuw_common.h"
#include "common/wuw_io.h"
#include "common/defines.h"

// #include "dtw\dtw_score.h"
#include "hmm/hmm.h"
#include "svm/svm.h"

/*********************************  Defines  *********************************/


/*****************************  Class Definition  *****************************/

class BackEnd {

 public:
  // Default Constructor
  BackEnd();
  // BackEndConfig must be initialized by interface function
  BackEnd(BackEndConfig *BackEndConfig);

  ~BackEnd();

  void Init(BackEndConfig *BackEndConfig);
  void Reset();
//   BE_SCORE *Eos();

  //BE_SCORE *Run(FE_FEATURES * in_features);
  void *Run(FE_FEATURES * in_features);

  //FLOAT32   Run_DTW();
  //mScores * Run_DTW2();
  //mScores * Run_HMM();

  // Utility Functions
  void      StoreFeatures();
  void      ClearSegment();
  void      AllocBuffers();
  void      Run_Monitor();
  void      End_Monitor();
  void      Open_MonitorFiles(CfgBackEnd* cfgBackEnd);

 private:
  //
  // Member objects are created through composition that make-up FE Processing
  //
  BACK_END_TYPE  me_back_end_type;              // Back End Type, e.g., HMM_CD, DTW_m, ..., COMBINED
  //UINT16         mu16_num_mfcc_features;      // number of elements in a mfcc feature vector
  //UINT16         mu16_num_enh_mfcc_features;  // number of elements in a enh_mfcc feature vector
  UINT32         mu32_FrameCounter;
  //UINT16         mu16_NumWordRecogs;
  UINT16         mu16_WordFrameCounter;         // Counter of Number of Frames per VAD segment
  UINT16         mu16_NumVADCounter;            // Counts Number of VAD triggers
  UINT16         mu16_frame_rate;

  FLOAT32        BAD_SCORE;
  FLOAT32        threshold;
  FLOAT32        threshold2;

//   ms_DTW_Score  *ms_dtw_m_scores;
  //ms_HMM_Scores        ms_hmm_scores;

  bool           mb_vad_on_flag;
  bool           mb_vad_off_flag;

  bool           mb_train_flag;
  bool           mb_genFEF_flag;
  bool           mb_genAllFEF_flag;

  // Input Samples buffer
  FE_FEATURES      * mp_fe_feature_vecs;
  SEGMENT_FEATURES   m_SegFeatureVecs; // Back-end has the only copy of the buffered features

  char        mw_ALLfef_filename[_MAX_PATH];
  char        mw_fef_filename[_MAX_PATH];
  char        mw_vad_filename[_MAX_PATH];
  char *         STARTsnd;
  char *         RECsnd;
  char *         CMDsnd;
  char *         EXITsnd;

  WUW_IO        *FEF_file;
  WUW_IO        *vad_file;

  UINT16         min_WUW_length;
  UINT16         max_WUW_length;
  //
  // Pointers to feature buffers
  //
  FLOAT32       *pf32_fe_mfcc_features_buffer;
  FLOAT32       *pf32_fe_lpc_mfcc_features_buffer;
  FLOAT32       *pf32_fe_enh_mfcc_features_buffer;
  FLOAT32       *feature_vector;
  UINT16         num_fe_mfcc_features;
  UINT16         num_fe_enh_mfcc_features;
  VAD_STATE      me_fe_vad_state;
  //
  // Configuration Structure
  //
  BackEndConfig  *backEndConfig;
  CfgBackEnd     *mp_cfgBackEnd;
  CfgTrain       *mp_cfgTrain;
  //
  // Monitoring Flag
  //
  BOOLEAN         mb_monitor;
  //
  // Loggers
  //
  WuwLogger      *m_vad_logger;
  //
  // BackEnd Modules
  //
  BACK_END_TYPE   me_scoring;
//   DTW_Score      *pm_DTW;       // Simple DTW matching with means only model vectors

  // HMM
  HmmModel*       hmm_model[3][3];
  HmmNetwork*     hmm_net[3][3];
  FLOAT64*        hmm_features[3];
	int           numModels;

  // SVM
  struct svm_model*      svm_model;
  struct svm_node*       svm_features;
};

#endif //BACK_END_H

/*******************************  END OF FILE  *******************************/

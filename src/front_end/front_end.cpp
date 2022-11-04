/******************************************************************************
                    Copyright (C) 2006-2015 VoiceKey Inc.
                         Cocoa Beach, Florida 32931
                            All rights reserved

                      Confidential and Proprietary

     Module: front_end.cpp
     Author: Veton Kepuska
    Created: May 27, 2006
    Updated: September 9, 2006

Description: A modified version of front_end class definition based on 2004
             Veton Kepuska's version. Inspired by David Shipman's  model
             of modular software architecture originaly implemented in C.

$Log:$

*******************************************************************************

             Proprietary and Confidential. All rights reserved.
                VoiceKey Inc., Cocoa Beach, Florida 32931.

******************************************************************************/

/**********************************  Notes  **********************************

 +------------------------------------------------------------------------+
 | front_end                                                              |
 |                                                                        |
 |                          magsq                                         |
 | input   +--------------+ spectrum  +--------------+                    |
---------->| computation  |---+------>|     MFCC     |-----+              |
 | samples | of spectrum  |   |       +--------------+     |              |
 |         +--------------+   |                            |              |
 |                            |                            |              |
 |    +-----------------------+                            |              |
 |    |                                                    |              |
 | +--~------------+---------------------------------------+              |
 | |  |            | mfcc                                                 |
 | |  |            |                                                      | VAD
 | |  |            |         +------------------------------------------------>
 | |  |            |         |                                            | State
 | |  |            |         |                                  8000   1  |
 | |  |            v         |                                 ------ --- |
 | |  | e  +--------------+  |              +--------------+      M   sec | enh
 | |  +--->|      VAD     |--+  +---------->|     ENH      |------------------------>
 | |  |    +--------------+  |  | Spectral  +--------------+   Frame Rate | spectrum
 | |  |            ^         |  | Estimate         ^                      |
 | |  | spectrum   |         |  | Background       |                      |
 | |  +------------+         |  |                  |                      |
 | |  |  magsq               |  |                  |                      |mfcc
 | |  |            +---------+  |                  |                 +---------->
 | |  |            |            |                  |                 |    |features
 | |  |            v            |    Synchronized  | Spectral        |    |
 | |  |    +-+-+-+-+-+-+-+-+    |    Spectral      | Estimates       |    |
 | +--+===>| | | | | | | | |====+------------------+ and VAD State   |    |
 |  mfcc   +-+-+-+-+-+-+-+-+    |    Features with                   |    |
 |             BUFFERING        |                                    |    |
 |                              |                                    |    |
 |                              |                                    |    |
 |                              +------------------------------------+    |
 |                                                                        |
 +------------------------------------------------------------------------+

-----------------------------------------------------------------------------*/

/******************************  Include Files  ******************************/

#include <stdio.h>
#include <stdarg.h>
#include <cstdlib>
//using std::rand;
//using std::srand;jjj
#include <ctime>
//using std::time;

#include "front_end.h"

#include "vad/vad.cpp"


/*********************************  Defines  *********************************/

#define RAND_RANGE_MAX    (2.0F*16.0F)

/****************************  Function Members  *****************************/

FrontEnd::FrontEnd()
{
    /* We are phasing out default constructors -
    a function should always be initialized with its "config" constructor */
}

FrontEnd::FrontEnd(CfgFrontEnd* cfgFrontEnd)
{
   mycfgFrontEnd  = cfgFrontEnd;
   // Initialize parameters
   // Initialize monitor flags
   if (cfgFrontEnd->monitor) {
      monitor                       = true;
      cfgFrontEnd->cfgMFCC.bMonitor = true;
      // This value shoudl is passed with constructor not needed here
      // wcscpy_s(cfgFrontEnd->cfgMFCC.output_dir, _MAX_PATH, cfgFrontEnd->out_data_dir);
      cfgFrontEnd->cfgDelay.bMonitor = true; //false; //true;
      cfgFrontEnd->cfgVAD.monitor    = true; //false; //true;
   }
   else {
      monitor = false;
   }
   // Must make it configurable
   agc_flag  = true;

   FrameCounter       = 0;
   front_end_type     = cfgFrontEnd->fe_type; // Front-End Type
   sample_rate        = cfgFrontEnd->sample_rate;
   frame_rate         = cfgFrontEnd->frame_rate;
   frame_shift_size   = cfgFrontEnd->frame_shift_size;
   frame_shift_sizex2 = 2*frame_shift_size;

   Frame_Energy_Floor_log = cfgFrontEnd->frame_energy_floor_log;
   Frame_Energy_Floor     = (FLOAT32) exp ((FLOAT64) Frame_Energy_Floor_log/10.0);

   // Initialize modules
   sprintf(filename, "%s\\%s", cfgFrontEnd->out_data_dir, cfgFrontEnd->basefilename);
   myFE_spectrum = new Spectrum(&cfgFrontEnd->cfgSpectrum,  filename);
   myFE_mfcc     = new MFCC(&cfgFrontEnd->cfgMFCC, filename);
   myFE_lpc_mfcc = new MFCC(&cfgFrontEnd->cfgMFCC, filename);
   myFE_vad      = new VAD(&cfgFrontEnd->cfgVAD, filename);
   myFE_delay    = new Delay(&cfgFrontEnd->cfgDelay, filename);

   //myFE_spectrum = new Spectrum(&cfgFrontEnd->cfgSpectrum,  cfgFrontEnd->out_data_dir);
   //myFE_mfcc     = new MFCC(&cfgFrontEnd->cfgMFCC, cfgFrontEnd->out_data_dir);
   //myFE_vad      = new VAD(&cfgFrontEnd->cfgVAD, cfgFrontEnd->out_data_dir);
   //myFE_delay    = new Delay(&cfgFrontEnd->cfgDelay, cfgFrontEnd->out_data_dir);

   if (front_end_type & FET_VK_LPC_ENH_MFCC) {
      myFE_enh      = new Enhance();                    // TODO: use cfgEnhance.
      cfgFrontEnd->cfgMFCC.bMonitor = false;            // Because we are using the same cfg must have two cfg's for bMonitor
      myFE_enh_mfcc = new MFCC(&cfgFrontEnd->cfgMFCC, cfgFrontEnd->out_data_dir);  // do we need to use same cfgMFCC ????
   }
   else {
      myFE_enh      = NULL;
      myFE_enh_mfcc = NULL;
   }
   // Setting up pointer to delayed features struct & frame_rate parameter
   fe_delayed_features_str     = myFE_delay->getFeatureStats();
   myFE_delay->setFrameRate(frame_rate);

   // Initialize buffers
   fe_in_samples_buffer = new FLOAT32 [frame_shift_size];

   // Initializing Random Number Generator
   srand((unsigned) time(NULL));

   //
   // Must instrument this code to dump VAD segmented input signal
   //
   //samples_counter    = 0;
   //n_vads             = 0;
   //n_frames           = myFE_delay->getBufferSize();
   //for (int i=0; i<n_frames; i++) {
   //    SINT16 *tmp = new SINT16 [frame_shift_size];
   //    vp_i16.push_back(tmp);
   //}
   //
   if (monitor) {
      Open_MonitorFiles(cfgFrontEnd);
   }
}

FrontEnd::~FrontEnd ()
{
   // Closing Monitoring Files
   if (monitor)
      End_Monitor();

  // Release modules
  delete myFE_spectrum;
  delete myFE_mfcc;
  delete myFE_lpc_mfcc;
  delete myFE_vad;
  delete myFE_delay;
  if (myFE_enh != NULL) {
     delete myFE_enh;
     delete myFE_enh_mfcc;
  }
  // Release buffers
  delete[] fe_in_samples_buffer;
}

// End-Of-Stream Function
FE_FEATURES*  FrontEnd::Eos()
{
   SINT32 vad;

   // Must flush the buffers here
   fe_features_str = myFE_delay->Eos();
   if (fe_features_str != NULL) {
      if (fe_features_str->vad_state == VAD_ON)
         vad = 1;
      else
         vad = 0;

      m_vad_logger->WriteBinary(&vad); // Binary Info
   }
   return(fe_features_str);
}

void FrontEnd::Open_MonitorFiles(CfgFrontEnd* cfgFrontEnd)
{
   // Build file path
    char filename[_MAX_PATH];
    sprintf(filename, "%s\\%s", cfgFrontEnd->out_data_dir, cfgFrontEnd->basefilename);
    //wcscpy_s(filename, _MAX_PATH, cfgFrontEnd->out_data_dir);
    char* ext = filename + strlen(filename);

    strcpy(ext, ".log2e");
    m_log2fe_logger  = new WuwLogger(filename, sizeof(FLOAT32), 1); // Opening File in Binary Mode

    strcpy(ext, ".loge");
    m_logfe_logger  = new WuwLogger(filename, sizeof(FLOAT32), 1); // Opening File in Binary Mode

    strcpy(ext, ".enhs");
    m_enhs_logger  = new WuwLogger(filename, sizeof(FLOAT32), fe_delayed_features_str->spec_vector_size); // Opening File in Binary Mode

    strcpy(ext, ".mfcc0");
    m_mfcc0_logger  = new WuwLogger(filename, sizeof(FLOAT32), fe_delayed_features_str->mfcc_vector_size-1); // Opening File in Binary Mode

    strcpy(ext, ".lpcmfcc0");
    m_lpc_mfcc0_logger  = new WuwLogger(filename, sizeof(FLOAT32), fe_delayed_features_str->mfcc_vector_size-1); // Opening File in Binary Mode

    strcpy(ext, ".mfcc");
    m_mfcc_logger  = new WuwLogger(filename, sizeof(FLOAT32), 3*fe_delayed_features_str->mfcc_vector_size); // Opening File in Binary Mode

    strcpy(ext, ".lpcmfcc");
    m_lpc_mfcc_logger  = new WuwLogger(filename, sizeof(FLOAT32), 3*fe_delayed_features_str->mfcc_vector_size); // Opening File in Binary Mode

    strcpy(ext, ".emfcc");
    m_emfcc_logger = new WuwLogger(filename, sizeof(FLOAT32), 3*fe_delayed_features_str->enh_mfcc_vector_size); // Opening File in Binary Mode

    strcpy(ext, ".vad");
    m_vad_logger   = new WuwLogger(filename, sizeof(SINT32), 1); // Opening in Binary Mode  - Not Text Mode

   //wcscpy(ext, L".txt_vad");
    //m_vad_txt_logger  = new WuwLogger(filename, false); // Opening File in Text Mode
}
void FrontEnd::End_Monitor()
{
   delete m_log2fe_logger;
   delete m_logfe_logger;
    delete m_enhs_logger;
    delete m_mfcc0_logger;
    delete m_lpc_mfcc0_logger;
    delete m_mfcc_logger;
    delete m_lpc_mfcc_logger;
    delete m_emfcc_logger;
    delete m_vad_logger;
    //delete m_vad_txt_logger;
}

// Monitoring Function
void FrontEnd::Run_Monitor()
{
   SINT32 vad = 0;

   m_logfe_logger->WriteBinary(&log_fe_frame_energy);
   m_enhs_logger->WriteBinary(fe_enh_features_buffer);
   m_mfcc_logger->WriteBinary(fe_features_str->fe_mfcc_features_buffer);
   m_lpc_mfcc_logger->WriteBinary(fe_features_str->fe_lpc_mfcc_features_buffer);
   m_emfcc_logger->WriteBinary(fe_features_str->fe_enh_features_buffer);

   if (fe_features_str->vad_state == VAD_ON) {
      vad = 1;
   }
   else {
      vad = 0;
   }

   m_vad_logger->WriteBinary(&vad); // Binary Info
   //m_vad_txt_logger->Printf(L"[%4i] VAD_STATE: %i\n", FrameCounter, vad); // Text Info
}

// Main Processing Function
FE_FEATURES* FrontEnd::Run(FLOAT32* in_samples)
{
  if(!in_samples)
    return NULL;

  FrameCounter++;
  //memcpy(fe_in_samples_buffer, in_samples, frame_shift_size * sizeof(FLOAT32));
  //
  // Make it addtive noise optional
  //
  for (UINT16 i=0; i<frame_shift_size; i++) {
     fe_in_samples_buffer[i] = in_samples[i]; // + RAND_RANGE_MAX*(((FLOAT32)rand()/(FLOAT32)RAND_MAX) - 0.5F);
  }
  //
  // Processing Samples
  //
  //fe_spectrum_buffer          = myFE_spectrum->Run(fe_in_samples_buffer);
  p_SpectMagStr               = myFE_spectrum->Run(fe_in_samples_buffer);
  fe_spectrum_buffer          = p_SpectMagStr->pf32_spectmag;
  fe_lpc_spectrum_buffer      = p_SpectMagStr->pf32_lpc_spectmag;
  fe_frame_energy             = myFE_spectrum->getEnergy();
  //fe_mfcc_features_buffer     = myFE_mfcc->Run(fe_spectrum_buffer);
  //fe_vad_state                = myFE_vad->Run(fe_spectrum_buffer, fe_mfcc_features_buffer, fe_frame_energy);
  fe_mfcc_features_buffer     = myFE_mfcc->Run(fe_spectrum_buffer);
  fe_lpc_mfcc_features_buffer = myFE_lpc_mfcc->Run(fe_lpc_spectrum_buffer);
  fe_vad_state                = myFE_vad->Run(fe_lpc_spectrum_buffer, fe_lpc_mfcc_features_buffer, fe_frame_energy);
  fe_vad_states               = myFE_vad->getVAD_States();
  log2_fe_frame_energy        = myFE_vad->getFrameEnergy(); // Used only for testing and AGC

  Compute_LOGE ();            // Sets log_fe_frame_energy

  // Before Buffering
  if (monitor) {
     m_mfcc0_logger->WriteBinary(fe_mfcc_features_buffer);
     m_lpc_mfcc0_logger->WriteBinary(fe_lpc_mfcc_features_buffer);
     m_log2fe_logger->WriteBinary(&log2_fe_frame_energy);
  }
  //
  // Feeding-back background/channel estimates of spectral & mfcc features
  //
  fe_delayed_features_str     = myFE_delay->getFeatureStats();
  if (myFE_enh) {
     // Enhanced Spectrogram Feature Computation
     fe_enh_features_buffer      = myFE_enh->Run(fe_lpc_spectrum_buffer, fe_delayed_features_str->ma_lpc_spec_vector);
     fe_enh_mfcc_features_buffer = myFE_enh_mfcc->Run(fe_enh_features_buffer);
     //
     // Buffering Features in order to align them with VAD also computing dynamic features
     //
     fe_features_str             = myFE_delay->Run(fe_spectrum_buffer,
                                                   fe_lpc_spectrum_buffer,
                                                   fe_mfcc_features_buffer,
                                                   fe_lpc_mfcc_features_buffer,
                                                   fe_enh_mfcc_features_buffer,
                                                   fe_frame_energy,
                                                   log2_fe_frame_energy,
                                                   log_fe_frame_energy,
                                                   fe_vad_states);
  }
  else {
     fe_features_str             = myFE_delay->Run(fe_spectrum_buffer,
                                                   fe_lpc_spectrum_buffer,
                                                   fe_mfcc_features_buffer,
                                                   fe_lpc_mfcc_features_buffer,
                                                   NULL,
                                                   fe_frame_energy,
                                                   log2_fe_frame_energy,
                                                   log_fe_frame_energy,
                                                   fe_vad_states);
  }
  if ((monitor) && (fe_features_str != NULL))
     Run_Monitor();
  //
  // Must Instrument this code as well but separete from monitor switch or
  // extend monitor switch to include be able to selectively indicate which
  // functions are invocked
  //
  //DumpVADSegSignal(fe_features_str);
  if ((agc_flag) && (fe_features_str != NULL)) {
     SetAGCGain(fe_features_str);
  }

  return(fe_features_str);
}

void FrontEnd::SetAGCGain(FE_FEATURES *fe_features)
{
   fe_features_str->Delta_AGC_Gain = ((1 - LAMBDA_AGC)/AGC_TARGET_ENERGY)*(AGC_TARGET_ENERGY - fe_features->AGC_info_str->peek_energy);
}

void FrontEnd::Reset()
{
}

void FrontEnd::Init(FrontEndConfig* cfgFrontEnd)
{
}

void FrontEnd::Compute_LOGE (FLOAT32 Energy)
{
    if (Energy < Frame_Energy_Floor) {
        log_fe_frame_energy = Frame_Energy_Floor_log;
    }
    else {
        log_fe_frame_energy = (FLOAT32) log ((FLOAT64) Energy);
    }
}
void FrontEnd::Compute_LOGE ()
{
   if ( fe_frame_energy < Frame_Energy_Floor) {
        log_fe_frame_energy = Frame_Energy_Floor_log;
    }
    else {
        log_fe_frame_energy = (FLOAT32) log ((FLOAT64) fe_frame_energy);
    }
}
//
//void FrontEnd::DumpVADSegSignal (FE_FEATURES       *fe_features_str)
//{
//   static UINT16 vad = 0;
//
//   if (fe_features_str != NULL) {
//      if (fe_features_str->vad_state == VAD_ON) {
//         if (!vad) {
//            wchar_t filename[_MAX_PATH];
//            swprintf(filename, L"%s\\%s_%03d", mycfgFrontEnd->out_data_dir, mycfgFrontEnd->basefilename, n_vads++);
//            wchar_t* ext = filename + wcslen(filename);
//
//            wcscpy(ext, L".sig");
//            m_vadsegsig_logger   = new WuwLogger(filename, sizeof(SINT16), frame_shift_size); // Opening in Binary Mode  - Not Text Mode
//         }
//
//         vad = 1;
//         m_vadsegsig_logger->WriteBinary(vp_i16[(FrameCounter-n_frames+1)%n_frames]);
//      }
//      else {
//         if (vad) {
//            delete m_vadsegsig_logger;
//         }
//         vad = 0;
//      }
//   }
//}

/*******************************  End of File  *******************************/

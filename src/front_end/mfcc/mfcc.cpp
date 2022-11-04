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

/**********************************  Notes  **********************************

+------------------------------------------------------------------------+
| mfcc                                                                   |
|                                                                        |
|  mag                                                                   |
|  spectrum  +-----------+      +-------+      +-------+   mfcc          |
------------->|    Mel    |----->|       |----->|  DCT  |------------------->
|            | filtering |      |  log  |      |transf.|                 |
|            +-----------+      +-------+      +-------+                 |
+------------------------------------------------------------------------+

-------------------------------------------------------------------------------*/

/******************************  Include Files  ******************************/

#include "mfcc.h"
#include <stdio.h>

/*********************************  Defines  *********************************/

/****************************  Function Members  *****************************/
//
// Default Constructor
//
MFCC::MFCC()
{
    // We are phasing out default constructors
    // A function should always be initialized with its "config" constructor
}

MFCC::MFCC(CfgMFCC* cfgMFCC, char* output_basename)
{
    // Initialize parameters
    in_vector_size         = cfgMFCC->i16_in_vector_size;
    out_vector_size        = cfgMFCC->i16_out_vector_size;

    // Initialize modules
    myMFCC_mel_filter      = new MelFilter(&cfgMFCC->cfgMelFilter);
    myMFCC_vlog            = new VLOG(&cfgMFCC->cfgVLog);
    myMFCC_dct             = new DCT(&cfgMFCC->cfgDCT);

   // Monitor Functions Must be last
   if (cfgMFCC->bMonitor) {
      bMonitor     = true;
      call_no      = 0;
      num_calls    = 3;
      strcpy(cfgMFCC->output_dir, output_basename);
      // disabling other objects reporting
      // cfgMFCC->bMonitor = false;
      Open_MonitorFiles(cfgMFCC);
   }
   else {
      bMonitor = false;
   }
}
//
// Desctructor
//
MFCC::~MFCC()
{
    // Release modules
    delete myMFCC_mel_filter;
    delete myMFCC_vlog;
    delete myMFCC_dct;

   if (bMonitor) {
      delete m_melfilt_logger;
      delete m_dctdata_logger;
      //delete m_meldata_logger;
      m_meldata_logger.empty();
   }
}

void MFCC::Reset()
{
}

void MFCC::Init(CfgMFCC* cfgMFCC)
{
}
//
//
//
void MFCC::Open_MonitorFiles(CfgMFCC* cfgMFCC)
{
    // Build file path
    char filename[_MAX_PATH];
    char extension[20];
    //swprintf(filename, L"%s\\%s", cfgMFCC->out_data_dir, cfgMFCC->basefilename);
    strcpy(filename, cfgMFCC->output_dir);
    char* ext = filename + strlen(filename);

   if (call_no == 0) {
      strcpy(ext, ".melf");
      m_melfilt_logger = new WuwLogger(filename, sizeof(FLOAT32), cfgMFCC->cfgMelFilter.fft_size/2); // Opening File in Binary Mode

      strcpy(ext, ".dctd");
      m_dctdata_logger = new WuwLogger(filename, sizeof(FLOAT32), (cfgMFCC->cfgDCT.out_vec_size-1)*cfgMFCC->cfgMelFilter.num_mel_filters); // Opening File in Binary Mode
   }

   for (int i=0; i<num_calls; i++) {
      memset(extension, 0, 20*sizeof(wchar_t));
      sprintf(extension, ".meld%1d", i);
      strcpy(ext, extension);
      //wcscpy(ext, L".meld");
      m_meldata_logger.push_back(new WuwLogger(filename, sizeof(FLOAT32), cfgMFCC->cfgMelFilter.num_mel_filters)); // Opening File in Binary Mode
   }

   // Dumping Mel-Filters
   myMFCC_mel_filter->PrintTriangles(m_melfilt_logger);
   myMFCC_dct->PrintDCTMatrix(m_dctdata_logger);

}
//
void MFCC::Run_Monitor()
{
   (m_meldata_logger[call_no])->WriteBinary(mel_filtered_spectrum);
   call_no = (call_no+1)%num_calls;
}
//
// Run Function
//
FLOAT32 *MFCC::Run(FLOAT32 *mag_spectrum)
{
    mel_filtered_spectrum     = myMFCC_mel_filter->Run(mag_spectrum);
    log_mel_filtered_spectrum = myMFCC_vlog->Run(mel_filtered_spectrum);
    out_mfcc_vector           = myMFCC_dct->Run(log_mel_filtered_spectrum);

   if (bMonitor) {
      Run_Monitor();
   }

    return(out_mfcc_vector);
}

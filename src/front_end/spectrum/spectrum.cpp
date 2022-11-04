/******************************************************************************

                      Copyright (C) 2006-2015 VoiceKey Inc.
                           Cocoa Beach, Florida 32931
                                All rights reserved

                           Confidential and Proprietary

     Module: spectrum.cpp
     Author: Veton Kepuska
    Created: May 27, 2006
    Updated:

Description: A modified version of spectrum class definition based on 2004
             Veton Kepuska's version. Inspired by David Shipman's  model
             of modular software architecture originaly implemented in C.

$Log:$

*******************************************************************************

******************************************************************************/

/**********************************  Notes  **********************************

  +------------------------------------------------------------------------+
  | front_end                                                              |
  |                          preemphasized    N points           N points  |
  |  +--------+   +--------+ samples +--------+       +---------+          |
---->| DC filt|-->| preemp |-------->| window |------>| hamming |------+   |
  |  +--------+   +--------+  8kHz   |(advance|       | window  |      |   |
  | float                            |  M pts)|       +---------+      |   |
  | samples                          +--------+ (8000/M)/sec           |   |
  | (8kHz)                                                             |   |
  |                                                      (8000/M)/sec  |   |
  | +------------------------------------------------------------------+   |
  | |                                                                      |
  | |                              fft_size      magsq                     |
  | |      +---------+ L-lpc   +---------------+ spectrum                  |
  | +----->| lpc     |-------->| real-to-magsq |-----------------------+   |
  |        +---------+ coeffs  +---------------+ (8000/M)/SEC          |   |
  |                                                                    |   |
  | +------------------------------------------------------------------+   |
  | |                                                                      |
  | |                                                          8000   1    |
  | |fft_size/2               magsq                           ------ ---   |
  | |       +---------------+ spectrum                           M   sec   |
  | +------>| time smooth & |-------------------------------------------------->
  |         |  downsample   |                                  Frame Rate  |
  |         +---------------+                                              |
  +------------------------------------------------------------------------+

-------------------------------------------------------------------------------*/

/******************************  Include Files  ******************************/

#include <iostream>
#include <cstdlib>
#include "spectrum.h"

using namespace std;

/*********************************  Defines  *********************************/

//-----------------------------------------------------------------------------
// Name: Spectrum::Spectrum()
// Desc: Constructor
//-----------------------------------------------------------------------------
Spectrum::Spectrum(SpectrumConfig* cfgSpectrum, char* output_basename)
{
	// Initialize parameters
	bMonitor                  = cfgSpectrum->monitor;
	i32_frame_counter         = 0;

	// Initialize modules
	mod_dc_offset     = new DC_OffsetFilter(&cfgSpectrum->cfgDCOffset);
	mod_preemphasis   = new PreEmphasis(&cfgSpectrum->cfgPreEmphasis);
	mod_advancewindow = new AdvanceWindow(&cfgSpectrum->cfgAdvanceWindow);
	mod_window        = new Window(&cfgSpectrum->cfgWindow);
	mod_lpc           = new LPC(&cfgSpectrum->cfgLPC);
	mod_fft           = new FFT(&cfgSpectrum->cfgFFT);
	mod_fft_lpc       = new FFT(&cfgSpectrum->cfgFFTlpc);
	mod_timesmooth    = new TimeSmooth(&cfgSpectrum->cfgTimeSmooth);

	// Initialize loggers
	if (bMonitor)
		InitMonitor(output_basename);
}

// Front End Destructor
Spectrum::~Spectrum ()
{
	// Release modules
	delete mod_dc_offset;
	delete mod_preemphasis;
	delete mod_advancewindow;
	delete mod_window;
	delete mod_lpc;
	delete mod_fft;
	delete mod_fft_lpc;
	delete mod_timesmooth;

	// Release loggers
	if(bMonitor)
	{
		list<WuwLogger*>::iterator it;
		for(it = m_logger.begin(); it != m_logger.end(); it++)
			delete (*it);
	}
}


void Spectrum::InitMonitor(char* output_basename)
{
	// Build file path
	char filename[_MAX_PATH];
	strcpy(filename, output_basename);
	char* ext = filename + strlen(filename);

	strcpy(ext, ".dcof");
	m_logger.push_back(new WuwLogger(filename, 1, mod_dc_offset->GetOutputBufferSize(), (void**)&pf32_dc_offset));

	strcpy(ext, ".preem");
	m_logger.push_back(new WuwLogger(filename, 1, mod_preemphasis->GetOutputBufferSize(), (void**)&pf32_preemphasis));

	strcpy(ext, ".lpc");
	m_logger.push_back(new WuwLogger(filename, 1, mod_lpc->GetOutputBufferSize(), (void**)&pf64_lpc));

	strcpy(ext, ".fft");
	m_logger.push_back(new WuwLogger(filename, 1, mod_fft->GetOutputBufferSize(), (void**)&pf32_fft));

	//wcscpy(ext, ".t1s");   m_logger[6] = new WuwLogger(filename, true);
}


void Spectrum::RunMonitor()
{
	list<WuwLogger*>::iterator it;

	for(it = m_logger.begin(); it != m_logger.end(); it++)
		(*it)->Run();
}

//FLOAT32* Spectrum::Eos(FLOAT32* in_samples)
SpectMagStr* Spectrum::Eos(FLOAT32* in_samples)
{
	return NULL;
}
//
// Return Signal Energy
//
FLOAT32 Spectrum::getEnergy() {

   return (f32_signal_energy);
}

// Main Processing Function
SpectMagStr* Spectrum::Run (FLOAT32* in_samples)
{
	i32_frame_counter++;

	pf32_dc_offset     = mod_dc_offset->Run(in_samples);
	pf32_preemphasis   = mod_preemphasis->Run(pf32_dc_offset);
	pf32_advancewindow = mod_advancewindow->Run(pf32_preemphasis);
	pf32_window        = mod_window->Run(pf32_advancewindow);
	pf64_lpc           = mod_lpc->Run(pf32_window);
   f32_signal_energy  = mod_lpc->getEnergy();
   pf32_fft_lpc       = mod_fft_lpc->Run_ComputePowerSpectrum(pf64_lpc);
	pf32_fft           = mod_fft->Run_ComputePowerSpectrum(pf32_window);
	pf32_timesmooth    = mod_timesmooth->Run(pf32_fft_lpc);

	//pf32_output        = pf32_window;
	pf32_output        = pf32_timesmooth;

	if (bMonitor)
		RunMonitor();

   m_SpectMagStr.pf32_lpc_spectmag = pf32_output;
   m_SpectMagStr.pf32_spectmag     = pf32_fft;

	//return(pf32_output);
   return(&m_SpectMagStr);
}

/*******************************  End of File  *******************************/

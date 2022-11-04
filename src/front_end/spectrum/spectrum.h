/******************************************************************************

                      Copyright (C) 2006-2015 VoiceKey Inc.
                           Cocoa Beach, Florida 32931
                                All rights reserved

                           Confidential and Proprietary

     Module: spectrum.h
     Author: Veton Kepuska
    Created: May 27, 2006
    Updated:

Description: A modified version of spectrum class definition based on 2004
             Veton Kepuska's version. Inspired by David Shipman's  model
             of modular software architecture implemented in C.

$Log:$

*******************************************************************************/
#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <list>
#include "common/wuw_config.h"
#include "common/wuw_util.h"

#include "dc_offset.h"
#include "pre_emphasis.h"
#include "advance_window.h"
#include "window.h"
#include "lpc.h"
#include "fft.h"
#include "time_smooth.h"

/*********************************  Defines  *********************************/

typedef struct SpectralFeatures {
   FLOAT32  *pf32_spectmag;
   FLOAT32  *pf32_lpc_spectmag;
} SpectMagStr;

/****************************  Class Definition  *****************************/

class Spectrum
{
public:
	Spectrum(SpectrumConfig*, char*);
	~Spectrum();

	//FLOAT32* Eos(FLOAT32 *in_samples);
	//FLOAT32* Run(FLOAT32 *in_samples);
	SpectMagStr* Eos(FLOAT32 *in_samples);
	SpectMagStr* Run(FLOAT32 *in_samples);

	void InitMonitor(char* output_basename);
	void RunMonitor();

   // Utility Functions
   FLOAT32   getEnergy();

private:
	// Parameters
	INT32 i32_frame_counter;

	// Buffers
	FLOAT32* pf32_dc_offset;
	FLOAT32* pf32_preemphasis;
	FLOAT32* pf32_advancewindow;
	FLOAT32* pf32_window;
	FLOAT64* pf64_lpc;
	FLOAT32* pf32_fft_lpc;
	FLOAT32* pf32_fft;
	FLOAT32* pf32_timesmooth;
	FLOAT32* pf32_output;

   FLOAT32  f32_signal_energy;

   SpectMagStr m_SpectMagStr;

	// Debugging Information
	bool bMonitor;
	list<WuwLogger*> m_logger;

	// Modules
	DC_OffsetFilter* mod_dc_offset;     // High Pass filter removing DC offset
	PreEmphasis*     mod_preemphasis;   // Pre-emphasis filter 6dB gain in high freq.
	AdvanceWindow*   mod_advancewindow; // Filling Windowing Buffer and Advancing
	Window*          mod_window;        // Windowing of the frame
	LPC*             mod_lpc;           // LP module computing LPC's
	FFT*             mod_fft;           // FFT module computing PowerSpectrum
	FFT*             mod_fft_lpc;       // FFT module computing PowerSpectrum from LPC
	TimeSmooth*      mod_timesmooth;    // PowerSpectrum Smoothing Filter
};

#endif //SPECTRUM_H

/*******************************  End of File  *******************************/

/******************************************************************************

	              Copyright (C) 2006-2015 VoiceKey Inc.
                           Cocoa Beach, Florida 32931
			      All rights reserved

			  Confidential and Proprietary


     Module: defines.h
     Author: Veton Kepuska
    Created: June 8, 2006
    Updated:

Description: A modified version of fft class definition based on 2004
             Veton Kepuska's version. Inspired by David Shipman's  model
             of modular software architecture implemented in C.

       $Log:$

*******************************************************************************

                     Proprietary and Confidential.
                        All rights reserved.
                           VoiceKey Inc.,
                      Cocoa Beach, Florida 32931.

******************************************************************************/

/**********************************  Notes  **********************************

Notes: Those functions were imported from existing implemenations of FFT code
       in C. To take advantage and speed up the computations those functions
       must be replaced by custom C++ implementaions. This goal woudl be one
       of the first goals when optimisation of the overall implementation is
       done. One such a source is provided in the book:
                          "C++ Alorithms for DSP"
       See under:
          C:\Documents and Settings\vkepuska\My Documents
            \C++ Alorithms for DSP\0131791443_CD\DSPC\CPP
          on the CEEC051695.fit.edu computer.


******************************************************************************/

#ifndef FFT_H
#define FFT_H

/******************************  Include Files  ******************************/

#include "common/wuw_common.h"

/*********************************  Defines  *********************************/

#define LOCAL_SIGNUM(i) (i < 0 ? -1 : i == 0 ? 0 : 1)

/*****************************  Class Definition  ****************************/

class FFT {

public:
	// Default Constructor
	FFT ();
	// constructor that initializes:
	//	allocates internal buffers for power-spectrum
	//	computation based on buffer length and fft size
	FFT (FFT_Config *fftConfig);
	//
	// Destructor
	//
	~FFT();

	// Computation of Power Spectrum
	// Input sample buffer of length buffer_length
	// Output - power spectrum
	FLOAT32 *Run_ComputePowerSpectrum (FLOAT32 *sample_buffer);
	FLOAT32 *Run_ComputePowerSpectrum (FLOAT64 *sample_buffer);
	FLOAT32 *Run_ComputeMagnitudeSpectrum (FLOAT64 *sample_buffer);
	UINT16 GetOutputBufferSize() {return out_buffer_size * sizeof(FLOAT32);};


private:
	UINT16    fft_order;	       // fft_size = 2^fft_order
	UINT16    fft_size; 	       // fft_size = 2^fft_order
	UINT16    in_buffer_size;   // Lenght of buffer in number of samples
	FLOAT64  *in_buffer;        // Input Buffer of fft_size
	UINT16    log2fft_order;    //

	UINT16    out_buffer_size;  // Lenght of buffer in number of samples
	FLOAT32  *out_spectrum;     // Buffer of output power spectrum
	FLOAT64  *tmp_fft_buffer;   // Temp Place Holder for fft computation

	FLOAT64  *w;                // cos/sin table
	SINT32   *ip;               // work area for bit reversal
                                    // necessary length is >=:
                                    // 2+(1<<(int)(log(n/2+0.5)/log(2))/2)
                                    // that can be apoximated with 2+sqrt(n/2)
                                    // wich is implemented
	//
	// Speeding up by including all local var's in here
	//
	//
	// Function Members
	//
	void   AllocBuffers();
	UINT16 fastlog2(UINT16 n);
	void   rfft (FLOAT64 *x, UINT32 n, UINT32 m);

#if 0
	void   CpxFFT(FLOAT64 *s, UINT16 n);
	void   RealFFT(FLOAT64 *s, UINT16 n);
	void   rfe_find_rmagsq(FLOAT64 *magsq, FLOAT64 *cpx_vector, UINT16 npts);

	//void  RealFFT (float *buffer, int fft_order);
	//void  rmagsq (float *out_spectrum, float *fft_buffer, int half_of_fft_order);
	//void  FFT (float *buffer, int fft_order);
	//void  rfft (float *x, int n, int m);

	void   powerspectrum(FLOAT64 *in, FLOAT64 *out, UINT16 in_buff_size, UINT16 fft_size);
	UINT16 FAST(FLOAT64 *b, UINT16 n);
	void   FR2TR(UINT16 in, FLOAT64 *b0, FLOAT64 *b1);
	void   FR4TR(UINT16 in, UINT16 nn, FLOAT64 *b0, FLOAT64 *b1, FLOAT64 *b2, FLOAT64* b3);
	void   FORD1(UINT16 m, FLOAT64 *b);
	void   FORD2(UINT16 m, FLOAT64 *b);
#endif
};

#endif //FFT_H

/*******************************  End of File  *******************************/


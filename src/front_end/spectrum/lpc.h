/******************************************************************************

                      Copyright (C) 2006-2015 VoiceKey Inc.
                           Cocoa Beach, Florida 32931
                               All rights reserved

                           Confidential and Proprietary


     Module: lpc.h
     Author: Veton Kepuska
    Created: June 7, 2006
    Updated:

Description: A modified version of lpc class. Inspired by David Shipman's
             model of modular software architecture implemented in C.

$Log:$

*******************************************************************************

Notes:

******************************************************************************/

#ifndef LPC_H
#define LPC_H

/******************************  Include Files  ******************************/

//#include <vector>
//#include <algorithm>
//using namespace std;
#include <float.h>

#include "common/wuw_config.h"

/*********************************  Defines  *********************************/

#define DEF_TINY2     (1E-12)

/*****************************  Class Definition  ****************************/

class LPC
{

public:
	LPC();                           // Default constructor
	LPC(CfgLPC* lpcConfig);       // Configurable Constructor

	~LPC();                          // Destructor

	// Function that initialisis already existing object
	void Init(CfgLPC* lpcConfig);

	void Reset();                    // Function that resets the existing object
	// data members
	void Eos();                      // Function that flashes the buffers at the
	// end of file or input data stream
	// processing
	UINT16 GetOutputBufferSize() {return lpc_order * sizeof(FLOAT64);};

	// LPC member function that performs actual linear prediction
	FLOAT64 *Run (FLOAT32 *in_samples);
	FLOAT64  autocorrelation(FLOAT32 *in_samples);
	FLOAT64 *durbin();
	FLOAT64  autocorr(FLOAT32 *in_samples);
	FLOAT64 *levinson_durbin();
	void     computeMVDR();

	// Utility Functions
	void     AllocBuff();
   FLOAT32  getEnergy();

private:
	UINT16      window_size;	     // Length of the window in number of samples
	UINT16      lpc_order;	        // Number of lpc coeffs
	FLOAT32     gain;               // Gain factor
   FLOAT64     signal_energy;      // Signal Energy
	FLOAT64    *r;                  // Reflection Coeffs
	FLOAT64    *a;                  // LP Coeffs
	FLOAT64    *tmp;                // tmp array

	FLOAT64    *mv_coeffs;          // MVDR coeffs
};

#endif  //LPC_H

/*******************************  End of File  *******************************/

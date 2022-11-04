/******************************************************************************

Copyright (C) 2006-2015 VoiceKey Inc.
Cocoa Beach, Florida 32931
All rights reserved

Confidential and Proprietary


Module: window.h
Author: Veton Kepuska
Created: May 27, 2006
Updated:

Description: A modified version of window class definition based on 2004
Veton Kepuska's version. Inspired by David Shipman's  model
of modular software architecture implemented in C.

$Log:$

*******************************************************************************

Notes: Proprietary and Confidential. All rights reserved.
VoiceKey Inc.,
Cocoa Beach, Florida 32931.

******************************************************************************/

#ifndef WINDOW_H
#define WINDOW_H

/******************************  Include Files  ******************************/

#include "common/wuw_config.h"

/*********************************  Defines  *********************************/

/*****************************  Class Definition  ****************************/

class Window {

public:
	Window();
	Window(WindowConfig *pConfig);
	~Window();

	FLOAT32* Run(FLOAT32* pf32_in_samples);
	FLOAT32* Eos(FLOAT32* pf32_in_samples);

	// Utility Functions
	void InitWindowCoefficients();

private:
	// Parameters
	WINDOW_TYPE m_window_type;    // window type
	UINT16      i16_window_size;    // window size
	FLOAT32*    pf32_window_coeffs; // window coefficients

	// Internal data
	FLOAT32*    pf32_out_samples;   // output buffer
};

#endif  //WINDOW_H

/*******************************  End of File  *******************************/

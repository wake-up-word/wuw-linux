/******************************************************************************

Copyright (C) 2006-2015 VoiceKey Inc.
Cocoa Beach, Florida 32931
All rights reserved

Confidential and Proprietary


Module: advance_window.h
Author: Veton Kepuska
Created: May 27, 2006
Updated:

Description: A modified version of advanceWindow class definition based on
2004 Veton Kepuska's version. Inspired by David Shipman's  model
of modular software architecture implemented in C.

$Log:$

*******************************************************************************

Notes: Proprietary and Confidential. All rights reserved.
VoiceKey Inc.,
Cocoa Beach, Florida 32931.

******************************************************************************/

#ifndef ADVANCE_WINDOW_H
#define ADVANCE_WINDOW_H

/******************************  Include Files  ******************************/

#include "common/defines.h"

/*********************************  Defines  *********************************/

/*****************************  Class Definition  *****************************/

class AdvanceWindow {

public:
	AdvanceWindow();
	AdvanceWindow(AdvanceWindowConfig* pConfig);
	~AdvanceWindow();

	FLOAT32* Run(FLOAT32* pf32_in_samples);
	FLOAT32* Eos(FLOAT32* pf32_in_samples);

private:
	// Parameters
	UINT16   i16_advance_size;    // window advance length in samples
	UINT16   i16_window_size;     // length of entire window

	// Internal data
	UINT16   i16_shift_size;      // number of samples to be shifted each time
	UINT16   i16_num_shifts;      // number of shifts needed to flush the buffer
	FLOAT32* pf32_out_samples;    // output buffer
};

#endif  //ADVANCE_WINDOW_H

/*******************************  End of File  *******************************/

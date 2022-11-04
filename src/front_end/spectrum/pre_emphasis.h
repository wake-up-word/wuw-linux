/******************************************************************************

Copyright (C) 2006-2015 VoiceKey Inc.
Cocoa Beach, Florida 32931
All rights reserved

Confidential and Proprietary


Module: pre_emphasis.h
Author: Veton Kepuska
Created: May 27, 2006
Updated:

Description: A modified version of pre_emphasis class definition based on 2004
Veton Kepuska's version. Inspired by David Shipman's  model
of modular software architecture originaly implemented in C.

$Log:$

*******************************************************************************

Notes: Proprietary and Confidential. All rights reserved.
VoiceKey Inc.,
Cocoa Beach, Florida 32931.

******************************************************************************/

#ifndef PRE_EMPHASIS_H
#define PRE_EMPHASIS_H

/******************************  Include Files  ******************************/

#include "common/wuw_config.h"

/*****************************  Class Definition  *****************************/

class PreEmphasis
{
public:
	PreEmphasis();
	PreEmphasis(PreEmphasisConfig *preemphConfig);
	~PreEmphasis();

	FLOAT32* Run(FLOAT32* pf32_in_samples);
	FLOAT32* Eos(FLOAT32* pf32_in_samples);
	UINT16 GetOutputBufferSize() {return i16_buffer_size * sizeof(FLOAT32);};

private:
	// Parameters
	UINT16   i16_buffer_size;    // buffer size
	FLOAT32  f32_preemp_factor;  // pre-emphasis coefficient

	// Internal data
	FLOAT32  f32_prev_sample;    // last sample of previous buffer
	FLOAT32* pf32_out_samples;   // output samples buffer

};

#endif  //PRE_EMPHASIS_H

/*******************************  End of File  *******************************/

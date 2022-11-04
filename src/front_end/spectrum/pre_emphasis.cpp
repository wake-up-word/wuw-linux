/******************************************************************************

Copyright (C) 2006-2015 VoiceKey Inc.
Cocoa Beach, Florida 32931
All rights reserved

Confidential and Proprietary


Module: pre_emphasis.cpp
Author: Veton Kepuska
Created: May 27, 2006
Updated: 

Description: A modified version of pre_emphasis class definition based on 2004 
Veton Kepuska's version. Inspired by David Shipman's  model
of modular software architecture originaly implemented in C.

$Log:$

*******************************************************************************

Notes: 

******************************************************************************/


/******************************  Include Files  ******************************/

#include "pre_emphasis.h"

/*********************************  Defines  *********************************/

/****************************  Function Members  *****************************/

// Constructor Initializes:
//        pre_emphasis filter factor &        
//        buffer_size
//
// Default Constructor sets the pre_emphasis factor to FE_PREEMPH_FACTOR and 
//  buffer size to FE_FRAME_SHIFT_SIZE
//
PreEmphasis::PreEmphasis()
{
	i16_buffer_size   = FE_FRAME_SHIFT_SIZE;
	f32_preemp_factor = FE_PREEMPH_FACTOR;
	f32_prev_sample   = 0.0F;
	pf32_out_samples  = new FLOAT32[i16_buffer_size];
}

//
// Non-Default Constructor
//
PreEmphasis::PreEmphasis(PreEmphasisConfig* pConfig)
{
	i16_buffer_size    = pConfig->buffer_size;
	f32_preemp_factor  = pConfig->preemphasis_factor;
	f32_prev_sample    = 0.0F;
	pf32_out_samples   = new FLOAT32 [i16_buffer_size];
}

//
// Destructor
//
PreEmphasis::~PreEmphasis()
{
	delete[] pf32_out_samples;
}

FLOAT32* PreEmphasis::Eos(FLOAT32* pf32_in_samples)
{
	if(pf32_in_samples)
		return Run(pf32_in_samples);
	else
		return NULL;
}

//
// Filtering Function 
//
FLOAT32* PreEmphasis::Run(FLOAT32* pf32_in_samples)
{
	// First sample in the buffer uses previous_sample stored in the objects private data memeber
	pf32_out_samples[0] = pf32_in_samples[0] - f32_preemp_factor * f32_prev_sample;

	for (int i=1; i < i16_buffer_size; i++) 
	{
		pf32_out_samples[i] = pf32_in_samples[i] - f32_preemp_factor * pf32_in_samples[i-1];
	}

	// Storing last sample in the buffer for the next call
	f32_prev_sample = pf32_in_samples[i16_buffer_size - 1];

	return pf32_out_samples;
}

/*******************************  End of File  *******************************/

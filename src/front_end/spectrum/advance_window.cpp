/******************************************************************************

Copyright (C) 2006-2015 VoiceKey Inc.
Cocoa Beach, Florida 32931
All rights reserved

Confidential and Proprietary


Module: advance_window.cpp
Author: Veton Kepuska
Created: May 27, 2006
Updated: 

Description: A modified version of advanceWindow class definition based on 
2004 Veton Kepuska's version. Inspired by David Shipman's  model
of modular software architecture originaly implemented in C.

$Log:$

*******************************************************************************

Proprietary and Confidential. All rights reserved.
VoiceKey Inc., Cocoa Beach, Florida 32931.

*******************************************************************************

Notes:
                                          |<- advance_n ->|
                                          +===============+
                              New Samples |xxxxxxxxxxxxxxx|
                                          +===============+
                                                  | Copy
                                                  V  
|<----------------------- window_n ---------------------->|
+===============+===============+=== ~ ===+===============+
|bbbbbbbbbbbbbbb|bbbbbbbbbbbbbbb|   ...   |bbbbbbbbbbbbbbb|
+===============+===============+=== ~ ===+===============+
|<- advance_n ->|<- advance_n ->|		   |<- advance_n ->|
|<----------- n_shift_samples ----------->|     
|<------------------------ n_chunks --------------------->|     

1. Shift chunks 1...n_chunks to 0...n_chunks-1
2. Copy new samples into the last chank => (n_chungs-1)

This function only fills-in new sample in the last block.

******************************************************************************/
#include "advance_window.h"
#include <string.h>


//-----------------------------------------------------------------------------
// Name: AdvanceWindow()
// Desc: Default constructor
//-----------------------------------------------------------------------------
AdvanceWindow::AdvanceWindow()
{
	i16_advance_size  = FE_FRAME_SHIFT_SIZE;
	i16_window_size   = FE_WINDOW_SIZE;
	i16_shift_size    = i16_window_size - i16_advance_size;
	i16_num_shifts    = i16_window_size / i16_advance_size / 2;
	pf32_out_samples  = new FLOAT32[i16_window_size];
	memset(pf32_out_samples, 0, sizeof(FLOAT32) * i16_window_size);
}


//-----------------------------------------------------------------------------
// Name: AdvanceWindow(AdvanceWindowConfig *pConfig)
// Desc: Constructor based on config structure
//-----------------------------------------------------------------------------
AdvanceWindow::AdvanceWindow(AdvanceWindowConfig *pConfig)
{
	i16_advance_size  = pConfig->advance_n;
	i16_window_size   = pConfig->window_n;
	i16_shift_size    = i16_window_size - i16_advance_size;
	i16_num_shifts    = i16_window_size / i16_advance_size / 2;
	pf32_out_samples  = new FLOAT32[i16_window_size];
	memset(pf32_out_samples, 0, sizeof(FLOAT32) * i16_window_size);
}


//-----------------------------------------------------------------------------
// Name: ~AdvanceWindow() 
// Desc: Destructor
//-----------------------------------------------------------------------------
AdvanceWindow::~AdvanceWindow() 
{
	delete[] pf32_out_samples;
}


//-----------------------------------------------------------------------------
// Name: Run(FLOAT32* pf32_in_samples)
// Desc: Performs main computation
//-----------------------------------------------------------------------------
FLOAT32* AdvanceWindow::Run(FLOAT32* pf32_in_samples)
{
	// Shift 'i16_shift_size' number of samples from the end to the beginning
	memmove(pf32_out_samples, pf32_out_samples + i16_advance_size, i16_shift_size * sizeof(FLOAT32)); 

	// Copy incoming 'i16_advance_size' number of samples to the end
	memcpy(pf32_out_samples + i16_shift_size, pf32_in_samples, i16_advance_size * sizeof(FLOAT32));

	return pf32_out_samples;
}


//-----------------------------------------------------------------------------
// Name: Eos(FLOAT32* pf32_in_samples)
// Desc: End of stream function - returns left over buffers
//-----------------------------------------------------------------------------
FLOAT32* AdvanceWindow::Eos(FLOAT32* pf32_in_samples)
{
	if(pf32_in_samples)
		return Run(pf32_in_samples);
	else
	{
		return NULL;	
	}
}
/*******************************  End of File  *******************************/

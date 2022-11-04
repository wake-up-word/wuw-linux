/******************************************************************************

Copyright (C) 2006-2015 VoiceKey Inc.
Cocoa Beach, Florida 32931
All rights reserved

Confidential and Proprietary


Module: dc_offset_filter.cpp
Author: Veton Kepuska
Created: May 27, 2006
Updated: 

Description: A modified version of dc_offset_filter class definition based on 
2004 Veton Kepuska's version. Inspired by David Shipman's  model
of modular software architecture originaly implemented in C.

$Log:$

*******************************************************************************

Notes: 

******************************************************************************/


/******************************  Include Files  ******************************/

#include <new>
#include <memory.h>
using namespace std;

#include "dc_offset.h"

/****************************  Function Members  *****************************/

// Default Constructor
DC_OffsetFilter::DC_OffsetFilter()
{
	// The filtering is performed on the block of samples defined by the feature vector frame-rate
	i16_buffer_size = FE_FRAME_SHIFT_SIZE;

	pf32_out_samples = new FLOAT32 [FE_FRAME_SHIFT_SIZE];
	memset(pf32_out_samples, 0, sizeof(FLOAT32)*FE_FRAME_SHIFT_SIZE);
	pf32_tmp_zeros_buff = new FLOAT32 [FE_FRAME_SHIFT_SIZE];
	memset(pf32_tmp_zeros_buff, 0, sizeof(FLOAT32)*FE_FRAME_SHIFT_SIZE);

	i16_buffs_per_win_size = FE_WINDOW_SIZE/FE_FRAME_SHIFT_SIZE/2;

	/* IIR filter: y[n]=x[n]-x[n-1]+0.999*y[n-1] */
	i16_num_yCoeffs  = 2;	
	i16_num_xCoeffs  = 2;

	pf32_xCoeffs    = new float [i16_num_xCoeffs];
	pf32_yCoeffs    = new float [i16_num_yCoeffs];

	pf32_yCoeffs[0] =  1.0F;	// This is always one and is not used in filtering
	pf32_yCoeffs[1] =  0.999F;
	pf32_xCoeffs[0] =  1.0F;
	pf32_xCoeffs[1] = -1.0F;

	f32_prev_xVal  = 0.0F;
	f32_prev_yVal  = 0.0F;

}
// Constructor Initializes:
//	Filter Coefficients
//  Filter Length
DC_OffsetFilter::DC_OffsetFilter(DC_OffsetFilterConfig *dc_offsetFilterConfig)
{
	UINT16 i;

	i16_buffer_size    = dc_offsetFilterConfig->buffer_size;
	i16_window_size    = dc_offsetFilterConfig->window_size;

	pf32_out_samples    = new FLOAT32 [i16_buffer_size];
	memset(pf32_out_samples, 0, sizeof(FLOAT32)*i16_buffer_size);
	pf32_tmp_zeros_buff = new FLOAT32 [i16_buffer_size];
	memset(pf32_tmp_zeros_buff, 0, sizeof(FLOAT32)*i16_buffer_size);

	i16_buffs_per_win_size = i16_window_size/i16_buffer_size/2;

	i16_num_xCoeffs     = dc_offsetFilterConfig->n_xCoeffs;
	i16_num_yCoeffs     = dc_offsetFilterConfig->n_yCoeffs;
	pf32_xCoeffs       = new FLOAT32 [i16_num_xCoeffs];
	pf32_yCoeffs       = new FLOAT32 [i16_num_yCoeffs];

	for (i=0; i<i16_num_xCoeffs; i++) {
		pf32_xCoeffs[i] = dc_offsetFilterConfig->xCoeffs[i];
	}
	for (i=0; i<i16_num_yCoeffs; i++) {
		pf32_yCoeffs[i] = dc_offsetFilterConfig->yCoeffs[i];
	}

	f32_prev_xVal = 0.0F;
	f32_prev_yVal = 0.0F;
}
//
// DC Offset Filter Destructor
//
DC_OffsetFilter::~DC_OffsetFilter()
{
	delete [] pf32_xCoeffs;
	delete [] pf32_yCoeffs;

	delete [] pf32_out_samples;
	delete [] pf32_tmp_zeros_buff;

}

FLOAT32* DC_OffsetFilter::Eos(FLOAT32* pf32_in_samples)
{
	if(pf32_in_samples)
	{
		return Run(pf32_in_samples);
	}
	else
	{
		return Run(pf32_tmp_zeros_buff);
	}
}
//
// DC Offset Removal Filter Implementation
//

FLOAT32* DC_OffsetFilter::Run (FLOAT32* pf32_in_samples)
{
	FLOAT32 sumX, sumY;
	UINT16 n_samples = i16_buffer_size;

	/* y[n] = x[n] - x[n-1] + 0.999 * y[n-1] */
	sumX = pf32_xCoeffs[0] * pf32_in_samples[0] + pf32_xCoeffs[1] * f32_prev_xVal;
	sumY = pf32_yCoeffs[1] * f32_prev_yVal;
	pf32_out_samples[0] = sumX + sumY;

	for(int n = 1; n < n_samples; n++) 
	{		
		sumX = 0.0F;
		sumY = 0.0F;

		for (int i = 0; i < i16_num_xCoeffs; i++) 
		{
			sumX += pf32_xCoeffs[i] * pf32_in_samples[n-i];
		}

		for (int i = 1; i < i16_num_yCoeffs; i++) 
		{
			sumY += pf32_yCoeffs[i] * pf32_out_samples[n-i];
		}

		pf32_out_samples[n] = sumX + sumY;
	}

	f32_prev_xVal = pf32_in_samples[n_samples-1];
	f32_prev_yVal = pf32_out_samples[n_samples-1];

	return pf32_out_samples;
}

/*******************************  End of File  *******************************/

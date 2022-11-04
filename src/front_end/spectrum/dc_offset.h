/******************************************************************************

Copyright (C) 2006-2015 VoiceKey Inc.
Cocoa Beach, Florida 32931
All rights reserved

Confidential and Proprietary


Module: dc_offset_filter.h
Author: Veton Kepuska
Created: May 27, 2006
Updated:

Description: A modified version of dc_offset_filter class definition based
on 2004 Veton Kepuska's version. Inspired by David Shipman's
model of modular software architecture originaly implemented in C.

$Log:$

*******************************************************************************

Notes: Proprietary and Confidential. All rights reserved.
VoiceKey Inc.,
Cocoa Beach, Florida 32931.

******************************************************************************/

#ifndef DC_OFFSET_FILTER_H
#define DC_OFFSET_FILTER_H

/******************************  Include Files  ******************************/
#include <vector>
#include <algorithm>
using namespace std;

#include "common/defines.h"

/*********************************  Defines  *********************************/

/*****************************  Class Definition  *****************************/

class DC_OffsetFilter
{
public:

	DC_OffsetFilter ();
	DC_OffsetFilter (DC_OffsetFilterConfig *dc_offsetFilterConfig);
	~DC_OffsetFilter ();

	FLOAT32* Run(FLOAT32* pf32_in_samples);
	FLOAT32* Eos(FLOAT32* pf32_in_samples);

	UINT16 GetOutputBufferSize() {return i16_buffer_size * sizeof(FLOAT32);};

private:

	UINT16   i16_buffer_size;
	UINT16   i16_window_size;
	UINT16   i16_buffs_per_win_size;

	FLOAT32* pf32_xCoeffs;    // denominator coeffs
	FLOAT32  f32_prev_xVal;   // denominator coeffs
	UINT16   i16_num_xCoeffs; // Lenght of buffer in number of samples

	FLOAT32* pf32_yCoeffs;	  // DC Filter numerator coeffs
	FLOAT32  f32_prev_yVal;   // DC Filter numerator coeffs
	UINT16   i16_num_yCoeffs; // Lenght of buffer in number of samples

	FLOAT32* pf32_tmp_zeros_buff;
	FLOAT32* pf32_out_samples;
};

#endif //DC_OFFSET_FILTER_H

/*******************************  End of File  *******************************/

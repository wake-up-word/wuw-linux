/******************************************************************************

Copyright (C) 2006-2015 VoiceKey Inc.
Cocoa Beach, Florida 32931
All rights reserved

Confidential and Proprietary


Module: time_smooth.cpp
Author: Veton Kepuska
Created: June 09, 2006
Updated: 

Description: A modified version of TimeSmooth class definition based on 2004 
Veton Kepuska's version. Inspired by David Shipman's  model
of modular software architecture originaly implemented in C.

$Log:$

*******************************************************************************

Proprietary and Confidential. All rights reserved.
VoiceKey Inc., Cocoa Beach, Florida 32931.

******************************************************************************/

/**********************************  Notes  **********************************

/******************************  Include Files  ******************************/

#include <cmath>
#include <new>
#include <iostream>
using namespace std;

#include "time_smooth.h"

/*********************************  Defines  *********************************/

/****************************  Function Members  *****************************/
// Constructor Initializes:
//        window size and window type
//        buffer_size
//
// Default Constructor sets the window type to Hamming and size to 200
//
TimeSmooth::TimeSmooth (UINT16 filterSize)
{
	vector_size   = FE_FFT_SIZE/2 + 1;
	filter_size   = filterSize;
	AllocBuffers();
	Reset();
}
//
// Non-Default Constructor
//
TimeSmooth::TimeSmooth (TimeSmoothConfig *timeSmoothConfig)
{
	vector_size   = timeSmoothConfig->vector_size + 1;
	filter_size   = timeSmoothConfig->filter_size;
	AllocBuffers();
	Init(timeSmoothConfig);
}
void TimeSmooth::AllocBuffers()
{
	UINT16 i;

	w             = new FLOAT32  [filter_size];
	out_vector    = new FLOAT32  [vector_size];
	inputs_buffer = new FLOAT32* [filter_size];
	inputs_ptr    = new FLOAT32* [filter_size];

	for (i=0; i<filter_size; i++) {
		inputs_buffer[i] = new FLOAT32 [vector_size];
		inputs_ptr[i]    = inputs_buffer[i];
		memset(inputs_ptr[i], 0, sizeof(FLOAT32)*vector_size);
	}
	index = filter_size/2;
}
//
// Destructor
//
TimeSmooth::~TimeSmooth() 
{
	UINT16 i;

	delete [] w;
	delete [] out_vector;
	for (i=0; i<filter_size; i++) {
		delete [] inputs_buffer[i];
	}
   delete [] inputs_ptr;
	delete [] inputs_buffer;

}
//
// Init TimeSmooth Function on already alocated object
//
void TimeSmooth::Init(TimeSmoothConfig *timeSmoothConfig)
{
	UINT16 i;
	//
	// Setting Private Data Members
	//
	if (timeSmoothConfig->filter_size != filter_size) {
		filter_size = timeSmoothConfig->filter_size;
		delete [] w;
		w = new FLOAT32 [filter_size];
	}

	if (timeSmoothConfig->w == NULL) {
		Reset();
	}
	else { // Copying the filter coeffs from config data
		for (i=0; i<filter_size; i++) {
			w[i] = timeSmoothConfig->w[i];
		}
	}
}
//
// Reset TimeSmooth Function
//
void TimeSmooth::Reset()
{
	UINT16 i;

	switch (filter_size) 
	{
	case 3:
      // Disabling Smoothing but preserving filter GAIN
		w[0] = 0.0F; //1.0F;
		w[1] = 5.0F; //3.0F;
		w[2] = 0.0F; //1.0F;
		break;
	default:
		// Must implement the Pascal triangle number system
		//                         1
		//                       1   1
		//                     1   2   1
		//                   1   3   3   1
		//                 1   4   6   4   1
		//               1   5   10  10  5   1
		break;
	}
}
//
// Filtering Function 
//
FLOAT32 *TimeSmooth::Run(FLOAT32 *in_vector)
{
	UINT32   i, j;
	FLOAT32  sum;
	FLOAT32 *tmp_ptr;
	FLOAT32 *target = inputs_ptr[index++];
	//
	// Loading input vector into appropriate buffer slot pointed to by target
	//
	for (i=0; i<vector_size; i++) {
		target[i] = in_vector[i];
	}
	//
	// Performing smoothing of each element of the vector when buffer is full
	//
	if (index > filter_size-1) 
	{
		for (i=0; i<vector_size; i++)  // frequency axis
		{   
			sum = 0.0F;
			// cross-hair median filtering -- to be implemented
			for (j=0; j<filter_size; j++) { // time axis
				sum += inputs_ptr[j][i];
			}
			out_vector[i] = sum;
		}
		//
		// adjust the pointers for the next vector by rotating them
		//
		tmp_ptr = inputs_ptr[0];
		for (j=1; j<filter_size; j++) {
			inputs_ptr[j-1] = inputs_ptr[j];
		}
		inputs_ptr[filter_size-1] = tmp_ptr;

		index = filter_size - 1;
		
	}
	else // Do nothing
	{
		memcpy(out_vector, in_vector, vector_size*sizeof(FLOAT32));
	}

	return out_vector;
}
//
// EOF Function
//
FLOAT32 *TimeSmooth::Eos() 
{
	UINT16 i;

	if (index > filter_size/2) {
		index--;
		return(Run(zeros_vector));
	}
	else {
		return(NULL);
	}

}

/*******************************  End of File  *******************************/

/******************************************************************************

Copyright (C) 2006-2015 VoiceKey Inc.
Cocoa Beach, Florida 32931
All rights reserved

Confidential and Proprietary

Module: process_in_samples.h
Author: Veton Kepuska
Created: May 31, 2006
Updated: 

Description: A modified version of process_in_samples class definition based 
on 2004 Veton Kepuska's version. Inspired by David Shipman's  model
of modular software architecture implemented in C.

$Log:$

*******************************************************************************

Notes: 

******************************************************************************/

#ifndef PROCESS_INPUT_SAMPLES_H
#define PROCESS_INPUT_SAMPLES_H

/******************************  Include Files  ******************************/

#include <vector>
using namespace std;
#include <algorithm>

#include "common/wuw_common.h"

/*********************************  Defines  *********************************/
static const FLOAT32 f32_ulaw2f[256] = 
{
	#include "ulaw_table.h"
};

static const FLOAT32 f32_alaw2f[256] = 
{
	#include "ulaw_table.h"
};

/*****************************  Class Definition  *****************************/

class ProcessInputSamples {

public:
	ProcessInputSamples();                      // Default Constructor
	ProcessInputSamples(CfgInput* pConfig); // Constructor with config structure
	~ProcessInputSamples();                     // Destructor

	void     Init(CfgInput* pConfig);
	void     Reset();
	void     Eos();

	FLOAT32 *Run(void *in_samples);

private:
	//
	// Member objects are created through composition that make-up FE Processing
	// 
	SAMPLE_TYPE  sampleType;
	UINT16       in_sampleRate;
	UINT16       bufferSize;
	FLOAT32     *outSamples;





};

#endif // PROCESS_INPUT_SAMPLES

/*******************************  End of File  *******************************/

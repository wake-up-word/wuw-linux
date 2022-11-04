/******************************************************************************

                    Copyright (C) 2006-2015 VoiceKey Inc.
                        Cocoa Beach, Florida 32931
                           All rights reserved

                      Confidential and Proprietary


    Module: process_in_samples.cpp
    Author: Veton Kepuska
   Created: May 31, 2006
   Updated: 

Description: A modified version of process_in_samples class definition 
             based on 2004 Veton Kepuska's version. Inspired by David 
             Shipman's  model of modular software architecture originaly 
             implemented in C.

$Log:$

*******************************************************************************

Notes: 

  +-----------------------------------------+      F - Front
  | Conversion Module                       |      E - End
  |                                         |      F - Features
  |       +--------+       		           |
--------->| ulaw2f |----+                   |
  | ulaw  +--------+  float		           |
  | samples           samples	              |
  |       +--------+      |                 |       +-----------+     
--------->| alaw2f |----+-------------------------->| front_end |------->
  | alaw  +--------+  float     8khz        |       +-----------+ FEF 
  | samples           samples   sampling    |              |  
  |       +--------+    |	      rate       | FLOAT32      |
---XXXXX->| piby2f |----+		              |              +------------->
  | s16   +--------+  float		           |              Other Outputs
  | samples           samples	              |              for Monitoring
  |       +--------+    |		              |
--------->| lin2f  |----+		              |
  | lin   +--------+  float  	              |
  | samples           samples  	           |
  |                                         |
  +-----------------------------------------+

******************************************************************************/


/******************************  Include Files  ******************************/

#include <cmath>
#include <new>
#include <iostream>
using namespace std;

#include "process_in_samples.h"

/*********************************  Defines  *********************************/

/****************************  Function Members  *****************************/
//
// Default Constructor sets the window type to Hamming and size to 200
//
ProcessInputSamples::ProcessInputSamples ()
{
	Reset();
}

//
// Non-Default Constructor
//
ProcessInputSamples::ProcessInputSamples(CfgInput* pConfig)
{
	outSamples = NULL;
	Init(pConfig);
}

//
// Init ProcessInputSamples Function on already alocated object
//
void ProcessInputSamples::Init(CfgInput* pConfig)
{
	sampleType    = pConfig->sampleType;
	in_sampleRate = pConfig->sampleRate;
	bufferSize    = pConfig->bufferSize;
	outSamples    = new FLOAT32 [bufferSize];

	if (outSamples)
		delete[] outSamples;

	outSamples = new FLOAT32 [bufferSize];
}

//
// Reset ProcessInputSamples Function
//
void ProcessInputSamples::Reset()
{
	sampleType    = ST_ULAW;
	in_sampleRate = FE_SAMPLE_RATE;
	bufferSize    = FE_FRAME_SHIFT_SIZE;
	outSamples    = new FLOAT32 [FE_FRAME_SHIFT_SIZE];  
}

//
// Destructor
//
ProcessInputSamples::~ProcessInputSamples() 
{
	delete [] outSamples;
}

//
// Conversion Function - Must be optimized so that switch statement
// is not done for every buffer rather it sets the conversion table
// in the constructor
//
FLOAT32* ProcessInputSamples::Run(void *in_samples)
{
	UINT16 i;

	switch (sampleType) 
	{
		case ST_ULAW:
		{
			for (i=0; i<bufferSize; i++) 
			{
				outSamples[i] = f32_ulaw2f[((UCHAR *)in_samples)[i]];
			}
			break;
		}

		case ST_ALAW:
		{
			for (i=0; i<bufferSize; i++) 
			{
				outSamples[i] = f32_alaw2f[((UCHAR *)in_samples)[i]];
			}
			break;
		}

		case ST_SINT16:
		{
			for (i=0; i<bufferSize; i++) 
			{
				outSamples[i] = (FLOAT32)((SINT16 *)in_samples)[i];
			}
			break;
		}

		default:
		{
			cerr << "Unsupported Conversion for Sample of Type <***> " 
				<< sampleType << " <***>" << endl
				<< "ST_ULAW    => (" << ST_ULAW    << ")\t" 
				<< "ST_ALAW    => (" << ST_ALAW    << ")\t"
				<< "ST_SCHAR   => (" << ST_SCHAR   << ")\t"
				<< "ST_UCHAR   => (" << ST_UCHAR   << ")" << endl
				<< "ST_UINT8   => (" << ST_UINT8   << ")\t" 
				<< "ST_SINT8   => (" << ST_SINT8   << ")\t" 
				<< "ST_UINT16  => (" << ST_UINT16  << ")\t" 
				<< "ST_SINT16  => (" << ST_SINT16  << ")" << endl
				<< "ST_UINT32  => (" << ST_UINT32  << ")\t" 
				<< "ST_SINT32  => (" << ST_SINT32  << ")\t" 
				<< "ST_FLOAT32 => (" << ST_FLOAT32 << ")\t" 
				<< "ST_FLOAT64 => (" << ST_FLOAT64 << ")" << endl;
		}
	}
	return(outSamples);
}
//
// End-Of-Stream Function
//
void ProcessInputSamples::Eos() 
{

}

/*******************************  End of File  *******************************/

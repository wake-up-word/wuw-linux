/******************************************************************************

 	              Copyright (C) 2006-2015 VoiceKey Inc.
                           Cocoa Beach, Florida 32931
			      All rights reserved

			  Confidential and Proprietary


     Module: time_smooth.h
     Author: Veton Kepuska
    Created: June 09, 2006
    Updated:

Description: A modified version of TimeSmooth class definition based on 2004
             Veton Kepuska's version. Inspired by David Shipman's  model
             of modular software architecture implemented in C.

       $Log:$

*******************************************************************************

Notes: Proprietary and Confidential. All rights reserved.
       VoiceKey Inc.,
       Cocoa Beach, Florida 32931.

******************************************************************************/

#ifndef TIME_SMOOTH_H
#define TIME_SMOOTH_H

/******************************  Include Files  ******************************/

#include "common/wuw_config.h"

/*********************************  Defines  *********************************/

/*****************************  Class Definition  ****************************/

class TimeSmooth {

public:
  TimeSmooth (UINT16);                   // Default constructor
  // Configurable Constructor
  TimeSmooth (TimeSmoothConfig *timeSmoothConfig);
  ~TimeSmooth();                         // Destructor

  // Function that initialisis already existing object
  void Init(TimeSmoothConfig *timeSmoothConfig);

  void Reset();            // Function that resets the existing object
                           // data members
  FLOAT32 *Eos();          // Function that flashes the buffers at the
                           // end of file or input data stream
                           // processing

  // TimeSmoothing member function that performs actual filtering
  FLOAT32 *Run (FLOAT32 *in_vector);

  // Utility Functions

private:
  UINT16    vector_size;   // Lenght of the window in number of samples
  UINT16    filter_size;   // Lenght of the window in number of samples
  UINT16    index;
  FLOAT32  *w;             // TimeSmoothing Coeffs of the filter
  FLOAT32  *out_vector;    // Output Feature Vector
  FLOAT32  *zeros_vector;  // Output Feature Vector
  FLOAT32 **inputs_buffer; // Buffering is required to perform filtering
  FLOAT32 **inputs_ptr;    // Working Buffer of the Pointers to inputs_buffer

  // Utility Functions
  void AllocBuffers();
};

#endif  //WINDOW_H

/*******************************  End of File  *******************************/

/******************************************************************************

 	              Copyright (C) 2006-2015 VoiceKey Inc.
                           Cocoa Beach, Florida 32931
			      All rights reserved

			  Confidential and Proprietary


     Module: log.h
     Author: Veton Kepuska
    Created: September 15, 2006
    Updated:

Description: log module that computes natural log of mel-filtred spectrum vector.

             Software architecture was inspired by David Shipman's  model
             of modular software architecture that originaly David has
             implemented in C and I (Veton Kepuska) have extended and used.

       $Log:$

*******************************************************************************

            Proprietary and Confidential. All rights reserved.
                VoiceKey Inc., Cocoa Beach, Florida 32931.

******************************************************************************/

#ifndef LOG_H
#define LOG_H

/******************************  Include Files  ******************************/

#include "common/wuw_common.h"

/*********************************  Defines  *********************************/

/*****************************  Class Definition  ****************************/

class VLOG {

public:
  // Default Constructor
  VLOG ();
  // constructor that initializes:
  // vector size and energy floor of the module
  //VLOG (int in_vec_size, float EnergyFloor_log);
  VLOG (VLOG_Config *vlogConfig);
  // Default Desctructor
  ~VLOG ();

  // Function that resets already exisiting object's data members
  // to default values
  void Reset();

  // Function that initializes already exisiting object
  void Init(VLOG_Config *mfccConfig);

  // End Of Stream Function that flushes the buffers after end of the input
  // stream
  FLOAT32 *Eos();

  // Computation of LOG of input vector
  // Input:  Mel Spectrum Vector of lenght in_vec_size
  // Output: Loged Input Vector of the same size in_vec_size
  FLOAT32 *Run (FLOAT32 *melspec_vec);

private:
  UINT16        in_vec_size;      // size of mel spectrum vector
  UINT16        out_vec_size;     // size of mel spectrum vector

  FLOAT32       Energy_Floor_log;
  FLOAT32       Energy_Floor;

  FLOAT32      *in_vec;	          // Input vector log melspec_buffer
  FLOAT32      *out_vec;          // Output vector of dicrete transform transform
};

#endif    // LOG_H

/*******************************  End of File  *******************************/

/******************************************************************************

 	              Copyright (C) 2006-2015 VoiceKey Inc.
                           Cocoa Beach, Florida 32931
			      All rights reserved

			  Confidential and Proprietary


     Module: enh.h
     Author: Veton Kepuska
    Created: June 21, 2006
    Updated:

Description: A modified version of Enhance class definition based on 2004
             Veton Kepuska's version. Inspired by David Shipman's  model
             of modular software architecture implemented in C.

       $Log:$

*******************************************************************************

Notes:
           Proprietary and Confidential. All rights reserved.
              VoiceKey Inc., Cocoa Beach, Florida 32931.

******************************************************************************/

#ifndef ENHANCE_H
#define ENHANCE_H

/******************************  Include Files  ******************************/

#include <stdio.h>
#include <string.h>
#include "common/wuw_common.h"

/*********************************  Defines  *********************************/

/*****************************  Class Definition  ****************************/

class Enhance {

public:
  //Enhance (UINT16 cSize, UINT16 nSize);          // Default constructor
  Enhance ();          // Default constructor
  // Configurable Constructor
  Enhance (EnhanceConfig *enhanceConfig);

  ~Enhance();                                    // Destructor

  // Function that initialisis already existing object
  void Init(EnhanceConfig *enhanceConfig);

  void Reset();            // Function that resets the existing object
                           // data members
  FLOAT32 *Eos();          // Function that flushes the buffers at the
                           // end of file or input data stream
                           // processing

  // Enhancment member function that performs actual non-linear enhancement
  // FLOAT32 *Run (DELAYED_FEATURES *fea_struct);
  FLOAT32 *Run (FLOAT32 *spectrum, FLOAT32 *ma_spectrum_estimate);

  // Utility Functions
  void     AllocBuffers();
  void     FreeBuffers();
  FLOAT32 *SuppressNoise(FLOAT32 *in_vector, FLOAT32 *background_estm);
  FLOAT32 *EnhanceSpecVector(FLOAT32 *in_vector, FLOAT32 *background_estm);

private:
  UINT16    spec_vector_size;   // Length of the input vector
  FLOAT32   alpha;
  FLOAT32  *ma_spec_estimate;
  FLOAT32  *spec_vector;
  FLOAT32  *temp_spec_vector;

  UINT16    center_size;   // Length of the center size
  UINT16    neighb_size;   // Length of the neighborhood size
  UINT16    half_neighb_size;   // Half-Length of the neighborhood size
  UINT16    index;
  // Constant Parameters
  FLOAT32   sil_en_floor;  // Silence Energy Floor
  FLOAT32   sf;            // Silence Factor
  FLOAT32   nf;            // Neighborhood Factor
  FLOAT32   bg;            // Noise Gain Factor
  FLOAT32   eg;            // Enhacement Module Gain Factor

  FLOAT32   normz_term;    //
  FLOAT32   local_sum;
  FLOAT32  *neighborhood_sum; // Local Neighborhood Sum
  FLOAT32  *denom;         // Denominator
  FLOAT32  *out_spec_vector;  // Output Feature Vector
  FLOAT32  *zeros_vector;  // Vector of zeros

};

#endif  //ENHANCE_H

/*******************************  End of File  *******************************/

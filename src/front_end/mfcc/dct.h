/******************************************************************************

 	              Copyright (C) 2006-2015 VoiceKey Inc.
                           Cocoa Beach, Florida 32931
			      All rights reserved

			  Confidential and Proprietary


     Module: dct.h
     Author: Veton Kepuska
    Created: September 16, 2006
    Updated:

Description: DCT - Discrete Cosine Transform  module that computes real
             fourier transform of log of mel-filtered magnitude spectrum
             vector.

             Software architecture was inspired by David Shipman's  model
             of modular software architecture that originaly David has
             implemented in C and I (Veton Kepuska) have extended and used.

       $Log:$

*******************************************************************************

            Proprietary and Confidential. All rights reserved.
                VoiceKey Inc., Cocoa Beach, Florida 32931.

******************************************************************************/

#ifndef DCT_TRANSFORM_H
#define DCT_TRANSFORM_H

/******************************  Include Files  ******************************/

#include "common/wuw_config.h"
#include "common/wuw_util.h"
#include "common/wuw_common.h"

/*********************************  Defines  *********************************/

/*****************************  Class Definition  ****************************/

class DCT {

public:
  // Default Constructor
  DCT ();
  // constructor that initializes:
  //	DCT that is not of defautl
  DCT (DCT_Config *dctConfig);
  // Default Desctructor
  ~DCT ();

  // Function that resets already exisiting object's data members
  // to default values
  void Reset();

  // Function that initializes already exisiting object
  void Init(DCT_Config *dctConfig);

  // End Of Stream Function that flushes the buffers after end of the input
  // stream
  FLOAT32 *Eos();

  // Computation of Discrete Cosine Transform
  // Input:  log of Mel Spectrum Vector of lenght melspec_vec_size
  // Output: Transformed Input Vector of size dct_vec_size
  FLOAT32 *Run (FLOAT32 *melspec_buffer);

  // Debugging Functions
   void PrintDCTMatrix(WuwLogger *wuwLogger);

private:

  UINT16   in_vec_size;	// size of log mel spectrum vector
  UINT16   out_vec_size;// size of discrete trasform vector

  FLOAT32 *in_vec;	// Input vector log melspec_buffer
  FLOAT32 *out_vec;	// Output vector of dicrete transform transform

  FLOAT32 *DCTMx;	// DCT Matrix/Vector
  FLOAT32 *DCTbias;

  FLOAT32 *InitDCTMatrix(UINT16 NumCepstralCoeff, UINT16 NumChannels);
  void     DCTransform(FLOAT32 *outData, FLOAT32 *inData, FLOAT32 *Mx,
		       FLOAT32 *DCTbias, UINT16 NumCepstralCoeff, UINT16 NumChannels);

};

#endif

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

/******************************  Include Files  ******************************/

#include <new>
#include <cmath>
#include <memory.h>

#include "dct.h"

/*********************************  Defines  *********************************/

/****************************  Function Members  *****************************/
//
// Default Constructor 
//
DCT::DCT()
{
   UINT16 i;

   in_vec_size  = NUM_MEL_FILTERS;   //25;	// this has to be configured automaticaly 
   // from the size of output vector connecting 
   // to this module: e.g. FE Config Module
   out_vec_size = NUM_DCT_ELEMENTS; //13;// c1..c12 + c0

   out_vec      = new FLOAT32 [out_vec_size];
   for (i=0; i<out_vec_size; i++) {
      out_vec[i] = 0.0F;
   }

   DCTMx = InitDCTMatrix(out_vec_size, in_vec_size);

}
//
// Constructor that initializes non-defualt parameters 
//
DCT::DCT(DCT_Config *dctConfig)
{
   UINT16 i;

   in_vec_size  = dctConfig->in_vec_size;	// this has to be configured automaticaly from the size 
   // of output vector connecting to this module FE Config Module
   out_vec_size = dctConfig->out_vec_size; // c1..c12 + c0

   out_vec      = new FLOAT32 [out_vec_size];
   for (i=0; i<out_vec_size; i++) {
      out_vec[i] = 0.0F;
   }

   DCTMx = InitDCTMatrix(out_vec_size, in_vec_size);

}
//
// Reset Function
//
void DCT::Reset()
{
   UINT16 i;

   delete [] out_vec;

   in_vec_size  = NUM_MEL_FILTERS;  //25;// this has to be configured automaticaly 
   // from the size of output vector connecting 
   // to this module: e.g. FE Config Module
   out_vec_size = NUM_DCT_ELEMENTS; //13;// c1..c12 + c0

   out_vec      = new FLOAT32 [out_vec_size];
   for (i=0; i<out_vec_size; i++) {
      out_vec[i] = 0.0F;
   }

   DCTMx = InitDCTMatrix(out_vec_size, in_vec_size);
}
//
// Init Function
//
void DCT::Init(DCT_Config *dctConfig)
{
   UINT16 i;

   delete [] out_vec;

   in_vec_size  = dctConfig->in_vec_size;	// this has to be configured automaticaly from the size 
   // of output vector connecting to this module FE Config Module
   out_vec_size = dctConfig->out_vec_size;       // c1..c12 + c0

   out_vec      = new FLOAT32 [out_vec_size];
   for (i=0; i<out_vec_size; i++) {
      out_vec[i] = 0.0F;
   }

   DCTMx = InitDCTMatrix(out_vec_size, in_vec_size);
}
// Default Destructor 
//
DCT::~DCT()
{
   delete [] out_vec;
   delete [] DCTMx;
}
//
// Computation of DCT Transform
//
FLOAT32 *DCT::Run (FLOAT32 *in_vec)
{
   DCTransform(out_vec, in_vec, DCTMx, DCTbias, out_vec_size, in_vec_size);

   return (out_vec);
}
/*---------------------------------------------------------------------------
 * FUNCTION NAME: InitDCTMatrix
 *
 * PURPOSE:       Initializes matrix for DCT computation (DCT is implemented
 *                as matrix-vector multiplication). The DCT matrix is of size
 *                (NumCepstralCoeff-1)-by-NumChannels. The zeroth cepstral
 *                coefficient is computed separately (needing NumChannels
 *                additions and only one multiplication), so the zeroth row
 *                of DCT matrix corresponds to the first DCT basis vector, the
 *                first one to the second one, and so on up to
 *                NumCepstralCoeff-1.
 *
 * INPUT:
 *   NumCepstralCoeff
 *                Number of cepstral coeffficients
 *   NumChannels  Number of filter bank channels
 *
 * OUTPUT
 *   none
 *
 * RETURN VALUE
 *                Pointer to the initialized DCT matrix
 *---------------------------------------------------------------------------*/
FLOAT32 *DCT::InitDCTMatrix (UINT16 NumCepstralCoeff, UINT16 NumChannels)
{
   UINT16 i, j;
   FLOAT32 *Mx;

   /* Allocating memory for DCT-matrix */
   Mx = new FLOAT32 [(NumCepstralCoeff-1)*NumChannels];

   /* Computing matrix entries */
   for (i=1; i<NumCepstralCoeff; i++) {
      for (j=0; j<NumChannels; j++) {
         Mx[(i-1)*NumChannels+j] = (FLOAT32) cos (PI*i/NumChannels*(j+0.5));
      }
   }

   return Mx;
}
void DCT::PrintDCTMatrix(WuwLogger *wuwLogger) 
{
   //for (int i=0, count=1; i<out_vec_size*in_vec_size; i++) {
   //   if (i%in_vec_size == 0)
   //      wprintf_s(L"===== C%d =====\n", count++);
   //   wprintf_s(L"[%02d] %+f\n", i%in_vec_size, DCTMx[i]);
   //}
   wuwLogger->WriteBinary(DCTMx);
}
/*---------------------------------------------------------------------------
 * FUNCTION NAME: DCT
 *
 * PURPOSE:       Computes DCT transformation of filter bank outputs, results
 *                in cepstral coefficients. The DCT transformation is
 *                implemented as matrix-vector multiplication. The zeroth
 *                cepstral coefficient is computed separately and appended.
 *                Final cepstral coefficient order is c1, c2, ...,c12, c0. The
 *                output is stored right after the input values in the memory.
 *                Since the mel filter bank outputs are stored at the beginning
 *                of the FFT magnitude array it shouldn`t cause any problems.
 *                Some memory saving can be done this way.
 *
 * INPUT:
 *   Data         Pointer to input data buffer (filter bank outputs)
 *   Mx           Pointer to DCT matrix
 *   NumCepstralCoeff
 *                Number of cepstral coefficients
 *   NumChannels  Number of filter bank channels
 *
 * OUTPUT
 *                Cepstral coefficients stored after the input filter bank
 *                values pointed to by *Data*
 *
 * RETURN VALUE
 *   none
 *---------------------------------------------------------------------------*/
void DCT::DCTransform(FLOAT32 *outData, FLOAT32 *inData, FLOAT32 *Mx, 
                      FLOAT32 *DCTbias, UINT16 NumCepstralCoeff, UINT16 NumChannels)
{
   UINT16 i, j;
   FLOAT32 sum = 0.0F;

   /* Computing c0, as the last element of output vector */
   for (i=0, sum = 0.0F; i<NumChannels; i++)
      sum += inData[i];

   // Computing c1..c[NumCepstralCoeff-1], 
   // Storing result in outData vector
   if (sum > 0) {
      //
      // This test has beed added to check for c0 value if it is too small < 0.0
      // to avoid meanigless computation of the remaining features
      //
      for (i=0; i<NumCepstralCoeff-1; i++) {
         outData[i] = 0.0F;
         //outData[i] = DCTbias[i];	
         for (j=0; j<NumChannels; j++)
            outData[i] += inData[j]*Mx[i*NumChannels+j];
      }
      // Last element of the vector is sum of log of the energy of the signal 
      outData[NumCepstralCoeff-1] = sum;
   }
   else {
      outData[NumCepstralCoeff-1] = 0.0F;
      for (i=0; i<NumCepstralCoeff; i++)
         outData[i] = 0.0F;
   }
}

/*******************************  End of File  *******************************/

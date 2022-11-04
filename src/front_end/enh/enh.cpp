/******************************************************************************

 	              Copyright (C) 2006-2015 VoiceKey Inc.
                           Cocoa Beach, Florida 32931
			      All rights reserved

			  Confidential and Proprietary


     Module: enh.cpp
     Author: Veton Kepuska
    Created: June 21, 2006
    Updated: 

Description: A modified version of Enhance class definition based on 2004 
             Veton Kepuska's version. Inspired by David Shipman's  model
             of modular software architecture originaly implemented in C.

       $Log:$

*******************************************************************************

            Proprietary and Confidential. All rights reserved.
                VoiceKey Inc., Cocoa Beach, Florida 32931.
       
******************************************************************************/

/**********************************  Notes  **********************************

/******************************  Include Files  ******************************/

#include <cstdlib>
#include <cmath>
#include <new>
#include <iostream>
using namespace std;

#include "enh.h"

/*********************************  Defines  *********************************/

/****************************  Function Members  *****************************/
//
// Default Constructor
//
Enhance::Enhance()
{
  spec_vector_size = FE_FFT_SIZE/2 + 1;
  center_size      = CENTER_SIZE;        // 31.25Hz =>  1 Bin resolution
  half_neighb_size = NEIGHBORHOOD_SIZE;  // ~150Hz  =>  5 for 8000Hz sample rate and 256 FFT -- formant bandwidth
  neighb_size      = 2*half_neighb_size; // ~300Hz  => 10

  sil_en_floor     = SIL_EN_FLOOR;       // This must be increased ????
  sf               = SF;
  sf              /= spec_vector_size;   // Silence Factor - weight factor of the total energy of the signal
  nf               = NF;
  nf              /= neighb_size;        // Normalized Neighborhood Factor
  bg               = BG;                 // Background Energy Gain Factor
  eg               = EG;                 // Enhacement Module Gain Factor

  alpha            = ALPHA;               // Noise suppression factor

  AllocBuffers();  
}
//
// Configurable Constructor
//
Enhance::Enhance(EnhanceConfig *enhanceConfig)
{
  spec_vector_size = enhanceConfig->spec_vector_size + 1;
  center_size      = enhanceConfig->center_size;
  neighb_size      = 2*enhanceConfig->neighb_size;
  half_neighb_size = neighb_size/2;

  sil_en_floor     = enhanceConfig->sil_en_floor; // This must be increased
  sf               = enhanceConfig->sf;  // Silence Factor - weight factor of the total energy of the signal
  nf               = enhanceConfig->nf;  // Normalized Neighborhood Factor
  bg               = enhanceConfig->bg;  // Background Energy Gain Factor
  eg               = enhanceConfig->eg;  // Enhacement Module Gain Factor

  alpha            = enhanceConfig->alpha; // Noise suppression factor
  
  AllocBuffers();

}
//
// Allocation of Internal Buffers
//
void Enhance::AllocBuffers()
{
  UINT32  c = 0;

  temp_spec_vector = new FLOAT32 [spec_vector_size];
  out_spec_vector  = new FLOAT32 [spec_vector_size];
  zeros_vector     = new FLOAT32 [spec_vector_size];
  neighborhood_sum = new FLOAT32 [spec_vector_size];
  denom            = new FLOAT32 [spec_vector_size];

  memset(out_spec_vector, c, spec_vector_size*sizeof(FLOAT32));
  memset(zeros_vector, c, spec_vector_size*sizeof(FLOAT32));
  memset(neighborhood_sum, c, spec_vector_size*sizeof(FLOAT32));
  memset(denom, c, spec_vector_size*sizeof(FLOAT32));
}
//
// Free Buffers
//
void Enhance::FreeBuffers() {

  if (temp_spec_vector) 
    delete [] temp_spec_vector;
  if (out_spec_vector) 
    delete [] out_spec_vector;
  if (zeros_vector)
    delete [] zeros_vector;
  if (neighborhood_sum)
    delete [] neighborhood_sum;
  if (denom)
    delete [] denom;
}
//
// Reset Function
//
void Enhance::Reset() {

  FreeBuffers();

  spec_vector_size = FE_FFT_SIZE/2 + 1; 
  center_size      = CENTER_SIZE;        // 31.25Hz =>  1 Bin resolution
  half_neighb_size = NEIGHBORHOOD_SIZE;  // ~150Hz  =>  5 for 8000Hz sample rate and 256 FFT -- formant bandwidth
  neighb_size      = 2*half_neighb_size; // ~300Hz  => 10

  sil_en_floor     = SIL_EN_FLOOR;       // This must be increased ????
  sf               = SF;
  sf              /= spec_vector_size;   // Silence Factor - weight factor of the total energy of the signal
  nf               = NF;
  nf              /= neighb_size;        // Normalized Neighborhood Factor
  bg               = BG;                 // Neighborhood Gain Factor
  eg               = EG;                 // Enhacement Module Gain Factor

  alpha            = ALPHA;               // Noise suppression factor

  AllocBuffers();
}
//
// Configurable Constructor
//
void Enhance::Init(EnhanceConfig *enhanceConfig)
{
  FreeBuffers();

  spec_vector_size = enhanceConfig->spec_vector_size + 1;
  center_size      = enhanceConfig->center_size;
  neighb_size      = 2*enhanceConfig->neighb_size;
  half_neighb_size = neighb_size/2;

  sil_en_floor     = enhanceConfig->sil_en_floor; // This must be increased
  sf               = enhanceConfig->sf; // Silence Factor - weight factor of the total energy of the signal
  nf               = enhanceConfig->nf; // Normalized Neighborhood Factor
  bg               = enhanceConfig->bg; // Neighborhood Gain Factor
  eg               = enhanceConfig->eg; // Enhacement Module Gain Factor
  
  alpha            = enhanceConfig->alpha; // Noise suppression factor

  AllocBuffers();
}
//
// Destructor
//
Enhance::~Enhance() {
  FreeBuffers();
}
//
// Run Function
//
//FLOAT32 *Enhance::Run (FEATURES_STATS *fea_struct)
FLOAT32 *Enhance::Run (FLOAT32 *in_spec_vector, FLOAT32 *in_ma_spec_estimate)
{
  if (in_spec_vector == NULL)
    return (NULL);

  return(EnhanceSpecVector(in_spec_vector, in_ma_spec_estimate));

}
//
// Enhancement Procedure on the Spectrum
//
FLOAT32* Enhance::EnhanceSpecVector(FLOAT32 *in_vector, FLOAT32 *background_estm) 
{
  SINT16      i, j, k, indx1, indx2;

  // Supressing effects of the background -- sets the result in temp_spec_vector
  SuppressNoise(in_vector, background_estm);

  // "Silence Factor" Computation
  for (i=0, normz_term = 0.0F; i<spec_vector_size; i++)
    normz_term += temp_spec_vector[i];

  if (normz_term > 0) {
     normz_term = sf*normz_term + sil_en_floor;

     // Computation of initial neighborhood sum for bin i=0
     for (j=1, local_sum = temp_spec_vector[0]; j<=half_neighb_size; j++)
       local_sum += temp_spec_vector[j];

     neighborhood_sum[0] = local_sum;
     //
     //            N/2
     // Computing Sum X[j+i]
     //           j=-N/2 
     //           j!=0
     //
     //    half_neighb_size  half_neighb_size
     // 0 2 4    |<------->i<------->|                           spec_vector_size-1
     // ^++++++++|----%----#----%----|+++++++++++++++++++++++++++^
     //               k         j
     //   
     for (i=1; i<spec_vector_size; i++) {
        j = i + half_neighb_size;
        k = i - half_neighb_size-1;
        //
        // Handling edge effects
        //
        if (j>=spec_vector_size) indx1 = spec_vector_size-1;
        else                indx1 = j;
        if (k<0)            indx2 = 0;
        else                indx2 = k;
        // Adding New Element - Dropping Old one from local
        local_sum += temp_spec_vector[indx1] - temp_spec_vector[indx2];
        // Removing Center Element from local_sum
        neighborhood_sum[i] = local_sum - temp_spec_vector[i];
     }

     // Computing denominator
     for (i=0; i<spec_vector_size; i++) {
        denom[i] = normz_term + nf*neighborhood_sum[i] + bg*background_estm[i];
     }

     // Scaling the output
     for (i=0; i<spec_vector_size; i++) {
        FLOAT32 tmp = eg*(temp_spec_vector[i]/denom[i]);

        if (tmp > 0)
           out_spec_vector[i] = tmp*tmp;
        else
           out_spec_vector[i] = 0.0F;
     }
     return(out_spec_vector);
  }
  else { // input vector is all zeros - short-cut to save un-necessary compuations
    //
    // for (i=0; i<spec_vector_size; i++) {
    //    out_spec_vector[i] = 0.0F;
    // }
    return(zeros_vector);
  }
}
//
// Additive Noise Suppression
//
FLOAT32 *Enhance::SuppressNoise(FLOAT32 *in_vector, FLOAT32 *background_estm)
{
  UINT16    i;
  FLOAT32 xbr;

  for (i=0; i<spec_vector_size; i++) {
     if (background_estm[i]>0) {
        xbr = in_vector[i]/background_estm[i];
        // Spectral Subtraction
        //if (in_vector[i] > background_estm[i])
        //   in_vector[i] = in_vector[i] - background_estm[i];
        // Spectral Supression
        temp_spec_vector[i] = in_vector[i]*(1.0 - exp(-alpha*xbr*xbr));
     }
     else {
        temp_spec_vector[i] = in_vector[i];
     }
  }

  return(temp_spec_vector);
}

/*******************************  End of File  *******************************/


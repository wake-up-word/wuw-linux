/******************************************************************************

Copyright (C) 2006-2015 VoiceKey Inc.
Cocoa Beach, Florida 32931
All rights reserved

Confidential and Proprietary


Module: mel_filter.cpp
Author: Veton Kepuska
Created: June 29, 2006
Updated: 

Description: A modified version of mel_filter class definition based on 2004 
Veton Kepuska's version. 

Inspired by David Shipman's  model of modular software 
architecture implemented in C.

$Log:$

*******************************************************************************

Proprietary and Confidential. All rights reserved.
VoiceKey Inc., Cocoa Beach, Florida 32931.

*******************************************************************************

Notes:

******************************************************************************/


/******************************  Include Files  ******************************/

#include <cmath>
#include <new>
#include <memory.h>
#include <string.h>
#include <iostream>
using namespace std;
#include "mel_filter.h"

/*********************************  Defines  *********************************/

/****************************  Function Members  *****************************/
//
// Default Constructor 
//
MelFilter::MelFilter ()
{
   // info included in defines.h; 
   sampling_frequency = FE_SAMPLE_RATE;          //8000.0F;
   fft_vec_length     = FE_FFT_SIZE;             //256

   starting_frequency = MEL_STARTING_FREQUENCY;  //64.0F;
   num_mel_filters    = NUM_MEL_FILTERS;         // ??? How was this number determined 

   AllocMemory();
}
//
// Configurable Constructor
//
MelFilter::MelFilter(MF_Config *mfConfig)
{
   sampling_frequency = mfConfig->sample_rate;  //8000.0F;
   fft_vec_length     = mfConfig->fft_size;     // 256

   starting_frequency = mfConfig->mel_starting_frequency;  //64.0F;
   num_mel_filters    = mfConfig->num_mel_filters;         // ??? How was this number determined 

   AllocMemory();  
}
//
// Init Function
//
void MelFilter::Init(MF_Config *mfConfig)
{
   // Freeing allocated memory space
   FreeMemory();

   sampling_frequency = mfConfig->sample_rate;  // 8000.0F;
   fft_vec_length     = mfConfig->fft_size;     // 256

   starting_frequency = mfConfig->mel_starting_frequency;  // 64.0F;
   num_mel_filters    = mfConfig->num_mel_filters;         // ??? How was this number determined 

   AllocMemory();
}
//
// Reset Function
//
void MelFilter::Reset()
{
   // Freeing allocated memory space
   FreeMemory();

   // info included in defines.h; 
   sampling_frequency = FE_SAMPLE_RATE;          // 8000.0F;
   fft_vec_length     = FE_FFT_SIZE;             // 256

   starting_frequency = MEL_STARTING_FREQUENCY;  // 64.0F;
   num_mel_filters    = NUM_MEL_FILTERS;         // ??? How was this number determined 

   AllocMemory();
}
//
// Allocation of Objects Memory Space
//
void MelFilter::AllocMemory() 
{
   UINT16  i, FFTLength, NumChannels;
   FLOAT32 StFreq, SmplFreq;

   StFreq      = starting_frequency;
   SmplFreq    = sampling_frequency;
   NumChannels = num_mel_filters;
   FFTLength   = fft_vec_length;

   out_mel_spectrum = new FLOAT32 [num_mel_filters];
   for (i=0; i<num_mel_filters; i++) {
      out_mel_spectrum[i] = 0.0F;
   }

   memset(&FirstWin, 0, sizeof(FirstWin));

   ComputeMelScaleWindows (&FirstWin, StFreq, SmplFreq, FFTLength, NumChannels); 
   ComputeTriangle (&FirstWin);

}
//
// Freeing Memory Space
//
void MelFilter::FreeMemory()
{
   FFT_Mel_Scale_Window *p;

   while ( FirstWin.Next!=NULL )
   {
      p = (FirstWin.Next)->Next;
      free(FirstWin.Next->Data);
      free(FirstWin.Next);
      FirstWin.Next = p;
   }
   free(FirstWin.Data);

   delete [] out_mel_spectrum;
}
//
// Default Destructor
//
MelFilter::~MelFilter ()
{
   FreeMemory();
}
//
// Mel Filtering Computation Function
//
FLOAT32 *MelFilter::Run (FLOAT32 *magspec_buffer)
{
   UINT16 num_mf;

   num_mf = MelFilterBank (magspec_buffer, out_mel_spectrum, &FirstWin);

   if (num_mf != num_mel_filters) {
      cerr << "Error: Number of filter returned from MelFilterBank function"
         << num_mf
         << "is not eqaul to requested number of  mel filters"
         << num_mel_filters
         << endl;
      exit (1);	
   }
   return (out_mel_spectrum);
}
/*---------------------------------------------------------------------------
* FUNCTION NAME: ConputeMelScaleWindows
*
* PURPOSE:       Initializes data structure for FFT windows (mel filter bank).
*                Computes starting point and length of each window, allocates
*                memory for window coefficients.
*
* INPUT:
*   FirstWin     Pointer to first FFT window structure
*   StFreq       Starting frequency of mel filter bank
*   SmplFreq     Sampling frequency
*   FFTLength    FFT length
*   NumChannels  Number of channels
*
* OUTPUT
*                Chained list of FFT window data structures. NOTE FFT window
*                coefficients are not computed yet.
*
* RETURN VALUE
*   none
*---------------------------------------------------------------------------*/
void MelFilter::ComputeMelScaleWindows(FFT_Mel_Scale_Window *FirstWin,
                                       FLOAT32 StFreq, FLOAT32 SmplFreq, 
                                       UINT16 FFTLength, UINT16 NumChannels)
{
   FFT_Mel_Scale_Window *p1, *p2;
   FLOAT32 freq, start_mel, fs_per_2_mel;
   UINT16   i, TmpInt;

   /* Constants for calculation */
   start_mel    = (FLOAT32)( 2595.0F * log10 (1.0F + (FLOAT32) StFreq / 700.0F));
   fs_per_2_mel =  (FLOAT32)(2595.0F * log10 (1.0F + (SmplFreq / 2.0F) / 700.0F));

   p1 = FirstWin;

   for (i = 0; i < NumChannels; i++)
   {
      /* Calculating mel-scaled frequency and the corresponding FFT-bin */
      /* number for the lower edge of the band                          */
      freq = (FLOAT32)(700 * (pow (10.0F, (start_mel + (FLOAT32) i / (NumChannels + 1) *
         (fs_per_2_mel - start_mel)) / 2595.0F) - 1.0F));
      TmpInt = (UINT16) (FFTLength * freq / SmplFreq + 0.5F);

      /* Storing */
      p1->StartingPoint = TmpInt;

      /* Calculating mel-scaled frequency for the upper edge of the band */
      freq = (FLOAT32)(700 * (pow (10.0F, (start_mel + (FLOAT32) (i + 2) / (NumChannels + 1)
         * (fs_per_2_mel - start_mel)) / 2595.0F) - 1.0F));

      /* Calculating and storing the length of the band in terms of FFT-bins*/
      p1->Length = (UINT16) (FFTLength * freq / SmplFreq + 0.5F) - TmpInt + 1;

      /* Allocating memory for the data field */
      p1->Data = (FLOAT32 *) malloc (sizeof (FLOAT32) * p1->Length);

      /* Continuing with the next data structure or close the last structure
      with NULL */
      if (i < NumChannels - 1)
      {
         p2 = (FFT_Mel_Scale_Window *) malloc (sizeof (FFT_Mel_Scale_Window));
         p1->Next = p2;
         p1 = p2;
      }
      else
         p1->Next = NULL;
   }
}
/*---------------------------------------------------------------------------
* FUNCTION NAME: ComputeTriangle
*
* PURPOSE:       Computes and stores FFT window coefficients (triangle points)
*                into initialized chained list of FFT window structures
*
* INPUT:
*   FirstWin     Pointer to first FFT window structure
*
* OUTPUT
*                Chained list of FFT window data structures with correct
*                window coefficients
*
* RETURN VALUE
*   none
*---------------------------------------------------------------------------*/
void MelFilter::ComputeTriangle (FFT_Mel_Scale_Window *FirstWin)
{	
   FFT_Mel_Scale_Window *p1;

   UINT16 low_part_length, hgh_part_length, TmpInt=0, i, j;
   //UINT16 count = 1;

   p1 = FirstWin;
   j = 0;
   while (p1)
   {

      low_part_length = p1->Next ?
         p1->Next->StartingPoint - p1->StartingPoint + 1 : TmpInt - p1->StartingPoint + 1;
      hgh_part_length = p1->Length - low_part_length + 1;

      /* Lower frequency part of the triangle */
      for (i = 0; i < low_part_length; i++){
         p1->Data[i] = (FLOAT32) (i + 1) /low_part_length;
      }

      /* Higher frequency part of the triangle */
      for (i = 1; i < hgh_part_length; i++){
         p1->Data[low_part_length + i - 1] = (FLOAT32) (hgh_part_length - i)/hgh_part_length;
      }
      for (i = 0; i < p1->Length; i++){
         p1->Data[ i ] =p1->Data[ i ];
      }

      /* Store upper edge (for calculating the last triangle) */
      TmpInt = p1->StartingPoint + p1->Length - 1;

      /* Next triangle ... */
      p1 = p1->Next;
      //count=count+1;

   }

   return;
}

UINT16 MelFilter::PrintTriangles (WuwLogger *wuwLogger)
{
   FFT_Mel_Scale_Window *p1;
   FLOAT32 *melFilters = new FLOAT32 [fft_vec_length/2+1];
   UINT16 i, j, l;

   p1 = &FirstWin;
   j = 0;
   while (p1)
   {
      for (l = 0; l < p1->StartingPoint; l++)
         melFilters[l] = 0.0F;

      for (i = 0; i < p1->Length; i++, l++)
         melFilters[l]= p1->Data[i];

      for (; l <= fft_vec_length/2; l++)
         melFilters[l] = 0.0F;

      wuwLogger->WriteBinary(melFilters);

      j++;
      p1 = p1->Next;
   }
   return j;
}
/*---------------------------------------------------------------------------
* FUNCTION NAME: MelFilterBank
*
* PURPOSE:       Performs mel filtering on FFT magnitude spectrum using the
*                filter bank defined by a chained list of FFT Mel Filter window
*                structures
*
* INPUT:
*   SigFFT       Pointer to signal FFT magnitude spectrum
*   FirstWin     Pointer to the first channel of the filter bank (first
*                element in the chained list of FFT Mel Filter window data structures)
*
* OUTPUT
*                Filter bank outputs stored at the beginning of input signal
*                FFT buffer pointed by *SigMelFFT*
*
* RETURN VALUE
*   none
*---------------------------------------------------------------------------*/
UINT16 MelFilter::MelFilterBank (FLOAT32 *SigFFT, FLOAT32 *SigMelFFT, FFT_Mel_Scale_Window *FirstWin)
{
   FFT_Mel_Scale_Window *p1;
   FLOAT32 Sum;
   UINT16 i, j;

   p1 = FirstWin;
   j = 0;
   while (p1)
   {
      Sum = 0.0;
      for (i = 0; i < p1->Length; i++)
         Sum += SigFFT[p1->StartingPoint + i] * p1->Data[i];
      SigMelFFT[j] = Sum;
      j++;
      p1 = p1->Next;
   }
   return j;
}

/*******************************  End of File  *******************************/

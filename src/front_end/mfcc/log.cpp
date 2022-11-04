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

/******************************  Include Files  ******************************/

#include <new>
#include <cmath>
#include <memory.h>

#include "log.h"

/*********************************  Defines  *********************************/

/****************************  Function Members  *****************************/
//
// Default Constructor 
//
VLOG::VLOG()
{
  UINT16 i;
  
  in_vec_size  = NUM_MEL_FILTERS;
  out_vec_size = in_vec_size;  
  
  out_vec      = new FLOAT32 [out_vec_size];
  for (i=0; i<out_vec_size; i++) {
    out_vec[i] = 0.0F;
  }
  
  Energy_Floor_log = FE_FRAME_ENERGY_FLOOR_LOG;     //-50.0F; dB Scale
  Energy_Floor     = (FLOAT32) exp ((FLOAT64) Energy_Floor_log);
}
//
// Configurable Constructor 
//
//VLOG::VLOG (UINT16 inVecSize, FLOAT32 EnergyFloor_log)
VLOG::VLOG (VLOG_Config *vlogConfig)
{
  UINT16 i;
  
  in_vec_size  = vlogConfig->in_vec_size;
  out_vec_size = in_vec_size;
  
  out_vec      = new FLOAT32 [out_vec_size];
  for (i=0; i<out_vec_size; i++) {
    out_vec[i] = 0.0F;
  }
  
  Energy_Floor_log  = vlogConfig->Energy_Floor_log; // dB Scale
  Energy_Floor      = (FLOAT32) exp ((FLOAT64) Energy_Floor_log);
}
//
// Reset Function
//
void VLOG::Reset()
{
  UINT16 i;

  delete [] out_vec;

  in_vec_size  = NUM_MEL_FILTERS;
  out_vec_size = in_vec_size;
  
  out_vec      = new FLOAT32 [out_vec_size];
  for (i=0; i<out_vec_size; i++) {
    out_vec[i] = 0.0F;
  }
  
  Energy_Floor_log = FE_FRAME_ENERGY_FLOOR_LOG;     //-50.0F; dB Scale
  Energy_Floor     = (FLOAT32) exp ((FLOAT64) Energy_Floor_log);
}
//
//
//
void VLOG::Init (VLOG_Config *vlogConfig)
{
  delete [] out_vec;

  in_vec_size  = vlogConfig->in_vec_size;
  out_vec_size = in_vec_size;
  
  out_vec      = new FLOAT32 [out_vec_size];

  Energy_Floor_log  = vlogConfig->Energy_Floor_log; // dB Scale
  Energy_Floor      = (FLOAT32) exp ((FLOAT64) Energy_Floor_log);
}
//
// Default Destructor 
//
VLOG::~VLOG()
{
  delete [] out_vec;
}
//
// Computation of Natural Log 
//
FLOAT32 *VLOG::Run (FLOAT32 *in_vec)
{
  UINT16 i;
  
  for (i=0; i<in_vec_size; i++) {
    if (in_vec[i] < Energy_Floor)        
      out_vec[i] = Energy_Floor_log;
    else
      out_vec[i] = (FLOAT32) log ((FLOAT64) in_vec[i]);
  }
  
  return (&out_vec[0]);
}

/*******************************  End of File  *******************************/

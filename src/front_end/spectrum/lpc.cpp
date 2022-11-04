/******************************************************************************

                      Copyright (C) 2006-2015 VoiceKey Inc.
                           Cocoa Beach, Florida 32931
                               All rights reserved

                           Confidential and Proprietary


     Module: lpc.cpp
     Author: Veton Kepuska
    Created: June 06, 2006
    Updated: 

Description: A modified version of lpc class. Inspired by David Shipman's  
             model of modular software architecture implemented in C.

$Log:$

*******************************************************************************

Notes:

******************************************************************************/

/**********************************  Notes  **********************************

                         +---------------------------+ 
         Input           |           gain            | gain
          -------------->| ------------------------- |----------> 
         Time Samples    |       -1    -2        -p  | ai; i=1,...,P
                         |  1+a1z  +a2z  +...+aPz    |
                         +---------------------------+ 

/******************************  Include Files  ******************************/

#include <cmath>
#include <new>
#include <iostream>
using namespace std;

#include "lpc.h"

/*********************************  Defines  *********************************/

/****************************  Function Members  *****************************/
//
// Default Constructor
//
LPC::LPC()
{
	window_size = FE_WINDOW_SIZE;
	lpc_order   = FE_LPC_ORDER;   // a1..aP; P=FE_LPC_ORDER
	AllocBuff();                  // a0 - energy of the windowed signal
}
//
// Configurable Constructor
//
LPC::LPC(CfgLPC* lpcConfig)
{
	window_size = lpcConfig->buffer_size;
	lpc_order   = lpcConfig->lpc_order;   // a1..aP; P=lpc_order
	AllocBuff();                          // a0 - energy of the windowed signal
}
//
// Allocation of Internal Buffers
//
void LPC::AllocBuff() 
{
	r   = new FLOAT64 [lpc_order+1];
	a   = new FLOAT64 [lpc_order+1];
	// Temp array
	tmp = new FLOAT64 [lpc_order+1];
}
//
// Destructor
//
LPC::~LPC()
{
	delete [] r;
	delete [] a;
   delete [] tmp;
}
//
// Main Function that Performs Computation
//
FLOAT64 *LPC::Run(FLOAT32 *in_buffer)
{
	FLOAT64 r0;

	r0 = autocorr(in_buffer);
   signal_energy = r0;
	levinson_durbin();

	return(a);
}
//
// Returning Signal Energy
//
FLOAT32 LPC::getEnergy() {
   if (signal_energy > FLT_MAX) {
      cerr << "WARNING: Signal Energy Exceeds FLOAT32 precission: " 
         << signal_energy << endl;
      return (FLT_MAX);
   }
   else {
      return ((FLOAT32)signal_energy);
   }
}
//
// Autocorrelation function
//
FLOAT64 LPC::autocorr(FLOAT32 *signal)
{
	SINT32    i, j;
	FLOAT64   sum;

	for (i=0; i<=lpc_order; i++) {
		sum = 0.0;
		for (j=0; j<window_size-i; j++) {
			sum += signal[j]*signal[j+i];
		}
		r[i]=sum;
	}
	return (r[0]);
}
//
// Levinson-Durbin Recursive Solution of autocorrelation function
//
FLOAT64 *LPC::levinson_durbin()
{
	SINT32        i, j;
	FLOAT64       k, alfa, s, energy;

	// Initializing LPC filter coefficients
	for (i=1; i<=lpc_order; i++) {
		a[i] = 0.0;
	}
	a[0] = energy = alfa = r[0];
	// Energy can not be less then zero
	if (alfa <= DEF_TINY2) {
		return a;
	}
	//
	for (i=1; i<=lpc_order; i++) {
		// Computation of the sum     
		for (j=1, s=0; j<i; j++) {
			s += a[j]*r[i-j];
		}

		k = -(r[i]+s)/alfa;
		if (fabs(k) >= 1.0) {
			cerr << "levinson_durbin : panic! "
				<< "Absolute value of reflection coefficient greater then one." << endl
				<< "\tabs(k) >= 1, order " << i << ". Aborting..." << endl;
			for (j=i; j<=lpc_order; j++) {
				r[j]=0;
			}
			break;
		}
		//TO DO: Convert to only one loop to compute LP coeffs.
		for (j=1; j<i; j++) {
			tmp[j] = a[j] + k*a[i-j];
		}
		for (j=1; j<i; j++) {
			a[j] = tmp[j];
		}

		a[i] = k;
		alfa *= (1-k*k);
	}
	// Energy is stored in a[0], 
	// a[1]...a[lpc_order] are filter coeffs.
	a[0] = energy;

	return (a);
}
//
// Original Version
//
FLOAT64 LPC::autocorrelation(FLOAT32 *signal)
{
	SINT32    i, j;
	FLOAT64   normal;
	FLOAT64  *corcoeff = &r[0];
	//
	// Performing autocorrelation
	//
	for(i=0; i<=lpc_order; i++) {
		corcoeff[i] = 0.0;
		for(j=0; j<window_size-i; j++) {
			corcoeff[i] += signal[j] * signal[j + i];
		}
	}
	/* normalize the autocorrelation coeffs */
	normal = corcoeff[0];
	if (normal <= DEF_TINY2)
		normal = 1.0;

	for (i=1; i<=lpc_order; i++) {
		corcoeff[i] /= normal;
	}

	return normal;
}
//
// durbin(p, r, a);
//   durbin recursion to compute the ar coefficients
//   solution for autoregressive model
//
FLOAT64 *LPC::durbin()
{
	SINT32    i, ip, ib;
	FLOAT64   E, vE;
	FLOAT64   aib, aip, vaib, vaip;
	FLOAT64   alp, rc, valp, vrc;
	FLOAT64   energy = r[0];

	a[0] = 1.0;
	for(i=1; i<=lpc_order; i++) {
		a[i] = 0.0;
	}

	alp = r[0];
	//alp = 1.0; // Using Normalized coeffs
	for(i=1; i<=lpc_order; i++)  {

		if(alp<DEF_TINY2) {
			a[0] = 1.0;

			for(ip=1; ip<=lpc_order; ip++) {
				a[ip] = 0.0;
			}      
			//return;
			return(a);
		}

		for(ip=1, E=0, vE=0; ip<i; ip++) {
			E += r[i-ip] * a[ip];
		}

		rc = -E / alp;

		for(ip=1; ip<=i/2; ip++) {
			ib    = i - ip;
			aip   = a[ip];
			aib   = a[ib];
			a[ip] = aip + rc * aib;
			a[ib] = aib + rc * aip;
		}

		a[i] = rc;
		alp += rc * E;    
	}

	//a[0] /= alp; /* this is reciprocal of gain */
	a[0] = energy; // Energy is stored in a0

	return(a);
}
/* 
Computation of mvdr coefficients:
IEEE trans. on Speech and Audio Processing,
May 2000, Volume 08, Number 03  
Seech Analysis: "All-Pole Modeling of Speech Based on the Minimum
Variance Distortionless Response Spectrum",
M.N. Murhi and B. D. Rao, [p. 221]
*/
void LPC::computeMVDR()
{
	UINT16   i, k;
	FLOAT64  sum;
	FLOAT64 *lp_coeffs = a;

	sum = 0.0;
	for (i=0; i<lpc_order; i++) {
		sum += (lpc_order - 2*i)*lp_coeffs[i]*lp_coeffs[i];
	}
	mv_coeffs[lpc_order-1] = sum;

	for (k=1; k<lpc_order; k++) {
		sum = 0.0;
		for (i=0; i<lpc_order-k; i++) {
			sum += (lpc_order-k-2*i)*lp_coeffs[i]*lp_coeffs[i+k];
		}
		mv_coeffs[lpc_order-1+k] = sum;
		mv_coeffs[lpc_order-1-k] = sum;
	}
}

/*******************************  End of File  *******************************/


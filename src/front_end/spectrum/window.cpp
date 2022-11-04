/******************************************************************************

Copyright (C) 2006-2015 VoiceKey Inc.
Cocoa Beach, Florida 32931
All rights reserved

Confidential and Proprietary


Module: window.cpp
Author: Veton Kepuska
Created: May 27, 2006
Updated: 

Description: A modified version of window class definition based on 2004 
Veton Kepuska's version. Inspired by David Shipman's  model
of modular software architecture originaly implemented in C.

$Log:$

*******************************************************************************

Notes: 

******************************************************************************/


/******************************  Include Files  ******************************/

#include <cmath>
#include <new>
#include <iostream>
using namespace std;

#include "window.h"


Window::Window ()
{
	m_window_type      = FE_WINDOW_TYPE;
	i16_window_size    = FE_WINDOW_SIZE;
	pf32_window_coeffs = new FLOAT32[i16_window_size];
	pf32_out_samples   = new FLOAT32[i16_window_size];

	InitWindowCoefficients();
}

Window::Window(WindowConfig *pConfig)
{
	m_window_type      = pConfig->windowType;
	i16_window_size    = pConfig->size;
	pf32_window_coeffs = new FLOAT32[i16_window_size];
	pf32_out_samples   = new FLOAT32[i16_window_size];

	InitWindowCoefficients();
}


void Window::InitWindowCoefficients()
{
	switch(m_window_type) 
	{
		case HAMMING:
		{
			FLOAT64 base_angle = PIx2 / (FLOAT64)(i16_window_size - 1);
			for(int i = 0; i < i16_window_size; i++) 
			{
				pf32_window_coeffs[i] = (FLOAT32)(0.54 - 0.46 * cos(i * base_angle));  
			}
			break;
		}
		case RECTANGULAR:
		{	
			for(int i = 0; i < i16_window_size; i++) 
			{
				pf32_window_coeffs[i] = 1.0F;
			}
			break;
		}
		default:
		{
			cerr << "Warning: Window Type Not Implemented " << m_window_type << endl
			     << "         Using HAMMING Window instead" << endl;
			
			m_window_type = HAMMING;
			InitWindowCoefficients();
			break;
		}
	}
}


Window::~Window() 
{
	delete[] pf32_window_coeffs;
	delete[] pf32_out_samples;
}

FLOAT32* Window::Run(FLOAT32* pf32_in_samples)
{
	for (int i = 0; i < i16_window_size; i++) 
	{
		pf32_out_samples[i] = pf32_window_coeffs[i] * pf32_in_samples[i];
	}

	return pf32_out_samples;
}

FLOAT32* Window::Eos(FLOAT32* pf32_in_samples)
{
	if(pf32_in_samples)
		return Run(pf32_in_samples);
	else
		return NULL;
}

/*******************************  End of File  *******************************/

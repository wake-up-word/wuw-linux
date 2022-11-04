/*****************************************************************************

                     Copyright (C) 2006-2015 VoiceKey Inc.
                          Cocoa Beach, Florida 32931
                              All rights reserved

                         Confidential and Proprietary

     Module: ml_gmm.h
     Author: Veton Kepuska
    Created: March 24, 2007
    Updated: 

Description: Gaussian Mixture Maximum Likelihood estimation functions.

$Log:$

*******************************************************************************

Notes: 

******************************************************************************/

#ifndef ML_GMM1D_H
#define ML_GMM1D_H

/******************************  Include Files  ******************************/

#include <cmath>
#include <algorithm>
#include <numeric>
#include <vector>
using namespace std;

#include "defines.h" // For win32types and PIx2

/*********************************  Defines  *********************************/

typedef struct {
   FLOAT32  mean;
   FLOAT32  var;  //std^2
   FLOAT32  w;    //mixture weight
   FLOAT32  stdv;
} GMM1D;

typedef vector<vector<FLOAT32> >     MATRIX2D;
typedef vector<GMM1D>                MV_GMM1D;
typedef vector<UINT16>               VECTORu16;

/****************************  Class Definition  *****************************/

class ML_GMM1D
{
public:
   // Default Constructor
   ML_GMM1D(UINT16 num_gmix, UINT16 vect_size);
   ~ML_GMM1D();

   void Init();
   void Reset();

   MV_GMM1D Run(vector<FLOAT32> *vect);
   UINT16   Classify(FLOAT32 x);

   void  FreeMemory(); 


private:

   MATRIX2D    *mp_GMProb;
   MATRIX2D    *mp_ClassProb;
   VECTORu16   *vp_DataPointClass;
   VECTORu16   *vp_NumDataPoints;

   vector<FLOAT32> *data_vect;
   MV_GMM1D        *mvp_gmm1d;
   UINT16           num_mixtures;

   // working paramters
   FLOAT32          mean;  // gaussian mean
   FLOAT32          var;   // gaussian variance
   FLOAT32          stdv;  // gaussian standard deviation
   FLOAT32          w;     // gaussian mixture weight
   // Total log likelihood of the model to determine convergence
   FLOAT32          log_likelihood, prev_log_likelihood, epsilon;

   FLOAT64          sqrt_pix2;
   FLOAT32          root[2];

   // Utility Functions
   void     InitMeansVars();
   void     Estimate();
   FLOAT32  gpdf(FLOAT32 x, GMM1D g1d);
   void     Maximize();
   FLOAT32  Evaluate();
   void     MinError();
   bool     QuadraticRoots(FLOAT32 a, FLOAT32 b, FLOAT32 c);

};

#endif //ML_GMM1D_H

/*******************************  END OF FILE  *******************************/

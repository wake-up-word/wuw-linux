/*****************************************************************************

                     Copyright (C) 2006-2015 VoiceKey Inc.
                          Cocoa Beach, Florida 32931
                              All rights reserved

                         Confidential and Proprietary

     Module: ml_gmm.cpp
     Author: Veton Kepuska
    Created: March 24, 2007
    Updated: 

Description: Gaussian Mixture Maximum Likelihood estimation functions.

$Log:$

*******************************************************************************

             Proprietary and Confidential. All rights reserved.
                VoiceKey Inc., Cocoa Beach, Florida 32931.

******************************************************************************/

/**********************************  Notes  **********************************


/******************************  Include Files  ******************************/

#include <limits>
#include "ml_gmm.h"

/*********************************  Defines  *********************************/

/****************************  Function Members  *****************************/

ML_GMM1D::ML_GMM1D(UINT16 num_gmix, UINT16 vect_size)
{
   num_mixtures      = num_gmix;
   sqrt_pix2         = sqrt(PIx2);
   mp_GMProb         = new MATRIX2D(vect_size, vector<FLOAT32> (num_mixtures, 0.0));
   mp_ClassProb      = new MATRIX2D(vect_size, vector<FLOAT32> (num_mixtures, 0.0));
   vp_DataPointClass = new VECTORu16(vect_size, 0);
   vp_NumDataPoints  = new VECTORu16(num_mixtures, 0);

   log_likelihood    = prev_log_likelihood = numeric_limits<FLOAT32>::min();
   epsilon           = 1.0e-5;

   //vector<vector<FLOAT64> > GMProb(vect.size(), vector<FLOATA64>(num_mixtures, 0.0);
   //vector<vector<FLOAT64> > ClassProb(vect.size(), vector<FLOATA64>(num_mixtures, 0.0);
   //vector<UINT16> DataPointClass(vect.size(), 0);
   //vector<UINT16> NumDataPoints(num_mixtures, 0);

}
ML_GMM1D::~ML_GMM1D()
{
}

void ML_GMM1D::FreeMemory() 
{
   //mp_GMProb  
}

MV_GMM1D ML_GMM1D::Run(vector<FLOAT32> *vect)
{
   UINT16 i;
   data_vect         = vect;

   //MV_GMM1D          mv_gmm1d(num_mixtures);
   //mvp_gmm1d = &mv_gmm1d;
  
   mvp_gmm1d = new MV_GMM1D(num_mixtures);

   // Initialization
   i = 0;
   InitMeansVars();
   prev_log_likelihood = Evaluate();
   fwprintf(stderr, L"\tIteration\tLog_Likelihood\n");      
   fwprintf(stderr, L"\t---------\t--------------\n");
   fwprintf(stderr, L"\t   [%2d] \t%f\n", i, prev_log_likelihood);      
   do {
      if (i>0) {
         prev_log_likelihood = log_likelihood;
      }
      // Estimation Step
      Estimate();
      // Maximization Step
      Maximize();
      // Evaluation Step
      log_likelihood = Evaluate();
      fwprintf(stderr, L"\t   [%2d] \t%f\n", ++i, log_likelihood);      
   } while (((log_likelihood-prev_log_likelihood)/log_likelihood > epsilon) && (i<50));
   fwprintf(stderr, L"\t=========\t===============\n");
   fwprintf(stderr, L"GMM Parameters:\n");
   fwprintf(stderr, L"\tMean       \tStd        \tWeight     \n");
   fwprintf(stderr, L"\t-----------\t-----------\t-----------\n");
   for (i=0; i<num_mixtures; i++) {
      fwprintf(stderr,L" <%1d>\t%f\t%f\t%f\n", i, (*mvp_gmm1d)[i].mean, (*mvp_gmm1d)[i].stdv, (*mvp_gmm1d)[i].w);
   }
   fwprintf(stderr, L"\t===========\t===========\t===========\n");

   FreeMemory();
   
   return(*mvp_gmm1d);
}
//
void ML_GMM1D::InitMeansVars() 
{
   UINT16 i;
   //vector<FLOAT32>::iterator  vf32_it;
   FLOAT32   min_elem, max_elem, delta, data_mean, data_var, data_stdv, tmp;
   FLOAT64   sum2;
   // Mean & Variance Computation of the overall data
   data_mean = accumulate(data_vect->begin(), data_vect->end(), 0.0F)/data_vect->size();

   //for (vf32_it = vect.begin(), sum2 = 0.0; vf32_it != vect.end(); vf32_it++) {
   //   sum2 += (*vf32_it)*(*vf32_it);
   //}
   for (i=0, sum2 = 0.0; i < data_vect->size(); i++) {
      FLOAT32 x = (*data_vect)[i];
      sum2 += x*x;
   }
   data_var  = (sum2/data_vect->size())-data_mean*data_mean;
   data_stdv = sqrt(data_var);
   if (data_stdv*sqrt_pix2 < 1.0F) {
      data_stdv = 2.0F/sqrt_pix2;
      data_var  = data_stdv*data_stdv;
   }

   min_elem = *min_element(data_vect->begin(), data_vect->end());
   max_elem = *max_element(data_vect->begin(), data_vect->end());

   tmp = data_mean-data_stdv;
   if (tmp < min_elem) { 
      tmp = data_mean;
   }
   (*mvp_gmm1d)[0].mean = tmp;
   (*mvp_gmm1d)[0].var  = data_var;
   (*mvp_gmm1d)[0].stdv = data_stdv;
   
   for (i=1; i<num_mixtures; i++) {
      tmp = data_mean+i*data_stdv/num_mixtures;
      if (tmp > max_elem) { 
         tmp = max_elem;
      }
      (*mvp_gmm1d)[i].mean = tmp;
      (*mvp_gmm1d)[i].var  = data_var;
      (*mvp_gmm1d)[i].stdv = data_stdv;
   }
}
//
void ML_GMM1D::Estimate()
{
   UINT16    i, j, count, indx_max_prob, vect_size;
   FLOAT64   sum, tmp;
   FLOAT32   x, max_prob, prob;

   vect_size = data_vect->size();
   // Defining ClassProb matrix for each data point(column- index1) & class (row - index2) 

   // For each data point
   for (i=0; i<vect_size; i++) {
      x        = (*data_vect)[i];
      max_prob = 0;
      for (j=0; j<num_mixtures; j++) {
         // gpdf(data point x| gaussian model: mean, var)
         prob = gpdf(x, (*mvp_gmm1d)[j]);
         (*mp_GMProb)[i][j] = prob;
         if (prob > max_prob) {
            max_prob      = prob;
            indx_max_prob = j;
         }
      }
      // Marking which data point belongs to which GMM
      (*vp_DataPointClass)[i] = indx_max_prob;
      // Counting number of Data points going to GM pointed to by indx_max_prob
      (*vp_NumDataPoints)[indx_max_prob]++;
   }
   for (i=0; i<vect_size; i++) {
      sum      = 0.0;
      for (j=0; j<num_mixtures; j++) {
         // (number of data points belonging to mixture j)/(total number of data points)*gpdf(data point x|mean, var)
         tmp = (((FLOAT64)(*vp_NumDataPoints)[j])/vect_size)*((FLOAT64)((*mp_GMProb)[i][j]));
         //fwprintf(stderr, L"(*vp_NumDataPoints)[%d] = <%d>\n", j, (*vp_NumDataPoints)[j]);
         //fwprintf(stderr, L"(*mp_GMProb)[%d][%d] = <%f>\n", i, j, (*mp_GMProb)[i][j]);
         sum += tmp;
         (*mp_ClassProb)[i][j]  = tmp;
      }
      if (sum > 0) 
      {
         for (j=0; j<num_mixtures; j++) {
            (*mp_ClassProb)[i][j] /= sum;
         }
      }
      else 
      {
         // Serching closest Mixture
         vector<FLOAT32> dist(num_mixtures, 0.0F);
         FLOAT32 dsum;
         int l;
         x        = (*data_vect)[i];
         for (l=0; l<num_mixtures; l++) {
            dist[l] = abs(x-(*mvp_gmm1d)[l].mean);
         }
         // Sorting in default assending order
         sort(dist.begin(),dist.end());
         dsum = accumulate(dist.begin(),dist.end(), 0.0F);
         for (l=0; l<num_mixtures; l++) {
            (*mp_ClassProb)[i][l] = dist[num_mixtures-l-1]/dsum;
         }
      }
   }
   // Clearing Accumulators
   for (j=0; j<num_mixtures; j++) {
      (*vp_NumDataPoints)[j] = 0;
   }
}
//
FLOAT32 ML_GMM1D::gpdf(FLOAT32 x, GMM1D g1d) 
{
   FLOAT32 dist;
   FLOAT32 delta = (x-g1d.mean);
   FLOAT32 val;

   dist = -0.5F*(delta*delta)/g1d.var;
   if (dist < -50.0F)
      val = 0.0F;
   else
      val = 1.0F/(sqrt_pix2*g1d.stdv)*exp(dist);

   return(val);
}
//
void ML_GMM1D::Maximize()
{
   UINT16 i, j, vect_size;
   vector<FLOAT32> sum1(num_mixtures, 0.0F), sum2(num_mixtures, 0.0F);
   FLOAT32 prob, diff, sum;

   vect_size = data_vect->size();
   // Updating Means
   for (j=0; j<num_mixtures; j++) { 
      for (i=0; i<vect_size; i++) {
         prob  = (*mp_ClassProb)[i][j];
         sum2[j] +=  prob;
         sum1[j] +=  (*data_vect)[i]*prob;
     }
     (*mvp_gmm1d)[j].mean = sum1[j]/sum2[j];
   }
   // Updating Variances
   for (j=0; j<num_mixtures; j++) { 
      sum1[j] = 0.0F;
      for (i=0; i<vect_size; i++) {
         prob  = (*mp_ClassProb)[i][j];
         diff  = (*data_vect)[i]-(*mvp_gmm1d)[j].mean;
         sum1[j] += prob*diff*diff;
     }
     if (sum1[j] != 0) {
        // Update only if var does not become eqaul to zero
        (*mvp_gmm1d)[j].var  = sum1[j]/sum2[j];
        (*mvp_gmm1d)[j].stdv = sqrt(sum1[j]/sum2[j]);
     }
     else {
        // Not allowing termination of a mixture that leads to singularity
        (*mvp_gmm1d)[j].var  *= 4.0F;
        (*mvp_gmm1d)[j].stdv *= 2.0F;
        sum2[j]              *= 2.0F;              
     }
   }
   // In case of exception (re-)normalize sum2
   for (j=0, sum =0.0F; j<num_mixtures; j++) { 
      sum += sum2[j]/vect_size;
   }
   if (sum != 1.0F) {
      for (j=0; j<num_mixtures; j++) { 
         sum2[j] /= sum;
      }
   }
   // Updating Priors
   for (j=0; j<num_mixtures; j++) { 
     (*mvp_gmm1d)[j].w = sum2[j]/vect_size;
   }
}
//
FLOAT32 ML_GMM1D::Evaluate()
{
   UINT16 i, j;
   FLOAT32 sum;
   //
   Estimate();
   // Compute Overall Likelihood
   sum = 0.0F;
   for (j=0; j<num_mixtures; j++) { 
      for (i=0; i<data_vect->size(); i++) {
         sum += ((*mp_ClassProb)[i][j])*((*mp_GMProb)[i][j]);
     }
   }
   return(log(sum));
}
// Returns class ID-num
UINT16 ML_GMM1D::Classify(FLOAT32 x) 
{
   UINT16 j, indx_max_prob;
   FLOAT32 max_prob = 0, prob;

   for (j=0; j<num_mixtures; j++) {
      // gpdf(data point x| gaussian model: mean, var)
      prob = gpdf(x, (*mvp_gmm1d)[j]);
      if (prob > max_prob) {
         max_prob      = prob;
         indx_max_prob = j;
      }
   }
   return(indx_max_prob);
}
//
void ML_GMM1D::MinError()
{
   FLOAT32 a;
   FLOAT32 b;
   FLOAT32 c;

   for (int i=0; i<num_mixtures-1; i++) {
      FLOAT32 inv_sig1 = 1.0/(*mvp_gmm1d)[i].stdv, inv_sig2 = 1.0/(*mvp_gmm1d)[i+1].stdv;
      FLOAT32 mean1    = (*mvp_gmm1d)[i].mean, mean2 = (*mvp_gmm1d)[i+1].mean;
      FLOAT32 pw1      = (*mvp_gmm1d)[i].w, pw2 = (*mvp_gmm1d)[i+1].w;

      a = -0.5*(inv_sig2 - inv_sig1);
      b = inv_sig2*mean2-inv_sig1*mean1;
      c = -0.5*((mean2*mean2*inv_sig2-mean1*mean1*inv_sig1)+log(abs(inv_sig2/inv_sig1)))+log(pw2/pw1);
      QuadraticRoots(a, b, c);

      fwprintf(stderr,L"\t<%1d> <%1d> ROOTS:\t%f\t%f\n", i, i+1, root[0], root[1]);
      fwprintf(stderr,L"\tmean2-mean1:\t%f",mean2-mean1); 
   }
}
//
bool ML_GMM1D::QuadraticRoots(FLOAT32 a, FLOAT32 b, FLOAT32 c)
{
   FLOAT32 disc = b*b - 4.0*a*c;

   if (disc < 0) {
      root[0] = 0;
      root[1] = 0;
      return(false);
   }
   root[0] = (-b+sqrt(disc))/(2.0*a);
   root[1] = (-b-sqrt(disc))/(2.0*a);

   return(true);
}

/*******************************  End of File  *******************************/
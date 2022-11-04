/******************************************************************************

     Module: fft.cpp
     Author: Veton Kepuska
    Created: June 08, 2006
    Updated: 

Description: A modified version of fft class. Inspired by David Shipman's  
             model of modular software architecture implemented in C.

       $Log:$

*******************************************************************************

                     Proprietary and Confidential. 
                        All rights reserved.
                           VoiceKey Inc., 
                      Cocoa Beach, Florida 32931.

******************************************************************************/

/**********************************  Notes  **********************************


/******************************  Include Files  ******************************/

#include <cfloat>
#include <climits>
#include <cmath>
#include <memory.h>

using namespace std;

#include "fft.h"


/*********************************  Defines  *********************************/

/****************************  Function Members  *****************************/
//
// Default Constructor
//
FFT::FFT()
{
  in_buffer_size  = FE_LPC_ORDER;
  //in_buffer_size  = FE_WINDOW_SIZE;
  fft_size        = FE_FFT_SIZE; // >=2 & power of 2;
  fft_order       = fastlog2(fft_size);
  out_buffer_size = FE_FFT_SIZE/2;  // + 1 is needed becasue of mfcc implementation
  AllocBuffers();
}
//
// Configurable Constructor
//
FFT::FFT(FFT_Config *fftConfig)
{
  in_buffer_size  = fftConfig->in_buffer_size;
  fft_size        = fftConfig->fft_size; // >=2 & power of 2; 
  fft_order       = fastlog2(fft_size);
  out_buffer_size = fftConfig->fft_size/2;
  AllocBuffers();
}
//
// Allocation of Internal Buffers
//
void FFT::AllocBuffers() 
{
  UINT32 length = (UINT32) ceil(2+(float)sqrt((float)fft_size/2));

  out_spectrum   = new FLOAT32 [out_buffer_size+1];
  memset(out_spectrum, 0, (out_buffer_size+1)*sizeof(FLOAT32));
  in_buffer      = new FLOAT64 [fft_size];   // input out data
  memset(in_buffer, 0, (fft_size)*sizeof(FLOAT64));
  tmp_fft_buffer = new FLOAT64 [out_buffer_size+1];
  memset(tmp_fft_buffer, 0, (out_buffer_size+1)*sizeof(FLOAT64));
  //
  // fft internal data buffers
  //
  ip             = new SINT32 [length];
  memset(ip, 0, (length)*sizeof(SINT32));  
  w              = new FLOAT64 [fft_size/2];
  memset(w, 0, (fft_size/2)*sizeof(FLOAT64));
}
//
// Destructor
//
FFT::~FFT()
{
  delete [] out_spectrum;
  delete [] in_buffer;
  delete [] tmp_fft_buffer;
  delete [] ip;
  delete [] w;
}
//
// Main Function that Performs Computation
//
//
// Main Function that Performs Computation
//
FLOAT32 *FFT::Run_ComputePowerSpectrum(FLOAT32 *in_data)
{
  UINT16  i;

  // Zero-ing elements of the vector before copying input-data
  memset(in_buffer, 0, fft_size*sizeof(FLOAT64));
  // Converting from FLOAT32 to FLOAT64
  for (i=0; i<in_buffer_size; i++)
    in_buffer[i] = in_data[i];
  //
  // Real Input Sequence in-place FFT 
  //
  rfft(in_buffer, fft_size, fft_order);
  //
  // Power Spectrum
  //
  tmp_fft_buffer[0] = in_buffer[0]*in_buffer[0];  /* DC */
  /* pi/(N/2), 2pi/(N/2), ...,(N/2-1)*pi/(N/2) */
  for (i = 1; i < out_buffer_size; i++)  {  
    tmp_fft_buffer[i] = (in_buffer[i] * in_buffer[i]) +
                        (in_buffer[fft_size - i] * in_buffer[fft_size - i]);
  }
  /* Nyquist frequency -  pi/2 */
  tmp_fft_buffer[out_buffer_size] = in_buffer[out_buffer_size]*in_buffer[out_buffer_size];  
  //
  // Conversion from FLOAT64 to FLOAT32
  //
  for (i=0; i<=out_buffer_size; i++) {
     out_spectrum[i] = (FLOAT32)(tmp_fft_buffer[i]);
  }

  return(out_spectrum);
}
//
// Overloaded Function
//
FLOAT32 *FFT::Run_ComputePowerSpectrum(FLOAT64 *in_data)
{
  UINT16  i;
  FLOAT64 gain = in_data[0];
  //
  // in_data[0] = a[0] -- is gain of the signal
  //
  // Zeroing out elements of the vector
  memset(&in_buffer[0], 0, fft_size*sizeof(FLOAT64));
  in_buffer[0] = 1.0; // 1+a[1]+...+a[in_buffer_size]
  // copying a[1]..a[in_buffer_size] to in_buffer 
  memcpy(&in_buffer[1], &in_data[1], sizeof(FLOAT64)*in_buffer_size);
  // Zeroing out remaninig elements of the vector
  //memset(&in_buffer[in_buffer_size+1], 0, (fft_size-in_buffer_size-1)*sizeof(FLOAT64));

  //
  // Real Input Sequence in-place FFT 
  //
  rfft(in_buffer, fft_size, fft_order); // A faster FFT code is desirable
                                        // One obvious speed up is by pre-computing the
                                        // SIN() & COS() values in the table
  // 
  // Power Spectrum
  //
  tmp_fft_buffer[0] = in_buffer[0]*in_buffer[0];  /* DC */
  /* pi/(N/2), 2pi/(N/2), ...,(N/2-1)*pi/(N/2) */
  for (i = 1; i < out_buffer_size; i++)  {  
    tmp_fft_buffer[i] = (in_buffer[i] * in_buffer[i]) +
                        (in_buffer[fft_size - i] * in_buffer[fft_size - i]);
  }
  /* Nyquist frequency -  pi/2 */
  tmp_fft_buffer[out_buffer_size] = in_buffer[out_buffer_size]*in_buffer[out_buffer_size];  
  //
  // Conversion from FLOAT64 to FLOAT32
  //
  for (i=0; i<=out_buffer_size; i++) {
    FLOAT64 tmp = tmp_fft_buffer[i];
    if (tmp > 0) {
      out_spectrum[i] = (FLOAT32)(gain/tmp);
    }
    else {
      out_spectrum[i] = FLT_MAX;
    }
  }

  return(out_spectrum);
}
/*---------------------------------------------------------------------------
 * FUNCTION NAME: rfft
 *
 * PURPOSE:       Real valued, in-place split-radix FFT
 *
 * INPUT:
 *   x            Pointer to input and output array
 *   n            Length of FFT, must be power of 2 /FFTLength
 *   m            int) (log10 (FFTLength) / log10 (2)) = log2(FFTLength)
 *                FFT Order - that is power of two interger specifing n 
 *                            
 *
 * OUTPUT         Output order
 *                  Re(0), Re(1), ..., Re(n/2), Im(N/2-1), ..., Im(1)
 *
 * RETURN VALUE
 *   none
 *
 * DESIGN REFERENCE:
 *                IEEE Transactions on Acoustic, Speech, and Signal Processing,
 *                Vol. ASSP-35. No. 6, June 1987, pp. 849-863.
 *
 *                Subroutine adapted from fortran routine pp. 858-859.
 *                Note corrected printing errors on page 859:
 *                    SS1 = SIN(A3) -> should be SS1 = SIN(A);
 *                    CC3 = COS(3)  -> should be CC3 = COS(A3)
 *---------------------------------------------------------------------------*/
void FFT::rfft (FLOAT64 *x, UINT32 n, UINT32 m)
{
  UINT32 j, i, k, is, id;
  UINT32 i0, i1, i2, i3, i4, i5, i6, i7, i8;
  UINT32 n2, n4, n8;
  FLOAT64 xt, a0, e, a, a3;
  FLOAT64 t1, t2, t3, t4, t5, t6;
  FLOAT64 cc1, ss1, cc3, ss3;
  FLOAT64 *r0;

  /* Digit reverse counter */

  j = 0;
  r0 = x;

  for (i = 0; i < n - 1; i++)
    {

      if (i < j)
        {
	  xt = x[j];
	  x[j] = *r0;
	  *r0 = xt;
        }
      r0++;

      k = n >> 1;

      while (k <= j)
        {
	  j = j - k;
	  k >>= 1;
        }
      j += k;
    }

  /* Length two butterflies */
  is = 0;
  id = 4;

  while (is < n - 1)
    {

      for (i0 = is; i0 < n; i0 += id)
        {
	  i1 = i0 + 1;
	  a0 = x[i0];
	  x[i0] += x[i1];
	  x[i1] = a0 - x[i1];
        }

      is = (id << 1) - 2;
      id <<= 2;
    }

  /* L shaped butterflies */
  n2 = 2;
  for (k = 1; k < m; k++)
    {
      n2 <<= 1;
      n4 = n2 >> 2;
      n8 = n2 >> 3;
      e = (PI * 2) / n2;
      is = 0;
      id = n2 << 1;
      while (is < n)
        {
	  for (i = is; i <= n - 1; i += id)
            {
	      i1 = i;
	      i2 = i1 + n4;
	      i3 = i2 + n4;
	      i4 = i3 + n4;
	      t1 = x[i4] + x[i3];
	      x[i4] -= x[i3];
	      x[i3] = x[i1] - t1;
	      x[i1] += t1;

	      if (n4 != 1)
                {
		  i1 += n8;
		  i2 += n8;
		  i3 += n8;
		  i4 += n8;
		  t1 = (x[i3] + x[i4]) / SQRT2;
		  t2 = (x[i3] - x[i4]) / SQRT2;
		  x[i4] = x[i2] - t1;
		  x[i3] = -x[i2] - t1;
		  x[i2] = x[i1] - t2;
		  x[i1] = x[i1] + t2;
                }
            }
	  is = (id << 1) - n2;
	  id <<= 2;
        }

      for (j = 1; j < n8; j++)
        {
	  a = j * e;
	  a3 = 3 * a;
	  cc1 = cos (a);                 // SIN() & COS() Values 
	  ss1 = sin (a);                 // can be pre-stored in the table
	  cc3 = cos (a3);                // during instantiation of the object
	  ss3 = sin (a3);                // in the constructor

	  is = 0;
	  id = n2 << 1;

	  while (is < n)
            {
	      for (i = is; i <= n - 1; i += id)
                {
		  i1 = i + j;
		  i2 = i1 + n4;
		  i3 = i2 + n4;
		  i4 = i3 + n4;
		  i5 = i + n4 - j;
		  i6 = i5 + n4;
		  i7 = i6 + n4;
		  i8 = i7 + n4;
		  t1 = x[i3] * cc1 + x[i7] * ss1;
		  t2 = x[i7] * cc1 - x[i3] * ss1;
		  t3 = x[i4] * cc3 + x[i8] * ss3;
		  t4 = x[i8] * cc3 - x[i4] * ss3;
		  t5 = t1 + t3;
		  t6 = t2 + t4;
		  t3 = t1 - t3;
		  t4 = t2 - t4;
		  t2 = x[i6] + t6;
		  x[i3] = t6 - x[i6];
		  x[i8] = t2;
		  t2 = x[i2] - t3;
		  x[i7] = -x[i2] - t3;
		  x[i4] = t2;
		  t1 = x[i1] + t5;
		  x[i6] = x[i1] - t5;
		  x[i1] = t1;
		  t1 = x[i5] + t4;
		  x[i5] = x[i5] - t4;
		  x[i2] = t1;
                }
	      is = (id << 1) - n2;
	      id <<= 2;
            }
        }
    }
}
//
// FFT_ORDER from FFT_SIZE = 2^FFT_ORDER 
// 
UINT16 FFT::fastlog2(UINT16 n)
{
  UINT16 num_bits, power = 0;

  if((n < 2) || (n % 2 != 0)) return(0);
  num_bits = sizeof(UINT16) * 8;   /* How big are UINT16s on this machine? */

  while(power <= num_bits) {
    n >>= 1;
    power += 1;
    if(n & 0x01) {
      if(n > 1)	return(0);
      else return(power);
    }
  }
  return(0);
}
#if 0
//
// Support Functions
//
void FFT::CpxFFT(FLOAT64 *s, UINT16 n) 
{
  SINT16  ii,jj,nn,limit,m,j,inc,i;
  FLOAT64 wx,wr,wpr,wpi,wi,theta;
  FLOAT64 xre,xri,x;

  nn=n / 2; j = 1;
  for (ii=1;ii<=nn;ii++) {
    i = 2 * ii - 1;
    if (j>i) {
      xre = s[j-1]; xri = s[j + 1-1];
      s[j-1] = s[i-1];  s[j + 1-1] = s[i + 1-1];
      s[i-1] = xre;
      s[i + 1-1] = xri;
    }
    m = n / 2;
    while (m >= 2  && j > m) {
      j -= m; m /= 2;
      }
    j += m;
  };
  limit = 2;
  while (limit < n) {
    inc = 2 * limit; theta = 2.0*PI / limit;
#ifdef INVERT_FFT
    theta = -theta;
#endif
    x = sin(0.5 * theta);                     // to speed-up 
    wpr = -2.0 * x * x; wpi = sin(theta);     // they can be pre-computed
    wr = 1.0; wi = 0.0;
    for (ii=1; ii<=limit/2; ii++) {
      m = 2 * ii - 1;
      for (jj = 0; jj<=(n - m) / inc;jj++) {
	i = m + jj * inc;
	j = i + limit;
	xre = wr * s[j-1] - wi * s[j + 1-1];
	xri = wr * s[j + 1-1] + wi * s[j-1];
	s[j-1] = s[i-1] - xre; s[j + 1-1] = s[i + 1-1] - xri;
	s[i-1] = s[i-1] + xre; s[i + 1-1] = s[i + 1-1] + xri;
      }
      wx = wr;
      wr = wr * wpr - wi * wpi + wr;
      wi = wi * wpr + wx * wpi + wi;
    }
    limit = inc;
  }
#ifdef INVERT_FFT
  for (i = 1;i<=n;i++)
    s[i-1] = s[i-1] / nn;
#endif
}
//
// Real sequence FFT
//
void FFT::RealFFT(FLOAT64 *s, UINT16 n) 
{
  SINT16  n2, i, i1, i2, i3, i4;
  FLOAT64 xr1, xi1, xr2, xi2, wrs, wis;
  FLOAT64 yr, yi, yr2, yi2, yr0, theta, x;
  
  CpxFFT(s, n);
  n=n/2; n2 = n/2;
  theta = PI / n;
  x = sin(0.5 * theta);                          // pre-computation necessary for speed-up
  yr2 = -2.0 * x * x;
  yi2 = sin(theta); yr = 1.0 + yr2; yi = yi2;    // pre-computation necessary for speed-up
  for (i=2; i<=n2; i++) {
    i1 = i + i - 1;      i2 = i1 + 1;
    i3 = n + n + 3 - i2; i4 = i3 + 1;
    wrs = yr; wis = yi;
    xr1 = (s[i1-1] + s[i3-1])/2.0; xi1 = (s[i2-1] - s[i4-1])/2.0;
    xr2 = (s[i2-1] + s[i4-1])/2.0; xi2 = (s[i3-1] - s[i1-1])/2.0;
    s[i1-1] = xr1 + wrs * xr2 - wis * xi2;
    s[i2-1] = xi1 + wrs * xi2 + wis * xr2;
    s[i3-1] = xr1 - wrs * xr2 + wis * xi2;
    s[i4-1] = -xi1 + wrs * xi2 + wis * xr2;
    yr0 = yr;
    yr = yr * yr2 - yi  * yi2 + yr;
    yi = yi * yr2 + yr0 * yi2 + yi;
  }
  xr1 = s[1-1];
  s[1-1] = xr1 + s[2-1];
  s[2-1] = 0.0;
}
//
// Magnitude Square Computation
//
void FFT::rfe_find_rmagsq(FLOAT64 *magsq, FLOAT64 *cpx_vector, UINT16 npts)
{
   FLOAT64    acc, acc1;
   FLOAT64    *P_real;
   FLOAT64    *P_imag;

   P_real = cpx_vector;
   P_imag = P_real + 2 * npts - 1;

   acc = *P_real++;     /* 0 freq component */
   *magsq++ = acc * acc;

   for ( ; --npts > 0; )
   {
      acc = *P_real++;
      acc1 = *P_imag--;
      acc *= acc;
      acc += acc1 *  acc1;
      *magsq++ = acc;
   }

   acc = *P_real++;     /* 0 freq component */
   *magsq++ = acc * acc;
}
//
// powerspectrum function
//
/*
 * Compute power spectrum of input signal
 *
 * orig (in): original input signal, for power spectrum
 * power(out): power spectrum
 * windowsz (in): size of window (number of samples)
 * npoUINT16s  (in): log _2 (winlength)
 *
 */
void FFT::powerspectrum(FLOAT64 *orig, FLOAT64 *power, UINT16 windowsz, UINT16 npoints)
{
  UINT16 i,j,k;
  FLOAT64 tmp;
  // Copying input data into output buffer
  memcpy((SCHAR *)power,(SCHAR *)orig, windowsz*sizeof(FLOAT64));
  // Setting remaining data to 0
  memset((SCHAR *)(power+windowsz), 0, (npoints-windowsz)*sizeof(FLOAT64));

  FAST(power,npoints);

  /* rearrange data 
   * Only the first half of the power[] array is filled with data.
   * The second half would just be a mirror image of the first half
   * since orig is real.
   */
  tmp=power[1];
  for(i=1;i<npoints>>1;i++){
    j=2*i;
    k=2*i+1;
    power[i]=power[j]*power[j]+power[k]*power[k];
  }
  power[npoints>>1]=tmp*tmp;
  power[0]*=power[0];

}
/*
 * This routine replaces the FLOAT64 vector b
 * of length n with its finite discrete fourier transform.
 * DC term is returned in b[0]; 
 * n/2th harmonic FLOAT64 part in b[1].
 * jth harmonic is returned as complex number stored as
 * b[2*j] + i b[2*j + 1] 
 * (i.e., remaining coefficients are as a DPCOMPLEX vector).
 * 
 */
UINT16 FFT::FAST(FLOAT64 *b, UINT16 n)
{
  FLOAT64 fn;
  UINT16 i, in, nn, n2pow, n4pow, nthpo;

  // ????????????????????????????????????????????????
  // Move those two lines into contructor functions -- n2pow is constant
  // ????????????????????????????????????????????????
  n2pow = fastlog2(n);
  if(n2pow <= 0) return (0);

  nthpo = n;
  fn = (FLOAT64)nthpo;
  n4pow = n2pow / 2;

  /* radix 2 iteration required; do it now */  
  if(n2pow % 2) {
    nn = 2;
    in = n / nn;
    FR2TR(in, b, b + in);
  }
  else nn = 1;
  
  /* perform radix 4 iterations */
  for(i = 1; i <= n4pow; i++) {
    nn *= 4;
    in = n / nn;
    FR4TR(in, nn, b, b + in, b + 2 * in, b + 3 * in);
  }

  /* perform inplace reordering */
  FORD1(n2pow, b);
  FORD2(n2pow, b);

  /* take conjugates */
  for(i = 3; i < n; i += 2) b[i] = -b[i];

  return 1;
}
/* 
 * radix 2 subroutine 
 */
void FFT::FR2TR(UINT16 in, FLOAT64 *b0, FLOAT64 *b1)
{
  UINT16 k;
  FLOAT64 t;
  for(k = 0; k < in; k++) {
    t = b0[k] + b1[k];
    b1[k] = b0[k] - b1[k];
    b0[k] = t;
  }
}
/*
 * radix 4 subroutine 
 */
void FFT::FR4TR(UINT16 in, UINT16 nn, FLOAT64 *b0, FLOAT64 *b1, FLOAT64 *b2, FLOAT64* b3)
{
  FLOAT64 arg, piovn, th2;
  FLOAT64 *b4 = b0, *b5 = b1, *b6 = b2, *b7 = b3;
  FLOAT64 t0, t1, t2, t3, t4, t5, t6, t7;
  FLOAT64 r1, r5, pr, pi;
  FLOAT64 c1, c2, c3, s1, s2, s3;
  
  UINT16 j, k, jj, kk, jthet, jlast, ji, jl, jr, UINT164;
  UINT16 L[16], L1, L2, L3, L4, L5, L6, L7, L8, L9, L10, L11, L12, L13, L14, L15;
  UINT16 j0, j1, j2, j3, j4, j5, j6, j7, j8, j9, j10, j11, j12, j13, j14;
  UINT16 k0, kl;
  
  L[1] = nn / 4;	
  for(k = 2; k < 16; k++) {  /* set up L's */
    switch (LOCAL_SIGNUM(L[k-1] - 2)) {
    case -1:
      L[k-1]=2;
    case 0:
      L[k]=2;
      break;
    case 1:
      L[k]=L[k-1]/2;
    }
  }

  L15=L[1]; L14=L[2]; L13=L[3]; L12=L[4]; L11=L[5]; L10=L[6]; L9=L[7];
  L8=L[8];  L7=L[9];  L6=L[10]; L5=L[11]; L4=L[12]; L3=L[13]; L2=L[14];
  L1=L[15];
  
  piovn = (FLOAT64)PI / (FLOAT64)nn;
  ji=3;
  jl=2;
  jr=2;
  
  for(j1=2;j1<=L1;j1+=2) {
    for(j2=j1;j2<=L2;j2+=L1) {
      for(j3=j2;j3<=L3;j3+=L2) {
	for(j4=j3;j4<=L4;j4+=L3) {
	  for(j5=j4;j5<=L5;j5+=L4) {
	    for(j6=j5;j6<=L6;j6+=L5) {
	      for(j7=j6;j7<=L7;j7+=L6) {
		for(j8=j7;j8<=L8;j8+=L7) {
		  for(j9=j8;j9<=L9;j9+=L8) {
		    for(j10=j9;j10<=L10;j10+=L9) {
		      for(j11=j10;j11<=L11;j11+=L10) {
			for(j12=j11;j12<=L12;j12+=L11) {
			  for(j13=j12;j13<=L13;j13+=L12) {
			    for(j14=j13;j14<=L14;j14+=L13) {
			      for(jthet=j14;jthet<=L15;jthet+=L14) {
				th2 = (FLOAT64)(jthet - 2);
				if(th2<=0.0) {
				  for(k=0;k<in;k++) {
				    t0 = b0[k] + b2[k];
				    t1 = b1[k] + b3[k];
				    b2[k] = b0[k] - b2[k];
				    b3[k] = b1[k] - b3[k];
				    b0[k] = t0 + t1;
				    b1[k] = t0 - t1;
				  }
				  if(nn-4>0) {
				    k0 = in*4 + 1;
				    kl = k0 + in - 1;
				    for (k=k0;k<=kl;k++) {
				      kk = k-1;
				      pr = IRT2 * (b1[kk]-b3[kk]);
				      pi = IRT2 * (b1[kk]+b3[kk]);
				      b3[kk] = b2[kk] + pi;
				      b1[kk] = pi - b2[kk];
				      b2[kk] = b0[kk] - pr;
				      b0[kk] = b0[kk] + pr;
				    }
				  }
				} else {
				  arg = th2*piovn;
				  // 
				  // Must convert cos & sin functions UINT16o tables
				  //
				  c1 = cos(arg); // <- convert to table lookups
				  s1 = sin(arg); // <- convert to table lookups
				  c2 = c1*c1 - s1*s1;
				  s2 = c1*s1 + c1*s1;
				  c3 = c1*c2 - s1*s2;
				  s3 = c2*s1 + s2*c1;

				  UINT164 = in*4;
				  j0=jr*UINT164 + 1;
				  k0=ji*UINT164 + 1;
				  jlast = j0+in-1;
				  for(j=j0;j<=jlast;j++) {
				    k = k0 + j - j0;
				    kk = k-1; jj = j-1;
				    r1 = b1[jj]*c1 - b5[kk]*s1;
				    r5 = b1[jj]*s1 + b5[kk]*c1;
				    t2 = b2[jj]*c2 - b6[kk]*s2;
				    t6 = b2[jj]*s2 + b6[kk]*c2;
				    t3 = b3[jj]*c3 - b7[kk]*s3;
				    t7 = b3[jj]*s3 + b7[kk]*c3;
				    t0 = b0[jj] + t2;
				    t4 = b4[kk] + t6;
				    t2 = b0[jj] - t2;
				    t6 = b4[kk] - t6;
				    t1 = r1 + t3;
				    t5 = r5 + t7;
				    t3 = r1 - t3;
				    t7 = r5 - t7;
				    b0[jj] = t0 + t1;
				    b7[kk] = t4 + t5;
				    b6[kk] = t0 - t1;
				    b1[jj] = t5 - t4;
				    b2[jj] = t2 - t7;
				    b5[kk] = t6 + t3;
				    b4[kk] = t2 + t7;
				    b3[jj] = t3 - t6;
				  }
				  jr += 2;
				  ji -= 2;
				  if(ji-jl <= 0) {
				    ji = 2*jr - 1;
				    jl = jr;
				  }
				}
			      }
			    }
			  }
			}
		      }
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }
}

/*
 * an inplace reordering subroutine 
 */
void FFT::FORD1(UINT16 m, FLOAT64 *b)
{
  UINT16 j, k = 4, kl = 2, n = 0x1 << m;
  FLOAT64 t;
  
  for(j = 4; j <= n; j += 2) {
    if(k - j>0) {
      t = b[j-1];
      b[j - 1] = b[k - 1];
      b[k - 1] = t;
    }
    k -= 2;
    if(k - kl <= 0) {
      k = 2*j;
      kl = j;
    }
  }	
}
/*
 *  the other inplace reordering subroutine 
 */
void FFT::FORD2(UINT16 m, FLOAT64 *b) 
{
  FLOAT64 t;
  
  UINT16 n = 0x1<<m, k, ij, ji, ij1, ji1;
  
  UINT16 l[16], l1, l2, l3, l4, l5, l6, l7, l8, l9, l10, l11, l12, l13, l14, l15;
  UINT16 j1, j2, j3, j4, j5, j6, j7, j8, j9, j10, j11, j12, j13, j14;
  

  l[1] = n;
  for(k=2;k<=m;k++) l[k]=l[k-1]/2;
  for(k=m;k<=14;k++) l[k+1]=2;
  
  l15=l[1];l14=l[2];l13=l[3];l12=l[4];l11=l[5];l10=l[6];l9=l[7];
  l8=l[8];l7=l[9];l6=l[10];l5=l[11];l4=l[12];l3=l[13];l2=l[14];l1=l[15];

  ij = 2;
  
  for(j1=2;j1<=l1;j1+=2) {
    for(j2=j1;j2<=l2;j2+=l1) {
      for(j3=j2;j3<=l3;j3+=l2) {
	for(j4=j3;j4<=l4;j4+=l3) {
	  for(j5=j4;j5<=l5;j5+=l4) {
	    for(j6=j5;j6<=l6;j6+=l5) {
	      for(j7=j6;j7<=l7;j7+=l6) {
		for(j8=j7;j8<=l8;j8+=l7) {
		  for(j9=j8;j9<=l9;j9+=l8) {
		    for(j10=j9;j10<=l10;j10+=l9) {
		      for(j11=j10;j11<=l11;j11+=l10) {
			for(j12=j11;j12<=l12;j12+=l11) {
			  for(j13=j12;j13<=l13;j13+=l12) {
			    for(j14=j13;j14<=l14;j14+=l13) {
			      for(ji=j14;ji<=l15;ji+=l14) {
				ij1 = ij-1; ji1 = ji - 1;
				if(ij-ji<0) {
				  t = b[ij1-1];
				  b[ij1-1]=b[ji1-1];
				  b[ji1-1] = t;
				  
				  t = b[ij1];
				  b[ij1]=b[ji1];
				  b[ji1] = t;
				}
				ij += 2;
			      }
			    }
			  }
			}
		      }
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }
}
#endif

/*******************************  End of File  *******************************/


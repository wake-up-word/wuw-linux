/*****************************************************************************

                     Copyright (C) 2006-2015 VoiceKey Inc.
                          Cocoa Beach, Florida 32931
                              All rights reserved

                         Confidential and Proprietary

     Module: defines.h
     Author: Veton Kepuska
    Created: May 27, 2006
    Updated: September 9, 2006

Description: Contains all parameters and structs that define WUW.

$Log:$

*******************************************************************************

Notes:

******************************************************************************/

#ifndef DEFINE_H
#define DEFINE_H

/******************************  Include Files  ******************************/

#include "win32types.h"
#include <map>
#include <set>
#include <vector>
using namespace std;

#include <string>


#include <stdlib.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

#define PI    (3.1415926535897932384626433832795)
#define M_PI  (3.1415926535897932384626433832795)
#define PIx2  (6.2831853071795864769252867665590)
#define SQRT2 (1.4142135623730950488016887242097)
#define PI8   (PI/8.0)
#define RT2   (SQRT2)
#define IRT2  (1.0/SQRT2)

#define TEXT_LINE_LENGTH 256
#define _MAX_PATH 4096
#define _MAX_FNAME 4096
#define _MAX_DRIVE 4096
#define _MAX_EXT 4096
#define _MAX_DIR 4096

//-----------------------------------------------------------------------------
// Enums
//-----------------------------------------------------------------------------
enum SAMPLE_TYPE
{
    ST_ULAW,
    ST_ALAW,
    ST_SCHAR,
    ST_UCHAR,
    ST_UINT8,
    ST_SINT8,
    ST_UINT16,
    ST_SINT16,
    ST_UINT32,
    ST_SINT32,
    ST_FLOAT32,
    ST_FLOAT64
};

enum WINDOW_TYPE
{
    BARTHANN,
    BARTLETT,
    BLACKMAN,
    BLACKMANHARRIS,
    BOHMAN,
    CHEBYSHEV,
    FLATTOP,
    GAUSSIAN,
    HAMMING,
    HANNING,
    KAISER,
    NUTTALL,
    PARZEN,
    RECTANGULAR,
    TUKEY,
    TRIANGULAR
};

enum SPECTRUM_TYPE
{
    STD_SPEC,        // "Standard" Spectrum Magnitude Computation
    VK_LPCSPEC,         // LPC smoothed Sectral Magnitude Computation -- 2006 Implementation
    VK_FILTER_BANK,  // Not Implemented as of June 2006 - VK Dissertation Work
};

enum FRONT_END_TYPE
{
   FET_STD_MFCC        =  1,// Standard ETSI MFCC's
    FET_ETSI_MFCC       =  2,// ETSI Based MFCC's
    FET_LPC_MFCC        =  4,// LPC Smoothed Specturm Based MFCC's
    FET_VK_LPCENH       =  8,// 2006 Implementation
    FET_VK_STDENH       = 16,// 2006 Implementation
    FET_VK_LPC_ENH_MFCC = 32,// 2006 Implementation LPC Smoothed Enhanced Spectrum MFCC
    FET_VK_FB_SPEC      = 64,// VK Dissertation Work - Not Implemented
};

enum FEATURE_TYPE
{
    FT_STD_MFCC        =  1,// ETSI Standard MFCC's
    FT_ETSI_MFCC       =  2,// ETSI Based MFCC's
    FT_LPC_MFCC        =  4,// LPC Smoothed Specturm Based MFCC's
    FT_VK_LPCENH       =  8,// 2006 Implementation
    FT_VK_STDENH       = 32,// Standard 2006 Implementation
    FT_VK_LPC_ENH_MFCC = 16,// 2006 Implementation LPC Smoothed Enhanced Spectrum MFCC
    FT_VK_FB_SPEC      = 64,// VK Dissertation Work - Not Implemented
};

enum VAD_STATE
{
    VAD_INIT,     // Initial VAD state
    VAD_OFF,      // No Speech
    VAD_ON,       // Speech Detected
    VAD_UNKNOWN,  // Unknown
};

typedef struct VAD_STATES {
  VAD_STATE  VAD_State_Flag;
  VAD_STATE  VAD_EN_State_Flag;
  VAD_STATE  VAD_SPEC_State_Flag;
  VAD_STATE  VAD_MFCC_State_Flag;
  VAD_STATE  VAD_MFCC_Enhs_State_Flag;
} VAD_States;

enum BACK_END_TYPE
{
   NONE     = 0, // No Back End Recogntion
    HMM_CD   = 1, // Scoring Based on HMM_CD only
    DTW_m    = 2, // Scoring Based on DTW with mean vector only
    DTW_dgm  = 4, // Scoring Based on DTW with diagonal gaussian mixtures
    ALL      = 7, // Use Combined ALL Scores of all Features
};

//-----------------------------------------------------------------------------
// Config Structs
//-----------------------------------------------------------------------------
typedef struct DC_OffsetFilterConfig
{
    UINT16   buffer_size;  // Size of the input buffer
    UINT16   window_size;  // ??? Size of the output buffer
    UINT16   n_yCoeffs;    // Order of denominator
    UINT16   n_xCoeffs;    // Order of enumerator
    FLOAT32 *yCoeffs;      // IIR Filter Coefficients
    FLOAT32 *xCoeffs;      // IIR Filter Coefficients
} CfgDCOffset;

typedef struct PreEmphasisConfig
{
    UINT16  buffer_size;
    FLOAT32 preemphasis_factor;
} CfgPreEmphasis;

typedef struct AdvanceWindowConfig
{
    UINT16       advance_n;
    UINT16       window_n;
} CfgAdvanceWindow;

typedef struct WindowConfig
{
    WINDOW_TYPE  windowType;
    UINT16       size;
    //FLOAT32     *wCoeffs;
} CfgWindow;

typedef struct LPC_Config
{
    UINT16       buffer_size;
    UINT16       lpc_order;
} CfgLPC;

typedef struct FFT_Config
{
    UINT16       in_buffer_size;      // window size or lpc_order
    UINT16       fft_size;            // 2^n
} CfgFFT;

typedef struct TimeSmoothConfig
{
    UINT16       vector_size;
    UINT16       filter_size;
    FLOAT32     *w;
} CfgTimeSmooth;

typedef struct SpectrumConfig
{
    bool monitor;
    wchar_t output_dir[_MAX_PATH];

    CfgDCOffset      cfgDCOffset;
    CfgPreEmphasis   cfgPreEmphasis;
    CfgAdvanceWindow cfgAdvanceWindow;
    CfgWindow        cfgWindow;
    CfgLPC           cfgLPC;
    CfgFFT           cfgFFT;
    CfgFFT           cfgFFTlpc;
    CfgTimeSmooth    cfgTimeSmooth;

} CfgSpectrum;

typedef struct MF_Config
{
    FLOAT32               sample_rate;
    FLOAT32               mel_starting_frequency;

    UINT16                fft_size;
    UINT16                num_mel_filters;
} CfgMelFilter;

typedef struct VLOG_Config
{
    UINT16                in_vec_size;
    FLOAT32               Energy_Floor_log;
} CfgVLog;

typedef struct DCT_Config
{
    UINT16                in_vec_size;
    UINT16                out_vec_size;
} CfgDCT;

typedef struct MFCC_Config
{
    bool                  bMonitor;
    char               output_dir[_MAX_PATH];
    UINT16                i16_in_vector_size;
    UINT16                i16_out_vector_size;

    CfgMelFilter          cfgMelFilter;
    CfgVLog               cfgVLog;
    CfgDCT                cfgDCT;

} CfgMFCC;

typedef struct VAD_Config
{
    UINT16                VAD_feature_buf_size;     // I am not sure if this is used
    UINT16                n_init_frames;
    UINT16                hangover;
    UINT16                n_min_frames;
    UINT16                n_min_frames2;
    UINT16                snr_threshold_vad;
    UINT16                snr_threshold_upd_lte;
    UINT16                nb_frame_threshold_lte;
    UINT16                min_speech_frame_hangover;

    FLOAT32               energy_floor;
    FLOAT32               lambda_lte_lower_e;
    FLOAT32               lambda_lte_higher_e;

   // Debugging & Monitoring Var's
   bool                        monitor;
   bool                  genVAD_FEF;

} CfgVAD;

typedef struct EnhanceConfig
{
    UINT16         spec_vector_size;
    UINT16         mfcc_vector_size;  // ????Why is this needed here????
    UINT16         center_size;
    UINT16         neighb_size;

    FLOAT32        sil_en_floor; // FLOOR
    FLOAT32        sf;           // Silence Factor - weight factor of the total energy of the signal
    FLOAT32        nf;           // Normalized Neighborhood Factor
    FLOAT32        bg;           // Background Energy Gain Factor
    FLOAT32        eg;           // Enhacement Module Gain Factor

    FLOAT32        alpha;        // Noise suppression factor

} CfgEnhance;

struct ENHANCED_FEATURES
{
    UINT16         spec_vector_size;
    UINT16         mfcc_vector_size;

    FLOAT32       *enhanced_spec_vec;
    FLOAT32       *enhanced_mfcc_vec;
};
//
// Buffering Module Defintions
//
typedef struct GradientConfig
{
    UINT16         delay;              // Buffer size equal to total delay
                                       //   incured by gradient computation
    UINT16         vec_size;           // Vector Lengths in number of elements
                                       //   of each type of feature vector
    UINT16         num_vecs;           // Number of Vectors used to compute grad1
                                       //   in each side of the center vector
} CfgGradient;

typedef struct DelayConfig
{
    BOOLEAN         bMonitor;
    char            output_dir[_MAX_PATH];  // All those allocated output_dir should be turned into pointers
                                          // pointing to only one memory space

    UINT16          delay;              // Buffer size = total delay
    UINT16          spec_vector_size;   // Vector Length in number of elements
    UINT16          mfcc_vector_size;   // Vector Length in number of elements
    UINT16          enh_mfcc_vector_size;   // Vector Length in number of elements

    UINT16          num_gradI;
    UINT16          num_gradII;

    // Configuration of Sub-Modules
    CfgGradient    cfgGradient1;
    CfgGradient    cfgGradient2;

} CfgDelay;

struct FE_FEATURES_STATS
{
    UINT16      spec_vector_size;        // Length of input spectral vector
    UINT16      mfcc_vector_size;        // Length of input spectral vector
    UINT16      enh_mfcc_vector_size; // Length of MFCC-ed ENH Spectrum Vector

    FLOAT32    *spec_buffer;          // buffer[delay][spec_vector_size]
    FLOAT32    *lpc_spec_buffer;      // buffer[delay][spec_vector_size]
    FLOAT32    *mfcc_buffer;          // buffer[delay][mfcc_vector_size]
    FLOAT32    *lpc_mfcc_buffer;      // buffer[delay][mfcc_vector_size]
    FLOAT32    *enh_mfcc_buffer;      // buffer[delay][mfcc_vector_size]

    FLOAT32    *ma_spec_vector;       // moving average vector estimate
    FLOAT32    *ma_lpc_spec_vector;   // moving average vector estimate
    FLOAT32    *ma_mfcc_vector;       // moving average vector estimate
    FLOAT32    *ma_lpc_mfcc_vector;   // moving average vector estimate
    FLOAT32    *ma_enh_mfcc_vector;   // moving average vector estimate
};
//
// AGC Support Data
//
#define LAMBDA_AGC           (FLOAT32)(0.995F)
#define AGC_TARGET_ENERGY    (FLOAT32)(180.0F)
#define MAX_VOLUME           (UINT32)(10000)

struct AGC_Info_Struct
{
   FLOAT32        aver_energy;
   FLOAT32        peek_energy;
};

//
// Output Front End Features Packet
//
struct FE_FEATURES
{
    VAD_STATE   vad_state;
    UINT16      frame_rate;           // Number of feature vectors per second
    //
    // Number of static and dynamic features
    //
    UINT16      num_mfcc_features;  // Number of elements in the vector
    UINT16      num_enh_features;   // Number of elements in the vector

    FLOAT32 *   fe_mfcc_features_buffer;
    FLOAT32 *   fe_lpc_mfcc_features_buffer;
    FLOAT32 *   fe_enh_features_buffer;
   //
   // Must transition to this new convention to accomodate more types of features
   //
   UINT16      num_feature_types;  // Number of feature types (e.g., MFCC, LPC, ENH)
   UINT16 *    num_features;       // Number of elements in a feature vector for each feature type
    FLOAT32 **  p32_fef_buffer;
   //
   // AGC Information
   //
   FLOAT32     Delta_AGC_Gain;
   AGC_Info_Struct *AGC_info_str;
};

struct SEGMENT_FEATURES
{
    UINT16             frame_rate;           // Number of feature vectors per second
   //
   // Number of static and dynamic features
   //
   UINT16             num_feature_vectors; // Total Number of vectors (of all types) in a segment

   UINT16             num_feature_types;   // Number of feature types (e.g., MFCC, LPC, ENH)
   UINT16             num_features;        // Number of elements in feature vector of each type
   //vector<UINT16 *>   num_features;        // Number of elements in feature vector of each type
   vector<FLOAT32 **> fe_features_buffer;  // Vector of pointers to each feature vector type

   // The back_end.cpp and dtw_score.cpp needs to be modified to a new convetion above
   UINT16             num_mfcc_features;   // Number of elements in a mfcc feature vector
   UINT16             num_enh_features;    // Number of elements in a enh feature vector

   vector<FLOAT32 *>  fe_mfcc_features_buffer;      // Vector of pointers to featueres buffer:
   vector<FLOAT32 *>  fe_lpc_mfcc_features_buffer;  // Vector of pointers to featueres buffer:
   vector<FLOAT32 *>  fe_enh_mfcc_features_buffer;  // Note that this means that each memory block pointed to must persist
   //FLOAT32 **         fe_mfcc_features_buffer;      // Vector of pointers to featueres buffer:
   //FLOAT32 **         fe_enh_mfcc_features_buffer;  // Note that this means that each memory block pointed to must persist
};

typedef struct FrontEndConfig
{
    UINT16         sample_rate;           // Number of samples per second
    UINT16         frame_rate;           // Number of analysis windows per second
    UINT16         frame_shift_size;     // Number of analysis windows per second
    UINT16         window_size;           // Size of analysis window in number of samples
    FLOAT32        window_duration;      // Size of analysis window in mili-seconds
    FLOAT32        preemphasis_factor;   // preemphasis factor
   FLOAT32        frame_energy_floor_log; // frame energy floor in dB
    WINDOW_TYPE    window_type;           // window type
    SPECTRUM_TYPE  sp_type;              // spectrogram feature type
    //FRONT_END_TYPE fe_type;              // front-end/feature type
    UINT16         fe_type;              // front-end/feature type
    UINT16         lpc_order;            // lpc order of the polinomial
    UINT16         fft_size;             // number of FFT Points - must be power of 2
    UINT16         num_mel_filters;      // Number of mel filters
    UINT16         num_out_enh_features; // Number of Output Features
    UINT16         num_out_mfcc_features;// Number of Output Features
    //
    // Sub-Modules Configuration Information
    //
    CfgSpectrum    cfgSpectrum;
    CfgMFCC        cfgMFCC;
    CfgVAD         cfgVAD;
    CfgDelay       cfgDelay;
    CfgEnhance     cfgEnhance;
    //
    // Useful Debugging Information
    //
    char        basefilename[_MAX_FNAME];
    char        in_data_dir[_MAX_PATH];
    char        out_data_dir[_MAX_PATH];
    char        output_dir[_MAX_PATH];

    BOOLEAN        monitor;
    BOOLEAN        liveinput;

} CfgFrontEnd, WUWCFG_FRONTEND;

struct CfgOutput
{
    bool           bMonitor;
    char        basefilename[_MAX_FNAME];
    char        output_dir[_MAX_PATH];
};

struct CfgInput
{
    SAMPLE_TYPE    sampleType;
    UINT16         sampleSize;
    UINT16         headerSize;
    UINT16         sampleRate;
    UINT16         bufferSize;
    CHAR*          dataFormat;
    BOOLEAN        bLiveAudio;
    char        filename[_MAX_PATH];
};

//typedef struct FEF_INFO {
//    FEATURE_TYPE   feature_type;
//    SAMPLE_TYPE    sample_type;  // data type of a vector
//    size_t         sample_size;  // value as returned by sizeof(sample_type);
//    UINT16         frame_rate;   // number of elements/dimensionality of a vector
//    UINT16         num_features; // number of elements/dimensionality of a vector
//};

//
// BackEnd
//
typedef struct DTW_m_MODEL      // Simple Means Only DTW Model
{
    wchar_t        model_name[_MAX_PATH];
   // Need to change this to refer to FEF_TYPE structure for each FEATURE TYPE
    FEATURE_TYPE   feature_type;
    SAMPLE_TYPE    sample_type;  // data type of a vector
    size_t         sample_size;  // value as returned by sizeof(sample_type);
    UINT16         frame_rate;   // number of elements/dimensionality of a vector
    UINT16         num_features; // number of elements/dimensionality of a vector
    UINT16         num_vectors;  // number of vectors

    FLOAT32        NUMD_FACTOR;  // Non-Uniformity Distortion Factor
    FLOAT32        NNZ_FACTOR;   // Number of Zeros Factor

    //FLOAT32      **dtw_m_model;
   vector<FLOAT32 *> *dtw_m_model; // Segment Features are stored as Vector data type
} DTW_m_Model;

typedef struct DTW_Model_Vects
{
   UINT16             num_vectors;
   UINT16             num_elements;
   //FLOAT64 **         v_DTW_model_vects; // Those are vectors that are used as accumulators during training
   //UINT32  *          v_DTW_vect_count;  // Holds the counter of accumulated vectors for each model vector
   vector<FLOAT64 *> *v_DTW_model_vects;
   vector<UINT32>    *v_DTW_vect_count;
} DTW_ModelVects;

typedef struct DTW_Config
{
    wchar_t       *DTW_model_filename;
    vector<wchar_t *> v_DTW_model_filenames;
    UINT16         DTW_header_size;
    UINT16         DTW_num_models;
    BACK_END_TYPE  DTW_type;            // model back-end/feature type
    DTW_m_MODEL   *DTW_model;           // DTW Model Loader has to be written
    vector<DTW_m_MODEL *> v_DTW_model;  // Vector of Pointers to DTW Model
   DTW_ModelVects *DTW_model_vecs_str; // Used in Training to hold DTW accumulators
    bool           mb_monitor;
} CfgDTW;


// ORDER IMPORTANT
enum HMM_FEF_TYPE {
    HMM_FEF_ENH = 0, 
    HMM_FEF_MFC = 1, 
    HMM_FEF_LPC = 2
};

typedef struct HMM_CD_Config
{
    char* model_filename[3];
    bool           mb_monitor;
} CfgHmm;

typedef struct Scoring_Config
{
    float threshold;
    float threshold2;
    //HMM_CD_MODEL   *HMMCD_model;  // HMM Model Loader has to be written
    bool           mb_monitor;

} CfgScoring;

typedef struct GenFEFConfig
{
   //
   // Pointer to FE Config Information
   //
   CfgFrontEnd*   cfgFrontEnd;
    //
    // Input-Output information
    //
    char        basefilename[_MAX_FNAME];
    char       *in_data_dir;
    char       *out_data_dir;
    char       *out_log_dir;
   char       *filelist;

   BOOLEAN        genFEF;
   BOOLEAN        genAllFEF;

} CfgGenFEF;

typedef struct BackEndConfig
{
    char        model[_MAX_PATH];
    char        svm_file[_MAX_PATH];
    char        model_dir[_MAX_PATH];
    char        modelbasefilename[_MAX_FNAME];
    //
    // Useful Debugging Information
    //
    char        basefilename[_MAX_FNAME];
    char        out_data_dir[_MAX_PATH];
    char        output_dir[_MAX_PATH];

    // Parameters
    BACK_END_TYPE  be_type;              // model back-end/feature type
    BACK_END_TYPE  be_scoring;           // Scoring
    bool           monitor;
   BOOLEAN        liveinput;
   //
   // Pointer to Training & FrontEnd Configuration Structure
   //
   CfgFrontEnd   *cfgFrontEnd;
   //CfgTrain      *cfgTrain;
   void          *cfgTrain;
   CfgGenFEF     *cfgGenFEF;
   //
    // Sub-Modules Configuration Information
    //
    CfgDTW         cfgDTW;
    CfgHmm         cfgHmm;
    CfgScoring     cfgScoring;

} CfgBackEnd;

// BaseFileName 2 FullFileName MAP
typedef   map<wstring, wstring, less<wstring> > BFN2FFN_MAP;
typedef   set<wstring> BFN_SET;

typedef struct TrainConfig
{
   //
   // Pointer to FE Config Information
   //
   CfgFrontEnd *  cfgFrontEnd;   // Is this needed ???
   CfgBackEnd  *  cfgBackEnd;    //
    //
    // Input-Output information
    //
    char        basefilename[_MAX_FNAME];
    char        modelfile[_MAX_FNAME];
    char       *modelname;
    char       *in_data_dir;
    char       *out_data_dir;
    char       *out_log_dir;
   char       *filelist;

    bool           monitor;
   BOOLEAN        train;
   BACK_END_TYPE  train_type;

   BFN2FFN_MAP   *fef_map;
   //void          *dtw_model;
   DTW_m_MODEL   *dtw_model;

} CfgTrain;


struct CfgWUW
{
    //vector<CfgInput> cfgInputs;
   //
   // Uninitialized Configuration Modules
   //
    CfgInput    cfgInput;
    CfgFrontEnd cfgFrontEnd;
    CfgBackEnd  cfgBackEnd;
   CfgTrain    cfgTrain;
};

//
// Default FE Parameters
//
#define FE_TYPE                        ((FET_ETSI_MFCC) | (FET_VK_LPC_ENH_MFCC))
#define SP_TYPE         (FET_VK_LPCSPEC)
#define FEF_TYPE                       ((FT_ETSI_MFCC) | (FT_VK_LPC_ENH_MFCC))
#define FE_SAMPLE_RATE            (8000)      // Sample Rate in Hz
#define FE_HEADER_SIZE               (0)      // Wave File Header Size
#define FE_WINDOW_DURATION          (25)      // In [msec]
// 200 samples <=> 25 msec  for 8000 SAMPLING_RATE
#define FE_WINDOW_SIZE                 ((FE_WINDOW_DURATION)*(FE_SAMPLE_RATE)/1000)
#define FE_WINDOW_TYPE           HAMMING
#define FE_FRAME_RATE              (200)      // Number of Frames Per Second
#define FE_FRAME_SHIFT_SIZE            ((FE_SAMPLE_RATE)/(FE_FRAME_RATE)) // 8000/200 = 40
#define FE_PREEMPH_FACTOR            (0.975F) // preemphasis factor
#define FE_NUM_FEATURES             (15)      // After DCT transformation
#define FE_LPC_ORDER                (17)      // LP Model Order (17)
#define FE_FFT_SIZE                (256)      // Number of FFT Points
#define FE_TIME1SMOOTH_FILTER_SIZE   (3)      // Spectral Smoothing Filter
//
// Default MEL FILTERING Configuration PARAMETERS
//
#define MEL_STARTING_FREQUENCY     (64.0F)    // Starting Low-Frequency of Mel-Filters
#define NUM_MEL_FILTERS            (25)       // Number of MEL Filters
#define FE_FRAME_ENERGY_FLOOR_LOG (-50.0F)    // dB Scale
//#define ENERGY_FLOOR_LOG          (-50.0F)    // Energy Floor in dB Scale ???
//
// Default DCT Configuration PARAMETERS
//
#define NUM_DCT_ELEMENTS           (13)
//
// VAD PARAMETER DEFINITIONS
//
//
#define LOG2                       (FLOAT32)(log(2.0F))
#define VAD_FEATURE_BUF_SIZE       (UINT16)(5)

#define N_INIT_FRAMES              (UINT16)(20)
#define HANGOVER                   (UINT16)(15)      //(15)
#define N_MIN_FRAMES               (UINT16)(10)
#define N_MIN_FRAMES2              (UINT16)(5)
#define SNR_THRESHOLD_VAD          (UINT16)(15)
#define SNR_THRESHOLD_UPD_LTE      (UINT16)(20)
#define SNR_EN_THRESHOLD_VAD       (UINT16)(75)      //(40)
#define SNR_EN_THRESHOLD_UPD_LTE   (UINT16)(150)     //(75)     //(20)
#define SNR_SPEC_THRESHOLD_VAD     (UINT16)(10)      //(7)       //(9) (10)
#define SNR_SPEC_THRESHOLD_UPD_LTE (UINT16)(15)      //(7)       //(15)
#define SNR_MFCC_THRESHOLD_VAD     (UINT16)(200)     //(5000)    //(200)
#define SNR_MFCC_THRESHOLD_UPD_LTE (UINT16)(300)     //(7500)    //(300)

#define NB_FRAME_THRESHOLD_LTE     (UINT16)(10)      //(20)
#define MIN_SPEECH_FRAME_HANGOVER  (UINT16)(15)      //(5)       //(15)       // (5)
                                                     // SVM VAD
#define VAD_ON_LEAD                (UINT16)(5)       //(15)
#define VAD_ON_MIN_COUNT           (UINT16)(10)      //(10)
#define VAD_OFF_MIN_COUNT          (UINT16)(15)      //(15)      //(40) // (35) (15)
#define VAD_OFF_TRAIL              (UINT16)(5)       //(20)      //(10)       // (5)

#define ENERGY_FLOOR               (FLOAT32)(200.0)  //(1.0)     // (50.0)
#define SPEC_VAR_FLOOR             (FLOAT32)(30.0)   //(1.0)     // (10.0)
#define MFCC_VAR_FLOOR             (FLOAT32)(100.0)  //(1.0)     // (10.0)
#define MFCC_FLOOR                 (FLOAT32)(100.0)

#define LAMBDA_VAD_ON              (FLOAT32)(0.99)

#define LAMBDA_LTE_LOWER_E         (FLOAT32)(0.97)   //(0.99)   //(0.97)
#define LAMBDA_LTE_HIGHER_E        (FLOAT32)(0.995)  //(0.995)  //(0.995) (0.99)
//
// ENHANCEMENT MODULE PARAMETER DEFINITIONS
//
#define CENTER_SIZE                (UINT16)(1)
#define NEIGHBORHOOD_SIZE          (UINT16)(5)
#define SIL_EN_FLOOR               (FLOAT32)(1.0e10F) //(1.0e12F); // (1.0e10F); // SILENCE FLOOR
#define SF                         (FLOAT32)(1.0e-2F) //(1.0e1F);  // (0.5e3F);  // Silence Factor - Total Energy Factor
#define NF                         (FLOAT32)(1.0e-2F) //(1.0e3Fg); // (1.0e1F);  // Neighborhood Factor
#define BG                         (FLOAT32)(1.0e2F)  //(1.0e0F);  // (1.0e9F);  // Bacground Energy Gain Factor
#define EG                         (FLOAT32)(1.0e8F)  //(1.0e10F); // (1.0e12F); // Enhancement Module Gain Factor
#define ALPHA                      (FLOAT32)(1.0F)    //(2.0F);   //(0.1F);    // Noise Suppression Factor
//
// Dynamic Features Parameters
//
// Number of frames spanning 50 msec divided by 2 => 5 Frames
#define NUM_GRAD1_VECS             (((FE_FRAME_RATE*50)/1000)/2)
// For Smoothed Grad1 values it sufficies to compute grad2 = x'[n+1]-x'[n-1]
#define NUM_GRAD2_VECS             (UINT16)(1)

enum FE_PARAM1 {
    PR_FE_TYPE,
    PR_FE_SAMPLE_RATE,
    PR_FE_FRAME_RATE,
    PR_FE_FRAME_SHIFT_SIZE,
    PR_FE_WINDOW_SIZE,
    PR_FE_WINDOW_TYPE,
    PR_FE_PREEMPH_FACTOR,
    PR_FE_LPC_ORDER,
    PR_FE_FFT_ORDER,
    PR_FE_NUM_SPEC_FEATURES,
    PR_FE_NUM_MFCC_FEATURES,
    PR_FE_NUM_ENH_FEATURES
};

enum SP_PARAM1 {
    PR_SP_FRAME_RATE,
    PR_SP_FRAME_SHIFT_SIZE,
    PR_SP_WINDOW_SIZE,
    PR_SP_WINDOW_TYPE,
    PR_SP_PREEMPH_FACTOR,
    PR_SP_LPC_ORDER,
    PR_SP_FFT_ORDER,
    PR_SP_NUM_SPEC_FEATURES
};

//
// Default BE Parameters
//
#define BE_SCORING                 (NONE)
#define DTW_HEADER_SIZE            (1024)
#define FEF_HEADER_SIZE            (1024)

#endif //DEFINES_H

/*******************************  End of File  *******************************/

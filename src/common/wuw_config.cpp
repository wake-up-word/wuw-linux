/******************************************************************************

Copyright (C) 2006-2015 VoiceKey Inc.
Cocoa Beach, Florida 32931
All rights reserved

Confidential and Proprietary

Module: wuw_config.cpp
Author: Veton Kepuska
Created: June 1, 2006
Updated:

Description: Configures wuw Recognizor from command line or config file.

$Log:$

*******************************************************************************

Notes:

******************************************************************************/


/******************************  Include Files  ******************************/

#include "wuw_config.h"


/*********************************  Defines  *********************************/

/****************************  Member Functions  *****************************/

//
// Constructors
//
WuwConfig::WuwConfig()
{
    Reset();
}

//
// Reset Function Sets all Default Parameters
//
void WuwConfig::Reset ()
{
    memset(&m_cfgInput, 0, sizeof(CfgInput));
    memset(&m_cfgFrontEnd, 0, sizeof(CfgFrontEnd));
    memset(&m_cfgBackEnd, 0, sizeof(CfgBackEnd));
    memset(&m_cfgTrain, 0, sizeof(CfgTrain));
    memset(&m_cfgGenFEF, 0, sizeof(CfgGenFEF));

   liveInput             = false;
   recordLiveInput       = false;

    // Input
    m_cfgInput.sampleType = ST_ULAW;
    m_cfgInput.sampleRate = FE_SAMPLE_RATE;
    m_cfgInput.sampleSize = 1;
    m_cfgInput.bufferSize = FE_FRAME_SHIFT_SIZE;
    m_cfgInput.headerSize = FE_HEADER_SIZE;


    // Frontend
    m_cfgFrontEnd.sample_rate           = FE_SAMPLE_RATE;
    m_cfgFrontEnd.frame_rate            = FE_FRAME_RATE;
    m_cfgFrontEnd.frame_shift_size      = FE_FRAME_SHIFT_SIZE;
    m_cfgFrontEnd.window_size           = FE_WINDOW_SIZE;
    m_cfgFrontEnd.window_duration       = FE_WINDOW_DURATION;
    m_cfgFrontEnd.window_type           = FE_WINDOW_TYPE;
    m_cfgFrontEnd.preemphasis_factor    = FE_PREEMPH_FACTOR;
    m_cfgFrontEnd.frame_energy_floor_log = FE_FRAME_ENERGY_FLOOR_LOG;
    m_cfgFrontEnd.fe_type               = FE_TYPE;
    m_cfgFrontEnd.lpc_order             = FE_LPC_ORDER;
    m_cfgFrontEnd.fft_size              = FE_FFT_SIZE;
    m_cfgFrontEnd.monitor               = false; //true; // false;


    // FrontEnd - Spectrum
    m_cfgFrontEnd.cfgSpectrum.monitor   = false; //true;
    // FrontEnd - Vad
    m_cfgFrontEnd.cfgVAD.monitor           = false; //true;

    // FrontEnd - Spectrum - DC Offset
    m_cfgFrontEnd.cfgSpectrum.cfgDCOffset.buffer_size  = FE_FRAME_SHIFT_SIZE;
    m_cfgFrontEnd.cfgSpectrum.cfgDCOffset.n_xCoeffs    = 2;
    m_cfgFrontEnd.cfgSpectrum.cfgDCOffset.n_yCoeffs    = 2;
    m_cfgFrontEnd.cfgSpectrum.cfgDCOffset.window_size  = FE_WINDOW_SIZE;
    m_cfgFrontEnd.cfgSpectrum.cfgDCOffset.xCoeffs      = new FLOAT32[2];
    m_cfgFrontEnd.cfgSpectrum.cfgDCOffset.xCoeffs[0]   = 1.0F;
    m_cfgFrontEnd.cfgSpectrum.cfgDCOffset.xCoeffs[1]   = -1.0F;
    m_cfgFrontEnd.cfgSpectrum.cfgDCOffset.yCoeffs      = new FLOAT32[2];
    m_cfgFrontEnd.cfgSpectrum.cfgDCOffset.yCoeffs[0]   = 1.0F;
    m_cfgFrontEnd.cfgSpectrum.cfgDCOffset.yCoeffs[1]   = 0.999F;

    // FrontEnd - Spectrum - AdvanceWindow
    m_cfgFrontEnd.cfgSpectrum.cfgAdvanceWindow.advance_n = FE_FRAME_SHIFT_SIZE;
    m_cfgFrontEnd.cfgSpectrum.cfgAdvanceWindow.window_n  = FE_WINDOW_SIZE;

    // FrontEnd - Spectrum - Window
    m_cfgFrontEnd.cfgSpectrum.cfgWindow.windowType = FE_WINDOW_TYPE;
    m_cfgFrontEnd.cfgSpectrum.cfgWindow.size       = FE_WINDOW_SIZE;

    // FrontEnd - Spectrum - PreEmphasis
    m_cfgFrontEnd.cfgSpectrum.cfgPreEmphasis.preemphasis_factor = FE_PREEMPH_FACTOR;
    m_cfgFrontEnd.cfgSpectrum.cfgPreEmphasis.buffer_size        = FE_FRAME_SHIFT_SIZE;

    // FrontEnd - Spectrum - LPC
    m_cfgFrontEnd.cfgSpectrum.cfgLPC.lpc_order   = FE_LPC_ORDER;
    m_cfgFrontEnd.cfgSpectrum.cfgLPC.buffer_size = FE_WINDOW_SIZE;

    // FrontEnd - Spectrum - FFT
    m_cfgFrontEnd.cfgSpectrum.cfgFFTlpc.in_buffer_size = FE_LPC_ORDER;
    m_cfgFrontEnd.cfgSpectrum.cfgFFTlpc.fft_size       = FE_FFT_SIZE;

    m_cfgFrontEnd.cfgSpectrum.cfgFFT.in_buffer_size = FE_WINDOW_SIZE;
    m_cfgFrontEnd.cfgSpectrum.cfgFFT.fft_size       = FE_FFT_SIZE;

    // FrontEnd - Spectrum - TimeSmooth
    m_cfgFrontEnd.cfgSpectrum.cfgTimeSmooth.filter_size = 3;
    m_cfgFrontEnd.cfgSpectrum.cfgTimeSmooth.vector_size = (FE_FFT_SIZE / 2);

    // FrontEnd - MFCC
    m_cfgFrontEnd.cfgMFCC.i16_in_vector_size  = FE_FFT_SIZE;
    m_cfgFrontEnd.cfgMFCC.i16_out_vector_size = NUM_DCT_ELEMENTS;

    // FrontEnd - MFCC - MelFilter
    m_cfgFrontEnd.cfgMFCC.cfgMelFilter.fft_size               = FE_FFT_SIZE;
    m_cfgFrontEnd.cfgMFCC.cfgMelFilter.mel_starting_frequency = MEL_STARTING_FREQUENCY;
    m_cfgFrontEnd.cfgMFCC.cfgMelFilter.num_mel_filters        = NUM_MEL_FILTERS;
    m_cfgFrontEnd.cfgMFCC.cfgMelFilter.sample_rate            = FE_SAMPLE_RATE;

    // FrontEnd - MFCC - VLog
    m_cfgFrontEnd.cfgMFCC.cfgVLog.Energy_Floor_log = FE_FRAME_ENERGY_FLOOR_LOG;
    m_cfgFrontEnd.cfgMFCC.cfgVLog.in_vec_size      = NUM_MEL_FILTERS;

    // FrontEnd - MFCC - DCT
    m_cfgFrontEnd.cfgMFCC.cfgDCT.in_vec_size  = NUM_MEL_FILTERS;
    m_cfgFrontEnd.cfgMFCC.cfgDCT.out_vec_size = NUM_DCT_ELEMENTS;

    // Delay Module
    m_cfgFrontEnd.cfgDelay.delay              = max(VAD_ON_MIN_COUNT + VAD_ON_LEAD, VAD_OFF_MIN_COUNT + VAD_OFF_TRAIL);   //  5+15
    m_cfgFrontEnd.cfgDelay.spec_vector_size   = FE_FFT_SIZE/2;                          //  256/2
    m_cfgFrontEnd.cfgDelay.mfcc_vector_size   = NUM_DCT_ELEMENTS;                       //  12
    m_cfgFrontEnd.cfgDelay.enh_mfcc_vector_size = NUM_DCT_ELEMENTS;                     //  12
    m_cfgFrontEnd.cfgDelay.num_gradI          = NUM_GRAD1_VECS;
    m_cfgFrontEnd.cfgDelay.num_gradII         = NUM_GRAD2_VECS;

    // Spectral Enhancement Module
    m_cfgFrontEnd.cfgEnhance.center_size      = CENTER_SIZE;
    m_cfgFrontEnd.cfgEnhance.neighb_size      = NEIGHBORHOOD_SIZE;
    m_cfgFrontEnd.cfgEnhance.mfcc_vector_size = NUM_DCT_ELEMENTS;
    m_cfgFrontEnd.cfgEnhance.spec_vector_size = FE_FFT_SIZE/2;
    m_cfgFrontEnd.cfgEnhance.sil_en_floor     = SIL_EN_FLOOR;
    m_cfgFrontEnd.cfgEnhance.eg               = EG;
    m_cfgFrontEnd.cfgEnhance.nf               = NF;
    m_cfgFrontEnd.cfgEnhance.bg               = BG;
    m_cfgFrontEnd.cfgEnhance.sf               = SF;

    // BackEnd Module
    m_cfgBackEnd.be_scoring                   = BE_SCORING;        // Use all features and All scoring types
    m_cfgBackEnd.be_type                      = DTW_m;
    m_cfgBackEnd.cfgDTW.DTW_type              = DTW_m;

    // Generate FEF Module
    m_cfgGenFEF.cfgFrontEnd                   = &m_cfgFrontEnd;

    // Train Module
    m_cfgTrain.cfgFrontEnd                    = &m_cfgFrontEnd;
    // Setting a Pointer in CfgBackEnd to CfgTrain configuration structure
    m_cfgBackEnd.cfgTrain                     = (CfgTrain *) &m_cfgTrain;
}
//
// Destructor
//
WuwConfig::~WuwConfig()
{
}

bool WuwConfig::ParseConfigFile(char* szFileName)
{
    FILE* f = fopen(szFileName, "r");
    if(!f)
    {
        wcerr << "Couldn't open '" << szFileName << "' for reading.\n";
        return false;
    }

    wcerr << "Config file parser not yet implemented\n";

    fclose(f);
    return false;
}
//
// Command Line Convention Information
//
void WuwConfig::printInfo(int argc, char* argv[])
{
    wcerr << "Usage: " << argv[0] << endl
        << "\t Options:" << endl
        << "\t\t<filename.conf>" << endl
        << "\t or" << endl
        << "\t\t -s \t sample rate     <[8], 16> kHz" << endl
        << "\t\t -f \t sample type     <[ulaw], alaw, short, nist>" << endl
        << "\t\t -h \t header size     <[1024]>" << endl
        << "\t\t -i \t input           <d:/path/filename.ulaw>" << endl
        << "\t\t -l \t live input" << endl
        << "\t\t -r \t record live input" << endl
        << "\t\t -d \t output data dir <o:/path or ../path>" << endl
        << "\t\t -o \t FE features dir <o:/path or ../path>" << endl
        << "\t\t -G \t generate FEF    <o:/path/wavefiles.list>" << endl
        << "\t\t -e \t ENH features    <o:/path/filename.enh or ../path/filename.enh>" << endl
        << "\t\t -M \t model files     <m:/path/WUWmodel1.dtw m:/path/WUWmodel2.dtw ...>" << endl
        << "\t\t -S \t SVM Model       <m:/path/SVMmodel1.svm>" << endl
        << "\t\t -m \t monitor         <true, [false]> binary switch - if used is set to \"true\"" << endl
        << "\t\t -t \t modelname train <true, [false]> sets binary switch - if used is set to \"true\"" << endl;
}
//
// Command Line Parsing Function
//

bool WuwConfig::ParseCommandLine (int argc, char* argv[])
{
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR], tmp[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];
    int  i, j, num_models = 0;
    bool batchFlag = false;

    m_cfgFrontEnd.monitor             = false;
    m_cfgFrontEnd.cfgSpectrum.monitor = false;
    m_cfgFrontEnd.cfgDelay.bMonitor   = false;
    m_cfgFrontEnd.cfgVAD.monitor      = false;

    m_cfgBackEnd.monitor              = false;
    m_cfgFrontEnd.liveinput           = false;
    m_cfgBackEnd.liveinput            = false;


    memset(m_cfgBackEnd.cfgHmm.model_filename, 0, 3 * sizeof(char*));

    for (i=1; i<argc; i++)
    {
      fprintf(stderr, "[%2d] INPUT ARG: %S\n", i, argv[i]);
        // Checking for specific switches up-front: -L, -T -t
        // then -i is a path otherwise it specifies an input file
        if (argv[i][0] == '-' )
        {
            switch (argv[i][1])
            {
            case 't':              // Training Models
                {
                    m_cfgTrain.train      = true;
                    // Case must be determined based on the input type
                    m_cfgTrain.train_type = DTW_m;
                    break;
                }
            case 'T':              // Transcription File
                {
                    batchFlag            = true;
                    break;
                }
            case 'L':              // List File
                {
                    batchFlag            = true;
                    break;
                }
            case 'G':              // List File
            case 'g':              // No List File
                {
                    m_cfgGenFEF.genFEF   = true;
                    break;
                }

            case 'D':              // Generate All Features
                {
                    m_cfgGenFEF.genAllFEF = true;
                    break;
                }

         case 'l':              // List File
                {
                    liveInput             = true;
                    break;
                }

            }
        }
    }
    //
    // Scanning second time to set filelist to appropriate sturcture
    //
    if (batchFlag) {
        for (i=1; i<argc; i++)
        {
            // Checking for specific switches up-front: -L, -T -t
            // then -i is a path otherwise it specifies an input file
            if (argv[i][0] == '-' )
            {
                switch (argv[i][1])
                {
                case 'L':
                    {
                        strcpy(fname, std::filesystem::path(argv[i+1]).filename().string().c_str());
                        strcpy(dir, std::filesystem::path(argv[i+1]).parent_path().string().c_str());
                        // int err = _wsplitpath_s(argv[i+1], drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);

                        // if(err != 0) {
                        //     wcerr << "ERROR: Parsing input wave file\n" << argv[i+1] << endl
                        //         << "\t Drive: " << drive << endl
                        //         << "\t Dir  : " << dir << endl
                        //         << "\t File : " << fname << endl
                        //         << "\t Ext  : " << ext << endl;

                        //     return false;
                        // }
                    }
                    if (m_cfgTrain.train) {
                        m_cfgTrain.filelist = strdup(argv[i+1]);
                        memset(m_cfgTrain.basefilename, 0, sizeof(char)*_MAX_FNAME);
                        sprintf(m_cfgTrain.basefilename, "%s", fname);
                    }
                    if (m_cfgGenFEF.genFEF || m_cfgGenFEF.genAllFEF) {
                        m_cfgGenFEF.filelist = strdup(argv[i+1]);
                        memset(m_cfgGenFEF.basefilename, 0, sizeof(char)*_MAX_FNAME);
                        sprintf(m_cfgGenFEF.basefilename, "%s", fname);
                    }
                    break;

                case 'l':
                    {
                        if (m_cfgTrain.train) {
                            m_cfgTrain.out_log_dir = strdup(argv[i+1]);
                        }
                        if (m_cfgGenFEF.genFEF || m_cfgGenFEF.genAllFEF) {
                            m_cfgGenFEF.out_log_dir = strdup(argv[i+1]);
                        }
                        break;
                    }
                }
            }
        }
    }
    for (i=1; i<argc; i++)
    {
        if (argv[i][0] == '-' )
        {

            switch (argv[i][1])
            {
            case 's':              // Sample Rate
                {
                    m_cfgFrontEnd.sample_rate = atof(argv[i+1]);
                    break;
                }

            case 'f':             // Sample Type
                {
                    char* szSampleType = strdup(argv[i+1]);

                    if (strcmp(szSampleType, "ulaw") == 0)
                    {
                        m_cfgInput.sampleType = ST_ULAW;
                        m_cfgInput.sampleSize = 1;
                    }
                    else if (strcmp(szSampleType, "alaw") == 0)
                    {
                        m_cfgInput.sampleType = ST_ALAW;
                        m_cfgInput.sampleSize = 1;
                    }
                    else if (strcmp(szSampleType, "short") == 0)
                    {
                        m_cfgInput.sampleType = ST_SINT16;
                        m_cfgInput.sampleSize = 2;
                    }
                    else if (strcmp(szSampleType, "nist") == 0)
                    {
                        cerr << "WARNING: Not able to handle NIST formated files yet. "
                            << "Use External Tools to convert the file to a compatible format.\n";
                        return false;
                    }
                    else
                    {
                        cerr << "WARNING: Unknown case for data format: " << szSampleType << "\n"
                            << "Must specify one of the known formats. " << "\n";
                        return false;
                    }
                    break;
                }

            case 'h':             // Header Size
                {
                    m_cfgInput.headerSize = atof(argv[i+1]);
                    break;
                }

            case 'i':             // Input File or Input Data Directory
                {
                    if (batchFlag == false) {
                        
                        // printf("input %s\n", argv[i+1]);
                        strcpy(m_cfgInput.filename, std::filesystem::absolute(argv[i+1]).string().c_str());
                        strcpy(fname, std::filesystem::path(argv[i+1]).filename().string().c_str());
                        strcpy(dir, std::filesystem::path(argv[i+1]).parent_path().string().c_str());
                        // printf("copy %s\n", fname);


                        if(m_cfgFrontEnd.out_data_dir[0] == NULL) {
                            memset(m_cfgFrontEnd.out_data_dir, 0, sizeof(char)*_MAX_PATH);
                            sprintf(m_cfgFrontEnd.out_data_dir, "%s%s", dir, fname);
                        }

                        if(m_cfgBackEnd.out_data_dir[0] == NULL) {
                            memset(m_cfgBackEnd.out_data_dir, 0, sizeof(char)*_MAX_PATH);
                            sprintf(m_cfgBackEnd.out_data_dir, "%s%s", dir, fname);
                        }

                        memset(m_cfgFrontEnd.in_data_dir, 0, sizeof(char)*_MAX_PATH);
                        sprintf(m_cfgFrontEnd.in_data_dir, "%s", dir);

                        strcpy(m_cfgFrontEnd.basefilename, fname);
                        strcpy(m_cfgBackEnd.basefilename, fname);
                    }
                    else {            // it is input data directory
                        if (m_cfgTrain.train) {
                            m_cfgTrain.in_data_dir = strdup(argv[i+1]);
                        }
                        if (m_cfgGenFEF.genFEF || m_cfgGenFEF.genAllFEF) {
                            m_cfgGenFEF.in_data_dir = strdup(argv[i+1]);
                        }
                    }
                    break;
                }
            case 'd':             // Output Data directory path
                {
                    memset(m_cfgFrontEnd.out_data_dir, 0, sizeof(char)*_MAX_PATH);
                    sprintf(m_cfgFrontEnd.out_data_dir, "%s", argv[i+1]);
                    memset(m_cfgBackEnd.out_data_dir, 0, sizeof(char)*_MAX_PATH);
                    sprintf(m_cfgBackEnd.out_data_dir, "%s", argv[i+1]);
                    break;
                }
            case 'o':             // Output Front End Features directory path
                if (m_cfgTrain.train)
                    m_cfgTrain.out_data_dir = strdup(argv[i+1]);

                if ((m_cfgGenFEF.genFEF) || (m_cfgGenFEF.genAllFEF))
                    m_cfgGenFEF.out_data_dir = strdup(argv[i+1]);
                break;

            case 'e':             // Output Enhanced Features path/file.enh
                enh_filename = strdup(argv[i+1]);
                break;

            case 'm':             // Binary switch indicating monitoring
                // Can check the level of monitoring from argv[i+1] to set individual modules
                // monitor                           = _wcsdup(argv[i+1]);
                m_cfgFrontEnd.monitor             = true;
                m_cfgFrontEnd.cfgSpectrum.monitor = true;
                m_cfgFrontEnd.cfgDelay.bMonitor   = true;
                m_cfgFrontEnd.cfgVAD.monitor      = true;

                m_cfgBackEnd.monitor              = true;
                break;

            case 'M':             // Model Files -- at a moment only dtw's with hmm's need to test file extension
                {
                    // Counting Number of model files
                    // Need to develop a single file convention for multiple models of the same type
                    j = i+1;
                    while (j<argc) {
                        if (argv[j++][0] == '-') {
                            break;
                        }
                        num_models++;
                    }

                    m_cfgBackEnd.cfgDTW.DTW_num_models = num_models;

                    for (j=0; j<num_models; j++) {
                        // Extending model file with full path
                        // _wfullpath(m_cfgBackEnd.model, argv[i+1+j], _MAX_PATH);
                        strcpy(m_cfgBackEnd.model, std::filesystem::absolute(argv[i+1+j]).string().c_str());
                        strcpy(fname, std::filesystem::path(argv[i+1+j]).filename().string().c_str());
                        strcpy(dir, std::filesystem::path(argv[i+1+j]).parent_path().string().c_str());
                        strcpy(ext, std::filesystem::absolute(argv[i+1+j]).extension().string().c_str());
                        // drive = 'x';
                        // int err = _wsplitpath_s(m_cfgBackEnd.model, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);

                        // if(err != 0) {
                        //     wcerr << L"ERROR: Parsing input wave file\n" << m_cfgInput.filename << endl
                        //         << L"\t Drive: " << drive << endl
                        //         << L"\t Dir  : " << dir << endl
                        //         << L"\t File : " << fname << endl
                        //         << L"\t Ext  : " << ext << endl;

                        //     return false;
                        // }
                        if(m_cfgBackEnd.model_dir[0] == NULL)
                            memset(m_cfgBackEnd.model_dir, 0, sizeof(char)*_MAX_PATH);
                            sprintf(m_cfgBackEnd.model_dir, "%s%s", dir, fname);

                        // if (strcmp(ext, ".dtw") == 0)
                        // {
                        //     m_cfgBackEnd.be_type          = DTW_m;
                        //     m_cfgBackEnd.cfgDTW.DTW_type  = DTW_m;
                        //     m_cfgBackEnd.cfgDTW.v_DTW_model_filenames.push_back((char *)m_cfgBackEnd.model);
                        //     if (num_models == 1) {
                        //         // m_cfgBackEnd.cfgDTW.DTW_model_filename = m_cfgBackEnd.model;
                        //         sprintf(m_cfgBackEnd.modelbasefilename, "%s", fname);
                        //     }
                        //     else if (j==0) {
                        //         sprintf(m_cfgBackEnd.modelbasefilename, "%s_%dm", fname, num_models);
                        //     }
                        // }
                        // else
                        if(strcmp(ext, ".enh_bhmm") == 0 || strcmp(ext, ".e") == 0)
                        {
                            m_cfgBackEnd.be_type    = HMM_CD;
                            m_cfgBackEnd.be_scoring = HMM_CD;
                            m_cfgBackEnd.cfgHmm.model_filename[HMM_FEF_ENH] = strdup(m_cfgBackEnd.model);
                        }
                        else if(strcmp(ext, ".mfc_bhmm") == 0 || strcmp(ext, ".m") == 0)
                        {
                            m_cfgBackEnd.be_type    = HMM_CD;
                            m_cfgBackEnd.be_scoring = HMM_CD;
                            m_cfgBackEnd.cfgHmm.model_filename[HMM_FEF_MFC] = strdup(m_cfgBackEnd.model);
                        }
                        else if(strcmp(ext, ".lpc_bhmm") == 0 || strcmp(ext, ".l") == 0)
                        {
                            m_cfgBackEnd.be_type    = HMM_CD;
                            m_cfgBackEnd.be_scoring = HMM_CD;
                            m_cfgBackEnd.cfgHmm.model_filename[HMM_FEF_LPC] = strdup(m_cfgBackEnd.model);
                        }
                    }
                    break;
                }
            case 'S':             // SVM Model
            {
               sprintf(m_cfgBackEnd.svm_file, "%s", argv[i+1]);
               m_cfgBackEnd.cfgScoring.threshold  = atof(argv[i+2]);
               m_cfgBackEnd.cfgScoring.threshold2 = atof(argv[i+3]);
               //fwprintf(stderr, L"\nSVM: %s %s %s\n", argv[i+1], argv[i+2], argv[i+3]);
               //fwprintf(stderr, L"Read: %f %f\n", m_cfgBackEnd.cfgScoring.threshold, m_cfgBackEnd.cfgScoring.threshold2);

               break;
            }

            case 'r':             // Record Live Input
            {
               if (liveInput) {
                  recordLiveInput         = true;
                  m_cfgFrontEnd.liveinput = true;
                  m_cfgBackEnd.liveinput  = true;

                  if(m_cfgFrontEnd.out_data_dir[0] == NULL) {
                     memset(m_cfgFrontEnd.out_data_dir, 0, sizeof(char)*_MAX_PATH);
                     sprintf(m_cfgFrontEnd.out_data_dir, ".\\");
                  }

                  if(m_cfgBackEnd.out_data_dir[0] == NULL) {
                     memset(m_cfgBackEnd.out_data_dir, 0, sizeof(char)*_MAX_PATH);
                     sprintf(m_cfgBackEnd.out_data_dir, ".\\");
                  }

                  if (argv[i+1][0] == '-' ) {
                     if (m_cfgFrontEnd.basefilename[0] == NULL) {
                        sprintf(m_cfgFrontEnd.basefilename, "liveinput");
                     }
                     if (m_cfgBackEnd.basefilename[0] == NULL) {
                        sprintf(m_cfgBackEnd.basefilename, "liveinput");
                     }
                  }
                  else {
                     if (m_cfgFrontEnd.basefilename[0] == NULL) {
                        sprintf(m_cfgFrontEnd.basefilename, argv[i+1]);
                     }
                     if (m_cfgBackEnd.basefilename[0] == NULL) {
                        sprintf(m_cfgBackEnd.basefilename, argv[i+1]);
                     }
                  }
               }
               break;
            }
         case 't':             // Used to initiate training for dtw and/or hmm models
            {
               m_cfgTrain.train      = true;
               m_cfgTrain.modelname  = strdup(argv[i+1]);

               break;
            }
         case 'v':             // Used to initiate training for dtw and/or hmm models
            {
               m_cfgFrontEnd.cfgVAD.genVAD_FEF      = true;
               break;
            }
            case '?':
            {
               printInfo(argc, argv);
               break;
            }
            }
        }
    }
    // Non-batch mode feature generation
    //if (m_cfgTrain.train) {
    //   // Parsing output feature file into full path, basefilename and extension
    //   int err = _wsplitpath_s(m_cfgInput.filename, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);

    //   if(err != 0) {
    //      wcerr << L"ERROR: Parsing input wave file\n" << m_cfgInput.filename << endl
    //            << L"\t Drive: " << drive << endl
    //            << L"\t Dir  : " << dir << endl
    //            << L"\t File : " << fname << endl
    //            << L"\t Ext  : " << ext << endl;
    //      return false;
    //   }

    //   swprintf_s(m_cfgTrain.basefilename, _MAX_FNAME, L"%s", fname);

    //   if(m_cfgTrain.in_data_dir[0] == NULL)
    //      swprintf_s(m_cfgTrain.in_data_dir, _MAX_PATH, L"%s%s", drive, dir);

    //   if(m_cfgTrain.out_data_dir[0] == NULL)
    //      swprintf_s(m_cfgTrain.out_data_dir, _MAX_PATH, L"%s", L"fef");

    //}

    return true;
}
/*******************************  End of File  *******************************/

/******************************************************************************

Copyright (C) 2006-2015 VoiceKey Inc.
Cocoa Beach, Florida 32931
All rights reserved

Confidential and Proprietary


Module: vad.cpp
Author: Veton Kepuska
Created: September 02, 2006
Updated:

Description: A modified version of VAD class definition based on 2004
Veton Kepuska's version. It combines ETSI VAD with proprietary
solutions based on Veton Kepuska's original idea of VAD feature.

Inspired by David Shipman's  model of modular software
architecture originaly implemented in C.

$Log:$

*******************************************************************************

Proprietary and Confidential. All rights reserved.
VoiceKey Inc., Cocoa Beach, Florida 32931.

*******************************************************************************

Notes:
VAD does not do buffering. It is the responsibility of the calling
function to store/buffer the feature data in order to synchronize streams.
Synchronization of feature streams between the modules (e.g., front-end &
back-end) is done based on the VAD configuration and its delay as well as
the functioning of the overall system. Therefore, the burden of figuring out
the latency of each module is upon the calling module.
Main governing principle is that the higher level modules will take on
syncrhonization as well as buffering functions as necessary to allow
simplification of the each module implementation.

******************************************************************************/


/******************************  Include Files  ******************************/

#include <cmath>
#include <cstdlib>

#include <algorithm>

#include <iostream>
using namespace std;

#include <iomanip>
using namespace std;

#include "vad.h"

/*********************************  Defines  *********************************/
//#define SIGN(X) ((X)?(1.0F):(-1.0F))
//
//==============================================================================
// All parameters that define VAD fetures and its performance
// must be tuned based on statistics of specific WUW.
// Note that those statistics can be collected during WUW model building
// and incroporated into the model.
// Having WUW Model contain those statistics allows for VAD to be configured
// upon WUW model load. Model loading can be done at
//    1) Start of application or
//    2) Dynamicaly as provisioned by the application.
//==============================================================================

/********************************** Defines **********************************/

/****************************  Function Members  *****************************/
//
// Default Constructor
//
VAD::VAD()
{
	Reset();
	//
	// Input Vector Sizes
	//
	in_spec_vector_size       = FE_FFT_SIZE/2;
	spec_vector_size          = FE_FFT_SIZE/2;
	in_mfcc_vector_size       = NUM_DCT_ELEMENTS;
	mfcc_vector_size          = NUM_DCT_ELEMENTS;
	//
	// VAD Configuration Parameters -- must be set from config file or model

	n_init_frames             = N_INIT_FRAMES;             // 10
	hangover                  = HANGOVER;                  // 15
	n_min_frames              = N_MIN_FRAMES;              // 10
	n_min_frames2             = N_MIN_FRAMES2;             //  5
	snr_threshold_vad         = SNR_THRESHOLD_VAD;         // 15
	snr_threshold_upd_lte     = SNR_THRESHOLD_UPD_LTE;  // 20
	nb_frame_threshold_lte    = NB_FRAME_THRESHOLD_LTE;    // 10
	min_speech_frame_hangover = MIN_SPEECH_FRAME_HANGOVER; //  5

	VAD_ON_min_count          = VAD_ON_MIN_COUNT;          // 10
	VAD_ON_lead_count         = VAD_ON_LEAD;               //  5
	VAD_ON_Delay              = VAD_ON_min_count - VAD_ON_lead_count;
	VAD_OFF_min_count         = VAD_OFF_MIN_COUNT;         // 15
	VAD_OFF_trail_count       = VAD_OFF_TRAIL;             //  5
	VAD_OFF_Delay             = VAD_OFF_min_count + VAD_OFF_trail_count;
	VAD_Delay                 = max((VAD_ON_MIN_COUNT + VAD_ON_LEAD),      // 10 + 5
		(VAD_OFF_MIN_COUNT + VAD_OFF_TRAIL));  // 25 + 5

	energy_floor              = ENERGY_FLOOR;              // 80
	lambda_lte_lower_e        = LAMBDA_LTE_LOWER_E;        //  0.97
	lambda_lte_higher_e       = LAMBDA_LTE_HIGHER_E;       //  0.99

	VAD_State_Flag            = VAD_INIT;
	myVAD_State_Flag          = VAD_INIT;
}
//
// Configurable Constructor -- needs to be converted to Config Initialization
//
VAD::VAD(VAD_Config *vadConfig, char* output_basename)
{
	Reset();
	//
	// Input Vector Sizes
	//
	in_spec_vector_size       = FE_FFT_SIZE/2;
	spec_vector_size          = FE_FFT_SIZE/2;
	in_mfcc_vector_size       = NUM_DCT_ELEMENTS;
	mfcc_vector_size          = NUM_DCT_ELEMENTS;
	//
	// VAD Configuration Parameters -- must be set from config file or model
	//
	n_init_frames             = N_INIT_FRAMES;             // 20
	hangover                  = HANGOVER;                  // 15
	n_min_frames              = N_MIN_FRAMES;              // 10
	n_min_frames2             = N_MIN_FRAMES2;             //  5
	snr_threshold_vad         = SNR_THRESHOLD_VAD;         // 15
	snr_threshold_upd_lte     = SNR_THRESHOLD_UPD_LTE;     // 20
	nb_frame_threshold_lte    = NB_FRAME_THRESHOLD_LTE;    // 10
	min_speech_frame_hangover = MIN_SPEECH_FRAME_HANGOVER; //  5

	VAD_ON_min_count          = VAD_ON_MIN_COUNT;          // 10
	VAD_ON_lead_count         = VAD_ON_LEAD;               //  5
	VAD_ON_Delay              = VAD_ON_min_count - VAD_ON_lead_count;
	VAD_OFF_min_count         = VAD_OFF_MIN_COUNT;         // 15
	VAD_OFF_trail_count       = VAD_OFF_TRAIL;             //  5
	VAD_OFF_Delay             = VAD_OFF_min_count + VAD_OFF_trail_count;
	VAD_Delay                 = max((VAD_ON_MIN_COUNT + VAD_ON_LEAD),      // 10 + 5
		(VAD_OFF_MIN_COUNT + VAD_OFF_TRAIL));  // 25 + 5

	energy_floor              = ENERGY_FLOOR;              // 80
	lambda_lte_lower_e        = LAMBDA_LTE_LOWER_E;        //  0.97
	lambda_lte_higher_e       = LAMBDA_LTE_HIGHER_E;       //  0.99

	VAD_State_Flag            = VAD_INIT;
	myVAD_State_Flag          = VAD_INIT;

	monitor  = vadConfig->monitor;
	if (monitor)
		Open_MonitorFiles(output_basename);

	genVAD_FEF = vadConfig->genVAD_FEF;
	fp_vadFEF = NULL;
	if (genVAD_FEF) {
		string _output_basename(output_basename);
		size_t found = _output_basename.find_last_of(".");
		_output_basename.replace(found,1,"_");
		
		char filename[_MAX_PATH];

		sprintf(filename, "%s.vad", _output_basename.c_str());
		fp_vadFEF     = fopen(filename, "wb");
		if (!fp_vadFEF)
			fprintf(stderr,"ERROR: Unable to open the file %s\n", filename);
	}
	else {
		svm_model = svm_load_model("models/vad.svm");
		if(!svm_model)
			printf("Failed to load VAD SVM model\n");

		svm_features[6].index = -1;
	}

}
//
// Reseting VAD object to default configuration
//
void VAD::Reset()
{
	frame_counter         = 0;
	log2_frame_energy     = 0.0F;
	mean_log2_fm_en       = ENERGY_FLOOR;
	mean_log2_fm_en_VAD_ON = ENERGY_FLOOR;
	hangover              = 0;
	n_speech_frames       = 0;

	var_spec              = 0.0F;
	hangover_spec         = 0;
	n_speech_frames_spec  = 0;

	vad_mfcc_feature      = 0.0F;
	mean_vad_mfcc_feature = 0.0F;
	hangover_mfcc         = 0;
	n_speech_frames_mfcc  = 0;

	aver_var_mfcc         = 0.0F;
	prev_mean_mfcc        = 0.0F;
	prev_mean_var_mfcc    = 0.0F;
	var_var_mfcc          = 0.0F;
	var_mfcc_VAD_ON		   = 0.0F;

	VAD_ON_Count          = 0;
	VAD_OFF_Count         = 0;


	aver_var_spec         = SPEC_VAR_FLOOR;

	VAD_State_Flag        = VAD_INIT;
	myVAD_State_Flag      = VAD_INIT;
	VAD_EN_State_Flag     = VAD_INIT;
	VAD_SPEC_State_Flag   = VAD_INIT;
	VAD_MFCC_State_Flag   = VAD_INIT;
	VAD_MFCC_Enhs_State_Flag = VAD_INIT;

}
//
// Vad Open monitor files
//
void VAD::Open_MonitorFiles(char* output_basename) {

	// Build file path
	char filename[_MAX_PATH];
	strcpy(filename, output_basename);
	char * ext = filename + strlen(filename); // setting the pointer to end of wchar string

	// Energy Based VAD
	strcpy(ext, ".log2");
	v_logger_log2_frame_energy = new WuwLogger(filename, sizeof(FLOAT32), 1);
	strcpy(ext, ".mlog2");
	v_logger_energy_mean_log2 = new WuwLogger(filename, sizeof(FLOAT32), 1);
	strcpy(ext, ".ENflg");
	v_logger_EN_state_flag = new WuwLogger(filename, sizeof(SINT32), 1);
	// Spectrum Based VAD
	strcpy(ext, ".vars");
	v_logger_spec_var_spec = new WuwLogger(filename, sizeof(FLOAT32), 1);
	strcpy(ext, ".avars");
	v_logger_spec_avg_var = new WuwLogger(filename, sizeof(FLOAT32), 1);
	strcpy(ext, ".SPflg");
	v_logger_SPEC_state_flag = new WuwLogger(filename, sizeof(SINT32), 1);
	// MFCC Feature Based VAD
	strcpy(ext, ".mfccf");
	v_logger_mfcc_feature = new WuwLogger(filename, sizeof(FLOAT32), 1);
	strcpy(ext, ".mmfccf");
	v_logger_mfcc_feature_mean = new WuwLogger(filename, sizeof(FLOAT32), 1);
	strcpy(ext, ".MFflg");
	v_logger_MFCC_state_flag = new WuwLogger(filename, sizeof(SINT32), 1);

	strcpy(ext, ".SVMflg");
	v_logger_SVM_state_flag = new WuwLogger(filename, sizeof(SINT32), 1);

	strcpy(ext, ".MFEflg");
	v_logger_MFCC_enhs_state_flag = new WuwLogger(filename, sizeof(SINT32), 1);

	strcpy(ext, ".VADflg");
	v_logger_VAD_state_flag = new WuwLogger(filename, sizeof(SINT32), 1);

}

//
// Vad End monitor files
//

void VAD::End_Monitor()
{
	delete v_logger_log2_frame_energy;
	delete v_logger_energy_mean_log2;
	delete v_logger_EN_state_flag;
	delete v_logger_spec_var_spec;
	delete v_logger_spec_avg_var;
	delete v_logger_SPEC_state_flag;
	delete v_logger_mfcc_feature;
	delete v_logger_mfcc_feature_mean;
	delete v_logger_MFCC_state_flag;
	delete v_logger_SVM_state_flag;

	delete v_logger_MFCC_enhs_state_flag;
	delete v_logger_VAD_state_flag;
}
//
// Vad Run monitor files
//
void VAD::Run_Monitor() {
	SINT32 vad;

	v_logger_log2_frame_energy->WriteBinary(&log2_frame_energy);
	v_logger_energy_mean_log2->WriteBinary(&mean_log2_fm_en);
	//v_logger_EN_state_flag->WriteBinary(&VAD_EN_State_Flag);
	if (VAD_EN_State_Flag == VAD_ON) {
		vad = 1;
	} else {
		vad = 0;
	}
	v_logger_EN_state_flag->WriteBinary(&vad);

	v_logger_spec_var_spec->WriteBinary(&var_spec);
	v_logger_spec_avg_var->WriteBinary(&aver_var_spec);
	//v_logger_SPEC_state_flag->WriteBinary(&VAD_SPEC_State_Flag);
	if (VAD_SPEC_State_Flag == VAD_ON) {
		vad = 1;
	} else {
		vad = 0;
	}
	v_logger_SPEC_state_flag->WriteBinary(&vad);

	//v_logger_mfcc_feature->WriteBinary(&vad_mfcc_feature);
	v_logger_mfcc_feature->WriteBinary(&vad_mfcc_feature);
	v_logger_mfcc_feature_mean->WriteBinary(&mean_vad_mfcc_feature);
	//v_logger_MFCC_state_flag->WriteBinary(&VAD_MFCC_State_Flag);
	if (VAD_MFCC_State_Flag == VAD_ON) {
		vad = 1;
	} else {
		vad = 0;
	}
	v_logger_MFCC_state_flag->WriteBinary(&vad);

	if (VAD_SVM_State_Flag == VAD_ON) {
		vad = 1;
	} else {
		vad = 0;
	}
	v_logger_SVM_state_flag->WriteBinary(&vad);

	//v_logger_MFCC_enhs_state_flag->WriteBinary(&VAD_MFCC_Enhs_State_Flag);
	if (VAD_MFCC_Enhs_State_Flag == VAD_ON) {
		vad = 1;
	} else {
		vad = 0;
	}
	v_logger_MFCC_enhs_state_flag->WriteBinary(&vad);

	//v_logger_VAD_state_flag->WriteBinary(&VAD_State_Flag);
	if (VAD_State_Flag == VAD_ON) {
		vad = 1;
	} else {
		vad = 0;
	}
	v_logger_VAD_state_flag->WriteBinary(&vad);
}
//
// VAD Destructor
//
VAD::~VAD()
{
	if (monitor)
		End_Monitor();

	if (fp_vadFEF)
		fclose(fp_vadFEF);

	// To be implemented
}
//
// Initialization; Re-Configuration of the object
//
void VAD::Init(VAD_Config *vadConfig)
{
	Reset();
	// Remaining Parameters must be intialized from vadConfig
}
//
// End-of-Stream Processing
//
VAD_STATE VAD::Eos()
{
	// To be implemented
	return(myVAD_State_Flag);
}
//
// Computation of VAD State based on
//    1) Current internal VAD state
//    2) current VAD feature
//
// Input:
//    1) Spectral Vector,
//    2) MFCC Vector
//    3) Energy of the Frame
//
VAD_STATE VAD::Run (FLOAT32 *in_spec_vector, FLOAT32 *in_mfcc_vector, FLOAT32 energy)
{
	// Debugging variables

	frame_counter++;
	//
	// Setting up private data memeber for efficincy and encapsulation
	//
	spec_vector       = in_spec_vector;
	mfcc_vector       = in_mfcc_vector;
	frame_energy      = energy + 64.0F; // adding floor energy because of log
	//
	// Scaled version of log2 of frame energy
	//
	log2_frame_energy = 16.0F*(log(frame_energy/64.0F)/LOG2) + 0.5F;
	//
	// Computing VAD MFCC feature
	//
	compute_VAD_mfcc_feature();
	//
	//
	//
	compute_signal_energy_stats();
	compute_VAD_SPEC_stats();
	compute_VAD_MFCC_stats();
	//compute_VAD_MFCC_Enhanced_stats();
	//
	// Individual Features Decision Logic
	//
	set_VAD_EN_state();
	set_VAD_SPEC_state();
	set_VAD_MFCC_state();

	if (fp_vadFEF) {
		fwrite(&VAD_log2_frame_energy_feature, 1, sizeof(FLOAT32), fp_vadFEF);
		fwrite(&VAD_spec_feature, 1, sizeof(FLOAT32), fp_vadFEF);
		fwrite(&VAD_mfcc_feature, 1, sizeof(FLOAT32), fp_vadFEF);
	}
	else {
		//set_VAD_MFCC_Enhs_state();
		//compute_VAD_SVM();
	}
	//
	// Global Decision Logic
	//
	set_VAD_state();

	if (monitor)
		Run_Monitor();

	return(myVAD_State_Flag);
}
//
// Computation of signal energy statistics
//
void VAD::compute_signal_energy_stats()
{
	//
	// Gradual ramp-up of the filter constant to nominal value set by
	//   LAMBDA_LTE_LOWER_E
	if (frame_counter < NB_FRAME_THRESHOLD_LTE)
		lambda_LTE = 1 - 1 / (FLOAT32) frame_counter;
	else
		lambda_LTE = LAMBDA_LTE_LOWER_E;

	//if (VAD_EN_State_Flag == VAD_ON) {
	if (VAD_State_Flag == VAD_ON) {
		// Slower update of the mean energy
		mean_log2_fm_en_VAD_ON += (1 - LAMBDA_LTE_HIGHER_E) * (log2_frame_energy - mean_log2_fm_en_VAD_ON);

      //if ((mean_log2_fm_en_VAD_ON - log2_frame_energy) > SNR_EN_THRESHOLD_VAD/5) {
      //   // Allowing update of the mean_log2_fm_en in VAD_ON State
      //   mean_log2_fm_en += (1 - LAMBDA_LTE_HIGHER_E) * (log2_frame_energy - mean_log2_fm_en);
      //}

		// mean energy not allowed to drop bellow the ENERGY_FLOOR
		if (mean_log2_fm_en_VAD_ON < ENERGY_FLOOR)
			mean_log2_fm_en_VAD_ON = ENERGY_FLOOR;
	}
	else {
		if (((log2_frame_energy - mean_log2_fm_en) < SNR_EN_THRESHOLD_UPD_LTE) ||
			(frame_counter < N_MIN_FRAMES)) {

				//if (mean_log2_fm_en > 1.5*log2_frame_energy)
				//   mean_log2_fm_en = log2_frame_energy;

				if ((log2_frame_energy < mean_log2_fm_en) ||
					(frame_counter < N_MIN_FRAMES))
					// Faster update of the mean energy
					mean_log2_fm_en += (1 - lambda_LTE) * (log2_frame_energy - mean_log2_fm_en);
				else
					// Slower update of the mean energy
					mean_log2_fm_en += (1 - LAMBDA_LTE_HIGHER_E) * (log2_frame_energy - mean_log2_fm_en);

				// mean energy not allowed to drop bellow the ENERGY_FLOOR
				if (mean_log2_fm_en < ENERGY_FLOOR)
					mean_log2_fm_en = ENERGY_FLOOR;
		}
	}
}
//
// Energy Feature Based VAD State Determination
//
void VAD::set_VAD_EN_state()
{

	VAD_log2_frame_energy_feature = (log2_frame_energy - mean_log2_fm_en);

	// Ingoring first N_MIN_FRAMES2
	if (frame_counter > N_MIN_FRAMES2) {
		// SNR VAD THRESHOLD
		if (VAD_log2_frame_energy_feature > SNR_EN_THRESHOLD_VAD) {
			VAD_EN_State_Flag = VAD_ON;
			n_speech_frames++;
		}
		else {
			//
			// Waiting MIN_SPEECH_FRAME_HANGOVER frames to change the state to VAD_EN_OFF
			//
			if (n_speech_frames > MIN_SPEECH_FRAME_HANGOVER)
				hangover = HANGOVER;

			n_speech_frames = 0;
			if (hangover != 0) {
				hangover--;
			}
			else {
				VAD_EN_State_Flag = VAD_OFF;
				mean_log2_fm_en_VAD_ON = 0.0F;
			}
		}
		//if (VAD_State_Flag == VAD_ON) {
		//Negative dB - SNR_THRESHOLD_VAD dB bellow mean VAD_ON energy
		//   if ((mean_log2_fm_en_VAD_ON - log2_frame_energy) > SNR_THRESHOLD_VAD) {
		//      VAD_EN_State_Flag = VAD_OFF;
		//      mean_log2_fm_en_VAD_ON = 0.0F;
		//   }
		//}
	}
}
//
// computation of SPECTOGRAM stats
//
void VAD::compute_VAD_SPEC_stats()
{
	FLOAT32 sum=0.0F, sum2=0.0F, tmp=0.0F;
	//
	// Computing sum & sum of squares of spectral fector features
	//
	for (UINT16 i=0 ; i<spec_vector_size ; i++) {
		tmp   = spec_vector[i];
		sum  += tmp;
		sum2 += tmp * tmp;
	}
	// Computing Mean of spec_vec_size elements of spectral vector
	mean_spec = sum/spec_vector_size;
	// Variance Computation
	var_spec  = fabs(sum2/spec_vector_size - mean_spec*mean_spec);
	var_spec  = log(var_spec+1.0);

	if (VAD_State_Flag == VAD_ON) {
		// Slower update of the mean energy
		aver_var_spec_VAD_ON += (1 - LAMBDA_LTE_HIGHER_E) * (log2_frame_energy - aver_var_spec_VAD_ON);

      //if ((aver_var_spec_VAD_ON - var_spec) > SNR_SPEC_THRESHOLD_UPD_LTE) {
      //   // Allowing update of aver_var_spec during VAD_ON State
      //   aver_var_spec += (1 - lambda_LTE/10) * (var_spec - aver_var_spec);
      //}

		// mean energy not allowed to drop bellow the ENERGY_FLOOR
		if (aver_var_spec_VAD_ON < SPEC_VAR_FLOOR)
			aver_var_spec_VAD_ON = SPEC_VAR_FLOOR;
	}
	else {
		if (((var_spec - aver_var_spec) < SNR_SPEC_THRESHOLD_UPD_LTE) ||
			(frame_counter < N_MIN_FRAMES)) {

				if (aver_var_spec > 1.5*var_spec)
					aver_var_spec = var_spec;

				if ((log2_frame_energy < aver_var_spec) ||
					(frame_counter < N_MIN_FRAMES))
					// Faster update of the mean energy
					aver_var_spec += (1 - lambda_LTE) * (var_spec - aver_var_spec);
				else
					// Slower update of the mean energy
					aver_var_spec += (1 - LAMBDA_LTE_HIGHER_E) * (var_spec - aver_var_spec);

				// mean energy not allowed to drop bellow the ENERGY_FLOOR
				if (aver_var_spec < SPEC_VAR_FLOOR)
					aver_var_spec = SPEC_VAR_FLOOR;
		}
	}
}

void VAD::compute_VAD_SVM()
{
	//svm_features[0].index = 1;
	//svm_features[0].value = VAD_log2_frame_energy_feature;
	//svm_features[1].index = 2;
	//svm_features[1].value = VAD_spec_feature;
	//svm_features[2].index = 3;
	//svm_features[2].value = VAD_mfcc_feature;
	//svm_features[3].index = -1;
	//FLOAT64 score = svm_predict(svm_model, svm_features);

	// Linear classifier: y = w . x - b;
	FLOAT64 w[3] = {-0.037877911117903, 0.032465319494065, -0.000186593873877};
	FLOAT64 b = -1.697236986015523;
	FLOAT64 score = (VAD_log2_frame_energy_feature * w[0] +
		VAD_spec_feature * w[1] +
		VAD_mfcc_feature * w[2]) - b;


	//printf("VAD Features: [%5.1f %5.1f %5.1f] Score: %5.1f\n", svm_features[0].value,
	//															svm_features[1].value,
	//															svm_features[2].value,
	//															score);
	VAD_SVM_State_Flag = score > 0 ? VAD_ON : VAD_OFF;

}
//
// SPEC Feature Based VAD
//
void VAD::set_VAD_SPEC_state()
{

	VAD_spec_feature = (var_spec - aver_var_spec);

	// Ingoring first N_MIN_FRAMES2
	if (frame_counter > N_MIN_FRAMES2) {
		// SNR VAD THRESHOLD
		if (VAD_spec_feature > SNR_SPEC_THRESHOLD_VAD) {
			VAD_SPEC_State_Flag = VAD_ON;
			n_speech_frames_spec++;
		}
		else {
			//
			// Waiting MIN_SPEECH_FRAME_HANGOVER frames to change the state to VAD_EN_OFF
			//
			if (n_speech_frames_spec > MIN_SPEECH_FRAME_HANGOVER)
				hangover_spec = HANGOVER;

			n_speech_frames_spec = 0;
			if (hangover_spec != 0) {
				hangover_spec--;
			}
			else {
				VAD_SPEC_State_Flag = VAD_OFF;
				aver_var_spec_VAD_ON = 0.0F;
			}
		}
		//if (VAD_State_Flag == VAD_ON) {
		//Negative dB - SNR_THRESHOLD_VAD dB bellow mean VAD_ON energy
		//   if ((aver_var_spec_VAD_ON - var_spec) > SNR_SPEC_THRESHOLD_VAD) {
		//      VAD_SPEC_State_Flag = VAD_OFF;
		//      aver_var_spec_VAD_ON = 0.0F;
		//   }
		//}
	}
}
//
// VAD feature Computation from MFCC's
//
void VAD::compute_VAD_mfcc_feature()
{
	//
	// 1.0*|c1|+2.0|c2|+1.0|c3|+2.0*|c4|+2.0*|c5|+1.0*|c6|
	//
	// prev_vad_mfcc_feature = vad_mfcc_feature;
	/*
	vad_mfcc_feature =   ((float) (fabs(mfcc_vector[0]) +   // c1
	fabs(mfcc_vector[1]) +   // c2
	fabs(mfcc_vector[2]) +   // c3
	fabs(mfcc_vector[3]) +   // c4
	fabs(mfcc_vector[4]) +   // c5
	fabs(mfcc_vector[5]))    // c6
	);
	partialSum  = vad_mfcc_feature;
	*/
	vad_mfcc_feature = ((float) ((mfcc_vector[0])*(mfcc_vector[0]) +   // c1
		(mfcc_vector[1])*(mfcc_vector[1]) +   // c2
		(mfcc_vector[2])*(mfcc_vector[2]) +   // c3
		(mfcc_vector[3])*(mfcc_vector[3]) +   // c4
		(mfcc_vector[4])*(mfcc_vector[4]) +   // c5
		(mfcc_vector[5])*(mfcc_vector[5]))    // c6
		);
}
//
// computation of VAD feature stats
//
void VAD::compute_VAD_MFCC_stats()
{

	if (VAD_State_Flag == VAD_ON) {
		// Slower update of the mean energy
		mean_vad_mfcc_feature_VAD_ON += (1 - LAMBDA_LTE_HIGHER_E) * (vad_mfcc_feature - mean_vad_mfcc_feature_VAD_ON);

		// mean energy not allowed to drop bellow the MFCC_FLOOR
		if (mean_vad_mfcc_feature_VAD_ON < MFCC_FLOOR)
			mean_vad_mfcc_feature_VAD_ON = MFCC_FLOOR;
	}
	else {
		if (((vad_mfcc_feature - mean_vad_mfcc_feature) < SNR_MFCC_THRESHOLD_UPD_LTE) ||
			(frame_counter < N_MIN_FRAMES)) {

				if ((vad_mfcc_feature < mean_vad_mfcc_feature) ||
					(frame_counter < N_MIN_FRAMES))
					// Faster update of the mean energy
					mean_vad_mfcc_feature += (1 - lambda_LTE) * (vad_mfcc_feature - mean_vad_mfcc_feature);
				else
					// Slower update of the mean energy
					mean_vad_mfcc_feature += (1 - LAMBDA_LTE_HIGHER_E) * (vad_mfcc_feature - mean_vad_mfcc_feature);

				// mean energy not allowed to drop bellow the ENERGY_FLOOR
				if (mean_vad_mfcc_feature < MFCC_FLOOR)
					mean_vad_mfcc_feature = MFCC_FLOOR;
		}
	}
}
//
// MFCC Feature Based VAD
//
void VAD::set_VAD_MFCC_state() {

	VAD_mfcc_feature = (vad_mfcc_feature - mean_vad_mfcc_feature);

	// Ingoring first N_MIN_FRAMES2
	if (frame_counter > N_MIN_FRAMES2) {
		// SNR VAD THRESHOLD
		if (VAD_mfcc_feature > SNR_MFCC_THRESHOLD_VAD) {
			VAD_MFCC_State_Flag = VAD_ON;
			n_speech_frames_mfcc++;
		}
		else {
			//
			// Waiting MIN_SPEECH_FRAME_HANGOVER frames to change the state to VAD_EN_OFF
			//
			if (n_speech_frames_mfcc > MIN_SPEECH_FRAME_HANGOVER)
				hangover_mfcc = HANGOVER;

			n_speech_frames_mfcc = 0;
			if (hangover_mfcc != 0) {
				hangover_mfcc--;
			}
			else {
				VAD_MFCC_State_Flag  = VAD_OFF;
				mean_vad_mfcc_feature_VAD_ON = 0.0F;
			}
		}
	}
}
//
// computation of VAD feature stats
//
void VAD::compute_VAD_MFCC_Enhanced_stats()
{
	FLOAT32 tmp = 0.0F;
	//
	// Computing sum & sum of squares of spectral fector features
	//
	/*
	for (UINT16 i=0 ; i<mfcc_vector_size ; i++) {
	tmp   = mfcc_vector[i];
	sum  += tmp;
	sum2 += tmp * tmp;
	}
	*/
	// Computing Mean of the spec_vec_size elements of spectral vector
	mean_mfcc = partialSum/mfcc_vector_size;
	tmp = mean_mfcc;
	if (frame_counter>1) {
		mean_mfcc = 0.2F*mean_mfcc + 0.8F*prev_mean_mfcc;
	}
	prev_mean_mfcc = tmp;

	if (VAD_MFCC_Enhs_State_Flag == VAD_ON) {
		var_mfcc_VAD_ON = var_mfcc;
		// Variance Computation
		var_mfcc_VAD_ON  = partialSum2/mfcc_vector_size - mean_mfcc*mean_mfcc;
		tmp = var_mfcc_VAD_ON;
		if (frame_counter>1) {
			mean_var_mfcc = 0.2F*var_mfcc_VAD_ON + 0.8F*prev_mean_var_mfcc;
			var_var_mfcc = (var_mfcc_VAD_ON - mean_var_mfcc)*(var_mfcc_VAD_ON - mean_var_mfcc);
		}
		prev_mean_var_mfcc = tmp;
	} else {
		// Variance Computation
		var_mfcc  = partialSum2/mfcc_vector_size - mean_mfcc*mean_mfcc;
		tmp = var_mfcc;
		if (frame_counter>1) {
			mean_var_mfcc = 0.2F*var_mfcc + 0.8F*prev_mean_var_mfcc;
			var_var_mfcc = (var_mfcc - mean_var_mfcc)*(var_mfcc - mean_var_mfcc);
		}
		prev_mean_var_mfcc = tmp;
	}
	// Applying special handling for first 15 frames
	if (frame_counter < VAD_OFF_MIN_COUNT) {
		aver_var_mfcc = max (aver_var_mfcc, var_var_mfcc);
	}
	// Computing Moving Average of the spectral variance estimate
	if ((var_var_mfcc < 1.25F*aver_var_mfcc) &&
		(var_var_mfcc > 0.85F*aver_var_mfcc)) {
			aver_var_mfcc = (0.8F*aver_var_mfcc + 0.2F*var_var_mfcc);
	}

	if (frame_counter < 100) {
		if (var_var_mfcc <= 0.15F*aver_var_mfcc)  {
			aver_var_mfcc = (0.977F*aver_var_mfcc + 0.003F*var_var_mfcc);
		}
	}

}
//
// MFCC Enhanced Feature Based VAD
//
void VAD::set_VAD_MFCC_Enhs_state() {
	if (var_var_mfcc > 1.65F*aver_var_mfcc) {
		VAD_MFCC_Enhs_State_Flag = VAD_ON;
	}
	else if (var_mfcc_VAD_ON <= 0.859F*var_mfcc) {
		VAD_MFCC_Enhs_State_Flag = VAD_OFF;
	}
}
//
// Global Decision Logic
//
void VAD::set_VAD_state()
{
	UINT16 countVAD_ON = 0;
	bool bVadOn;

	if (frame_counter <= n_init_frames)
		return;

#if 0

	// Original VAD decision logic (at least 2 flags are on)
	if (VAD_EN_State_Flag   == VAD_ON) countVAD_ON++;
	if (VAD_SPEC_State_Flag == VAD_ON) countVAD_ON++;
	if (VAD_MFCC_State_Flag == VAD_ON) countVAD_ON++;
	bVadOn = countVAD_ON >= 2 ? true : false;

#else

	// New VAD decision logic (linear SVM based)
	FLOAT64 w[3] = {-0.037877911117903, 0.032465319494065, -0.000186593873877};
	FLOAT64 b = -1.697236986015523;
	FLOAT64 score = (VAD_log2_frame_energy_feature * w[0] + VAD_spec_feature * w[1] + VAD_mfcc_feature * w[2]) - b;

	//printf("VAD features: %5.1f %5.1f %5.1f SCORE: %3.1f\n", VAD_log2_frame_energy_feature, VAD_spec_feature, VAD_mfcc_feature, score);
	bVadOn = score < 0 ? true : false;

#endif

	if(bVadOn) // Current frame is speech
	{
		//if(VAD_ON_Count == 0)
		//   printf("First VAD ON at frame %d\n", frame_counter);

		VAD_ON_Count++;

		if (VAD_ON_Count >= VAD_ON_min_count)
		{
			VAD_State_Flag = VAD_ON; // Internal VAD State Change after VAD_ON_min_count Frames
			VAD_OFF_Count = 0;

			// Adjusting reporting VAD_ON flag to align with VAD_Delay-VAD_ON_lead_count time
			if (VAD_ON_Count + VAD_ON_lead_count >= VAD_Delay)
			{
				//if(myVAD_State_Flag == VAD_OFF)
				//	printf("VAD ON reported at frame %d\n", frame_counter);

				myVAD_State_Flag = VAD_ON;
			}
		}
	}
	else // Current frame is non-speech
	{
		//if(VAD_OFF_Count == 0)
		//   printf("First VAD OFF at frame %d\n", frame_counter);

		VAD_OFF_Count++;
		if (VAD_OFF_Count >= VAD_OFF_min_count)
		{
			VAD_State_Flag = VAD_OFF; // Internal VAD State Change after VAD_ON_min_count Frames
			VAD_ON_Count = 0;

			// Adjusting reporting VAD_OFF flag to align with VAD_Delay+VAD_OFF_trail_count time
			if (VAD_OFF_Count - VAD_OFF_trail_count >= VAD_Delay)
			{
				//if(myVAD_State_Flag == VAD_ON)
				//	printf("VAD OFF reported at frame %d\n", frame_counter);

				myVAD_State_Flag = VAD_OFF;
			}
		}
	}
}
//
// Return Log2 Frame Energy
//
FLOAT32 VAD::getFrameEnergy() {
	return(log2_frame_energy);
}
//
// Return VAD States
//
VAD_States *VAD::getVAD_States() {

	VAD_states.VAD_State_Flag           = myVAD_State_Flag;
	VAD_states.VAD_EN_State_Flag        = VAD_EN_State_Flag;
	VAD_states.VAD_SPEC_State_Flag      = VAD_SPEC_State_Flag;
	VAD_states.VAD_MFCC_State_Flag      = VAD_MFCC_State_Flag;
	VAD_states.VAD_MFCC_Enhs_State_Flag = VAD_MFCC_Enhs_State_Flag;

	return(&VAD_states);
}

/*******************************  End of File  *******************************/

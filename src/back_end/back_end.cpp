/******************************************************************************

Copyright (C) 2006-2015 VoiceKey Inc.
Cocoa Beach, Florida 32931
All rights reserved

Confidential and Proprietary


Module: back_end.cpp
Author: Veton Kepuska
Created: October 28, 2006
Updated:


Description: Back_End module of the e-WUW Speech Recognizor.

$Log:$

*******************************************************************************

Proprietary and Confidential. All rights reserved.
VoiceKey Inc., Cocoa Beach, Florida 32931.

******************************************************************************/

/**********************************  Notes  **********************************


/******************************  Include Files  ******************************/

#include <limits>
#include <stdlib.h>
// #include <windows.h>

#include "back_end.h"
#include "svm/svm.cpp"
#include "hmm/hmm_network.cpp"
#include "hmm/hmm_util.cpp"


/*********************************  Defines  *********************************/

/****************************  Function Members  *****************************/

BackEnd::BackEnd()
{
	/* We are phasing out default constructors -
	a function should always be initialized with its "config" constructor */
}

BackEnd::BackEnd(CfgBackEnd* cfgBackEnd)
{
	// Initialize parameters
	mb_monitor            = cfgBackEnd->monitor;
	mu32_FrameCounter     = 0;
	mu16_WordFrameCounter = 0;
	mu16_NumVADCounter    = 0;
	// ms_dtw_m_scores       = NULL;
	mp_cfgBackEnd         = cfgBackEnd;
	me_back_end_type      = cfgBackEnd->be_type;    // Back-End Type
	me_scoring            = cfgBackEnd->be_scoring; // Back-End Scoring
	mu16_frame_rate       = cfgBackEnd->cfgFrontEnd->frame_rate;
	mp_cfgTrain           = (CfgTrain *) cfgBackEnd->cfgTrain;
	mb_train_flag         = mp_cfgTrain->train;
	mb_genFEF_flag        = cfgBackEnd->cfgGenFEF->genFEF;
	mb_genAllFEF_flag     = cfgBackEnd->cfgGenFEF->genAllFEF;
	memset(mw_fef_filename, 0, _MAX_PATH*sizeof(char));
	memset(mw_vad_filename, 0, _MAX_PATH*sizeof(char));

	if (mb_genFEF_flag) {
		string _mw_vad_filename(mp_cfgBackEnd->basefilename);
		size_t found = _mw_vad_filename.find_last_of(".");
		_mw_vad_filename.replace(found,1,"_");

		sprintf(mw_vad_filename, "%s/%s_vad.dat",
			mp_cfgBackEnd->out_data_dir, _mw_vad_filename.c_str());
	
		vad_file = new WUW_IO(mw_vad_filename, false, "w");
	}

	if (mb_genAllFEF_flag) {
		SCHAR header[FEF_HEADER_SIZE];
		memset(header, 0, FEF_HEADER_SIZE);

		string _mw_ALLfef_filename(mp_cfgBackEnd->basefilename);
		size_t found = _mw_ALLfef_filename.find_last_of(".");
		_mw_ALLfef_filename.replace(found,1,"_");

		std::filesystem::create_directories(mp_cfgBackEnd->out_data_dir);

		sprintf(mw_ALLfef_filename, "%s/%s.FEF", mp_cfgBackEnd->out_data_dir, _mw_ALLfef_filename.c_str());
		FEF_file = new WUW_IO(mw_ALLfef_filename, true, "wb");
		FEF_file->SetFEFInfo();
		FEF_file->Write_FEF_Header("VK_FEF_FILE", header);
	}

	//
	// Must be configurable parameters based on statitical analysis of actual
	// WUW sample features and selected WUW.
	//
	min_WUW_length        = 0;   // Unit is in Number of Frames
	max_WUW_length        = 300; // Unit is in Number of Frames

	mb_vad_on_flag        = false;
	mb_vad_off_flag       = false;

	BAD_SCORE             = HUGE_VAL;//numeric_limits<FLOAT32>::max();

	// Initialize modules
	switch (me_scoring)
	{
		// case DTW_m:
		// {
		// 	pm_DTW = new DTW_Score (&cfgBackEnd->cfgDTW);
		// 	break;
		// }
		case HMM_CD:
		{
			memset(hmm_model, 0, 3 * 3 * sizeof(HmmModel*));
			memset(hmm_net, 0, 3 * 3 * sizeof(HmmNetwork*));
			numModels = 0;

			// TODO add deallocation of memory
			// See HMM_FEF_TYPE
			for(int i = 0; i < 3; i++)
			{
				if(cfgBackEnd->cfgHmm.model_filename[i])
				{
					numModels++;

					// Load HMM
					// i = 0 forward 
					// i = 1 reversed
					// i = 2 expanded
					for(int j = 0; j < 3; j++)
						hmm_model[i][j] = HmmUtil::LoadBinaryHmm(cfgBackEnd->cfgHmm.model_filename[i]);

					// Modify topology
					HmmUtil::ReverseModel(hmm_model[i][1]);
					HmmUtil::ExpandModel(hmm_model[i][2]);

					// Create network
					for(int j = 0; j < 3; j++)
					{
						hmm_net[i][j] = new HmmNetwork(0);
						hmm_net[i][j]->AddModel(hmm_model[i][j]);
					}

					// TODO verify # of dimensions is correct
					hmm_features[i] = new FLOAT64[hmm_model[i][0]->nDim];
				}
			}

			char svmmodel[_MAX_PATH];
			sprintf(svmmodel, "%s", cfgBackEnd->svm_file);

			// SVM
			svm_model = svm_load_model(svmmodel);
			if(!svm_model)
				printf("Failed to load SVM model\n");

			svm_features = new svm_node[numModels * 3 + 1];
			for(int i = 0; i < numModels * 3; i++)
				svm_features[i].index = i+1;
			svm_features[numModels * 3].index = -1;

         threshold  = mp_cfgBackEnd->cfgScoring.threshold;
         threshold2 = mp_cfgBackEnd->cfgScoring.threshold2;

         fprintf(stderr, "\nThresholds: T1=%f\tT2=%f\n", threshold, threshold2);

		}
	}

	// Initialize filepointers
   //STARTsnd  = wcsdup(L"sounds\\Star Trek TNG Beep.wav");
   if (cfgBackEnd->liveinput) {

   STARTsnd  = strdup("sounds\\Star Trek TNG Start Show.wav");
   RECsnd    = strdup("sounds\\Star Trek TNG Beep.wav");
   CMDsnd    = strdup("sounds\\Computer sound 44.wav");
   EXITsnd   = strdup("sounds\\Star Trek TNG It is a good day to die.wav");
   }

   if (cfgBackEnd->liveinput)
    //   PlaySound(STARTsnd, NULL, SND_ASYNC);

	// Initialize loggers
	if (mb_monitor)
		Open_MonitorFiles(mp_cfgBackEnd);

}
//
BackEnd::~BackEnd ()
{
//    if (mp_cfgBackEnd->liveinput)
//       PlaySound(EXITsnd, NULL, SND_ASYNC);

   if (mb_genAllFEF_flag) {
		// Set File Header Information
		FEF_file->SetFEFInfo(mu32_FrameCounter, num_fe_mfcc_features, num_fe_enh_mfcc_features, mu16_frame_rate);
		FEF_file->Write_FEF_Header("VK_FEF_FILE");

		delete FEF_file;
	}

	// Release modules
	// if (me_scoring)
	// 	delete pm_DTW;

	if (mb_genFEF_flag)
		delete vad_file;

	if (mb_monitor)
		End_Monitor();

	// Release buffers
	ClearSegment();

   free(STARTsnd);
   free(RECsnd);
   free(CMDsnd);
   free(EXITsnd);
}
//
// End-Of-Stream Function
//
// BE_SCORE *BackEnd::Eos()
// {
// 	return(NULL);
// }
//
void BackEnd::Open_MonitorFiles(CfgBackEnd* cfgBackEnd)
{
	// Build file path
	char filename[_MAX_PATH];
	sprintf(filename, "%s\\%s", cfgBackEnd->out_data_dir, cfgBackEnd->basefilename);
	//wcscpy_s(filename, _MAX_PATH, cfgFrontEnd->out_data_dir);
	char* ext = filename + strlen(filename);

	strcpy(ext, ".BEvad");
	m_vad_logger   = new WuwLogger(filename, sizeof(SINT32), 1); // Opening in Binary Mode  - Not Text Mode

}
//
void BackEnd::End_Monitor()
{
	delete m_vad_logger;
}
//
// Monitoring Function
//
void BackEnd::Run_Monitor()
{
	SINT32 vad = 0;

	if (me_fe_vad_state == VAD_ON) {
		vad = 1;
	}
	else {
		vad = 0;
	}

	m_vad_logger->WriteBinary(&vad); // Binary Info

}
//
// Only BackEnd Object contains a buffer of FE features stored for second pass processing
//
void BackEnd::StoreFeatures()
{
	feature_vector = new FLOAT32 [num_fe_mfcc_features];
	memcpy(feature_vector, pf32_fe_mfcc_features_buffer, sizeof(FLOAT32)*num_fe_mfcc_features);
	m_SegFeatureVecs.fe_mfcc_features_buffer.push_back(feature_vector);

	feature_vector = new FLOAT32 [num_fe_mfcc_features];
	memcpy(feature_vector, pf32_fe_lpc_mfcc_features_buffer, sizeof(FLOAT32)*num_fe_mfcc_features);
	m_SegFeatureVecs.fe_lpc_mfcc_features_buffer.push_back(feature_vector);

	feature_vector = new FLOAT32 [num_fe_enh_mfcc_features];
	memcpy(feature_vector, pf32_fe_enh_mfcc_features_buffer, sizeof(FLOAT32)*num_fe_enh_mfcc_features);
	m_SegFeatureVecs.fe_enh_mfcc_features_buffer.push_back(feature_vector);
}
//
// Clear Segment Features
//
void BackEnd::ClearSegment()
{
	mu16_WordFrameCounter = 0;
	m_SegFeatureVecs.num_feature_vectors = 0;
	for(int i = 0; i < m_SegFeatureVecs.fe_mfcc_features_buffer.size(); i++)
	{
		delete m_SegFeatureVecs.fe_mfcc_features_buffer[i];
		delete m_SegFeatureVecs.fe_lpc_mfcc_features_buffer[i];
		delete m_SegFeatureVecs.fe_enh_mfcc_features_buffer[i];
	}

	m_SegFeatureVecs.fe_mfcc_features_buffer.clear();
	m_SegFeatureVecs.fe_lpc_mfcc_features_buffer.clear();
	m_SegFeatureVecs.fe_enh_mfcc_features_buffer.clear();
}
//
// Main Processing Function
//
//BE_SCORE *BackEnd::Run(FE_FEATURES * in_features)
void *BackEnd::Run(FE_FEATURES * in_features)
{
	if(!in_features)
		return NULL;

	mp_fe_feature_vecs               = in_features;
	pf32_fe_mfcc_features_buffer     = mp_fe_feature_vecs->fe_mfcc_features_buffer;
	pf32_fe_lpc_mfcc_features_buffer = mp_fe_feature_vecs->fe_lpc_mfcc_features_buffer;
	pf32_fe_enh_mfcc_features_buffer = mp_fe_feature_vecs->fe_enh_features_buffer;
	num_fe_mfcc_features             = 3*mp_fe_feature_vecs->num_mfcc_features;     // Static + GradientI + GradientII
	num_fe_enh_mfcc_features         = 3*mp_fe_feature_vecs->num_enh_features;      // Static + GradientI + GradientII
	me_fe_vad_state                  = mp_fe_feature_vecs->vad_state;
	// ms_dtw_m_scores                  = NULL;

	// if (mb_genAllFEF_flag) {
	// 	FEF_file->WriteBinary(pf32_fe_mfcc_features_buffer, sizeof(FLOAT32), num_fe_mfcc_features-3);
	// 	FEF_file->WriteBinary(pf32_fe_lpc_mfcc_features_buffer, sizeof(FLOAT32), num_fe_mfcc_features-3);
	// 	FEF_file->WriteBinary(pf32_fe_enh_mfcc_features_buffer, sizeof(FLOAT32), num_fe_enh_mfcc_features-3);
	// }

	mu32_FrameCounter++;

	switch (me_fe_vad_state)
	{
		case VAD_ON:
		{
			// printf("VAD ON \n");
			mb_vad_on_flag = true;
			mb_vad_off_flag = false;

			if (mu16_WordFrameCounter == 0)
			{
				if (mb_genFEF_flag)
					vad_file->Printf(L"[%06d] %d\t", mu16_NumVADCounter, 1000*mu32_FrameCounter/(mp_cfgBackEnd->cfgFrontEnd->frame_rate));

			}


			// Storing Feature Vector into SEGMENT_FEATURES structure
			StoreFeatures();

			// Scoring
			switch(me_scoring)
			{
				// case DTW_m:
				// {
				// 	if ((me_back_end_type == DTW_m) || (me_back_end_type == ALL))
				// 		pm_DTW->Run(in_features, mu32_FrameCounter);
				// 	break;
				// }
				case HMM_CD:
				{
					// Set up new HMM scoring sequence
					if(mu16_WordFrameCounter == 0)
					{
						// See HMM_FEF_TYPE
						for(int i = 0; i < 3; i++)
						{
							if(hmm_net[i][0])
							{
								for(int j = 0; j < 3; j++)
									hmm_net[i][j]->NewRecoSeq();
							}
						}
					}

					// Convert features to float64
					if(hmm_net[HMM_FEF_ENH][0])
					{
						for(int i = 0; i < 13; i++)
							hmm_features[HMM_FEF_ENH][i] = (FLOAT64)pf32_fe_enh_mfcc_features_buffer[i];
						for(int i = 13; i < 26; i++)
							hmm_features[HMM_FEF_ENH][i] = (FLOAT64)pf32_fe_enh_mfcc_features_buffer[i+1];
						for(int i = 26; i < 39; i++)
							hmm_features[HMM_FEF_ENH][i] = (FLOAT64)pf32_fe_enh_mfcc_features_buffer[i+2];
					}

					if(hmm_net[HMM_FEF_MFC][0])
					{
						for(int i = 0; i < 13; i++)
							hmm_features[HMM_FEF_MFC][i] = (FLOAT64)pf32_fe_mfcc_features_buffer[i];
						for(int i = 13; i < 26; i++)
							hmm_features[HMM_FEF_MFC][i] = (FLOAT64)pf32_fe_mfcc_features_buffer[i+1];
						for(int i = 26; i < 39; i++)
							hmm_features[HMM_FEF_MFC][i] = (FLOAT64)pf32_fe_mfcc_features_buffer[i+2];
					}

					if(hmm_net[HMM_FEF_LPC][0])
					{
						for(int i = 0; i < 13; i++)
							hmm_features[HMM_FEF_LPC][i] = (FLOAT64)pf32_fe_lpc_mfcc_features_buffer[i];
						for(int i = 13; i < 26; i++)
							hmm_features[HMM_FEF_LPC][i] = (FLOAT64)pf32_fe_lpc_mfcc_features_buffer[i+1];
						for(int i = 26; i < 39; i++)
							hmm_features[HMM_FEF_LPC][i] = (FLOAT64)pf32_fe_lpc_mfcc_features_buffer[i+2];
					}

					// Send data to HMM recognizer
					for(int i = 0; i < 3; i++)
					{
						if(hmm_net[i][0])
						{
							for(int j = 0; j < 3; j++)
								hmm_net[i][j]->AddRecoObs(hmm_features[i]);
						}
					}

					break;
				}
			}

			mu16_WordFrameCounter++;

			break;
		}
		case VAD_OFF:
		{
			// printf("VAD OFF \n");
			mb_vad_off_flag = true;
			if (mb_vad_on_flag)
			{
				mb_vad_on_flag = false;
				// Not Storing last frame
				// mu16_WordFrameCounter++;
				// Storing Feature Vector into SEGMENT_FEATURES structure
				// StoreFeatures();
				// Setting up the SegFeatureVecs data structure
				m_SegFeatureVecs.num_mfcc_features   = num_fe_mfcc_features;     // Static + GradientI + GradientII
				m_SegFeatureVecs.num_enh_features    = num_fe_enh_mfcc_features; // Static + GradientI + GradientII
				m_SegFeatureVecs.num_feature_vectors = mu16_WordFrameCounter;

				// Final DTW Matching
				switch(me_scoring)
				{
					// case DTW_m:
					// {
					// 	if ((me_back_end_type == DTW_m) || (me_back_end_type == ALL))
					// 		ms_dtw_m_scores = pm_DTW->get_DTW_Scores(&m_SegFeatureVecs);
					// 	break;
					// }
					case HMM_CD:
					{
						int c = 0;
						// See HMM_FEF_TYPE
						for(int i = 0; i < 3; i++)
						{
							if(hmm_net[i][0])
							{
								for(int j = 0; j < 3; j++)
								{
									// [ HMM_FEF_ENH Forward, HMM_FEF_ENH Reversed, HMM_FEF_ENH Enhanced,
									//   HMM_FEF_MFC Forward, HMM_FEF_MFC Reversed, HMM_FEF_MFC Enhanced,
									//   HMM_FEF_LPC Forward, HMM_FEF_LPC Reversed, HMM_FEF_LPC Enhanced ]
									svm_features[c].value = hmm_net[i][j]->GetRecoScore();
									printf("svm_features [%i]: [%5.1f]\n", c, svm_features[c].value);
									c++;
								}
							}
						}

						FLOAT64 score;
						svm_predict_values(svm_model, svm_features, &score);
						 c = 0;
						for(int i = 0; i < 3; i++)
						{
							if(hmm_net[i][0])
							{
                        // printf("[%4d frames] HMM: [%6.1f, %6.1f, %6.1f] SVM SCORE: [%5.1f]\n",
                        //    mu16_WordFrameCounter,
                        //    svm_features[c].value,
                        //    svm_features[c+1].value,
                        //    svm_features[c+2].value,
                        //    score);
						// 		c+= 3;
						// 	}
						// }

						 printf("[%4d frames] SVM SCORE: [%5.1f]\n",
                           mu16_WordFrameCounter,
                           score);
								c+= 3;
							}
						}
						//FLOAT64 treshold = -0.8;
						//FLOAT64 treshold = mp_cfgBackEnd->cfgScoring.threshold;

						if(score > threshold)
                  {
                     //MessageBeep(0);
                    //  PlaySound(CMDsnd, NULL, SND_ASYNC);
							printf("***WUW DETECTED***\n");
                  }
                  else if (score > threshold2)
                  {
                    //  PlaySound(RECsnd, NULL, SND_ASYNC);
							printf("???WUW MAY BE DETECTED???\n");
                  }
					}
				}
				// Perform SVM Scoring

				// Here need to add matching conditions if the model was used for
				// scoring the features to be used for bulding of a new model
				//if ((ms_dtw_m_scores->for_matchII < F_DTWII_MATCH) &&
				//   (ms_dtw_m_scores->revM_matchII > R_DTWII_MATCH)) {
				//}

				
				// Print ascii fef
				if(false) {
					string _mw_fef_filename(mp_cfgBackEnd->basefilename);
						size_t found = _mw_fef_filename.find_last_of(".");
						_mw_fef_filename.replace(found,1,"_");
					sprintf(mw_fef_filename, "%s/%s_%02d_ascii.fef",
					  mp_cfgBackEnd->out_data_dir, _mw_fef_filename.c_str(), mu16_NumVADCounter++);
					
					WUW_IO fef_printseg(mw_fef_filename, sizeof(ST_FLOAT32), m_SegFeatureVecs.num_feature_vectors, "w");
					
					for (int i=0; i<m_SegFeatureVecs.num_feature_vectors; i++) {
					  fef_printseg.Printf(L"[%3d][%3d] mfcc ", mu32_FrameCounter, i);
					  for (int j=0; j<m_SegFeatureVecs.num_mfcc_features; j++) {
					     fef_printseg.Printf(L"%u ", m_SegFeatureVecs.fe_mfcc_features_buffer[i][j]);
					  }
					  fef_printseg.Printf(L"\n");
					   fef_printseg.Printf(L"[%3d][%3d] lpc  ", mu32_FrameCounter, i);
					  for (int j=0; j<m_SegFeatureVecs.num_mfcc_features; j++) {
					     fef_printseg.Printf(L"%u ", m_SegFeatureVecs.fe_lpc_mfcc_features_buffer[i][j]);
					  }
					  fef_printseg.Printf(L"\n");
					  fef_printseg.Printf(L"[%3d][%3d] enh  ", mu32_FrameCounter, i);
					  for (int j=0; j<m_SegFeatureVecs.num_enh_features; j++) {
					     fef_printseg.Printf(L"%u ", m_SegFeatureVecs.fe_enh_mfcc_features_buffer[i][j]);
					  }
					  fef_printseg.Printf(L"\n");
					}
				}

				if ((mb_train_flag) || (mb_genFEF_flag))
				{
					//if ((mu16_WordFrameCounter > min_WUW_length) &&
					//    (mu16_WordFrameCounter < max_WUW_length)) {

					if (mb_train_flag) {
						string _mw_fef_filename(mp_cfgTrain->basefilename);
						size_t found = _mw_fef_filename.find_last_of(".");
						_mw_fef_filename.replace(found,1,"_");

						sprintf(mw_fef_filename, "%s/%s_%05d.fef",
							mp_cfgTrain->out_data_dir, _mw_fef_filename.c_str(), mu16_NumVADCounter);
					}
					if (mb_genFEF_flag) {
						string _mw_fef_filename(mp_cfgBackEnd->basefilename);
						size_t found = _mw_fef_filename.find_last_of(".");
						_mw_fef_filename.replace(found,1,"_");

						sprintf(mw_fef_filename, "%s/%s_%05d.fef",
							mp_cfgBackEnd->out_data_dir, _mw_fef_filename.c_str(), mu16_NumVADCounter);
					}

					WUW_IO fef_segment(mw_fef_filename, sizeof(ST_FLOAT32), m_SegFeatureVecs.num_feature_vectors, "wb");
					m_SegFeatureVecs.frame_rate = mu16_frame_rate;

					fef_segment.Write_FEF(&m_SegFeatureVecs);

					vad_file->Printf(L"%d\n", 1000*mu32_FrameCounter/(mp_cfgBackEnd->cfgFrontEnd->frame_rate));
				
					// // Print ascii fef
					// string _mw_fef_filename(mp_cfgBackEnd->basefilename);
					// 	size_t found = _mw_fef_filename.find_last_of(".");
					// 	_mw_fef_filename.replace(found,1,"_");
					// sprintf(mw_fef_filename, "%s/%s_%02d_ascii.fef",
					//   mp_cfgBackEnd->out_data_dir, _mw_fef_filename.c_str(), mu16_NumVADCounter++);
					
					// WUW_IO fef_printseg(mw_fef_filename, sizeof(ST_FLOAT32), m_SegFeatureVecs.num_feature_vectors, "w");
					
					// for (int i=0; i<m_SegFeatureVecs.num_feature_vectors; i++) {
					//   fef_printseg.Printf(L"[%3d][%3d] mfcc", mu32_FrameCounter, i);
					//   for (int j=0; j<m_SegFeatureVecs.num_mfcc_features; j++) {
					//      fef_printseg.Printf(L"%e ", m_SegFeatureVecs.fe_mfcc_features_buffer[i][j]);
					//   }
					//   fef_printseg.Printf(L"\n");
					//    fef_printseg.Printf(L"[%3d][%3d] lpc ", mu32_FrameCounter, i);
					//   for (int j=0; j<m_SegFeatureVecs.num_mfcc_features; j++) {
					//      fef_printseg.Printf(L"%e ", m_SegFeatureVecs.fe_lpc_mfcc_features_buffer[i][j]);
					//   }
					//   fef_printseg.Printf(L"\n");
					//   fef_printseg.Printf(L"[%3d][%3d] enh ", mu32_FrameCounter, i);
					//   for (int j=0; j<m_SegFeatureVecs.num_enh_features; j++) {
					//      fef_printseg.Printf(L"%e ", m_SegFeatureVecs.fe_enh_mfcc_features_buffer[i][j]);
					//   }
					//   fef_printseg.Printf(L"\n");
					// }
					ClearSegment();
					//}
					mu16_NumVADCounter++;
				}
            					mu16_WordFrameCounter = 0;

			}

			break;

		}
	}

	if (mb_monitor)
		Run_Monitor();

}
//
//
//
void BackEnd::Reset()
{
}
//
//
//
void BackEnd::Init(CfgBackEnd* cfgBackEnd)
{
}

/*******************************  End of File  *******************************/

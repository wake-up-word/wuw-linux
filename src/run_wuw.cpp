/******************************************************************************

                  Copyright (C) 2006-2015 VoiceKey Inc.
                      Cocoa Beach, Florida 32931
                         All rights reserved

                    Confidential and Proprietary

     Module: main.cpp
     Author: Veton Kepuska
    Created: May 31, 2006
    Updated:

Description: A wrapper file the invokes wuw recognizor.

$Log:$

*******************************************************************************

Notes:

******************************************************************************/

/******************************  Include Files  ******************************/

#include <iostream>
//#include <sapi.h>
//#include <sperror.h>

#include "common/wuw_config.h"
#include "process_in_samples.h"
#include "audio_stream.h"
#include "front_end/front_end.h"
#include "back_end/back_end.h"

using namespace std;

#include <iostream>


//-----------------------------------------------------------------------------
// Name: printInfo
// Desc: prints command line instructions
//-----------------------------------------------------------------------------
void printInfo(char* exename)
{
   std::cerr << "Usage: " << exename << endl
      << "\t Options:" << endl
      << "\t\t<filename.conf>" << endl
      << "\t or" << endl
      << "\t\t -s \t sample rate     <[8]|16> in kHz" << endl
      << "\t\t -f \t sample type     <[ulaw], alaw, short, nist>" << endl
      << "\t\t -h \t header size     <[1024]>" << endl
      << "\t\t -i \t input           <d:/path/filename.ulaw>" << endl
      << "\t\t -d \t output data dir <o:/path or ../path>" << endl
      << "\t\t -o \t FE Features dir <o:/path or ../path" << endl
      << "\t\t -G \t generate FEF    <true, [false]> binary switch - if used is set to \"true\"" << endl
      << "\t\t -e \t ENH features    <o:/path/filename.enh or ../path/filename.enh" << endl
      << "\t\t -M \t model files     <m:/path/WUWmodel1.dtw m:/path/WUWmodel2.dtw ..." << endl
      << "\t\t -m \t monitor         <true, [false]> binary switch - if used is set to \"true\"" << endl;
}

void run_e_wuw (WuwConfig *configInfo)
{
   char drive[_MAX_DRIVE];
   char dir[_MAX_DIR];
   char path[_MAX_PATH];
   char fname[_MAX_FNAME];
   char ext[_MAX_EXT];
   char filename[_MAX_PATH];

   FILE*   fp_input;

//    HRESULT audioGainRes;
   //ULONG  pLevel;
   //FLOAT32 DeltaGain = 0.0F;
   //bool    AGC = false;
   //ISpAudio iSpAudio;

   //FILE* fp_ulaw;
	//
   // Need to have a function call to hide this complexity of setting cfg structures
   //
   CfgInput*    cfgInput;
   CfgFrontEnd* cfgFrontEnd;
   CfgBackEnd*  cfgBackEnd;
   CfgTrain*    cfgTrain;
   CfgGenFEF*   cfgGenFEF;

   cfgInput    = configInfo->GetInputConfig();
   cfgFrontEnd = configInfo->GetFrontEndConfig();
   cfgBackEnd  = configInfo->GetBackEndConfig();
   cfgTrain    = configInfo->GetTrainConfig();
   cfgGenFEF   = configInfo->GetGenFEFConfig();
   //
   // Setting cfgBackEnd Pointer to cfgFrontEnd
   //
   cfgBackEnd->cfgFrontEnd  = cfgFrontEnd;
   cfgBackEnd->cfgGenFEF    = cfgGenFEF;
   cfgGenFEF->cfgFrontEnd   = cfgFrontEnd;
   cfgTrain->cfgFrontEnd    = cfgFrontEnd;
   cfgTrain->cfgBackEnd     = cfgBackEnd;
   cfgBackEnd->be_scoring   = HMM_CD;
   //
   // Object Pointers
   //
   ProcessInputSamples  *procInSamples;
   FrontEnd             *frontEnd;
   BackEnd              *backEnd = NULL;

	int          count           = 0;
	char     *wavfile         = cfgInput->filename;
	SINT32       bufferSize      = cfgInput->bufferSize;
	BYTE        *samples_buffer;
	FLOAT32     *fsamples;
	FE_FEATURES *features;
	// BE_SCORE    *score;
	//ms_DTW_Score  *score = NULL;
    printf("cfgInput->filename %s\n", cfgInput->filename);

   if (cfgInput->filename[0] != NULL) {
      // Parsing output feature file into full path, basefilename and extension
    //   int err = _wsplitpath_s(cfgInput->filename, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);

    //   if(err != 0) {
    //      wcerr << "ERROR: Parsing input wave file\n" << cfgInput->filename << endl
    //         << "\t Drive: " << drive << endl
    //         << "\t Dir  : " << dir << endl
    //         << "\t File : " << fname << endl
    //         << "\t Ext  : " << ext << endl;
    //      exit;
    //   }


      strcpy(fname, std::filesystem::path(cfgInput->filename).filename().string().c_str());
      strcpy(dir, std::filesystem::path(cfgInput->filename).parent_path().string().c_str());

      memset(dir, 0, sizeof(char)*_MAX_DIR);
      strcpy(dir, cfgFrontEnd->out_data_dir);
      //swprintf_s(dir, _MAX_DIR, L"%s", cfgFrontEnd->out_data_dir);
      sprintf(cfgFrontEnd->basefilename, "%s", fname);
      sprintf(cfgTrain->basefilename, "%s", fname);
      sprintf(cfgGenFEF->basefilename, "%s", fname);
      sprintf(cfgBackEnd->basefilename, "%s", fname);
   }
   else {
      sprintf(cfgTrain->basefilename, "%s", cfgFrontEnd->basefilename);
      sprintf(cfgGenFEF->basefilename, "%s", cfgFrontEnd->basefilename);
      sprintf(cfgBackEnd->basefilename, "%s", cfgFrontEnd->basefilename);
   }

	// Create audio stream object
   CfgFileAudio cfgAudio;
   cfgAudio.szFilename       = cfgInput->filename;
   cfgAudio.bytesPerSample   = cfgInput->sampleSize;
   cfgAudio.samplesPerBlock  = cfgInput->bufferSize;
    
   WuwFileAudio* audio = new WuwFileAudio(&cfgAudio);

   // Create procInSamples Object
	procInSamples = new ProcessInputSamples(cfgInput);

	// Create FrontEnd Object with specified configuration
	frontEnd      = new FrontEnd(cfgFrontEnd);

	samples_buffer = new BYTE[cfgInput->bufferSize * cfgInput->sampleSize];

   // Create BackEnd Object with specified configuration
   if (cfgBackEnd->model[0] != NULL) {
      backEnd = new BackEnd(cfgBackEnd);
   } else {
      printf("cfgBackEnd->model[0] is null\n");
   }



   // if (configInfo->liveInput)
   // {
      // CfgLiveAudio cfgAudio;
      // cfgAudio.useUlaw    = false;
      // cfgAudio.sampleRate = 8000;
      // cfgAudio.nChannels  = 1;
      // cfgAudio.bitsPerSample = 16;
      // cfgAudio.dwBufferLength = cfgInput->bufferSize;
      // cfgAudio.nBuff = 16;
      // audio = new WuwLiveAudio(&cfgAudio);
      // if(!audio->Init())
      //    printf("Failed to initialize audio\n");

      //AGC = true;
      //audioGainRes = ISpAudio::GetVolumeLevel(&pLevel);
      //if (!audioGainRes) {
      //   printf("Audio Volume Information not returned: Disableing AGC\n");
      //   AGC = false;
      //}
      //else {
      //   printf(Audio Level Set To: *** %d ***\n", pLevel);
      //}
   // }
   // else
   // {
      // audio = new WuwFileAudio(cfgInput->filename, cfgInput->sampleSize, cfgInput->bufferSize);
   // }

   // Set the proper type for input samples buffer based on the configuration settings
	// samples_buffer = new BYTE[cfgInput->bufferSize * cfgInput->sampleSize];

   if (configInfo->recordLiveInput)
   {
      // sprintf(filename, "%s%s.sig", cfgFrontEnd->out_data_dir, cfgFrontEnd->basefilename);
      //swprintf_s(filename, _MAX_PATH, L"liveinput.sig");
      //swprintf_s(fname, _MAX_FNAME, L"liveinput.ulaw");

		// fp_input = fopen(filename, "wb");
		// if (!fp_input)
		// {
		// 	wcerr << "ERROR: Openning file for writing: " << filename << "\n";
		// }
		//fp_ulaw = _wfopen(fname, L"wb");
		//if (!fp_ulaw)
		//{
		//	wcerr << L"ERROR: Openning file for writing: " << fname << L"\n";
		//}
	}

	std::cout << "Processing Samples ...\n";



	audio->Start();
	while (audio->Read(samples_buffer))
	{
		if (count % 100 == 0) printf("\n%6d ", count);
		//if (count % 2 == 0)   wprintf(L".");

		// Convert samples to float
		fsamples = procInSamples->Run(samples_buffer);

		// Write to file
      if (configInfo->recordLiveInput) {
			fwrite(samples_buffer, sizeof(SINT16), bufferSize, fp_input);
			//fwrite(samples_buffer, sizeof(UCHAR), bufferSize, fp_ulaw);
      }
		// Send input sample data to front end
		features  = frontEnd->Run(fsamples);
      //if (AGC) {
      //   DeltaGain = features.Delta_AGC_Gain; // % correction from nominal Gain
      //   pLevel    += DeltaGain*MAX_VOLUME;

      //   //printf(Audio Level Set To: *** %d ***\n", pLevel);
      //}

      // Process FE features with BE
      if (backEnd != NULL) {
         // printf("backEnd->Run\n");
         backEnd->Run(features);

         // score    = backEnd->Run(features);
         //if (score == NULL)
         //   score    = backEnd->Run(features);

         // if (score != NULL) {
         //    FILE *fp_score;

         //    memset(filename, 0, sizeof(char)*_MAX_PATH);
         //    sprintf(filename, "%s/Scores.txt", dir);

         //    fp_score = fopen(filename, "at");
         //    if (!fp_score) {
         //       wcerr << L"ERROR: Openning file for writing: " << filename << "\n";
         //    }

         //    fprintf(fp_score, "%s,% 10.5f,% 10.5f,% 10.5f,% 10.5f,% 10.5f\n",  fname,
         //       score->for_matchI,
         //       score->revM_matchI,
         //       score->for_matchII,
         //       score->revM_matchII,
         //       score->revT_matchII);

         //    fprintf(stderr, "\n%s\t% 10.5f % 10.5f % 10.5f % 10.5f % 10.5f\n",  fname,
         //       score->for_matchI,
         //       score->revM_matchI,
         //       score->for_matchII,
         //       score->revM_matchII,
         //       score->revT_matchII);

         //    fclose(fp_score);
         // }
      }
		count++;
	}
   audio->Stop();
	// delete audio;
	//fclose(fp_input);

   // Flushing the buffers
   //while ((features = frontEnd.Eos())) {
   //   score = backEnd.Run(features);
   //}
}

int main(int argc, char* argv[]) {
    WuwConfig cfg;
    switch (argc)
	{
		// No arguments passed - exit
	    case 1:
		{
			printInfo(argv[0]);
			return 1;
		}

		// Single argument passed - assume it is a config file name
	    case 2:
		{
			if(!cfg.ParseConfigFile(argv[1]))
				return false;
			break;
		}

		// Multiple arguments passed - invoke command line parser
	    default:
		{
			if(!cfg.ParseCommandLine(argc, argv))
				return false;
			break;
		}
	}

	printf("Processing ...\n");


	// Check start time
	// DWORD dwTime = GetTickCount();

	// Run the front end
	run_e_wuw(&cfg);

	// Calculate elapsed time
	// dwTime = GetTickCount() - dwTime;

	// printf("\nElapsed Time <%.2f [sec]>\n", (dwTime / 1000.0));

	return 0;
}

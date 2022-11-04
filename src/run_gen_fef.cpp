/******************************************************************************

                    Copyright (C) 2006-2015 VoiceKey Inc.
                         Cocoa Beach, Florida 32931
                            All rights reserved

                      Confidential and Proprietary

 Module: run_genFEF.cpp
 Author: Veton Kepuska
Created: January 19, 2007
Updated: 

Description: A wrapper file the invokes front_end to generate FEF features.
             It my also invoce back_end to run recogntion if model is provided. 
             Back_end is used to filter out outliers from training while 
             generating FEF.

$Log:$

*******************************************************************************

Notes: 

******************************************************************************/

/******************************  Include Files  ******************************/

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>

using namespace std;

#include "wuw_config.h"
#include "front_end/front_end.h"
#include "back_end/back_end.h"
#include "process_in_samples.h"
#include "audio_stream.h"

//-----------------------------------------------------------------------------
// Name: printInfo
// Desc: prints command line instructions
//-----------------------------------------------------------------------------
void printInfo(char* exename) 
{
   wcerr << "Usage: " << exename << endl
      << "\t Options:" << endl
      << "\t\t<filename.conf>" << endl
      << "\t and/or" << endl
      << "\t\t -s \t sample rate     <[8]|16> in kHz" << endl
      << "\t\t -f \t sample type     <[ulaw], alaw, short, nist>" << endl
      << "\t\t -h \t header size     <[1024]>" << endl
      << "\t\t -i \t input data dir  <d:/path or ../path>" << endl
      << "\t\t -T \t trans file      <c:/path/corpus.trans or ../path/corpus.trans>" << endl
      << "\t\t OR" << endl
      << "\t\t -L \t list file       <c:/path/list.dat or ../path/list.dat>" << endl
      << "\t\t -o \t output FEF dir  <o:/path or ../path" << endl
      << "\t\t -M \t model files     <m:/path/WUWmodel1.dtw m:/path/WUWmodel2.dtw ..." << endl
      << "\t\t -G \t generate FEFs   <true, [false]> binary switch - if used is set to \"true\"" << endl
      << "\t\t -m \t monitor         <true, [false]> binary switch - if used is set to \"true\"" << endl
      << "\t\t -D \t dump ALL FEF    <true, [false]> binary switch - if used is set to \"true\"" << endl;
}
//-----------------------------------------------------------------------------
// run_wuw function - 
// must convert most of the functionality in this file into RunWUW class.
//-----------------------------------------------------------------------------
void run_e_wuw (WuwConfig *configInfo) 
{
   
   //_CrtDumpMemoryLeaks();
   //_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
   //_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
   //_CrtMemState s1, s2, s3;
   //int n_iter = 0;*/

   // Check start time
   // DWORD BeginTime = GetTickCount();
   // DWORD StartTime, CurrentTime;

   char drive[_MAX_DRIVE];
   char dir[_MAX_DIR];
   char path[_MAX_PATH];
   char fname[_MAX_FNAME];
   char ext[_MAX_EXT];
   //
   // Need to have a function call to hide this complexity of setting cfg structures
   //
   CfgInput*    cfgInput;   // = configInfo->GetInputConfig();
   CfgFrontEnd* cfgFrontEnd;// = configInfo->GetFrontEndConfig();
   CfgBackEnd*  cfgBackEnd; //  = configInfo->GetBackEndConfig();
   CfgTrain*    cfgTrain;   //  = configInfo->GetTrainConfig();
   CfgGenFEF*   cfgGenFEF;  //   = configInfo->GetGenFEFConfig();

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
   //
   // Opening the list file
   //
   WUW_IO   listfile(cfgGenFEF->filelist, false, "r");
   vector<string> *input_list;
   vector<string>::iterator   vec_it;
   //
   // Reading list file one line at a time
   //
   while ((input_list = listfile.ReadListFile()) != NULL) {
      //n_iter++;
      //if (n_iter%2 == 0) {
      //   _CrtMemCheckpoint( &s2 );
      //   //_CrtMemDumpStatistics( &s2 );

      //   if ( _CrtMemDifference( &s3, &s1, &s2) )
      //      _CrtMemDumpStatistics( &s3 );
      //   
      //   _CrtMemDumpAllObjectsSince( NULL );
      //}
      //else {
      //   _CrtMemCheckpoint( &s1 );
      //   //_CrtMemDumpStatistics( &s1 );
      //}

      // StartTime = GetTickCount();
      //
      // Main Loop - Processing all data files from the list file.
      // Parsing the line into tokens -- at a moment only one token per line is assumed
      //
      for (vec_it = input_list->begin(); vec_it != input_list->end(); vec_it++) {
         //wchar_t* wavfile         = cfgInput->filename;
         string  bslash            = "\\";
         string  dot               = "\.";
         string  wavfile;
         //wstring  wavfile           = cfgGenFEF->in_data_dir + *vec_it;
         int loc                    =  (*vec_it).find(dot);

         if (vec_it[0] != bslash) 
            if ( loc == -1) 
               wavfile           = cfgGenFEF->in_data_dir + *vec_it + ".ulaw";
            else
               wavfile           = cfgGenFEF->in_data_dir + *vec_it;

         else 
            if (loc == -1) 
               wavfile           = cfgGenFEF->in_data_dir + *vec_it + ".ulaw";
            else
               wavfile           = cfgGenFEF->in_data_dir + *vec_it;

         SINT32   bufferSize        = cfgInput->bufferSize;
         SINT32   count             = 0;
         void    *samples_buffer;
         FLOAT32 *fsamples;
         FE_FEATURES *features;
         //BE_SCORE    *score;
         // ms_DTW_Score  *score;

         strcpy(cfgInput->filename, wavfile.c_str());
         printf("%s\t", vec_it->c_str());

         // Parsing output feature file into full path, basefilename and extension
         // int err = _wsplitpath_s(cfgInput->filename, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT); 

         // if(err != 0) {
         //    wcerr << L"ERROR: Parsing input wave file\n" << cfgInput->filename << endl
         //       << L"\t Drive: " << drive << endl
         //       << L"\t Dir  : " << dir << endl
         //       << L"\t File : " << fname << endl
         //       << L"\t Ext  : " << ext << endl;
         //    break;
         // }

         strcpy(fname, std::filesystem::path(cfgInput->filename).filename().string().c_str());
         strcpy(dir, std::filesystem::path(cfgInput->filename).parent_path().string().c_str());


         sprintf(cfgFrontEnd->basefilename, "%s", fname);
         sprintf(cfgTrain->basefilename, "%s", fname);
         sprintf(cfgGenFEF->basefilename, "%s", fname);
         sprintf(cfgBackEnd->basefilename, "%s", fname);

         if(cfgTrain->in_data_dir == NULL) 
            cfgTrain->in_data_dir = new char[_MAX_PATH];

         sprintf(cfgTrain->in_data_dir, "%s", dir);

         if(cfgTrain->out_data_dir == NULL) 
            cfgTrain->out_data_dir = new char[_MAX_PATH];

         strcpy(cfgTrain->out_data_dir, cfgGenFEF->out_data_dir);

         //
         // Processing Samples
         //
         ProcessInputSamples procInSamples(cfgInput);

         // Create FrontEnd Object with specified configuration
         FrontEnd frontEnd(cfgFrontEnd);

         // Create BackEnd Object with specified configuration
         BackEnd  backEnd(cfgBackEnd);

         // Create audio stream object
         WuwFileAudio* audio;

         //	if(cfgInput->bLiveAudio)
         //		audio = new WuwLiveAudio();
         //	else
         audio = new WuwFileAudio(cfgInput->filename, cfgInput->sampleSize, cfgInput->bufferSize);

         // Set the proper type for input samples buffer based on the configuration settings
         samples_buffer = new BYTE[cfgInput->bufferSize * cfgInput->sampleSize];

         // Create output file
         char filename[_MAX_PATH];
         //swprintf_s(filename, _MAX_PATH, L"%s.input", cfgFrontEnd->out_data_dir);
         // swprintf_s(filename, _MAX_PATH, L"%s/%s.input", cfgFrontEnd->out_data_dir, cfgFrontEnd->basefilename);
         sprintf(filename, "%s/%s.input", cfgFrontEnd->out_data_dir, cfgFrontEnd->basefilename);

         if(!std::filesystem::exists(cfgInput->filename)) {
            printf("Skipping %s, does not exist", cfgInput->filename);
            continue;
         }

         FILE* fp_input;

         // If logging, open file for writing
         if (cfgFrontEnd->monitor) 
         {
            fp_input = fopen(filename, "wb");
            if (!fp_input) 
            {
               wcerr << "ERROR: Openning file for writing: " << filename << L"\n";
            }
         }

         audio->Start();
         while (audio->Read(samples_buffer)) 
         {
            //if (count % 100 == 0) wprintf(L"\n%6d ", count);
            //if (count % 2 == 0)   wprintf(L".");

            // Convert samples to float
            fsamples = procInSamples.Run(samples_buffer);

            // Write to file
            if ( cfgFrontEnd->monitor && fp_input )
               fwrite(fsamples, sizeof(FLOAT32), bufferSize, fp_input);

            // Send input sample data to front end
            features = frontEnd.Run(fsamples);

            // Process FE features with BE
            backEnd.Run(features);

            count++;
         }

         audio->Stop();
         // delete audio;
         delete [] samples_buffer;

         // Calculate elapsed time
         // CurrentTime = GetTickCount();

         // wprintf(L"<%.2f [sec]>\t", ((CurrentTime-StartTime) / 1000.0));
         // wprintf(L"<%.2f [sec]>\n", ((CurrentTime-BeginTime) / 1000.0));

         // Flushing the buffers
         //while ((features = frontEnd.Eos())) {
         //   score = backEnd.Run(features);
         //}
      }
   }
}
//-----------------------------------------------------------------------------
// Name: FE_Thread
// Desc: 
//-----------------------------------------------------------------------------
int FE_Thread()
{
   return -1;
}

//-----------------------------------------------------------------------------
// Name: wmain
// Desc: program entry point
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
   WuwConfig cfg;
   char *h[] = { 
      "?", "-?", 
      "h", "-h", "--help"
   };
   size_t  i, num = sizeof(h)/sizeof(h[0]);

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
         for (i=0; i<num; i++) {
            if (strcmp(argv[1], h[i])==0) {
               printInfo(argv[0]);
               return 1;
            }
         }

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

   wprintf(L"Processing ...\n");

   // Check start time
   // DWORD dwTime = GetTickCount();

   // Run the front end
   run_e_wuw(&cfg);	

   // Calculate elapsed time
   // dwTime = GetTickCount() - dwTime;

   // wprintf(L"\nElapsed Time <%.2f [sec]>\n\n", (dwTime / 1000.0));

   return(0);
}

/*******************************  End of File  *******************************/

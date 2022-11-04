/******************************************************************************

                    Copyright (C) 2006-2015 VoiceKey Inc.
                         Cocoa Beach, Florida 32931
                            All rights reserved

                       Confidential and Proprietary


     Module: fe_config.h
     Author: Veton Kepuska
    Created: May 27, 2006
    Updated:

Description: Contains all parameters that define each e_wuw class.

$Log:$

*******************************************************************************

Notes:

******************************************************************************/

#ifndef WUW_CONFIG_H
#define WUW_CONFIG_H

/******************************  Include Files  ******************************/

#include <algorithm>
#include <iostream>
#include "defines.h"
#include <string.h>
#include <iostream>
#include <filesystem>

using namespace std;

/*********************************  Defines  *********************************/


/*****************************  Class Definition  *****************************/

class WuwConfig
{
public:

	WuwConfig();                        // Default Constructor
	~WuwConfig();

	void Init(string feConfigFileName);
	void Reset();

	// Utility Functions
	bool ParseConfigFile(char* szFileName);
	bool ParseCommandLine(int argc, char *argv[]);

	CfgInput*    GetInputConfig()    { return &m_cfgInput; }
	CfgFrontEnd* GetFrontEndConfig() { return &m_cfgFrontEnd; }
	CfgBackEnd*  GetBackEndConfig()  { return &m_cfgBackEnd; }
	CfgTrain*    GetTrainConfig()    { return &m_cfgTrain; }
	CfgGenFEF*   GetGenFEFConfig()   { return &m_cfgGenFEF; }

   BOOLEAN      liveInput;
   BOOLEAN      recordLiveInput;

private:

	// Those are temporarily here
	UINT16       header_size;
	char*     wav_filename;
	char*     output_wav_filename;
	char*     fef_dir;
	char*     enh_filename;
	char*     monitor;

	CfgInput     m_cfgInput;
	CfgFrontEnd  m_cfgFrontEnd;
	CfgBackEnd   m_cfgBackEnd;
   CfgTrain     m_cfgTrain;
   CfgGenFEF    m_cfgGenFEF;

   void printInfo(int argc, char* argv[]);

};

#endif //WUW_CONFIG_H

/*******************************  End of File  *******************************/

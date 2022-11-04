/******************************************************************************

	              Copyright (C) 2006-2015 VoiceKey Inc.
                           Cocoa Beach, Florida 32931
			      All rights reserved

			  Confidential and Proprietary


     Module: fio.h
     Author: Veton Kepuska
    Created: June 1, 2006
    Updated:

Description: Provides basic File I/O functions

       $Log:$

*******************************************************************************

Notes:

******************************************************************************/

#ifndef FIO_H
#define FIO_H


/******************************  Include Files  ******************************/

#include <string>

#include <iostream>
using namespace std;

#include <iomanip>
using namespace std;

#include <fstream>
using namespace std;

#include "common/win32types.h"

class FIO {

 public:
  // Default Constructor
  FIO();
  // Constructor
  FIO (string &file);
  FIO (string &file, int mode);

  // mode must be one ios::in | ios::out or both alos it can ios::binary
  fstream *OpenFile (string &fname, int mode);
  // mode must be one or both ios::in | ios::binary
  fstream *OpenFileForReading (int mode);
  // mode must be one or both ios::out | ios::binary
  fstream *OpenFileForWriting (int mode);
  bool   isOpen();
  SCHAR  *ReadSamples(SCHAR  *buffer, int sampleSize, int n_samples);

  void CloseFile();

 private:
  string   filename;
  fstream  myfstream;

};

#endif // FIO_H

/*******************************  End of File  *******************************/


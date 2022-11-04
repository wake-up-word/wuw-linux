// Name: Audio Stream
// Date: 
// Author: Tudor Klein

#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

// #include <windows.h>
// #include <mmreg.h>
#include <iostream>
#include <cstring>
#include "common/wuw_common.h"

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------
// void Audio_Interface_Play_Sound(TCHAR* filename);
// void aiError(MMRESULT r, char* loc);

typedef unsigned char BYTE;

struct CfgLiveAudio
{
	int sampleRate;
	int bitsPerSample;
	int nChannels;
	int dwBufferLength;
	int nBuff;
	bool useUlaw;
};

struct CfgFileAudio
{
	char* szFilename;
	int bytesPerSample;
	int samplesPerBlock;
};


class WuwAudioStream
{
public:
		
	virtual bool Init() = 0;                   // Allocates buffers, opens stream for reading
	virtual bool Release() = 0;                // Releases all resources

	virtual bool Start() = 0;                  // Starts reading from stream
	virtual bool Stop() = 0;                   // Stops reading from stream
	virtual bool Reset() = 0;                  // Clears buffer or rewinds stream
	virtual int Read(void* dest) = 0;		   // Returns next data block
	
private:
	CfgFileAudio m_cfg;
	FILE* fpInputFile;
};

// class WuwLiveAudio : public WuwAudioStream
// {
// public:

// 	WuwLiveAudio(CfgLiveAudio* cfg);
// 	~WuwLiveAudio();

// 	virtual bool Init();
// 	virtual bool Release();
// 	virtual bool Start();
// 	virtual bool Stop();
// 	virtual bool Reset() {return false;};
// 	virtual int Read(void* dest);

// 	// friend void CALLBACK aiWaveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

// private:
// 	struct AudioBuffer
// 	{
// 		// WAVEHDR*	whdr;
// 		BYTE*		data;
// 		int			dataLength;
// 	} ;
// 	CfgLiveAudio m_cfg;
// 	AudioBuffer*		buffers;				// data buffers
// 	int					nBuffers;				// number of data buffers
// 	// HWAVEIN				hwi;					// handle to waveIn device
// 	int					currentBuf;				// current buffer to read from
// 	int					nextBuf;				// next buffer to write to 
// 	// HANDLE				hSemaphoreInputWaiting;	// semaphore that signals when new input is ready
// 	// HANDLE				hEvStopEvent;			// event used to stop recording
// 	bool				bIsRecording;	//
// 	BYTE*				tempBuffer;				//


// };

// class WuwFileAudio : public WuwAudioStream
class WuwFileAudio
{
public:
	WuwFileAudio();
	WuwFileAudio(CfgFileAudio* cfg);
	WuwFileAudio(char* szFilename, int dwBytesPerSample, int dwBlockSize);
	~WuwFileAudio();

	bool Init();
	bool Release();
	bool Start();
	bool Stop();
	bool Reset();
	int Read(void* dest);

private:
	CfgFileAudio m_cfg;
	FILE* fpInputFile;
};

class WuwFileAudio2
{
	public:
		WuwFileAudio2();
};


#endif

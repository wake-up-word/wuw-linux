// Name: File Audio
// Date: 
// Author: Tudor Klein

#include "audio_stream.h"

WuwFileAudio::WuwFileAudio(CfgFileAudio* cfg)
{
	memcpy(&m_cfg, cfg, sizeof(CfgFileAudio));
	//bIsRecording = false;
}

WuwFileAudio::WuwFileAudio(char* szFilename, int dwBytesPerSample, int dwBlockSize)
{
	m_cfg.bytesPerSample = dwBytesPerSample;
	m_cfg.samplesPerBlock = dwBlockSize;
	m_cfg.szFilename = szFilename;
	//bIsRecording = false;
}

WuwFileAudio::~WuwFileAudio()
{
	Release();
}

//-----------------------------------------------------------------------------
// Name: Audio_Interface_Initialize()
// Desc: Initializes the audio interface and memory associated with it
//-----------------------------------------------------------------------------
bool WuwFileAudio::Init()
{

	return true;
}

//-----------------------------------------------------------------------------
// Name: Audio_Interface_Shutdown()
// Desc: Shuts down the audio interface and frees all memory
// Notes: Make sure recording is stopped prior to calling this function
//-----------------------------------------------------------------------------
bool WuwFileAudio::Release()
{
	fclose(fpInputFile);
	return true;
}

//-----------------------------------------------------------------------------
// Name: Audio_Interface_Start_Recording()
// Desc: Starts recording
//-----------------------------------------------------------------------------
bool WuwFileAudio::Start()
{
	fpInputFile = fopen(m_cfg.szFilename, "rb");
   if (fpInputFile == NULL) {
      perror( "fopen failed WuwFileAudio::Start" );
		printf("Filename: %s\n", m_cfg.szFilename);
      return false;
   }
	return true;
}

//-----------------------------------------------------------------------------
// Name: Audio_Interface_Stop_Recording()
// Desc: Stops recording
//-----------------------------------------------------------------------------
bool WuwFileAudio::Stop()
{
   if (fpInputFile)
      fclose(fpInputFile);

	return true;
}

bool WuwFileAudio::Reset()
{
   if (fpInputFile)
      fclose(fpInputFile);

	return false;
}

//-----------------------------------------------------------------------------
// Name: Audio_Interface_Get_Next_Data()
// Desc: Returns a pointer to the next available data buffer
//-----------------------------------------------------------------------------
int WuwFileAudio::Read(void* dest)
{	
	return fread(dest, m_cfg.bytesPerSample, m_cfg.samplesPerBlock, fpInputFile);
}



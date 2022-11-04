/*****************************************************************************

                     Copyright (C) 2006-2015 VoiceKey Inc.
                          Cocoa Beach, Florida 32931
                              All rights reserved

                         Confidential and Proprietary

     Module: wuw_io.cpp
     Author: Veton Kepuska
    Created: September 17, 2006
    Updated:

Description: WUW IO functions.

$Log:$

*******************************************************************************

            Proprietary and Confidential. All rights reserved.
                VoiceKey Inc., Cocoa Beach, Florida 32931.

******************************************************************************/

/**********************************  Notes  **********************************
//
// It may be benefitial to merge wuwlogger class functionality and
// WUWIO via inherentance or composition. Function members of ModelIO
// are similar to wuwlogger member functions.
//
/******************************  Include Files  ******************************/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
//#include <string.h>
#include <stdarg.h>
#include <wchar.h>

#include "wuw_io.h"

/*********************************  Defines  *********************************/

/****************************  Function Members  *****************************/
//
// Default Constructor
//
WUW_IO::WUW_IO(const char* filename,
                 bool is_binary,
                 const char *mode,
                 BACK_END_TYPE be_type)
{
  model_type  = be_type;
  m_header    = NULL;
  buffer      = NULL;
  m_ppvItem   = NULL;
  m_pvItem    = NULL;
mp_num_features = NULL;
  Set_WUW_IO(filename, is_binary, mode);

  // It must be set after the previous call
  m_is_model  = true;

  SetFileInfo();
}

WUW_IO::WUW_IO(const char* filename,
               bool is_binary,
               const char *mode)
{
   Set_WUW_IO(filename, is_binary, mode);
}

void WUW_IO::Set_WUW_IO(const char* filename,
                        bool is_binary,
                        const char *mode)
{

  m_filename  = strdup(filename);
  m_fBinary   = is_binary;

  m_itemSize  = 0;
  m_itemCount = 0;
  m_ppvItem   = NULL;
  m_pvItem    = NULL;
  m_header    = NULL;
  buffer      = NULL;
  mp_num_features = NULL;

  m_is_model  = false;

  Open(mode);

}

WUW_IO::WUW_IO(const char* filename,
	           size_t itemSize,
	           size_t itemCount,
	           const char *mode,
	           BACK_END_TYPE be_type)
{
  model_type  = be_type;
  m_header    = NULL;
  buffer      = NULL;
  m_ppvItem   = NULL;
  m_pvItem    = NULL;
  mp_num_features = NULL;

  Set_WUW_IO(filename, itemSize, itemCount, mode);

  // It must be set after the previous call
  m_is_model  = true;

  SetFileInfo();
}

WUW_IO::WUW_IO(const char* filename,
	            size_t itemSize,
	            size_t itemCount,
	            const char *mode)
{
   Set_WUW_IO(filename, itemSize, itemCount, mode);
}

void WUW_IO::Set_WUW_IO(const char* filename,
	                     size_t itemSize,
	                     size_t itemCount,
	                     const char *mode)
{
  m_filename  = strdup(filename);
  m_fBinary   = true;

  m_itemSize  = itemSize;
  m_itemCount = itemCount;

  m_is_model  = false;
  m_header    = NULL;
  buffer      = NULL;
  m_ppvItem   = NULL;
  m_pvItem    = NULL;
  mp_num_features = NULL;

  Open(mode);

}
//
//
//
WUW_IO::WUW_IO(const char* filename,
	       size_t itemSize,
	       size_t itemCount,
	       const char *mode,
	       void** ptrItem,
          BACK_END_TYPE be_type)
{
  model_type  = be_type;
  m_header    = NULL;
  buffer      = NULL;
  m_pvItem    = NULL;

  Set_WUW_IO(filename, itemSize, itemCount, mode, ptrItem);

  // It must be set after the previous call
  m_is_model  = true;

  SetFileInfo();
}

WUW_IO::WUW_IO(const char* filename,
	       size_t itemSize,
	       size_t itemCount,
	       const char *mode,
	       void** ptrItem)
{
   Set_WUW_IO(filename, itemSize, itemCount, mode, ptrItem);
}
//
WUW_IO::WUW_IO(const char* filename,
	       size_t sampleSize,
	       size_t numVectors,
          size_t numElements,
          size_t frameRate,
	       const char *mode,
	       vector<FLOAT32 *>* ptrItem, //vector<FLOAT32 *> *
          BACK_END_TYPE be_type)
{
  model_type  = be_type;
  m_header    = NULL;
  buffer      = NULL;
  m_ppvItem   = NULL;

  m_frame_rate = frameRate;

  Set_WUW_IO(filename,  sampleSize, numVectors, numElements, mode, ptrItem);

  // It must be set after the previous call
  m_is_model  = true;

  SetFileInfo();
}
//
void WUW_IO::Set_WUW_IO(const char* filename,
	       size_t itemSize,
	       size_t itemCount,
	       const char *mode,
	       void** ptrItem)
{
  m_filename   = strdup(filename);
  m_fBinary    = true;
  m_itemSize   = itemSize;
  m_itemCount  = itemCount;
  m_ppvItem    = ptrItem;

  m_pvItem     = NULL;
  m_header     = NULL;
  buffer       = NULL;
  m_is_model   = false;

  Open(mode);

}
//
void WUW_IO::Set_WUW_IO(const char* filename,
	       size_t itemSize,
	       size_t itemCount,
          size_t numItems,
	       const char *mode,
	       vector<FLOAT32*>* ptrItem)
{
  m_filename   = strdup(filename);
  m_fBinary    = true;
  m_itemSize   = itemSize;
  m_itemCount  = (m_num_vectors = itemCount); // Number of Vectors
  m_num_features = numItems;  // Number of Elements per Vector
  m_pvItem     = ptrItem;

  m_ppvItem    = NULL;
  m_header     = NULL;
  buffer       = NULL;
  m_is_model   = false;

  Open(mode);

}
//
// Open File in mode
//
bool WUW_IO::Open(const char *mode)
{
  m_f = fopen(m_filename, mode);

  if(!m_f) {
    fwprintf(stderr, L"ERROR: %s Opening file in %s mode: %S\n",
	    strerror( errno ), mode, m_filename);
    return(false);
  }

  InitTypes();

  return(true);
}
//
void WUW_IO::SetDataType(SAMPLE_TYPE sample_type, FEATURE_TYPE feature_type)
{
   m_sample_type  = sample_type;
   m_feature_type = feature_type;
}
//
void WUW_IO::InitTypes()
{
   //
   // This is not the most logical place to set map associative array
   //
   m_mapDataType[ST_ULAW]    = "ulaw";
   m_mapDataType[ST_ALAW]    = "alaw";
   m_mapDataType[ST_SCHAR]   = "char";
   m_mapDataType[ST_UCHAR]   = "uchar";
   m_mapDataType[ST_SINT8]   = "char";
   m_mapDataType[ST_UINT8]   = "uchar";
   m_mapDataType[ST_SINT16]  = "short";
   m_mapDataType[ST_UINT16]  = "ushort";
   m_mapDataType[ST_SINT32]  = "int";
   m_mapDataType[ST_UINT32]  = "uint";
   m_mapDataType[ST_FLOAT32] = "float";
   m_mapDataType[ST_FLOAT64] = "double";

   m_mapFeatureType[FT_STD_MFCC]        = "STD_MFCC";
   m_mapFeatureType[FT_ETSI_MFCC]       = "ETSI_MFCC";
   m_mapFeatureType[FT_LPC_MFCC]        = "LPC_MFCC";
   m_mapFeatureType[FT_VK_LPCENH]       = "VK_LPCENH";
   m_mapFeatureType[FT_VK_STDENH]       = "VK_STDENH";
   m_mapFeatureType[FT_VK_LPC_ENH_MFCC] = "VK_LPC_ENH_MFCC";
   m_mapFeatureType[FT_VK_FB_SPEC]      = "VK_FB_SPEC";
}
//
//
//
void WUW_IO::SetFileInfo()
{
  if (m_is_model)
    SetModelInfo();
  else
    SetFEFInfo();
}
//
//
//
void WUW_IO::SetModelInfo()
{
  switch (model_type) {
  case DTW_m:
    // Note that DTW_HEADER_SIZE - is given in Byte Units
    // m_header_size shoudl be in sizeof(char) units
    m_header_size = DTW_HEADER_SIZE;
    if (m_header != NULL) delete [] m_header;
    m_header      = new char [m_header_size];
    memset(m_header, 0, m_header_size*sizeof(char));

    if (m_itemSize != 0)
      dtw_m_model.sample_size  = (UINT16) m_itemSize;
    if (m_itemCount != 0)
      dtw_m_model.num_features = (UINT16) m_itemCount;

    dtw_m_model.frame_rate   = m_frame_rate;
    dtw_m_model.feature_type = FT_LPC_MFCC;     // m_feature_type;
    //dtw_m_model.model_name   = m_filename;
    // snprintf(dtw_m_model.model_name, _MAX_PATH, L"%s", m_filename);
    dtw_m_model.num_features = m_num_features;
    dtw_m_model.num_vectors  = m_num_vectors;
    dtw_m_model.sample_size  = m_itemSize;
    dtw_m_model.sample_type  = ST_FLOAT32;
    dtw_m_model.NNZ_FACTOR   = 0;
    dtw_m_model.NUMD_FACTOR  = 0;
    dtw_m_model.dtw_m_model  = m_pvItem;

    break;
  case HMM_CD:
    break;
  }
}
//
//
//
void WUW_IO::SetFEFInfo()
{
  // Note that FEF_HEADER_SIZE - is given in Byte Units
  // m_header_size shoudl be in sizeof(char) units
  m_header_size = FEF_HEADER_SIZE;
  if (m_header == NULL)
	  m_header      = new char [m_header_size];

  memset(m_header, 0, m_header_size*sizeof(char));
  m_num_feature_types = 0;
  m_feature_type      = 0;
}

void WUW_IO::SetFEFInfo(UINT16 num_vecs, UINT16 num_mfcc_features, UINT16 num_enh_features, UINT16 frame_rate)
{
   SetFEFInfo();
   m_num_feature_types = 3;
   m_feature_type      = 0;
   m_feature_type     |= FT_ETSI_MFCC;
   m_feature_type     |= FT_LPC_MFCC;
   m_feature_type     |= FT_VK_LPC_ENH_MFCC;
   //
   // This needs to be set elsewhere
   //
   m_sample_type = ST_FLOAT32;
   switch (m_sample_type) {
     case ST_FLOAT32:
        mp_num_features = new UINT16 [m_num_feature_types];
        break;
     default:
        mp_num_features = new UINT16 [m_num_feature_types];
        break;
   }
   mp_num_features[0] = num_mfcc_features;
   mp_num_features[1] = num_mfcc_features;
   mp_num_features[2] = num_enh_features;

   m_num_vectors = num_vecs;
   m_frame_rate  = frame_rate;
}

void WUW_IO::SetFEFInfo(FE_FEATURES *fef_packet, UINT16 num_vecs, UINT16 frame_rate)
{
  // Allocates header -- Must have configurable version
  SetFEFInfo();
  if (fef_packet->num_mfcc_features > 0) {
    m_num_feature_types++;
    m_feature_type |= FT_ETSI_MFCC;
    m_num_feature_types++;
    m_feature_type |= FT_LPC_MFCC;
  }
  if (fef_packet->num_enh_features > 0) {
    m_num_feature_types++;
    m_feature_type |= FT_VK_LPC_ENH_MFCC;
  }
  //
  // This needs to be set elsewhere
  //
  m_sample_type = ST_FLOAT32;
  switch (m_sample_type) {
     case ST_FLOAT32:
        mp_num_features = new UINT16 [m_num_feature_types];
        break;
     default:
        mp_num_features = new UINT16 [m_num_feature_types];
        break;
  }
  //
  // Must Covert this type into Hash Set
  //  Key - FEATURE TYPE
  //  Val - NUM of FEATURES
  // Assumes equal size of MFCC values for ETSI_MFCC and LPC_MFCC
  if (fef_packet->num_mfcc_features > 0) {
    mp_num_features[0] = fef_packet->num_mfcc_features;
    mp_num_features[1] = fef_packet->num_mfcc_features;
  }
  if (fef_packet->num_enh_features > 0)
    mp_num_features[2] = fef_packet->num_enh_features;

  m_num_vectors = num_vecs;
  m_frame_rate  = frame_rate;
}
//
//
//
void WUW_IO::SetFEFInfo(SEGMENT_FEATURES *fef_segment)
{
  // Allocates header -- Must have configurable version
  SetFEFInfo();
  if (fef_segment->num_mfcc_features > 0) {
    m_num_feature_types++;
    m_feature_type |= FT_ETSI_MFCC;
    m_num_feature_types++;
    m_feature_type |= FT_LPC_MFCC;
  }
  if (fef_segment->num_enh_features > 0) {
    m_num_feature_types++;
    m_feature_type |= FT_VK_LPC_ENH_MFCC;
  }
  //
  // This needs to be set elsewhere
  //
  m_sample_type = ST_FLOAT32;
  switch (m_sample_type) {
     case ST_FLOAT32:
        mp_num_features = new UINT16 [m_num_feature_types];
        break;
     default:
        mp_num_features = new UINT16 [m_num_feature_types];
        break;
  }
  // Assuming that Standard/ETSI and LPC MFCC Features are of the same size
  if (fef_segment->num_mfcc_features > 0) {
    mp_num_features[0] = fef_segment->num_mfcc_features;
    mp_num_features[1] = fef_segment->num_mfcc_features;
  }
  if (fef_segment->num_enh_features > 0)
    mp_num_features[2] = fef_segment->num_enh_features;

  m_num_vectors = fef_segment->num_feature_vectors;
  m_frame_rate  = fef_segment->frame_rate;
}
//
DTW_m_Model *WUW_IO::get_DTW_Model()
{
   return(&dtw_m_model);
}
//
//
//
WUW_IO::~WUW_IO()
{
  if(m_f != NULL){
      // TODO fix this
      //fclose(m_f);
  }

  if(m_header)
	  delete [] m_header;

  free(m_filename);

  if(mp_num_features)
	  delete [] mp_num_features;

  //if (m_is_model) {
  //   for (int i=0; i<dtw_m_model.num_vectors; i++) {
  //      delete [] dtw_m_model.dtw_m_model->at(i);
  //   }
  //   dtw_m_model.dtw_m_model->clear();

  //   //for (int i=0; i<dtw_m_model.num_vectors; i++) {
  //   //   delete [] dtw_m_model.dtw_m_model[i];
  //   //}
  //   //delete [] dtw_m_model.dtw_m_model;
  //}
  // This space must persist
  //delete [] buffer;

  //if (m_seg_features)
  //   delete m_seg_features;
  //if (mp_num_features)
  //   delete mp_num_features;
  //if (m_header)
  //   delete m_header;
}
//
//
//
//bool WUW_IO::FreeMemory(SEGMENT_FEATURES *seg_features) {
//   if (seg_features) {
//      for (int i=0; i<seg_features->num_feature_vectors; i++) {
//         delete [] seg_features->fe_mfcc_features_buffer[i];
//         delete [] seg_features->fe_enh_mfcc_features_buffer[i];
//      }
//      seg_features->fe_mfcc_features_buffer.clear();
//      seg_features->fe_enh_mfcc_features_buffer.clear();
//      delete seg_features;
//   }
//   if (mp_num_features)
//      delete [] mp_num_features;
//   if (m_header)
//      delete [] m_header;
//
//   return(true);
//}
//
// Write Member Functions
//
bool WUW_IO::WriteBinary(const void* p, size_t size, size_t count)
{
  size_t wcount;

  if(!m_f || !m_fBinary || !p) {
    fwprintf(stderr, L"ERROR: Writting to file:\n\tFILE: %s\n\t", m_filename);
    fwprintf(stderr, L"File NOT Opened in Binary Mode or\n\t");
    fwprintf(stderr, L"File NOT Opened or File Pointer corrupted or\n\t");
    fwprintf(stderr, L"Data Pointer is NULL\n");

    return (false);
  }

  if ((wcount = fwrite(p, size, count, m_f)) != count) {
    perror("SYSMSG:");
    fwprintf(stderr, L"ERROR: %S\nWritting to file:\n\tFILE: %s\n\tExpected %d items, written %d\n",
	    strerror( errno ), m_filename, count, wcount);

    return (false);
  }

  return(true);
}

bool WUW_IO::WriteBinary(const void* p)
{
  size_t wcount;

  if(!m_f || !m_fBinary || !p) {
    fwprintf(stderr, L"ERROR: %s\nWritting to file:\n\tFILE: %S\n\t", strerror( errno ), m_filename);
    fwprintf(stderr, L"File NOT Opened in Binary Mode or\n\t");
    fwprintf(stderr, L"File NOT Opened or File Pointer corrupted or\n\t");
    fwprintf(stderr, L"Data Pointer is NULL\n");

    return (false);
  }

  if ((wcount = fwrite(p, m_itemSize, m_itemCount, m_f)) != m_itemCount) {
    perror("SYSMSG:");
    fwprintf(stderr, L"ERROR: %s\nWritting to file:\n\tFILE: %S\n\tExpected %d items, written %d\n",
	    strerror( errno ), m_filename, m_itemCount, wcount);

    return (false);
  }

  return(true);
}

bool WUW_IO::Printf(const wchar_t* format, ...)
{
  if(!m_f){
    fwprintf(stderr, L"ERROR: %s\nWritting to file:\n\tFILE: %S\n\t", strerror( errno ), m_filename);
    fwprintf(stderr, L"File NOT Opened or File Pointer Corrupted\n");

    return (false);
  }

  va_list args;
  va_start(args, format);
  if (!vfwprintf(m_f, format, args)) {
    fwprintf(stderr, L"ERROR: %s\n\tPrint error using vfwprintf to file:\n\tFILE: %s\n",
    strerror( errno ), m_filename);

    return (false);
  }
  va_end(args);

  return (true);
}

bool WUW_IO::Write_DTW_Header(SCHAR *file_type)
{
  UINT16 i;
  bool   retval;

  if ((strcmp(file_type, "VK_FEATURES_FILE") != 0) &&
      (strcmp(file_type, "VK_DTW_MODEL_FILE") != 0)) {
    fwprintf(stderr, L"ERROR: Improper file type header information.\n\t%S\n", file_type);
    return (false);
  }
  // Constructing header information
  i  = snprintf( m_header, m_header_size, "%s\n", file_type );
  i += snprintf( m_header + i, m_header_size - i, "FEATURE_TYPE: %s\n", m_mapFeatureType[dtw_m_model.feature_type]);
  i += snprintf( m_header + i, m_header_size - i, "SAMPLE_TYPE: %s\n", m_mapDataType[dtw_m_model.sample_type]);
  i += snprintf( m_header + i, m_header_size - i, "NUM_ELEMENTS_PER_FEATURE: %d\n", dtw_m_model.num_features);
  i += snprintf( m_header + i, m_header_size - i, "NUM_FEATURE_VECTORS: %d\n", dtw_m_model.num_vectors);
  i += snprintf( m_header + i, m_header_size - i, "NUM_VECTORS_PER_SEC: %d\n", dtw_m_model.frame_rate);
  if (dtw_m_model.NUMD_FACTOR != 0)
    i += snprintf( m_header + i, m_header_size - i, "NUMD_FACTOR: %f\n", dtw_m_model.NUMD_FACTOR);
  if (dtw_m_model.NNZ_FACTOR != 0)
    i += snprintf( m_header + i, m_header_size - i, "NNZ_FACTOR: %f\n", dtw_m_model.NNZ_FACTOR);

  //printf_s( "Composed:\n%s\ncharacter count = %d\n", m_header, i );
  // This shoudl not be necessary but snprintf is inserting funny characters at the end of the buffer
  memset(m_header+i, 0, (m_header_size-i)*sizeof(char));
  // Make sure that the file pointer is SET at the begining of the file
  fseek(m_f, 0, SEEK_SET);
  retval = WriteBinary(m_header, sizeof(SCHAR), m_header_size);

  return (retval);
}

bool WUW_IO::Write_FEF_Header(SCHAR *file_type)
{
  UINT16 i;
  bool   retval;

  if ((strcmp(file_type, "VK_FEATURES_FILE") != 0) &&
      (strcmp(file_type, "VK_FEF_FILE") != 0)) {
    fwprintf(stderr, L"ERROR: Improper file type header information.\n\t%S\n", file_type);
    return (false);
  }
  // Constructing header information
  i  = snprintf( m_header, m_header_size, "%s\n", file_type );
  i += snprintf( m_header + i, m_header_size - i, "NUM_FEATURE_TYPES: %d\n", m_num_feature_types);
  i += snprintf( m_header + i, m_header_size - i, "FEATURE_TYPE: ");
  if (m_feature_type & FT_STD_MFCC)  // ETSI Standard MFCC's
     i += snprintf( m_header + i, m_header_size - i, "STD_MFCC ");
  if (m_feature_type & FT_ETSI_MFCC) // ETSI Based MFCC's
     i += snprintf( m_header + i, m_header_size - i, "ETSI_MFCC ");
  if (m_feature_type & FT_LPC_MFCC) // LPC Smoothed Specturm Based MFCC's
     i += snprintf( m_header + i, m_header_size - i, "LPC_MFCC ");
  if (m_feature_type & FT_VK_LPCENH) // 2006 Implementation
     i += snprintf( m_header + i, m_header_size - i, "VK_LPCENH ");
  if (m_feature_type & FT_VK_STDENH) // Standard 2006 Implementation
     i += snprintf( m_header + i, m_header_size - i, "VK_STDENH ");
  if (m_feature_type & FT_VK_LPC_ENH_MFCC) // 2006 Implementation LPC Smoothed Enhanced Spectrum MFCC
     i += snprintf( m_header + i, m_header_size - i, "VK_LPC_ENH_MFCC ");
  if (m_feature_type & FT_VK_FB_SPEC) // VK Dissertation Work - Not Implemented
     i += snprintf( m_header + i, m_header_size - i, "VK_FB_SPEC");
  i += snprintf( m_header + i, m_header_size - i, "\n");
  i += snprintf( m_header + i, m_header_size - i, "SAMPLE_TYPE: %s\n", m_mapDataType[m_sample_type]);
  // Allowing for different number of elements for each feature vector type
  i += snprintf( m_header + i, m_header_size - i, "NUM_ELEMENTS_PER_FEATURE: ");
  for (UINT16 j=0; j<m_num_feature_types-1; j++) {
     // Hard-coded Number of Features to (mp_num_features[j]-3) -- HMM's do not need c0 and logE
     i += snprintf( m_header + i, m_header_size - i, "%d, ", mp_num_features[j]-3);
  }
  // Hard-coded Number of Features to (mp_num_features[j]-3) -- HMM's do not need c0 and logE
  i += snprintf( m_header + i, m_header_size - i, "%d\n", mp_num_features[m_num_feature_types-1]-3);
  i += snprintf( m_header + i, m_header_size - i, "NUM_FEATURE_VECTORS: %d\n", m_num_vectors);
  i += snprintf( m_header + i, m_header_size - i, "NUM_VECTORS_PER_SEC: %d\n", m_frame_rate);

  //printf_s( "Composed:\n%s\ncharacter count = %d\n", m_header, i );
  // This shoudl not be necessary but snprintf is inserting funny character at the end of the buffer
  memset(m_header+i, 0, (m_header_size-i)*sizeof(char));
  // Make sure that the file pointer is SET at the begining of the file
  fseek(m_f, 0, SEEK_SET);
  retval = WriteBinary(m_header, sizeof(SCHAR), m_header_size);

  return (retval);
}

bool WUW_IO::Write_FEF_Header(SCHAR *file_type, SCHAR *header)
{
  UINT16 i;
  bool   retval;
  // Make sure that the file pointer is SET at the begining of the file
  fseek(m_f, 0, SEEK_SET);
  retval = WriteBinary(header, sizeof(SCHAR), m_header_size);

  return (retval);
}
bool WUW_IO::Write_DTW_m_Model()
{
  UINT16 i, j;

  if (Write_DTW_Header("VK_DTW_MODEL_FILE") == false)
     return (false);

  for (i=0; i<dtw_m_model.num_vectors; i++) {
    // dtw_m_model.dtw_m_model[i] = new FLOAT32 [dtw_m_model.num_features];
    // Need to overload this function for various data types of the model
    // for (j=0; j<dtw_m_model.num_features; j++) {

     FLOAT32 *tmp = (*(dtw_m_model.dtw_m_model))[i];

    if (WriteBinary(tmp, dtw_m_model.sample_size, dtw_m_model.num_features) == false)
       return (false);
  }
  return (true);
}


bool WUW_IO::Write_FEF(SEGMENT_FEATURES *seg_features)
{
  UINT16 i;

  m_num_vectors = seg_features->num_feature_vectors;
  if ((seg_features->fe_mfcc_features_buffer.size()     != m_num_vectors) ||
      (seg_features->fe_lpc_mfcc_features_buffer.size() != m_num_vectors) ||
      (seg_features->fe_enh_mfcc_features_buffer.size() != m_num_vectors)) {
         fwprintf(stderr, L"FATAL ERROR: Inconsistend size of the feature vectors in SEGMENT_FEATURES\n");
         fwprintf(stderr, L"\tMFCC number of vectors      : %d\n", seg_features->fe_mfcc_features_buffer.size());
         fwprintf(stderr, L"\tLPC MFCC number of vectors  : %d\n", seg_features->fe_lpc_mfcc_features_buffer.size());
         fwprintf(stderr, L"\tENH MFCC number of vectors  : %d\n", seg_features->fe_enh_mfcc_features_buffer.size());
         fwprintf(stderr, L"\tSEGMENT_FEATURES num_vectors: %d\n", m_num_vectors);

         return(false);
  }

  SetFEFInfo(seg_features);

  if (Write_FEF_Header("VK_FEF_FILE") == false) {
     fwprintf(stderr, L"ERROR: Unrecognized Header Info = \"VK_FEF_FILE\"\n");
     return (false);
  }
  // Writting Feature Vectors Consequetively
  for (i=0; i<m_num_vectors; i++) {
     // Skipping c0 feature
        //if (WriteBinary(seg_features->fe_mfcc_features_buffer[i], m_itemSize, seg_features->num_mfcc_features) == false)
        //   return (false);


     if (WriteBinary(seg_features->fe_mfcc_features_buffer[i], m_itemSize, seg_features->num_mfcc_features/3-1) == false)
        return (false);
     if (WriteBinary(&(seg_features->fe_mfcc_features_buffer[i])[seg_features->num_mfcc_features/3], m_itemSize, seg_features->num_mfcc_features/3-1) == false)
        return (false);
     if (WriteBinary(&(seg_features->fe_mfcc_features_buffer[i])[2*seg_features->num_mfcc_features/3], m_itemSize, seg_features->num_mfcc_features/3-1) == false)
        return (false);


        //for (int j=0; j<seg_features->num_mfcc_features; j++) {
        //   fwprintf(stderr, L"[%2d] %f\n", j, (seg_features->fe_mfcc_features_buffer[i])[j]);
        //}
        //if (WriteBinary(seg_features->fe_lpc_mfcc_features_buffer[i], m_itemSize, seg_features->num_mfcc_features) == false)
        //   return (false);

     if (WriteBinary(seg_features->fe_lpc_mfcc_features_buffer[i], m_itemSize, seg_features->num_mfcc_features/3-1) == false)
        return (false);
     if (WriteBinary(&(seg_features->fe_lpc_mfcc_features_buffer[i])[seg_features->num_mfcc_features/3], m_itemSize, seg_features->num_mfcc_features/3-1) == false)
        return (false);
     if (WriteBinary(&(seg_features->fe_lpc_mfcc_features_buffer[i])[2*seg_features->num_mfcc_features/3], m_itemSize, seg_features->num_mfcc_features/3-1) == false)
        return (false);

        //if (WriteBinary(seg_features->fe_enh_mfcc_features_buffer[i], m_itemSize, seg_features->num_enh_features) == false)
        //   return (false);
     if (WriteBinary(seg_features->fe_enh_mfcc_features_buffer[i], m_itemSize, seg_features->num_enh_features/3-1) == false)
        return (false);
     if (WriteBinary(&(seg_features->fe_enh_mfcc_features_buffer[i])[seg_features->num_enh_features/3], m_itemSize, seg_features->num_enh_features/3-1) == false)
        return (false);
     if (WriteBinary(&(seg_features->fe_enh_mfcc_features_buffer[i])[2*seg_features->num_enh_features/3], m_itemSize, seg_features->num_enh_features/3-1) == false)
        return (false);
  }

  if(m_f != NULL)
     fclose(m_f);

   // This is done in BackEnd
  //FreeMemory(seg_features);

  return (true);

}

bool WUW_IO::Write_Feature_Vector(const void *p)
{
  // It is assumend that element size and count has been already set
  return(WriteBinary(p));
}

bool WUW_IO::Write_Feature_Vector(const void *p, size_t size, size_t count)
{
  return(WriteBinary(p, size, count));
}
//
// Read Member Functions
//
bool WUW_IO::ReadBinary(void* p, size_t size, size_t count)
{
  size_t wcount;

  if(!m_f || !m_fBinary || !p)
    return (false);

  if ((wcount = fread(p, size, count, m_f)) != count) {
    perror("SYSMSG:");
    fwprintf(stderr, L"ERROR: %s\nRading from file:\n\t%s\nExpected %d items, read %d\n",
	    strerror( errno ), m_filename, count, wcount);
    return(false);
  }
  return(true);
}

bool WUW_IO::ReadBinary(void* p)
{
  size_t wcount;

  if(!m_f || !m_fBinary || !p)
    return (false);

  if ((wcount = fread(p, m_itemSize, m_itemCount, m_f)) != m_itemSize) {
    perror("SYSMSG:");
    fwprintf(stderr, L"ERROR: %s\nRading from file:\n\t%s\nExpected %d items, read %d\n",
	    strerror( errno ), m_filename, m_itemCount, wcount);
    return(false);
  }
  return(true);
}
//
// DTW Header interpreter
//
bool WUW_IO::Read_DTW_Header ()
{
  CHAR  delim[] = " ,\t\n";
  CHAR *token;

  dtw_m_model.NUMD_FACTOR = 0.0F;
  dtw_m_model.NNZ_FACTOR  = 0.0F;

  ReadBinary(m_header, sizeof(CHAR), m_header_size);

  token = strtok(m_header, delim);

  if ((strcmp(token, "VK_FEATURES_FILE") != 0) &&
      (strcmp(token, "VK_DTW_MODEL_FILE") != 0)) {
    fwprintf(stderr, L"ERROR: Not a VK_DTW_MODEL_FILE or VK_FEATURES_FILE.\n\ttoken:%S\n", m_header);
    return (false);
  }

  token = strtok(NULL, delim);

  while (token != NULL) {

    if (strcmp(token, "FEATURE_TYPE:") == 0) {
       token = strtok(NULL, delim);
      if (strcmp(token, "ETSI_MFCC") == 0) {  // OLD VERSION
         dtw_m_model.feature_type = FT_ETSI_MFCC;
      }
      else if (strcmp(token, "FT_STD_MFCC") == 0) {  // OLD VERSION
         dtw_m_model.feature_type = FT_STD_MFCC;
      }
      else if ((strcmp(token, "FT_LPC_MFCC") == 0) || (strcmp(token, "LPC_MFCC") == 0)) {  // NEW VERSION
         dtw_m_model.feature_type = FT_LPC_MFCC;
      }
      else if (strcmp(token, "FT_VK_LPC_ENH_MFCC") == 0) {  // NEW VERSION
         dtw_m_model.feature_type = FT_VK_LPC_ENH_MFCC;
      }
      else {
         fwprintf(stderr, L"ERROR: Not Supported Feature Type:\n\tTOKEN: %S\n", token);
         return (false);
      }
    }

    if (strcmp(token, "SAMPLE_TYPE:") == 0) {
      token = strtok(NULL, delim);
      if (strcmp(token, "float") == 0) {
         dtw_m_model.sample_type = ST_FLOAT32;
         dtw_m_model.sample_size = sizeof(FLOAT32);
      }
      else {
         fwprintf(stderr, L"ERROR: Not Supported Sample Type:\n\tTOKEN: %S\n", token);
         return (false);
      }
    }

    if (strcmp(token,  "NUM_ELEMENTS_PER_FEATURE:") == 0) {
      token = strtok(NULL, delim);
      dtw_m_model.num_features = atoi(token);
      if (dtw_m_model.num_features < 1) {
         fwprintf(stderr, L"ERROR: Non Pozitive Number of Features in a Feature Vector:\n\tTOKEN: %S => %d\n",
            token, dtw_m_model.num_features);
         return (false);
      }
    }

    if (strcmp(token,  "NUM_FEATURE_VECTORS:") == 0) {
      token = strtok(NULL, delim);
      dtw_m_model.num_vectors = atoi(token);
      if (dtw_m_model.num_vectors < 1) {
         fwprintf(stderr, L"ERROR: Non Pozitive Number of Feature Vectors:\n\tTOKEN: %S => %d\n",
            token, dtw_m_model.num_vectors);
         return (false);
         }
       }

    if (strcmp(token,  "NUM_VECTORS_PER_SEC:") == 0) {
       token = strtok(NULL, delim);
       dtw_m_model.frame_rate = atoi(token);
       if ((dtw_m_model.frame_rate !=  20) && (dtw_m_model.frame_rate !=  50) &&
           (dtw_m_model.frame_rate != 100) && (dtw_m_model.frame_rate != 200)) {
              fwprintf(stderr, L"ERROR: Not Supported Frame Rate:\n\tTOKEN: %S => %d\n",
                 token, dtw_m_model.frame_rate);
              return (false);
       }
    }

    if (strcmp(token,  "NUMD_FACTOR:") == 0) {
      token = strtok(NULL, delim);
      dtw_m_model.NUMD_FACTOR = (FLOAT32) atof(token);
      if ((dtw_m_model.NUMD_FACTOR > 1.0F) || (dtw_m_model.NUMD_FACTOR < -1.0F)) {
         fwprintf(stderr, L"WARNING: Possibly Incorrect NUMD_FACTOR:\n\tTOKEN: %S => %f\n",
            token, dtw_m_model.NUMD_FACTOR);
      }
    }

    if (strcmp(token,  "NNZ_FACTOR:") == 0) {
      token = strtok(NULL, delim);
      dtw_m_model.NNZ_FACTOR = atof(token);
      if ((dtw_m_model.NNZ_FACTOR > 1.0F) || (dtw_m_model.NNZ_FACTOR < -1.0F)) {
         fwprintf(stderr, L"WARNING: Possibly Incorrect NNZ_FACTOR:\n\tTOKEN: %S => %f\n",
            token, dtw_m_model.NNZ_FACTOR);
      }
    }

    token = strtok(NULL, delim);

  }
  return(true);
}
//
// FEF Header interpreter
//
bool WUW_IO::Read_FEF_Header ()
{
  CHAR  delim[] = " ,\t\n";
  CHAR *token;

  ReadBinary(m_header, sizeof(CHAR), m_header_size);

  token = strtok(m_header, delim);

  if ((strcmp(token, "VK_FEATURES_FILE") != 0) &&
      (strcmp(token, "VK_FEF_FILE") != 0)) {
    fwprintf(stderr, L"ERROR: Not a VK_FEF_FILE or VK_FEATURES_FILE.\n\ttoken:%S\n", m_header);
    return (false);
  }

  token = strtok(NULL, delim);

   //VK_FEF_FILE
   //NUM_FEATURE_TYPES: 2
   //FEATURE_TYPE: ETSI_MFCC VK_LPC_ENH_MFCC
   //SAMPLE_TYPE: float
   //NUM_ELEMENTS_PER_FEATURE: 42, 42
   //NUM_FEATURE_VECTORS: 125
   //NUM_VECTORS_PER_SEC: 200

  while (token != NULL) {

    if (strcmp(token, "NUM_FEATURE_TYPES:") == 0) {
       token = strtok(NULL, delim);
       m_num_feature_types = atoi(token);
       //mp_num_features     = new size_t [m_num_feature_types];
    }

    // Must check all feature types
    if (strcmp(token, "FEATURE_TYPE:") == 0) {
       token = strtok(NULL, delim);
       if (strcmp(token, "ETSI_MFCC") == 0) {  // OLD ETSI VERSION
          m_feature_type = FT_ETSI_MFCC;
       }
       else if (strcmp(token, "FT_STD_MFCC") == 0) {  // STD ETSI MFCC VERSION
          m_feature_type = FT_STD_MFCC;
       }
       else if (strcmp(token, "FT_LPC_MFCC") == 0) {  // NEW VERSION
          m_feature_type = FT_LPC_MFCC;
       }
       else if (strcmp(token, "FT_VK_LPC_ENH_MFCC") == 0) {  // NEW VERSION
          m_feature_type = FT_VK_LPC_ENH_MFCC;
       }
       else {
          fwprintf(stderr, L"ERROR: Not Supported Feature Type:\n\tTOKEN: %S\n", token);
          return (false);
       }
    }

    if (strcmp(token, "SAMPLE_TYPE:") == 0) {
      token = strtok(NULL, delim);
      if (strcmp(token, "float") == 0) {
         m_sample_type = ST_FLOAT32;
         m_sample_size = sizeof(FLOAT32);
      }
      else {
         fwprintf(stderr, L"ERROR: Not Supported Sample Type:\n\tTOKEN: %S\n", token);
         return (false);
      }
    }
   // Must check for all feature types included in the segment
    if (strcmp(token,  "NUM_ELEMENTS_PER_FEATURE:") == 0) {
      token = strtok(NULL, delim);
      m_num_features = atoi(token);
      if (m_num_features < 1) {
         fwprintf(stderr, L"ERROR: Non Pozitive Number of Features in a Feature Vector:\n\tTOKEN: %S => %d\n",
            token, m_num_features);
         return (false);
      }
    }

    if (strcmp(token,  "NUM_FEATURE_VECTORS:") == 0) {
      token = strtok(NULL, delim);
      m_num_vectors = atoi(token);
      if (m_num_vectors < 1) {
         fwprintf(stderr, L"ERROR: Non Pozitive Number of Feature Vectors:\n\tTOKEN: %S => %d\n",
            token, m_num_vectors);
         return (false);
         }
       }

    if (strcmp(token,  "NUM_VECTORS_PER_SEC:") == 0) {
       token = strtok(NULL, delim);
       m_frame_rate = atoi(token);
       if ((m_frame_rate !=  20) && (m_frame_rate !=  50) &&
           (m_frame_rate != 100) && (m_frame_rate != 200)) {
              fwprintf(stderr, L"ERROR: Not Supported Frame Rate:\n\tTOKEN: %S => %d\n",
                 token, m_frame_rate);
              return (false);
       }
    }

    token = strtok(NULL, delim);
  }

  return(true);
}
//
//
//
size_t WUW_IO::get_DTW_HeaderSize()
{
  return(m_header_size);
}
//
//
//
DTW_m_MODEL *WUW_IO::load_DTW_Model()
{
   UINT16 i;

   if (Read_DTW_Header() == false) {
      fwprintf(stderr, L"ERROR: Model Header:\n\tFILE: %s\n", m_filename);
      return (NULL);
   }
   //
   // Allocating Memory Space for Model
   //
   Read_DTW_m_Model();

   return(&dtw_m_model);
}

void WUW_IO::Read_DTW_m_Model()
{
   UINT16 i;
   UINT16 total_num_elements = dtw_m_model.num_vectors*dtw_m_model.num_features;

   if (buffer != NULL) delete [] buffer;
   buffer             = new FLOAT32 [total_num_elements];
   ReadBinary(buffer, dtw_m_model.sample_size, total_num_elements);
   // Allocating the vector structure of pointers to FLOAT32
   dtw_m_model.dtw_m_model = new vector<FLOAT32 *> [dtw_m_model.num_vectors];
   // Setting up the pointers
   for (i=0; i<dtw_m_model.num_vectors; i++) {
      dtw_m_model.dtw_m_model->push_back(&buffer[i*dtw_m_model.num_features]);
   }
}
//
//
//
SEGMENT_FEATURES *WUW_IO::load_FEF()
{
  UINT16 i;

  SetFEFInfo();

  if (Read_FEF_Header() == false) {
    fwprintf(stderr, L"ERROR: FEF Header:\n\tFILE: %s\n", m_filename);
    return (NULL);
  }
  //
  // Allocating Memory Space for FEF
  //
  if (Read_FEF_Data() == false) {
     // Print Error Message
     return(NULL);
  }
  return(m_seg_features);
}

bool WUW_IO::Read_FEF_Data()
{
   UINT16 i, j, total_num_elements;

   // Must make this function generic to automatically use data type
   // during run-time insted fixed to FLOAT32
   m_seg_features = new SEGMENT_FEATURES;
   m_seg_features->num_feature_vectors = m_num_vectors;
   m_seg_features->num_features        = m_num_features;
   m_seg_features->num_feature_types   = m_num_feature_types;
   m_seg_features->frame_rate          = m_frame_rate;
   m_seg_features->num_mfcc_features   = m_num_features;
   m_seg_features->num_enh_features    = m_num_features;
   total_num_elements = m_num_feature_types*m_num_vectors*m_num_features;
   // Allocating the whole FEF buffer space
   if (buffer != NULL) delete [] buffer;
   buffer             = new FLOAT32 [total_num_elements];
   ReadBinary(buffer, sizeof(FLOAT32), total_num_elements);
   //
   // Allocate Buffer of Pointer that point at each vector
   //
   //m_seg_features->fe_mfcc_features_buffer     = new FLOAT32 * [m_num_vectors];
   //m_seg_features->fe_enh_mfcc_features_buffer = new FLOAT32 * [m_num_vectors];
   //
   // Assuming same number of elemente per feature vector type
   //
   for (i=0, j=0; i<m_num_vectors; i++, j+=2) {
      // Setting up pointers of the vector<FLOAT32 *> to loaded features.
      m_seg_features->fe_mfcc_features_buffer.push_back(&buffer[j*m_num_features]);
      m_seg_features->fe_enh_mfcc_features_buffer.push_back(&buffer[(j+1)*m_num_features]);
   }

   return(true);
}
//
// Read List File
//
vector<string> * WUW_IO::ReadListFile()
{
   char          c_buffer[_MAX_FNAME];
   char          *c_ptr;
   char          *state;
   char seps[] = " ,\t\n";

   m_list.clear();

   c_ptr = fgets(c_buffer, _MAX_FNAME, m_f);
   if(c_ptr != NULL) {
      // Parsing input line
      c_ptr = strtok(c_buffer, seps);
      while ( c_ptr != NULL) {
         string s = c_ptr;
         //s.assign(c_ptr);
         m_list.push_back(s);
         c_ptr = strtok(NULL, seps);
      }
      return(&m_list);
   }
   else {
      m_list.clear();

      return(NULL);
   }
}
/*******************************  End of File  *******************************/

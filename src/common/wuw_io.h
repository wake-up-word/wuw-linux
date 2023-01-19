/*****************************************************************************

                     Copyright (C) 2006-2015 VoiceKey Inc.
                          Cocoa Beach, Florida 32931
                              All rights reserved

                         Confidential and Proprietary

     Module: wuw_io.h
     Author: Veton Kepuska
    Created: September 17, 2006
    Updated:

Description: WUW IO functions.

$Log:$

*******************************************************************************

Notes:

******************************************************************************/

#ifndef WUW_IO_H
#define WUW_IO_H

/******************************  Include Files  ******************************/

#include <map>
#include <vector>
#include <string>
#include <string.h>
#include <algorithm>
#include <filesystem>
using namespace std;

#include "defines.h" // For BACK_END_TYPE and win32types

/*********************************  Defines  *********************************/

struct ModelFile
{
  FILE*      file;
  size_t     bytes;
  void*      pvBuffer;
  ModelFile* next;
};

/****************************  Class Definition  *****************************/

class WUW_IO
{
public:
  WUW_IO(const char* filename, bool is_binary, const char *mode);
  WUW_IO(const char* filename, bool is_binary, const char *mode, BACK_END_TYPE be_type);

  WUW_IO(const char* filename, size_t itemSize, size_t itemCount, const char *mode);
  WUW_IO(const char* filename, size_t itemSize, size_t itemCount, const char *mode, BACK_END_TYPE be_type);

  WUW_IO(const char* filename, size_t itemSize, size_t itemCount, const char *mode, void** ptrItem);
  WUW_IO(const char* filename, size_t itemSize, size_t itemCount, const char *mode, void** ptrItem, BACK_END_TYPE be_type);
  WUW_IO(const char* filename, size_t itemSize, size_t itemCount, size_t numItems, size_t frameRate, const char *mode, vector<FLOAT32*>* ptrItem, BACK_END_TYPE be_type);

  ~WUW_IO();

  void Set_WUW_IO(const char* filename, bool is_binary, const char *mode);
  void Set_WUW_IO(const char* filename, size_t itemSize, size_t itemCount, const char *mode);
  void Set_WUW_IO(const char* filename, size_t itemSize, size_t itemCount, const char *mode, void** ptrItem);
  void Set_WUW_IO(const char* filename, size_t itemSize, size_t itemCount, size_t numItems, const char *mode, vector<FLOAT32 *>* ptrItem);

  bool Open(const char *mode);
  void InitTypes();

  void SetFileInfo();
  void SetDataType(SAMPLE_TYPE sample_type, FEATURE_TYPE feature_type);

  void SetModelInfo();
  void SetFEFInfo();
  void SetFEFInfo(SEGMENT_FEATURES *fef_segment);
  void SetFEFInfo(FE_FEATURES *fef_packet, UINT16 num_vecs, UINT16 frame_rate);
  void SetFEFInfo(UINT16 num_vecs, UINT16 num_mfcc_features, UINT16 num_enh_features, UINT16 frame_rate);

  bool WriteBinary(const void* p); // This can be used if size parameters are constant and set by the constructor
  bool ReadBinary(void* p);

  bool WriteBinary(const void* p, size_t size, size_t count);
  bool ReadBinary(void* p, size_t size, size_t count);

  bool Printf(const wchar_t *str, ...);

  bool Write_DTW_Header(SCHAR *header);
  bool Read_DTW_Header();
  size_t get_DTW_HeaderSize();

  bool Write_FEF_Header(SCHAR *file_type);
  bool Write_FEF_Header(SCHAR *file_type, SCHAR *header);
  bool Read_FEF_Header();
  size_t get_FEF_HeaderSize();

  bool Write_DTW_m_Model();

  bool Write_Feature_Vector(const void *p);
  bool Write_Feature_Vector(const void *p, size_t size, size_t count);

  DTW_m_MODEL *load_DTW_Model();
  DTW_m_Model *get_DTW_Model();

  void         Read_DTW_m_Model();
  bool         Write_DTW_m_Model(SEGMENT_FEATURES *seg_features);

  SEGMENT_FEATURES * load_FEF();
  bool               Read_FEF_Data();
  bool               Write_FEF(SEGMENT_FEATURES *seg_features);

  vector<string>   *ReadListFile();

  //bool FreeMemory(SEGMENT_FEATURES *seg_features);

private:
  FILE*    m_f;
  char* m_filename;
  bool     m_fBinary;
  bool     m_is_model;
  size_t   m_itemSize, m_sample_size;
  size_t   m_itemCount;
  size_t   m_header_size;
  char    *m_header;

  void**   m_ppvItem;
  vector<FLOAT32*> * m_pvItem;

  UINT16   m_num_feature_types;  // number of different feature types
  UINT16   m_num_vectors;        // total number of vectors in a segment -- must be same for all feature types
  UINT16   m_frame_rate;         // frame rate in number of vectors per second -- must be same for all feature types
  UINT16   m_num_features;

  //vector<UINT16> mv_num_elems;
  UINT16 *  mp_num_features;      // number of elements in a feature vector

  SAMPLE_TYPE  m_sample_type;
  //FEATURE_TYPE m_feature_type;
  UINT16       m_feature_type;   // Encoded information of computed feature types

  FLOAT32 *    buffer;           // Storage Space for Features

  SEGMENT_FEATURES *m_seg_features;

  BACK_END_TYPE model_type;
  DTW_m_MODEL   dtw_m_model;

  map<enum SAMPLE_TYPE, char *> m_mapDataType;
  map<enum FEATURE_TYPE, char *> m_mapFeatureType;

  vector<string> m_list;
};

#endif // WUW_IO_H

/*******************************  End of File  *******************************/

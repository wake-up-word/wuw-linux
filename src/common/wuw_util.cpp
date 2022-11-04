#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <cerrno>
#include <wchar.h>

#include "wuw_util.h"

WuwLogger::WuwLogger(const char* filename, bool is_binary)
{
  m_filename = strdup(filename);

  if(is_binary)
    m_f = fopen(filename, "wb");
  else
    m_f = fopen(filename, "w");

  fBinary     = is_binary;
  m_itemSize  = 0;
  m_itemCount = 0;
  m_ppvItem   = NULL;

  if(!m_f)
    printf("ERROR: %s Opening file for writing: %S\n", strerror( errno ), filename);
}

WuwLogger::WuwLogger(const char* filename, size_t itemSize, size_t itemCount)
{
  m_filename = strdup(filename);
  fBinary = true;
  m_itemSize = itemSize;
  m_itemCount = itemCount;

  m_f = fopen(filename, "wb");

  if(!m_f)
    printf("ERROR: %s Opening file for writing: %S\n", strerror( errno ), filename);
}

WuwLogger::WuwLogger(const char* filename, size_t itemSize, size_t itemCount, void** ptrItem)
{
  m_filename = strdup(filename);
  fBinary = true;
  m_itemSize = itemSize;
  m_itemCount = itemCount;
  m_ppvItem = ptrItem;
  m_f = fopen(filename, "wb");

  if(!m_f)
    printf("ERROR: %s Opening file for writing: %S\n", strerror( errno ), filename);
}

WuwLogger::~WuwLogger()
{
  if(m_f != NULL)
    fclose(m_f);
}

void WuwLogger::WriteBinary(const void* p, size_t size, size_t count)
{
  size_t wcount;

  if(!m_f || !fBinary || !p)
    return;

  if ((wcount = fwrite(p, size, count, m_f)) != count) {
    perror("SYSMSG:");
    printf("ERROR: Writting to file:\n%S\nExpected %d items, written %d\n",
	   m_filename, count, wcount);
  }

}

void WuwLogger::WriteBinary(const void* p)
{
  if(!m_f || !fBinary || !p)
    return;

  fwrite(p, m_itemSize, m_itemCount, m_f);
}
void WuwLogger::Printf(const wchar_t* format, ...)
{
  if(!m_f || fBinary)
    return;

  va_list args;
  va_start(args, format);
  if (!vfwprintf(m_f, format, args))
    printf("ERROR: %s Writting file for writing: %S\n", strerror( errno ), m_filename);

  va_end(args);
}

void WuwLogger::Run()
{
  if(m_ppvItem && m_f)
    fwrite(*m_ppvItem, m_itemSize, m_itemCount, m_f);
}

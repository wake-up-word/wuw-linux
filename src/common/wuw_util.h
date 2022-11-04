#ifndef WUW_UTIL_H
#define WUW_UTIL_H

struct LogFile
{
	FILE*  file;
	size_t bytes;
	void*  pvBuffer;
	LogFile* next;
};

class WuwLogger
{
public:
	WuwLogger(const char* filename, bool is_binary);
	WuwLogger(const char* filename, size_t itemSize, size_t itemCount);
	WuwLogger(const char* filename, size_t itemSize, size_t itemCount, void** ptrItem);

	~WuwLogger();

	void WriteBinary(const void* p);
	void WriteBinary(const void* p, size_t size, size_t count);
	void Printf(const wchar_t *str, ...);
	void Run();

private:
	FILE*    m_f;
	char* m_filename;
	bool     fBinary;
	size_t   m_itemSize;
	size_t   m_itemCount;
	void**   m_ppvItem;

};

#endif
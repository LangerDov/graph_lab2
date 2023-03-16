#pragma once

#include "filedefs.h"
#include "str.h"


namespace FILES
{
	class File
	{
	public:
		File();
		File(UString path, int mode, int file_openning_attr);
		bool FileIsOpenned();
		UString GetPath();
		bool Open(UString path, int mode, int file_openning_attr);
		bool Close();
		size_t Write(void* buffer, size_t bytes);
        size_t Write(void* buffer, size_t bytes, size_t pos);
        size_t WriteToEnd(void* buffer, size_t bytes);
		bool SetPos(size_t pos);
		size_t GetPos();
        size_t Read(void* buffer, size_t bytes);
        size_t Read(void* buffer, size_t bytes, size_t pos);
		bool RemoveRegion(size_t start, size_t end);
		bool Insert(void* buffer, size_t size);
		bool Insert(void* buffer, size_t size, size_t pos);
		size_t GetSize();
		~File();

	private:
		void* file;
		UString path;
	};
	size_t GetSize(UString path);
	bool Delete(UString path);
	bool Move(UString origin, UString dest);
	bool Copy(UString origin, UString dest);
	bool PathExist(UString path);
	bool CreateDir(UString path);
	bool RemoveFile(UString path);
	UString GetAppDir();
	UString GetUserHomeDir();
}
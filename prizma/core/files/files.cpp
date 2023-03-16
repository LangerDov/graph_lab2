#include "files.h"

#include "list.h"


#include "debugging/log.h"

#ifdef _WIN32

#include <Windows.h>


wchar_t* convert_to_wstring(const char* str)
{
    int str_len = (int)strlen(str);
    int num_chars = MultiByteToWideChar(CP_UTF8, 0, str, str_len, NULL, 0);
    wchar_t* wstrTo = (wchar_t*)malloc((num_chars + 1) * sizeof(wchar_t));
    if (wstrTo)
    {
        MultiByteToWideChar(CP_UTF8, 0, str, str_len, wstrTo, num_chars);
        wstrTo[num_chars] = L'\0';
    }
    return wstrTo;
}

char* convert_from_wstring(const wchar_t* wstr)
{
    int wstr_len = (int)wcslen(wstr);
    int num_chars = WideCharToMultiByte(CP_UTF8, 0, wstr, wstr_len, NULL, 0, NULL, NULL);
    char* strTo = (char*)malloc((num_chars + 1) * sizeof(char));
    if (strTo)
    {
        WideCharToMultiByte(CP_UTF8, 0, wstr, wstr_len, strTo, num_chars, NULL, NULL);
        strTo[num_chars] = '\0';
    }
    delete[]wstr;
    return strTo;
}

FILES::File::File()
{
    file = NULL;
}

FILES::File::File(UString path, int mode, int file_openning_attr)
{
    file = NULL;
    Open(path, mode, file_openning_attr);
    Log(lDebug, "GetLastError %d",GetLastError());
}

bool FILES::File::FileIsOpenned()
{
    return file;
}

UString FILES::File::GetPath()
{
    return path;
}

bool FILES::File::Open(UString path, int mode, int file_openning_attr)
{
    if (file)
        return false;
    wchar_t* file_name = convert_to_wstring(path.Str());
    DWORD Access = 0;
    if (mode & write_)
        Access = Access | GENERIC_WRITE;
    if (mode & read_)
        Access = Access | GENERIC_READ;
    if (!Access)
        return false;
    DWORD crt_disp = 0;
    switch (file_openning_attr)
    {
    case open_exist:
        crt_disp = OPEN_EXISTING;
        break;
    case always_open:
        crt_disp = OPEN_ALWAYS;
        break;
    case open_new:
        crt_disp = CREATE_NEW;
        break;
    default:
        return false;
        break;
    }
    HANDLE f = CreateFileW(file_name, Access, FILE_SHARE_READ, NULL, crt_disp, NULL, NULL);
    if (!f)
        return false;
    this->path = path.Str();
    file = f;
    return true;
}

bool FILES::File::Close()
{
    if (file)
    {
        bool ret = CloseHandle(file);
        file = 0;
        return ret;
    }
    return false;
}

size_t FILES::File::Write(void* buffer, size_t size)
{
    return WriteFile(file, buffer, size, NULL, NULL);
}

size_t FILES::File::Write(void* buffer, size_t size, size_t pos)
{
    SetFilePointer(file, pos, NULL, pos);
    return WriteFile(file, buffer, size, NULL, NULL);
}

size_t FILES::File::WriteToEnd(void* buffer, size_t bytes)
{
    if (!file)
        return false;
    DWORD fend2;
    DWORD fend = GetFileSize(file, &fend2);
    DWORD fsize = fend | fend2;
    SetFilePointer(file, NULL, NULL, FILE_END);
    return WriteFile(file, buffer, bytes, &fsize, nullptr);
}

bool FILES::File::SetPos(size_t pos)
{
    if (file)
    {
        LARGE_INTEGER ps;
        ps.QuadPart = pos;
        return (SetFilePointer(file, ps.LowPart, &ps.HighPart, FILE_BEGIN) != INVALID_SET_FILE_POINTER);
    }
    return false;
}

size_t FILES::File::GetPos()
{
    LARGE_INTEGER Offset{ 0 };
    if (SetFilePointerEx(file, Offset, &Offset, FILE_CURRENT))
        return Offset.QuadPart;
    else
        return 0;
}

size_t FILES::File::GetSize()
{
    if (file)
    {
        DWORD start, end;
        start = GetFileSize(file, &end);
        return unsigned long long(start) | ((unsigned long long)end << 32);
    }
    return 0;
}

FILES::File::~File()
{
    if (file)
        CloseHandle(file);
}

size_t FILES::File::Read(void* buffer, size_t bytes)
{
    return ReadFile(file, buffer, bytes, NULL, NULL);
}

size_t FILES::File::Read(void* buffer, size_t bytes, size_t pos)
{
    SetFilePointer(file, pos, NULL, FILE_BEGIN);
    DWORD readed = 0;
    bool res = ReadFile(file, buffer, bytes, &readed, NULL);
    return res;
}

size_t FILES::GetSize(UString path)
{
    wchar_t* file_name = convert_to_wstring(path.Str());
    HANDLE hFile = CreateFileW(file_name, GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL);
    LARGE_INTEGER fileSize = { 0 };
    if (hFile)
    {

        GetFileSizeEx(hFile, &fileSize);
        CloseHandle(hFile);
    }
    delete[]file_name;
    return fileSize.QuadPart;
}


bool FILES::Delete(UString path)
{
    return false;
}

bool FILES::Move(UString from, UString dest)
{
    return false;
}

bool FILES::Copy(UString origin, UString dest)
{
    return false;
}

bool FILES::PathExist(UString path)
{
    wchar_t* name = convert_to_wstring(path.Str());
    DWORD dwAttrib = GetFileAttributesW(name);

    bool res = dwAttrib != INVALID_FILE_ATTRIBUTES;
    free(name);
    return res;
}

bool FILES::CreateDir(UString name)
{
    wchar_t* wname = convert_to_wstring(name.Str());
    bool res = CreateDirectoryW(wname, NULL);
    delete[]wname;
    return res;
}

bool FILES::RemoveFile(UString path)
{
    wchar_t* wname = convert_to_wstring(path.Str());
    bool res = DeleteFileW(wname);
    free(wname);
    return res;
}

UString FILES::GetAppDir()
{
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    return UString(convert_from_wstring(buffer));
}

UString FILES::GetUserHomeDir()
{
    return UString();
}


#elif __linux

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>

FILES::File::File()
{
    file = NULL;
}

FILES::File::File(UString path, int mode, int file_openning_attr)
{
    file = NULL;
    Open(path, mode, file_openning_attr);
}

bool FILES::File::FileIsOpenned()
{
    return file;
}

UString FILES::File::GetPath()
{
    return path;
}

bool FILES::File::Open(UString path, int mode, int file_openning_attr)
{
    if (file)
        return false;
    int flag = 0;
    if (mode & write_)
        flag = flag | O_WRONLY;
    if (mode & read_)
        flag = flag | O_RDONLY;
    switch (file_openning_attr)
    {
        case open_exist:
            break;
        case always_open:
            if (access(path.Str(),0))
            {
                flag = flag | O_CREAT;
            }
            break;
        case open_new:
            flag = flag | O_CREAT;
            break;
        default:
            return false;
            break;
    }
    int f = open(path.Str(),flag);
    if (f == -1)
        return false;
    this->path = path.Str();
    file = (void*)f;
    return true;
}

bool FILES::File::Close()
{
    if (file)
    {
        if (close((int)file) == -1)
            return false;
        file = 0;
        return true;
    }
    return false;
}

size_t FILES::File::Write(void* buffer, size_t size)
{
    size_t res = write(file, buffer, size);
    if (res == -1)
        return 0;
    return res;
}

size_t FILES::File::Write(void* buffer, size_t size, size_t pos)
{
    if (SetPos(pos)) {
        size_t res = write(file, buffer, size);
        if (res != -1)
            return res;
    }
    return 0;
}

size_t FILES::File::WriteToEnd(void* buffer, size_t bytes)
{
    if (!file)
        return 0;
    lseek64((int)file,0,SEEK_END);
    size_t res = write(file, buffer, bytes);
    if (res == -1)
        return 0;
    return res;
}

bool FILES::File::SetPos(size_t pos)
{
    if (file)
        return lseek64((int)file,pos,SEEK_SET) != -1;
    return false;
}

size_t FILES::File::GetPos()
{
    if (file)
        return lseek64((int) file, 0, SEEK_CUR) != -1;
    return 0;
}

size_t FILES::File::GetSize()
{
    if (file)
    {
        int cur_pos = GetPos();
        size_t res = lseek64((int)file,0,SEEK_END);
        if (SetPos(cur_pos) and res != -1)
        {
            return res;
        }
    }
    return 0;
}

FILES::File::~File()
{
    if (file)
        close((int)file);
}

size_t FILES::File::Read(void* buffer, size_t bytes)
{
    if (file)
        return read(file, buffer, bytes) != -1;
    return 0;
}

size_t FILES::File::Read(void* buffer, size_t bytes, size_t pos)
{
    if (SetPos(pos))
        return 0;
    return Read(buffer,bytes);
}

size_t FILES::GetSize(UString path)
{
    return 0;
}


bool FILES::Delete(UString path)
{
    return false;
}

bool FILES::Move(UString from, UString dest)
{
    return false;
}

bool FILES::Copy(UString origin, UString dest)
{
    return false;
}

bool FILES::PathExist(UString path)
{
    return stat(path.Str(),0) != -1;
}

bool FILES::CreateDir(UString path)
{
    if (!PathExist(path))
        return mkdir(path.Str(),0700) != -1;
    return false;
}

bool FILES::RemoveFile(UString path)
{
    if (PathExist(path))
        return remove(path.Str()) != -1;
    return false;
}

UString FILES::GetAppDir()
{
    return "";
}

UString FILES::GetUserHomeDir()
{
    return UString();
}
#endif


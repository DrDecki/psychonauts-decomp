// Win32 file implementation; class names are confirmed by RTTI.
#include "engine.h"
#include "diagnostic.h"
#include "win32.h"

char *CopyFileName(const char *s, int category);
void FreeFileName(char *block, int category, int flags);
extern unsigned int g_win32FileCount; // 0x00795a30; retain original storage

void DiagnosticFile::Close()
{
    delete this;
}

class EWin32File : public DiagnosticFile {
public:
    void *operator new(size_t size) { return _aligned_malloc(size, 16); }
    EWin32File(HANDLE handle, const char *name);
    ~EWin32File();
    unsigned int Read(void *data, unsigned int size);
    unsigned int Write(const void *data, unsigned int size);
    unsigned int Seek(int offset, int origin);
    unsigned int Size();
    __int64 Vf18();
    bool Vf1c(unsigned int low, unsigned int high);

    HANDLE m_handle; // +8
};

class EWin32StreamingFile : public EWin32File {
public:
    EWin32StreamingFile(HANDLE handle, const char *name, int blockSize, int count);
    ~EWin32StreamingFile();
    unsigned int Read(void *data, unsigned int size);
    unsigned int Write(const void *data, unsigned int size);
    virtual void Vf20();
    virtual void Vf24();

    HANDLE m_thread;                 // +0x0c; see Vf24
    CRITICAL_SECTION m_lock;         // +0x10
    int m_blockSize;                 // +0x28
    int m_blockCount;                // +0x2c
    void *m_buffer;                  // +0x30
    int m_capacity;                  // +0x34
    int m_writePosition;             // +0x38
    int m_readPosition;              // +0x3c
    int m_available;                 // +0x40
    bool m_stop;                     // +0x44
};

unsigned int EWin32StreamingFile::Read(void *data, unsigned int size)
{
    char *destination = (char *)data;
    int bytesLeft = size;
    while (bytesLeft > 0) {
        if (m_available == 0) {
            Sleep(0);
            continue;
        }
        EnterCriticalSection(&m_lock);
        int n = bytesLeft;
        if (n > m_available)
            n = m_available;
        bool linear = true;
        if (m_readPosition + n > m_capacity)
            linear = false;
        if (linear) {
            memcpy(destination, (char *)m_buffer + m_readPosition, n);
            destination += n;
            m_readPosition += n;
            if (m_readPosition >= m_capacity)
                m_readPosition = 0;
        } else {
            int tail = m_capacity - m_readPosition;
            memcpy(destination, (char *)m_buffer + m_readPosition, tail);
            destination += tail;
            memcpy(destination, m_buffer, n - tail);
            destination += n - tail;
            m_readPosition = n - tail;
        }
        m_available -= n;
        bytesLeft -= n;
        LeaveCriticalSection(&m_lock);
    }
    return size;
}

DWORD WINAPI NativeStreamingThread(void *parameter)
{
    EWin32StreamingFile *file = (EWin32StreamingFile *)parameter;
    file->Vf20();
    return 0;
}

void EWin32StreamingFile::Vf24()
{
    // The original stores the thread ID here and discards the returned handle.
    CreateThread(0, 0, NativeStreamingThread, this, 0, (DWORD *)&m_thread);
}

void EWin32StreamingFile::Vf20()
{
    while (true) {
        if (m_stop)
            break;
        if (m_capacity - m_available < m_blockSize) {
            Sleep(0);
            continue;
        }
        int n = 0;
        int result = ReadFile(m_handle, (char *)m_buffer + m_writePosition,
            m_blockSize, (DWORD *)&n, 0);
        EnterCriticalSection(&m_lock);
        m_writePosition += n;
        if (m_writePosition >= m_capacity)
            m_writePosition = 0;
        m_available += n;
        if (n < m_blockSize) {
            LeaveCriticalSection(&m_lock);
            break;
        }
        LeaveCriticalSection(&m_lock);
    }
}

EWin32StreamingFile::EWin32StreamingFile(HANDLE handle, const char *name, int blockSize, int count)
    : EWin32File(handle, name)
{
    m_thread = 0;
    m_blockSize = blockSize;
    m_blockCount = count;
    m_capacity = blockSize * count;
    m_writePosition = 0;
    m_readPosition = 0;
    m_available = 0;
    m_stop = false;
    unsigned int size = m_capacity;
    void *p = _aligned_malloc(size, 16);
    m_buffer = p;
    InitializeCriticalSection(&m_lock);
}

EWin32StreamingFile::~EWin32StreamingFile()
{
    if (m_thread) {
        m_stop = true;
        DWORD status = 0;
        do {
            GetExitCodeThread(m_thread, &status);
            Sleep(0);
        } while (status == 0x103);
        CloseHandle(m_thread);
    }
    if (m_buffer) {
        void *buffer = m_buffer;
        _aligned_free(buffer);
    }
    DeleteCriticalSection(&m_lock);
}

unsigned int EWin32StreamingFile::Write(const void *data, unsigned int size)
{
    return 0xffffffff;
}

EWin32File::EWin32File(HANDLE handle, const char *name) : m_handle(handle)
{
    g_win32FileCount++;
    if (name)
        m_name = CopyFileName(name, 0x18);
}

EWin32File::~EWin32File()
{
    if (m_name)
        FreeFileName(m_name, 0x18, 1);
    g_win32FileCount--;
    if (m_handle) {
        CloseHandle(m_handle);
        m_handle = 0;
    }
}

unsigned int EWin32File::Read(void *data, unsigned int size)
{
    DWORD n;
    int result = ReadFile(m_handle, data, size, &n, 0);
    if (!result) {
        DWORD error = GetLastError();
    }
    return result ? n : 0xffffffff;
}

unsigned int EWin32File::Write(const void *data, unsigned int size)
{
    DWORD n;
    int result = WriteFile(m_handle, data, size, &n, 0);
    return result ? n : 0xffffffff;
}

unsigned int EWin32File::Seek(int offset, int origin)
{
    DWORD method;
    switch (origin) {
    case 0: method = 0; break;
    case 2: method = 2; break;
    case 1: method = 1; break;
    default: method = 0; break;
    }
    DWORD result = SetFilePointer(m_handle, offset, 0, method);
    return result == 0xffffffff ? 0xffffffff : result;
}

unsigned int EWin32File::Size()
{
    unsigned int position = Seek(0, 1);
    unsigned int size = Seek(0, 2);
    Seek(position, 0);
    return size;
}

__int64 EWin32File::Vf18()
{
    __int64 time;
    int result = GetFileTime(m_handle, 0, 0, (FILETIME *)&time);
    if (!result)
        return 0;
    return time;
}

bool EWin32File::Vf1c(unsigned int low, unsigned int high)
{
    FILETIME time;
    time.dwLowDateTime = low;
    time.dwHighDateTime = high;
    int result = SetFileTime(m_handle, 0, 0, &time);
    return result != 0;
}

void CreateParentDirectories(char *path, bool skipLast)
{
    char *separator = strrchr(path, '\\');
    if (!separator)
        return;
    *separator = 0;
    CreateParentDirectories(path, false);
    *separator = '\\';
    if (!skipLast)
        CreateDirectoryA(path, 0);
}

DiagnosticFile *OpenNativeFile(const char *name, unsigned int options)
{
    HANDLE nativeHandle;
    DWORD openAccess = 0;
    if (options & 0x400)
        openAccess |= 0x80000000;
    if (options & 0x800)
        openAccess |= 0x40000000;
    if (!(options & 0xc00))
        return 0;
    if ((options & 0x800) && (options & 0x1000))
        return 0;
    DWORD shareMode = 0;
    if (options & 0x4000)
        shareMode |= 1;
    DWORD dwCreationDisposition = 0;
    switch (options & 0x300) {
    case 0: dwCreationDisposition = 3; break;
    case 0x100: dwCreationDisposition = 4; break;
    case 0x200: dwCreationDisposition = 5; break;
    case 0x300: dwCreationDisposition = 2; break;
    }
    DWORD dwFlagsAndAttributes = 0x80;
    if (options & 0x8000)
        dwFlagsAndAttributes = 0x60000001;
    bool bRetry = true;
retryOpen:
    nativeHandle = CreateFileA(name, openAccess, shareMode, 0, dwCreationDisposition, dwFlagsAndAttributes, 0);
    if (nativeHandle == (HANDLE)-1) {
        DWORD error = GetLastError();
        if ((options & 0x100) && bRetry && error == 3) {
            char path[260];
            strcpy(path, name);
            CreateParentDirectories(path, true);
            bRetry = false;
            goto retryOpen;
        }
        return 0;
    } else {
        if (options & 0x1000) {
            EWin32StreamingFile *file = new EWin32StreamingFile(nativeHandle, name, 0x8000, 12);
            file->Vf24();
            return file;
        } else {
            return new EWin32File(nativeHandle, name);
        }
    }
}

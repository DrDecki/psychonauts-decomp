// The few Win32 declarations the engine code needs. The Express edition of
// Visual C++ 2008 comes without the Platform SDK, so they are written out here
// with the same layout and calling convention as in windows.h.
#ifndef WIN32_H
#define WIN32_H

#define WINAPI __stdcall
#define WINBASEAPI __declspec(dllimport)

typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned long SIZE_T;
typedef void *HANDLE;

struct _OVERLAPPED;
struct _SECURITY_ATTRIBUTES;

typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME;

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME;

typedef union _ULARGE_INTEGER {
    struct {
        DWORD LowPart;
        DWORD HighPart;
    };
    unsigned __int64 QuadPart;
} ULARGE_INTEGER;

typedef struct _MEMORYSTATUS {
    DWORD dwLength;
    DWORD dwMemoryLoad;
    SIZE_T dwTotalPhys;
    SIZE_T dwAvailPhys;
    SIZE_T dwTotalPageFile;
    SIZE_T dwAvailPageFile;
    SIZE_T dwTotalVirtual;
    SIZE_T dwAvailVirtual;
} MEMORYSTATUS;

typedef struct _RTL_CRITICAL_SECTION {
    void *DebugInfo;
    long LockCount;
    long RecursionCount;
    HANDLE OwningThread;
    HANDLE LockSemaphore;
    unsigned long SpinCount;
} CRITICAL_SECTION;

typedef DWORD (WINAPI *LPTHREAD_START_ROUTINE)(void *parameter);

extern "C" {
WINBASEAPI void WINAPI GetSystemTimeAsFileTime(FILETIME *ft);
WINBASEAPI int WINAPI FileTimeToSystemTime(const FILETIME *ft, SYSTEMTIME *st);
WINBASEAPI void WINAPI GetLocalTime(SYSTEMTIME *st);
WINBASEAPI void WINAPI GlobalMemoryStatus(MEMORYSTATUS *ms);
WINBASEAPI HANDLE WINAPI CreateThread(_SECURITY_ATTRIBUTES *security, SIZE_T stackSize,
    LPTHREAD_START_ROUTINE start, void *parameter, DWORD flags, DWORD *threadId);
WINBASEAPI int WINAPI GetExitCodeThread(HANDLE thread, DWORD *exitCode);
WINBASEAPI void WINAPI Sleep(DWORD milliseconds);
WINBASEAPI void WINAPI InitializeCriticalSection(CRITICAL_SECTION *lock);
WINBASEAPI void WINAPI DeleteCriticalSection(CRITICAL_SECTION *lock);
WINBASEAPI void WINAPI EnterCriticalSection(CRITICAL_SECTION *lock);
WINBASEAPI void WINAPI LeaveCriticalSection(CRITICAL_SECTION *lock);
WINBASEAPI HANDLE WINAPI CreateFileA(const char *name, DWORD access, DWORD share,
    _SECURITY_ATTRIBUTES *security, DWORD creation, DWORD attributes, HANDLE templateFile);
WINBASEAPI int WINAPI CloseHandle(HANDLE handle);
WINBASEAPI DWORD WINAPI GetLastError();
WINBASEAPI int WINAPI ReadFile(HANDLE handle, void *data, DWORD size,
    DWORD *read, _OVERLAPPED *overlapped);
WINBASEAPI int WINAPI WriteFile(HANDLE handle, const void *data, DWORD size,
    DWORD *written, _OVERLAPPED *overlapped);
WINBASEAPI DWORD WINAPI SetFilePointer(HANDLE handle, long offset, long *high, DWORD origin);
WINBASEAPI int WINAPI GetFileTime(HANDLE handle, FILETIME *creation,
    FILETIME *access, FILETIME *write);
WINBASEAPI int WINAPI SetFileTime(HANDLE handle, const FILETIME *creation,
    const FILETIME *access, const FILETIME *write);
WINBASEAPI int WINAPI CreateDirectoryA(const char *path, _SECURITY_ATTRIBUTES *security);
}

#endif

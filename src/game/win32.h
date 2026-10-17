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

extern "C" {
WINBASEAPI void WINAPI GetSystemTimeAsFileTime(FILETIME *ft);
WINBASEAPI int WINAPI FileTimeToSystemTime(const FILETIME *ft, SYSTEMTIME *st);
WINBASEAPI void WINAPI GetLocalTime(SYSTEMTIME *st);
WINBASEAPI void WINAPI GlobalMemoryStatus(MEMORYSTATUS *ms);
}

#endif

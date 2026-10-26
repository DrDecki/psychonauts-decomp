// Partial layouts/names reconstructed from the hash-gated Steam executable.
// Diagnostic names are provisional; EFileManager identity is RTTI confirmed.
// Full sizes/inheritance are unresolved; do not allocate these partial objects.
#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

#include <stdarg.h>
#include <malloc.h>

// Partial interfaces from the stream writer's dispatch; original names and
// ignored return types are unknown. Open's two numeric options are untranslated.
class DiagnosticFile {
public:
    // RTTI EFile; keep this name distinct from script_lua.cpp's file wrapper.
    virtual ~DiagnosticFile() {}
    void operator delete(void *block) { _aligned_free(block); }
    virtual void Close();
    virtual unsigned int Read(void *data, unsigned int size) = 0; // slot +8
    virtual unsigned int Write(const void *data, unsigned int size) = 0; // raw EAX observed by writer flush
    virtual unsigned int Seek(int offset, int origin) = 0; // raw EAX saved by buffered writer
    virtual unsigned int Size() = 0; // slot +0x14, forwarded by buffered files
    virtual __int64 Vf18() { return 0; } // EDX:EAX; meaning unresolved
    virtual bool Vf1c(unsigned int a, unsigned int b) { return false; }

    // Inferred inline byte-count forwarding; its original declaration is lost.
    void Write(const void *data, unsigned int size, unsigned int count)
    {
        Write(data, size * count);
    }

    char *m_name; // +4; owned string, original member name unknown
};

// EFileManager identity is confirmed by RTTI and its static-object constructor.
// The layout below covers only fields used by Open; do not allocate instances.
class EFileManager {
public:
    virtual DiagnosticFile *Open(const char *name, unsigned int options, int mode);
    virtual void Vf04(void *unknown);
    virtual void Vf08();

    const char *ResolvePath(const char *name, int mode, void **first, void **second);

    char pad4[0x20 - 4];
    unsigned int m_category : 5;
    unsigned int m_entryCount : 27; // packed array count at +0x20
};

typedef EFileManager DiagnosticFileFactory; // preserve existing source spelling

// Reference pointer initially targets 0x00795a38. Storage/initialization of that
// object is not reconstructed; keep this global external, without a zero stub.
extern EFileManager *g_diagnosticFileFactory; // 0x00781d90

class DiagnosticStream {
public:
    bool Write(const char *text); // native 0x006a4ee0; AL return now observed

    char pad0[4];
    unsigned char m_enabled;      // observed byte at sink +0x20
    char pad5[0x10 - 5];
    const char *m_name;           // +0x10; passed to factory, exact role provisional
};

class DiagnosticSink {
public:
    void Write(const char *text); // 0x004024d0
    void VPrintf(const char *format, va_list args); // 0x00402560

    char pad0[0x1c];
    DiagnosticStream m_stream;    // observed receiver at sink +0x1c
};

extern DiagnosticSink *g_diagnosticSink; // 0x00781fd8
void NativeDiagnostic(const char *format, ...); // 0x00402300

#endif

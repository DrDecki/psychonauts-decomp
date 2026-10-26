// Buffered files; RTTI confirms the classes and their EFile base.
#include "engine.h"
#include "diagnostic.h"
#include <string.h>

char *CopyFileName(const char *s, int category);
void FreeFileName(char *block, int category, int flags);

struct EReadBufferedFile : DiagnosticFile {
    EReadBufferedFile(DiagnosticFile *file, unsigned int capacity);
    ~EReadBufferedFile()
    {
        if (m_file)
            m_file->Close();
        if (m_buffer) {
            void *mem = m_buffer;
            _aligned_free(mem);
        }
        if (m_name)
            FreeFileName(m_name, 0x18, 1);
    }
    DiagnosticFile *m_file;       // +0x08
    void *m_buffer;               // +0x0c
    unsigned int m_capacity;      // +0x10
    unsigned int m_valid;         // +0x14
    unsigned int m_position;      // +0x18

    unsigned int Read(void *data, unsigned int size);
    void Refill();
    unsigned int Size();
    unsigned int Write(const void *data, unsigned int size) { return 0; }
    unsigned int Seek(int offset, int origin) { return 0; }
    __int64 Vf18();
};

EReadBufferedFile::EReadBufferedFile(DiagnosticFile *file, unsigned int capacity)
    : m_file(file), m_capacity(capacity)
{
    const char *sourceName = file->m_name;
    m_name = CopyFileName(sourceName, 0x18);
    unsigned int size = m_capacity;
    void *p = _aligned_malloc(size, 16);
    m_buffer = p;
    m_valid = 0;
    m_position = 0;
}

unsigned int EReadBufferedFile::Read(void *data, unsigned int size)
{
    unsigned int result, refillAvailable, refillCopy;
    unsigned int bytesLeft = size;
    unsigned int availableCount = m_valid - m_position;
    // Both minima use signed comparisons in the original x86 code. Keep
    // arithmetic unsigned so subtraction/addition retain their 32-bit wrap.
    unsigned int initialCopy = (int)bytesLeft < (int)availableCount ? bytesLeft : availableCount;
    if (initialCopy) {
        memcpy(data, (char *)m_buffer + m_position, initialCopy);
        m_position += initialCopy;
        bytesLeft -= initialCopy;
        data = (char *)data + initialCopy;
    }
    if (!bytesLeft)
        return initialCopy;
    if (bytesLeft > m_capacity) {
        result = m_file->Read(data, bytesLeft);
        return initialCopy + result;
    } else {
        Refill();
        refillAvailable = m_valid - m_position;
        refillCopy = (int)bytesLeft < (int)refillAvailable ? bytesLeft : refillAvailable;
        if (refillCopy) {
            memcpy(data, (char *)m_buffer + m_position, refillCopy);
            m_position += refillCopy;
            bytesLeft -= refillCopy;
        }
        return initialCopy + refillCopy;
    }
}

void EReadBufferedFile::Refill()
{
    m_valid = m_file->Read(m_buffer, m_capacity);
    m_position = 0;
}

unsigned int EReadBufferedFile::Size()
{
    return m_file->Size();
}

__int64 EReadBufferedFile::Vf18()
{
    return m_file->Vf18();
}

// Separate layout from EReadBufferedFile: the buffer is at +0x18 here.
// RTTI confirms EWriteBufferedFile; this is still an opaque partial view.
// In-class Flush is deliberately retained: RTM declines to inline its body
// but reserves its temporary stack space in Write/Seek, as in the executable.
struct EWriteBufferedFile : DiagnosticFile {
    EWriteBufferedFile(DiagnosticFile *file, unsigned int capacity);
    ~EWriteBufferedFile()
    {
        Flush();
        if (m_file)
            m_file->Close();
        if (m_buffer) {
            void *mem = m_buffer;
            _aligned_free(mem);
        }
        if (m_name)
            FreeFileName(m_name, 0x18, 1);
    }
    DiagnosticFile *m_file;
    unsigned int m_capacity, m_pending, m_start;
    void *m_buffer;
    unsigned int Write(const void *data, unsigned int size);
    void Flush()
    {
        if (m_pending == 0)
            return;
        unsigned int result = m_file->Write(m_buffer, m_pending);
        m_pending = 0;
        m_start = 0;
    }
    unsigned int Seek(int offset, int origin);
    unsigned int Read(void *data, unsigned int size);
    unsigned int Size();
    __int64 Vf18();
};

EWriteBufferedFile::EWriteBufferedFile(DiagnosticFile *file, unsigned int capacity)
    : m_file(file), m_capacity(capacity)
{
    const char *sourceName = file->m_name;
    m_name = CopyFileName(sourceName, 0x18);
    unsigned int size = m_capacity;
    void *p = _aligned_malloc(size, 16);
    m_buffer = p;
    m_pending = 0;
    m_start = 0;
}

unsigned int EWriteBufferedFile::Read(void *data, unsigned int size)
{
    Flush();
    return m_file->Read(data, size);
}

unsigned int EWriteBufferedFile::Write(const void *data, unsigned int size)
{
    if (size >= m_capacity) {
        Flush();
        return m_file->Write(data, size);
    } else {
        if (m_pending + size >= m_capacity)
            Flush();
        if (m_pending == 0)
            m_start = m_file->Seek(0, 1);
        memcpy((char *)m_buffer + m_pending, data, size);
        m_pending += size;
        return size;
    }
}

unsigned int EWriteBufferedFile::Seek(int offset, int origin)
{
    Flush();
    return m_file->Seek(offset, origin);
}

unsigned int EWriteBufferedFile::Size()
{
    Flush();
    return m_file->Size();
}

__int64 EWriteBufferedFile::Vf18()
{
    Flush();
    return m_file->Vf18();
}

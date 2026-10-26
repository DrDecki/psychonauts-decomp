// Native diagnostic path. Provisional names and partial layouts: diagnostic.h.
#include <stdio.h>
#include <string.h>
#include "diagnostic.h"

DiagnosticSink *g_diagnosticSink;

bool DiagnosticStream::Write(const char *text)
{
    if (!m_enabled)
        return false;
    const char *name = m_name;
    DiagnosticFile *file = g_diagnosticFileFactory->Open(name, 0x800, 4);
    if (file) {
        file->Seek(0, 2);
        file->Write(text, 1, strlen(text));
        file->Close();
        return true;
    } else {
        return false;
    }
}

void NativeDiagnostic(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    if (g_diagnosticSink)
        g_diagnosticSink->VPrintf(format, args);
    va_end(args);
}

void DiagnosticSink::Write(const char *text)
{
    if (!text || !*text)
        return;
    unsigned char enabled = m_stream.m_enabled;
    if (enabled) {
        m_stream.Write(text);
        m_stream.Write("\n");
    }
}

void DiagnosticSink::VPrintf(const char *format, va_list args)
{
    char buffer[1024];
    _vsnprintf(buffer, 1023, format, args);
    buffer[1023] = 0;
    Write(buffer);
}

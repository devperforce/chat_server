#include "pch.h"
#include "utility/exception/exception_handler.h"

#include "utility/log.h"

#include <new.h>
#include <tchar.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <signal.h>
#include <exception>
#include <sys/stat.h>
#include <psapi.h>
#include <rtcapi.h>
#include <Shellapi.h>
#include <dbghelp.h>

struct ExceptionHandlerInfo {
    std::wstring name;
    MINIDUMP_TYPE dump_type;
};

static std::unique_ptr<ExceptionHandlerInfo> exception_handler_info = nullptr;

static void GetExceptionPointers(DWORD dwExceptionCode,  EXCEPTION_POINTERS** ppExceptionPointers) {
    EXCEPTION_RECORD ExceptionRecord;
    CONTEXT ContextRecord;
    memset(&ContextRecord, 0, sizeof(CONTEXT));

#ifdef _X86_
    __asm {
        mov dword ptr [ContextRecord.Eax], eax
        mov dword ptr [ContextRecord.Ecx], ecx
        mov dword ptr [ContextRecord.Edx], edx
        mov dword ptr [ContextRecord.Ebx], ebx
        mov dword ptr [ContextRecord.Esi], esi
        mov dword ptr [ContextRecord.Edi], edi
        mov word ptr [ContextRecord.SegSs], ss
        mov word ptr [ContextRecord.SegCs], cs
        mov word ptr [ContextRecord.SegDs], ds
        mov word ptr [ContextRecord.SegEs], es
        mov word ptr [ContextRecord.SegFs], fs
        mov word ptr [ContextRecord.SegGs], gs
        pushfd
        pop [ContextRecord.EFlags]
    }
    ContextRecord.ContextFlags = CONTEXT_CONTROL;
#pragma warning(push)
#pragma warning(disable:4311)
    ContextRecord.Eip = (ULONG)_ReturnAddress();
    ContextRecord.Esp = (ULONG)_AddressOfReturnAddress();
#pragma warning(pop)
    ContextRecord.Ebp = *((ULONG *)_AddressOfReturnAddress()-1);
#elif defined (_IA64_) || defined (_AMD64_)
    /* Need to fill up the Context in IA64 and AMD64. */
    RtlCaptureContext(&ContextRecord);
#else  /* defined (_IA64_) || defined (_AMD64_) */
    ZeroMemory(&ContextRecord, sizeof(ContextRecord));
#endif  /* defined (_IA64_) || defined (_AMD64_) */
    ZeroMemory(&ExceptionRecord, sizeof(EXCEPTION_RECORD));
    ExceptionRecord.ExceptionCode = dwExceptionCode;
    ExceptionRecord.ExceptionAddress = _ReturnAddress();
    ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;

    EXCEPTION_RECORD* pExceptionRecord = new EXCEPTION_RECORD;
    memcpy(pExceptionRecord, &ExceptionRecord, sizeof(EXCEPTION_RECORD));
    CONTEXT* pContextRecord = new CONTEXT;
    memcpy(pContextRecord, &ContextRecord, sizeof(CONTEXT));
    *ppExceptionPointers = new EXCEPTION_POINTERS;
    (*ppExceptionPointers)->ExceptionRecord = pExceptionRecord;
    (*ppExceptionPointers)->ContextRecord = pContextRecord;  
}


static void CreateMiniDump(
    __in struct _EXCEPTION_POINTERS* exception_info
) {
    HMODULE hDbgHelp = NULL;
    HANDLE hFile = NULL;
    MINIDUMP_EXCEPTION_INFORMATION mei;
    MINIDUMP_CALLBACK_INFORMATION mci;

    // Load dbghelp.dll
    hDbgHelp = LoadLibrary(_T("dbghelp.dll"));
    if (hDbgHelp == NULL)
    {
        // Error - couldn't load dbghelp.dll
        return;
    }

    SYSTEMTIME system_time = { 0 };
    ::GetLocalTime(&system_time);

    const auto name = exception_handler_info->name;
    const auto dump_type = exception_handler_info->dump_type;

    const std::wstring dump_name = std::format(L"{}_{}-{}-{}-{}-{}-{}.dmp",
        name,
        system_time.wYear, system_time.wMonth, system_time.wDay,
        system_time.wHour, system_time.wMinute, system_time.wSecond
    );

    // Create the minidump file
    hFile = CreateFile(
        dump_name.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        // Couldn't create file
        return;
    }

    // Write minidump to the file
    mei.ThreadId = GetCurrentThreadId();
    mei.ExceptionPointers = exception_info;
    mei.ClientPointers = FALSE;
    mci.CallbackRoutine = nullptr;
    mci.CallbackParam = nullptr;

    typedef BOOL(WINAPI* LPMINIDUMPWRITEDUMP)(
        HANDLE hProcess,
        DWORD ProcessId,
        HANDLE hFile,
        MINIDUMP_TYPE DumpType,
        CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
        CONST PMINIDUMP_USER_STREAM_INFORMATION UserEncoderParam,
        CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

    LPMINIDUMPWRITEDUMP pfnMiniDumpWriteDump =
        (LPMINIDUMPWRITEDUMP)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
    if (!pfnMiniDumpWriteDump)
    {
        // Bad MiniDumpWriteDump function
        return;
    }

    HANDLE hProcess = GetCurrentProcess();
    DWORD dwProcessId = GetCurrentProcessId();

    BOOL bWriteDump = pfnMiniDumpWriteDump(
        hProcess,
        dwProcessId,
        hFile,
        dump_type,
        &mei,
        NULL,
        &mci);

    if (!bWriteDump)
    {
        // Error writing dump.
        return;
    }

    // Close file
    CloseHandle(hFile);

    // Unload dbghelp.dll
    FreeLibrary(hDbgHelp);
}

static LONG WINAPI SehCallback(PEXCEPTION_POINTERS pExceptionPtrs)
{ 
    // Write minidump file
    CreateMiniDump(pExceptionPtrs);

    // Terminate process
    TerminateProcess(GetCurrentProcess(), 1);    

    // Unreacheable code  
    return EXCEPTION_EXECUTE_HANDLER;
}

// CRT Pure virtual method call handler
static void PureCallCallback() {
    // Pure virtual function call

    // Retrieve exception information
    EXCEPTION_POINTERS* pExceptionPtrs = NULL;
    GetExceptionPointers(0, &pExceptionPtrs);

    // Write minidump file
    CreateMiniDump(pExceptionPtrs);

    // Terminate process
    TerminateProcess(GetCurrentProcess(), 1);    
}

static int32_t NewCallback(size_t) {
    // 'new' operator memory allocation exception

    // Retrieve exception information
    EXCEPTION_POINTERS* pExceptionPtrs = NULL;
    GetExceptionPointers(0, &pExceptionPtrs);

    // Write minidump file
    CreateMiniDump(pExceptionPtrs);

    // Terminate process
    TerminateProcess(GetCurrentProcess(), 1);

    // Unreacheable code
    return 0;
}

// CRT invalid parameter handler
static void InvalidParameterCallback(
    const wchar_t* expression, 
    const wchar_t* function, 
    const wchar_t* file, 
    unsigned int line, 
    uintptr_t pReserved) {
    UNREFERENCED_PARAMETER(pReserved);

    // Invalid parameter exception

    // Retrieve exception information
    EXCEPTION_POINTERS* pExceptionPtrs = nullptr;
    GetExceptionPointers(0, &pExceptionPtrs);

    // Write minidump file
    CreateMiniDump(pExceptionPtrs);

    // Terminate process
    TerminateProcess(GetCurrentProcess(), 1);    
}

// CRT SIGABRT signal handler
static void SigabrtCallback(int32_t signal) {
    // Caught SIGABRT C++ signal

    // Retrieve exception information
    EXCEPTION_POINTERS* pExceptionPtrs = nullptr;
    GetExceptionPointers(0, &pExceptionPtrs);

    // Write minidump file
    CreateMiniDump(pExceptionPtrs);

    // Terminate process
    TerminateProcess(GetCurrentProcess(), 1);  
}

// CRT SIGFPE signal handler
static void SigfpeCallback(int32_t code, int32_t subcode) {
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(subcode);

    // Floating point exception (SIGFPE)
    EXCEPTION_POINTERS* pExceptionPtrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;

    // Write minidump file
    CreateMiniDump(pExceptionPtrs);

    // Terminate process
    TerminateProcess(GetCurrentProcess(), 1);
}

// CRT sigill signal handler
static void SigillCallback(int32_t singal) {
    // Illegal instruction (SIGILL)
    UNREFERENCED_PARAMETER(singal);
    // Retrieve exception information
    EXCEPTION_POINTERS* pExceptionPtrs = NULL;
    GetExceptionPointers(0, &pExceptionPtrs);

    // Write minidump file
    CreateMiniDump(pExceptionPtrs);

    // Terminate process
    TerminateProcess(GetCurrentProcess(), 1);    
}

static void SigtermCallback(int signal) {
    // Termination request (SIGTERM)
    UNREFERENCED_PARAMETER(signal);

    // Retrieve exception information
    EXCEPTION_POINTERS* pExceptionPtrs = NULL;
    GetExceptionPointers(0, &pExceptionPtrs);

    // Write minidump file
    CreateMiniDump(pExceptionPtrs);

    // Terminate process
    TerminateProcess(GetCurrentProcess(), 1); 
}

namespace dev::utility {

int64_t ExceptionHandler::Start(
    const std::wstring& name,
    MINIDUMP_TYPE dump_type
) {
    if (exception_handler_info != nullptr) {
        return ERROR_APP_INIT_FAILURE;
    }

    exception_handler_info = std::make_unique<ExceptionHandlerInfo>(name, dump_type);

    SetUnhandledExceptionFilter(SehCallback);

    // Catch pure virtual function calls.
    // Because there is one _purecall_handler for the whole process, 
    // calling this function immediately impacts all threads. The last 
    // caller on any thread sets the handler. 
    // http://msdn.microsoft.com/en-us/library/t296ys27.aspx
    _set_purecall_handler(PureCallCallback);

    // Catch new operator memory allocation exceptions
    _set_new_handler(NewCallback);

    // Catch invalid parameter exceptions.
    _set_invalid_parameter_handler(InvalidParameterCallback); 

    // Set up C++ signal handlers
    _set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);

    // Catch an abnormal program termination
    signal(SIGABRT, SigabrtCallback);  

    // Catch illegal instruction handler
    signal(SIGINT, SigillCallback);     

    // Catch a termination request
    signal(SIGTERM, SigtermCallback);      

    return ERROR_SUCCESS;
}

} // namespace dev::utility

// TODO: StackWalk 구현
// https://ozt88.tistory.com/48
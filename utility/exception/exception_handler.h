#pragma once

#pragma warning(push)
#pragma warning(disable: 4091)
#include <DbgHelp.h>
#pragma warning(pop)

#pragma comment(lib, "Dbghelp.lib")

namespace dev::utility {

class ExceptionHandler {
public:
    static int64_t Start(const std::wstring& name, MINIDUMP_TYPE dump_type);
};

} // namespace dev::utility

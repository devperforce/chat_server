#include "pch.h"
#include "utility/file/path.h"

namespace dev::utility {

static std::wstring GetExePath(const std::wstring& path) {
    return path.substr(0, path.find_last_of(L"\\/" ));
}

std::wstring Path::GetExeFileName() {
    wchar_t current_directory[MAX_PATH];
    GetModuleFileName(nullptr, current_directory, MAX_PATH);
    return current_directory;
}

void Path::SetCurrentPath(const std::wstring& filename) {
    const std::wstring module_path = GetExePath(filename);
    std::filesystem::current_path(module_path);
}

} // namespace dev::utility

#pragma once

namespace dev::utility {

class Path {
public:
    
    static std::wstring GetExeFileName();

    static void SetCurrentPath(const std::wstring& filename);
    //static std::wstring GetExePath(const std::wstring& path);
};

} // namespace dev::utility

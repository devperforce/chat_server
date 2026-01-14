#pragma once

namespace dev::utility {

class StringConvert {
public:
    static std::wstring Utf8ToUtf16(const std::string& string);
    static std::string Utf16ToUtf8(const std::wstring& wide_string);
};

} // namespace dev::utility

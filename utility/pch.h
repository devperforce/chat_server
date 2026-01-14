// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

//#define BOOST_USE_WINAPI_VERSION 0x0A00
#define _WIN32_WINNT 0x0A00
// add headers that you want to pre-compile here
#include "framework.h"

#include <cstdint>
#include <chrono>
#include <ctime>
#include <format>
#include <filesystem>
#include <type_traits>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/core/detail/base64.hpp>
#include <boost/beast/version.hpp>
#include <boost/lexical_cast.hpp>

#include "Windows.h"

#endif //PCH_H

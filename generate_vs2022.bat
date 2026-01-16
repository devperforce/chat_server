set TARGET="Visual Studio 17 2022"
set VCPKG_ROOT=%LOCALAPPDATA%\Dev\vcpkg
set VCPKG_CMAKE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake

set SCRIPT_DIR=%~dp0

cmake -B build -G %TARGET% -DCMAKE_TOOLCHAIN_FILE=%VCPKG_CMAKE% -DVCPKG_TARGET_TRIPLET=x64-windows-static

pause

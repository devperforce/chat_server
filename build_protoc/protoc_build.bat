set CURRENT_PATH=%~dp0
echo %CURRENT_PATH%

..\build\vcpkg_installed\x64-windows-static\tools\protobuf\protoc -I ..\build\vcpkg_installed\x64-windows-static\include -I %CURRENT_PATH%..\protobuf\idl\ --cpp_out=%CURRENT_PATH%..\protobuf\generated\ packet_id.proto chatting.proto

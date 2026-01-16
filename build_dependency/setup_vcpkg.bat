@echo off
setlocal

REM 현재 위치 저장
set CURRENT_DIR=%CD%

REM 경로 설정
set DEV_PATH=%LOCALAPPDATA%\Dev
set VCPKG_ROOT=%DEV_PATH%\vcpkg

REM Dev 폴더 생성
if not exist "%DEV_PATH%" (
    mkdir "%DEV_PATH%"
)

cd /d "%DEV_PATH%"

REM vcpkg 설치 or 업데이트
if not exist "%VCPKG_ROOT%" (
    echo Cloning vcpkg...
    git clone https://github.com/microsoft/vcpkg.git
) else (
    echo vcpkg already exists. Updating...
    cd /d "%VCPKG_ROOT%"
    git pull
)

cd /d "%VCPKG_ROOT%"

REM bootstrap
echo Bootstrapping vcpkg...
call bootstrap-vcpkg.bat

REM VS 연동
echo Integrating vcpkg with Visual Studio...
vcpkg integrate install

REM 원래 위치로 복귀
cd /d "%CURRENT_DIR%"
pause
endlocal

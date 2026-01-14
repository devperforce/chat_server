mkdir %LOCALAPPDATA%\Dev
cd %LOCALAPPDATA%\Dev
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
call bootstrap-vcpkg.bat
vcpkg integrate install
git pull
pause
$Currentlocation = Get-Location
if (-not (Test-Path -Path $env:LOCALAPPDATA\Dev)) {
    New-Item -Path $env:LOCALAPPDATA\Dev -ItemType Directory
} else {
    echo "Folder Exist!"
}
cd $env:LOCALAPPDATA\Dev
git clone https://github.com/Microsoft/vcpkg.git
cd $env:LOCALAPPDATA\Dev\vcpkg
./bootstrap-vcpkg.bat
./vcpkg integrate install
git pull
cd $Currentlocation
pause

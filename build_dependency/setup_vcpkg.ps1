$CurrentLocation = Get-Location
$VcpkgRoot = Join-Path $env:LOCALAPPDATA "Dev\vcpkg"

# Dev 폴더 생성
$DevPath = Join-Path $env:LOCALAPPDATA "Dev"
if (-not (Test-Path $DevPath)) {
    New-Item -Path $DevPath -ItemType Directory | Out-Null
}

Set-Location $DevPath

# vcpkg 설치 or 업데이트
if (-not (Test-Path $VcpkgRoot)) {
    Write-Host "Cloning vcpkg..."
    git clone https://github.com/microsoft/vcpkg.git
} else {
    Write-Host "vcpkg already exists. Updating..."
    Set-Location $VcpkgRoot
    git pull
}

Set-Location $VcpkgRoot

# bootstrap
Write-Host "Bootstrapping vcpkg..."
.\bootstrap-vcpkg.bat

# VS 연동
Write-Host "Integrating vcpkg with Visual Studio..."
.\vcpkg integrate install

# 원래 위치로 복귀
Set-Location $CurrentLocation
pause
# chat_server

## 📌 개요

`chat_server`는 C++ 기반 **TCP 네트워크 채팅 서버** 프로젝트입니다.  
Windows 환경, Visual Studio와 CMake를 사용해 빌드 및 실행이 가능하며, 네트워크 소켓을 이용해 여러 클라이언트 간 메시지를 송수신할 수 있는 서버 프로젝트입니다.
> 이 프로젝트는 Boost.Asio 기반 TCP 채팅 서버의 구조를 보여주기 위한 샘플입니다.  
> Windows Service 등록/운영과 같은 실제 서비스 환경 구성은 다루지 않습니다.
---

## 📂 파일 구조 및 설명

```text
chat_server/
├── build_dependency/       # 프로젝트 빌드에 필요한 외부 라이브러리/종속성
├── build_protoc/           # Protocol Buffers 관련 빌드 스크립트
├── chat_server/            # 서버 실행 코드 및 메인 로직
├── dummy_client/           # 테스트용 클라이언트 코드
├── network/                # 네트워크 처리 관련 코드 (소켓, 패킷 처리 등)
├── property_sheets/        # Visual Studio 프로퍼티 시트
├── protobuf/               # Protobuf 메시지 정의 파일(.proto)
├── system/                 # 시스템 레벨 유틸리티 코드
├── utility/                # 프로젝트 전반에서 사용하는 유틸리티 함수 모음
├── CMakeLists.txt          # CMake 빌드 설정
├── README.txt              # 기존 README 텍스트 파일
├── generate_vs2022.bat     # VS2022 프로젝트 자동 생성 배치 파일
├── vcpkg.json              # vcpkg 패키지 매니페스트
├── .editorconfig           # 코드 스타일 설정
├── .gitattributes          # Git 속성
└── .gitignore              # Git 무시 파일 설정
```

---

## 🛠 요구 사항

- **Windows 10/11**
- **Visual Studio 2022 이상**
- **CMake 3.18 이상**
- **vcpkg** — 외부 라이브러리 의존성 관리

---

## 🚀 설치 & 빌드

### 1. 리포지토리 복제

```powershell
git clone https://github.com/devperforce/chat_server.git
cd chat_server
```

### 2. vcpkg 설치
```
PowerShell 관리자 권한 실행
Set-ExecutionPolicy Unrestricted
build_dependency/setup_vcpkg.ps1
```

### 3. 솔루션 시작
```
build/server_all.sln
```
![generate_sln](https://raw.githubusercontent.com/devperforce/chat_server/main/build_dependency/generate_vs2022.png)

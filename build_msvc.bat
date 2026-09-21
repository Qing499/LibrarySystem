@echo off
rem ============================================================
rem  Build with Visual Studio MSBuild (Debug x64)
rem  Usage: double-click this file, or run: build_msvc.bat
rem  Note: messages are ASCII-only to avoid cmd code page issues.
rem ============================================================
setlocal
set "SLN=%~dp0LibrarySystem.sln"
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "MSBUILD="

if exist "%VSWHERE%" (
  for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do set "MSBUILD=%%i"
)

if not defined MSBUILD (
  echo [ERROR] MSBuild not found. Please open LibrarySystem.sln in Visual Studio and press F5.
  exit /b 1
)

echo [INFO] MSBuild: %MSBUILD%
"%MSBUILD%" "%SLN%" /p:Configuration=Debug /p:Platform=x64 /v:minimal /nologo
if errorlevel 1 (
  echo [ERROR] Build failed.
  exit /b 1
)

echo.
echo [OK] Output: %~dp0build\x64\Debug\LibrarySystem.exe
echo [OK] Run self test: build\x64\Debug\LibrarySystem.exe --selftest
endlocal

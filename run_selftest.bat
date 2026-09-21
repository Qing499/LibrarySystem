@echo off
rem ============================================================
rem  Run the built-in self test (dates / borrow / return / fine / file IO)
rem ============================================================
setlocal
set "EXE=%~dp0build\x64\Debug\LibrarySystem.exe"
if not exist "%EXE%" set "EXE=%~dp0build\x64\Release\LibrarySystem.exe"
if not exist "%EXE%" set "EXE=%~dp0build\gcc\LibrarySystem_gcc.exe"

if not exist "%EXE%" (
  echo [ERROR] Executable not found. Run build_msvc.bat or build_gcc.bat first.
  exit /b 1
)

echo [INFO] Running: "%EXE%" --selftest
echo.
"%EXE%" --selftest
set "CODE=%ERRORLEVEL%"
echo.
if "%CODE%"=="0" echo [OK] Self test passed.
if not "%CODE%"=="0" echo [ERROR] Self test failed, exit code %CODE%.
endlocal & exit /b %CODE%

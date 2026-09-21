@echo off
rem ============================================================
rem  Build with MinGW g++ - backup toolchain used for cross-check
rem  Source layout - see README.md
rem    src\Core      Date / Config / TextUtils
rem    src\Model     Person / Student / Teacher / Book / BorrowRecord
rem    src\Service   Library
rem    src\UI        ConsoleUtils / main
rem    src\Test      SelfTest
rem    src\Demo      ClassDemo
rem  Usage: build_gcc.bat
rem ============================================================
setlocal
set "SRC=%~dp0"
set "OUT=%~dp0build\gcc"
set "GXX="

where g++ >nul 2>nul && set "GXX=g++"
if not defined GXX if exist "C:\Program Files\CodeBlocks\MinGW\bin\g++.exe" set "GXX=C:\Program Files\CodeBlocks\MinGW\bin\g++.exe"
if not defined GXX if exist "C:\mingw64\bin\g++.exe" set "GXX=C:\mingw64\bin\g++.exe"

if not defined GXX (
  echo [ERROR] g++ not found. Install MinGW-w64 or use Visual Studio via build_msvc.bat.
  exit /b 1
)

if not exist "%OUT%" mkdir "%OUT%"

"%GXX%" -std=c++17 -Wall -Wextra -Wpedantic -O2 -static -static-libgcc -static-libstdc++ ^
  -I"%SRC%src\Core" -I"%SRC%src\Model" -I"%SRC%src\Service" -I"%SRC%src\UI" -I"%SRC%src\Test" -I"%SRC%src\Demo" ^
  -o "%OUT%\LibrarySystem_gcc.exe" ^
  "%SRC%src\Core\Date.cpp" ^
  "%SRC%src\Model\Book.cpp" "%SRC%src\Model\BorrowRecord.cpp" "%SRC%src\Model\Person.cpp" ^
  "%SRC%src\Model\Student.cpp" "%SRC%src\Model\Teacher.cpp" ^
  "%SRC%src\Service\Library.cpp" ^
  "%SRC%src\UI\ConsoleUtils.cpp" "%SRC%src\UI\main.cpp" ^
  "%SRC%src\Test\SelfTest.cpp" ^
  "%SRC%src\Demo\ClassDemo.cpp"
if errorlevel 1 (
  echo [ERROR] Build failed.
  exit /b 1
)

echo.
echo [OK] Output: %OUT%\LibrarySystem_gcc.exe
endlocal

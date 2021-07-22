@echo off

REM copied from Cellular-automaton build file

setlocal

set SourceFiles=../main.c ../component.c
set OutputName=MinimaLogic.exe

pushd ..

if not exist "Libraries" mkdir Libraries
if exist "Libraries/SDL2" goto SkipDownloadSDL

pushd Libraries
mkdir SDL2
curl "https://www.libsdl.org/release/SDL2-devel-2.0.14-VC.zip" --output SDL2.zip
tar -zxvf SDL2.zip -C SDL2
del SDL2.zip
ren "SDL2\SDL2-2.0.14\include" "SDL2"
popd

:SkipDownloadSDL

if exist "Libraries/SDL2MinGw/SDL2-2.0.14/" if exist "Libraries/SDL2MinGw/SDL2_ttf-2.0.15/" goto SkipDownloadSDLMinGw

pushd Libraries
mkdir SDL2MinGw
curl "https://www.libsdl.org/release/SDL2-devel-2.0.14-mingw.tar.gz" --output SDL2MinGw.tar.gz
curl "https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-devel-2.0.15-mingw.tar.gz" --output SDL2MinGw_ttf.tar.gz
tar -xf SDL2MinGw.tar.gz -C SDL2MinGw
tar -xf SDL2MinGw_ttf.tar.gz -C SDL2MinGw
del SDL2MinGw.zip
del SDL2MinGw_ttf.tar.gz
popd

:SkipDownloadSDLMinGw
popd

set SDL2_Include="../../Libraries/SDL2/SDL2-2.0.14/"
set SDL2_Library="../../Libraries/SDL2/SDL2-2.0.14/lib/x64/"
set SDL2_DLL="..\..\Libraries\SDL2\SDL2-2.0.14\lib\x64\SDL2.dll"

set SDL2MinGw_Include="../../Libraries/SDL2MinGw/SDL2-2.0.14/i686-w64-mingw32/include/"
set SDL2MinGw_Library="../../Libraries/SDL2MinGw/SDL2-2.0.14/i686-w64-mingw32/lib/"
set SDL2MinGw_DLL="..\..\Libraries\SDL2MinGw\SDL2-2.0.14\i686-w64-mingw32\bin\SDL2.dll"

set SDL2MinGw_ttf_Library="../../Libraries/SDL2MinGw/SDL2_ttf-2.0.15/i686-w64-mingw32/lib/"
set SDL2MinGw_ttf_Include = "../../Libraries/SDL2MinGw/SDL2_ttf-2.0.15/i686-w64-mingw32/include"
set SDL2MinGw_ttf_DLL="..\..\Libraries\SDL2MinGw\SDL2_ttf-2.0.15\i686-w64-mingw32\bin\SDL2_ttf.dll"

if not exist "bin" mkdir bin

echo -------------------------------------
echo SDL2 inlude path:  %SDL2_Include%
echo SDL2 library path: %SDL2_Library%
echo SDL2MinGw inlude path:  %SDL2_Include%
echo SDL2MinGw library path: %SDL2_Library%

where cl >nul 2>nul
if %ERRORLEVEL% neq 0 goto SkipMSVC
echo Building with MSVC
pushd bin
xcopy %SDL2_DLL% .\ /Y
call cl -I%SDL2_Include% -nologo %CLFlags% -Zi -EHsc %SourceFiles% -Fe%OutputName% /link /LIBPATH:%SDL2_Library% SDL2.lib SDL2main.lib Shell32.lib /subsystem:console
goto Finished

:SkipMSVC

where clang >nul 2>nul
IF %ERRORLEVEL% NEQ 0 goto SkipCLANG
echo Building with CLANG
pushd bin
xcopy %SDL2_DLL% .\ /Y
xcopy %SDL2MinGw_ttf_DLL% .\ /Y
call clang -I%SDL2_Include% -I%SDL2MinGw_ttf_Include% -L%SDL2_Library% -L%SDL2MinGw_ttf_Library% %CLANGFlags% %SourceFiles% -o %OutputName% -lSDL2main -lSDL2 -lSDL2-ttf -lShell32 -Xlinker -subsystem:console
popd
goto Finished

:SkipCLANG

where gcc >nul 2>nul
IF %ERRORLEVEL% NEQ 0 goto SkipGCC
echo Building with GCC
pushd bin
xcopy %SDL2MinGw_DLL% .\ /Y
xcopy %SDL2MinGw_ttf_DLL% .\ /Y
call gcc -I%SDL2MinGw_Include% -I%SDL2MinGw_ttf_Include% -L%SDL2MinGw_Library% -L%SDL2MinGw_ttf_Library% %GCCFlags% %SourceFiles% -o %OutputName% -w -Wl,-subsystem,console -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf
popd
goto Finished

:SkipGCC

echo No compiler found, please setup compiler. Instructions: https://github.com/IT-Club-Pulchowk/C-Programming-Guide

:Finished
echo -------------------

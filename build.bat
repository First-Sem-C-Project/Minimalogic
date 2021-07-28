@echo off

setlocal

set SourceFiles=../../src/*.c
set OutputName=MinimaLogic.exe

pushd ..

if not exist "Libraries" mkdir Libraries
if exist "Libraries/SDL2/SDL2-2.0.14" if exist "Libraries/SDL2/SDL2_ttf-2.0.15" goto SkipDownloadSDL

pushd Libraries
mkdir SDL2
curl "https://www.libsdl.org/release/SDL2-devel-2.0.14-VC.zip" --output SDL2.zip
curl "https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-devel-2.0.15-VC.zip" --output SDL2_ttf.zip
tar -zxvf SDL2.zip -C SDL2
tar -zxvf SDL2_ttf.zip -C SDL2
del SDL2.zip
del SDL2_ttf.zip
xcopy /Y SDL2\SDL2_ttf-2.0.15\include\SDL_ttf.h SDL2\SDL2-2.0.14\include
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
xcopy /Y SDL2MinGw\SDL2_ttf-2.0.15\x86_64-w64-mingw32\include\SDL2\SDL_ttf.h SDL2MinGw\SDL2-2.0.14\x86_64-w64-mingw32\include\SDL2
popd

:SkipDownloadSDLMinGw
popd

set SDL2_Include="../../../Libraries/SDL2/SDL2-2.0.14/"
set SDL2_Library="../../../Libraries/SDL2/SDL2-2.0.14/lib/x64/"
set SDL2_DLL="..\..\..\Libraries\SDL2\SDL2-2.0.14\lib\x64\SDL2.dll"

set SDL2_ttf_Library="../../../Libraries/SDL2/SDL2_ttf-2.0.15/lib/x64"
set SDL2_ttf_DLL="..\..\..\Libraries\SDL2\SDL2_ttf-2.0.15\lib\x64\*.dll"

set SDL2MinGw_Include="../../../Libraries/SDL2MinGw/SDL2-2.0.14/x86_64-w64-mingw32/include/"
set SDL2MinGw_Library="../../../Libraries/SDL2MinGw/SDL2-2.0.14/x86_64-w64-mingw32/lib/"
set SDL2MinGw_DLL="..\..\..\Libraries\SDL2MinGw\SDL2-2.0.14\x86_64-w64-mingw32\bin\SDL2.dll"

set SDL2MinGw_ttf_Library="../../../Libraries/SDL2MinGw/SDL2_ttf-2.0.15/x86_64-w64-mingw32/lib/"
set SDL2MinGw_ttf_DLL="..\..\..\Libraries\SDL2MinGw\SDL2_ttf-2.0.15\x86_64-w64-mingw32\bin\*.dll"

if not exist "bin" mkdir bin

echo ----------------------------------------
:MSVC
where cl >nul 2>nul
IF %ERRORLEVEL% NEQ 0 goto SkipMSVC
echo Building with MSVC
if not exist "bin\MSVCBuild" mkdir bin\MSVCBuild
pushd bin\MSVCBuild
xcopy %SDL2_DLL% .\ /Y
xcopy %SDL2_ttf_DLL% .\ /Y
call cl -I%SDL2_Include% -nologo -Zi -EHsc %SourceFiles% -Fe%OutputName% /link /LIBPATH:%SDL2_Library% SDL2.lib SDL2main.lib Shell32.lib /LIBPATH:%SDL2_ttf_Library% SDL2_ttf.lib /subsystem:windows
popd
echo MSVC Build Complete
echo ----------------------------------------
goto CLANG

:SkipMSVC
echo MSVC not found. Skipping build with MSVC. 
echo ----------------------------------------

:CLANG
where clang >nul 2>nul
IF %ERRORLEVEL% NEQ 0 goto SkipCLANG
echo Building with CLANG
if not exist "bin\ClangBuild" mkdir bin\ClangBuild
pushd bin\ClangBuild
xcopy %SDL2_DLL% .\ /Y
xcopy %SDL2_ttf_DLL% .\ /Y
call clang -I%SDL2_Include% -L%SDL2_Library% -L%SDL2_ttf_Library% %SourceFiles% -o %OutputName% -lSDL2main -lSDL2 -lSDL2_ttf -lShell32 -Xlinker -subsystem:windows
echo Clang Build Complete
echo ----------------------------------------
popd
goto GCC

:SkipCLANG
echo Clang not found. Skipping build with Clang. 
echo ----------------------------------------

:GCC
where gcc >nul 2>nul
IF %ERRORLEVEL% NEQ 0 goto SkipGCC
echo Building with GCC
if not exist "bin\GccBuild" mkdir bin\GccBuild
pushd bin\GccBuild
xcopy %SDL2MinGw_DLL% .\ /Y
xcopy %SDL2MinGw_ttf_DLL% .\ /Y
call gcc -I%SDL2MinGw_Include% -L%SDL2MinGw_Library% -L%SDL2MinGw_ttf_Library% %SourceFiles% -o %OutputName% -w -Wl,-subsystem,windows -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf
echo Gcc Build Complete
echo ----------------------------------------
popd
goto Finished

:SkipGCC
echo Gcc not found. Skipping build with Gcc. 
echo ----------------------------------------

:Finished

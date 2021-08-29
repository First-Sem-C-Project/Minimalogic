@echo off

setlocal

set SourceFiles=../../src/*.c
set OutputName=MinimaLogic.exe

set CLFlags=-Od
set CLANGFlags=-g -gcodeview
set GCCFlags=-O

if "%1" neq "optimize" goto DoneConfig
set CLFlags=/Ox
set CLANGFlags=-O3 -gcodeview
set GCCFlags=-O3

echo -------------------------------------
echo Optimize Build configured
echo -------------------------------------
:DoneConfig

if not exist "fonts" mkdir fonts
pushd fonts
if exist "anka-coder-narrow/AnkaCoder-C75-r.ttf" goto SkipDownloadUIFont
echo ----------------------------------------
echo Downloading UI Font
echo ----------------------------------------
curl "https://fontlibrary.org/assets/downloads/anka-coder-narrow/4ed9dd9fcfa020d9f9c3de3fffb2b2ad/anka-coder-narrow.zip" --output anka.zip
mkdir anka-coder-narrow
tar -zxvf anka.zip -C anka-coder-narrow
del anka.zip
:SkipDownloadUIFont
if exist "segment7/Segment7Standard.otf" goto SkipDownloadLEDFont
echo ----------------------------------------
echo Downloading LED Font
echo ----------------------------------------
curl "https://fontlibrary.org/assets/downloads/segment7/4cc82137fc130708919bf201c0dc9aae/segment7.zip" --output seg.zip
mkdir segment7
tar -zxvf seg.zip -C segment7
del seg.zip
:SkipDownloadLEDFont
popd

if not exist "libs" mkdir libs
pushd libs
where cl >nul 2>nul
IF %ERRORLEVEL% NEQ 0 goto CheckForClang
goto DownloadSDL
:CheckForClang
where clang >nul 2>nul
IF %ERRORLEVEL% NEQ 0 goto SkipDownloadSDL
:DownloadSDL
if exist "SDL2/" goto SkipDownloadSDL
mkdir SDL2
echo ----------------------------------------
echo Downloading SDL
echo ----------------------------------------
curl "https://www.libsdl.org/release/SDL2-devel-2.0.14-VC.zip" --output SDL2.zip
tar -zxvf SDL2.zip -C SDL2
del SDL2.zip
echo ----------------------------------------
echo Downloading SDL_ttf
echo ----------------------------------------
curl "https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-devel-2.0.15-VC.zip" --output SDL2_ttf.zip
tar -zxvf SDL2_ttf.zip -C SDL2
del SDL2_ttf.zip
xcopy /Y SDL2\SDL2_ttf-2.0.15\include\SDL_ttf.h SDL2\SDL2-2.0.14\include
ren "SDL2\SDL2-2.0.14\include" "SDL2"

:SkipDownloadSDL

where gcc >nul 2>nul
IF %ERRORLEVEL% NEQ 0 goto SkipDownloadSDLMinGw

if exist "SDL2MinGw/" goto SkipDownloadSDLMinGw
mkdir SDL2MinGw
echo ----------------------------------------
echo Downloading SDLMinGw
echo ----------------------------------------
curl "https://www.libsdl.org/release/SDL2-devel-2.0.14-mingw.tar.gz" --output SDL2MinGw.tar.gz
tar -xf SDL2MinGw.tar.gz -C SDL2MinGw
del SDL2MinGw.tar.gz
echo ----------------------------------------
echo Downloading SDLMinGw_ttf
echo ----------------------------------------
curl "https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-devel-2.0.15-mingw.tar.gz" --output SDL2MinGw_ttf.tar.gz
tar -xf SDL2MinGw_ttf.tar.gz -C SDL2MinGw
del SDL2MinGw_ttf.tar.gz
xcopy /Y SDL2MinGw\SDL2_ttf-2.0.15\x86_64-w64-mingw32\include\SDL2\SDL_ttf.h SDL2MinGw\SDL2-2.0.14\x86_64-w64-mingw32\include\SDL2

:SkipDownloadSDLMinGw
popd

set SDL2_Include="../../libs/SDL2/SDL2-2.0.14/"
set SDL2_Library="../../libs/SDL2/SDL2-2.0.14/lib/x64/"
set SDL2_DLL="..\..\libs\SDL2\SDL2-2.0.14\lib\x64\SDL2.dll"

set SDL2_ttf_Library="../../libs/SDL2/SDL2_ttf-2.0.15/lib/x64"
set SDL2_ttf_DLL="..\..\libs\SDL2\SDL2_ttf-2.0.15\lib\x64\*.dll"

set SDL2MinGw_Include="../../libs/SDL2MinGw/SDL2-2.0.14/x86_64-w64-mingw32/include/"
set SDL2MinGw_Library="../../libs/SDL2MinGw/SDL2-2.0.14/x86_64-w64-mingw32/lib/"
set SDL2MinGw_DLL="..\..\libs\SDL2MinGw\SDL2-2.0.14\x86_64-w64-mingw32\bin\SDL2.dll"

set SDL2MinGw_ttf_Library="../../libs/SDL2MinGw/SDL2_ttf-2.0.15/x86_64-w64-mingw32/lib/"
set SDL2MinGw_ttf_DLL="..\..\libs\SDL2MinGw\SDL2_ttf-2.0.15\x86_64-w64-mingw32\bin\*.dll"
set UIFont="..\..\fonts\anka-coder-narrow\AnkaCoder-C75-r.ttf"
set LEDFont="..\..\fonts\segment7\*.otf"

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
if not exist "ui_font.ttf" (
xcopy %UIFont% .\ /Y
ren "AnkaCoder-C75-r.ttf" "ui_font.ttf"
)
if not exist "led_font.otf" (
xcopy %LEDFont% .\ /Y
ren "Segment7Standard.otf" "led_font.otf"
)
call cl -I%SDL2_Include% %CLFlags% -nologo -Zi -EHsc %SourceFiles% -Fe%OutputName% /link /LIBPATH:%SDL2_Library% SDL2.lib SDL2main.lib Shell32.lib Comdlg32.lib /LIBPATH:%SDL2_ttf_Library% SDL2_ttf.lib /subsystem:windows
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
if not exist "ui_font.ttf" (
xcopy %UIFont% .\ /Y
ren "AnkaCoder-C75-r.ttf" "ui_font.ttf"
)
if not exist "led_font.otf" (
xcopy %LEDFont% .\ /Y
ren "Segment7Standard.otf" "led_font.otf"
)
call clang -I%SDL2_Include% -L%SDL2_Library% -L%SDL2_ttf_Library% %CLANGFlags% %SourceFiles% -o %OutputName% -lSDL2main -lSDL2 -lSDL2_ttf -lShell32 -lComdlg32 -Xlinker -subsystem:windows
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
if not exist "ui_font.ttf" (
xcopy %UIFont% .\ /Y
ren "AnkaCoder-C75-r.ttf" "ui_font.ttf"
)
if not exist "led_font.otf" (
xcopy %LEDFont% .\ /Y
ren "Segment7Standard.otf" "led_font.otf"
)
call gcc -I%SDL2MinGw_Include% -L%SDL2MinGw_Library% -L%SDL2MinGw_ttf_Library% %GCCFlags% %SourceFiles% -o %OutputName% -w -Wl,-subsystem,windows -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lComdlg32
echo Gcc Build Complete
echo ----------------------------------------
popd
goto Finished

:SkipGCC
echo Gcc not found. Skipping build with Gcc. 
echo ----------------------------------------

:Finished

@echo off

set C_FLAGS=-EHsc -nologo -FC -I../inc -DWINDOWS=1  -std:c++14
set D_FLAGS=-Zi -MT -Od -D__DEBUG
set R_FLAGS=-MD -O2 
set C_LINK_FLAGS=/link /LIBPATH:..\lib\win64 SDL2.lib SDL2main.lib freetype.lib
set R_LINK_FLAGS=/SUBSYSTEM:windows /RELEASE 
set EXPORTS=/EXPORT:step /EXPORT:sound /EXPORT:reload /EXPORT:replace /EXPORT:init /EXPORT:destroy 

set ORIGINAL_PATH=%PATH%
pushd .
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
E:
popd

pushd "bin"

REM this toggles release and debug mode.
set IS_RELEASE="" 

if %IS_RELEASE% == "yes" (
   echo "Not done..."
) else (
	del *.pdb
	cl %C_FLAGS% %D_FLAGS% /LD -Felibbear.dll ..\src\bear_main.cpp /link /PDB:libbear_%RANDOM%.pdb %EXPORTS% -incremental:no 
	cl %C_FLAGS% %D_FLAGS% -Fegame.exe ..\src\bear_windows.cpp %C_LINK_FLAGS%
	cl %C_FLAGS% %D_FLAGS% -Feasset_manager.exe ..\src\asset_handler.cpp
)

if not %ErrorLevel% == 0 (
	echo Compilation failed
	goto _END
)

copy /Y ..\lib\win64\SDL2.dll
cd ..
if "%1" == "run" (
	if %IS_RELEASE% == "yes" (
		start bin/game.exe
	) else (
		start bin/game.exe
	)
)
:_END
set PATH=%ORIGINAL_PATH%
popd
rem pause

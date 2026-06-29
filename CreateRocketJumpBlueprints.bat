@echo off
setlocal

set "PROJECT=%~dp0rocket_jump_blueprint.uproject"
set "SCRIPT=%~dp0Content\Editor\CreateRocketJumpBlueprints.py"

set "UE_EDITOR="

for %%V in (5.7 5.6 5.5 5.4 5.3 5.2 5.1 5.0) do (
  if exist "C:\Program Files\Epic Games\UE_%%V\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" (
    set "UE_EDITOR=C:\Program Files\Epic Games\UE_%%V\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
    goto :found
  )
)

:found
if "%UE_EDITOR%"=="" (
  echo UnrealEditor-Cmd.exe bulunamadi.
  echo Unreal Engine kurulum yolunu bu dosyada UE_EDITOR degiskenine elle yaz.
  pause
  exit /b 1
)

if not exist "%PROJECT%" (
  echo Bulunamadi: %PROJECT%
  pause
  exit /b 1
)

if not exist "%SCRIPT%" (
  echo Bulunamadi: %SCRIPT%
  pause
  exit /b 1
)

echo Unreal: %UE_EDITOR%
echo Project: %PROJECT%
echo Script: %SCRIPT%

call "%UE_EDITOR%" "%PROJECT%" -run=pythonscript -script="%SCRIPT%" -unattended -nop4 -nosplash
set "RESULT=%errorlevel%"

echo.
if not "%RESULT%"=="0" (
  echo Blueprint olusturma basarisiz. Hata kodu: %RESULT%
) else (
  echo Blueprint assetleri olusturuldu.
)

pause
exit /b %RESULT%

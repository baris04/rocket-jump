@echo off
REM Unreal Editor ile bu projeyi acar (cift tik / dosya iliskisi gerekmez).
set "UE_EDITOR=C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe"
set "PROJECT=%~dp0rocket_jump.uproject"

if not exist "%UE_EDITOR%" (
  echo Bulunamadi: %UE_EDITOR%
  echo Epic Games icinde UE 5.7 yolunu bu dosyada duzelt.
  pause
  exit /b 1
)

if not exist "%PROJECT%" (
  echo Bulunamadi: %PROJECT%
  pause
  exit /b 1
)

start "" "%UE_EDITOR%" "%PROJECT%"

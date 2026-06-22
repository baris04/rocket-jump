@echo off
REM Editor kapaliyken calistir: modulleri yeniden derler.
set "BUILD_BAT=C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat"
set "PROJECT=%~dp0rocket_jump.uproject"

if not exist "%BUILD_BAT%" (
  echo Bulunamadi: %BUILD_BAT%
  pause
  exit /b 1
)

call "%BUILD_BAT%" rocket_jumpEditor Win64 Development -Project="%PROJECT%" -WaitMutex
echo.
if errorlevel 1 (
  echo Derleme basarisiz. Hata kodu: %errorlevel%
) else (
  echo Derleme tamam. Simdi OpenRocketJumpProject.bat calistirabilirsin.
)
pause

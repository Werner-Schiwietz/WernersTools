@rem
@echo off

@rem cd /d %~p0

:nextpid
if "%~1" equ "" exit /b 0

@echo %1

call pid_activ %1
if "%errorlevel%" equ "0" (
	shift
) else (
	timeout /T 2 >nul
)
goto nextpid
 

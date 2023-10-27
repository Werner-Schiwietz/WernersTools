@rem wait_pids by werner schiwietz source@gisbw.de 2023
@rem wartet, bis alle prozesse mit den pids als einzelparameter beendet sind


@rem
@echo off


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
 

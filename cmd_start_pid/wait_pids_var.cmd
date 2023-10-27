@rem wait_pids_var by werner schiwietz source@gisbw.de 2023
@rem wartet, bis alle prozesse mit den pids, als ein parameter in "15220 10983 ..." beendet sind

@rem 
@echo off

@setlocal enabledelayedexpansion

@echo %~n0%~x0    "wait on pids %~1"

@for /f "tokens=3 delims=. " %%i in ('@chcp') do @set codepage=%%i
@chcp 65001 >nul


set myparams=%~1

call :endlos

@rem aufräumen
@chcp %codepage% >nul
@endlocal
exit /b

:endlos
@rem doing

@rem myparams leer wenn fertig
if "!myparams!" equ "" exit /b 0
set pids_running=

@rem über alle pids
for %%i in ( !myparams! ) do (
	call %~dp0pid_activ.cmd %%i
	if !errorlevel! neq 0 (
		set pids_running=!pids_running! %%i
	) else (
		echo %%i läuft nicht mehr
	)
)

@rem die restlichen pids für nächsten durchlauf setzen
set myparams=!pids_running!

@rem myparams leer wenn fertig
if "!myparams!" equ "" exit /b 0

@rem laufender,horizontaler punkt als lebensanzeige
@rem @echo|@set /p=.
@<nul set /p=.


@rem ggf 2 sekunden warten
timeout /T 2 >nul
goto endlos

 

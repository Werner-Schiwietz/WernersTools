@rem pid_activ by werner schiwietz source@gisbw.de 2023
@rem liefert als errorlevel die pid wenn der prozess läuft und 0 wenn er nicht läuft

@setlocal enabledelayedexpansion
@rem 
@echo off
@rem @echo %~n0%~x0

for /f  "tokens=2,7 delims= " %%i in ('tasklist /nh /fi "PID eq %1"') do (
	if "%%j" == "" (
		@rem echo pid %1 ist noch aktiv
		endlocal
		exit /b %%i
	)
)

@rem echo pid %1 pid nicht aktiv
endlocal


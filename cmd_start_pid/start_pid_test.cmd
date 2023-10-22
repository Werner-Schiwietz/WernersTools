@rem ein test cmd

@echo off
@setlocal enabledelayedexpansion

set PIDS=
@rem cd /d %~p0
 
echo %CD%

echo öffne zwei consolen die manuell geschlossen werden sollen 

call :subStartProcess_RealizePID start_pid.cmd "cmd 1" cmd /k dir /s
call :subStartProcess_RealizePID start_pid.cmd "cmd 2" cmd /k dir /s


@rem warten, bis die consolen beendet wurden
@rem call %~p0\wait_pids %PIDS%
call %~dp0\wait_pids_var "%PIDS%"

exit /b


:subStartProcess_RealizePID
call "%~dp0%~1" "%~2" %3 %4 %5 %6 %7 %8 %9 >nul
set pid=!errorlevel!
set pids=!pids! !pid!
echo PID=!pid! "%~1 "%~2" %3 %4 %5 %6 %7 %8 %9 

exit /b 
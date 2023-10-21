@rem ein test cmd

@echo off

@setlocal
set PIDS=
@rem cd /d %~p0
 
echo %CD%
 
call %~p0\start_pid.cmd "cmd 1" cmd /k dir /s 
set pids=%pids% %errorlevel%

call %~p0\start_pid.cmd "cmd 2" cmd /k dir /s 
set pids=%pids% %errorlevel%

echo PIDS = %PIDS%

@rem call %~p0\wait_pids %PIDS%
call %~dp0\wait_pids_var "%PIDS%"

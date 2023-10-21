@rem
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

call wait_pids %PIDS%

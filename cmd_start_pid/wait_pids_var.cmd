﻿@rem wartet, bis alle prozesse mit den pids, als ein parameter in "15220 10983 ..." beendet sind

@setlocal enabledelayedexpansion
@rem 
@echo off

@echo %~n0%~x0    "wait on pids %~1"

@chcp 65001 >nul

set myparams=%~1


:nextpid

if "!myparams!" equ "" exit /b 0
set pids_running=


for %%i in ( !myparams! ) do (
	call %~dp0pid_activ.cmd %%i
	if !errorlevel! neq 0 (
		set pids_running=!pids_running! %%i
	) else (
		echo %%i läuft nicht mehr
	)
)

set myparams=!pids_running!
if "!myparams!" neq "" timeout /T 2 >nul
goto nextpid

 
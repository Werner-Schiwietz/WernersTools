@rem
@echo off


for /f  "tokens=2,7 delims= " %%i in ('tasklist /nh /fi "PID eq %1"') do (
	if "%%j" == "" (
		echo pid %1 ist noch aktiv
		exit /b %%i
	)
)

echo pid %1 pid nicht aktiv
exit /b 0	


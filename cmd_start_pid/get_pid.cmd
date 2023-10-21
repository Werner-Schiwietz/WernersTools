 
@rem liefert die pid Windowstitel, wenn Dieser eindeutig ist
@rem %1 muss der Titel des Fensters sein, sonst klappt das nicht wird um wildcard * erweitert
@rem die PID wird über die console ausgegeben und als %errorlevel% geliefert. im fehlerfall ist der errorlevel 0
 
@rem 
@echo off
@rem echo %~1

if "%~1" equ "" exit /b 0


for /f  "tokens=2,7 delims= " %%i in ('tasklist /nh /fi "WINDOWTITLE eq %~1*"') do (
	@rem wenn 7tes token text enthält -> fehler prozess wurde nicht gefunden
	if "%%j" equ "" (
              echo %%i 
              exit /b %%i
	)
)

@rem fehler
exit /b 0
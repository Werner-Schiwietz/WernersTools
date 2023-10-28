@rem start_pid by werner schiwietz source@gisbw.de 2023
@rem liefert die pid zum gestarteten Prozess, wenn der Windowstitel eindeutig ist
@rem %1 muss der Titel des Command-Fensters sein, sonst klappt das nicht
@rem die PID wird über die console ausgegeben und als %errorlevel% geliefert
 
@rem
@echo off
 
if "%~2" equ "" exit /b 0
 
start "%~1" %2 %3 %4 %5 %6 %7 %8 %9

call %~dp0\get_pid.cmd "%~1"

exit /b %errorlevel%
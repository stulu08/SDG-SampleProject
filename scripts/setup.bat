@echo off

python --version 3>NUL
if errorlevel 1 goto errorNoPython

python python/setup.py
PAUSE
exit


:errorNoPython
echo Trying to install python from Microsoft Store
python
echo After installing python execute this file again
pause
exit
@echo off

set mapname=%1

rem =================================================>
rem Update Configs Here

rem example : 
rem set AIBOOTCAMP2_ENGINE_DIR=D:\AIBootcamp2\AIBootcamp2\Releases
rem set AIBOT_PATH=D:\AIBootcamp2\LesOutardes\build-solution-x64\Debug\lesoutardes.dll
rem set TEAM=LesOutardes

set AIBOOTCAMP2_ENGINE_DIR="..\AIBootCamp2-2024.2.01"
set AIBOT_PATH=".\build-solution-x64\Debug\myfirstaibot.dll"
set TEAM="LesArchitectes"

rem in s, -1 = default
set ConnectToProcessDelay=10

rem in ms, -1 = default
set InitTime=-1
set TurnTime=5000000

rem <=================================================

python PlayLocalMatch.py %mapname% %AIBOOTCAMP2_ENGINE_DIR% %AIBOT_PATH% %TEAM% %ConnectToProcessDelay% %InitTime% %TurnTime%

exit /b
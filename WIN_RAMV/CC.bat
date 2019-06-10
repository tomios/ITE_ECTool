@echo off
REM  1. For compile the cpp file
REM  2. 

@find "//#pragma" %1
if '%errorlevel%' EQU '1' (
	echo No "//#pragma" in the %1
	goto error
)

@find "//#pragma" %1 | find "WinIo.lib"

:: It's find the  //#pragma comment(lib,"WinIo.lib")
:: So, compile the x64
if '%errorlevel%' EQU '0' goto x64

:: It's don't find the  //#pragma comment(lib,"WinIo.lib")
:: So, compile the x86
if '%errorlevel%' EQU '1' goto x86

goto error


:x86
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x86
cl.exe /Fe%~n1_x86.exe %1
goto end


:x64
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x86_amd64
cl.exe /Fe%~n1_x64.exe %1
goto end

:error
echo.
echo Please specify the file name
echo Example : CC.bat 123.cpp
echo.


:end
del %~n1.obj
echo.
echo Done
echo.

@echo off
setlocal

set dest=%AppData%\owclauncher
set owcl_exe=owclauncher.exe
set owcl_u=UninstallWin.bat
set owcl_c=owcl_chrome.json
set owcl_f=owcl_firefox.json
set owcl_c_reg=HKCU\Software\Google\Chrome\NativeMessagingHosts\open_with
set owcl_f_reg=HKCU\Software\Mozilla\NativeMessagingHosts\open_with

IF NOT EXIST "%owcl_exe%" (
    echo %owcl_exe% not found, installation will stop
    pause
    exit /b
)

if NOT "%cd%"=="%dest%" (
    if not exist %dest% md %dest%
    if exist "%dest%\%owcl_exe%" (
        for /F "tokens=*" %%# in ('del /F /Q "%dest%\%owcl_exe%" 2^>^&1 1^> nul') do (
            echo Deleting existing %owcl_exe% failed, installation will stop
            pause
            exit /b
        )
        echo Deleting existing %owcl_exe%.......................Done
    )
    
    copy "%owcl_exe%" "%dest%" > nul
    if ERRORLEVEL==1 (
        echo Error copying %owcl_exe% to destination, installation will stop
        pause
        exit /b
    )
    echo Copying %owcl_exe%.................................Done
    
    if exist "%owcl_u%" (
        copy /Y "%owcl_u%" "%dest%" > nul
        if ERRORLEVEL==0 (
            echo Copying %owcl_u%................................Done
        )
    )
    cd /D "%dest%"
)

IF EXIST "%owcl_c%" del "%owcl_c%"
IF EXIST "%owcl_f%" del "%owcl_f%"

set owclpath="%dest%\%owcl_exe%"
set owclpath=%owclpath:\=\\%

set owcl_cpath="%dest%\%owcl_c%"
set owcl_fpath="%dest%\%owcl_f%"

@echo {> %owcl_c%
@echo 	"allowed_origins": [>> %owcl_c%
@echo 		"chrome-extension://cogjlncmljjnjpbgppagklanlcbchlno/",>> %owcl_c%
@echo 		"chrome-extension://fbmcaggceafhobjkhnaakhgfmdaadhhg/">> %owcl_c%
@echo 	],>> %owcl_c%
@echo 	"description": "Open With native host",>> %owcl_c%
@echo 	"name": "open_with",>> %owcl_c%
@echo 	"path": %owclpath%,>> %owcl_c%
@echo 	"type": "stdio">> %owcl_c%
@echo }>> %owcl_c%

echo Generating %owcl_c%.............................Done

for /F "tokens=*" %%# in ('reg add "%owcl_c_reg%" /f /t REG_SZ /d %owcl_cpath% 2^>^&1 1^> nul') do (
    echo Updating registry entry for %owcl_c%............Failed
    set "cregfailed=1"
)
if not defined cregfailed echo Updating registry entry for %owcl_c%............Done

@echo {> %owcl_f%
@echo 	"allowed_extensions": [>> %owcl_f%
@echo 		"openwith@darktrojan.net">> %owcl_f%
@echo 	],>> %owcl_f%
@echo 	"description": "Open With native host",>> %owcl_f%
@echo 	"name": "open_with",>> %owcl_f%
@echo 	"path": %owclpath%,>> %owcl_f%
@echo 	"type": "stdio">> %owcl_f%
@echo }>> %owcl_f%

echo Generating %owcl_f%............................Done

for /F "tokens=*" %%# in ('reg add "%owcl_f_reg%" /f /t REG_SZ /d %owcl_fpath% 2^>^&1 1^> nul') do (
    echo Updating registry entry for %owcl_f%...........Failed
    set "fregfailed=1"
)
if not defined fregfailed echo Updating registry entry for %owcl_f%...........Done

echo Installation completed at %dest%
pause
exit /b
@echo off
setlocal

set dest=%AppData%\owclauncher
set owcl_exe=owclauncher.exe
set owcl_u=UninstallWin.bat
set owcl_c=owcl_chrome.json
set owcl_f=owcl_firefox.json
set owcl_c_reg=HKCU\Software\Google\Chrome\NativeMessagingHosts\open_with
set owcl_f_reg=HKCU\Software\Mozilla\NativeMessagingHosts\open_with

if exist %dest% (
    if exist "%dest%\%owcl_exe%" (
        for /F "tokens=*" %%# in ('del /F /Q "%dest%\%owcl_exe%" 2^>^&1 1^> nul') do (
            echo Deleting %owcl_exe%................................failed
            set "exefailed=1"
        )
        if not defined exefailed set "found=1" & echo Deleting %owcl_exe%................................Done
    )

    if exist "%dest%\%owcl_c%" (
        for /F "tokens=*" %%# in ('del /F /Q "%dest%\%owcl_c%" 2^>^&1 1^> nul') do (
            echo Deleting %owcl_c%...............................failed
            set "cfailed=1"
        )
        if not defined cfailed set "found=1" & echo Deleting %owcl_c%...............................Done
    )

    if exist "%dest%\%owcl_f%" (
        for /F "tokens=*" %%# in ('del /F /Q "%dest%\%owcl_f%" 2^>^&1 1^> nul') do (
            echo Deleting %owcl_f%..............................failed
            set "ffailed=1"
        )
        if not defined ffailed set "found=1" & echo Deleting %owcl_f%..............................Done
    )
    if exist "%dest%\%owcl_u%" set "found=1"
    
    if defined found (    
        for /F "tokens=*" %%# in ('reg query "%owcl_c_reg%" 2^>^&1 1^> nul') do (
            set "cregabsent=1"
        )
        if not defined cregabsent (
            for /F "tokens=*" %%# in ('reg delete "%owcl_c_reg%" /f 2^>^&1 1^> nul') do (
                echo Removing registry entry for %owcl_c%............Failed
                set "cregfailed=1"
            )
            if not defined cregfailed echo Removing registry entry for %owcl_c%............Done
        )
        
        for /F "tokens=*" %%# in ('reg query "%owcl_f_reg%" 2^>^&1 1^> nul') do (
            set "fregabsent=1"
        )
        if not defined fregabsent (
            for /F "tokens=*" %%# in ('reg delete "%owcl_f_reg%" /f 2^>^&1 1^> nul') do (
                echo Removing registry entry for %owcl_f%...........Failed
                set "fregfailed=1"
            )
            if not defined fregfailed echo Removing registry entry for %owcl_f%...........Done
        )
        
        echo Uninstall completed.........................................
        pause
        
        dir /b /ad "%dest%\*" | >nul findstr "^" && (call :rembat) || (set "nofolders=1")
        
        if defined nofolders (
            cd /D %dest%
            for /f %%a in ('dir /b /a-d ^| find /c /v ""') do (
                if %%a==0 (call :remdirectory)
                if %%a==1 (if exist "%dest%\%owcl_u%" call :remdirectory)
                call :rembat
            )
        )
        
    ) else (
        echo Nothing to delete ! Uninstall will exit
        pause
        exit /b
    )
) else (
    echo %dest% not found !, Uninstall will exit
    pause
    exit /b
)

:rembat
if exist "%dest%\%owcl_u%" (goto) 2>nul & del /F /Q "%dest%\%owcl_u%"
exit /b

:remdirectory
cd /D %dest%
cd ..
rmdir "%dest%" /S /Q & exit /b
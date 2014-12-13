@echo off

set DATA_DIR=test-consul-data

if "%1"=="start" (
    consul info > nul
    if NOT ERRORLEVEL 1 (echo Consul is already running && exit /b 1)
    if EXIST "%DATA_DIR%" (rd /S /Q "%DATA_DIR%" || echo Can not delete data dir && exit /b 1)
    start consul agent -server -bootstrap-expect=1 "-data-dir=%DATA_DIR%"
    timeout 3
    consul info > nul
    if ERRORLEVEL 1 (echo Can not start Consul && exit /b 1)
    echo Consul started successfully
    exit /b 0
)

if "%1"=="stop" (
    consul info > nul
    if ERRORLEVEL 1 (echo Consul is not running && exit /b 0)
    consul leave
    if ERRORLEVEL 1 exit /b 1
    exit /b 0
)

echo Usage: test-consul start^|stop
exit /b 2
